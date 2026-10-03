"""Audit audio lifecycle and compare actual C orchestration with saved owner bodies."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_lifecycle_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-lifecycle-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text();kernel=(root/'src/audio/AudioLifecycle.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
initialise=migration.initialise(old);terminate=function(old,'cAudioManager::','Terminate')
start=kernel.index('void\nAudioLifecycle_Initialise(');end=kernel.index('\nvoid\nAudioLifecycle_Terminate(',start)
actual=kernel[start:end].rstrip('\n')
assert normalize(migration.body(initialise))==normalize(actual[actual.index('{'):])
actual=function(kernel,'AudioLifecycle_','Terminate');assert normalize(migration.body(terminate))==normalize(actual[actual.index('{'):])
for name,transform in [('AudioManager.cpp',migration.owner),('AudioManager.h',migration.header),('DMAudio.cpp',migration.facade)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
facades=(out/'before/src/audio/DMAudio.cpp').read_text()
for name in ['Initialise','Terminate']:
    expected=function(facades,'DMAudio_',name).replace('AudioManager.'+name+'()', 'AudioLifecycle_'+name+'(&AudioManager)')
    assert normalize(expected)==normalize(function(kernel,'DMAudio_',name))
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','AudioPoliceState.h','sampman.h'])
template=(root/'utils/tests/audio_lifecycle.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ['before','after']:
    source=template.replace('@METHODS@',initialise+'\n'+terminate if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '')
    source=source.replace('@INITIALISE@','active->Initialise()' if variant=='before' else 'AudioLifecycle_Initialise(active)').replace('@TERMINATE@','active->Terminate()' if variant=='before' else 'AudioLifecycle_Terminate(active)')
    source=source.replace('@FACADE@','AudioManager.Initialise(); AudioManager.Terminate();' if variant=='before' else 'TestLifecycleC();')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "DMAudio.h"\nvoid TestLifecycleC(void)\n{\n    // Exercise both C facades against the live global owner\n    DMAudio_Initialise();\n    DMAudio_Terminate();\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Both lifecycle bodies, channel narrowing, destructor binding, facades and callback edits audited')
