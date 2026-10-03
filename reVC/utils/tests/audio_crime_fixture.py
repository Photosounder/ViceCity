"""Run actual old/new report adapters with controlled game eligibility state."""
from pathlib import Path
import re
import audio_game_entries_migration as entries
from audio_controls_migration import function as entry_function

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-crime-c-tests'
before = out / 'before/src/audio'

def function(source, signature):
    # Extract the exact body of the requested production function
    start = source.index('void\n' + signature)
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

old = (before / 'PolRadio.cpp').read_text()
# Retain the saved thin adapter solely to drive the unchanged crime record kernel
current = (root / 'build/audio-game-entries-c-tests/before/src/audio/PolRadio.cpp').read_text()
old_report = function(old, 'cAudioManager::ReportCrime')
new_report = function(current, 'cAudioManager::ReportCrime')
gate = lambda source: source[source.index('if (m_bIsInitialised'):source.index(' {', source.index('if (m_bIsInitialised'))]
assert gate(old_report) == gate(new_report), 'Game eligibility or cooldown checks changed'
old_age = function(old, 'cAudioManager::AgeCrimes')
kernel = (root / 'src/audio/AudioCrimes.c').read_text()

def normalize(source):
    # Compare implementation statements independently of comments and formatting
    source = re.sub(r'//[^\n]*|/\*.*?\*/', '', source, flags=re.S)
    return re.sub(r'\s+', '', source)

actual_entry=entry_function((root/'src/audio/AudioGameEntries.c').read_text(),'AudioPolice_','ReportCrime')
assert normalize(entries.crime_body(new_report))==normalize(actual_entry[actual_entry.index('{'):]), 'Actual C crime entry changed'
expected_age = old_age[old_age.index('{'):]
expected_age = expected_age.replace('ARRAY_SIZE(m_aCrimes)', 'AUDIO_CRIME_COUNT').replace('m_aCrimes', 'crimes').replace('uint8', 'uint8_t')
new_age = function(kernel, 'AudioCrimes_Age')
assert normalize(expected_age) == normalize(new_age[new_age.index('{'):]), 'Aging statements changed'
gate_brace = old_report.index(' {', old_report.index('if (m_bIsInitialised')) + 1
depth = 1
gate_end = gate_brace + 1
while depth:
    depth += (old_report[gate_end] == '{') - (old_report[gate_end] == '}')
    gate_end += 1
expected_report = '{\nint32_t lastCrime = AUDIO_CRIME_COUNT;\n' + old_report[gate_brace + 1:gate_end - 1] + '\n}'
expected_report = expected_report.replace('ARRAY_SIZE(m_aCrimes)', 'AUDIO_CRIME_COUNT').replace('m_aCrimes', 'crimes').replace('int32 ', 'int32_t ')
expected_report = expected_report.replace('gMinTimeToNextReport', 'nextReport').replace('m_FrameCounter', 'frame')
for index in ('i', 'lastCrime'):
    expected_report = expected_report.replace(f'crimes[{index}].position = pos;', ''.join(f'crimes[{index}].position.{axis} = {axis};' for axis in 'xyz'))
new_report_kernel = function(kernel, 'AudioCrimes_Report')
assert normalize(expected_report) == normalize(new_report_kernel[new_report_kernel.index('{'):]), 'Crime update statements changed'
record_header = (before / 'PolRadio.h').read_text()
record_header = record_header[record_header.index('struct cAMCrime'):record_header.index('VALIDATE_SIZE(cAMCrime')]
common = '''
#include "audio_enums.h"
#include "CrimeTypes.h"
using int32 = int32_t; using uint32 = uint32_t; using uint16 = uint16_t; using uint8 = uint8_t;
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
struct CVector {float x,y,z; CVector() {} CVector(float a,float b,float c):x(a),y(b),z(c) {}};
'''
old_init = '''
static void Init(cAMCrime *crimes) {
 // Construct the original crime records explicitly for the comparison fixture
 for (int i=0; i<10; ++i) new (&crimes[i]) cAMCrime;
}
'''
template = (root / 'utils/tests/audio_crime.c.in').read_text()
for variant in ('before', 'after'):
    headers = common + ('#include <new>\n' + record_header if variant == 'before' else '#include "AudioCrimes.h"\n#include <type_traits>\nstatic_assert(std::is_trivial<cAMCrime>::value, "no implicit crime lifecycle");\nextern "C" void TestCrimeC(void);')
    replacement = {
        '@HEADERS@': headers,
        '@AGE_DECL@': 'void AgeCrimes();' if variant == 'before' else '',
        '@METHODS@': old_report + '\n' + old_age + '\n' + old_init if variant == 'before' else new_report,
        '@INIT@': 'Init' if variant == 'before' else 'AudioCrimes_Init',
        '@AGE@': 'manager.AgeCrimes();' if variant == 'before' else 'AudioCrimes_Age(manager.m_aCrimes);',
        '@C_CHECK@': '' if variant == 'before' else 'TestCrimeC();',
    }
    text = template
    for key, value in replacement.items(): text = text.replace(key, value)
    (out / (variant + '.cpp')).write_text(text, newline='\n')
(out / 'caller.c').write_text('''
#include "PolRadio.h"
#include <assert.h>
void TestCrimeC(void) {
 // Exercise initialization, last-slot insertion, duplicate refresh, and expiration from C
 cAMCrime crimes[AUDIO_CRIME_COUNT];
 uint32_t next[NUM_CRIME_TYPES] = {0};
 AudioCrimes_Init(crimes);
 AudioCrimes_Report(crimes, next, CRIME_HIT_PED, 17, 1, 2, 3);
 assert(crimes[9].type == CRIME_HIT_PED && next[CRIME_HIT_PED] == 517);
 AudioCrimes_Report(crimes, next, CRIME_HIT_PED, 999, 4, 5, 6);
 assert(crimes[9].position.x == 4 && next[CRIME_HIT_PED] == 517);
 crimes[9].timer = 1200;
 AudioCrimes_Age(crimes);
 assert(crimes[9].type == CRIME_NONE && crimes[9].timer == 1201);
}
''', newline='\n')
print('Original and current game report adapters extracted; eligibility gate unchanged')
