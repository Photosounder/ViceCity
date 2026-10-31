"""Audit complete game setup/shutdown and generate original/C runtime comparisons."""
from pathlib import Path
import re
from audio_owner_init_migration import fields
import audio_game_setup_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-game-setup-c-tests'

def normalize(source):
    # Compare every original statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioGameSetup.c').read_text();methods=[]
for name,target in migration.METHODS.items():
    original=migration.function(old,'cAudioManager::',name);actual=migration.function(kernel,'AudioGameSetup_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioLifecycle.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','soundlist.h'])
time_host='extern "C" '+migration.function(old,'AudioPlayerEngineHost_','Time')
template=(root/'utils/tests/audio_game_setup.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@HOST@',migration.HOST).replace('@TIME_HOST@',time_host)
for variant in ['before','after']:
    # Compile complete unchanged originals or call the actual separate C11 operations
    run='switch(operation){case 0:'+''.join('active->'+name+'();' for name in migration.METHODS)+'break;'+''.join('case '+str(i+1)+':active->'+name+'();break;' for i,name in enumerate(migration.METHODS))+'}' if variant=='before' else 'TestGameSetupC(active,(uint8_t)operation)'
    (out/(variant+'.cpp')).write_text(template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@',run),newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestGameSetupC(cAudioManager *manager, uint8_t operation)
{
    // Exercise the complete lifecycle sequence or each original operation through real C declarations
    switch (operation) {
    case 0:
        AudioGameSetup_PreInitialise(manager);
        AudioGameSetup_PostInitialise(manager);
        AudioGameSetup_PreTerminate(manager);
        AudioGameSetup_PostTerminate(manager);
        break;
    case 1: AudioGameSetup_PreInitialise(manager); break;
    case 2: AudioGameSetup_PostInitialise(manager); break;
    case 3: AudioGameSetup_PreTerminate(manager); break;
    case 4: AudioGameSetup_PostTerminate(manager); break;
    }
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
pre_init=methods[0];banks=re.findall(r'BankStartOffset\[(SAMPLEBANK_\w+)\]',pre_init);samples=(root/'src/audio/AudioSamples.h').read_text();missing=sorted({name for name in re.findall(r'= (SFX_\w+);',pre_init) if not re.search(r'\b'+name+r'\b',samples)})
ps2_definitions='\n'.join('#define '+name+' '+str(4+i) for i,name in enumerate(banks))+'\n'+'\n'.join('#define '+name+' '+str(100+i) for i,name in enumerate(missing))+'\n'
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-reflections':'#undef AUDIO_REFLECTIONS\n','bridge':'#define GTA_BRIDGE\n'}.items():
    # Preserve unsupported production PS2 names while supplying explicit test-only bank and sample identities
    (out/(shape+'.h')).write_text('#include "config.h"\n'+flags+'#ifdef GTA_PS2\n'+ps2_definitions+'#endif\n',newline='\n')
(out/'ps2-identities.txt').write_text(ps2_definitions,newline='\n')
print('Four complete setup/shutdown operations, direct lifecycle calls, one adapter, reused timer, conditional bank writes and unchanged owner layout audited')
