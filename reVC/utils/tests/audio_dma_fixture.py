"""Compare the saved DMAudio class with the C facade using unchanged bodies."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-dma-c-api-tests'
before = out / 'before/src/audio'
current = (root / 'src/audio/DMAudio.cpp').read_text()
original = (before / 'DMAudio.cpp').read_text()

def normalize(text):
    text = re.sub(r'//[^\n]*|/\*.*?\*/', '', text, flags=re.S)
    text = re.sub(r'#include[^\n]*', '', text)
    text = re.sub(r'\s+', '', text)
    return text

expected = original.replace('cDMAudio DMAudio;', '')
expected = expected.replace('cDMAudio::', 'DMAudio_')
expected = expected.replace('ReportCrime(eCrimeType crime, const CVector &pos)',
                            'ReportCrime(eCrimeType crime, const CVector *pos)')
expected = expected.replace('AudioManager.ReportCrime(crime, pos)',
                            'AudioManager.ReportCrime(crime, *pos)')
assert normalize(expected) == normalize(current), 'Unexpected forwarding body change'

header = (root / 'src/audio/DMAudio.h').read_text()
decls = re.findall(r'\b([\w]+(?:\s*\*)?|const char\s*\*)\s*(DMAudio_\w+)\(([^;]*)\);', header)
assert len(decls) == 62
cpp_types = '''
#include <stdint.h>
#include <stdio.h>
#include <type_traits>
#include "config.h"
using int32 = int32_t; using uint32 = uint32_t; using uint16 = uint16_t;
using uint8 = uint8_t; using int8 = int8_t; using bool8 = uint8_t;
#define Const const
#define TRUE 1
#define MAX_VOLUME 127
struct CVector { float x, y, z; };
struct CEntity {}; struct CPhysical {}; struct CPed {};
struct cAudioScriptObject { int16_t AudioId; CVector Posn; int32_t AudioEntity; };
static CVector vectorObject = {1.25f, -2.5f, 3.75f};
static cAudioScriptObject scriptObject = {-19, {}, -1};
static CEntity entityObject; static CPed pedObject;
static char textObject[] = "mission"; static float listenObject[] = {4.5f};
static uint64_t digest = 1469598103934665603ULL;
static int32_t entityResult;
static void mix(uint64_t value) {
 // Record each argument and return value in a deterministic digest
 digest = (digest ^ value) * 1099511628211ULL;
}
static void argument(const CVector &v) {
 // Preserve the crime position values in the trace
 mix((uint64_t)(int64_t)(v.x * 100)); mix((uint64_t)(int64_t)(v.y * 100)); mix((uint64_t)(int64_t)(v.z * 100));
}
template<class T> static void argument(T value) {
 // Record scalar values and stable object identities
 if constexpr (std::is_pointer_v<T>) {
  const void *p = (const void *)value;
  mix(p == &vectorObject ? 1 : p == &scriptObject ? 2 : p == &entityObject ? 3 : p == &pedObject ? 4 : p == textObject ? 5 : p == listenObject ? 6 : p == nullptr ? 0 : 7);
 } else if constexpr (std::is_floating_point_v<T>) mix((uint64_t)(int64_t)(value * 100));
 else mix((uint64_t)value);
}
struct Result {
 template<class T> operator T() const {
  // Return stable values using the exact facade return type
  if constexpr (std::is_pointer_v<T>) {
   if constexpr (std::is_same_v<T, float *>) return listenObject;
   else return textObject;
  } else return (T)37;
 }
};
extern "C" void *test_object(int id) {
 // Supply real objects to the C caller through opaque pointers
 switch(id) {case 1:return &vectorObject; case 2:return &scriptObject; case 3:return &entityObject; case 4:return &pedObject; default:return textObject;}
}
extern "C" void test_int(int64_t value) {
 // Record scalar results from the C caller
 mix((uint64_t)value);
}
extern "C" void test_pointer(const void *value) {
 // Record pointer results without process-specific addresses
 argument(value);
}
extern "C" void test_entity_result(int32_t value) {
 // Select successful and failed script entity creation paths
 entityResult = value;
}
'''
managers = []
for manager in ('AudioManager', 'MusicManager'):
    methods = sorted(set(re.findall(manager + r'\.(\w+)\(', current)))
    lines = [f'struct Fake{manager} {{', ' int32_t m_nFrontEndEntity = -27; uint8_t m_bIsPlayerShutUp = 0;']
    for name in methods:
        ret = 'int32_t' if name == 'CreateEntity' else 'void' if name in ('SetPedTalkingStatus', 'SetPlayersMood') else 'Result'
        lines += [f' template<class... T> {ret} {name}(T... values) {{',
                  '  // Trace the real wrapper forwarding operation',
                  f'  for (const char *p = "{manager}.{name}"; *p; ++p) mix((unsigned char)*p);',
                  '  (argument(values), ...);']
        if ret == 'int32_t': lines += ['  return entityResult;']
        elif ret == 'Result': lines += ['  return {};']
        lines += [' }']
    lines += [f'}} {manager};']
    managers += lines

calls = []
for ret, name, params in decls:
    args = []
    for param in ([] if params == 'void' else params.split(',')):
        if 'CVector' in param: arg = '(const CVector *)test_object(1)'
        elif 'cAudioScriptObject' in param: arg = '(cAudioScriptObject *)test_object(2)'
        elif 'CEntity' in param: arg = '(CEntity *)test_object(3)'
        elif 'CPed' in param: arg = '(CPed *)test_object(4)'
        elif 'char' in param: arg = '(const char *)test_object(5)'
        elif 'void *' in param: arg = 'test_object(3)'
        elif 'enum eAudioType' in param: arg = '(enum eAudioType)2'
        elif 'enum eCrimeType' in param: arg = '(enum eCrimeType)3'
        elif 'float' in param: arg = '-2.75f'
        elif 'int32_t' in param and 'uint32_t' not in param: arg = '-17'
        else: arg = '239'
        args.append(arg)
    call = name + '(' + ', '.join(args) + ')'
    calls += [call + ';' if ret == 'void' else ('test_pointer' if '*' in ret else 'test_int') + '(' + call + ');']
for volume in ('SetMP3BoostVolume', 'SetEffectsMasterVolume', 'SetMusicMasterVolume', 'SetEffectsFadeVol', 'SetMusicFadeVol'):
    calls += [f'for (int v=0; v<256; ++v) DMAudio_{volume}((uint8_t)v);']
calls += ['for (int v=-1; v<=1; ++v) { test_entity_result(v); test_int(DMAudio_CreateLoopingScriptObject((cAudioScriptObject *)test_object(2))); DMAudio_CreateOneShotScriptObject((cAudioScriptObject *)test_object(2)); }']
caller = '''
#include "DMAudio.h"
extern void *test_object(int);
extern void test_int(int64_t);
extern void test_pointer(const void *);
extern void test_entity_result(int32_t);
void run_facade(void) {
 // Exercise all exports, volume bounds, and script entity success paths
''' + '\n'.join(calls) + '\n}\n'
(out / 'caller.c').write_text(caller, newline='\n')
baseline_header = (before / 'DMAudio.h').read_text().replace('#include "Crime.h"', '#include "CrimeTypes.h"')
(out / 'baseline.h').write_text(baseline_header, newline='\n')
for variant, source in (('before', original), ('after', current)):
    source = re.sub(r'#include[^\n]*', '', source)
    include = '#include "baseline.h"' if variant == 'before' else '#include "DMAudio.h"'
    fixture = cpp_types + include + '\n' + '\n'.join(managers) + '\n' + source
    if variant == 'before':
        baseline_caller = caller.replace('#include "DMAudio.h"', '').replace('DMAudio_', 'DMAudio.')
        baseline_caller = baseline_caller.replace('DMAudio.ReportCrime((enum eCrimeType)3, (const CVector *)test_object(1))', 'DMAudio.ReportCrime((enum eCrimeType)3, *(const CVector *)test_object(1))')
        fixture += baseline_caller
    else: fixture += '\nextern "C" void run_facade(void);\n'
    fixture += '''
int main() {
 // Compare the complete forwarding trace and final player speech flag
 run_facade(); mix(AudioManager.m_bIsPlayerShutUp); printf("%llu\\n", (unsigned long long)digest); return 0;
}
'''
    (out / (variant + '.cpp')).write_text(fixture, newline='\n')
print('All 62 forwarding bodies preserved; C and baseline callers generated')
