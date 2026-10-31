"""Audit complete pedestrian data relocation and generate original/C behavioral comparisons."""
from pathlib import Path
import json,re,hashlib
import ped_data_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/ped-data-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text();expected=m.project(original) if path.startswith('build/') else m.calls(original)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
original=(out/'before/src/peds/PedType.cpp').read_text();header=(out/'before/src/peds/PedType.h').read_text()
assert (root/'src/peds/PedestrianData.c').read_text()==m.kernel(original)
assert (root/'src/peds/PedData_backend.h').read_text()==m.BACKEND
for path in ['src/core/common.h','src/core/config.h','src/core/FileMgr.h','src/core/General.h','src/core/General.c','src/save/SaveBuf.h']:assert (out/'before'/path).read_bytes()==(root/path).read_bytes(),path
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <memory>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
typedef uint32_t uint32;typedef uint8_t uint8;typedef uint16_t uint16;
typedef int8_t int8;typedef int16_t int16;
#define VALIDATE_SIZE(t,n) static_assert(sizeof(t)==n,"Original size")
#endif
#include "FileMgr.h"
extern "C" void *Test_Allocate(size_t);
extern "C" void Test_Free(void *);
extern "C" void Test_Debug(const char *,...);
extern "C" void Test_Error(const char *,...);
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#undef malloc
#undef free
#undef debug
#undef Error
#define malloc Test_Allocate
#define free Test_Free
#define debug Test_Debug
#define Error Test_Error
#undef assert
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
'''
(out/'common.h').write_text(common,newline='\n')
prototypes='''void Test_TypeInit(void);void Test_TypeShutdown(void);void Test_TypeRead(void);
void Test_StatsInit(void);void Test_StatsShutdown(void);void Test_StatsRead(void);
int Test_FindType(char *);uint32_t Test_FindFlag(char *);int Test_FindStat(char *);
void Test_Save(uint8_t *,uint32_t *);void Test_Load(uint8_t *,uint32_t);
PedType *Test_Type(int);PedStats *Test_Stat(int);
uint32_t Test_GetFlag(int);uint32_t Test_GetAvoid(int);uint32_t Test_GetThreats(int);
void Test_SetThreats(int,uint32_t);void Test_AddThreat(int,int);void Test_RemoveThreat(int,int);bool Test_IsThreat(int,int);
'''
(out/'caller.h').write_text('#pragma once\n#include "'+(root/'src/peds/PedType.h').as_posix()+'"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
methods=[('void','TypeInit','void','CPedType','Initialise',''),('void','TypeShutdown','void','CPedType','Shutdown',''),('void','TypeRead','void','CPedType','LoadPedData',''),('void','StatsInit','void','CPedStats','Initialise',''),('void','StatsShutdown','void','CPedStats','Shutdown',''),('void','StatsRead','void','CPedStats','LoadPedStats',''),('int','FindType','char *name','CPedType','FindPedType','name'),('uint32_t','FindFlag','char *name','CPedType','FindPedFlag','name'),('int','FindStat','char *name','CPedStats','GetPedStatType','name'),('void','Save','uint8_t *buf,uint32_t *size','CPedType','Save','buf,size'),('void','Load','uint8_t *buf,uint32_t size','CPedType','Load','buf,size')]
for kind,name,args,body in m.ACCESSORS:methods.append((kind,name,args,'CPedType',name,','.join(p.strip().split()[-1] for p in args.split(','))))
for before in [True,False]:
    text='#include "common.h"\n#include "caller.h"\n#include "General.h"\n#include "SaveBuf.h"\n'
    if before:text+='namespace Original {\n'+header.replace('#pragma once','').replace('class CPedType\n{','class CPedType\n{\npublic:')+'\n'+re.sub(r'^#include[^\n]*\n','',original,flags=re.M)+'\n}\n'
    for kind,alias,args,cls,name,invoke in methods:
        prefix='Original::'+cls+'::' if before else ('PedType_' if cls=='CPedType' else 'PedStats_')
        text+='extern "C" '+kind+' Test_'+alias+'('+args+')\n{\n    // Enter the complete original or converted operation with unchanged arguments\n    '+('return ' if kind!='void' else '')+prefix+name+'('+invoke+');\n}\n'
    for kind,count,array,cls in [('PedType','NUM_PEDTYPES','PedType_records','CPedType'),('PedStats','NUM_PEDSTATS','PedStats_records','CPedStats')]:
        text+='extern "C" '+kind+' *Test_'+('Type' if kind=='PedType' else 'Stat')+'(int i)\n{\n    // Expose the actual canonical allocation for byte and pointer-identity checks\n    return ('+kind+' *)'+('Original::'+cls+'::'+('ms_apPedType' if kind=='PedType' else 'ms_apPedStats') if before else array)+'[i];\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/ped_data.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited 18 complete pedestrian operations, 23 type and 40 statistic records, ten callers and both project files')
