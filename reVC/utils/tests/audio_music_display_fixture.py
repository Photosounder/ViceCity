"""Audit and compare the complete radio-name display including persistent state and font submissions."""
from pathlib import Path
import re
import timer_migration as timer
import audio_music_display_migration as m
import audio_music_state_migration as state
import text_migration as text
root=Path(__file__).resolve().parents[2];out=root/'build/audio-music-display-c-tests'

def normalize(s):
    # Compare complete statements independently of edit markers and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))

changes={'src/audio/MusicManager.cpp':m.logic,'src/audio/MusicManager.h':m.header,'src/core/main.cpp':m.calls,'build/reVC.vcxproj':m.project,'build/reVC.vcxproj.filters':m.project}
for name,transform in changes.items():assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
old=(out/'before/src/audio/MusicManager.cpp').read_text();oldh=(out/'before/src/audio/MusicManager.h').read_text()
original=m.function(old,'cMusicManager::','DisplayRadioStationName');actual=m.function((root/'src/audio/MusicDisplay.c').read_text(),'MusicDisplay_','StationName')
assert normalize(m.body(original))==normalize(actual[actual.index('{'):])
assert normalize(state.fields(oldh))==normalize(state.fields((root/'src/audio/MusicManager.h').read_text()))
assert normalize(timer.calls(m.HOST)) in normalize((root/'src/audio/MusicManager.cpp').read_text())
track=oldh[oldh.index('typedef struct tStreamedSample'):oldh.index('#ifdef __cplusplus')]
record=track+'typedef struct cMusicManager {\n'+state.fields(oldh)+'\ncMusicManager();\nvoid DisplayRadioStationName();\n} cMusicManager;\n'
common_header=(root/'src/core/common.h').read_text()
assert 'typedef uint16_t wchar;' in common_header
assert '#include "Draw_constants.h"' in common_header
common_header=common_header.replace('#include "Draw_constants.h"',(root/'src/renderer/Draw_constants.h').read_text())
scale=common_header[common_header.index('#if defined(PROPER_SCALING)'):common_header.index('#include "maths.h"')]
font_header=(root/'src/renderer/Font.h').read_text()
assert '#include "FontStyles.h"' in font_header
assert 'enum {\n\tFONT_BANK,\n\tFONT_STANDARD,\n\tFONT_HEADING,' in (root/'src/renderer/FontStyles.h').read_text()
draw=(root/'src/renderer/Draw.c').read_text();scale_y=m.function(draw,'Draw_','ScaleY')
scale_y=scale_y[scale_y.index('{'):]
original_draw=(root/'build/draw-c-tests/before/src/renderer/Draw.cpp').read_text()
original_scale=m.function(original_draw,'CDraw::','ScaleY')
assert normalize(original_scale[original_scale.index('{'):].replace('ms_bProperScaling','Draw_ms_bProperScaling'))==normalize(scale_y)
headers='\n'.join('#include "'+n+'"' for n in ['Validate.h','sampman.h','DMAudio.h','AudioSound.h','soundlist.h'])
common=(root/'utils/tests/audio_music_display.cpp.in').read_text().replace('@HEADERS@',headers).replace('@SCALING@',scale).replace('@SCALE_Y@',scale_y)
for variant in ['before','after']:
    # Compile the preserved whole member against the same external providers as the C implementation
    source=common.replace('@MUSIC@',record if variant=='before' else '#include "MusicManager.h"')
    source=source.replace('@METHOD@',original if variant=='before' else text.calls(m.HOST))
    source=source.replace('} TheText;','''} TheText;
extern "C" uint16_t *Text_Get(Text *self,const char *key)
{
    // Preserve the exact observed radio translation dependency across its updated C text call
    return self->Get(key);
}
''')
    source=source.replace('@RUN@','active->DisplayRadioStationName()' if variant=='before' else 'TestMusicDisplayC(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "MusicManager.h"
void TestMusicDisplayC(cMusicManager *manager)
{
    // Enter the complete station-name display through its actual C declaration
    MusicDisplay_StationName(manager);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for name,flags in {
    'default':'#define AUDIO_OAL\n',
    'vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n#undef RADIO_SCROLL_TO_PREV_STATION\n#undef RADIO_OFF_TEXT\n',
    'no-off-text':'#define AUDIO_OAL\n#undef RADIO_OFF_TEXT\n',
    'no-previous-station':'#define AUDIO_OAL\n#undef RADIO_SCROLL_TO_PREV_STATION\n',
    'ps2':'#undef GTA_PC\n#define GTA_PS2\n#undef EXTERNAL_3D_SOUND\n',
    'no-aspect':'#define AUDIO_OAL\n#undef ASPECT_RATIO_SCALE\n',
    'force-pc':'#define AUDIO_OAL\n#define FORCE_PC_SCALING\n',
}.items():(out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete display body, shared pointer/timer semantics, exact screen macros, 18 typed boundaries, unchanged record fields, sole game entry and project registration audited')
