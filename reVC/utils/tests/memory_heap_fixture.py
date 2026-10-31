"""Prepare entire original and C custom heaps with controlled external game-owner boundaries."""
from pathlib import Path
import re
import memory_heap_migration as m
from memory_heap_schema import GROUPS
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/memory-heap-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <new>
#include <malloc.h>
#define nil NULL
typedef uint32_t uint32;
typedef int32_t int32;
typedef int16_t int16;
typedef uint8_t uint8;
typedef uintptr_t uintptr;
#ifdef ORIGINAL
#include "original-MemoryHeap.h"
#else
#include "MemoryHeap.h"
#endif
#include "FileMgr.h"
#include "Timer.h"
enum eLevelName {LEVEL_GENERIC=0,LEVEL_INDUSTRIAL=11,LEVEL_COMMERCIAL=22,LEVEL_SUBURBAN=33};
class CGame {public:static bool playingIntro;static eLevelName currLevel;static void TidyUpMemory(bool,bool);};
class CStreaming {public:static size_t ms_memoryUsed;static void MakeSpaceFor(int32);static void RemoveIslandsNotUsed(eLevelName);static void RemoveBigBuildings(eLevelName);static void RequestBigBuildings(eLevelName);static void LoadAllRequestedModels(bool);};
class CModelInfo {public:static void RemoveColModelsFromOtherLevels(eLevelName);};
class CCollision {public:static eLevelName ms_collisionInMemory;};
class CFileLoader {public:static void LoadCollisionFromDatFile(eLevelName);};
void LoadingScreen(const char*,const char*,const char*);
#ifdef _MSC_VER
#define CITA_TLS __declspec(thread)
#else
#define CITA_TLS __thread
#endif
extern "C" {
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
void re3_assert(const char*,const char*,unsigned int,const char*);
}
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#ifndef ORIGINAL
extern "C" {
#endif
extern void **gPtrList[4000];
extern int32 numPtrs,gPosnInList;
extern CIntStack4000 m_ptrListIndexStack;
extern uint32 memMoved;
#ifndef ORIGINAL
}
#endif
'''
config='#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n'
entry='''#pragma once
#include <stdint.h>
typedef struct HeapResult {void *pointer;uint32_t number;} HeapResult;
#ifdef __cplusplus
extern "C" {
#endif
HeapResult TestHeap(unsigned operation,void *self,void *a,void *b,uint32_t value);
#ifdef __cplusplus
}
#endif
'''
before='#include "owners.h"\n#include "entry.h"\n';after='#include "MemoryHeap.h"\n#include "entry.h"\n';enums=[];beforeCases=[];afterCases=[];index=0
for prefix,rows,owner,origin in GROUPS:
    for result,name,args,signature in rows:
        # Generate independent complete dispatchers for all record and owner methods
        enums.append('OP_'+prefix+'_'+name+'='+str(index));index+=1;ctype={'block':'HeapBlockDesc','list':'HeapBlockList','common':'CommonSize','heap':'CMemoryHeap'}[owner];parameters=[]
        for item in args.split(',')[1:]:
            if 'HeapBlockDesc *' in item:parameters.append('(HeapBlockDesc*)'+('b' if item.split('*')[1] in ['end','second','src'] else 'a'))
            elif 'void *ptr' in item or 'void *block' in item:parameters.append('a')
            elif 'int32_t id' in item:parameters.append('(int32_t)value')
            else:parameters.append('value')
        native='new(self) CMemoryHeap' if name=='InitRecord' else '(('+ctype+'*)self)->'+name+'('+','.join(parameters)+')';modern=prefix+'_'+name+'(('+ctype+'*)self'+(',' if parameters else '')+','.join(parameters)+')'
        for cases,expression in [(beforeCases,native),(afterCases,modern)]:
            store='' if result=='void' else 'result.pointer=' if '*' in result else 'result.number=';cases.append('    case OP_'+prefix+'_'+name+':'+store+expression+';break;')
entry+='enum {\n    '+',\n    '.join(enums)+'\n};\n'
for name,cases,prefix in [('before-caller.cpp',beforeCases,before),('after-caller.c',afterCases,after)]:
    # Compile the after-side dispatcher as independent strict C11 with no synthetic C++ owner declarations
    value=prefix+'HeapResult TestHeap(unsigned operation,void *self,void *a,void *b,uint32_t value)\n{\n    // Invoke each complete original or C owner operation through an independent public caller\n    HeapResult result={0};switch(operation){\n'+'\n'.join(cases)+'\n    }return result;\n}\n';(probe/name).write_text(value,newline='\n')
original=re.sub(r'^#include[^\n]*\n','',(out/'before/src/rw/MemoryHeap.cpp').read_text(),flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
bindings='#include "owners.h"\n'
for name in ['src/rw/MemoryMgr_native.cpp','src/collision/ColModel_native.cpp','src/animation/RpAnimBlend_native.cpp']:
    # Exercise the exact former forwarding definitions on the original side of the comparison
    for _,_,definition,symbol in m.forwarder_definitions((out/'before'/name).read_text()):
        owners+='\nextern "C" '+definition[:definition.index('\n{')]+';\n'
        if m.FORWARDERS[symbol]:bindings+='#ifdef USE_CUSTOM_ALLOCATOR\n'
        bindings+=definition+'\n'
        if m.FORWARDERS[symbol]:bindings+='#endif\n'
for name,value in [('owners.h',owners),('config.h',config),('entry.h',entry),('original-MemoryHeap.h',(out/'before/src/rw/MemoryHeap.h').read_text()),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "MemoryHeap_backend.h"\n'+native),('fixture.cpp',(root/'utils/tests/memory_heap_fixture.cpp.in').read_text())]:
    # Materialize complete producers and SDK-only fixtures without modifying the coherent candidate
    (probe/name).write_text(value,newline='\n')
print('Prepared all',index,'original/C custom-heap operations and controlled legacy owner boundaries')
(probe/'before-bindings.cpp').write_text(bindings,newline='\n')
