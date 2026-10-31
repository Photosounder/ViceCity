"""Prepare whole shot producers with actual C metadata, vector definitions and native leaves."""
from pathlib import Path
import re
from memory_heap_migration import body
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shot-info-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True);saved=out/'before';tree=out/'overlay'
owners='''#pragma once
#include "config.h"
#include <math.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "maths.h"
typedef uint32_t uint32;
struct RwV3d {float x,y,z;};
#include "Vector.h"
#include "WeaponInfo.h"
#include "Timer.h"
#include "General.h"
class CEntity {
public:
    unsigned id;bool ped,bFireProof;CVector position;
    bool IsPed(void);CVector &GetPosition(void);
};
class CPed: public CEntity {
public:
    bool valid,control,player;int m_numNearPeds;CPed *m_nearPeds[8];
    bool IsPointerValid(void);bool IsPedInControl(void);bool IsPlayer(void);
    void SetFindPathAndFlee(CEntity *,uint32_t);void SetMoveState(int);
};
enum {PEDMOVE_SPRINT=7};
class CWorld {public:static void SetCarsOnFire(float,float,float,float,CEntity*);};
#include "Fire.h"
void TestDebug(const char *,...);
void re3_assert(const char *,const char *,unsigned,const char *);
#define debug(...) TestDebug(__VA_ARGS__)
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__FUNCTION__),0)))
#else
#define assert(expression) (expression)
#endif
#define nil NULL
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Max(a,b) ((a)>(b)?(a):(b))
'''
metadata=(tree/'src/weapons/WeaponInfo.h').read_text();metadata=re.sub(r'#ifdef __cplusplus\n#include "AnimManager.h"\n#include "AnimationId.h"\n#endif','',metadata)
(probe/'WeaponInfo.h').write_text(metadata,newline='\n');(probe/'owners.h').write_text(owners,newline='\n')
(probe/'original-ShotInfo.h').write_text((saved/'src/weapons/ShotInfo.h').read_text(),newline='\n')
original=(saved/'src/weapons/ShotInfo.cpp').read_text();(probe/'original.cpp').write_text('#include "owners.h"\n#include "original-ShotInfo.h"\n'+re.sub(r'^#include[^\n]*\n','',original,flags=re.M),newline='\n')
(probe/'after-owner.c').write_text((tree/'src/weapons/ShotInfo.c').read_text(),newline='\n')
native=(tree/'src/weapons/ShotInfo_native.cpp').read_text();(probe/'native.cpp').write_text('#include "owners.h"\n#include "ShotInfo_backend.h"\n'+re.sub(r'^#include[^\n]*\n','',native,flags=re.M),newline='\n')
vector=(saved/'src/math/Vector.cpp').read_text();(probe/'vector.cpp').write_text('#include "owners.h"\nvoid CVector::Normalise(void)\n{'+body(vector,'CVector::Normalise(void)')+'}\n',newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct TestRecord {int32_t weapon;float start[3],velocity[3],radius,timeout;void *source;bool active;} TestRecord;
#ifdef __cplusplus
extern "C" {
#endif
void TestInitialise(void);
void TestShutdown(void);
void TestUpdate(void);
bool TestAdd(void *source,int32_t weapon,const float *start,const float *end);
void TestRead(unsigned slot,TestRecord *record);
void TestSet(unsigned slot,const TestRecord *record);
void TestRandom(float *table);
uint32_t TestCounter(void);
void TestSetCounter(uint32_t value);
#ifdef __cplusplus
}
#endif
'''
(probe/'entry.h').write_text(entry,newline='\n')
for before in [True,False]:
    # Compile original native value calls and genuine C calls independently from both complete producers
    record='CShotInfo' if before else 'ShotInfo';owner='CShotInfo::' if before else 'ShotInfo_'
    text=('#include "owners.h"\n#include "original-ShotInfo.h"\nextern CShotInfo gaShotInfo[NUMSHOTINFOS];\n' if before else '#include "ShotInfo.h"\n')+'#include "entry.h"\n'
    if before:text+='#ifdef SQUEEZE_PERFORMANCE\nextern uint32_t shotInfoInUse;\n#endif\n'
    for operation in ['Initialise','Shutdown','Update']:
        # Enter complete public operations without reimplementing any shot selection or update policy
        text+='void Test'+operation+'(void)\n{\n    // Dispatch the complete original or C operation\n    '+owner+operation+'();\n}\n'
    vectorType='CVector' if before else 'ShotInfoVector'
    text+='bool TestAdd(void *source,int32_t weapon,const float *start,const float *end)\n{\n    // Marshal the original by-value vectors and typed opaque source only\n    '+vectorType+' first'+('(' if before else '={')+'start[0],start[1],start[2]'+(');' if before else '};')+vectorType+' last'+('(' if before else '={')+'end[0],end[1],end[2]'+(');' if before else '};')+'return '+owner+'AddShot((CEntity*)source,(enum eWeaponType)weapon,first,last);\n}\n'
    for read in [True,False]:
        # Copy defined record fields for independent observation and controlled scenario setup
        text+='void Test'+('Read' if read else 'Set')+'(unsigned slot,'+('' if read else 'const ')+'TestRecord *record)\n{\n    // Access actual live owner fields without inspecting padding\n    '+record+' *shot=&gaShotInfo[slot];\n'
        pairs=[('weapon','m_weapon'),('radius','m_radius'),('timeout','m_timeout'),('source','m_sourceEntity'),('active','m_inUse')]+[(name+'['+str(i)+']',field+'.'+axis) for name,field in [('start','m_startPos'),('velocity','m_areaAffected')] for i,axis in enumerate(['x','y','z'])]
        for target,field in pairs:
            # Retain exact scalar types when populating or reading the actual original and C records
            text+=('record->'+target+'=shot->'+field+';' if read else 'shot->'+field+'='+('(CEntity*)' if target=='source' else '(enum eWeaponType)' if target=='weapon' else '')+'record->'+target+';')+'\n'
        text+='}\n'
    table='CShotInfo::ms_afRandTable' if before else 'ShotInfo_ms_afRandTable'
    text+='void TestRandom(float *table)\n{\n    // Read the actual initialized random table in declaration order\n    for(unsigned i=0;i<20;++i)table[i]='+table+'[i];\n}\nuint32_t TestCounter(void)\n{\n    // Expose optional state without inventing a counter in other profiles\n#ifdef SQUEEZE_PERFORMANCE\nreturn shotInfoInUse;\n#else\nreturn UINT32_MAX;\n#endif\n}\nvoid TestSetCounter(uint32_t value)\n{\n    // Configure the actual optional counter for explicitly prepared test scenarios\n#ifdef SQUEEZE_PERFORMANCE\nshotInfoInUse=value;\n#else\n(void)value;\n#endif\n}\n'
    (probe/('before-caller.cpp' if before else 'after-caller.c')).write_text(text,newline='\n')
(probe/'fixture.cpp').write_text((root/'utils/tests/shot_info_cases.cpp.in').read_text(),newline='\n')
print('Prepared complete shot producers, native definitions, actual metadata and vector normalization')
