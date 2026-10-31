"""Prepare independent C callers for every complete original/converted association operation."""
from pathlib import Path
import json,re,sys
import anim_association_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-association-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
for name in ['AnimBlendSequence.h','AnimBlendSequence.c','AnimBlendSequence_backend.h','AnimBlendHierarchy.h','AnimBlendList.h','AnimBlendList.c','AnimBlendClumpData.h','AnimBlendFrameData.h','AnimBlendAssociation_flags.h','KeyFrames.h','KeyFrames.c']:(out/name).write_text((saved/'src/animation'/name).read_text(),newline='\n')
for name,path in [('Quaternion.h','src/math/Quaternion.h'),('Quaternion.c','src/math/Quaternion.c'),('LinkList.h','src/core/LinkList.h'),('Quaternion_native.h','src/math/Quaternion_native.h'),('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('Maths.h','src/math/maths.h'),('KeyFrames_native.h','src/animation/KeyFrames_native.h')]:(out/name).write_text((saved/path).read_text(),newline='\n')
for name in ['AnimBlendAssociation.h','AnimBlendNode.h','AnimBlendNode_backend.h','AnimBlendNode.c']:(out/name).write_text('#ifdef ORIGINAL\n'+(saved/'src/animation'/name).read_text()+'\n#else\n'+(tree/'src/animation'/name).read_text()+'\n#endif\n',newline='\n')
for name in ['AnimBlendAssociation.c','AnimBlendAssociation_backend.h','AnimBlendClumpData_backend.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
entry='''#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
struct CAnimBlendAssociation;
typedef void (*TestCallback)(struct CAnimBlendAssociation*,void*);
typedef struct AssociationInput {void *self,*other,*clump,*hierarchy,*argument;TestCallback callback;float x,y;int32_t count;} AssociationInput;
typedef struct AssociationResult {float number;void *pointer;bool boolean;} AssociationResult;
#ifdef __cplusplus
extern "C" {
#endif
AssociationResult Run_Association(unsigned operation,AssociationInput input);
AssociationResult Before_Association(unsigned operation,AssociationInput input);
#ifdef __cplusplus
}
#endif
'''
values={'self':'(CAnimBlendAssociation*)input.self','other':'(CAnimBlendAssociation*)input.other','clump':'input.clump','hier':'(CAnimBlendHierarchy*)input.hierarchy','amount':'input.x','delta':'input.y','cb':'(void(*)(CAnimBlendAssociation*,void*))input.callback','arg':'input.argument','time':'input.x','timeDelta':'input.x','relSpeed':'input.y','weight':'input.x','i':'input.count','n':'input.count','link':'(CAnimBlendLink*)input.self'}
cases=[];before=[]
for index,(result,name,args) in enumerate(m.DECLARATIONS):
    parameters=[]
    for arg in args.split(','):parameters.append(values[re.search(r'(\w+)$',arg)[1]])
    call='AnimAssociation_'+name+'('+','.join(parameters)+')'
    destination='answer.number=' if result=='float' else 'answer.boolean=' if result=='bool' else 'answer.pointer=' if '*' in result else ''
    cases.append('case '+str(index)+':'+destination+call+';break;')
    obj='((CAnimBlendAssociation*)input.self)'
    if name=='Init':call='new(input.self) CAnimBlendAssociation'
    elif name=='CopyConstruct':call='new(input.self) CAnimBlendAssociation(*(CAnimBlendAssociation*)input.other)'
    elif name=='Destroy':call=obj+'->~CAnimBlendAssociation()'
    elif name=='BindClump':call=obj+'->Init((RpClump*)input.clump,(CAnimBlendHierarchy*)input.hierarchy)'
    elif name=='InitFrom':call=obj+'->Init(*(CAnimBlendAssociation*)input.other)'
    elif name=='FromLink':call='CAnimBlendAssociation::FromLink((CAnimBlendLink*)input.self)'
    else:call=obj+'->'+name+'('+','.join(parameters[1:])+')'
    before.append('case '+str(index)+':'+destination+call+';break;')
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "AnimBlendAssociation.h"
#endif
AssociationResult Run_Association(unsigned operation,AssociationInput input)
{
    // Exercise every complete operation from a separately compiled strict C11 consumer
#ifdef ORIGINAL
    return Before_Association(operation,input);
#else
    AssociationResult answer={0,NULL,false};switch(operation){
'''+''.join(cases)+'''}return answer;
#endif
}
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/AnimBlendAssociation.cpp').read_text(),flags=re.M)+'''
extern "C" AssociationResult Before_Association(unsigned operation,AssociationInput input)
{
    // Invoke original constructors and complete methods without translating their algorithms into a model
    AssociationResult answer={0,NULL,false};switch(operation){
'''+''.join(before)+'''}return answer;
}
'''
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original)]:(out/name).write_text(value,newline='\n')
(out/'operation-map.json').write_text(json.dumps({name:i for i,(_,name,_) in enumerate(m.DECLARATIONS)},indent=2),newline='\n')
print('Prepared independent strict C11 dispatch and original native dispatch for all 28 association operations')
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>
#include <new>
typedef int16_t int16;typedef int32_t int32;typedef uint8_t uint8;typedef uint32_t uint32;
#define nil NULL
#define VALIDATE_SIZE(a,b)
#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))
struct RwV3d {float x,y,z;};
struct RwMatrix {RwV3d right;uint32_t flags;RwV3d up;uint32_t pad1;RwV3d at;uint32_t pad2;RwV3d pos;uint32_t pad3;};
struct RwFrame {RwMatrix matrix;};
struct RpHAnimStdInterpFrame {struct {RwV3d imag;float real;} q;RwV3d t;};
struct RpClump;
#define PI ((float)3.1415926535897932384626433832795)
#define TWOPI (PI*2)
#include "Maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "Quaternion_native.h"
#include "KeyFrames_native.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendNode_backend.h"
#include "AnimBlendClumpData.h"
struct RpClump {CAnimBlendClumpData *data;};
#define RPANIMBLENDCLUMPDATA(clump) (&((RpClump*)(clump))->data)
void *RwMallocAlign(uint32_t,uint32_t);
void RwFreeAlign(void*);
AnimBlendFrameData *RpAnimBlendClumpFindFrame(RpClump*,const char*);
AnimBlendFrameData *RpAnimBlendClumpFindBone(RpClump*,uint32_t);
class CAnimManager {public:static void UncompressAnimation(CAnimBlendHierarchy*);};
extern unsigned allocations,frees,hostCalls;
extern unsigned lastBytes,lastAlignment,freeWasLinked;
extern CAnimBlendAssociation *observedOwner;
struct AnimBlendFrameUpdateData {int foobar;CAnimBlendNode *nodes[16];};
extern CAnimBlendClumpData *gpAnimBlendClump;
'''
nativeOwners=(root/'utils/tests/anim_node_owners.h.in').read_text();owners+=nativeOwners[nativeOwners.index('static inline RwMatrix *RwFrameGetMatrix'):]
(out/'owners.h').write_text(owners,newline='\n')
(out/'AnimBlendFrameData_native.h').write_text((saved/'src/animation/AnimBlendFrameData_native.h').read_text(),newline='\n')
math='''#include "owners.h"
#include "AnimBlendAssociation_backend.h"
#include "AnimBlendClumpData_backend.h"
unsigned allocations=0,frees=0,hostCalls=0;
unsigned lastBytes=0,lastAlignment=0,freeWasLinked=0;
CAnimBlendAssociation *observedOwner=NULL;
void *RwMallocAlign(uint32_t bytes,uint32_t alignment)
{
    // Keep actual aligned storage while bounding later allocation-width boundary cases
    ++allocations;lastBytes=bytes;lastAlignment=alignment;void *memory=_aligned_malloc(64*sizeof(CAnimBlendNode),alignment);assert(memory);memset(memory,0xa5,64*sizeof(CAnimBlendNode));return memory;
}
void RwFreeAlign(void *memory)
{
    // Preserve the real aligned release boundary independently of owner algorithms
    ++frees;if(observedOwner&&observedOwner->link.prev)++freeWasLinked;_aligned_free(memory);
}
void *AnimBlendClump_BackendAllocate(uint32_t bytes,uint32_t alignment)
{
    // Reuse the same native primitive used by the actual C association
    return RwMallocAlign(bytes,alignment);
}
void AnimBlendClump_BackendFree(void *memory)
{
    // Forward the original native free primitive
    RwFreeAlign(memory);
}
AnimBlendFrameData *RpAnimBlendClumpFindFrame(RpClump *clump,const char *name)
{
    // Supply deterministic native name lookup without changing binding policy
    ++hostCalls;unsigned index=(unsigned char)name[0];return index<(unsigned)clump->data->numFrames?&clump->data->frames[index]:NULL;
}
AnimBlendFrameData *RpAnimBlendClumpFindBone(RpClump *clump,uint32_t tag)
{
    // Retain the exact unsigned native bone lookup argument
    ++hostCalls;for(int i=0;i<clump->data->numFrames;++i)if((uint32_t)clump->data->frames[i].nodeID==tag)return &clump->data->frames[i];return NULL;
}
void CAnimManager::UncompressAnimation(CAnimBlendHierarchy *hierarchy)
{
    // Observe the external manager boundary before association node setup
    ++hostCalls;
}
extern "C" void *AnimSequence_BackendAllocate(size_t bytes)
{
    // Provide native sequence allocation for the actual linked C dependencies
    return malloc(bytes);
}
extern "C" void AnimSequence_BackendFree(void *memory)
{
    // Release actual sequence storage through its native primitive
    free(memory);
}
extern "C" void AnimSequence_BackendPushAnimation(void) {}
extern "C" void AnimSequence_BackendPopMemory(void) {}
extern "C" void AnimSequence_BackendRegister(void **memory) {}
'''+m.sdk_backend()+m.manager_backend()
(out/'math.cpp').write_text(math,newline='\n')
(out/'fixture.cpp').write_text((root/'utils/tests/anim_association.cpp.in').read_text(),newline='\n')
nodeFixture=(root/'utils/tests/anim_node.cpp.in').read_text();prefix=nodeFixture[:nodeFixture.index('static CAnimBlendNode staticNodes')];animation=method(nodeFixture,'animation')
animation=animation.replace('CAnimBlendSequence::KF_ROT','ANIM_KEYFRAME_ROT').replace('CAnimBlendSequence::KF_TRANS','ANIM_KEYFRAME_TRANS').replace('sequences[channel].RemoveQuaternionFlips();','AnimSequence_RemoveQuaternionFlips(&sequences[channel]);AssociationInput init={};init.self=&associations[channel];Run_Association(0,init);')
animation=animation.replace('data.flag=test&15;data.frame=&frame;data.hanimFrame=&skin;data.resetPos=CVector(0.5f,-1.25f,0.3f);','data.flag=(uint8_t)((test&15)<<1);if(which<=2||which==6)data.frame=&frame;else data.hanimFrame=&skin;data.resetPos={0.5f,-1.25f,0.3f};')
animation=animation.replace('AnimBlendFrameUpdateData update={(test&128)!=0,pointers};','AnimBlendFrameUpdateData update={};update.foobar=(test&128)!=0;memcpy(update.nodes,pointers,sizeof(pointers));')
animation=animation.replace('clump.velocity2d=((test&256)&&(which==0||which==3||which>=6))?NULL:&velocity2;clump.velocity3d=&velocity3;','if((test&256)&&(which==0||which==3||which>=6))clump.velocity2d=NULL;else if(which==2||which==5||which==6||which==7||((which==0||which==3)&&(test&12)==12))clump.velocity3d=&velocity3;else clump.velocity2d=&velocity2;')
animation=animation.replace('callbacks[which](&data,&update);','callbacks[which](&data,&update);memcpy(pointers,update.nodes,sizeof(pointers));')
helpers='''static void Run_Init(void *self)
{
    // Invoke the actual C node initializer before native frame accumulation
    AnimBlendNode_Init((CAnimBlendNode*)self);
}
static bool Run_Method(unsigned operation,void *self,void *translation,void *rotation,float value)
{
    // Exercise the real C interpolation methods beneath all nine native callbacks
    CAnimBlendNode *node=(CAnimBlendNode*)self;if(operation==6)AnimBlendNode_CalcDeltas(node);else if(operation==7)AnimBlendNode_CalcDeltasCompressed(node);else return AnimBlendNode_UpdateCompressed(node,KeyFrame_VectorOut(*(CVector*)translation),(CQuaternion*)rotation,value);return false;
}
'''
finish='''extern "C" uint64_t Association_FrameConsumers(void)
{
    // Return the complete consumer observation hash to the main original/C comparison
    animation();require(states==18432);return hashValue;
}
'''
(out/'frame-fixture.cpp').write_text(prefix+helpers+animation+finish,newline='\n')
framePath=current_path(tree,'src/animation/FrameUpdate.cpp')
if framePath.suffix=='.c':
    # Freeze the exact previous native consumer only after proving its entire current C successor
    import frame_update_migration as frameOwner
    framePath=root/'.diagnostics/frame-update-c-tests/before/src/animation/FrameUpdate.cpp'
    assert framePath.read_text()==m.anim_association_calls((saved/'src/animation/FrameUpdate.cpp').read_text())
    assert frameOwner.kernel(framePath.read_text())==current_path(tree,'src/animation/FrameUpdate.cpp').read_text()
before=re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M);after=re.sub(r'^#include[^\n]*\n','',framePath.read_text(),flags=re.M)
(out/'FrameUpdate.cpp').write_text('#include "owners.h"\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
