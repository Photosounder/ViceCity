# Audit complete collision generators and compare actual C queue submissions
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_collision_sounds_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-collision-sounds-c-tests'
def normalize(s):
    # Compare all production statements independently of comments and layout
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioCollision.cpp').read_text();kernel=(root/'src/audio/AudioCollisionSounds.c').read_text()
for name,target in migration.NAMES.items():
    method=function(old,'cAudioManager::',name);actual=function(kernel,'AudioCollisionSounds_',target)
    assert normalize(migration.body(method))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.table(old))==normalize(kernel[kernel.index('static const uint32_t gOneShotCol[]'):kernel.index(';',kernel.index('static const uint32_t gOneShotCol[]'))+1])
for name,transform in [('AudioCollision.cpp',migration.owner),('AudioCollisionService.c',migration.calls),('AudioManager.h',migration.header)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
header=(out/'before/src/audio/AudioManager.h').read_text();macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=macros
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:
    converted=re.sub(r'\b'+name+r'\b','AUDIO_COLLISION_SOUNDS_'+name,converted)
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('static const uint32_t')])
assert 'v <= 0.0f ? 0.0f : ::Sqrt(v)' in header
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','SurfaceTypes.h'])
template=(root/'utils/tests/audio_collision_sounds.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros)
service=function((root/'build/audio-collision-service-c-tests/before/src/audio/AudioCollision.cpp').read_text(),'cAudioManager::','ServiceCollisions')
methods='\n'.join(function(old,'cAudioManager::',name) for name in migration.NAMES)+'\n'+service
for variant in ['before','after']:
    source=template.replace('@TABLE@',migration.table(old) if variant=='before' else '').replace('@METHODS@',methods if variant=='before' else '')
    entries={'ONESHOT':'active->SetUpOneShotCollisionSound(collision)','LOOPING':'active->SetUpLoopingCollisionSound(collision,(uint8_t)seed)','VOLUME':'active->SetLoopingCollisionRequestedSfxFreqAndGetVol(collision)','SERVICE':'active->ServiceCollisions()'}
    for i,(name,expression) in enumerate(entries.items()):
        source=source.replace('@'+name+'@',expression if variant=='before' else f'TestCollisionSoundsC(active,&collision,(uint8_t)seed,{i})')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
uint32_t TestCollisionSoundsC(cAudioManager *m,const cAudioCollision *c,uint8_t counter,unsigned kind)
{
    // Invoke every collision generator and service operation from an actual C caller
    switch(kind) {
    case 0: AudioCollisionSounds_OneShot(m,c);break;
    case 1: AudioCollisionSounds_Looping(m,c,counter);break;
    case 2: return AudioCollisionSounds_LoopingVolume(m,c);
    case 3: AudioCollisionService_Run(m);break;
    }
    return 0;
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:
    (out/(name+'.h')).write_bytes((root/'build/audio-collision-report-c-tests'/(name+'.h')).read_bytes())
print('Complete generator bodies, surface table, macros and service calls audited')
