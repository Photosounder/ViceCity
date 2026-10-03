"""Compare live queue resets with the original owner methods."""
from pathlib import Path
import re
import audio_mission_data_migration as data
import audio_mission_state_migration as mission
import audio_owner_entities_migration as entities
import audio_owner_init_migration as init
import audio_provider_migration as provider
import audio_controls_migration as controls
from audio_math_migration import calls

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-sound-reset-c-tests'
old = (out / 'before/src/audio/AudioManager.cpp').read_text()
current = (root / 'src/audio/AudioManager.cpp').read_text()
kernel = (root / 'src/audio/AudioSoundReset.c').read_text()

def function(source, signature):
    # Extract the exact reset implementation including its conditional fields
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

def normalize(source):
    # Compare statements independently of comments and whitespace
    return re.sub(r'\s+', '', re.sub(r'//[^\n]*|/\*.*?\*/', '', source, flags=re.S))

old_queue = function(old, 'void\ncAudioManager::ClearRequestedQueue')
old_active = function(old, 'void\ncAudioManager::ClearActiveSamples')
expected_queue = old_queue[old_queue.index('{'):].replace('uint8 ', 'uint8_t ').replace('m_aRequestedOrderList[m_nActiveQueue]', 'order').replace('m_nRequestedCount[m_nActiveQueue]', '*count').replace('m_nActiveSamples', 'activeSamples')
expected_active = old_active[old_active.index('{'):].replace('uint8 ', 'uint8_t ').replace('m_asActiveSamples', 'active').replace('m_nActiveSamples', 'activeSamples').replace('FALSE', '0')
expected_active = expected_active.replace('active[i].m_vecPos = AudioSoundPosition_FromVector(CVector(0.0f, 0.0f, 0.0f));', 'active[i].m_vecPos.x = 0.0f; active[i].m_vecPos.y = 0.0f; active[i].m_vecPos.z = 0.0f;')
for signature, expected in [('void\nAudioSoundQueue_ClearRequested', expected_queue), ('void\nAudioSoundQueue_ClearActive', expected_active)]:
    actual = function(kernel, signature)
    assert normalize(expected) == normalize(actual[actual.index('{'):]), signature
expected_owner = old.replace(old_queue + '\n\n', '').replace(old_active + '\n\n', '').replace('ClearRequestedQueue();', 'AudioSoundQueue_ClearRequested(m_aRequestedOrderList[m_nActiveQueue], &m_nRequestedCount[m_nActiveQueue], m_nActiveSamples);').replace('ClearActiveSamples();', 'AudioSoundQueue_ClearActive(m_asActiveSamples, m_nActiveSamples);')
# Account for the separately audited scalar-method migration
for result, name in [('uint8','ComputeVolume'),('uint8','ComputeEmittingVolume'),('int32','ComputePan'),('int32','ComputeFrontRearMix'),('uint32','ComputeDopplerEffectedFrequency'),('int32','RandomDisplacement')]:
    expected_owner = expected_owner.replace(function(expected_owner, result+'\ncAudioManager::'+name)+'\n','')
table_start = expected_owner.index('Const static uint8 PanTable[64]')
table_end = expected_owner.index('\n\n', table_start)
expected_owner = expected_owner[:table_start]+expected_owner[table_end+2:]
assert normalize(mission.calls(entities.owner(init.owner(provider.owner(controls.owner(calls(expected_owner))))))) == normalize(current), 'Unexpected owner or startup sequence change' 
old_header = (out / 'before/src/audio/AudioManager.h').read_text()
expected_header = old_header.replace('\tvoid ClearRequestedQueue(); // inlined in vc\n', '').replace('\tvoid ClearActiveSamples();\n', '')
# Retain the header audit after the six scalar declarations move to their C header
expected_header = expected_header.replace('#include "AudioSound.h"', '#include "AudioSound.h"\n#include "AudioMath.h"')
for line in expected_header.splitlines():
    if any(re.search(r'\b'+name+r'\(',line) for name in ('ComputeVolume','ComputeEmittingVolume','ComputePan','ComputeFrontRearMix','ComputeDopplerEffectedFrequency','RandomDisplacement')):
        expected_header = expected_header.replace(line+'\n','')
assert normalize(data.header(mission.header(entities.header(init.header(provider.header(controls.header(expected_header))))))) == normalize((root / 'src/audio/AudioManager.h').read_text())

# Exercise the original vector-boundary helper with the real game coordinate constructor
vector = (root / 'src/math/Vector.h').read_text()
ctor = vector[vector.index('CVector(float x, float y, float z)'):vector.index('\n\tCVector(const RwV3d')]
(out / 'common.h').write_text('#pragma once\nstruct RwV3d {float x,y,z;};\nstruct CVector : RwV3d { CVector() {}\n' + ctor + '\n};\n', newline='\n')
header = (root / 'src/audio/AudioSound.h').read_text()
record = header[header.index('typedef struct tSound'):header.index('#ifdef __cplusplus')]
fields, seeds = [], []
for line in record.splitlines():
    if line.lstrip().startswith('#'):
        fields.append(line)
        seeds.append(line)
    field = re.match(r'\s*(int32_t|uint32_t|uint8_t|int8_t|AudioSoundPosition|float)\s+(\w+);', line)
    if field:
        type_name, name = field.groups()
        fields.append('record(&sound->' + name + ',sizeof(sound->' + name + '));')
        if type_name == 'AudioSoundPosition':
            seeds.append('sound->' + name + '.x = -0.0f; sound->' + name + '.y = -7.5f; sound->' + name + '.z = 91.25f;')
        else:
            seeds.append('sound->' + name + ' = (' + type_name + ')(seed + ' + str(len(seeds)) + ');')
template = (root / 'utils/tests/audio_sound_reset.cpp.in').read_text()
for variant in ('before', 'after'):
    source = template
    replacements = {'@METHODS@': old_queue + '\n' + old_active if variant == 'before' else '', '@CLEAR_QUEUE@': 'state.ClearRequestedQueue();' if variant == 'before' else 'AudioSoundQueue_ClearRequested(state.m_aRequestedOrderList[state.m_nActiveQueue], &state.m_nRequestedCount[state.m_nActiveQueue], state.m_nActiveSamples);', '@CLEAR_ACTIVE@': 'state.ClearActiveSamples();' if variant == 'before' else 'AudioSoundQueue_ClearActive(state.m_asActiveSamples, state.m_nActiveSamples);', '@C_CHECK@': '' if variant == 'before' else 'TestAudioSoundResetC();', '@FIELDS@': '\n'.join(fields), '@SEEDS@': '\n'.join(seeds)}
    for key, value in replacements.items():
        source = source.replace(key, value)
    (out / (variant + '.cpp')).write_text(source, newline='\n')
shapes = {'default': '', 'vanilla': '#undef FIX_BUGS\n', 'no-external': '#undef EXTERNAL_3D_SOUND\n', 'no-reflections': '#undef AUDIO_REFLECTIONS\n', 'ps2': '#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#undef AUDIO_REVERB\n#define GTA_PS2\n', 'ps2-reverb': '#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}
for name, flags in shapes.items():
    (out / (name + '.h')).write_text('#include "../../src/core/config.h"\n' + flags, newline='\n')
(out / 'caller.c').write_text('''#include "AudioSound.h"
#include "sampman.h"
#include <assert.h>
void TestAudioSoundResetC(void) {
    // Exercise live reset through C and verify retained record and order tail fields
    tSound active[2] = {0};
    uint8_t order[2] = {19,23};
    uint8_t count = 17;
    active[0].m_nFrontRearPan = 211;
    active[1].m_nSampleIndex = 123;
    AudioSoundQueue_ClearRequested(order, &count, 1);
    assert(count == 0 && order[0] == 1 && order[1] == 23);
    AudioSoundQueue_ClearActive(active, 1);
    assert(active[0].m_nSampleIndex == NO_SAMPLE && active[0].m_nFrontRearPan == 211);
    assert(active[0].m_vecPos.x == 0 && active[0].m_vecPos.y == 0 && active[0].m_vecPos.z == 0);
    assert(active[1].m_nSampleIndex == 123);
}
''', newline='\n')
print('Both reset bodies, all owner edits and constructor/RNG call ordering verified')
