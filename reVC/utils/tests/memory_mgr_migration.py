"""Move complete allocation and aligned-storage policy into C while preserving CITAlloc and custom heap paths."""
import re
import object_names_migration as objects
from rect_migration import mask
RENAMES={'src/rw/MemoryMgr.cpp':'src/rw/MemoryMgr.c'}
OPERATIONS=['InitMemoryMgr','MemoryMgrMalloc','MemoryMgrRealloc','MemoryMgrCalloc','MemoryMgrFree','RwMallocAlign','RwFreeAlign']

def header():
    # Expose the complete memory owner without native SDK types or allocator redirection macros
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
extern uint8_t *pMemoryTop;
void InitMemoryMgr(void);
void *MemoryMgrMalloc(size_t size);
void *MemoryMgrRealloc(void *ptr,size_t size);
void *MemoryMgrCalloc(size_t num,size_t size);
void MemoryMgrFree(void *ptr);
void *RwMallocAlign(uint32_t size,uint32_t align);
void RwFreeAlign(void *mem);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def native_header(original):
    # Retain the exact existing native SDK table declaration and caller tracking macros beside the new C API
    return '//+ rouz edit (ChatGPT)\n'+original.replace('#pragma once','#pragma once\n#include "MemoryMgr.h"',1)+'//- rouz edit (ChatGPT)\n'

def backend_header():
    # Keep actual custom allocator operations behind their existing native owner boundary
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void MemoryMgr_HostHeapInit(uint32_t size);
void *MemoryMgr_HostHeapMalloc(uint32_t size);
void *MemoryMgr_HostHeapRealloc(void *ptr,uint32_t size);
void MemoryMgr_HostHeapFree(void *ptr);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Preserve every complete original memory operation and use the same actual CITAlloc header macros in C
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source,count=re.subn(r'// rouz edit\nRwMemoryFunctions memFuncs = \{[\s\S]*?\n\};\n','',source);assert count==1
    for old,new in [('gMainHeap.Init(','MemoryMgr_HostHeapInit('),('gMainHeap.Malloc(','MemoryMgr_HostHeapMalloc('),('gMainHeap.Realloc(','MemoryMgr_HostHeapRealloc('),('gMainHeap.Free(','MemoryMgr_HostHeapFree(')]:source=source.replace(old,new)
    source=re.sub(r'\buint8\b','uint8_t',source);source=re.sub(r'\buintptr\b','uintptr_t',source);source=re.sub(r'\bRwUInt32\b','uint32_t',source);source=re.sub(r'\bnil\b','NULL',source)
    comments={
        'InitMemoryMgr':'Initialize the original custom heap size only under its existing platform guards',
        'MemoryMgrMalloc':'Preserve allocator selection and the original live high-water pointer update',
        'MemoryMgrRealloc':'Preserve the original reallocation argument and high-water update ordering',
        'MemoryMgrCalloc':'Retain the original count multiplication, single-size high-water update and conditional clearing',
        'MemoryMgrFree':'Preserve custom null-free guards and the actual normal allocator macro',
        'RwMallocAlign':'Retain both original size and alignment formulas and store the original allocation before the result',
        'RwFreeAlign':'Read and release the original allocation through the complete C memory owner'}
    for name,comment in comments.items():
        # Describe each entire converted operation inside its actual function body
        pattern=r'(?m)^('+name+r'\([^\n]*\)\n\{\n)';source,count=re.subn(pattern,lambda match:match[1]+'    // '+comment+'\n',source);assert count==1,name
    prefix='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "MemoryMgr.h"
#include "MemoryMgr_backend.h"
#include <stdlib.h>
#include <string.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#ifndef MASTER
#define ASSERT(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define ASSERT(expression) (expression)
#endif
'''
    return prefix+source+'//- rouz edit (ChatGPT)\n'

def native(original):
    # Retain the actual SDK callback table as native data and forward only custom heap primitives
    table=re.search(r'RwMemoryFunctions memFuncs = \{[\s\S]*?\n\};',original)[0]
    return '''//+ rouz edit (ChatGPT)
#include "common.h"
#include "MemoryMgr_native.h"
#include "MemoryMgr_backend.h"
#include "MemoryHeap.h"
#undef MemoryMgrMalloc
#undef MemoryMgrFree
#undef MemoryMgrRealloc
#undef MemoryMgrCalloc
'''+table+'''
#ifdef USE_CUSTOM_ALLOCATOR
void MemoryMgr_HostHeapInit(uint32_t size)
{
    // Initialize the actual native heap at the original external operation boundary
    gMainHeap.Init(size);
}
void *MemoryMgr_HostHeapMalloc(uint32_t size)
{
    // Forward the original narrowed byte request to the actual native heap
    return gMainHeap.Malloc(size);
}
void *MemoryMgr_HostHeapRealloc(void *ptr,uint32_t size)
{
    // Reallocate the exact caller pointer through the original native heap
    return gMainHeap.Realloc(ptr,size);
}
void MemoryMgr_HostHeapFree(void *ptr)
{
    // Release the exact caller pointer through the original native heap
    gMainHeap.Free(ptr);
}
#endif
//- rouz edit (ChatGPT)
'''

def memory_mgr_calls(source):
    # Move the complete owner and keep all existing native callers on the unchanged tracking macro contract
    if '#include "MemoryMgr_backend.h"' in source and '#include <cita_windows.h>' in source:return source
    if '#include "MemoryMgr.h"' in source and 'extern RwMemoryFunctions memFuncs;' in source:return source
    if 'RwMemoryFunctions memFuncs = {' in source and 'MemoryMgrCalloc(size_t num, size_t size)' in source:return kernel(source)
    if 'extern RwMemoryFunctions memFuncs;' in source and '#define MemoryMgrMalloc(s)' in source:return header()
    if '#include "MemoryMgr.h"' not in source:return source
    return '//+ rouz edit (ChatGPT)\n'+source.replace('#include "MemoryMgr.h"','#include "MemoryMgr_native.h"')+'//- rouz edit (ChatGPT)\n'


def memory_mgr_project(source):
    # Select the complete C memory-manager producer and add its distinct native primitive unit
    source=source.replace('src\\rw\\MemoryMgr.cpp','src\\rw\\MemoryMgr.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\MemoryMgr\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain all other metadata while explicitly selecting strict C11 compilation
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';native=match[1][:-len('MemoryMgr.c')]+'MemoryMgr_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\rw\\MemoryMgr\.h)"\s*/>',source)
    if anchor:
        # Keep the native boundary declarations beside the self-contained public C API
        entry='<ClInclude Include="'+anchor[1][:-len('MemoryMgr.h')]+'MemoryMgr_backend.h" />'
        if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
        nativeHeader='<ClInclude Include="'+anchor[1][:-len('MemoryMgr.h')]+'MemoryMgr_native.h" />'
        if nativeHeader not in source:source=source[:anchor.end()]+'\n    '+nativeHeader+source[anchor.end():]
    return objects.project(source)


def calls(source):
    # Retain complete memory-manager ownership before applying the complete C heap successor
    import memory_heap_migration as heap
    return heap.calls(memory_mgr_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting both complete C heap units
    import memory_heap_migration as heap
    return heap.project(memory_mgr_project(source))
