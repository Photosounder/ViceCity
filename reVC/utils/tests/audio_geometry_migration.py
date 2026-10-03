"""Known C camera geometry migration for complete audio source audits."""
import re
import audio_requests_migration as requests
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\nvoid\nAudioGeometryHost_CameraPosition(AudioSoundPosition *position)\n{\n    // Copy the existing camera coordinates without assuming the game vector layout\n    const CVector &camera = TheCamera.GetPosition();\n    position->x = camera.x;\n    position->y = camera.y;\n    position->z = camera.z;\n}\n\nvoid\nAudioGeometryHost_CameraMatrix(AudioCameraMatrix *matrix)\n{\n    // Copy only the basis and translation fields used by the original inverse multiply\n    const CMatrix &camera = TheCamera.GetMatrix();\n\tmatrix->rx = camera.rx;\n\tmatrix->ry = camera.ry;\n\tmatrix->rz = camera.rz;\n\tmatrix->fx = camera.fx;\n\tmatrix->fy = camera.fy;\n\tmatrix->fz = camera.fz;\n\tmatrix->ux = camera.ux;\n\tmatrix->uy = camera.uy;\n\tmatrix->uz = camera.uz;\n\tmatrix->px = camera.px;\n\tmatrix->py = camera.py;\n\tmatrix->pz = camera.pz;\n}\n//- rouz edit (ChatGPT)\n'
BRIDGE='\nstatic inline float\nAudioGeometry_DistanceVector(const CVector &position)\n{\n    // Copy the game coordinates before calling the C camera distance calculation\n    const AudioSoundPosition plain = AudioSoundPosition_FromVector(position);\n    return AudioGeometry_DistanceSquared(&plain);\n}\n\nstatic inline void\nAudioGeometry_TranslateVector(const CVector *position, CVector *translated)\n{\n    // Preserve an input snapshot and complete the C transform before writing the game vector\n    const AudioSoundPosition plain = AudioSoundPosition_FromVector(*position);\n    AudioSoundPosition result;\n    AudioGeometry_Translate(&plain, &result);\n    *translated = AudioSoundPosition_ToVector(&result);\n}\n'

def calls(source):
    # Bind plain positions directly to C and keep game vector copies at the boundary
    source=source.replace('AudioMissionHost_GetDistanceSquared(manager, ', 'AudioGeometry_DistanceSquared(').replace('AudioMissionHost_Translate(manager, ', 'AudioGeometry_Translate(')
    source=re.sub(r'(?<![\w.:>])GetDistanceSquared\(AudioSoundPosition_ToVector\(([^\n]*?)\)\)',r'AudioGeometry_DistanceSquared(\1)',source)
    source=source.replace('GetDistanceSquared(pos)', 'AudioGeometry_DistanceVector(pos)')
    source=source.replace('AudioManager.TranslateEntity(', 'AudioGeometry_TranslateVector(')
    source=re.sub(r'(?<![\w.:>])TranslateEntity\(', 'AudioGeometry_TranslateVector(',source)
    if '#include "AudioManager.h"' in source and '#include "AudioSoundGame.h"' not in source and re.search(r'AudioGeometry_(?:Distance|Translate)Vector\(',source):
        source=source.replace('#include "AudioManager.h"','#include "AudioManager.h"\n#include "AudioSoundGame.h"')
    return requests.radio(requests.owner(source))

def logic(source):
    # Remove the owner arithmetic and obsolete mission wrappers before updating callers
    for name in ('GetDistanceSquared','TranslateEntity'):
        if 'cAudioManager::'+name+'(' in source:
            source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    if re.search(r'(?m)^AudioMissionHost_GetDistanceSquared\(',source):
        for name in ('GetDistanceSquared','Translate'):
            source=source.replace(function(source,'AudioMissionHost_',name)+'\n','')
        source=source.replace('#pragma endregion All the mission audio stuff',HOST+'#pragma endregion All the mission audio stuff')
    return calls(source)

def header(source):
    # Remove only the migrated methods and obsolete wrappers while retaining stored fields
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:GetDistanceSquared|TranslateEntity|AudioMissionHost_GetDistanceSquared|AudioMissionHost_Translate)\(',line))
    return requests.header(source.replace('#include "AudioMath.h"','#include "AudioMath.h"\n#include "AudioGeometry.h"'))

def game_header(source):
    # Adapt vectors by coordinate copies without relying on a C++ object representation
    return source.replace('#include "AudioSound.h"','#include "AudioSound.h"\n#include "AudioGeometry.h"').replace('//- rouz edit (ChatGPT)',BRIDGE+'//- rouz edit (ChatGPT)')
