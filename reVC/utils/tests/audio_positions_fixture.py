"""Compare mission storage and the real reflection adapters against saved sources."""
from pathlib import Path
import re
import audio_reflections_migration as reflections
import audio_requests_migration as requests
import audio_mission_data_migration as data
import audio_mission_state_migration as mission
import audio_owner_entities_migration as entities
from audio_owner_init_migration import fields
from audio_math_migration import calls

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-positions-c-tests'
old_manager = (out / 'before/src/audio/AudioManager.cpp').read_text()
old_logic = (out / 'before/src/audio/AudioLogic.cpp').read_text()
manager = (root / 'src/audio/AudioManager.cpp').read_text()
logic = (root / 'src/audio/AudioLogic.cpp').read_text()

def function(source, signature):
    # Extract each production adapter verbatim with its conditional branches
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

def normalize(source):
    # Audit statements independently of formatting and explanatory comments
    return re.sub(r'\s+', '', re.sub(r'//[^\n]*|/\*.*?\*/', '', source, flags=re.S))

expected_manager = old_manager.replace('AudioSoundPosition_FromVector(m_avecReflectionsPos[i])', 'm_avecReflectionsPos[i]')
expected_manager = re.sub(r'm_avecReflectionsPos\[(\d+)\] = camPos;', r'm_avecReflectionsPos[\1] = AudioSoundPosition_FromVector(camPos);', expected_manager)
expected_manager = re.sub(r'(ProcessLineOfSight\(camPos, )(m_avecReflectionsPos\[\d+\])(,)', r'\1AudioSoundPosition_ToVector(&\2)\3', expected_manager)
for signature in ('void\ncAudioManager::UpdateReflections', 'void\ncAudioManager::AddReflectionsToRequestedQueue'):
    # Audit the converted position consumers independently of unrelated queue resets
    if 'AddReflectionsToRequestedQueue' in signature:
        expected=requests.body(calls(function(expected_manager,signature))).replace('AudioRequests_Submit(this)','AudioRequests_Submit(manager)')
        actual=function((root/'src/audio/AudioRequests.c').read_text(),'void\nAudioRequests_AddReflections')
        assert normalize(expected)==normalize(actual[actual.index('{'):])
    else:
        expected=reflections.body(function(expected_manager,signature))
        actual=function((root/'src/audio/AudioReflections.c').read_text(),'void\nAudioReflections_Update')
        assert normalize(expected)==normalize(actual[actual.index('{'):])
old_set = function(old_logic, 'void\ncAudioManager::SetMissionAudioLocation')
# Retain the saved thin setter only to drive the already converted coordinate kernel
new_set = function((root/'build/audio-mission-state-c-tests/before/src/audio/AudioLogic.cpp').read_text(), 'void\ncAudioManager::SetMissionAudioLocation')
expected_set = old_set[:old_set.index('{')] + '{AudioMissionPosition_Set(m_vecMissionAudioPosition, m_bIsMissionAudio2D, m_bIsInitialised, slot, x, y, z);}'
assert normalize(expected_set) == normalize(new_set)
expected_logic = old_logic.replace(old_set, new_set).replace('GetDistanceSquared(m_vecMissionAudioPosition[slot])', 'GetDistanceSquared(AudioSoundPosition_ToVector(&m_vecMissionAudioPosition[slot]))')
expected_logic = re.sub(r'(\t+)TranslateEntity\(&m_vecMissionAudioPosition\[slot\], &vec\);', r'\1const CVector missionPosition = AudioSoundPosition_ToVector(&m_vecMissionAudioPosition[slot]);\n\1TranslateEntity(&missionPosition, &vec);', expected_logic)
assert normalize(data.logic(mission.logic(entities.calls(calls(expected_logic))))) == normalize(logic)
old_header = (out / 'before/src/audio/AudioManager.h').read_text()
expected_header = old_header.replace('#define MISSION_AUDIO_SLOTS (2)', '#include "AudioMissionPosition.h"').replace('CVector m_avecReflectionsPos', 'AudioSoundPosition m_avecReflectionsPos').replace('CVector m_vecMissionAudioPosition', 'AudioSoundPosition m_vecMissionAudioPosition')
header = (root / 'src/audio/AudioManager.h').read_text()
assert normalize(fields(expected_header)) == normalize(fields(header))
state_fields = fields(header)
assert not re.search(r'\bCVector\b', state_fields), 'Embedded C++ vector remains'

# Use the actual game's coordinate constructor to test both storage representations
vector = (root / 'src/math/Vector.h').read_text()
ctor = vector[vector.index('CVector(float x, float y, float z)'):vector.index('\n\tCVector(const RwV3d')]
(out / 'common.h').write_text('#pragma once\nstruct RwV3d {float x,y,z;};\nstruct CVector : RwV3d { CVector() {}\n' + ctor + '\n};\n', newline='\n')
template = (root / 'utils/tests/audio_positions.cpp.in').read_text()
for variant in ('before', 'after'):
    # Retain the saved C++ driver for the already converted position records; audit the live C scheduler above
    source = old_manager if variant == 'before' else (root/'build/audio-reflections-c-tests/before/src/audio/AudioManager.cpp').read_text()
    methods = function(source, 'void\ncAudioManager::UpdateReflections') + '\n' + (old_set if variant == 'before' else new_set)
    replacements = {'@POSITION_TYPE@': 'CVector' if variant == 'before' else 'AudioSoundPosition', '@METHODS@': methods, '@C_CHECK@': '' if variant == 'before' else 'TestAudioMissionPositionC();', '@HEADERS@': '#include "common.h"\n' if variant == 'before' else '#include "AudioSoundGame.h"\n#include "AudioMissionPosition.h"\nextern "C" void TestAudioMissionPositionC(void);\n'}
    generated = template
    for key, value in replacements.items():
        generated = generated.replace(key, value)
    (out / (variant + '.cpp')).write_text(generated, newline='\n')
for version, value in {'current': 'GTAVC_PC_11', 'early': 'GTAVC_PS2'}.items():
    (out / (version + '.h')).write_text('#include "../../src/core/config.h"\n#undef GTA_VERSION\n#define GTA_VERSION ' + value + '\n', newline='\n')
(out / 'caller.c').write_text('''#include "AudioMissionPosition.h"
#include <assert.h>
void TestAudioMissionPositionC(void) {
    // Exercise valid and invalid mission storage updates from an actual C caller
    AudioSoundPosition positions[MISSION_AUDIO_SLOTS] = {{1,2,3},{4,5,6}};
    uint8_t is2D[MISSION_AUDIO_SLOTS] = {7,9};
    AudioMissionPosition_Set(positions, is2D, 0, 0, 11,12,13);
    assert(positions[0].x == 1 && is2D[0] == 7);
    AudioMissionPosition_Set(positions, is2D, 1, 2, 11,12,13);
    assert(positions[1].z == 6 && is2D[1] == 9);
    AudioMissionPosition_Set(positions, is2D, 1, 1, 11,12,13);
    assert(positions[1].x == 11 && positions[1].y == 12 && positions[1].z == 13 && is2D[1] == 0);
}
''', newline='\n')
# Emit native ABI constants for the complete real owner without linking game dependencies
abi_entries = []
for line in state_fields.splitlines():
    if line.lstrip().startswith('#'):
        abi_entries.append(line)
    field = re.match(r'\s*[A-Za-z_][A-Za-z_0-9]*\s+(m_[A-Za-z_0-9]+|field_[A-Za-z_0-9]+)(?:\[[^;]+)?;', line)
    if field:
        name = field.group(1)
        abi_entries.append('offsetof(cAudioManager,' + name + '),sizeof(((cAudioManager *)0)->' + name + '),alignof(decltype(((cAudioManager *)0)->' + name + ')),')
for variant in ('before', 'after'):
    owner = out / 'before/src/audio/AudioManager.h' if variant == 'before' else root / 'src/audio/AudioManager.h'
    abi = '#include "' + (root / 'src/core/common.h').as_posix() + '"\n#include "' + owner.as_posix() + '"\n#include <stddef.h>\nextern "C" const size_t AudioManagerAbi[] = {sizeof(cAudioManager),alignof(cAudioManager),\n' + '\n'.join(abi_entries) + '\n};\nextern "C" const size_t AudioManagerAbiCount = sizeof(AudioManagerAbi)/sizeof(AudioManagerAbi[0]);\n'
    (out / (variant + '-owner-abi.cpp')).write_text(abi, newline='\n')

print('Complete position storage, reflection, mission geometry, and header edits audited')
