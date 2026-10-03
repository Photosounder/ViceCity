"""Check entity operations against their saved original game implementation."""
from pathlib import Path
import re
from audio_owner_entities_migration import creation_body

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-entity-c-tests'
old = (out / 'before/src/audio/AudioManager.cpp').read_text()
current = (root / 'src/audio/AudioManager.cpp').read_text()
kernel = (root / 'src/audio/AudioEntities.c').read_text()

def function(source, signature):
    # Extract the exact balanced body and its original signature
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

def normalize(source):
    # Ignore formatting and comments for production statement comparisons
    return re.sub(r'\s+', '', re.sub(r'//[^\n]*|/\*.*?\*/', '', source, flags=re.S))

signatures = ['int32\ncAudioManager::CreateEntity', 'void\ncAudioManager::DestroyEntity', 'bool8\ncAudioManager::GetEntityStatus', 'void\ncAudioManager::SetEntityStatus', 'void *\ncAudioManager::GetEntityPointer', 'void\ncAudioManager::PlayOneShot']
exports = ['AudioEntities_Destroy', 'AudioEntities_GetStatus', 'AudioEntities_SetStatus', 'AudioEntities_GetPointer', 'AudioEntities_PlayOneShot']
for signature, export in zip(signatures[1:], exports):
    original = function(old, signature)
    expected = original[original.index('{'):]
    for before, after in [('m_bIsInitialised', 'initialized'), ('m_asAudioEntities', 'entities'), ('m_aAudioEntityOrderList', 'order'), ('m_nAudioEntitiesCount', '(*count)'), ('m_sAudioScriptObjectManager.', 'scripts->'), ('ARRAY_SIZE(scripts->m_anScriptObjectEntityIndices)', 'NUM_SCRIPT_MAX_ENTITIES')]:
        expected = expected.replace(before, after)
    for before, after in [('uint32', 'uint32_t'), ('int32', 'int32_t'), ('int16', 'int16_t'), ('FALSE', '0'), ('TRUE', '1')]:
        expected = re.sub(r'\b' + before + r'\b', after, expected)
    expected = expected.replace('tAudioEntity &entity = entities[index];', 'tAudioEntity *entity = &entities[index];').replace('entity.', 'entity->')
    actual = function(kernel, export + '(')
    assert normalize(expected) == normalize(actual[actual.index('{'):]), export

start = old.index('static Const uint8 OneShotPriority[]')
end = old.index('\nvoid\ncAudioManager::PlayOneShot', start)
priority = old[start:end]
assert normalize(priority.replace('Const uint8', 'const uint8_t')) == normalize(kernel[kernel.index('static const uint8_t OneShotPriority[]'):kernel.index('\nvoid\nAudioEntities_PlayOneShot')])
init_start = old.index('\t\t\tm_asAudioEntities[i].m_bIsUsed = TRUE;')
init_end = old.index('\n\t\t\tm_aAudioEntityOrderList', init_start)
original_init = old[init_start:init_end]
expected_init = original_init.replace('m_asAudioEntities[i].', 'entity->').replace('entity->m_pEntity = entity;', 'entity->m_pEntity = pointer;').replace('TRUE', '1').replace('FALSE', '0')
actual_init = function(kernel, 'AudioEntity_Init(')
assert normalize('{'+expected_init+'}') == normalize(actual_init[actual_init.index('{'):])
expected_file = old.replace(original_init, '\t\t\tAudioEntity_Init(&m_asAudioEntities[i], type, entity);').replace(priority, '')
# Audit the selected-record initializer and queued-ID scan after the member moves to C
expected_create = function(expected_file, signatures[0])
actual_create = function((root/'src/audio/AudioManagerEntities.c').read_text(), 'AudioManager_CreateEntity(')
assert normalize(creation_body(expected_create)) == normalize(actual_create[actual_create.index('{'):])
for signature in signatures:
    # Require each migrated entity member to be absent from the game owner
    assert signature not in current, signature

old_header = (out / 'before/src/audio/AudioManager.h').read_text()
record = old_header[old_header.index('class tAudioEntity'):old_header.index('VALIDATE_SIZE(tAudioEntity')]
for shape, flags in {'default': '', 'vanilla': '#undef FIX_BUGS\n', 'no-external': '#undef EXTERNAL_3D_SOUND\n', 'ps2': '#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out / (shape + '.h')).write_text('#include "config.h"\n' + flags, newline='\n')

template = (root / 'utils/tests/audio_entity.cpp.in').read_text()
common = '''#include "AudioState.h"
#include "audio_enums.h"
#include "soundlist.h"
using int32 = int32_t; using uint32 = uint32_t; using int16 = int16_t; using uint16 = uint16_t; using uint8 = uint8_t; using bool8 = uint8_t;
#define TRUE 1
#define FALSE 0
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Const const
'''
# Keep the saved thin adapters solely to drive the unchanged standalone entity kernels
adapter_reference = (root/'build/audio-owner-entities-c-tests/before/src/audio/AudioManager.cpp').read_text()
for variant in ('before', 'after'):
    headers = common + (record if variant == 'before' else '#include "AudioEntities.h"\n#include <type_traits>\nstatic_assert(std::is_trivial<tAudioEntity>::value, "plain entity lifecycle");\nextern "C" void TestAudioEntityC(void);\n')
    methods = '\n'.join(function(old if variant == 'before' else adapter_reference, signature) for signature in signatures)
    if variant == 'before':
        methods = priority + '\n' + methods
    source = template.replace('@HEADERS@', headers).replace('@METHODS@', methods).replace('@C_CHECK@', '' if variant == 'before' else 'TestAudioEntityC();')
    (out / (variant + '.cpp')).write_text(source, newline='\n')
(out / 'caller.c').write_text('''#include "AudioEntities.h"
#include "soundlist.h"
#include <assert.h>
void TestAudioEntityC(void) {
    // Exercise record creation, status, events, and removal directly from C
    tAudioEntity entities[NUM_AUDIOENTITIES] = {0};
    uint32_t order[NUM_AUDIOENTITIES] = {0};
    uint32_t count = 1;
    cAudioScriptObjectManager scripts;
    int object;
    AudioScriptObjectManager_Reset(&scripts);
    AudioEntity_Init(&entities[0], AUDIOTYPE_PHYSICAL, &object);
    AudioEntities_SetStatus(entities, 1, 0, 231);
    assert(AudioEntities_GetStatus(entities, 1, 0) == 231);
    assert(AudioEntities_GetPointer(entities, 1, 0) == &object);
    AudioEntities_PlayOneShot(entities, &scripts, 1, 0, 0, 3.5f);
    assert(entities[0].m_AudioEvents == 1 && entities[0].m_afVolume[0] == 3.5f);
    AudioEntities_Destroy(entities, order, &count, 1, 0);
    assert(count == 0 && order[0] == NUM_AUDIOENTITIES && !entities[0].m_bIsUsed);
}
''', newline='\n')
print('Entity initialization, removal, accessors, event ordering, priority table and game integration verified')
