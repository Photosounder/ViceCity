"""Audit the complete controller relocation and generate independent original/C comparisons."""
from pathlib import Path
import json,re
import escalators_migration as m
import timecycle_migration as cycle
import escalator_record_migration as records
root=Path(__file__).resolve().parents[2];out=root/'build/escalators-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('Escalators.h',m.HEADER),('Escalators_backend.h',m.BACKEND_HEADER),('Escalators.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
for path in ['src/core/common.h','src/control/Replay.h','src/math/Vector.h','src/math/Matrix.h','src/math/Matrix.cpp','src/objects/Object.h','src/objects/Object.cpp','src/core/config.h']:cycle.verify_dependency(out/'before'/path,root/path)
record_original=(root/'build/escalator-record-c-tests/before/src/renderer/Fluff.cpp').read_text()
assert m.record(header)==records.record((root/'build/escalator-record-c-tests/before/src/renderer/Fluff.h').read_text())
def record_method(source,name):
    # Hold every complete native record operation unchanged through the controller conversion
    match=re.search(r'(?m)^(?:void\s+)?CEscalator::'+name+r'\([^\n]*\)\s*\{',source);assert match,name
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]
for name in ['CEscalator','AddThisOne','Update','SwitchOff']:assert record_method(original,name)==record_method(record_original,name),name
assert (root/'src/renderer/Escalator.c').read_text()==records.kernel(record_original)
common=(root/'build/moving-things-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['Escalators.h','Escalators_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
(out/'owners.h').write_text('''#pragma once
#include "Escalators.h"
class CReplay { public:static bool IsPlayingBack(void); };
class CEscalator { public:
int32_t identity;CVector positions[4],midpoint;bool active,movingDown;uint32_t updates,offs,adds;
CEscalator();
void AddThisOne(CVector,CVector,CVector,CVector,bool);
void Update(void);void SwitchOff(void);
bool IsActive(void)const{return active;}
const CVector &GetPosition(void)const{return midpoint;}
};
''',newline='\n')
prototypes='''int32_t TestEs_Count(void);void TestEs_SetCount(int32_t value);
void TestEs_Init(void);void TestEs_Update(void);void TestEs_Shutdown(void);
void TestEs_Add(EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown);
const void *TestEs_Owner(int32_t index);
void TestEs_RunInit(void);void TestEs_RunUpdate(void);void TestEs_RunShutdown(void);
void TestEs_RunAdd(EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown);
'''
(out/'caller.h').write_text('#pragma once\n#include "Escalators.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
classes=m.controller(header).replace('class CEscalators\n{','class CEscalators\n{\npublic:');(out/'original-classes.h').write_text(classes,newline='\n')
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\nCEscalator CEscalators::aEscalators[NUM_ESCALATORS];\nint32 CEscalators::NumEscalators;\n'+'\n\n'.join(m.method(original,name) for name in m.METHODS)+'\nextern "C" {\n'
after='#include "'+(root/'src/renderer/Escalators.c').as_posix()+'"\n#include "caller.h"\n'
for old in [True,False]:
    text=before if old else after;prefix='CEscalators::' if old else 'Escalators_';count='CEscalators::NumEscalators' if old else 'Escalators_Num'
    text+='int32_t TestEs_Count(void)\n{\n    // Observe the actual canonical signed registration count at owner callbacks\n    return '+count+';\n}\nvoid TestEs_SetCount(int32_t value)\n{\n    // Mutate the real count without substituting a shadow controller state\n    '+count+'=value;\n}\n'
    for name in ['Init','Update','Shutdown']:text+='void TestEs_'+name+'(void)\n{\n    // Enter the complete original or C controller operation\n    '+prefix+name+'();\n}\n'
    text+='void TestEs_Add(EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown)\n{\n    // Capture all four by-value endpoints before entering a native record callback\n    '+prefix+'AddOne('+('CVector(first.x,first.y,first.z),CVector(lower.x,lower.y,lower.z),CVector(upper.x,upper.y,upper.z),CVector(last.x,last.y,last.z)' if old else 'first,lower,upper,last')+',movingDown);\n}\n'
    if old:text+='const void *TestEs_Owner(int32_t index)\n{\n    // Preserve the actual record reference returned by the original inline getter\n    return &CEscalators::GetEscalator(index);\n}\n}\n'
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
host='#include "common.h"\n#include "owners.h"\n#include "caller.h"\nstatic CEscalator EscalatorOwners[NUM_ESCALATORS];\n'+m.HOST+'''extern "C" const void *TestEs_Owner(int32_t index)
{
    // Borrow the real native record through the public const-reference compatibility entry
    return &EscalatorOwners[index];
}
extern "C" void Escalator_AddThisOne(int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown)
{
    // Keep the controller comparison attached to the same external record simulator
    EscalatorOwners[index].AddThisOne(CVector(first.x,first.y,first.z),CVector(lower.x,lower.y,lower.z),CVector(upper.x,upper.y,upper.z),CVector(last.x,last.y,last.z),movingDown);
}
extern "C" void Escalator_Update(int32_t index)
{
    // Preserve the original record-update callbacks through the direct C entry
    EscalatorOwners[index].Update();
}
extern "C" void Escalator_SwitchOff(int32_t index)
{
    // Preserve the original record shutdown callbacks through the direct C entry
    EscalatorOwners[index].SwitchOff();
}
'''
(out/'host.cpp').write_text(host,newline='\n')
caller='#include "caller.h"\n'
for name in ['Init','Update','Shutdown']:caller+='void TestEs_Run'+name+'(void)\n{\n    // Exercise the original or converted operation from a separate C11 translation unit\n    TestEs_'+name+'();\n}\n'
caller+='void TestEs_RunAdd(EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown)\n{\n    // Retain the actual C by-value vector and boolean argument ABI\n    TestEs_Add(first,lower,upper,last,movingDown);\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/escalators.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'fast-absf.c').write_text((root/'build/moving-things-c-tests/fast-absf.c').read_text(),newline='\n')
print('Audited all four controller methods, all 22 registrations, original record inputs and complete current C record relocation')
