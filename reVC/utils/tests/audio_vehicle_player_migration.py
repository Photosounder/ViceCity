# Preserve gas smoothing and complete player-car sample request construction
import re
import audio_player_engine_migration as engine
from audio_controls_migration import function
NAMES={'UpdateGasPedalAudio':'UpdateGas','AddPlayerCarSample':'AddSample'}
PROTOTYPES='void AudioVehiclePlayer_UpdateGas(CVehicle *vehicle, int32_t type);\nvoid AudioVehiclePlayer_AddSample(cAudioManager *manager, uint8_t volume, uint32_t frequency, uint32_t sample, uint8_t bank, uint8_t counter, uint8_t looping);\n'
def logic(source):
    # Remove both full member bodies and update every actual internal caller
    for name in NAMES:
        if 'cAudioManager::'+name+'(' in source:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    source=source.replace('UpdateGasPedalAudio(veh, params.m_VehicleType)','AudioVehiclePlayer_UpdateGas(veh, params.m_VehicleType)')
    return engine.logic(re.sub(r'\bAddPlayerCarSample\(', 'AudioVehiclePlayer_AddSample(this, ',source))
def header(source):
    # Retain all stored state while exposing the actual C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    return engine.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))
def body(source):
    # Keep the original arithmetic, loop conditions and live sample-offset arguments
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('veh->m_fGasPedal','AudioVehicleReverseHost_Gas(veh)').replace('&((CAutomobile *)veh)->m_fGasPedalAudio','AudioVehicleReverseHost_GasAudio(veh)').replace('&((CBike *)veh)->m_fGasPedalAudio','AudioVehicleEngineHost_BikeGasAudio(veh)')
    for a,b in [('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('Abs(', 'fabsf(').replace('Min(', 'AudioVehiclePlayer_Min(').replace('Max(', 'AudioVehiclePlayer_Max(').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_VEHICLE_PLAYER_'+name,source)
    return source
