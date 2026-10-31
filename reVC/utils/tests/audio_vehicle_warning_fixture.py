# Audit both generators and model predicates and generate actual C comparisons
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_warning_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-warning-c-tests'
def normalize(s):
    # Compare all production statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehicleWarning.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioVehicleWarning_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.CONSTANTS) in normalize(kernel)
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_WARNING_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('uint8_t\nAudioVehicleWarning_UsesReverse')])
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_vehicle_warning.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS).replace('@AUDIO_MODELS@',str(root/'src/audio/AudioVehicleModels.h').replace('\\','/'))
for variant in ['before','after']:
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@USES@','active->UsesReverseWarning(model)' if variant=='before' else 'TestVehicleWarningC(active,&params,2,model)').replace('@HAS@','active->HasAirBrakes(model)' if variant=='before' else 'TestVehicleWarningC(active,&params,3,model)').replace('@RUN@','kind==0?active->ProcessVehicleReverseWarning(params):active->ProcessAirBrakes(params)' if variant=='before' else 'TestVehicleWarningC(active,&params,kind,0)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
uint8_t TestVehicleWarningC(cAudioManager *manager,cVehicleParams *params,unsigned kind,uint32_t model)
{
    // Invoke both generators and predicates through their actual C declarations
    if(kind==0)return AudioVehicleWarning_Reverse(manager,params);
    if(kind==1)return AudioVehicleWarning_AirBrakes(manager,params);
    if(kind==2)return AudioVehicleWarning_UsesReverse(model);
    return AudioVehicleWarning_HasAirBrakes(model);
}
''',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-horn-c-tests'/(name+'.h')).read_bytes())
print('Complete reverse-warning and air-brake generators, predicates, constants, callers and macros audited')
