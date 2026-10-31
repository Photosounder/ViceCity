"""Move complete animation links and clump ownership, allocation policy and live frame traversal into C11."""
import re
from rect_migration import mask,closing,arguments
import object_names_migration as objects
RENAMES={'src/animation/AnimBlendClumpData.cpp':'src/animation/AnimBlendClumpData.c'}
LINK_OPERATIONS=['Init','Prepend','Remove']
CLUMP_OPERATIONS=['Init','Destroy','SetNumberOfFrames','SetNumberOfBones','ForAllFrames']

def link_header():
    # Keep the original two pointer fields without methods or native dependencies
    return '''//+ rouz edit (ChatGPT)
#pragma once
typedef struct CAnimBlendLink {struct CAnimBlendLink *next,*prev;} CAnimBlendLink;
#ifdef __cplusplus
extern "C" {
#endif
void AnimBlendLink_Init(CAnimBlendLink *self);
void AnimBlendLink_Prepend(CAnimBlendLink *self,CAnimBlendLink *link);
void AnimBlendLink_Remove(CAnimBlendLink *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def link_kernel():
    # Retain ordered live pointer reads including self and already-linked aliases
    return '''//+ rouz edit (ChatGPT)
#include "AnimBlendList.h"
#include <stddef.h>
void AnimBlendLink_Init(CAnimBlendLink *self)
{
    // Clear the two original link fields in their original order
    self->next=NULL;self->prev=NULL;
}
void AnimBlendLink_Prepend(CAnimBlendLink *self,CAnimBlendLink *link)
{
    // Keep each original pointer read and write live when either node aliases a neighbor
    if(self->next)self->next->prev=link;
    link->next=self->next;link->prev=self;self->next=link;
}
void AnimBlendLink_Remove(CAnimBlendLink *self)
{
    // Retain both conditional neighbor writes before clearing the detached link
    if(self->prev)self->prev->next=self->next;
    if(self->next)self->next->prev=self->prev;
    AnimBlendLink_Init(self);
}
//- rouz edit (ChatGPT)
'''

def frame_header(original):
    # Preserve the unchanged native SDK frame record as a separate dependency of native consumers
    start=original.index('struct AnimBlendFrameData\n');end=original.index('VALIDATE_SIZE(AnimBlendFrameData, 0x18);',start)+len('VALIDATE_SIZE(AnimBlendFrameData, 0x18);')
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+original[start:end]+'\n//- rouz edit (ChatGPT)\n'

def header():
    # Preserve the clump fields and velocity union while making native frames opaque to C
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "AnimBlendList.h"
#ifdef __cplusplus
#include "AnimBlendFrameData_native.h"
class CVector;
class CVector2D;
#else
typedef struct AnimBlendFrameData AnimBlendFrameData;
typedef struct CVector CVector;
typedef struct CVector2D CVector2D;
#endif
typedef struct CAnimBlendClumpData {
    CAnimBlendLink link;
    int32_t numFrames;
    union {CVector2D *velocity2d;CVector *velocity3d;};
    AnimBlendFrameData *frames;
} CAnimBlendClumpData;
typedef void (*AnimBlendFrameCallback)(AnimBlendFrameData *frame,void *arg);
#ifdef __cplusplus
extern "C" {
#endif
void AnimBlendClump_Init(CAnimBlendClumpData *self);
void AnimBlendClump_Destroy(CAnimBlendClumpData *self);
void AnimBlendClump_SetNumberOfFrames(CAnimBlendClumpData *self,int32_t count);
void AnimBlendClump_SetNumberOfBones(CAnimBlendClumpData *self,int32_t count);
void AnimBlendClump_ForAllFrames(CAnimBlendClumpData *self,AnimBlendFrameCallback callback,void *arg);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Expose only native frame size and the existing aligned memory primitives
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
size_t AnimBlendClump_BackendFrameSize(void);
void *AnimBlendClump_BackendAllocate(uint32_t size,uint32_t alignment);
void AnimBlendClump_BackendFree(void *memory);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend():
    # Retain the actual native metadata and allocator implementation without clump policy in C++
    return '''
size_t AnimBlendClump_BackendFrameSize(void)
{
    // Report the actual SDK frame stride used by the unchanged native record
    return sizeof(AnimBlendFrameData);
}
void *AnimBlendClump_BackendAllocate(uint32_t size,uint32_t alignment)
{
    // Preserve the native allocator's original unsigned argument widths
    return RwMallocAlign(size,alignment);
}
void AnimBlendClump_BackendFree(void *memory)
{
    // Retain the existing aligned free boundary
    RwFreeAlign(memory);
}
'''

def kernel():
    # Keep complete clump lifetime, replacement and live callback traversal policy in C
    return '''//+ rouz edit (ChatGPT)
#include "AnimBlendClumpData.h"
#include "AnimBlendClumpData_backend.h"
#include <stddef.h>
void AnimBlendClump_Init(CAnimBlendClumpData *self)
{
    // Retain constructor field order and initialize the embedded link last
    self->numFrames=0;self->velocity2d=NULL;self->frames=NULL;AnimBlendLink_Init(&self->link);
}
void AnimBlendClump_Destroy(CAnimBlendClumpData *self)
{
    // Detach the clump link before releasing any existing frame allocation
    AnimBlendLink_Remove(&self->link);
    if(self->frames)AnimBlendClump_BackendFree(self->frames);
}
void AnimBlendClump_SetNumberOfFrames(CAnimBlendClumpData *self,int32_t count)
{
    // Free old frames before storing the new signed count and forwarding the original unsigned allocation size
    if(self->frames)AnimBlendClump_BackendFree(self->frames);
    self->numFrames=count;
    self->frames=(AnimBlendFrameData*)AnimBlendClump_BackendAllocate((uint32_t)((size_t)self->numFrames*AnimBlendClump_BackendFrameSize()),64);
}
void AnimBlendClump_SetNumberOfBones(CAnimBlendClumpData *self,int32_t count)
{
    // Preserve the original bone-count alias at the same allocation boundary
    AnimBlendClump_SetNumberOfFrames(self,count);
}
void AnimBlendClump_ForAllFrames(CAnimBlendClumpData *self,AnimBlendFrameCallback callback,void *arg)
{
    // Re-read count and frame storage around every callback so callbacks can replace or resize the live container
    const size_t stride=AnimBlendClump_BackendFrameSize();
    for(int32_t i=0;i<self->numFrames;++i)callback((AnimBlendFrameData*)((unsigned char*)self->frames+(size_t)i*stride),arg);
}
//- rouz edit (ChatGPT)
'''

def anim_clump_calls(source):
    # Replace complete owner definitions while preserving the native SDK frame record separately
    original=source
    if 'class CAnimBlendLink\n' in source:return link_header()
    if 'class CAnimBlendClumpData\n' in source:return header()
    if re.search(r'(?m)^CAnimBlendClumpData::CAnimBlendClumpData\(',source):return kernel()
    source=source.replace('class CAnimBlendClumpData;','struct CAnimBlendClumpData;')
    if re.search(r'(?m)^CAnimBlendAssociation::CAnimBlendAssociation\(',source):
        source=source.replace('link.Init();','AnimBlendLink_Init(&link);').replace('link.Remove();','AnimBlendLink_Remove(&link);')
    code=mask(source)
    for match in reversed(list(re.finditer(r'\b(clumpData|pAnimBlendClumpData)->link\.Prepend\(',code))):
        end=closing(code,match.end()-1);source=source[:match.start()]+'AnimBlendLink_Prepend(&'+match[1]+'->link,'+source[match.end():end-1]+')'+source[end:]
    # Replace explicit plugin lifetime operations without changing malloc or free
    source=source.replace('std::allocator<CAnimBlendClumpData>().construct(data);','AnimBlendClump_Init(data);').replace('data->~CAnimBlendClumpData();','AnimBlendClump_Destroy(data);')
    code=mask(source)
    pattern=r'(\(\*RPANIMBLENDCLUMPDATA\([^\n]*?\)\)|clumpData|pAnimBlendClumpData|data)->(SetNumberOfFrames|SetNumberOfBones|ForAllFrames)\('
    for match in reversed(list(re.finditer(pattern,code))):
        end=closing(code,match.end()-1);source=source[:match.start()]+'AnimBlendClump_'+match[2]+'('+source[match.start(1):match.end(1)]+','+source[match.end():end-1]+')'+source[end:]
    if 'AnimBlendClump_BackendAllocate(' not in source and re.search(r'(?m)^RpAnimBlendAllocateData\(RpClump \*clump\)\n\{',source):
        source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendClumpData_backend.h"\n#include "MemoryMgr.h"',1)+backend()
    if source==original:return source
    # Describe each new native call at its existing lifetime or list operation boundary
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and not line.lstrip().startswith('#') and ('AnimBlendLink_' in line or 'AnimBlendClump_' in line) and not any(value in line for value in ['BackendFrameSize(','BackendAllocate(','BackendFree(']):lines.append(re.match(r'\s*',line)[0]+'// Use the C animation owner at the original native operation boundary')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def anim_clump_project(source):
    # Register both complete C producers while preserving native source object distinctions
    source=source.replace('..\\src\\animation\\AnimBlendClumpData.cpp','..\\src\\animation\\AnimBlendClumpData.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\animation\\AnimBlendClumpData\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Keep existing metadata while requiring C11 for the renamed producer
            body=match[1] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="..\\src\\animation\\AnimBlendClumpData.c">'+body+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
        if '..\\src\\animation\\AnimBlendList.c"' not in source:source=source.replace('<ClCompile Include="..\\src\\animation\\AnimBlendClumpData.c"','<ClCompile Include="..\\src\\animation\\AnimBlendList.c"><CompileAs>CompileAsC</CompileAs><LanguageStandard_C>stdc11</LanguageStandard_C></ClCompile>\n    <ClCompile Include="..\\src\\animation\\AnimBlendClumpData.c"',1)
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendClumpData.h" />'
    for name in ['AnimBlendFrameData_native.h','AnimBlendClumpData_backend.h']:
        entry='<ClInclude Include="..\\src\\animation\\'+name+'" />'
        if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete clump ownership before the complete C frame record
    import anim_frame_migration as clumps
    return clumps.calls(anim_clump_calls(source))


def project(source):
    # Retain clump C11 integration before the complete C frame record
    import anim_frame_migration as clumps
    return clumps.project(anim_clump_project(source))
