# Move plane routing and the complete Jumbo sound family into C11
import re
from audio_controls_migration import function
NAMES={name:name.removeprefix('Process').removeprefix('Setup').removeprefix('Get') for name in ['ProcessPlane','ProcessJumbo','ProcessJumboTaxi','ProcessJumboAccel','ProcessJumboTakeOff','ProcessJumboFlying','ProcessJumboLanding','ProcessJumboDecel','SetupJumboTaxiSound','SetupJumboWhineSound','SetupJumboEngineSound','SetupJumboFlySound','SetupJumboRumbleSound','GetJumboTaxiFreq']}
CONSTANTS='\tJUMBO_MAX_DIST = 440,\n\tJUMBO_RUMBLE_SOUND_MAX_DIST = 240,\n\tJUMBO_ENGINE_SOUND_MAX_DIST = 180,\n\tJUMBO_WHINE_SOUND_MAX_DIST = 170,\n'
HOST='//+ rouz edit (ChatGPT)\nextern "C" float\nAudioPlaneHost_Speed(const CVehicle *vehicle)\n{\n\t// Read the original captured plane speed\n\treturn ((const CPlane *)vehicle)->m_fSpeed;\n}\nextern "C" float\nAudioPlaneHost_Position(const CVehicle *vehicle)\n{\n\t// Read the live path position through the original plane ID\n\treturn PlanePathPosition[((const CPlane *)vehicle)->m_nPlaneId];\n}\nextern "C" float\nAudioPlaneHost_TakeOffPoint(void)\n{\n\t// Read the original live takeoff threshold\n\treturn TakeOffPoint;\n}\nextern "C" float\nAudioPlaneHost_LandingPoint(void)\n{\n\t// Read the original live landing threshold\n\treturn LandingPoint;\n}\n//- rouz edit (ChatGPT)\n'
SIGNATURES={}
for name,target in NAMES.items():
    result='uint8_t' if name.startswith('Setup') else 'int32_t' if name.startswith('Get') else 'void'
    args='cAudioManager *manager'
    if name in ['ProcessPlane','ProcessJumbo']:args+=', cVehicleParams *params'
    elif name in ['ProcessJumboAccel','ProcessJumboTakeOff','ProcessJumboLanding','ProcessJumboDecel']:args+=', CVehicle *plane'
    elif name.startswith('Setup'):args+=', uint8_t '+('vol' if name=='SetupJumboTaxiSound' else 'Vol')+(', uint32_t freq' if name in ['SetupJumboWhineSound','SetupJumboEngineSound'] else '')
    SIGNATURES[name]=result+' AudioPlane_'+target+'('+args+')'
PROTOTYPES='\n'.join(SIGNATURES.values())+';\n'
PROTOTYPES=PROTOTYPES.replace(')\n',');\n')
PROTOTYPES+='float AudioPlaneHost_Speed(const CVehicle *vehicle);\nfloat AudioPlaneHost_Position(const CVehicle *vehicle);\nfloat AudioPlaneHost_TakeOffPoint(void);\nfloat AudioPlaneHost_LandingPoint(void);\n'

def logic(source):
    # Remove the complete family while leaving partial older audit inputs unchanged
    if 'cAudioManager::ProcessPlane(' in source:
        for name in NAMES:source=source.replace(function(source,'cAudioManager::',name)+'\n','')
        source=source.replace('uint8 gJumboVolOffsetPercentage;\n','').replace(function(source,'','DoJumboVolOffset')+'\n','')
        source+=HOST
    for line in CONSTANTS.splitlines():source=source.replace(line+'\n','')
    if HOST in source:source=source.replace(HOST,'')+HOST
    import audio_vehicle_migration
    return audio_vehicle_migration.logic(re.sub(r'(?<![\w])ProcessPlane\(params\)', 'AudioPlane_Plane(this, &params)',source))

def header(source):
    # Replace member declarations without changing owner storage
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in NAMES))
    import audio_vehicle_migration
    return audio_vehicle_migration.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Preserve every original branch, partial write and callback position
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source).replace('params.','params->')
    source=source.replace('CPlane *plane','CVehicle *plane').replace('(CPlane*)params->m_pVehicle','params->m_pVehicle')
    source=source.replace('PlanePathPosition[plane->m_nPlaneId]','AudioPlaneHost_Position(plane)').replace('plane->m_fSpeed','AudioPlaneHost_Speed(plane)')
    source=re.sub(r'\bTakeOffPoint\b','AudioPlaneHost_TakeOffPoint()',source)
    source=re.sub(r'\bLandingPoint\b','AudioPlaneHost_LandingPoint()',source)
    for name,target in NAMES.items():
        source=re.sub(r'\b'+name+r'\(', 'AudioPlane_'+target+'(manager, ',source)
        source=source.replace('AudioPlane_'+target+'(manager, )','AudioPlane_'+target+'(manager)')
    source=source.replace('AudioAircraft_Cesna(this, &params)','AudioAircraft_Cesna(manager, params)').replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)').replace('DoJumboVolOffset()','AudioPlane_Offset()')
    for a,b in [('uint32','uint32_t'),('uint8','uint8_t'),('int32','int32_t'),('bool8','uint8_t'),('TRUE','1'),('FALSE','0'),('Min','AUDIO_PLANE_MIN')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('SQR(JUMBO_MAX_DIST)','(JUMBO_MAX_DIST * JUMBO_MAX_DIST)')
    for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:source=re.sub(r'\b'+name+r'\b','AUDIO_PLANE_'+name,source)
    return source
