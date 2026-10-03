"""Compare the original collision class with the real C queue module."""
from pathlib import Path
import re

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-collision-c-tests'
before = out / 'before/src/audio'
header = (before / 'AudioCollision.h').read_text()
original = (before / 'AudioCollision.cpp').read_text()
start = original.index('void\ncAudioCollisionManager::AddCollisionToRequestedQueue()')
end = original.index('void\ncAudioManager::ServiceCollisions()', start)
method = original[start:end]
compat = '''
#include <new>
using uint8 = uint8_t; using uint32 = uint32_t; using int32 = int32_t;
#define nil nullptr
#define VALIDATE_SIZE(type, size) static_assert(sizeof(void *) != 4 || sizeof(type) == size, "original layout")
struct CVector { float x, y, z; CVector() {} CVector(float a, float b, float c):x(a),y(b),z(c) {} };
''' + header
(out / 'baseline.h').write_text(compat, newline='\n')

def normalize(body):
    body = re.sub(r'//[^\n]*|/\*.*?\*/', '', body, flags=re.S)
    return re.sub(r'\s+', '', body)

original_body = method[method.index('{'):method.rindex('}') + 1]
expected = re.sub(r'(?<![.\w])(m_\w+)', r'manager->\1', original_body).replace('uint32', 'uint32_t')
current = (root / 'src/audio/AudioCollisionQueue.c').read_text()
current_body = current[current.index('{', current.index('AudioCollisionManager_AddCollisionToRequestedQueue')):current.rindex('}') + 1]
assert normalize(expected) == normalize(current_body), 'Collision insertion algorithm changed'

ratio_start = original.index('float\ncAudioManager::GetCollisionOneShotRatio')
ratio_methods = re.sub(r'^//[+-] rouz edit \(ChatGPT\)\n?', '', original[ratio_start:], flags=re.M)
ratio_class = '''
class cAudioManager {
public:
 float GetCollisionOneShotRatio(uint32 a, float b);
 float GetCollisionLoopingRatio(uint32 a, uint32 b, float c);
 float GetCollisionRatio(float a, float b, float c, float d);
};
''' + ratio_methods + '\nstatic cAudioManager ratioManager;\n'
current_math = (root / 'src/audio/AudioCollisionMath.c').read_text()
expected_math = ratio_methods.replace('uint32', 'uint32_t')
for old, new in (('GetCollisionOneShotRatio', 'AudioCollision_GetOneShotRatio'), ('GetCollisionLoopingRatio', 'AudioCollision_GetLoopingRatio'), ('GetCollisionRatio', 'AudioCollision_GetRatio')):
    expected_math = expected_math.replace('cAudioManager::' + old, new).replace(old + '(', new + '(')
current_math = re.sub(r'#include[^\n]*', '', current_math)
assert normalize(expected_math) == normalize(current_math), 'Collision ratio bodies changed'

adapters = method + ratio_class + '''
static void Reset(cAudioCollision *p) {
 // Invoke the unchanged live collision reset method
 p->Reset();
}
static void Init(cAudioCollisionManager *p) {
 // Invoke the original manager and record constructors
 new (p) cAudioCollisionManager;
}
static void Add(cAudioCollisionManager *p) {
 // Invoke the unchanged queue insertion method
 p->AddCollisionToRequestedQueue();
}
'''
template = (root / 'utils/tests/audio_collision.c.in').read_text()
for variant in ('before', 'after'):
    replacements = {
        '@HEADERS@': '#include "baseline.h"' if variant == 'before' else '#include "AudioCollision.h"\nextern void TestCollisionCpp(void);',
        '@ADAPTERS@': adapters if variant == 'before' else '',
        '@CPP_CHECK@': '' if variant == 'before' else 'TestCollisionCpp();',
        '@RESET@': 'Reset' if variant == 'before' else 'AudioCollision_Reset',
        '@INIT@': 'Init' if variant == 'before' else 'AudioCollisionManager_Init',
        '@ADD@': 'Add' if variant == 'before' else 'AudioCollisionManager_AddCollisionToRequestedQueue',
        '@ONE_SHOT_RATIO@': 'ratioManager.GetCollisionOneShotRatio' if variant == 'before' else 'AudioCollision_GetOneShotRatio',
        '@LOOP_RATIO@': 'ratioManager.GetCollisionLoopingRatio' if variant == 'before' else 'AudioCollision_GetLoopingRatio',
        '@RATIO@': 'ratioManager.GetCollisionRatio' if variant == 'before' else 'AudioCollision_GetRatio',
    }
    text = template
    for key, value in replacements.items(): text = text.replace(key, value)
    (out / (variant + ('.cpp' if variant == 'before' else '.c'))).write_text(text, newline='\n')
(out / 'cpp-caller.cpp').write_text('''
#include "AudioCollision.h"
#include <assert.h>
#include <type_traits>
static_assert(std::is_trivial<cAudioCollision>::value, "no implicit collision lifecycle");
static_assert(std::is_trivial<cAudioCollisionManager>::value, "no implicit manager lifecycle");
extern "C" void TestCollisionCpp(void) {
 // Exercise all three real C exports from C++ with defined request fields
 cAudioCollisionManager manager;
 AudioCollisionManager_Init(&manager);
 assert(manager.m_bCollisionsInQueue == 0);
 manager.m_sQueue.m_nBaseVolume = 17;
 manager.m_sQueue.m_fDistance = 3.5f;
 AudioCollisionManager_AddCollisionToRequestedQueue(&manager);
 assert(manager.m_bCollisionsInQueue == 1 && manager.m_asCollisions1[0].m_fDistance == 3.5f);
 AudioCollision_Reset(&manager.m_asCollisions1[0]);
 assert(manager.m_asCollisions1[0].m_nBaseVolume == 17 && manager.m_asCollisions1[0].m_fDistance == 0);
 // Exercise the three collision ratio exports through their C linkage
 assert(AudioCollision_GetOneShotRatio(0, 60.0f) == 1.0f);
 assert(AudioCollision_GetLoopingRatio(0, 0, 0.01f) == 0.5f);
 assert(AudioCollision_GetRatio(0, 0, 1, 1) == 0);
}
''', newline='\n')
print('Collision insertion and ratio bodies preserved; original-class and C fixtures generated')
