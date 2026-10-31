"""Audit complete world loops and generate original/C comparisons."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_world_loops_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-world-loops-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioWorldLoops.c').read_text();methods=[]
for name,target in migration.METHODS.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioWorldLoops_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioDispatch.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
a=header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS');b=header.index('#if defined(AUDIO_MSS)',a);macros=header[a:b]
converted=macros.replace('m_sQueueSample','manager->m_sQueueSample')
for name in migration.MACRO_NAMES:converted=re.sub(r'\b'+name+r'\b','AUDIO_WORLD_LOOPS_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioWorldLoops_Projectiles')])
assert normalize(migration.constants(old)) in normalize(kernel)
assert normalize(migration.C_ARRAY) in normalize(kernel)
assert migration.ARRAY in old
ctor=(root/'src/math/Vector.h').read_text();ctor=ctor[ctor.index('CVector(float x, float y, float z)'):ctor.index('\n\tCVector(const RwV3d')]
assert normalize(ctor)==normalize('CVector(float x, float y, float z) { this->x=x;this->y=y;this->z=z; }')
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','../weapons/WeaponType.h'])
template=(root/'utils/tests/audio_world_loops.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@HOST@',migration.HOST).replace('@CONSTANTS@',migration.constants(old))
for variant in ['before','after']:
    # Compile the untouched original generators or invoke their actual separate C11 implementation
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@ARRAY@',migration.ARRAY if variant=='before' else 'extern "C" AudioSoundPosition aVecExtraSoundPosition[2];').replace('@RUN@','active->ProcessProjectiles();active->ProcessEscalators();active->ProcessExtraSounds()' if variant=='before' else 'TestWorldLoopsC(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestWorldLoopsC(cAudioManager *manager)
{
    // Exercise all three complete generators through their real C declarations
    AudioWorldLoops_Projectiles(manager);
    AudioWorldLoops_Escalators(manager);
    AudioWorldLoops_ExtraSounds(manager);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-reflections':'#undef AUDIO_REFLECTIONS\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete projectile/escalator/arcade generators, six adapters, writable C positions, macros and direct C routing audited')
