# Audit full vehicle dispatch and generate original-body behavior comparisons
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_vehicle_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-vehicle-c-tests'
def normalize(source):
    # Compare every statement independently of comments and formatting
    import fire_migration
    source=fire_migration.calls(source)
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioVehicle.c').read_text();method=function(old,'cAudioManager::','ProcessVehicle');actual=function(kernel,'AudioVehicle_','Process')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(__import__('audio_vehicle_one_shots_migration').logic(migration.HOST)) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.HELPER) in normalize(kernel)
assert normalize(migration.calls((out/'before/src/audio/AudioDispatch.c').read_text()))==normalize((root/'src/audio/AudioDispatch.c').read_text())
vehicle=(out/'before/src/vehicles/Vehicle.h').read_text();appearance=re.search(r'enum eVehicleAppearance\s*\{.*?\n\};',vehicle,re.S).group()
assert normalize(appearance) in normalize((root/'src/modelinfo/VehicleAppearance.h').read_text())
assert normalize(migration.types(vehicle))==normalize((root/'src/vehicles/Vehicle.h').read_text())
for name in ['reVC.vcxproj','reVC.vcxproj.filters']:assert migration.project((out/'before/build'/name).read_text())==(root/'build'/name).read_text()
# Derive velocity arithmetic from the actual shared vector definition
vector=(root/'src/math/Vector.h').read_text();dot=function(vector,'','DotProduct');expected=dot[dot.index('{'):].replace('v1.','speed.').replace('v2.','direction.')
expected=expected.replace('{','{ AudioSoundPosition speed, direction; AudioVehicleHost_VelocityVectors(vehicle, &speed, &direction);',1)
actual=function(kernel,'AudioVehicle_','Velocity');assert normalize(expected)==normalize(actual[actual.index('{'):])
assert 'CVector &GetForward(void) { return m_matrix.GetForward(); }' in (root/'src/core/Placeable.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
names=['AudioVehicleModel_Car','AudioVehicleModel_Heli','AudioAircraft_Heli','AudioVehicleRoad_FlatTyre','AudioVehicleRoad_Dry','AudioVehicleReverse_Process','AudioVehicleRoad_Wet','AudioVehicleSkid_Process','AudioVehicleHorn_Horn','AudioVehicleHorn_Siren','AudioVehicleWarning_Reverse','AudioVehicleWarning_AirBrakes','AudioVehicleCondition_BombTick','AudioVehicleEngine_Process','AudioVehicleCondition_Damage','AudioVehicleDoors_Process','AudioBoat_Engine','AudioBoat_Water','AudioPlane_Plane','AudioVehicleRain_Process']
prototypes=[];generators=[]
for i,name in enumerate(names):
    signature=re.search(r'^(?:uint8_t|void) '+name+r'\(cAudioManager \*manager, cVehicleParams \*params\);',header,re.M).group();prototypes.append('extern "C" '+signature)
    token=i if i<19 else 20
    generators.append(signature[:-1].replace('cVehicleParams *params','cVehicleParams *p')+' {\n    // Observe the selected independently audited generator boundary\n    '+('generator(manager,p,'+str(token)+');' if signature.startswith('void') else 'return generator(manager,p,'+str(token)+');')+'\n}')
for name,token in [('AudioVehicleWarning_UsesReverse',21),('AudioVehicleWarning_HasAirBrakes',22)]:
    signature='uint8_t '+name+'(uint32_t index)';prototypes.append('extern "C" '+signature+';')
    generators.append(signature+' {\n    // Exercise both predicate outcomes at their original routing positions\n    ++layers['+str(token)+'];event('+str(token+40)+');value(index);return (seedValue+index+'+str(token)+')%3!=0;\n}')
prototypes+=['extern "C" '+line for line in migration.PROTOTYPES.strip().split('\n')]
for name in ['AudioGame_FindVehicle','AudioDispatchHost_Area','AudioReleaseHost_Position','AudioVehicleReverseHost_Model','AudioVehicleEngineHost_Status','AudioVehicleRoadHost_WetRoads','AudioVehiclePlayer_UpdateGas']:
    # Retain reused production adapter signatures without recreating their ABI
    source=(root/'src/audio/AudioPoliceGame.h').read_text() if name=='AudioGame_FindVehicle' else header
    prototype=re.search(r'^[^\n{}]+\b'+name+r'\([^\n]*\);',source,re.M).group();prototypes.append('extern "C" '+prototype)
template=(root/'utils/tests/audio_vehicle.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@PROTOTYPES@','\n'.join(prototypes)).replace('@GENERATORS@','\n'.join(generators)).replace('AudioVehicleHost_OneShots','AudioVehicleOneShots_Process')
for marker,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('VEHICLE_TYPES','src/modelinfo/VehicleTypes.h'),('APPEARANCE','src/modelinfo/VehicleAppearance.h'),('ENTITY_STATUS','src/entities/EntityStatus.h'),('GAME_AREAS','src/core/GameAreas.h')]:template=template.replace('@'+marker+'@',str(root/path).replace('\\','/'))
for variant in ['before','after']:
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@RUN@','active->ProcessVehicle(&cars[0])' if variant=='before' else 'TestVehicleC(active,&cars[0])')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestVehicleC(cAudioManager *manager,CVehicle *vehicle)\n{\n    // Invoke the complete dispatcher through its actual C declaration\n    AudioVehicle_Process(manager,vehicle);\n}\n',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','no-timescale','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-plane-c-tests'/(name+'.h')).read_bytes())
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Complete vehicle routing, velocity math, enum, adapters, direct C caller and unchanged owner storage audited')
