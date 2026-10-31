"""Move complete announcement playback and track switching into C11."""
import re
import timer_migration as timer
import audio_music_service_migration as service
from audio_controls_migration import function
METHODS=['ServiceAnnouncement','ServiceTrack']
PROTOTYPES='''uint8_t MusicStream_ServiceAnnouncement(cMusicManager *manager);
void MusicStream_ServiceTrack(cMusicManager *manager, CVehicle *vehicle, CPed *ped);
uint8_t MusicStreamHost_SlowMotion(void);
CEntity *MusicStreamHost_Target(void);
void MusicStreamHost_TargetPosition(CEntity *target, AudioSoundPosition *position);
'''
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint8_t
MusicStreamHost_SlowMotion(void)
{
    // Read the original slow motion gate without the reflection subsystem's build guards
    return CTimer::GetIsSlowMotionActive();
}
extern "C" CEntity *
MusicStreamHost_Target(void)
{
    // Read the live camera target at each original target access
    return TheCamera.pTargetEntity;
}
extern "C" void
MusicStreamHost_TargetPosition(CEntity *target, AudioSoundPosition *position)
{
    // Copy the original target coordinates for C distance and panning calculations
    *position = AudioSoundPosition_FromVector(target->GetPosition());
}
//- rouz edit (ChatGPT)
'''
STORE='''#if defined(_MSC_VER) && !defined(__clang__)
#define MUSIC_STREAM_STORE_TRACK_VALUE(manager, field, expression) do { \\
    /* Capture the provider result before selecting the live destination track */ \\
    uint32_t result = (expression); \\
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = result; \\
} while (0)
#else
#define MUSIC_STREAM_STORE_TRACK_VALUE(manager, field, expression) \\
    (manager)->m_aTracks[(manager)->m_nPlayingTrack].field = (expression)
#endif
'''
HELPERS='''static float
MusicStream_TargetDistanceSquared(void)
{
    // Preserve the original unweighted sum of coordinate squares
    AudioSoundPosition target, camera;
    float x, y, z;
    MusicStreamHost_TargetPosition(MusicStreamHost_Target(), &target);
    AudioGeometryHost_CameraPosition(&camera);
    x = target.x - camera.x;
    y = target.y - camera.y;
    z = target.z - camera.z;
    return x*x + y*y + z*z;
}

static void
MusicStream_TranslateTarget(AudioSoundPosition *translated)
{
    // Capture the live target coordinates before applying the existing C camera transform
    AudioSoundPosition target;
    MusicStreamHost_TargetPosition(MusicStreamHost_Target(), &target);
    AudioGeometry_Translate(&target, translated);
}
'''

def calls(source):
    # Route each remaining game-mode call with its original captured owner and vehicle
    source=source.replace('MusicManager.ServiceAnnouncement()', 'MusicStream_ServiceAnnouncement(&MusicManager)')
    source=source.replace('MusicManager.ServiceTrack(', 'MusicStream_ServiceTrack(&MusicManager, ')
    if 'cMusicManager::cMusicManager(' in source:
        source=re.sub(r'(?<![\w.:>])ServiceAnnouncement\(', 'MusicStream_ServiceAnnouncement(this',source)
        source=re.sub(r'(?<![\w.:>])ServiceTrack\(', 'MusicStream_ServiceTrack(this, ',source)
    return service.calls(source)

def logic(source):
    # Remove complete converted members while preserving remaining gameplay service logic
    owner='cMusicManager::cMusicManager(' in source
    for name in METHODS:
        if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
    source=calls(source)
    if owner and 'MusicStreamHost_Target(void)' not in source:source+='\n'+HOST
    # Recognize the same readers after timer calls have been converted
    host=timer.calls(HOST) if timer.calls(HOST) in source else HOST
    if host in source:
        # Keep new readers after adapters inserted by preceding migration stages
        source=source.replace(host,'')+host
    return service.logic(source)

def header(source):
    # Preserve record fields while exposing C types and both complete operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+n+r'\(',line) for n in METHODS))
    source=source.replace('#include <stdint.h>','#include <stdint.h>\n#include "AudioSound.h"',1)
    source=source.replace('class CPed;\n#else\ntypedef struct CVehicle CVehicle;', 'class CPed;\nclass CEntity;\n#else\ntypedef struct CVehicle CVehicle;\ntypedef struct CPed CPed;\ntypedef struct CEntity CEntity;',1)
    return service.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Retain shared flags, delayed stream acknowledgement, live track writes and volume arithmetic
    source=source[source.index('{'):]
    literals=[]
    def literal(match):
        # Protect diagnostic text from identifier substitutions inside code
        literals.append(match.group());return '_STREAM_LITERAL_'+str(len(literals)-1)+'_'
    source=re.sub(r'"(?:\\.|[^"\\])*"',literal,source)
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('nFramesSinceCutsceneEnded','manager->nFramesSinceCutsceneEnded')
    source=source.replace('MusicPositions_RecordRadioStats(this)','MusicPositions_RecordRadioStats(manager)').replace('MusicPositions_GetTrackStartPos(this,','MusicPositions_GetTrackStartPos(manager,').replace('MusicAmbience_ComputeAmbienceVol(this,','MusicAmbience_ComputeAmbienceVol(manager,')
    source=source.replace('CTimer::GetTimeInMillisecondsPauseMode()','MusicInitHost_PauseTime()').replace('CTimer::GetIsSlowMotionActive()','MusicStreamHost_SlowMotion()')
    source=source.replace('veh->m_nRadioStation','(*MusicRadioHost_Station(veh))')
    source=source.replace('(TheCamera.pTargetEntity->GetPosition() - TheCamera.GetPosition()).MagnitudeSqr()', 'MusicStream_TargetDistanceSquared()')
    source=source.replace('AudioGeometry_TranslateVector(&TheCamera.pTargetEntity->GetPosition(), &panVec)', 'MusicStream_TranslateTarget(&panVec)').replace('TheCamera.pTargetEntity','MusicStreamHost_Target()').replace('CVector panVec','AudioSoundPosition panVec')
    source=source.replace('Min(','MUSIC_STREAM_MIN(').replace('SQR(','MUSIC_STREAM_SQUARE(').replace('Sqrt(','sqrtf(')
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('bool8','uint8_t'),('FALSE','0'),('TRUE','1')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for field,expression in [('m_nPosition','SampleManager_GetStreamedFilePosition(&SampleManager, 0)'),('m_nLastPosCheckTimer','MusicInitHost_PauseTime()')]:
        source=source.replace('manager->m_aTracks[manager->m_nPlayingTrack].'+field+' = '+expression+';', 'MUSIC_STREAM_STORE_TRACK_VALUE(manager, '+field+', '+expression+');')
    for i,text in enumerate(literals):source=source.replace('_STREAM_LITERAL_'+str(i)+'_',text)
    return source.replace('debug("','re3_debug("[DBG]: ')

def project(source):
    # Register both complete streaming operations as C11
    if 'MusicStream.c' in source:return service.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicRadio.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return service.project(source[:b]+'\n'+source[a:b].replace('MusicRadio.c','MusicStream.c')+source[b:])
