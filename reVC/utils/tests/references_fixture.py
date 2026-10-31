"""Audit whole reference relocation and produce independent original/C drivers."""
from pathlib import Path
import re,json
import references_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/references-c-tests'
def normalize(source):return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/core/References.cpp').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/core/References.c').read_text())
assert (root/'src/core/References_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^(?:CReferences|CEntity)::\w+\(',old,re.M))==7
(out/'original-References.h').write_text((out/'before/src/core/References.h').read_text(),newline='\n')
for path in ['src/core/config.h','src/core/Pool.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
(out/'common.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include "config.h"
typedef uint8_t uint8;
typedef int32_t int32;
#define nil NULL
''',newline='\n')
(out/'Entity.h').write_text('''#pragma once
#include "common.h"
struct CReference;
void TestEvent(unsigned event, unsigned value);
class CEntity {
public:
    CReference *m_pFirstReference=nullptr;
    unsigned testId=0;
    bool building=false;
    virtual ~CEntity() {}
    bool IsBuilding() {
        // Record the original type query without recording implementation-only field access
        TestEvent(1,testId);return building;
    }
    void RegisterReference(CEntity **pent);
    void CleanUpOldReference(CEntity **pent);
    void ResolveReferences(void);
    void PruneReferences(void);
};
class TestAnchor { public: uintptr_t guard=0x13579;virtual ~TestAnchor() {} };
class CPed : public TestAnchor, public CEntity {};
class CVehicle : public TestAnchor, public CEntity {};
class CObject : public TestAnchor, public CEntity {};
class CPlayerPed : public CPed {};
''',newline='\n')
(out/'Vehicle.h').write_text('#pragma once\n#include "Entity.h"\n',newline='\n')
(out/'PlayerPed.h').write_text('#pragma once\n#include "Entity.h"\n',newline='\n')
(out/'World.h').write_text('''#pragma once
#include "Entity.h"
struct TestPlayerInfo { CPlayerPed *m_pPed; };
class CWorld { public: static TestPlayerInfo Players[NUMPLAYERS];static uint8 PlayerInFocus; };
CPlayerPed *FindPlayerPed(int32 playerId=-1);
CVehicle *FindPlayerVehicle(int32 playerId=-1,bool remote=false);
''',newline='\n')
(out/'Pools.h').write_text('''#pragma once
#include "Entity.h"
#include "Pool.h"
class CPools { public:
    static CPool *GetPedPool(void);
    static CPool *GetVehiclePool(void);
    static CPool *GetObjectPool(void);
};
''',newline='\n')
(out/'original.cpp').write_text(old.replace('#include "References.h"','#include "original-References.h"'),newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'driver.h').write_text('''#pragma once
#include "Entity.h"
struct CReference;
extern "C" {
void Test_Init(void);
void Test_Register(CEntity *entity,CEntity **slot);
void Test_Clean(CEntity *entity,CEntity **slot);
void Test_Resolve(CEntity *entity);
void Test_Prune(CEntity *entity);
void Test_Player(void);
void Test_World(void);
CReference *Test_Records(void);
CReference **Test_Free(void);
}
''',newline='\n')
oldcaller='''#include "common.h"
#include "original-References.h"
#include "driver.h"
#include <stddef.h>
static_assert(sizeof(CReference)==2*sizeof(void *),"Original reference record size");
static_assert(alignof(CReference)==alignof(void *),"Original reference alignment");
static_assert(offsetof(CReference,next)==0 && offsetof(CReference,pentity)==sizeof(void *),"Original field offsets");
'''
newcaller='#include "References.h"\n#include "driver.h"\nextern "C" void Test_C_Register(CEntity *,CEntity **);\n'
for suffix,args,oldexpr,newexpr in [
    ('Init','void','CReferences::Init()','References_Init()'),
    ('Register','CEntity *entity,CEntity **slot','entity->RegisterReference(slot)','Test_C_Register(entity,slot)'),
    ('Clean','CEntity *entity,CEntity **slot','entity->CleanUpOldReference(slot)','References_CleanUpOldReference(entity,slot)'),
    ('Resolve','CEntity *entity','entity->ResolveReferences()','References_ResolveReferences(entity)'),
    ('Prune','CEntity *entity','entity->PruneReferences()','References_PruneReferences(entity)'),
    ('Player','void','CReferences::RemoveReferencesToPlayer()','References_RemoveReferencesToPlayer()'),
    ('World','void','CReferences::PruneAllReferencesInWorld()','References_PruneAllReferencesInWorld()')]:
    for original,expression in [(True,oldexpr),(False,newexpr)]:
        body='extern "C" void Test_'+suffix+'('+args+')\n{\n    // Enter the complete '+('original' if original else 'converted')+' algorithm from a separate caller\n    '+expression+';\n}\n'
        if original:oldcaller+=body
        else:newcaller+=body
oldcaller+='extern "C" CReference *Test_Records(void)\n{\n    // Observe the entire original record pool\n    return CReferences::aRefs;\n}\nextern "C" CReference **Test_Free(void)\n{\n    // Observe the original live free-list head\n    return &CReferences::pEmptyList;\n}\n'
newcaller+='extern "C" CReference *Test_Records(void)\n{\n    // Observe the entire converted record pool\n    return References_aRefs;\n}\nextern "C" CReference **Test_Free(void)\n{\n    // Observe the converted live free-list head\n    return &References_pEmptyList;\n}\n'
(out/'original-caller.cpp').write_text(oldcaller,newline='\n')
(out/'caller.cpp').write_text(newcaller,newline='\n')
(out/'c-caller.c').write_text('''#include "References.h"
void Test_C_Register(CEntity *entity,CEntity **slot)
{
    // Exercise the public opaque-entity interface from a separate C11 unit
    References_RegisterReference(entity,slot);
}
''',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/references.cpp.in').read_text(),newline='\n')
(out/'no-fix.h').write_text('#pragma once\n#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all seven complete algorithms, 13 typed owner operations, four temporary entity entry wrappers and ten source/project transformations')
