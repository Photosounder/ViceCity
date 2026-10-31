"""Audit the gang relocation and generate complete original/C record and save comparisons."""
from pathlib import Path
import json,re
import gangs_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/gangs-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/peds/Gangs.cpp').read_text();header=(out/'before/src/peds/Gangs.h').read_text()
assert (root/'src/peds/GangData.c').read_text()==m.kernel(old)
assert (root/'src/peds/Gangs_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'dependencies.json').read_text()):assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "config.h"
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
#define VALIDATE_SIZE(type,size)
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#undef malloc
#undef free
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#undef assert
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
'''
(out/'common.h').write_text(common,newline='\n')
(out/'ModelIndices.h').write_text('#pragma once\n#include "ModelIndexData.h"\n',newline='\n')
(out/'Weapon.h').write_text('#pragma once\n#include "WeaponType.h"\n',newline='\n')
stream=(out/'before/src/core/Streaming.h').read_text()
state=re.search(r'enum StreamLoadState\n\{[\s\S]*?\n\};',stream)[0]
flags=re.search(r'enum StreamFlags\n\{[\s\S]*?\n\};',stream)[0]
record=re.search(r'class CStreamingInfo\n\{[\s\S]*?\n\};',stream)[0]
loaded=re.search(r'static bool HasModelLoaded\(int32 id\) \{[^\n]*\}',stream)[0]
assert loaded=='static bool HasModelLoaded(int32 id) { return ms_aInfoForModel[id].m_loadState == STREAMSTATE_LOADED; }'
loaded=loaded.replace('{ return','{\n    // Observe the original predicate before it reads the unchanged streaming state\n    TestGangs_StreamingQuery(id);return')
(out/'Streaming.h').write_text('#pragma once\n#include "common.h"\n'+flags+'\n'+state+'\n'+record+'\nvoid TestGangs_StreamingQuery(int32 id);\nclass CStreaming {public:static CStreamingInfo ms_aInfoForModel[4096];'+loaded+'};\n',newline='\n')
prototypes='''void TestGangs_Init(void);bool TestGangs_Loaded(int16_t gang);
void TestGangs_SetModels(int16_t gang,int32_t first,int32_t second);
void TestGangs_SetAttack(ePedType type,bool value);bool TestGangs_GetAttack(ePedType type);
int32_t TestGangs_Choose(int16_t gang);
void TestGangs_SetVehicle(int16_t gang,int32_t model);
void TestGangs_SetWeapons(int16_t gang,int32_t first,int32_t second);
void TestGangs_SetOverride(int16_t gang,int8_t value);int8_t TestGangs_GetOverride(int16_t gang);
void TestGangs_Save(uint8_t *buffer,uint32_t *size);void TestGangs_Load(uint8_t *buffer,uint32_t size);
int32_t TestGangs_GetModel1(int16_t gang);int32_t TestGangs_GetModel2(int16_t gang);int32_t TestGangs_GetVehicle(int16_t gang);
const void *TestGangs_Record(int16_t gang);void TestGangs_Read(int16_t gang,GangInfo *out);void TestGangs_Write(int16_t gang,const GangInfo *value);
'''
(out/'caller.h').write_text('#pragma once\n#include "Gangs.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
methods=[('void','Init','void','Initialise',''),('bool','Loaded','int16_t gang','HaveGangModelsLoaded','gang'),('void','SetModels','int16_t gang,int32_t first,int32_t second','SetGangPedModels','gang,first,second'),('void','SetAttack','ePedType type,bool value','SetWillAttackPlayerWithCops','type,value'),('bool','GetAttack','ePedType type','GetWillAttackPlayerWithCops','type'),('int32_t','Choose','int16_t gang','ChooseGangPedModel','gang'),('void','SetVehicle','int16_t gang,int32_t model','SetGangVehicleModel','gang,model'),('void','SetWeapons','int16_t gang,int32_t first,int32_t second','SetGangWeapons','gang,first,second'),('void','SetOverride','int16_t gang,int8_t value','SetGangPedModelOverride','gang,value'),('int8_t','GetOverride','int16_t gang','GetGangPedModelOverride','gang'),('void','Save','uint8_t *buffer,uint32_t *size','SaveAllGangData','buffer,size'),('void','Load','uint8_t *buffer,uint32_t size','LoadAllGangData','buffer,size'),('int32_t','GetModel1','int16_t gang','GetGangPedModel1','gang'),('int32_t','GetModel2','int16_t gang','GetGangPedModel2','gang'),('int32_t','GetVehicle','int16_t gang','GetGangVehicleModel','gang'),('const void *','Record','int16_t gang','GetGangInfo','gang')]
for before in [True,False]:
    text='#include "common.h"\n#include "caller.h"\n#include "ModelIndices.h"\n#include "General.h"\n#include "Streaming.h"\n#include "Weapon.h"\n#include "SaveBuf.h"\n'
    if before:
        h=re.sub(r'#pragma once|#include[^\n]*\n','',header).replace('private:','public:')
        text+='namespace Original {\n'+h+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    prefix='Original::CGangs::' if before else 'Gangs_'
    for kind,alias,args,name,values in methods:text+='extern "C" '+kind+' TestGangs_'+alias+'('+args+')\n{\n    // Execute the complete original or C gang operation with the same scalar inputs\n    '+('return ' if kind!='void' else '')+prefix+name+'('+values+');\n}\n'
    text+='extern "C" void TestGangs_Read(int16_t gang,GangInfo *out)\n{\n    // Read the actual canonical record representation without type punning\n    memcpy(out,'+prefix+'GetGangInfo(gang),sizeof(*out));\n}\n'
    text+='extern "C" void TestGangs_Write(int16_t gang,const GangInfo *value)\n{\n    // Seed complete real records including defined nonzero padding through byte copies\n    memcpy('+prefix+'GetGangInfo(gang),value,sizeof(*value));\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text,newline='\n')
(out/'host.cpp').write_text((root/'src/peds/Gangs.cpp').read_text(),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/gangs.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'crtrand.h').write_text('#include "config.h"\n#undef USE_PS2_RAND\n',newline='\n')
print('Audited all sixteen gang operations, constructor-equivalent static records, real streaming predicate, record-assignment saves and eleven complete callers')
