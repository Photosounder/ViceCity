# Move both complete boat generators while preserving global history and repeated throttle reads
import re
import audio_aircraft_migration as aircraft
from audio_controls_migration import function
NAMES={'ProcessBoatEngine':'Engine','ProcessBoatMovingOverWater':'Water'}
CONSTANTS='\tBOAT_ENGINE_MAX_DIST = 90,\n\tBOAT_ENGINE_REEFER_IDLE_VOLUME = 80,\n\tBOAT_MOVING_OVER_WATER_MAX_DIST = 50,\n\tBOAT_MOVING_OVER_WATER_VOLUME = 30,\n'
PROTOTYPES='uint8_t AudioBoat_Engine(cAudioManager *manager, cVehicleParams *params);\nuint8_t AudioBoat_Water(cAudioManager *manager, cVehicleParams *params);\nuint8_t AudioBoatHost_Propeller(const CVehicle *vehicle);\nuint8_t AudioBoatHost_InWater(const CVehicle *vehicle);\n'
HOST='//+ rouz edit (ChatGPT)\nextern "C" uint8_t\nAudioBoatHost_Propeller(const CVehicle *vehicle)\n{\n\t// Read the captured boat propeller-in-water bit\n\treturn ((const CBoat *)vehicle)->bPropellerInWater;\n}\nextern "C" uint8_t\nAudioBoatHost_InWater(const CVehicle *vehicle)\n{\n\t// Read the live boat-in-water bit after the velocity threshold\n\treturn ((const CBoat *)vehicle)->bBoatInWater;\n}\n//- rouz edit (ChatGPT)\n'
HELPER='static uint8_t AudioBoat_Decision(uint8_t result)\n{\n\t// Preserve conditional repeated throttle queries across compiler optimizations\n\tvolatile uint8_t comparison = result;\n\treturn comparison;\n}\n'
def logic(source):
    # Remove both complete members and their private constants before changing internal calls
    if 'cAudioManager::ProcessBoatEngine(' in source:
        for name in NAMES:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source+=HOST
    for line in CONSTANTS.splitlines():source=source.replace(line+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    for name,target in NAMES.items():source=source.replace(name+'(params)','AudioBoat_'+target+'(this, &params)')
    return aircraft.logic(source)

def header(source):
    # Replace only the two owning member declarations with C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return aircraft.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve captured boat state, live throttle inputs, static history and queue mutations
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('CBoat *boat','CVehicle *boat').replace('(CBoat *)params->m_pVehicle','params->m_pVehicle')
    for a,b in [('boat->GetStatus()','AudioVehicleEngineHost_Status(boat)'),('boat->GetModelIndex()','AudioVehicleReverseHost_Model(boat)'),('FindPlayerVehicle()','AudioPoliceHost_PlayerVehicle()'),('Pads[0].GetAccelerate()','AudioVehicleModelHost_Accelerate()'),('Pads[0].GetBrake()','AudioVehicleModelHost_Brake()'),('params->m_pVehicle->m_fGasPedal','AudioVehicleReverseHost_Gas(params->m_pVehicle)'),('params->m_pVehicle->m_fBrakePedal','AudioVehicleModelHost_BrakePedal(params->m_pVehicle)'),('boat->bPropellerInWater','AudioBoatHost_Propeller(boat)'),('((CBoat*)params->m_pVehicle)->bBoatInWater','AudioBoatHost_InWater(params->m_pVehicle)')]:source=source.replace(a,b)
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0'),('Min','AUDIO_BOAT_MIN'),('Max','AUDIO_BOAT_MAX'),('Abs','fabsf')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for name in ['BOAT_ENGINE_MAX_DIST','BOAT_MOVING_OVER_WATER_MAX_DIST']:source=source.replace('SQR('+name+')','('+name+' * '+name+')')
    source=source.replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_BOAT_'+name,source)
    return source
