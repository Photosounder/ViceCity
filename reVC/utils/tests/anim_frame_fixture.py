"""Compare the real frame schema, C traversal and all nine native animation callbacks."""
from pathlib import Path
import json,re,sys
import anim_frame_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-frame-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
if (tree/'src/animation/AnimBlendSequence.c').exists():
    tree=root/'.diagnostics/anim-sequence-c-tests/before'
    for name in json.loads((out/'changes.json').read_text()):assert (tree/name).read_text()==m.anim_frame_calls((saved/name).read_text()),name
    # Keep historical consumers on their exact pre-association schema while the current owner has its own complete fixture
if (tree/'src/animation/AnimBlendAssociation.c').exists():
    import anim_association_migration as associations
    frozen=root/'.diagnostics/anim-association-c-tests/before'
    for item in ['AnimBlendNode.h','AnimBlendNode.c','AnimBlendNode_backend.h','FrameUpdate.cpp']:
        assert current_path(tree,'src/animation/'+item).read_text()==associations.calls((frozen/'src/animation'/item).read_text()),item
    tree=frozen
for name in ['AnimBlendNode.h','AnimBlendNode.c','AnimBlendNode_backend.h','AnimBlendSequence.h']:assert (tree/'src/animation'/name).read_text()==(saved/'src/animation'/name).read_text(),name
for name in ['AnimBlendNode.h','AnimBlendNode.c','AnimBlendNode_backend.h','AnimBlendAssociation_flags.h','KeyFrames.h','KeyFrames.c','KeyFrames_native.h','AnimBlendList.h','AnimBlendList.c','AnimBlendFrameData.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
for name,path in [('Quaternion.h','src/math/Quaternion.h'),('Quaternion.c','src/math/Quaternion.c'),('Quaternion_native.h','src/math/Quaternion_native.h'),('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('Maths.h','src/math/maths.h'),('AnimBlendSequence.h','src/animation/AnimBlendSequence.h')]:(out/name).write_text((saved/path).read_text(),newline='\n')
for name in ['AnimBlendClumpData.h','AnimBlendClumpData.c','AnimBlendClumpData_backend.h','AnimBlendFrameData_native.h']:
    before=(saved/'src/animation'/name).read_text();after=(tree/'src/animation'/name).read_text();(out/name).write_text('#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
(out/'config.h').write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
owners=(root/'utils/tests/anim_node_owners.h.in').read_text();owners=owners.replace('#ifdef ORIGINAL\n#include "original-AnimBlendNode.h"\n#else\n','#include "AnimBlendNode_backend.h"\n').replace('#include "AnimBlendNode_backend.h"\n#endif','#include "AnimBlendNode_backend.h"')
start=owners.index('struct RwFrame {');end=owners.index('extern CAnimBlendClumpData *gpAnimBlendClump;',start)
owners=owners[:start]+'''struct RwFrame {RwMatrix matrix;int32_t hierarchyId;};
typedef uintptr_t uintptr;
struct RtQuat {RwV3d imag;float real;};
struct RpHAnimStdInterpFrame {RtQuat q;RwV3d t;};
#include "AnimBlendClumpData.h"
#include "AnimBlendClumpData_backend.h"
struct AnimBlendFrameUpdateData {bool foobar;CAnimBlendNode **nodes;};
'''+owners[end:]
owners+='''
#define RADTODEG(x) ((x)*(180.0f/PI))
enum {rwCOMBINEREPLACE=0,rwCOMBINEPRECONCAT=1};
struct LimbOrientation {float yaw,pitch;};
struct CPed {bool bDontAcceptIKLookAts;};
struct CPedIK {CPed *m_ped;void RotateTorso(AnimBlendFrameData*,LimbOrientation*,bool);};
extern RtQuat *expectedPose;
extern unsigned poseCalls;
static inline void RtQuatRotate(RtQuat *q,const RwV3d *axis,float angle,int operation)
{
    // Verify the actual IK callback writes directly into its original live SDK pose
    assert(q==expectedPose);++poseCalls;q->imag.x+=axis->x*angle;q->imag.y+=axis->y*angle;q->imag.z+=axis->z*angle;q->real=(float)operation;
}
static inline RwV3d *RwMatrixGetPos(RwMatrix *matrix)
{
    // Retain the actual native position subobject used by the initialization callback
    return &matrix->pos;
}
class CVisibilityPlugins {public:static int32 GetFrameHierarchyId(RwFrame *frame)
{
    // Read the original native hierarchy identity through the recovered frame pointer
    return frame->hierarchyId;
}};
extern AnimBlendFrameData *pFrameDataFound;
extern unsigned allocations,frees;
extern uint32_t lastSize,lastAlignment;
'''
(out/'owners.h').write_text(owners,newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Run_Init(void*);
bool Run_Method(unsigned,void*,void*,void*,float);
void Run_FrameWrite(void*,uint8_t,const uint32_t*,void*,int32_t,bool);
void Run_FrameCopy(void*,const void*);
void Before_FrameWrite(void*,uint8_t,const uint32_t*,void*,int32_t,bool);
void Before_FrameCopy(void*,const void*);
void *NativeFrameDefault(void*);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
#include "AnimBlendNode.h"
#include <string.h>
#ifndef ORIGINAL
#include "AnimBlendFrameData.h"
#endif
void Run_Init(void *self)
{
    // Initialize the unchanged complete C blend node used by both frame schemas
    AnimBlendNode_Init((CAnimBlendNode*)self);
}
bool Run_Method(unsigned op,void *self,void *trans,void *rot,float value)
{
    // Invoke the existing C animation algorithms from an independent strict C11 consumer
    CAnimBlendNode *node=(CAnimBlendNode*)self;
    if(op==6)AnimBlendNode_CalcDeltas(node);
    else if(op==7)AnimBlendNode_CalcDeltasCompressed(node);
    else {unsigned char *p=(unsigned char*)trans;KeyFrameVectorOut out={(float*)p,(float*)(p+4),(float*)(p+8)};return AnimBlendNode_UpdateCompressed(node,out,(CQuaternion*)rot,value);}
    return false;
}
void Run_FrameWrite(void *self,uint8_t flag,const uint32_t *position,void *handle,int32_t id,bool skin)
{
    // Exercise complete frame materialization through its actual public C representation
#ifdef ORIGINAL
    Before_FrameWrite(self,flag,position,handle,id,skin);
#else
    uint32_t bits[3];memcpy(bits,position,12);AnimBlendFrameData *frame=(AnimBlendFrameData*)self;frame->flag=flag;memcpy(&frame->resetPos,bits,12);if(skin)frame->hanimFrame=handle;else frame->frame=handle;frame->nodeID=id;
#endif
}
void Run_FrameCopy(void *target,const void *source)
{
    // Exercise ordinary value copying without assigning a policy to unspecified padding
#ifdef ORIGINAL
    Before_FrameCopy(target,source);
#else
    *(AnimBlendFrameData*)target=*(const AnimBlendFrameData*)source;
#endif
}
'''
math='''#include "owners.h"
#include "entry.h"
#include <malloc.h>
unsigned allocations=0,frees=0,poseCalls=0;uint32_t lastSize=0,lastAlignment=0;
RtQuat *expectedPose=NULL;AnimBlendFrameData *pFrameDataFound=NULL;
const RwV3d XaxisIK={1,0,0},ZaxisIK={0,0,1};
extern "C" void *NativeFrameDefault(void *memory)
{
    // Keep default construction opaque while checking the original no-write value lifetime
    return new(memory) AnimBlendFrameData;
}
#ifdef ORIGINAL
extern "C" void Before_FrameWrite(void *self,uint8_t flag,const uint32_t *position,void *handle,int32_t id,bool skin)
{
    // Materialize the original SDK-dependent record without changing position bits
    uint32_t bits[3];memcpy(bits,position,12);AnimBlendFrameData *frame=(AnimBlendFrameData*)self;frame->flag=flag;memcpy(&frame->resetPos,bits,12);if(skin)frame->hanimFrame=(RpHAnimStdInterpFrame*)handle;else frame->frame=(RwFrame*)handle;frame->nodeID=id;
}
extern "C" void Before_FrameCopy(void *target,const void *source)
{
    // Invoke the complete original trivial frame copy
    *(AnimBlendFrameData*)target=*(const AnimBlendFrameData*)source;
}
size_t AnimBlendClump_BackendFrameSize(void)
{
    // Preserve the original native stride query only in the original comparison executable
    return sizeof(AnimBlendFrameData);
}
#endif
void *AnimBlendClump_BackendAllocate(uint32_t size,uint32_t alignment)
{
    // Record the original narrowed byte count and alignment while bounding diagnostic allocations
    ++allocations;lastSize=size;lastAlignment=alignment;void *p=_aligned_malloc(64*sizeof(AnimBlendFrameData),alignment);assert(p);for(unsigned i=0;i<64;++i)new((AnimBlendFrameData*)p+i) AnimBlendFrameData;memset(p,0xa5,64*sizeof(AnimBlendFrameData));return p;
}
void AnimBlendClump_BackendFree(void *memory)
{
    // Retain the original native aligned allocation boundary
    ++frees;_aligned_free(memory);
}
'''
sequence=(saved/'src/animation/AnimBlendSequence.cpp').read_text();association=(saved/'src/animation/AnimBlendAssociation.cpp').read_text()
math+=method(sequence,'CAnimBlendSequence::CAnimBlendSequence')+'\nCAnimBlendSequence::~CAnimBlendSequence() {}\n'+method(sequence,'CAnimBlendSequence::RemoveQuaternionFlips')+'\n'+method(sequence,'AnimBlendNode_BackendSequence')+'\n'+method(association,'AnimBlendNode_BackendAssociation')+'\n'
for phase,base in [('before',saved),('after',tree)]:
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n');rp=(base/'src/animation/RpAnimBlend.cpp').read_text()
    for name in ['FrameInitCBnonskin','FrameInitCBskin','FillFrameArrayCBnonskin','FrameFindByBoneCB']:math+=method(rp,name)+'\n'
    math+=method((base/'src/peds/PedIK.cpp').read_text(),'CPedIK::RotateTorso')+'\n'
math+='#endif\n'
for name,value in [('entry.h',entry),('caller.c',caller),('math.cpp',math)]:(out/name).write_text(value,newline='\n')
before=re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M);after=re.sub(r'^#include[^\n]*\n','',(tree/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M)
(out/'FrameUpdate.cpp').write_text('#include "owners.h"\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
fixture=(root/'utils/tests/anim_node.cpp.in').read_text();prefix=fixture[:fixture.index('static CAnimBlendNode staticNodes')];animation=method(fixture,'animation')
old='data.flag=test&15;data.frame=&frame;data.hanimFrame=&skin;data.resetPos=CVector(0.5f,-1.25f,0.3f);'
new='data.flag=(uint8_t)((test&15)<<1);if(which<=2||which==6)data.frame=&frame;else data.hanimFrame=&skin;data.resetPos={0.5f,-1.25f,0.3f};'
assert old in animation;animation=animation.replace(old,new)
old='clump.velocity2d=((test&256)&&(which==0||which==3||which>=6))?NULL:&velocity2;clump.velocity3d=&velocity3;'
new='if((test&256)&&(which==0||which==3||which>=6))clump.velocity2d=NULL;else if(which==2||which==5||which==6||which==7||((which==0||which==3)&&(test&12)==12))clump.velocity3d=&velocity3;else clump.velocity2d=&velocity2;'
assert old in animation;animation=animation.replace(old,new)
(out/'fixture.cpp').write_text(prefix+'\n'+animation+'\n'+(root/'utils/tests/anim_frame.cpp.in').read_text(),newline='\n')
print('Prepared complete C frame records, live clump traversal, four actual initialization callbacks, nine animation callbacks and torso IK')
