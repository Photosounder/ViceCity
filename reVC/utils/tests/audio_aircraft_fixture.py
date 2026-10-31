# Audit both generators and model predicates and generate actual C comparisons
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_aircraft_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-aircraft-c-tests'
def normalize(s):
    # Compare all production statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioAircraft.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioAircraft_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.CONSTANTS) in normalize(kernel)
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_AIRCRAFT_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('static float AudioAircraft_UpDot')])
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
assert normalize(migration.HOST) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.HELPERS) in normalize(kernel)
assert 'v1.x*v2.x + v1.y*v2.y + v1.z*v2.z' in (root/'src/math/Vector.h').read_text()
assert 'mat.rx * vec.x + mat.fx * vec.y + mat.ux * vec.z + mat.px' in (root/'src/math/Vector.cpp').read_text()
assert 'CVector(void) {}' in (root/'src/math/Vector.h').read_text()
matrix_source=(root/'src/math/Matrix.cpp').read_text()
assert 'm_attachment = nil;' in matrix_source and 'm_hasRwMatrix = false;' in matrix_source
assert 'memcpy(this, &rhs, sizeof(f));' in matrix_source and 'if (m_attachment)\n\t\tUpdateRW();' in matrix_source
assert 'if (m_hasRwMatrix && m_attachment)\n\t\tRwMatrixDestroy(m_attachment);' in matrix_source
for macro in ['MIN','MAX','ABS','CLAMP']:assert '#define AUDIO_AIRCRAFT_'+macro+'(' in kernel
vector=(root/'src/math/Vector.h').read_text();dot=function(vector,'','DotProduct');dot_body=dot[dot.index('{'):]
up=dot_body
for axis,target in [('x','ux'),('y','uy'),('z','uz')]:up=up.replace('v1.'+axis,'matrix->'+target).replace('v2.'+axis, '1.0f' if axis=='y' else '0.0f')
assert normalize(up)==normalize(function(kernel,'AudioAircraft_','UpDot')[function(kernel,'AudioAircraft_','UpDot').index('{'):])
forward=dot_body
for axis,target in [('x','fx'),('y','fy'),('z','fz')]:forward=forward.replace('v1.'+axis,'forward.'+axis).replace('v2.'+axis,'matrix->'+target)
forward=forward.replace('{','{ AudioSoundPosition forward; AudioAircraftHost_Forward(vehicle, &forward);',1)
assert normalize(forward)==normalize(function(kernel,'AudioAircraft_','ForwardDot')[function(kernel,'AudioAircraft_','ForwardDot').index('{'):])
product=function((root/'src/math/Vector.cpp').read_text(),'','operator\*');terms=product[product.index('return CVector(')+len('return CVector('):product.rindex(');')].split(',')
for axis,term in zip('xyz',terms):
    expected=term.replace('mat.','matrix.').replace('vec.x','0.0f').replace('vec.y','-10.0f').replace('vec.z','0.0f')
    assert normalize('result.'+axis+' = '+expected+';') in normalize(function(kernel,'AudioAircraft_','Rear'))

template=(root/'utils/tests/audio_aircraft.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS)
for marker,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('VEHICLE_TYPES','src/modelinfo/VehicleTypes.h')]:template=template.replace('@'+marker+'@',str(root/path).replace('\\','/'))
for variant in ['before','after']:
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','kind==0?active->ProcessCarHeli(params):(active->ProcessCesna(params),0)' if variant=='before' else 'TestAircraftC(active,&params,kind)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nuint8_t TestAircraftC(cAudioManager *manager,cVehicleParams *params,unsigned kind)\n{\n    // Invoke both complete generators through their actual C declarations\n    if(kind==0)return AudioAircraft_Heli(manager,params);\n    AudioAircraft_Cesna(manager,params);\n    return 0;\n}\n',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','no-timescale','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-boat-c-tests'/(name+'.h')).read_bytes())
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Complete aircraft generators, camera/vector math, history, adapters, constants, callers and macros audited')
