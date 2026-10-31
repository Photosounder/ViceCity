"""Compare full original animation links, clump lifetime and live callback traversal against C11."""
from pathlib import Path
import json,re,sys
import anim_clump_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-clump-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
if (tree/'src/animation/AnimBlendFrameData.h').exists():
    tree=root/'.diagnostics/anim-frame-c-tests/before'
    for name in json.loads((out/'changes.json').read_text()):assert (tree/m.RENAMES.get(name,name)).read_text()==m.anim_clump_calls((saved/name).read_text()),name
    assert (tree/'src/animation/AnimBlendFrameData_native.h').read_text()==m.frame_header((saved/'src/animation/AnimBlendClumpData.h').read_text())
    assert (tree/'src/animation/AnimBlendClumpData_backend.h').read_text()==m.backend_header()
for name in ['AnimBlendList.h','AnimBlendList.c','AnimBlendClumpData.h','AnimBlendClumpData.c','AnimBlendClumpData_backend.h','AnimBlendFrameData_native.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
for name in ['AnimBlendList.h','AnimBlendClumpData.h']:
    source=(saved/'src/animation'/name).read_text()
    if name=='AnimBlendClumpData.h':source=source.replace('#include "AnimBlendList.h"','#include "original-AnimBlendList.h"')
    (out/('original-'+name)).write_text(source,newline='\n')
(out/'owners.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <new>
#include <memory>
#include <assert.h>
typedef uint8_t uint8;typedef int32_t int32;typedef uint32_t RwUInt32;typedef int32_t RwInt32;typedef uintptr_t uintptr;
#define nil NULL
#define VALIDATE_SIZE(a,b)
struct RwV3d {float x,y,z;};
struct RwMatrix {RwV3d right;uint32_t flags;RwV3d up;uint32_t pad1;RwV3d at;uint32_t pad2;RwV3d pos;uint32_t pad3;};
struct RwFrame {RwMatrix matrix;int32 hierarchyId;};
struct RpHAnimStdInterpFrame {float value[7];};
class CVector {public:float x,y,z;};
class CVector2D {public:float x,y;};
#ifdef ORIGINAL
#include "original-AnimBlendClumpData.h"
#else
#include "AnimBlendClumpData.h"
#include "AnimBlendClumpData_backend.h"
#endif
struct RpClump {CAnimBlendClumpData *data;};
#define RPANIMBLENDCLUMPDATA(value) (&((RpClump*)(value))->data)
#define RWPLUGINOFFSET(type,value,offset) ((type*)((unsigned char*)(value)+(offset)))
void *RwMallocAlign(RwUInt32 size,RwUInt32 align);
void RwFreeAlign(void *memory);
extern CAnimBlendClumpData *observedOwner;
extern unsigned allocations,frees;
extern uint32_t lastSize,lastAlignment;
extern unsigned events[4096],eventCount;
extern AnimBlendFrameData *pFrameDataFound;
void RpAnimBlendClumpRemoveAllAssociations(RpClump *clump);
void RpAnimBlendAllocateData(RpClump *clump);
void *AnimBlendClumpDestroy(void *object,RwInt32 offset,RwInt32 size);
static inline RwMatrix *RwFrameGetMatrix(RwFrame *frame)
{
    // Supply the original native matrix lookup used by actual frame initialization callbacks
    return &frame->matrix;
}
static inline RwV3d *RwMatrixGetPos(RwMatrix *matrix)
{
    // Supply the actual native position subobject to the original callback
    return &matrix->pos;
}
class CVisibilityPlugins {
public:
    static int32 GetFrameHierarchyId(RwFrame *frame) {
        // Supply the native hierarchy identity without changing the original callback body
        return frame->hierarchyId;
    }
};
''',newline='\n')
entry='''#pragma once
#include <stdint.h>
typedef struct AnimBlendFrameData AnimBlendFrameData;
typedef void (*CheckFrameCallback)(AnimBlendFrameData*,void*);
#ifdef __cplusplus
extern "C" {
#endif
void Run_Link(unsigned operation,void *self,void *other);
void Run_Init(void *self);
void Run_Destroy(void *self);
void Run_Set(void *self,int32_t count,unsigned bones);
void Run_Each(void *self,CheckFrameCallback callback,void *arg);
void Before_Link(unsigned operation,void *self,void *other);
void Before_Init(void *self);
void Before_Destroy(void *self);
void Before_Set(void *self,int32_t count,unsigned bones);
void Before_Each(void *self,CheckFrameCallback callback,void *arg);
#ifdef __cplusplus
}
#endif
'''
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "AnimBlendClumpData.h"\n#endif\n';original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/AnimBlendClumpData.cpp').read_text(),flags=re.M)+'\n'
ops=[('Link','unsigned operation,void *self,void *other','operation,self,other','CAnimBlendLink *link=(CAnimBlendLink*)self;if(operation==0)link->Init();else if(operation==1)link->Prepend((CAnimBlendLink*)other);else link->Remove();','if(operation==0)AnimBlendLink_Init((CAnimBlendLink*)self);else if(operation==1)AnimBlendLink_Prepend((CAnimBlendLink*)self,(CAnimBlendLink*)other);else AnimBlendLink_Remove((CAnimBlendLink*)self);'),('Init','void *self','self','new(self) CAnimBlendClumpData;','AnimBlendClump_Init((CAnimBlendClumpData*)self);'),('Destroy','void *self','self','((CAnimBlendClumpData*)self)->~CAnimBlendClumpData();','AnimBlendClump_Destroy((CAnimBlendClumpData*)self);'),('Set','void *self,int32_t count,unsigned bones','self,count,bones','if(bones)((CAnimBlendClumpData*)self)->SetNumberOfBones(count);else ((CAnimBlendClumpData*)self)->SetNumberOfFrames(count);','if(bones)AnimBlendClump_SetNumberOfBones((CAnimBlendClumpData*)self,count);else AnimBlendClump_SetNumberOfFrames((CAnimBlendClumpData*)self,count);'),('Each','void *self,CheckFrameCallback callback,void *arg','self,callback,arg','((CAnimBlendClumpData*)self)->ForAllFrames(callback,arg);','AnimBlendClump_ForAllFrames((CAnimBlendClumpData*)self,callback,arg);')]
for name,args,values,old,new in ops:
    original+='extern "C" void Before_'+name+'('+args+')\n{\n    // Invoke the complete original owner operation including its native lifetime and pointer updates\n    '+old+'\n}\n'
    caller+='void Run_'+name+'('+args+')\n{\n    // Exercise the public operation through an independently compiled strict C11 caller\n#ifdef ORIGINAL\n    Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
math='''#include "owners.h"
unsigned allocations=0,frees=0,eventCount=0,events[4096];uint32_t lastSize=0,lastAlignment=0;
CAnimBlendClumpData *observedOwner=NULL;
AnimBlendFrameData *pFrameDataFound=NULL;
void *RwMallocAlign(RwUInt32 size,RwUInt32 alignment)
{
    // Record the exact native argument widths and owner state while keeping boundary-size tests bounded
    ++allocations;lastSize=size;lastAlignment=alignment;events[eventCount++]=1;events[eventCount++]=observedOwner?(uint32_t)observedOwner->numFrames:0;
    void *memory=_aligned_malloc(64*sizeof(AnimBlendFrameData),alignment);assert(memory);for(unsigned i=0;i<64;++i)new((AnimBlendFrameData*)memory+i) AnimBlendFrameData;memset(memory,0xa5,64*sizeof(AnimBlendFrameData));return memory;
}
void RwFreeAlign(void *memory)
{
    // Observe old frame state before the original aligned free boundary
    assert(memory);++frees;events[eventCount++]=2;events[eventCount++]=observedOwner?(uint32_t)observedOwner->numFrames:0;_aligned_free(memory);
}
void RpAnimBlendClumpRemoveAllAssociations(RpClump *clump)
{
    // Preserve the external association-removal boundary before clump cleanup in the native plugin
    events[eventCount++]=3;CAnimBlendLink *head=&clump->data->link;
    while(head->next){
        // Detach one actual association link through the owner under test
#ifdef ORIGINAL
        head->next->Remove();
#else
        AnimBlendLink_Remove(head->next);
#endif
    }
}
extern "C" void *NativeLinkDefault(void *memory)
{
    // Keep original trivial default construction opaque to the initialized-storage fixture
    return new(memory) CAnimBlendLink;
}
'''
rp=(saved/'src/animation/RpAnimBlend.cpp').read_text()
for phase,base in [('before',saved),('after',tree)]:
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')
    source=(base/'src/animation/RpAnimBlend.cpp').read_text()
    for name in ['RpAnimBlendAllocateData','AnimBlendClumpDestroy']:math+=method(source,name)+'\n'
math+='#endif\n#ifndef ORIGINAL\n'+m.backend()+'\n#endif\n'
for name in ['FrameInitCBnonskin','FrameInitCBskin','FillFrameArrayCBnonskin','FrameFindByBoneCB']:math+=method(rp,name)+'\n'
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/anim_clump.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all original/C link and clump operations, native plugin lifetimes and four actual frame callbacks')
