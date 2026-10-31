# Audit both complete generators, predicates, every pattern bit and shared enum values
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_horn_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-horn-c-tests'
def normalize(s):
    # Compare complete production statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehicleHorn.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioVehicleHorn_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.models(old)) in normalize((root/'src/audio/AudioVehicleModels.h').read_text())
convertedPattern=migration.pattern(old).replace('Const static bool8','static const uint8_t').replace('TRUE','1').replace('FALSE','0')
assert normalize(convertedPattern) in normalize(kernel)
assert normalize(migration.CONSTANTS) in normalize(kernel)
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_HORN_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('static const uint8_t HornPattern')])
assert '#define Const const' in (root/'src/core/common.h').read_text()
assert 'bool IsAlarmOn(void) { return m_nAlarmState != 0 && m_nAlarmState != -1 && GetStatus() != STATUS_WRECKED; }' in (root/'src/vehicles/Vehicle.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_horn.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS)
for name,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('AUDIO_MODELS','src/audio/AudioVehicleModels.h'),('ENTITY_STATUS','src/entities/EntityStatus.h')]:template=template.replace('@'+name+'@',str(root/path).replace('\\','/'))
for variant in ['before','after']:
    run='kind==0?active->ProcessVehicleHorn(params):active->ProcessVehicleSirenOrAlarm(params)' if variant=='before' else 'TestVehicleHornC(active,&params,kind)'
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@PATTERN@',migration.pattern(old) if variant=='before' else '').replace('@RUN@',run)
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
uint8_t TestVehicleHornC(cAudioManager *manager,cVehicleParams *params,unsigned kind)
{
    // Invoke both complete generators through their actual C declarations
    return kind==0?AudioVehicleHorn_Horn(manager,params):AudioVehicleHorn_Siren(manager,params);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-skid-c-tests'/(name+'.h')).read_bytes())
print('Complete horn and siren generators, predicates, all pattern bits, model indices and queue macros audited')
