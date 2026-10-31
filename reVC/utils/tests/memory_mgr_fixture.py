"""Prepare whole original/C memory producers with the actual CITAlloc macro declarations."""
from pathlib import Path
import re
import memory_mgr_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/memory-mgr-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#include "config.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef uint8_t uint8;
typedef uint32_t RwUInt32;
typedef uintptr_t uintptr;
struct RwMemoryFunctions {
    void *(*rwmalloc)(size_t);
    void (*rwfree)(void*);
    void *(*rwrealloc)(void*,size_t);
    void *(*rwcalloc)(size_t,size_t);
};
class MemoryHeap {public:void Init(uint32_t);void *Malloc(uint32_t);void *Realloc(void*,uint32_t);void Free(void*);};
extern MemoryHeap gMainHeap;
#ifdef ORIGINAL
extern uint8 *pMemoryTop;
void InitMemoryMgr(void);
void *MemoryMgrMalloc(size_t);
void *MemoryMgrRealloc(void*,size_t);
void *MemoryMgrCalloc(size_t,size_t);
void MemoryMgrFree(void*);
void *RwMallocAlign(RwUInt32,RwUInt32);
void RwFreeAlign(void*);
#else
#include "MemoryMgr.h"
#endif
extern RwMemoryFunctions memFuncs;
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
#define nil NULL
#ifndef MASTER
#define ASSERT(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define ASSERT(expression) (expression)
#endif
'''
config='''#pragma once
#ifndef CLASSIC
#define FIX_BUGS
#endif
#if defined(_WIN64) || defined(__LP64__)
#define FIX_BUGS_64
#endif
'''
entry='''#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void *TestMemory(unsigned operation,void *ptr,size_t count,size_t size,uint32_t alignment);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
void *TestMemory(unsigned operation,void *ptr,size_t count,size_t size,uint32_t alignment)
{
    // Exercise every complete original or C memory operation through the independent public caller
    switch(operation){
    case 0:InitMemoryMgr();return NULL;
    case 1:return MemoryMgrMalloc(size);
    case 2:return MemoryMgrRealloc(ptr,size);
    case 3:return MemoryMgrCalloc(count,size);
    case 4:MemoryMgrFree(ptr);return NULL;
    case 5:return RwMallocAlign((uint32_t)size,alignment);
    case 6:RwFreeAlign(ptr);return NULL;
    }return NULL;
}
'''
original=re.sub(r'^#include[^\n]*\n','',(out/'before/src/rw/MemoryMgr.cpp').read_text(),flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native((out/'before/src/rw/MemoryMgr.cpp').read_text()),flags=re.M)
for name,value in [('owners.h',owners),('config.h',config),('entry.h',entry),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "MemoryMgr_backend.h"\n'+native),('before-caller.cpp','#include "owners.h"\n'+caller),('after-caller.c','#include "MemoryMgr.h"\n'+caller),('fixture.cpp',(root/'utils/tests/memory_mgr_fixture.cpp.in').read_text())]:
    # Keep complete producer fixtures isolated from the coherent original and staged source trees
    (probe/name).write_text(value,newline='\n')
print('Prepared all seven original/C memory operations, actual allocator macros and native SDK callback tables')
