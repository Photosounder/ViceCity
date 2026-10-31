# Compare the complete pedestrian event loop with its actual original C++ body
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_ped_one_shots_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-ped-one-shots-c-tests'
def normalize(source):
    # Ignore only comments and whitespace when comparing complete statements
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioPedOneShots.c').read_text()
original=function(old,'cAudioManager::','ProcessPedOneShots');actual=function(kernel,'AudioPedOneShots_','Process')
assert normalize(migration.body(original))==normalize(actual[actual.index('{'):])
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioPed.c',migration.calls),('src/peds/Ped.h',migration.types),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(migration.constants(old)) in normalize(kernel)
for name in migration.ENUMS:assert normalize(migration.enumeration((out/'before/src/peds/Ped.h').read_text(),name))==normalize(migration.enumeration((root/'src/peds/PedStates.h').read_text(),name))
for line in re.findall(r'static (?:uint8|uint32) \w+ = \d+;',original):assert normalize(line.replace('uint8','uint8_t').replace('uint32','uint32_t')) in normalize(kernel)
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_PED_ONE_SHOTS_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioPedOneShots_Process')])
events=list(dict.fromkeys(re.findall(r'case (SOUND_\w+):',original)));events.append('SOUND_BOAT_SLOWDOWN')
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_ped_one_shots.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.constants(old)).replace('@EVENTS@','static const int events[]={'+','.join(events)+'};')
for marker,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('WEAPON_TYPES','src/weapons/WeaponType.h'),('PED_STATES','src/peds/PedStates.h'),('SURFACES','src/core/SurfaceTypes.h'),('ENTITY_TYPES','src/entities/EntityTypes.h')]:template=template.replace('@'+marker+'@',str(root/path).replace('\\','/'))
host=''
for result,name,parameter,expression,comment in migration.QUERIES:host+='extern "C" '+result+' AudioPedOneShotsHost_'+name+'('+parameter+') {\n    // '+comment+'\n    return '+expression+';\n}\n'
template=template.replace('@HOST@',host)
for variant in ['before','after']:
    # Compile the untouched original body or invoke the separately compiled C entry point
    (out/(variant+'.cpp')).write_text(template.replace('@METHOD@',original if variant=='before' else '').replace('@RUN@','active->ProcessPedOneShots(params)' if variant=='before' else 'TestPedOneShotsC(active,&params)'),newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestPedOneShotsC(cAudioManager *manager,cPedParams *params)\n{\n    // Invoke the complete event loop through its actual C declaration\n    AudioPedOneShots_Process(manager,params);\n}\n',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Complete pedestrian event loop, two histories, constants, macros, enum values, adapters and '+str(len(events)-1)+' event labels audited')
