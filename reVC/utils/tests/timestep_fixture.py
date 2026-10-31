"""Audit complete global conversion and exercise the actual 12-byte save fragment."""
from pathlib import Path
import json,re
import timestep_migration as m

root=Path(__file__).resolve().parents[2];out=root/'build/timestep-c-tests'

def normalize(source):
    # Compare full statements apart from comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text()
    if path=='src/core/TimeStep.cpp':
        target=root/'src/core/TimeStep.c';expected=m.kernel(original)
        assert not (root/path).exists()
    else:
        target=root/path;expected=m.project(original) if path.startswith('build/') else m.calls(original)
    assert normalize(expected)==normalize(target.read_text()),path
    assert b'\r' not in target.read_bytes(),path
assert re.findall(r'float \w+ = ([^;]+);',(root/'src/core/TimeStep.c').read_text())==['1.0f']*3
original=(out/'before/src/save/GenericGameStorage.cpp').read_text()
card=(out/'before/src/save/MemoryCard.cpp').read_text()
macros='\n'.join(line for line in original.splitlines() if line.startswith(('#define ReadDataFromBufferPointer(','#define WriteDataToBufferPointer(')))
assert all(line in card for line in macros.splitlines())
save='\n'.join(re.findall(r'^\s*WriteDataToBufferPointer\(buf, CTimeStep::\w+\);',original,re.M))
load='\n'.join(re.findall(r'^\s*ReadDataFromBufferPointer\(buf, CTimeStep::\w+\);',original,re.M))
assert len(save.splitlines())==len(load.splitlines())==3
assert [m.FIELDS[name] for name in re.findall(r'CTimeStep::(\w+)',save)]==['TimeStep_timeStep','TimeStep_framesPerUpdate','TimeStep_timeScale']
align=re.search(r'int\nalign4bytes\(int32 size\)\n\{[^}]+\}',original)[0].replace('int32','int32_t')
align=align.replace('{\n','{\n    // Preserve the original four-byte cursor alignment\n',1)
declarations='''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
float *TestTimeStep_Field(uint32_t slot);
uint32_t TestTimeStep_Save(uint8_t *buffer);
uint32_t TestTimeStep_Load(uint8_t *buffer);
#ifdef __cplusplus
}
#endif
'''
(out/'caller.h').write_text(declarations,newline='\n')
wrapper='''float *TestTimeStep_Field(uint32_t slot)
{
    // Expose the actual lvalue storage across the C and C++ boundary
    switch(slot){case 0:return &CTimeStep::ms_fTimeStep;case 1:return &CTimeStep::ms_fFramesPerUpdate;default:return &CTimeStep::ms_fTimeScale;}
}
uint32_t TestTimeStep_Save(uint8_t *buffer)
{
    // Run the unchanged actual save fragment against an unaligned buffer
    uint8_t *buf=buffer;
@SAVE@
    return (uint32_t)(buf-buffer);
}
uint32_t TestTimeStep_Load(uint8_t *buffer)
{
    // Run the unchanged actual load fragment into the same live storage
    uint8_t *buf=buffer;
@LOAD@
    return (uint32_t)(buf-buffer);
}
'''.replace('@SAVE@',save).replace('@LOAD@',load)
(out/'caller.c').write_text('#include "TimeStep.h"\n#include "caller.h"\n#include <string.h>\n'+macros+'\n'+align+'\n'+m.calls(wrapper),newline='\n')
template=(root/'utils/tests/timestep.cpp.in').read_text()
oldowner=(out/'before/src/core/TimeStep.h').read_text()+'\n'+(out/'before/src/core/TimeStep.cpp').read_text().replace('#include "TimeStep.h"','')
for variant in ['before','after']:
    owner=oldowner+'\n'+macros+'\n'+align+'\nextern "C" {\n'+wrapper+'\n}\n' if variant=='before' else '#include "TimeStep.h"'
    source=template.replace('@OWNER@',owner)
    (out/(variant+'.cpp')).write_text(source,newline='\n')
print('Complete CTimeStep removal, three initialized float globals, both save-path field orders, four consumers and unit/project rename audited')
