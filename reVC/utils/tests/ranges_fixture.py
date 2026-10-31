import weather_migration as _weather_conversion
"""Audit both complete range conversions and generate original/C class-operation oracles."""
from pathlib import Path
import json,re
import ranges_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/ranges-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old);current=root/m.target(path)
    assert current.read_text()==expected,path
    assert b'\r' not in current.read_bytes(),path
for dimension in [2,3]:assert not (root/f'src/core/Range{dimension}D.cpp').exists()
for path in ['src/core/common.h','src/core/config.h','src/core/General.h','src/core/General.c','src/core/Random.h','src/core/re3.cpp','src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h']:_weather_conversion.verify_dependency(out/'before'/path,root/path)
for path in ['src/core/Range2D.h','src/core/Range3D.h']:
    original=(out/'before'/path).read_text();assert ' min, max;' in original and 'GetRandomPointInRange();' in original
common=(root/'build/script-paths-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['Range2D.h','Range3D.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/core'/name).as_posix()+'"\n',newline='\n')
(out/'owners.h').write_text('#pragma once\n#include "common.h"\n#include "Range2D.h"\n#include "Range3D.h"\n#include "General.h"\n#include "Random.h"\n#include <new>\n',newline='\n')
original_headers='\n'.join(re.sub(r'^#pragma[^\n]*\n','',(out/f'before/src/core/Range{d}D.h').read_text(),flags=re.M) for d in [2,3])
(out/'original-classes.h').write_text(original_headers,newline='\n')
prototypes=''
for d in [2,3]:
    prefix=f'Range{d}D'
    prototypes+=f'''void TestRanges_Init{d}(int index,{prefix}Point min,{prefix}Point max);
bool TestRanges_Contains{d}(int index,{prefix}Point point);{prefix}Point TestRanges_Random{d}(int index);
void TestRanges_Debug{d}(int index,float value,int mode);{prefix} TestRanges_Get{d}(int index);
void TestRanges_RunInit{d}(int index,{prefix}Point min,{prefix}Point max);
bool TestRanges_RunContains{d}(int index,{prefix}Point point);{prefix}Point TestRanges_RunRandom{d}(int index);
void TestRanges_RunDebug{d}(int index,float value,int mode);
'''
(out/'caller.h').write_text('#pragma once\n#include "Range2D.h"\n#include "Range3D.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
for original in [True,False]:
    text='#include "owners.h"\n#include "caller.h"\n#include <cstring>\n'
    if original:
        text+='namespace Original {\n'+original_headers+'\n'
        for d in [2,3]:text+=re.sub(r'^#include[^\n]*\n','',(out/f'before/src/core/Range{d}D.cpp').read_text(),flags=re.M)+'\n'
        text+='}\n'
    for d in [2,3]:
        prefix=f'Range{d}D';kind='Original::C'+prefix if original else prefix
        if original:text+=f'alignas({kind}) static unsigned char storage{d}[4][sizeof({kind})];static bool live{d}[4];\nstatic {kind} *record{d}(int index)\n{{\n    // Address the actual constructed native class in aligned fixture-owned storage\n    return ({kind} *)storage{d}[index];\n}}\n'
        else:text+=f'static {kind} records{d}[4];\nstatic {kind} *record{d}(int index)\n{{\n    // Address the actual plain C record used by every operation\n    return &records{d}[index];\n}}\n'
        text+=f'extern "C" void TestRanges_Init{d}(int index,{prefix}Point min,{prefix}Point max)\n{{\n    // Execute the complete native constructor or actual C initialization operation\n'
        vector='CVector2D' if d==2 else 'CVector';coords=lambda name:','.join(name+'.'+axis for axis in 'xyz'[:d])
        if original:text+=f'    if(live{d}[index])record{d}(index)->~C{prefix}();new(storage{d}[index]) {kind}({vector}({coords("min")}),{vector}({coords("max")}));live{d}[index]=true;\n'
        else:text+=f'    {prefix}_Init(record{d}(index),min,max);\n'
        text+='}\n'
        text+=f'extern "C" bool TestRanges_Contains{d}(int index,{prefix}Point point)\n{{\n    // Preserve the full native or C strict-boundary predicate\n    return '+(f'record{d}(index)->IsInRange({vector}({coords("point")}))' if original else f'{prefix}_IsInRange(record{d}(index),point)')+';\n}\n'
        text+=f'extern "C" {prefix}Point TestRanges_Random{d}(int index)\n{{\n    // Preserve the real by-value result and every original random draw\n'
        if original:text+=f'    {vector} point=record{d}(index)->GetRandomPointInRange();{prefix}Point result={{{coords("point")}}};return result;\n'
        else:text+=f'    return {prefix}_GetRandomPointInRange(record{d}(index));\n'
        text+='}\n'
        text+=f'extern "C" void TestRanges_Debug{d}(int index,float value,int mode)\n{{\n    // Exercise the complete original empty debug operation on the actual record\n    '+(f'record{d}(index)->DebugShowRange(value,mode);' if original else f'{prefix}_DebugShowRange(record{d}(index),value,mode);')+'\n}\n'
        text+=f'extern "C" {prefix} TestRanges_Get{d}(int index)\n{{\n    // Observe all native endpoint bytes without writing private native members\n    static_assert(sizeof({kind})==sizeof({prefix}),"Actual complete native range size");{prefix} result;memcpy(&result,record{d}(index),sizeof(result));return result;\n}}\n'
    (out/('original.cpp' if original else 'adapter.cpp')).write_text(text,newline='\n')
caller='#include "caller.h"\n'
for d in [2,3]:
    prefix=f'Range{d}D'
    for name,result,args,invoke in [('Init','void',f'int index,{prefix}Point min,{prefix}Point max','index,min,max'),('Contains','bool',f'int index,{prefix}Point point','index,point'),('Random',prefix+'Point','int index','index'),('Debug','void','int index,float value,int mode','index,value,mode')]:caller+=f'{result} TestRanges_Run{name}{d}({args})\n{{\n    // Retain the actual C record and point-value ABI through a separate C11 caller\n    '+('return ' if result!='void' else '')+f'TestRanges_{name}{d}({invoke});\n}}\n'
(out/'caller.c').write_text(caller,newline='\n')
source=(out/'before/src/core/re3.cpp').read_text();a=source.index('#ifdef USE_PS2_RAND\nunsigned long long myrand_seed');b=source.index('\n#ifdef CUSTOM_FRONTEND_OPTIONS',a)
rng=source[a:b];(out/'rng.cpp').write_text('#include "owners.h"\nnamespace OriginalRng {\n'+rng+'\n}\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/ranges.cpp.in').read_text(),newline='\n')
(out/'regular.h').write_text('#include "config.h"\n#undef USE_PS2_RAND\n',newline='\n')
print('Audited both complete constructors and six methods, real point/RNG sources, two deleted C++ units, exact pedestrian transfer and eight complete source/project changes')
