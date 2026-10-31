# Audit and compare the complete pedestrian dispatcher against its original C++ body
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_ped_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-ped-c-tests'
def normalize(source):
    # Ignore only comments and whitespace in complete statement comparisons
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioPed.c').read_text()
original=function(old,'cAudioManager::','ProcessPed');actual=function(kernel,'AudioPed_','Process')
assert normalize(migration.body(original))==normalize(actual[actual.index('{'):])
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioDispatch.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert 'AudioDispatchHost_Ped(' not in (root/'src/audio/AudioLogic.cpp').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_ped.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header))
for variant in ['before','after']:
    # Compile the unmodified original method or invoke the separately compiled C implementation
    source=template.replace('@METHODS@',original if variant=='before' else '').replace('@RUN@','active->ProcessPed(&peds[seed%2])' if variant=='before' else 'TestPedC(active,&peds[seed%2])')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestPedC(cAudioManager *manager, CPed *ped)\n{\n    // Exercise the public pedestrian entry point through its C declaration\n    AudioPed_Process(manager,ped);\n}\n',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Complete pedestrian dispatcher, direct physical caller, declarations, bridge and project registration audited')
