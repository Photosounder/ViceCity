"""Audit complete water-cannon generation and generate original/C comparisons."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_water_cannon_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-water-cannon-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioWaterCannon.c').read_text()
method=function(old,'cAudioManager::','ProcessWaterCannon');actual=function(kernel,'AudioWaterCannon_','Process')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioDispatch.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name

a=header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS');b=header.index('#if defined(AUDIO_MSS)',a);macros=header[a:b]
converted=macros.replace('m_sQueueSample','manager->m_sQueueSample')
for name in migration.MACRO_NAMES:converted=re.sub(r'\b'+name+r'\b','AUDIO_WATER_CANNON_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioWaterCannon_Process')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_water_cannon.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@HOST@',migration.HOST).replace('@CONSTANTS@',migration.CONSTANTS)
for variant in ['before','after']:
    # Compile the untouched original generator or call the real C11 implementation
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@RUN@','active->ProcessWaterCannon((int32)seed)' if variant=='before' else 'TestWaterCannonC(active,(int32)seed)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestWaterCannonC(cAudioManager *manager,int32_t id)
{
    // Invoke complete water-cannon generation through its real C declaration
    AudioWaterCannon_Process(manager,id);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-reflections':'#undef AUDIO_REFLECTIONS\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete water-cannon generator, two game adapters, macros, constants and direct C dispatcher audited')
