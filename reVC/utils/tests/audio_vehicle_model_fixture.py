# Audit both complete RC generators and compare actual C math and requested queues
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_model_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-model-c-tests'
def normalize(s):
    # Compare all production statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioVehicleModel.c').read_text()
for name,target in migration.NAMES.items():
    method=function(old,'cAudioManager::',name);actual=function(kernel,'AudioVehicleModel_',target)
    assert normalize(migration.body(method))==normalize(actual[actual.index('{'):]),name
for name,transform in [('AudioLogic.cpp',migration.logic),('AudioManager.h',migration.header)]:assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
header=(out/'before/src/audio/AudioManager.h').read_text();macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_MODEL_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioVehicleModel_Heli')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
for name in ['Decision','UpDot']:
    assert normalize(function(migration.HELPERS,'AudioVehicleModel_',name))==normalize(function(kernel,'AudioVehicleModel_',name))
common=(root/'src/core/common.h').read_text()
assert '#define Max(a,b) ((a) > (b) ? (a) : (b))' in common and '#define Min(a,b) ((a) < (b) ? (a) : (b))' in common
assert '#define Clamp2(v, center, radius) ((v) > (center) ? Min(v, center + radius) : Max(v, center - radius))' in common
assert 'return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;' in (root/'src/math/Vector.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_model.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS).replace('@MODEL_IDS@',str(root/'src/modelinfo/ModelIds.h').replace('\\','/'))
methods='\n'.join(function(old,'cAudioManager::',name) for name in migration.NAMES)
for variant in ['before','after']:
    source=template.replace('@METHODS@',methods if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@RUN@','if(kind==0) active->ProcessModelHeliVehicle(params);else active->ProcessModelVehicle(params);' if variant=='before' else 'TestVehicleModelC(active,&params,kind);')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestVehicleModelC(cAudioManager *manager,cVehicleParams *params,unsigned kind)
{
    // Invoke both RC model sound generators through their actual C declarations
    if(kind==0) AudioVehicleModel_Heli(manager,params);
    else AudioVehicleModel_Car(manager,params);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-reverse-c-tests'/(name+'.h')).read_bytes())
(out/'alternate.h').write_text('#include "config.h"\n#define AUDIO_OAL\n#define THIS_IS_STUPID\n',newline='\n')
print('Complete RC model generators, static history, adapters, constants and macros audited')
