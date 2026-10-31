"""Audit the complete bridge conversion and generate original/C runtime comparisons."""
from pathlib import Path
import re
from audio_owner_init_migration import fields
import audio_bridge_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-bridge-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioBridge.c').read_text();methods=[]
for name,target in migration.METHODS.items():
    original=migration.function(old,'cAudioManager::',name);actual=migration.function(kernel,'AudioBridge_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioDispatch.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project),('src/control/Bridge.h',migration.bridge_header)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(migration.enumeration((out/'before/src/control/Bridge.h').read_text()))==normalize(migration.enumeration((root/'src/control/BridgeStates.h').read_text()))
a=header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS');b=header.index('#if defined(AUDIO_MSS)',a);macros=header[a:b];converted=macros.replace('m_sQueueSample','manager->m_sQueueSample')
for name in migration.MACRO_NAMES:converted=re.sub(r'\b'+name+r'\b','AUDIO_BRIDGE_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioBridge_Process')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
assert '#ifdef GTA_BRIDGE\nenum {' in kernel and kernel.rstrip().endswith('#endif\n//- rouz edit (ChatGPT)')
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','soundlist.h','../control/BridgeStates.h'])
template=(root/'utils/tests/audio_bridge.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@HOST@',migration.HOST).replace('@CONSTANTS@',migration.CONSTANTS)
for variant in ['before','after']:
    # Compare complete unchanged originals with separately compiled C11 operations
    run='\n#ifdef GTA_BRIDGE\n    switch(operation){'+''.join('case '+str(i)+':active->'+name+'();break;' for i,name in enumerate(migration.METHODS))+'}\n#endif\n' if variant=='before' else 'TestBridgeC(active,(uint8_t)operation)'
    (out/(variant+'.cpp')).write_text(template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@',run),newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestBridgeC(cAudioManager *manager, uint8_t operation)
{
    // Exercise all original operations through C declarations only when the bridge feature exists
#ifdef GTA_BRIDGE
    switch (operation) {
    case 0: AudioBridge_Process(manager); break;
    case 1: AudioBridge_Warning(manager); break;
    case 2: AudioBridge_Motor(manager); break;
    case 3: AudioBridge_OneShots(manager); break;
    }
#else
    (void)manager;
    (void)operation;
#endif
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'#define GTA_BRIDGE\n','vanilla':'#define GTA_BRIDGE\n#undef FIX_BUGS\n','no-reverb':'#define GTA_BRIDGE\n#undef AUDIO_REVERB\n','no-external':'#define GTA_BRIDGE\n#undef EXTERNAL_3D_SOUND\n','ps2':'#define GTA_BRIDGE\n#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#define GTA_BRIDGE\n#undef USE_TIME_SCALE_FOR_AUDIO\n','no-reflections':'#define GTA_BRIDGE\n#undef AUDIO_REFLECTIONS\n','disabled':'#undef GTA_BRIDGE\n'}.items():
    # Supply an explicit fixture-only identity for the sample absent from the original VC project
    (out/(shape+'.h')).write_text('#include "config.h"\n'+flags+'#ifdef GTA_BRIDGE\n#define SFX_BRIDGE_OPEN_WARNING SFX_REVERSE_WARNING\n#endif\n',newline='\n')
print('Complete bridge state machine and three generators, four live adapters, shared states and optional feature gates audited')
