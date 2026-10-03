"""Audit camera geometry and compare actual C kernels with original C++ arithmetic."""
from pathlib import Path
import re
from audio_controls_migration import function
import audio_geometry_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-geometry-c-tests'
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text()
distance=function(old,'cAudioManager::','GetDistanceSquared')
translate=function((out/'before/src/audio/AudioManager.cpp').read_text(),'cAudioManager::','TranslateEntity')
inverse=function((root/'src/math/Matrix.h').read_text(),'','MultiplyInverse')
kernel=(root/'src/audio/AudioGeometry.c').read_text()
expected=distance[distance.index('{'):].replace('const CVector &c = TheCamera.GetPosition();','AudioSoundPosition camera; AudioGeometryHost_CameraPosition(&camera);').replace('sq(','AudioGeometry_Square(').replace('v.','position->').replace('c.','camera.')
actual=function(kernel,'AudioGeometry_','DistanceSquared');assert normalize(expected)==normalize(actual[actual.index('{'):])
assert normalize(function(kernel,'AudioGeometry_','Square'))==normalize('static float AudioGeometry_Square(float value) { return value * value; }')
expected=inverse[inverse.index('{'):].replace('CVector v(vec.x - mat.px, vec.y - mat.py, vec.z - mat.pz);','AudioCameraMatrix matrix; AudioSoundPosition relative, result; AudioGeometryHost_CameraMatrix(&matrix); relative.x = position->x - matrix.px; relative.y = position->y - matrix.py; relative.z = position->z - matrix.pz;')
expected=expected.replace('return CVector(','result.x = ').replace('mat.','matrix.').replace('v.','relative.').replace(',\n',';\nresult.y = ',1).replace(',\n',';\nresult.z = ',1).replace('matrix.uz * relative.z);','matrix.uz * relative.z; *translated = result;')
actual=function(kernel,'AudioGeometry_','Translate');assert normalize(expected)==normalize(actual[actual.index('{'):])
for name,transform in [('AudioLogic.cpp',migration.logic),('AudioManager.cpp',migration.logic),('AudioCollision.cpp',migration.logic),('MusicManager.cpp',migration.logic),('AudioMissionPlayback.c',migration.calls),('AudioManager.h',migration.header),('AudioSoundGame.h',migration.game_header)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
assert 'CMatrix &GetMatrix(void) { return m_matrix; }' in (root/'src/core/Placeable.h').read_text()
assert 'const CVector &GetPosition(void) { return m_matrix.GetPosition(); }' in (root/'src/core/Placeable.h').read_text()
assert 'inline float sq(float x) { return x*x; }' in (root/'src/core/common.h').read_text()
template=(root/'utils/tests/audio_geometry.cpp.in').read_text()
bridge=(root/'src/audio/AudioSoundGame.h').read_text().replace('#pragma once','').replace('#include "common.h"','')
source=template.replace('@DISTANCE@',distance).replace('@TRANSLATE@',translate).replace('@INVERSE@',inverse).replace('@HOST@',migration.HOST).replace('@BRIDGE@',bridge)
(out/'compare.cpp').write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioGeometry.h"\nfloat TestGeometryDistanceC(const AudioSoundPosition *position)\n{\n    // Call the production distance export through its C declaration\n    return AudioGeometry_DistanceSquared(position);\n}\nvoid TestGeometryTranslateC(const AudioSoundPosition *position, AudioSoundPosition *translated)\n{\n    // Call the production translation export with separate or shared position storage\n    AudioGeometry_Translate(position, translated);\n}\n',newline='\n')
print('Original formulas, matrix convention, camera adapters, and complete caller edits audited')
