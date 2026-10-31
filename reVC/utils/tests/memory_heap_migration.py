"""Project all custom heap records, list operations, allocation and relocation policy into C."""
import re
from rect_migration import mask,closing
from memory_heap_schema import GROUPS,DECLARATIONS,HOST,HEAP,HEAP_INLINE
RENAMES={'src/rw/MemoryHeap.cpp':'src/rw/MemoryHeap.c'}
FIELDS={'block':['m_size','m_memId','m_ptrListIndex','m_next','m_prev'],'list':['m_first','m_last'],'common':['m_freeList','m_size','m_failed','m_remaining'],'heap':['m_start','m_end','m_freeList','m_fixedSize','m_totalMemUsed','m_idStack','m_currentMemID','m_memUsed','m_totalBlocksUsed','m_blocksUsed','m_unkMemId']}

def body(source,signature):
    # Select exact declared definitions including same-line inline bodies and constructor initializer lists
    start=source.index(signature);code=mask(source);opening=code.index('{',start+len(signature));depth=1;end=opening+1
    while depth:
        if code[end]=='{':depth+=1
        elif code[end]=='}':depth-=1
        end+=1
    return source[opening+1:end-1]

def header(original):
    # Retain all original fields and enums while exposing complete records and explicit C operations
    enum=original[original.index('enum {'):original.index('\n};',original.index('enum {'))+3]
    source='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "IntStack.h"
#ifdef MoveMemory
#undef MoveMemory
#endif
'''+enum+'''
typedef struct HeapBlockDesc {
    uint32_t m_size;
    int16_t m_memId,m_ptrListIndex;
    struct HeapBlockDesc *m_next,*m_prev;
} HeapBlockDesc;
#ifdef USE_CUSTOM_ALLOCATOR
#ifdef __cplusplus
static_assert(sizeof(HeapBlockDesc)==0x10,"HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense");
#else
_Static_assert(sizeof(HeapBlockDesc)==0x10,"HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense");
#endif
#endif
typedef struct HeapBlockList {HeapBlockDesc m_first,m_last;} HeapBlockList;
typedef struct CommonSize {
    HeapBlockList m_freeList;
    uint32_t m_size,m_failed,m_remaining;
} CommonSize;
typedef struct CMemoryHeap {
    HeapBlockDesc *m_start,*m_end;
    HeapBlockList m_freeList;
    CommonSize m_fixedSize[NUM_FIXED_MEMBLOCKS];
    uint32_t m_totalMemUsed;
    CIntStack16 m_idStack;
    uint32_t m_currentMemID;
    uint32_t *m_memUsed;
    uint32_t m_totalBlocksUsed;
    uint32_t *m_blocksUsed;
    uint32_t m_unkMemId;
} CMemoryHeap;
#ifdef USE_CUSTOM_ALLOCATOR
#define PUSH_MEMID(id) MemoryHeap_PushMemId(&gMainHeap,(id))
#define POP_MEMID() MemoryHeap_PopMemId(&gMainHeap)
#define REGISTER_MEMPTR(ptr) MemoryHeap_RegisterMemPointer(&gMainHeap,(ptr))
#else
#define PUSH_MEMID(id)
#define POP_MEMID()
#define REGISTER_MEMPTR(ptr)
#endif
#ifdef __cplusplus
extern "C" {
#endif
extern CMemoryHeap gMainHeap;
'''
    source+=''.join(result+' '+name+'('+args+');\n' for result,name,args in DECLARATIONS)
    return source+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def backend_header():
    # Declare only external streaming and game-owner primitives while heap recovery decisions remain in C
    source='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stddef.h>\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    source+=''.join(result+' MemoryHeap_Host'+name+'('+args+');\n' for result,name,args,_ in HOST)
    return source+'void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def member_calls(source):
    # Convert only diagnosed heap-record member operations and preserve nested argument evaluation
    patterns=[(r'(self->m_fixedSize\[[^\]\n]+\]|\blist)(\.|->)(Init|Free|Malloc)\(','HeapCommon'),(r'(self->m_freeList)\.(Init|Insert)\(','HeapList'),(r'(self->m_freeList\.m_first)\.(FindSmallestFreeBlock)\(','HeapBlock'),(r'(\b(?:block|node|b|pos|remainder|next|newblock|dst|src)(?:->m_prev|->m_next)*)(->)(GetNextConsecutive|GetDataPointer|RemoveHeapFreeBlock|InsertHeapFreeBlock|FindSmallestFreeBlock)\(','HeapBlock')]
    for pattern,prefix in patterns:
        # Replace complete calls from the end so all original expression offsets remain valid
        code=mask(source)
        for match in reversed(list(re.finditer(pattern,code))):
            end=closing(code,match.end()-1);receiver=source[match.start(1):match.end(1)];name=match[len(match.groups())];args=source[match.end():end-1];dot='->' not in match[0][len(match[1]):]
            if dot:receiver='&'+receiver
            source=source[:match.start()]+prefix+'_'+name+'('+receiver+(',' if args.strip() else '')+args+')'+source[end:]
    return source

def transform(source,owner):
    # Replace live owner field access without touching fields reached through other record objects
    pattern=r'(?<![\w.>])\b(?:'+'|'.join(FIELDS[owner])+r')\b'
    for match in reversed(list(re.finditer(pattern,mask(source)))):source=source[:match.start()]+'self->'+source[match.start():]
    if owner=='heap':
        # Preserve recursion and unqualified same-owner helpers through explicit self parameters
        names=[row[1] for row in HEAP+HEAP_INLINE if row[1]!='InitRecord'];pattern=r'(?<![\w.>:])\b('+'|'.join(names)+r')\('
        for match in reversed(list(re.finditer(pattern,mask(source)))):
            end=closing(mask(source),match.end()-1);args=source[match.end():end-1];source=source[:match.start()]+'MemoryHeap_'+match[1]+'(self'+(',' if args.strip() else '')+args+')'+source[end:]
    source=member_calls(source)
    mapping={'CStreaming::ms_memoryUsed':'MemoryHeap_HostMemoryUsed()','CStreaming::MakeSpaceFor':'MemoryHeap_HostMakeSpaceFor','CGame::TidyUpMemory':'MemoryHeap_HostTidyGame','CGame::playingIntro':'MemoryHeap_HostPlayingIntro()','CModelInfo::RemoveColModelsFromOtherLevels(LEVEL_GENERIC)':'MemoryHeap_HostRemoveOtherCollision()','CCollision::ms_collisionInMemory':'MemoryHeap_HostCollisionLevel()','CFileLoader::LoadCollisionFromDatFile':'MemoryHeap_HostLoadCollision','CGame::currLevel':'MemoryHeap_HostCurrentLevel()','CStreaming::RequestBigBuildings':'MemoryHeap_HostRequestBuildings','CStreaming::LoadAllRequestedModels':'MemoryHeap_HostLoadRequested','LoadingScreen(':'MemoryHeap_HostLoadingScreen('}
    for level in ['INDUSTRIAL','COMMERCIAL','SUBURBAN']:
        # Keep the original legacy level operation bindings in native leaves without inventing Vice City mappings
        mapping['CStreaming::RemoveIslandsNotUsed(LEVEL_'+level+')']='MemoryHeap_HostRemoveIslands'+level.title()+'()'
        mapping['CStreaming::RemoveBigBuildings(LEVEL_'+level+')']='MemoryHeap_HostRemoveBuildings'+level.title()+'()'
    for old,new in mapping.items():source=source.replace(old,new)
    for old,new in [('uint32','uint32_t'),('int32','int32_t'),('uint8','uint8_t'),('uintptr','uintptr_t'),('nil','NULL'),('this','self')]:source=re.sub(r'\b'+old+r'\b',new,source)
    return source

def kernel(original,original_header):
    # Project every complete original algorithm and retain conditional availability and global state
    source='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "MemoryHeap.h"
#include "MemoryHeap_backend.h"
#include "FileMgr.h"
#include "Timer.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#define MEMORYHEAP_ASSERT(cond) assert(cond)
#define MEMORYHEAP_ASSERT_MESSAGE(cond,message) assert(cond)
'''
    source+='''#ifdef USE_CUSTOM_ALLOCATOR
void **gPtrList[4000];
int32_t numPtrs,gPosnInList;
CIntStack4000 m_ptrListIndexStack;
uint32_t memMoved;
CMemoryHeap gMainHeap;
#endif
'''
    for prefix,rows,owner,origin in GROUPS:
        # Keep all inline record algorithms available and original out-of-line policy under its custom-allocator guard
        if origin=='source':source+='#ifdef USE_CUSTOM_ALLOCATOR\n'
        for result,name,args,signature in rows:
            value=transform(body(original_header if origin=='header' else original,signature),owner)
            if name=='InitRecord':value='\n    self->m_start=NULL;\n'+value
            source+=result+' '+prefix+'_'+name+'('+args+')\n{\n    // Preserve the complete original '+owner+' '+name+' operation and live mutation order\n'+value+'\n}\n'
        if origin=='source':source+='#endif\n'
    return source+'//- rouz edit (ChatGPT)\n'

def native():
    # Keep only actual external owner primitives including the unchanged currently unsupported legacy bindings
    source='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "MemoryHeap.h"
#include "MemoryHeap_backend.h"
#include "Game.h"
#include "Streaming.h"
#include "ModelInfo.h"
#include "FileLoader.h"
#include "Collision.h"
#include "main.h"
#ifdef USE_CUSTOM_ALLOCATOR
'''
    for result,name,args,expression in HOST:
        # Bind one original native metadata access or external owner operation without moving heap policy into C++
        source+=result+' MemoryHeap_Host'+name+'('+args+')\n{\n    // Forward the original external owner primitive at its exact operation boundary\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
    return source+'#endif\n//- rouz edit (ChatGPT)\n'

FORWARDERS={
    'MemoryMgr_HostHeapInit':True,'MemoryMgr_HostHeapMalloc':True,
    'MemoryMgr_HostHeapRealloc':True,'MemoryMgr_HostHeapFree':True,
    'ColModel_BackendPush':False,'ColModel_BackendPop':False,'ColModel_BackendRegister':False,
    'AnimSequence_BackendPushAnimation':False,'AnimSequence_BackendPopMemory':False,
    'AnimSequence_BackendRegister':False,'AnimSequence_BackendMoveMemory':True,
}

def forwarder_definitions(source):
    # Select complete existing forwarding definitions without touching unrelated SDK primitives
    code=mask(source)
    result=[]
    for name in FORWARDERS:
        match=re.search(r'(?m)^[\w *]+\b'+name+r'\([^\n]*\)\n\{',code)
        if match:
            end=match.end();depth=1
            while depth:
                if code[end]=='{':depth+=1
                elif code[end]=='}':depth-=1
                end+=1
            result.append((match.start(),end,source[match.start():end],name))
    return sorted(result)

def heap_calls(source):
    # Replace each whole heap member call while retaining argument evaluation and data field access
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bgMainHeap\.([A-Za-z]+)\(',code))):
        assert match[1] in [row[1] for row in HEAP+HEAP_INLINE],match[1]
        end=closing(code,match.end()-1);args=source[match.end():end-1]
        source=source[:match.start()]+'MemoryHeap_'+match[1]+'(&gMainHeap'+(',' if args.strip() else '')+args+')'+source[end:]
    return source

def bindings(originals):
    # Move all existing public heap forwarding ABIs into C rather than retaining C-to-C++-to-C calls
    source='//+ rouz edit (ChatGPT)\n#include "config.h"\n#include "MemoryHeap.h"\n#include "MemoryMgr_backend.h"\n#include "ColModel_backend.h"\n#include "AnimBlendSequence_backend.h"\n'
    found=[]
    for original in originals:
        for _,_,definition,name in forwarder_definitions(original):
            if FORWARDERS[name]:source+='#ifdef USE_CUSTOM_ALLOCATOR\n'
            source+=heap_calls(definition)+'\n'
            if FORWARDERS[name]:source+='#endif\n'
            found.append(name)
    assert set(found)==set(FORWARDERS) and len(found)==len(FORWARDERS),found
    return source+'//- rouz edit (ChatGPT)\n'

def memory_heap_calls(source):
    # Convert the complete heap and move every existing caller onto the explicit C API
    from memory_heap_original_header import ORIGINAL_HEADER
    if 'CMemoryHeap::Init(uint32 total)' in source and 'CommonSize::Init(uint32 size)' in source:return kernel(source,ORIGINAL_HEADER)
    if 'class CMemoryHeap\n' in source and 'struct HeapBlockDesc\n' in source:return header(source)
    original=source
    for start,end,_,_ in reversed(forwarder_definitions(source) if '#include "common.h"' in source else []):
        source=source[:start]+source[end:]
    source=re.sub(r'#ifdef USE_CUSTOM_ALLOCATOR\n\s*#endif\n','',source)
    source=heap_calls(source)
    if source!=original and re.search(r'\bgMainHeap\.\w+\(',mask(original)):
        # Describe each changed caller statement without splitting the streaming accounting expression
        if 'CStreaming::UpdateMemoryUsed(void)' in source:
            source=source.replace('\tms_memoryUsed =\n\t\tMemoryHeap_', '\t// Read streaming categories through the complete C heap owner\n\tms_memoryUsed =\n\t\tMemoryHeap_')
        else:
            source=re.sub(r'(?m)^([ \t]*)([^\n]*\bMemoryHeap_[^\n]*)$',lambda match:match[1]+'// Use the complete C heap operation with the original live owner\n'+match[0],source)
    if source!=original:return '//+ rouz edit (ChatGPT)\n'+source+'//- rouz edit (ChatGPT)\n'
    return source

def memory_heap_project(source):
    # Select both C heap units and retain the separate external SDK primitive producer
    import object_names_migration as objects
    source=source.replace('src\\rw\\MemoryHeap.cpp','src\\rw\\MemoryHeap.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\MemoryHeap\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing per-file metadata while adding both independently compiled heap units
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>'
            prefix=match[1][:-len('MemoryHeap.c')]
            for name in ['MemoryHeap_native.cpp','MemoryHeap_bindings.c']:
                if '<ClCompile Include="'+prefix+name+'"' not in source:
                    result+='\n    <ClCompile Include="'+prefix+name+'"'
                    result+=('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if name.endswith('.c') else ' />')
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\rw\\MemoryHeap\.h)"\s*/>',source)
    if anchor:
        entry='<ClInclude Include="'+anchor[1][:-len('MemoryHeap.h')]+'MemoryHeap_backend.h" />'
        if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)


def calls(source):
    # Retain complete heap ownership before applying the complete C TXD successor
    import txd_store_migration as txd
    return txd.calls(memory_heap_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting both complete C TXD units
    import txd_store_migration as txd
    return txd.project(memory_heap_project(source))
