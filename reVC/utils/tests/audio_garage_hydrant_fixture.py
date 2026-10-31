"""Audit complete garage/hydrant conversion and generate original/C comparisons."""
from pathlib import Path
import re
from audio_owner_init_migration import fields
import audio_garage_hydrant_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-garage-hydrant-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioGarageHydrant.c').read_text();methods=[]
for name,target in migration.METHODS.items():
    original=migration.extract(old,'cAudioManager::',name);actual=migration.extract(kernel,'AudioGarageHydrant_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioDispatch.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project),('src/control/Garages.h',migration.garage_header)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
for name in ['eGarageState','eGarageType']:assert normalize(migration.enumeration((out/'before/src/control/Garages.h').read_text(),name))==normalize(migration.enumeration((root/'src/control/GarageTypes.h').read_text(),name))
a=header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS');b=header.index('#if defined(AUDIO_MSS)',a);macros=header[a:b]
converted=macros.replace('m_sQueueSample','manager->m_sQueueSample')
for name in migration.MACRO_NAMES:converted=re.sub(r'\b'+name+r'\b','AUDIO_GARAGE_HYDRANT_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioGarageHydrant_Garages')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','soundlist.h','../control/GarageTypes.h'])
template=(root/'utils/tests/audio_garage_hydrant.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@HOST@',migration.HOST).replace('@CONSTANTS@',migration.CONSTANTS)
for variant in ['before','after']:
    # Compile complete unchanged originals or invoke actual separate C11 operations
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','active->ProcessGarages();active->ProcessFireHydrant()' if variant=='before' else 'TestGarageHydrantC(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestGarageHydrantC(cAudioManager *manager)
{
    // Exercise both complete generators through their real C declarations
    AudioGarageHydrant_Garages(manager);
    AudioGarageHydrant_Hydrant(manager);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-reflections':'#undef AUDIO_REFLECTIONS\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete garage/hydrant generators, five adapters, shared enums, conditional index widths, cached distance and counter wrapping audited')
