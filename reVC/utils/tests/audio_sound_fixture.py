"""Audit sound-record boundaries and compare the real C ordering module."""
from pathlib import Path
import re
import audio_requests_migration as requests
import audio_mission_state_migration as mission
import audio_owner_entities_migration as entities
import audio_owner_init_migration as init
from audio_math_migration import calls

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-sound-c-tests'
old_header = (out / 'before/src/audio/AudioManager.h').read_text()
old_manager = (out / 'before/src/audio/AudioManager.cpp').read_text()
old_logic = (out / 'before/src/audio/AudioLogic.cpp').read_text()

def function(source, signature):
    # Extract the production ordering function without changing its statements
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

def normalize(source):
    # Compare statements independently of formatting and comments
    return re.sub(r'\s+', '', re.sub(r'//[^\n]*|/\*.*?\*/', '', source, flags=re.S))

old_order = function(old_manager, 'void\ncAudioManager::AddDetailsToRequestedOrderList')
expected_order = old_order[old_order.index('{'):]
for before, after in [('m_aRequestedQueue[m_nActiveQueue]', 'requested'), ('m_aRequestedOrderList[m_nActiveQueue]', 'order'), ('m_nActiveSamples', 'activeSamples'), ('uint32', 'uint32_t')]:
    expected_order = expected_order.replace(before, after)
kernel = (root / 'src/audio/AudioSoundQueue.c').read_text()
new_order = function(kernel, 'void\nAudioSoundQueue_InsertOrder')
assert normalize(expected_order) == normalize(new_order[new_order.index('{'):])

def adapt(source):
    # Apply only explicit copies at diagnosed game-vector boundaries
    source = source.replace('#include "AudioManager.h"', '#include "AudioManager.h"\n#include "AudioSoundGame.h"')
    lines = []
    for line in source.splitlines():
        assignment = re.match(r'(\s*)((?:m_sQueueSample|sample|m_asActiveSamples\[i\])\.m_vecPos) = (.*);$', line)
        outgoing = re.match(r'(\s*)((?:CVector oldPos|backPropellerPos|vecPosOld|const CVector &commentPosition)) = (m_sQueueSample\.m_vecPos);$', line)
        translate = re.match(r'(\s*)TranslateEntity\(&((?:m_sQueueSample|sample|m_asActiveSamples\[k\])\.m_vecPos), &(\w+)\);$', line)
        if assignment:
            indent, left, right = assignment.groups()
            line = indent + left + ' = AudioSoundPosition_FromVector(' + right + ');'
        elif outgoing:
            indent, left, right = outgoing.groups()
            line = indent + left + ' = AudioSoundPosition_ToVector(&' + right + ');'
        elif translate:
            indent, position, target = translate.groups()
            line = indent + 'const CVector soundPosition = AudioSoundPosition_ToVector(&' + position + ');\n' + indent + 'TranslateEntity(&soundPosition, &' + target + ');'
        else:
            line = re.sub(r'GetDistanceSquared\(((?:m_sQueueSample|sample)\.m_vecPos)\)', r'GetDistanceSquared(AudioSoundPosition_ToVector(&\1))', line)
            line = re.sub(r'(GetIsLineOfSightClear\(TheCamera.GetPosition\(\), )((?:m_sQueueSample|sample)\.m_vecPos)(,)', r'\1AudioSoundPosition_ToVector(&\2)\3', line)
        lines.append(line)
    return '\n'.join(lines) + '\n'

expected_manager = old_manager.replace(old_order + '\n\n', '').replace('AddDetailsToRequestedOrderList(sampleIndex);', 'AudioSoundQueue_InsertOrder(m_aRequestedQueue[m_nActiveQueue], m_aRequestedOrderList[m_nActiveQueue], m_nActiveSamples, sampleIndex);')
# Delimit adjacent sound consumers after the separately verified RNG method removal
expected_manager = expected_manager.replace(init.function(expected_manager, 'cAudioManager::', 'GenerateIntegerRandomNumberTable') + '\n', '')
def game_functions(source):
    # Keep conditional branches verbatim when delimiting top-level game functions
    declarations = list(re.finditer(r'(?m)^[A-Za-z_][A-Za-z_0-9 *]*\n(cAudioManager::[A-Za-z_][A-Za-z_0-9]*)\([^\n]*\)\n\{', source))
    return {declaration.group()[:-1]: source[declaration.start():declarations[index+1].start() if index+1<len(declarations) else len(source)] for index, declaration in enumerate(declarations)}

for expected_source, current_path in [(expected_manager, 'src/audio/AudioManager.cpp'), (old_logic, 'src/audio/AudioLogic.cpp')]:
    # Audit sound-position consumers independently of unrelated mission and reflection storage
    current_functions = game_functions((root / current_path).read_text())
    for signature, expected_function in game_functions(expected_source).items():
        # Compare each owning function after its explicit vector and ordering substitutions
        if not re.search(r'(?:m_sQueueSample|sample|m_asActiveSamples\[[^\]]+\])\.m_vecPos', expected_function):
            continue
        expected_function = mission.calls(entities.calls(calls(adapt(expected_function)))).replace('AudioSoundPosition_FromVector(m_avecReflectionsPos[i])', 'm_avecReflectionsPos[i]')
        if 'cAudioManager::AddReflectionsToRequestedQueue' in signature:
            expected=requests.body(function(expected_function,signature)).replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
            actual=function((root/'src/audio/AudioRequests.c').read_text(),'void\nAudioRequests_AddReflections')
            assert normalize(expected)==normalize(actual[actual.index('{'):])
        elif 'cAudioManager::ClearActiveSamples' in signature:
            # Check the migrated reset statements against their actual C implementation
            expected_reset = function(expected_function, signature)
            expected_reset = expected_reset[expected_reset.index('{'):].replace('uint8 ', 'uint8_t ').replace('m_asActiveSamples', 'active').replace('m_nActiveSamples', 'activeSamples').replace('FALSE', '0')
            expected_reset = expected_reset.replace('active[i].m_vecPos = AudioSoundPosition_FromVector(CVector(0.0f, 0.0f, 0.0f));', 'active[i].m_vecPos.x = 0.0f; active[i].m_vecPos.y = 0.0f; active[i].m_vecPos.z = 0.0f;')
            actual_reset = function((root / 'src/audio/AudioSoundReset.c').read_text(), 'void\nAudioSoundQueue_ClearActive')
            assert normalize(expected_reset) == normalize(actual_reset[actual_reset.index('{'):])
        elif 'cAudioManager::AddReleasingSounds' in signature:
            # Follow the independently audited attachment and fading conversion into C
            import audio_release_migration as release
            actual_release = function((root / 'src/audio/AudioRelease.c').read_text(), 'void\nAudioRelease_Add')
            assert normalize(release.body(expected_function)) == normalize(actual_release[actual_release.index('{'):])
        elif 'cAudioManager::ProcessActiveQueues' in signature:
            # Delimit mutually exclusive startup braces and follow the actual C playback operation
            import audio_active_migration as active
            expected_active = active.extract(expected_function)
            actual_active = active.extract((root / 'src/audio/AudioActive.c').read_text(), 'void\nAudioActive_Process(')
            assert normalize(active.body(expected_active)) == normalize(actual_active[actual_active.index('{'):])
        else:
            current_function = current_functions[signature]
            assert normalize(expected_function) == normalize(current_function), signature
record = old_header[old_header.index('class tSound'):old_header.index('VALIDATE_SIZE(tSound')]
expected_record = record.replace('class tSound\n{\npublic:', 'typedef struct tSound {').replace('CVector m_vecPos', 'AudioSoundPosition m_vecPos').replace('\n};', '\n} tSound;')
for before, after in [('int32', 'int32_t'), ('uint32', 'uint32_t'), ('int8', 'int8_t'), ('uint8', 'uint8_t'), ('bool8', 'uint8_t')]:
    expected_record = re.sub(r'\b'+before+r'\b', after, expected_record)
new_header = (root / 'src/audio/AudioSound.h').read_text()
actual_record = new_header[new_header.index('typedef struct tSound'):new_header.index('#ifdef __cplusplus')]
assert normalize(expected_record) == normalize(actual_record)

# Supply a controlled vector with the real game's coordinate constructor
vector = (root / 'src/math/Vector.h').read_text()
ctor = vector[vector.index('CVector(float x, float y, float z)'):vector.index('\n\tCVector(const RwV3d')]
assert normalize(vector[vector.index('CVector(void)'):vector.index('\n\tCVector(float')]) == normalize('CVector(void) {}')
(out / 'common.h').write_text('#pragma once\nstruct RwV3d {float x,y,z;};\nstruct CVector : RwV3d { CVector() {}\n' + ctor + '\n};\n', newline='\n')
fields, layout = [], []
for line in record.splitlines():
    if line.lstrip().startswith('#'):
        fields.append(line)
        layout.append(line)
    match = re.match(r'\s*(?:int32|uint32|uint8|bool8|CVector|float|int8)\s+(\w+);', line)
    if match:
        name = match.group(1)
        fields.append('record(&sound->' + name + ', sizeof(sound->' + name + '));')
        layout.append('const size_t layout_' + name + '[] = {offsetof(tSound,' + name + '),sizeof(((tSound *)0)->' + name + ')}; record(layout_' + name + ',sizeof(layout_' + name + '));')
template = (root / 'utils/tests/audio_sound.cpp.in').read_text()
for variant in ('before', 'after'):
    headers = '#include "config.h"\n#include "audio_enums.h"\n#include "common.h"\n'
    if variant == 'before':
        headers += 'using int32=int32_t; using uint32=uint32_t; using uint8=uint8_t; using int8=int8_t; using bool8=uint8_t;\n' + record
        method, operation = old_order, 'state.AddDetailsToRequestedOrderList(sample);'
        c_check = ''
    else:
        headers += '#include "AudioSoundGame.h"\n#include <type_traits>\nstatic_assert(std::is_trivial<tSound>::value, "C sound lifecycle");\nextern "C" void TestAudioSoundC(void);\n'
        method, operation = '', 'AudioSoundQueue_InsertOrder(state.m_aRequestedQueue[state.m_nActiveQueue], state.m_aRequestedOrderList[state.m_nActiveQueue], state.m_nActiveSamples, sample);'
        c_check = 'TestAudioSoundC();'
    source = template
    for key, value in {'@HEADERS@': headers, '@METHOD@': method, '@INSERT@': operation, '@FIELDS@': '\n'.join(fields), '@LAYOUT@': '\n'.join(layout), '@C_CHECK@': c_check, '@POSITION_FROM@': 'position' if variant == 'before' else 'AudioSoundPosition_FromVector(position)', '@POSITION_TO@': 'sound.m_vecPos' if variant == 'before' else 'AudioSoundPosition_ToVector(&sound.m_vecPos)'}.items():
        source = source.replace(key, value)
    (out / (variant + '.cpp')).write_text(source, newline='\n')
shapes = {'default': '', 'vanilla': '#undef FIX_BUGS\n', 'no-external': '#undef EXTERNAL_3D_SOUND\n', 'no-reflections': '#undef AUDIO_REFLECTIONS\n', 'ps2': '#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#undef AUDIO_REVERB\n#define GTA_PS2\n', 'ps2-reverb': '#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}
for name, flags in shapes.items():
    (out / (name + '.h')).write_text('#include "config.h"\n' + flags, newline='\n')
(out / 'caller.c').write_text('''#include "AudioSound.h"
#include <assert.h>
void TestAudioSoundC(void) {
    // Exercise the real ordering function using the plain C sound record
    tSound sounds[2] = {0};
    uint8_t order[2] = {0,1};
    sounds[0].m_nFinalPriority = 100;
    sounds[1].m_nFinalPriority = 17;
    AudioSoundQueue_InsertOrder(sounds, order, 2, 1);
    assert(order[0] == 1 && order[1] == 0);
}
''', newline='\n')
print('Sound fields, ordering statements, empty vector lifecycle and every game boundary audited')
