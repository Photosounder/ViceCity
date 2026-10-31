"""Compare complete original sequence methods, custom heap boundaries and real frame consumers."""
from pathlib import Path
import json,re,sys
import anim_sequence_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-sequence-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
# Keep historical consumers on their exact pre-association schema while the current owner has its own complete fixture
if (tree/'src/animation/AnimBlendAssociation.c').exists():
    import anim_association_migration as associations
    frozen=root/'.diagnostics/anim-association-c-tests/before'
    for item in ['AnimBlendNode.h','AnimBlendNode.c','AnimBlendNode_backend.h','FrameUpdate.cpp']:
        assert current_path(tree,'src/animation/'+item).read_text()==associations.calls((frozen/'src/animation'/item).read_text()),item
    tree=frozen
for name in ['KeyFrames.h','KeyFrames.c','KeyFrames_native.h','AnimBlendAssociation_flags.h','AnimBlendFrameData.h','AnimBlendFrameData_native.h','AnimBlendClumpData.h','AnimBlendList.h']:(out/name).write_text((saved/'src/animation'/name).read_text(),newline='\n')
for name,path in [('Quaternion.h','src/math/Quaternion.h'),('Quaternion.c','src/math/Quaternion.c'),('Quaternion_native.h','src/math/Quaternion_native.h'),('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('Maths.h','src/math/maths.h')]:(out/name).write_text((saved/path).read_text(),newline='\n')
for name in ['AnimBlendSequence.h','AnimBlendNode.h','AnimBlendNode_backend.h','AnimBlendNode.c']:
    (out/name).write_text('#ifdef ORIGINAL\n'+(saved/'src/animation'/name).read_text()+'\n#else\n'+(tree/'src/animation'/name).read_text()+'\n#endif\n',newline='\n')
for name in ['AnimBlendSequence.c','AnimBlendSequence_backend.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
owners=(root/'utils/tests/anim_node_owners.h.in').read_text();owners=owners.replace('#ifdef ORIGINAL\n#include "original-AnimBlendNode.h"\n#else\n','#include "AnimBlendNode_backend.h"\n').replace('#include "AnimBlendNode_backend.h"\n#endif','#include "AnimBlendNode_backend.h"')
start=owners.index('struct RwFrame {');end=owners.index('extern CAnimBlendClumpData *gpAnimBlendClump;',start)
owners=owners[:start]+'''struct RwFrame {RwMatrix matrix;};
struct RpHAnimStdInterpFrame {struct {RwV3d imag;float real;} q;RwV3d t;};
#include "AnimBlendClumpData.h"
struct AnimBlendFrameUpdateData {bool foobar;CAnimBlendNode **nodes;};
'''+owners[end:]
owners=owners.replace('#define PUSH_MEMID(x)\n#define POP_MEMID()\n#define REGISTER_MEMPTR(x)\n','')
owners+='''
#define MEMID_ANIMATION 3
extern unsigned events[1024],eventCount;
extern size_t lastBytes;
extern unsigned moveMode;
extern CAnimBlendSequence *observedSequence;
void recordEvent(unsigned operation,void *memory);
struct TestHeap {
    void PushMemId(int id) {
        // Observe the actual original memory category without moving sequence policy into the fixture
        assert(id==3);recordEvent(3,NULL);
    }
    void PopMemId() {
        // Observe the original memory category restoration
        recordEvent(4,NULL);
    }
    void RegisterMemPointer(void *memory) {
        // Verify registration borrows a live owner pointer field
        assert(observedSequence&&(memory==&observedSequence->keyFrames||memory==&observedSequence->keyFramesCompressed));recordEvent(5,*(void**)memory);
    }
    void *MoveMemory(void *memory);
};
extern TestHeap gMainHeap;
#ifdef USE_CUSTOM_ALLOCATOR
#define PUSH_MEMID(x) gMainHeap.PushMemId(x)
#define POP_MEMID() gMainHeap.PopMemId()
#define REGISTER_MEMPTR(x) gMainHeap.RegisterMemPointer(x)
#else
#define PUSH_MEMID(x)
#define POP_MEMID()
#define REGISTER_MEMPTR(x)
#endif
'''
(out/'owners.h').write_text(owners,newline='\n')
entry='''#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void Run_Sequence(unsigned operation,void *self,char *name,int32_t count,bool translation,bool compressed);
void *Run_Frame(void *self,int32_t index,bool packed);
bool Run_HasTranslation(void *self);
bool Run_Move(void *self);
void Before_Sequence(unsigned,void*,char*,int32_t,bool,bool);
void *Before_Frame(void*,int32_t,bool);
bool Before_HasTranslation(void*);
bool Before_Move(void*);
void Run_Init(void*);
bool Run_Method(unsigned,void*,void*,void*,float);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
#include "AnimBlendNode.h"
#ifndef ORIGINAL
#include "AnimBlendSequence.h"
#endif
void Run_Sequence(unsigned operation,void *self,char *name,int32_t count,bool translation,bool compressed)
{
    // Exercise complete sequence ownership from an independently compiled strict C11 consumer
#ifdef ORIGINAL
    Before_Sequence(operation,self,name,count,translation,compressed);
#else
    CAnimBlendSequence *s=(CAnimBlendSequence*)self;
    switch(operation){case 0:AnimSequence_Init(s);break;case 1:AnimSequence_Destroy(s);break;case 2:AnimSequence_SetName(s,name);break;case 3:AnimSequence_SetNumFrames(s,count,translation,compressed);break;case 4:AnimSequence_RemoveQuaternionFlips(s);break;case 5:AnimSequence_Uncompress(s);break;case 6:AnimSequence_CompressKeyframes(s);break;case 7:AnimSequence_RemoveUncompressedData(s);break;default:AnimSequence_SetBoneTag(s,count);break;}
#endif
}
void *Run_Frame(void *self,int32_t index,bool packed)
{
    // Query actual common frame subobjects at the original packed or unpacked stride
#ifdef ORIGINAL
    return Before_Frame(self,index,packed);
#else
    return packed?(void*)AnimSequence_GetKeyFrameCompressed((CAnimBlendSequence*)self,index):(void*)AnimSequence_GetKeyFrame((CAnimBlendSequence*)self,index);
#endif
}
bool Run_HasTranslation(void *self)
{
    // Preserve the original bit test independently of frame count and pointer storage
#ifdef ORIGINAL
    return Before_HasTranslation(self);
#else
    return AnimSequence_HasTranslation((CAnimBlendSequence*)self);
#endif
}
bool Run_Move(void *self)
{
    // Exercise complete conditional relocation policy when the original custom heap is enabled
#ifdef USE_CUSTOM_ALLOCATOR
#ifdef ORIGINAL
    return Before_Move(self);
#else
    return AnimSequence_MoveMemory((CAnimBlendSequence*)self);
#endif
#else
    return false;
#endif
}
void Run_Init(void *self)
{
    // Initialize the actual C blend node before comparing complete native frame consumers
    AnimBlendNode_Init((CAnimBlendNode*)self);
}
bool Run_Method(unsigned op,void *self,void *trans,void *rot,float value)
{
    // Exercise existing C animation methods with either sequence ownership representation
    CAnimBlendNode *node=(CAnimBlendNode*)self;if(op==6)AnimBlendNode_CalcDeltas(node);else if(op==7)AnimBlendNode_CalcDeltasCompressed(node);else {unsigned char *p=(unsigned char*)trans;KeyFrameVectorOut out={(float*)p,(float*)(p+4),(float*)(p+8)};return AnimBlendNode_UpdateCompressed(node,out,(CQuaternion*)rot,value);}return false;
}
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/AnimBlendSequence.cpp').read_text(),flags=re.M)+'''
extern "C" void Before_Sequence(unsigned operation,void *self,char *name,int32_t count,bool translation,bool compressed)
{
    // Invoke complete original methods without replacing their algorithms with a model
    CAnimBlendSequence *s=(CAnimBlendSequence*)self;switch(operation){case 0:new(s) CAnimBlendSequence;break;case 1:s->~CAnimBlendSequence();break;case 2:s->SetName(name);break;case 3:s->SetNumFrames(count,translation,compressed);break;case 4:s->RemoveQuaternionFlips();break;case 5:s->Uncompress();break;case 6:s->CompressKeyframes();break;case 7:s->RemoveUncompressedData();break;default:s->SetBoneTag(count);break;}
}
extern "C" void *Before_Frame(void *self,int32_t index,bool packed)
{
    // Invoke the original getters on their actual native owner
    CAnimBlendSequence *s=(CAnimBlendSequence*)self;return packed?(void*)s->GetKeyFrameCompressed(index):(void*)s->GetKeyFrame(index);
}
extern "C" bool Before_HasTranslation(void *self)
{
    // Preserve the original flag-only query
    return ((CAnimBlendSequence*)self)->HasTranslation();
}
#ifdef USE_CUSTOM_ALLOCATOR
extern "C" bool Before_Move(void *self)
{
    // Invoke the complete original first-buffer relocation policy
    return ((CAnimBlendSequence*)self)->MoveMemory();
}
#endif
'''
math='#include "owners.h"\n#include "AnimBlendSequence_backend.h"\n'+(root/'utils/tests/anim_sequence_memory.cpp.in').read_text()+'\n#ifndef ORIGINAL\n'+m.backend()+'\n#endif\n'+method((saved/'src/animation/AnimBlendAssociation.cpp').read_text(),'AnimBlendNode_BackendAssociation')+'\n'
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math)]:(out/name).write_text(value,newline='\n')
before=re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M);after=re.sub(r'^#include[^\n]*\n','',(tree/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M)
(out/'FrameUpdate.cpp').write_text('#include "owners.h"\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
nodeFixture=(root/'utils/tests/anim_node.cpp.in').read_text();prefix=nodeFixture[:nodeFixture.index('static CAnimBlendNode staticNodes')];animation=method(nodeFixture,'animation')
animation=animation.replace('sequences[channel].type=CAnimBlendSequence::KF_ROT|((test&16)?CAnimBlendSequence::KF_TRANS:0);','Run_Sequence(0,&sequences[channel],NULL,0,false,false);sequences[channel].type=ANIM_KEYFRAME_ROT|((test&16)?ANIM_KEYFRAME_TRANS:0);').replace('sequences[channel].RemoveQuaternionFlips();','Run_Sequence(4,&sequences[channel],NULL,0,false,false);')
animation=animation.replace('data.flag=test&15;data.frame=&frame;data.hanimFrame=&skin;data.resetPos=CVector(0.5f,-1.25f,0.3f);','data.flag=(uint8_t)((test&15)<<1);if(which<=2||which==6)data.frame=&frame;else data.hanimFrame=&skin;data.resetPos={0.5f,-1.25f,0.3f};')
animation=animation.replace('clump.velocity2d=((test&256)&&(which==0||which==3||which>=6))?NULL:&velocity2;clump.velocity3d=&velocity3;','if((test&256)&&(which==0||which==3||which>=6))clump.velocity2d=NULL;else if(which==2||which==5||which==6||which==7||((which==0||which==3)&&(test&12)==12))clump.velocity3d=&velocity3;else clump.velocity2d=&velocity2;')
animation=animation.replace('        ++states;','        // Detach borrowed keyframe arrays before automatic original sequence destruction\n        for(unsigned channel=0;channel<2;++channel){sequences[channel].keyFrames=NULL;sequences[channel].keyFramesCompressed=NULL;}\n        ++states;')
(out/'fixture.cpp').write_text(prefix+'\n'+animation+'\n'+(root/'utils/tests/anim_sequence.cpp.in').read_text(),newline='\n')
print('Prepared all thirteen original/C sequence operations, actual custom heap boundaries and all nine complete frame callbacks')
