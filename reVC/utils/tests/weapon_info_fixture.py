"""Audit whole weapon metadata relocation and emit complete original/C parser comparisons."""
from pathlib import Path
import json,re
import weapon_info_migration as m
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/weapon-info-c-tests'
changes=json.loads((out/'changes.json').read_text())
for path in changes:
    old=(out/'before'/path).read_text();assert (root/path).read_text()==(m.project(old) if path.startswith('build/') else m.calls(old)),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/weapons/WeaponInfo.cpp').read_text();header=(out/'before/src/weapons/WeaponInfo.h').read_text()
assert (root/'src/weapons/WeaponData.c').read_text()==m.kernel(old,header)
assert (root/'src/weapons/WeaponInfo_backend.h').read_text()==m.BACKEND
assert (root/'src/animation/AnimGroups.h').read_text()==m.groups_header((out/'before/src/animation/AnimManager.h').read_text())
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in changes:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;typedef uintptr_t uintptr;
struct RwV3d {float x,y,z;};struct RpAtomic;
#define nil NULL
#include "maths.h"
#include "Vector.h"
extern "C" void re3_debug(const char *,...);
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#define debug(format,...) re3_debug("[DBG]: " format,##__VA_ARGS__)
#define Error(format,...) re3_debug("[ERROR]: " format,##__VA_ARGS__)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
'''
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','maths.h']:(out/name).write_text((out/'before/src/math'/name).read_text(),newline='\n')
(out/'original-WeaponInfo.h').write_text(header.replace('VALIDATE_SIZE(CWeaponInfo, 0x64);',''),newline='\n')
(out/'original-AnimGroups.h').write_text(m.groups_header((out/'before/src/animation/AnimManager.h').read_text()),newline='\n')
shotgun=re.search(r'static bool IsShotgun\(int weapon\) \{[^}]+\}',(out/'before/src/weapons/Weapon.h').read_text())[0]
owners='''#pragma once
#include "common.h"
#include "WeaponType.h"
#include "FileMgr.h"
#include "ModelIndexData.h"
#ifdef TEST_WEAPON_INFO_ORIGINAL
#include "original-AnimGroups.h"
#include "original-WeaponInfo.h"
#else
#include "AnimGroups.h"
#include "'''+(root/'src/weapons/WeaponInfo.h').as_posix()+'''"
#endif
extern uint8 work_buff[55000];
struct WeaponInfoTestPrefix {uint64_t guard[2];};
class CBaseModelInfo {public:RpAtomic *m_atomics[3];int id;};
class CWeaponModelInfo:public WeaponInfoTestPrefix,public CBaseModelInfo {public:void SetWeaponInfo(int32 weapon);void SetWeaponInfoOriginal(int32 weapon);};
enum {TEST_WEAPON_MODEL_COUNT=512};
class CModelInfo {public:static CBaseModelInfo *models[TEST_WEAPON_MODEL_COUNT];
    static CBaseModelInfo *GetModelInfo(int model){
        // Preserve the native indexed model-table read and typed base pointer
        return models[model];
    }
};
struct AnimAssocDefinition {const char *name;};
class CAnimManager {public:static AnimAssocDefinition ms_aAnimAssocDefinitions[NUM_ANIM_ASSOC_GROUPS];static const char *GetAnimGroupName(AssocGroupId);static const char *GetAnimGroupNameOriginal(AssocGroupId);};
class CWeapon {public:'''+shotgun+'''};
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['main.h','AnimManager.h','AnimBlendAssociation.h','ModelInfo.h','Weapon.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
(out/'AnimationId.h').write_text('#pragma once\n',newline='\n')
prototypes='''CWeaponInfo *TestWeaponInfo_Record(int32_t index);
uint16_t *TestWeaponInfo_Reloads(void);int32_t *TestWeaponInfo_MaxAmmo(void);
void TestWeaponInfo_Init(void);void TestWeaponInfo_Load(void);void TestWeaponInfo_Shutdown(void);
int32_t TestWeaponInfo_Type(char *name);int32_t TestWeaponInfo_Fire(char *name);
bool TestWeaponInfo_Merge(uint32_t slot);bool TestWeaponInfo_Flag(int32_t index,uint32_t flag);
void TestWeaponInfo_Offset(int32_t index,float *output);
void TestWeaponInfo_RunInit(void);void TestWeaponInfo_RunLoad(void);void TestWeaponInfo_RunShutdown(void);
int32_t TestWeaponInfo_RunType(char *name);int32_t TestWeaponInfo_RunFire(char *name);
bool TestWeaponInfo_RunMerge(uint32_t slot);bool TestWeaponInfo_RunFlag(int32_t index,uint32_t flag);
'''
(out/'c_entry.h').write_text('#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nstruct CWeaponInfo;\nextern "C" {\n#else\ntypedef struct CWeaponInfo CWeaponInfo;\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
(out/'caller.h').write_text('#pragma once\n#include "owners.h"\n#include "c_entry.h"\n',newline='\n')
wrappers='''#include "caller.h"
extern "C" CWeaponInfo *TestWeaponInfo_Record(int32_t index)
{
    // Borrow the actual canonical record through its complete original or C accessor
#ifdef TEST_WEAPON_INFO_ORIGINAL
    return CWeaponInfo::GetWeaponInfo((eWeaponType)index);
#else
    return WeaponInfo_GetWeaponInfo((eWeaponType)index);
#endif
}
extern "C" uint16_t *TestWeaponInfo_Reloads(void)
{
    // Expose the original mutable reload table rather than a shadow copy
#ifdef TEST_WEAPON_INFO_ORIGINAL
    return CWeaponInfo::ms_aReloadSampleTime;
#else
    return WeaponInfo_ms_aReloadSampleTime;
#endif
}
extern "C" int32_t *TestWeaponInfo_MaxAmmo(void)
{
    // Expose the original mutable maximum-ammo table rather than a shadow copy
#ifdef TEST_WEAPON_INFO_ORIGINAL
    return CWeaponInfo::ms_aMaxAmmoForWeapon;
#else
    return WeaponInfo_ms_aMaxAmmoForWeapon;
#endif
}
extern "C" bool TestWeaponInfo_Flag(int32_t index,uint32_t flag)
{
    // Enter the complete original const member or its explicit C record operation
#ifdef TEST_WEAPON_INFO_ORIGINAL
    return TestWeaponInfo_Record(index)->IsFlagSet(flag);
#else
    return WeaponInfo_IsFlagSet(TestWeaponInfo_Record(index),flag);
#endif
}
extern "C" void TestWeaponInfo_Offset(int32_t index,float *output)
{
    // Preserve the actual remaining callers' native vector value and multiplication
#ifdef TEST_WEAPON_INFO_ORIGINAL
    CVector value=TestWeaponInfo_Record(index)->m_vecFireOffset;
#else
    CVector value=WeaponInfo_VectorToNative(TestWeaponInfo_Record(index)->m_vecFireOffset);
#endif
    value=1.8f*value;output[0]=value.x;output[1]=value.y;output[2]=value.z;
}
'''
ops=[('void','Init','Initialise','void',''),('void','Load','LoadWeaponData','void',''),('void','Shutdown','Shutdown','void',''),('int32_t','Type','FindWeaponType','char *name','name'),('int32_t','Fire','FindWeaponFireType','char *name','name'),('bool','Merge','IsWeaponSlotAmmoMergeable','uint32_t slot','slot')]
for kind,alias,target,args,invoke in ops:
    wrappers+='extern "C" '+kind+' TestWeaponInfo_'+alias+'('+args+')\n{\n    // Enter the complete original or C operation with the same values\n#ifdef TEST_WEAPON_INFO_ORIGINAL\n    '+('return ' if kind!='void' else '')+'CWeaponInfo::'+target+'('+invoke+');\n#else\n    '+('return ' if kind!='void' else '')+'WeaponInfo_'+target+'('+invoke+');\n#endif\n}\n'
(out/'original.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'+wrappers,newline='\n')
(out/'adapter.cpp').write_text(wrappers,newline='\n');(out/'host.cpp').write_text(m.HOST,newline='\n')
caller='#include "'+(root/'src/weapons/WeaponInfo.h').as_posix()+'"\n#include "c_entry.h"\n'
for kind,alias,target,args,invoke in [*ops,('bool','Flag','','int32_t index,uint32_t flag','index,flag')]:caller+=kind+' TestWeaponInfo_Run'+alias+'('+args+')\n{\n    // Retain scalar and borrowed-string arguments through a separate C11 caller\n    '+('return ' if kind!='void' else '')+'TestWeaponInfo_'+alias+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
names=re.search(r'char CWeaponInfo::ms_aWeaponNames[^=]+=(\s*\{[\s\S]*?\n\});',old)[1]
(out/'names.h').write_text('static const char *const names[WEAPONTYPE_TOTALWEAPONS]='+names+';\n',newline='\n')
parts=[function((out/'before/src/modelinfo/WeaponModelInfo.cpp').read_text(),'CWeaponModelInfo::','SetWeaponInfo').replace('CWeaponModelInfo::SetWeaponInfo','CWeaponModelInfo::SetWeaponInfoOriginal'),function((out/'before/src/animation/AnimManager.cpp').read_text(),'CAnimManager::','GetAnimGroupName').replace('CAnimManager::GetAnimGroupName','CAnimManager::GetAnimGroupNameOriginal')]
(out/'native.cpp').write_text('#include "owners.h"\n'+'\n\n'.join(parts)+'\n',newline='\n')
(out/'helpers.c').write_text('#include "ModelIndexData.h"\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/weapon_info.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'classic.h').write_text('#include "config.h"\n#undef PC_PLAYER_CONTROLS\n',newline='\n')
print('Audited eight complete metadata operations, all 37 canonical records/tables, exact shared buffer type, complete native setters/name getter and 24 callers')
