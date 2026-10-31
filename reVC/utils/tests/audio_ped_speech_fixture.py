# Compare speech setup and player controls against their complete original C++ bodies
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_ped_speech_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-ped-speech-c-tests'
def normalize(source):
    # Ignore only comments and whitespace in complete statement comparisons
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioPedSpeech.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    # Audit every statement in all four complete owner operations
    original=function(old,'cAudioManager::',name);methods.append(original);actual=function(kernel,'AudioPedSpeech_',target)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioEnvironment.c',migration.calls),('src/audio/AudioPedOneShots.c',migration.calls),('src/audio/AudioVehicleOneShots.c',migration.calls),('src/audio/DMAudio.cpp',migration.facade),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(__import__('audio_ped_talk_migration').logic(migration.HOST)) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
for name in ['SetPedTalkingStatus','SetPlayersMood']:
    # Check the public wrappers against the original global-owner calls
    original=function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_',name);body=original[original.index('{'):].replace('return AudioManager.SetPedTalkingStatus(ped, status);','AudioPedSpeech_SetTalking(ped, status);').replace('return AudioManager.SetPlayersMood(mood, time);','AudioPedSpeech_SetMood(&AudioManager, mood, time);')
    actual=function(kernel,'DMAudio_',name);assert normalize(body)==normalize(actual[actual.index('{'):])
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','AudioPoliceGame.h','audio_enums.h'])
constants='\n'.join(line for line in old.splitlines() if re.match(r'\s*PED_COMMENT_\w+\s*=',line))
volume_constants='\n'.join(line for line in header.splitlines() if re.match(r'\s*PED_COMMENT_VOLUME(?:_BEHIND_WALL)?\s*=',line))
for name in ['PED_COMMENT_MAX_DIST','PED_COMMENT_POLICE_HELI_MAX_DIST']:
    assert re.search(name+r'\s*=\s*(\d+)',old).group(1)==re.search(name+r'\s*=\s*(\d+)',kernel).group(1)
declarations='\n'.join(function(old,'cAudioManager::',name).split('{',1)[0].strip().replace('cAudioManager::','')+';' for name in migration.NAMES)
template=(root/'utils/tests/audio_ped_speech.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@CONSTANTS@',constants).replace('@VOLUME_CONSTANTS@',volume_constants).replace('@DECLARATIONS@',declarations)
for variant in ['before','after']:
    # Insert the original bodies or call the separately compiled C operations
    (out/(variant+'.cpp')).write_text(('#define TEST_ORIGINAL\n' if variant=='before' else '')+template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','runOriginal(kind,seed)' if variant=='before' else 'TestPedSpeechC(active,&params,kind,seed)'),newline='\n')
caller='''#include "AudioManager.h"
#include "DMAudio.h"
void TestPedSpeechC(cAudioManager *manager,cPedParams *params,unsigned kind,unsigned seed)
{
    // Exercise every C operation and both public wrappers through actual declarations
    switch(kind) {
    case 0: AudioPedSpeech_SetTalking(seed%7?params->m_pPed:NULL,(uint8_t)seed);break;
    case 1: AudioPedSpeech_SetMood(manager,(uint8_t)(seed%7),seed%11?seed*1000:UINT32_MAX);break;
    case 2: AudioPedSpeech_Mood(manager);break;
    case 3: AudioPedSpeech_Setup(manager,params,(uint16_t)(seed%5==0?SOUND_PED_HELI_PLAYER_FOUND:seed%5==1?SOUND_PED_VCPA_PLAYER_FOUND:seed%5==2?SOUND_INJURED_PED_MALE_OUCH:seed%5==3?SOUND_INJURED_PED_FEMALE:SOUND_BOAT_SLOWDOWN));break;
    case 4: DMAudio_SetPedTalkingStatus(seed%7?params->m_pPed:NULL,(uint8_t)seed);break;
    case 5: DMAudio_SetPlayersMood((uint8_t)(seed%7),seed%11?seed*1000:UINT32_MAX);break;
    }
}
'''
(out/'caller.c').write_text(caller,newline='\n');(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Four complete speech/control bodies, two public facades, direct callers, adapters and unchanged owner layout audited')
