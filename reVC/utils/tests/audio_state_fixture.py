"""Compare original audio constructors/queue methods with the real C headers."""
from pathlib import Path

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-state-c-tests'
before = out / 'before/src/audio'
template = (root / 'utils/tests/audio_state.c.in').read_text()

def extract_class(source, name):
    start = source.index('class ' + name + '\n')
    brace = source.index('{', start)
    depth = 1
    end = brace + 1
    # Extract the original class with its exact constructor and queue bodies
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end + 1]

manager = (before / 'AudioManager.h').read_text()
radio = (before / 'PolRadio.h').read_text()
classes = '\n'.join(extract_class(manager, name) for name in ('cAudioScriptObjectManager', 'cPedParams', 'cVehicleParams'))
classes += '\n' + extract_class(radio, 'cPoliceRadioQueue')
compat = '''
#include <new>
#include "config.h"
#include "AudioSamples.h"
using int32 = int32_t; using uint32 = uint32_t; using uint8 = uint8_t; using bool8 = uint8_t;
class CPed; class CVehicle; class cTransmission;
#define nil nullptr
#define TRUE 1
#define FALSE 0
#define POLICE_RADIO_QUEUE_MAX_SAMPLES 60
''' + classes
(out / 'baseline.h').write_text(compat, newline='\n')

adapters = '''
static void ManagerInit(cAudioScriptObjectManager *p) {
 // Execute the original constructor on seeded storage
 new (p) cAudioScriptObjectManager;
}
static void ManagerReset(cAudioScriptObjectManager *p) {
 // Perform the original live count reset without ending the object's lifetime
 p->m_nScriptObjectEntityTotal = 0;
}
static void PedInit(cPedParams *p) {
 // Execute the original constructor on seeded storage
 new (p) cPedParams;
}
static void VehicleInit(cVehicleParams *p) {
 // Execute the original constructor on seeded storage
 new (p) cVehicleParams;
}
static void QueueInit(cPoliceRadioQueue *p) {
 // Execute the original constructor on seeded storage
 new (p) cPoliceRadioQueue;
}
static void QueueReset(cPoliceRadioQueue *p) {
 // Invoke the unchanged queue reset method
 p->Reset();
}
static uint8_t Add(cPoliceRadioQueue *p, uint32_t sample) {
 // Invoke the unchanged queue producer method
 return p->Add(sample);
}
static uint32_t Remove(cPoliceRadioQueue *p) {
 // Invoke the unchanged queue consumer method
 return p->Remove();
}
'''
for variant in ('before', 'after'):
    replacements = {
        '@HEADERS@': '#include "baseline.h"' if variant == 'before' else '''
#include "AudioState.h"
#include "AudioPoliceQueue.h"
#ifdef __cplusplus
#include <type_traits>
static_assert(std::is_trivial<cAudioScriptObjectManager>::value, "no implicit script lifecycle");
static_assert(std::is_trivial<cPedParams>::value, "no implicit pedestrian lifecycle");
static_assert(std::is_trivial<cVehicleParams>::value, "no implicit vehicle lifecycle");
static_assert(std::is_trivial<cPoliceRadioQueue>::value, "no implicit radio queue lifecycle");
#endif
''',
        '@ADAPTERS@': adapters if variant == 'before' else '',
        '@MANAGER_INIT@': 'ManagerInit' if variant == 'before' else 'AudioScriptObjectManager_Reset',
        '@MANAGER_RESET@': 'ManagerReset' if variant == 'before' else 'AudioScriptObjectManager_Reset',
        '@PED_INIT@': 'PedInit' if variant == 'before' else 'PedParams_Init',
        '@VEHICLE_INIT@': 'VehicleInit' if variant == 'before' else 'VehicleParams_Init',
        '@QUEUE_INIT@': 'QueueInit' if variant == 'before' else 'PoliceRadioQueue_Reset',
        '@QUEUE_RESET@': 'QueueReset' if variant == 'before' else 'PoliceRadioQueue_Reset',
        '@ADD@': 'Add' if variant == 'before' else 'PoliceRadioQueue_Add',
        '@REMOVE@': 'Remove' if variant == 'before' else 'PoliceRadioQueue_Remove',
    }
    text = template
    for key, value in replacements.items(): text = text.replace(key, value)
    (out / (variant + ('.cpp' if variant == 'before' else '.c'))).write_text(text, newline='\n')
print('Saved constructors and queue methods extracted unchanged')
