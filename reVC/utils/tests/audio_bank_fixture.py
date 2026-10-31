"""Audit the bank helper and owner method cleanup, then compare original/C provider behavior."""
from pathlib import Path
import re
from audio_owner_init_migration import fields
import audio_bank_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-bank-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioManager.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();current=(root/'src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioBank.c').read_text()
original=migration.function(old,'cAudioManager::','LoadBankIfNecessary');actual=migration.function(kernel,'AudioBank_','LoadIfNecessary')
assert normalize(migration.body(original))==normalize(actual[actual.index('{'):])
assert normalize(fields(header))==normalize(fields(current))
for name,transform in [('src/audio/AudioManager.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert '\tfloat Sqrt(' not in current and 'LoadBankIfNecessary(' not in current
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_bank.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header))
for variant in ['before','after']:
    # Execute the complete unchanged member or the actual separately compiled C11 operation
    run='\n#ifdef GTA_PS2\n    active->LoadBankIfNecessary((uint8_t)input);\n#endif\n' if variant=='before' else 'TestBankC(input)'
    (out/(variant+'.cpp')).write_text(template.replace('@METHOD@',original if variant=='before' else '').replace('@RUN@',run),newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestBankC(uint32_t bank)
{
    // Exercise the real byte-parameter C declaration only where the original feature exists
#ifdef GTA_PS2
    AudioBank_LoadIfNecessary((uint8_t)bank);
#else
    (void)bank;
#endif
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','ps2-no-reverb':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n#undef AUDIO_REVERB\n','ps2-no-reflections':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n#undef AUDIO_REFLECTIONS\n','desktop':'#undef GTA_PS2\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete bank helper, all owner fields, removed unused square-root method and full header/project edits audited')
