"""Compare the original pedestrian-comment queue with the actual C module."""
from pathlib import Path
import re
import audio_mission_state_migration as mission
import audio_owner_entities_migration as entities
import audio_owner_init_migration as init
import audio_provider_migration as provider
from audio_math_migration import calls

root = Path(__file__).resolve().parents[2]
out = root / 'build/audio-ped-comments-c-tests'
old_header = (out / 'before/src/audio/AudioManager.h').read_text()
old_logic = (out / 'before/src/audio/AudioLogic.cpp').read_text()
kernel = (root / 'src/audio/AudioPedComments.c').read_text()

def function(source, signature):
    # Extract a balanced function without conditional alternate opening braces
    start = source.index('void\n' + signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

def normalize(source):
    # Ignore comments and formatting when checking statement equivalence
    return re.sub(r'\s+', '', re.sub(r'//[^\n]*|/\*.*?\*/', '', source, flags=re.S))

def qualify(source):
    # Replace implicit class state with an explicit C state pointer
    for name in ('m_aPedCommentQueue', 'm_aPedCommentOrderList', 'm_nPedCommentCount', 'm_nActiveQueue', 'm_bDelay', 'm_nDelayTimer'):
        source = re.sub(r'\b' + name + r'\b', 'comments->' + name, source)
    return source.replace('uint8 ', 'uint8_t ')

old_add = function(old_logic, 'cPedComments::Add')
new_add = function(kernel, 'PedComments_Add')
assert normalize(qualify(old_add[old_add.index('{'):])) == normalize(new_add[new_add.index('{'):])
old_process = old_logic[old_logic.index('void\ncPedComments::Process()'):old_logic.index('\n#undef cooldown_phrase')].rstrip()
tail = old_process[old_process.index('\t// Switch queue'):old_process.index('#if defined(GTA_PC) && !defined(FIX_BUGS)', old_process.index('\t// Switch queue'))]
new_advance = function(kernel, 'PedComments_Advance')
expected = '{ uint8_t queue; ' + qualify(tail).replace('Add(&', 'PedComments_Add(comments, &') + '}'
assert normalize(expected) == normalize(new_advance[new_advance.index('{'):])
record_types = old_header[old_header.index('class tPedComment'):old_header.index('#define MISSION_AUDIO_SLOTS')]
ctor = record_types[record_types.index('\tcPedComments()'):record_types.index('\tvoid Add')]
expected_init = qualify(ctor[ctor.index('{'):])
new_init = function(kernel, 'PedComments_Init')
assert normalize(expected_init) == normalize(new_init[new_init.index('{'):])

# Verify the entire playback adapter after only state and vector-boundary substitutions
expected_process = old_process.replace('cPedComments::Process()', 'PedComments_Process(cPedComments *comments)').replace('\tuint8 queue;\n', '')
expected_process = expected_process.replace(tail, '\tPedComments_Advance(comments);\n')
expected_process = qualify(expected_process)
position = '\t\t\tAudioManager.m_sQueueSample.m_vecPos = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_vecPos;'
expected_process = expected_process.replace(position, '\n'.join(position[:-1].replace('m_vecPos =', 'm_vecPos.' + axis + ' =') + '.' + axis + ';' for axis in 'xyz'))
current_logic = (root / 'src/audio/AudioLogic.cpp').read_text()
current_process = current_logic[current_logic.index('void\nPedComments_Process('):current_logic.index('\n#undef cooldown_phrase')].rstrip()
assert normalize(calls(expected_process)) == normalize(current_process.replace("uint8 ", "uint8_t ")), 'Playback decisions changed'

# Verify all game callers after the intended queue and vector-boundary substitutions
expected_logic = old_logic.replace(old_add + '\n\n', '').replace(old_process, current_process).replace('m_sPedComments.Add(&pedComment);', 'PedComments_Add(&m_sPedComments, &pedComment);')
for oldline, indent, expr in [ ('\t\t\tpedComment.m_vecPos = m_sQueueSample.m_vecPos;', '\t\t\t', 'm_sQueueSample.m_vecPos'), ('\tpedComment.m_vecPos = CWorld::Players[0].m_pPed->GetPosition();', '\t', 'CWorld::Players[0].m_pPed->GetPosition()') ]:
    replacement = indent + 'const CVector &commentPosition = ' + expr + ';\n' + '\n'.join(indent + 'pedComment.m_vecPos.' + axis + ' = commentPosition.' + axis + ';' for axis in 'xyz')
    expected_logic = expected_logic.replace(oldline, replacement)
for signature in ('cAudioManager::SetupPedComments', 'cAudioManager::DebugPlayPedComment'):
    # Check pedestrian producers after the separately verified sound-position copy boundary
    current_producer = function(current_logic, signature).replace('AudioSoundPosition_ToVector(&m_sQueueSample.m_vecPos)', 'm_sQueueSample.m_vecPos')
    assert normalize(calls(function(expected_logic, signature))) == normalize(current_producer), 'Unexpected pedestrian integration change'
old_manager = (out / 'before/src/audio/AudioManager.cpp').read_text()
expected_manager = old_manager.replace('AudioCrimes_Init(m_aCrimes);', 'AudioCrimes_Init(m_aCrimes);\nPedComments_Init(&m_sPedComments);').replace('m_sPedComments.Process();', 'PedComments_Process(&m_sPedComments);')
# Account for the separately verified C queue reset operations at the same owner call sites
expected_manager = expected_manager.replace('ClearRequestedQueue();', 'AudioSoundQueue_ClearRequested(m_aRequestedOrderList[m_nActiveQueue], &m_nRequestedCount[m_nActiveQueue], m_nActiveSamples);').replace('ClearActiveSamples();', 'AudioSoundQueue_ClearActive(m_asActiveSamples, m_nActiveSamples);')
# Account for the separately audited owner initializer at the same startup point
expected_manager = expected_manager.replace(init.function(expected_manager, 'cAudioManager::', 'cAudioManager'), init.CTOR)
current_manager = (root / 'src/audio/AudioManager.cpp').read_text()
for signature, next_signature in [('cAudioManager::cAudioManager()', 'cAudioManager::~cAudioManager()'), ('cAudioManager::ServiceSoundEffects()', '\nvoid\n')]:
    # Audit the owning lifecycle and service callers while allowing other subsystems to migrate
    expected_start = expected_manager.index(signature)
    if signature == 'cAudioManager::ServiceSoundEffects()':
        # Follow the independently audited service conversion into the actual C body
        import audio_effects_migration as effects
        expected_service = mission.calls(entities.calls(provider.calls(calls(function(expected_manager, signature)))))
        current_service = function((root / 'src/audio/AudioEffects.c').read_text(), 'AudioEffects_Service(')
        assert normalize(effects.body(expected_service)) == normalize(current_service[current_service.index('{'):])
        continue
    current_start = current_manager.index(signature)
    expected_end = expected_manager.index(next_signature, expected_start + len(signature))
    current_end = current_manager.index(next_signature, current_start + len(signature))
    assert normalize(calls(expected_manager[expected_start:expected_end])) == normalize(current_manager[current_start:current_end])

for shape, flags in {'default': '', 'vanilla': '#undef FIX_BUGS\n', 'no-external': '#undef EXTERNAL_3D_SOUND\n', 'ps2': '#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out / (shape + '.h')).write_text('#include "config.h"\n' + flags, newline='\n')

template = (root / 'utils/tests/audio_ped_comments.cpp.in').read_text()
for variant in ('before', 'after'):
    headers = '#include "config.h"\n'
    methods = ''
    if variant == 'before':
        headers += '''using uint32 = uint32_t; using int32 = int32_t; using uint8 = uint8_t; using int8 = int8_t; using bool8 = uint8_t;
struct CVector {float x,y,z; CVector() {}};
#define VALIDATE_SIZE(type,size)
''' + record_types
        methods = old_add + '\nvoid cPedComments::Process() { uint8 queue;\n' + tail + '}\n'
        init, add, advance = 'new (&state) cPedComments', 'state.Add(&comment)', 'state.Process()'
    else:
        headers += '#include "AudioPedComments.h"\n#include <type_traits>\nstatic_assert(std::is_trivial<cPedComments>::value, "explicit lifecycle");\nextern "C" void TestPedCommentsC(void);\n'
        init, add, advance = 'PedComments_Init(&state)', 'PedComments_Add(&state, &comment)', 'PedComments_Advance(&state)'
    source = template
    for key, value in {'@HEADERS@': headers, '@METHODS@': methods, '@INIT@': init, '@ADD@': add, '@ADVANCE@': advance, '@C_CHECK@': '' if variant == 'before' else 'TestPedCommentsC();'}.items():
        source = source.replace(key, value)
    (out / (variant + '.cpp')).write_text(source, newline='\n')
(out / 'caller.c').write_text('''#include "AudioPedComments.h"
#include <assert.h>
void TestPedCommentsC(void) {
    // Exercise the real queue module through C declarations
    cPedComments state;
    tPedComment comment = {0};
    PedComments_Init(&state);
    comment.m_nVolume = 127;
    comment.m_nLoadingTimeout = 1;
    PedComments_Add(&state, &comment);
    PedComments_Advance(&state);
    assert(state.m_nActiveQueue == 1 && state.m_nPedCommentCount[0] == 0);
    assert(state.m_nPedCommentCount[1] == 1 && state.m_aPedCommentQueue[1][0].m_nLoadingTimeout == 0);
    PedComments_Advance(&state);
    assert(state.m_nActiveQueue == 0 && state.m_nPedCommentCount[0] == 0 && state.m_nPedCommentCount[1] == 0);
}
''', newline='\n')
print('Initialization, insertion, carryover, and full playback adapter statements verified')
