# Audit both generators and model predicates and generate actual C comparisons
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_boat_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-boat-c-tests'
def normalize(s):
    # Compare all production statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioBoat.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioBoat_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.CONSTANTS) in normalize(kernel)
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_BOAT_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('static uint8_t AudioBoat_Decision')])
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
assert normalize(migration.HOST) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.HELPER)==normalize(function(kernel,'AudioBoat_','Decision'))
assert '#define AUDIO_BOAT_MIN(a,b) ((a) < (b) ? (a) : (b))' in kernel
assert '#define AUDIO_BOAT_MAX(a,b) (AudioBoat_Decision((a) > (b)) ? (a) : (b))' in kernel
assert 'inline float Abs(float x) { return fabsf(x); }' in (root/'src/math/maths.h').read_text()
assert 'int16 GetModelIndex(void) const { return m_modelIndex; }' in (root/'src/entities/Entity.h').read_text()
template=(root/'utils/tests/audio_boat.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS)
for marker,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('ENTITY_STATUS','src/entities/EntityStatus.h')]:template=template.replace('@'+marker+'@',str(root/path).replace('\\','/'))
for variant in ['before','after']:
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','kind==0?active->ProcessBoatEngine(params):active->ProcessBoatMovingOverWater(params)' if variant=='before' else 'TestBoatC(active,&params,kind)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nuint8_t TestBoatC(cAudioManager *manager,cVehicleParams *params,unsigned kind)\n{\n    // Invoke both complete generators through their actual C declarations\n    if(kind==0)return AudioBoat_Engine(manager,params);\n    return AudioBoat_Water(manager,params);\n}\n',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','no-timescale','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-vehicle-condition-c-tests'/(name+'.h')).read_bytes())
print('Complete boat generators, shared history, adapters, constants, callers and macros audited')
