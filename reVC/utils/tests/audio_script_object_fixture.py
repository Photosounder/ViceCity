"""Compile saved class bodies and real C script code against the same pool."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-script-object-c-tests'
before = out / 'before/src/audio'
template = (root / 'utils/tests/audio_script_object.c.in').read_text()
header = (before / 'AudioScriptObject.h').read_text()
compat = '''
#include <memory>
#include "config.h"
#include "Pool.h"
using int16 = int16_t; using int32 = int32_t; using uint32 = uint32_t; using uint8 = uint8_t;
#define nil nullptr
#define VALIDATE_SIZE(type, size) static_assert(sizeof(type) == size, "baseline layout")
struct CVector { float x, y, z; CVector() {} CVector(float a, float b, float c):x(a),y(b),z(c) {} };
''' + header + '''
extern "C" CPool *CPools_GetAudioScriptObjectPool(void);
struct CPools { static CPool *GetAudioScriptObjectPool() { return CPools_GetAudioScriptObjectPool(); } };
'''
(out / 'baseline.h').write_text(compat, newline='\n')
implementation = (before / 'AudioScriptObject.cpp').read_text()
implementation = re.sub(r'#include[^\n]*', '', implementation)
wrappers = '''
static void TestReset(cAudioScriptObject *object) {
 // Invoke the original reset method without changing its body
 object->Reset();
}
static void TestOneShot(uint8_t id, float x, float y, float z) {
 // Supply the original position-reference API with the same values
 const CVector position(x, y, z); PlayOneShotScriptObject(id, position);
}
'''
for variant in ('before', 'after'):
    text = template
    replacements = {
        '@TYPE_HEADER@': '#include "baseline.h"' if variant == 'before' else '#include "AudioScriptObject.h"',
        '@BASELINE_IMPLEMENTATION@': implementation + wrappers if variant == 'before' else '',
        '@RESET@': 'TestReset' if variant == 'before' else 'AudioScriptObject_Reset',
        '@SAVE@': 'cAudioScriptObject::SaveAllAudioScriptObjects' if variant == 'before' else 'AudioScriptObject_SaveAll',
        '@LOAD@': 'cAudioScriptObject::LoadAllAudioScriptObjects' if variant == 'before' else 'AudioScriptObject_LoadAll',
        '@ONESHOT@': 'TestOneShot' if variant == 'before' else 'PlayOneShotScriptObject',
    }
    for key, value in replacements.items(): text = text.replace(key, value)
    if variant == 'before':
        text = text.replace('#include "DMAudio.h"', '').replace('#include "baseline.h"', '#include "baseline.h"\n#include "DMAudio.h"')
    (out / (variant + ('.cpp' if variant == 'before' else '.c'))).write_text(text, newline='\n')

game = (root / 'src/core/re3.cpp').read_text()
counter = re.search(r'extern "C" \{\s*int32 _saveBufCount;\s*\}', game).group(0)
(out / 'counter.cpp').write_text('#include <stdint.h>\n#include "config.h"\nusing int32 = int32_t;\n#ifdef VALIDATE_SAVE_SIZE\n' + counter + '\n#endif\n', newline='\n')
after = (out / 'after.c').read_text().replace('int32_t _saveBufCount;', '')
after = after.replace('int main(void)', 'extern void TestCppCaller(void);\nint main(void)')
after = after.replace('    // Compare resets,', '    // Verify the C++ caller uses the C exports before audio is activated\n    TestCppCaller();\n\n    // Compare resets,')
(out / 'after.c').write_text(after, newline='\n')
(out / 'cpp-caller.cpp').write_text('''
#include "AudioScriptObject.h"
#include "DMAudio.h"
#include <assert.h>
#include <type_traits>
static_assert(std::is_standard_layout<cAudioScriptObject>::value, "plain layout");
static_assert(std::is_trivial<cAudioScriptObject>::value, "no implicit construction");
extern "C" void TestCppCaller(void) {
 // Exercise the C exports from C++ without invoking an object constructor
 cAudioScriptObject object;
 AudioScriptObject_Reset(&object);
 assert(object.AudioId == SCRIPT_SOUND_INVALID && object.AudioEntity == AEHANDLE_NONE);
 PlayOneShotScriptObject(1, 0, 0, 0);
}
''', newline='\n')
print('Original class and C production-source fixtures generated')
