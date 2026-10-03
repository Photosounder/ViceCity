"""Audit and compare the cached distance C helper with its saved C++ body."""
from pathlib import Path
import re
from audio_controls_migration import function
import audio_distance_migration as migration
root=Path(__file__).resolve().parents[2]
out=root/'build/audio-distance-c-tests'
old=(out/'before/src/audio/AudioLogic.cpp').read_text()
member=function(old,'cAudioManager::','CalculateDistance')
def normalize(source):
    # Compare statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
kernel=(root/'src/audio/AudioMath.c').read_text()
actual=function(kernel,'AudioMath_','CalculateDistance')
expected=member[member.index('{'):].replace('distCalculated','*calculated').replace('m_sQueueSample.m_fDistance','*distance').replace('Sqrt(dist)','squaredDistance <= 0.0f ? 0.0f : sqrtf(squaredDistance)').replace('TRUE','1')
assert normalize(expected)==normalize(actual[actual.index('{'):])
for name,transform in [('AudioLogic.cpp',migration.logic),('AudioCollision.cpp',migration.logic),('AudioManager.h',migration.header)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
header=(out/'before/src/audio/AudioManager.h').read_text()
wrapper='float Sqrt(float v) const { return v <= 0.0f ? 0.0f : ::Sqrt(v); }'
assert wrapper in header
assert 'inline float Sqrt(float x) { return sqrtf(x); }' in (root/'src/math/maths.h').read_text()
assert 'typedef uint8 bool8;' in (root/'src/core/common.h').read_text() or 'typedef uint8_t bool8;' in (root/'src/core/common.h').read_text()
template=(root/'utils/tests/audio_distance.cpp.in').read_text()
(out/'compare.cpp').write_text(template.replace('@MEMBER@',member).replace('@WRAPPER@',wrapper),newline='\n')
(out/'caller.c').write_text('#include "AudioMath.h"\nvoid TestDistanceFromC(uint8_t *calculated, float *distance, float input)\n{\n    // Exercise the public declaration and production helper through a C caller\n    AudioMath_CalculateDistance(calculated, distance, input);\n}\n',newline='\n')
print('Original clamp, complete helper body, and all 34 caller edits audited')
