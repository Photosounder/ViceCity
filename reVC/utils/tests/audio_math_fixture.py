"""Compare actual scalar kernels and random history with the saved C++ methods."""
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
out = root / 'build/audio-math-c-tests'
old = (out / 'before/src/audio/AudioManager.cpp').read_text()
kernel = (root / 'src/audio/AudioMath.c').read_text()
methods = [('uint8','ComputeVolume'),('uint8','ComputeEmittingVolume'),('int32','ComputePan'),('int32','ComputeFrontRearMix'),('uint32','ComputeDopplerEffectedFrequency'),('int32','RandomDisplacement')]

def function(source, signature):
    # Extract the original implementation without reconstructing its arithmetic
    start = source.index(signature)
    brace = source.index('{', start)
    end, depth = brace + 1, 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

def normalize(source):
    # Check statement equivalence independently of comments and formatting
    return re.sub(r'\s+', '', re.sub(r'//[^\n]*|/\*.*?\*/', '', source, flags=re.S))

original = {name: function(old, ret+'\ncAudioManager::'+name) for ret,name in methods}
for name, source in original.items():
    expected = source[source.index('{'):]
    for before, after in [('uint8','uint8_t'),('int8','int8_t'),('int32','int32_t'),('uint32','uint32_t'),('bool8','uint8_t'),('TRUE','1'),('Min','AUDIO_MIN'),('Max','AUDIO_MAX'),('SQR','AUDIO_SQUARE'),('ABS','AUDIO_ABS_INT'),('Clamp2','AUDIO_CLAMP_CENTER'),('Abs','fabsf')]:
        expected = re.sub(r'\b'+before+r'\b',after,expected)
    expected = expected.replace('vec->x','x').replace('vec->y','y').replace('TheCamera.Get_Just_Switched_Status()','cameraSwitched').replace('m_nTimeSpent','timeSpent').replace('m_fSpeedOfSound','speedOfSound').replace('m_anRandomTable','randomTable')
    actual = function(kernel, 'AudioMath_' + ('ComputeDopplerFrequency' if name=='ComputeDopplerEffectedFrequency' else name) + '(')
    assert normalize(expected) == normalize(actual[actual.index('{'):]), name
start = old.index('Const static uint8 PanTable[64]')
end = old.index('\n\n', start)
table = old[start:end]
assert normalize(table.replace('Const static uint8','static const uint8_t')) == normalize(kernel[kernel.index('static const uint8_t PanTable'):kernel.index('\n\n',kernel.index('static const uint8_t PanTable'))])
common = (root / 'src/core/common.h').read_text()
macros = ''
for name in ('Min','Max','SQR','ABS','Clamp2'):
    line = next(line for line in common.splitlines() if line.startswith('#define '+name+'('))
    macros += line+'\n'
    renamed = line
    for before, after in [('Min','AUDIO_MIN'),('Max','AUDIO_MAX'),('SQR','AUDIO_SQUARE'),('ABS','AUDIO_ABS_INT'),('Clamp2','AUDIO_CLAMP_CENTER')]:
        renamed = re.sub(r'\b'+before+r'\b',after,renamed)
    assert renamed in kernel
assert 'inline float Abs(float x) { return fabsf(x); }' in (root/'src/math/maths.h').read_text()


for name in ('AudioManager.cpp','AudioLogic.cpp','AudioCollision.cpp','MusicManager.cpp'):
    saved = (out/'before/src/audio'/name).read_text()
    if name=='AudioManager.cpp':
        for method in original.values(): saved = saved.replace(method+'\n','')
        saved = saved.replace(table+'\n\n','')
    assert normalize(mission.calls(entities.owner(init.owner(provider.owner(controls.owner(calls(saved)))))) if name=='AudioManager.cpp' else data.logic(mission.logic(entities.calls(calls(saved)))) if name=='AudioLogic.cpp' else mission.calls(entities.calls(calls(saved)))) == normalize((root/'src/audio'/name).read_text()), name
header = (out/'before/src/audio/AudioManager.h').read_text().replace('#include "AudioSound.h"','#include "AudioSound.h"\n#include "AudioMath.h"')
for line in header.splitlines():
    if any(re.search(r'\b'+name+r'\(',line) for _,name in methods): header=header.replace(line+'\n','')
assert normalize(data.header(mission.header(entities.header(init.header(provider.header(controls.header(header))))))) == normalize((root/'src/audio/AudioManager.h').read_text())
template = (root/'utils/tests/audio_math.cpp.in').read_text()
for variant in ('before','after'):
    source = template.replace('@METHODS@', macros+'\ninline float Abs(float x) { return fabsf(x); }\n'+table+'\n'+'\n'.join(original.values()) if variant=='before' else '')
    replacements = {'@VOLUME@': 'state.ComputeVolume' if variant=='before' else 'AudioMath_ComputeVolume', '@EMITTING@': 'state.ComputeEmittingVolume' if variant=='before' else 'AudioMath_ComputeEmittingVolume', '@PAN@': 'state.ComputePan(dist,&position)' if variant=='before' else 'AudioMath_ComputePan(dist,position.x)', '@FRONT@': 'state.ComputeFrontRearMix(dist,&position)' if variant=='before' else 'AudioMath_ComputeFrontRearMix(dist,position.y)', '@DOPPLER@': 'state.ComputeDopplerEffectedFrequency' if variant=='before' else 'AudioMath_ComputeDopplerFrequency(TheCamera.Get_Just_Switched_Status(),state.m_nTimeSpent,state.m_fSpeedOfSound,', '@RANDOM@': 'state.RandomDisplacement(seed)' if variant=='before' else 'AudioMath_RandomDisplacement(state.m_anRandomTable,seed)', '@C_CHECK@': '' if variant=='before' else 'TestAudioMathC();'}
    for key,value in replacements.items(): source=source.replace(key,value)
    if variant=='after': source=source.replace('state.m_fSpeedOfSound,(', 'state.m_fSpeedOfSound,')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioMath.h"
#include <assert.h>
void TestAudioMathC(void) {
    // Link every scalar export from C without consuming random history
    const int32_t table[5] = {1,2,3,4,5};
    assert(AudioMath_ComputeVolume(99,0,0)==0);
    assert(AudioMath_ComputeEmittingVolume(99,100,0)==99);
    assert(AudioMath_ComputePan(64,32)==20);
    assert(AudioMath_ComputeFrontRearMix(64,-64)==126);
    assert(AudioMath_ComputeDopplerFrequency(1,40,8.575f,44100,0,1,1)==44100);
    assert(AudioMath_RandomDisplacement(table,0)==0);
}
''',newline='\n')
print('All six kernels, pan table, macros and complete game caller edits audited')
