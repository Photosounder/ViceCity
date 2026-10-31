# Audit the complete plane family and generate comparisons from its actual original bodies
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_plane_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-plane-c-tests'
def normalize(source):
    # Compare all statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioPlane.c').read_text();methods=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioPlane_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.CONSTANTS) in normalize(kernel)
assert normalize(migration.HOST) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
original_offset=function(old,'','DoJumboVolOffset');actual_offset=function(kernel,'AudioPlane_','Offset')
assert normalize(original_offset[original_offset.index('{'):])==normalize(actual_offset[actual_offset.index('{'):])
assert 'static uint8_t gJumboVolOffsetPercentage;' in kernel
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_PLANE_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('static uint8_t gJumboVolOffsetPercentage')])
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
declarations=[];runs=[];c_runs=[]
for name,target in migration.NAMES.items():
    original=function(old,'cAudioManager::',name);signature=original[:original.index('{')].strip().replace('cAudioManager::','');declarations.append(signature+';')
    args='params' if name in ['ProcessPlane','ProcessJumbo'] else '(CPlane *)params.m_pVehicle' if name in ['ProcessJumboAccel','ProcessJumboTakeOff','ProcessJumboLanding','ProcessJumboDecel'] else '(uint8_t)(seedValue%128), 22050+(seedValue%10001)' if name in ['SetupJumboWhineSound','SetupJumboEngineSound'] else '(uint8_t)(seedValue%128)' if name.startswith('Setup') else ''
    runs.append('active->'+name+'('+args+')' if name.startswith(('Setup','Get')) else '(active->'+name+'('+args+'),0)')
    c_args='params' if name in ['ProcessPlane','ProcessJumbo'] else 'params->m_pVehicle' if name in ['ProcessJumboAccel','ProcessJumboTakeOff','ProcessJumboLanding','ProcessJumboDecel'] else '(uint8_t)(seed%128), 22050+(seed%10001)' if name in ['SetupJumboWhineSound','SetupJumboEngineSound'] else '(uint8_t)(seed%128)' if name.startswith('Setup') else ''
    c_call='AudioPlane_'+target+'(manager'+(', '+c_args if c_args else '')+')'
    c_runs.append('case '+str(len(c_runs))+': '+('return '+c_call+';' if name.startswith(('Setup','Get')) else c_call+'; return 0;'))
run=' : '.join('kind=='+str(i)+' ? '+expr for i,expr in enumerate(runs[:-1]))+' : '+runs[-1]
# Bind helper inputs explicitly through the C caller rather than duplicating production logic
(out/'caller.c').write_text('#include "AudioManager.h"\nint32_t TestPlaneC(cAudioManager *manager,cVehicleParams *params,unsigned kind,unsigned seed)\n{\n    // Invoke every original entry point through actual C declarations\n    switch(kind){\n'+ '\n'.join(c_runs)+'\n    default: return -1;\n    }\n}\n',newline='\n')
template=(root/'utils/tests/audio_plane.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS).replace('@DECLARATIONS@','\n'.join(declarations)).replace('TestPlaneC(cAudioManager *,cVehicleParams *,unsigned);','TestPlaneC(cAudioManager *,cVehicleParams *,unsigned,unsigned);')
methods.insert(0,original_offset)
for variant in ['before','after']:
    source=template.replace('@METHODS@','uint8 gJumboVolOffsetPercentage;\n'+'\n'.join(methods) if variant=='before' else '').replace('@RUN@',run if variant=='before' else 'TestPlaneC(active,&params,kind,seedValue)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','no-timescale','single','single-c','wav']:(out/(name+'.h')).write_bytes((root/'build/audio-aircraft-c-tests'/(name+'.h')).read_bytes())
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Complete plane/Jumbo bodies, global offset, macros, constants, declarations, adapters and callers audited')

for name in ['reVC.vcxproj','reVC.vcxproj.filters']:
    # Verify that adding the C compilation item is the only project change
    before=(out/'before/build'/name).read_text();actual=(root/'build'/name).read_text()
    a=before.index('    <ClCompile Include="..\\src\\audio\\AudioAircraft.c"');b=before.index('</ClCompile>',a)+len('</ClCompile>')
    block=before[a:b].replace('AudioAircraft.c','AudioPlane.c')
    assert actual==__import__('audio_vehicle_migration').project(before[:b]+'\n'+block+before[b:])
