"""Move complete car-radio selection and tuning operations into C11."""
import re
import audio_music_stream_migration as stream
from audio_controls_migration import function

METHODS=['PlayerInCar','GetRadioInCar','SetRadioInCar','GetNextCarTuning','GetCarTuning','UsesPoliceRadio','UsesTaxiRadio']
PROTOTYPES='''uint8_t MusicRadio_PlayerInCar(cMusicManager *manager);
uint32_t MusicRadio_GetRadioInCar(cMusicManager *manager);
void MusicRadio_SetRadioInCar(cMusicManager *manager, uint32_t station);
uint32_t MusicRadio_GetNextCarTuning(cMusicManager *manager);
uint32_t MusicRadio_GetCarTuning(cMusicManager *manager);
uint8_t MusicRadio_UsesPoliceRadio(CVehicle *vehicle);
uint8_t MusicRadio_UsesTaxiRadio(CVehicle *vehicle);
uint8_t *MusicRadioHost_Station(CVehicle *vehicle);
uint8_t MusicRadioHost_UsesSiren(CVehicle *vehicle);
uint8_t MusicRadioHost_HasMetDebbie(void);
'''
HOST='''//+ rouz edit (ChatGPT)
extern "C" uint8_t *
MusicRadioHost_Station(CVehicle *vehicle)
{
    // Expose the original byte station for live reads and narrowed compound assignments
    return &vehicle->m_nRadioStation;
}
extern "C" uint8_t
MusicRadioHost_UsesSiren(CVehicle *vehicle)
{
    // Query the existing vehicle siren rule after the explicit model exceptions
    return vehicle->UsesSiren();
}
extern "C" uint8_t
MusicRadioHost_HasMetDebbie(void)
{
    // Read the existing script flag only for the Kaufman taxi model
    return CTheScripts::bPlayerHasMetDebbieHarry;
}
//- rouz edit (ChatGPT)
'''

def sample_host(source):
    # Extract the full existing C-linkage wrapper without leaving its declaration behind
    start=source.index('extern "C" int AudioSample_GetRadioInCar(void)')
    end=source.index('{',start)+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]

def calls(source):
    # Route each caller with its original owner or captured vehicle argument
    if 'extern "C" int AudioSample_GetRadioInCar(void)' in source:source=source.replace(sample_host(source)+'\n','')
    for name in METHODS:
        replacement='MusicRadio_'+name+'('
        if name not in ['UsesPoliceRadio','UsesTaxiRadio']:replacement+='&MusicManager'+(', ' if name=='SetRadioInCar' else '')
        source=source.replace('MusicManager.'+name+'(',replacement)
    if 'cMusicManager::cMusicManager(' in source:
        for name in METHODS:
            replacement='MusicRadio_'+name+'('
            if name not in ['UsesPoliceRadio','UsesTaxiRadio']:replacement+='this'+(', ' if name=='SetRadioInCar' else '')
            source=re.sub(r'(?<![\w.:>])'+name+r'\(',replacement,source)
    return stream.calls(source)

def logic(source):
    # Remove complete converted members and both public facades before adapting remaining callers
    owner='cMusicManager::cMusicManager(' in source
    for name in METHODS:
        if 'cMusicManager::'+name+'(' in source:source=source.replace(function(source,'cMusicManager::',name)+'\n','')
    for name in ['GetRadioInCar','SetRadioInCar']:
        if re.search(r'(?m)^DMAudio_'+name+r'\(',source):source=source.replace(function(source,'DMAudio_',name)+'\n','')
    if 'extern "C" int AudioSample_GetRadioInCar(void)' in source:source=source.replace(sample_host(source)+'\n','')
    source=calls(source)
    if owner and 'MusicRadioHost_Station(CVehicle *vehicle)' not in source:source+='\n'+HOST
    if HOST in source:
        # Keep the new readers after adapters inserted by preceding migration stages
        source=source.replace(HOST,'')+HOST
    return stream.logic(source)

def header(source):
    # Preserve all fields while replacing member declarations with typed C operations
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+n+r'\(',line) for n in METHODS))
    source=source.replace('class CPed;\n#endif','class CPed;\n#else\ntypedef struct CVehicle CVehicle;\n#endif',1)
    return stream.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Preserve repeated vehicle queries, short circuit gates, byte narrowing and signed remainders
    source=source[source.index('{'):]
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source)
    for name in METHODS:
        replacement='MusicRadio_'+name+'('
        if name not in ['UsesPoliceRadio','UsesTaxiRadio']:replacement+='manager'+(', ' if name=='SetRadioInCar' else '')
        source=re.sub(r'(?<![\w.:>])'+name+r'\(',replacement,source)
    source=source.replace('FindPlayerPed()->m_nPedState','AudioPedTalkHost_PlayerState()').replace('vehicle->GetStatus()','AudioVehicleEngineHost_Status(vehicle)')
    source=source.replace('veh->GetModelIndex()','AudioVehicleModelHost_Model(veh)').replace('veh->UsesSiren()','MusicRadioHost_UsesSiren(veh)')
    source=source.replace('CReplay::IsPlayingBack()','AudioEnvironmentHost_Replay()').replace('CTheScripts::bPlayerHasMetDebbieHarry','MusicRadioHost_HasMetDebbie()')
    source=source.replace('veh->m_nRadioStation','(*MusicRadioHost_Station(veh))')
    for a,b in [('uint32','uint32_t'),('int32','int32_t'),('FALSE','0'),('TRUE','1'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    return source

def project(source):
    # Compile all seven operations and existing facades as C11
    if 'MusicRadio.c' in source:return stream.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicModes.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return stream.project(source[:b]+'\n'+source[a:b].replace('MusicModes.c','MusicRadio.c')+source[b:])
