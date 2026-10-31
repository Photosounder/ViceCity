"""Compare all thirteen original node methods with C and complete native animation consumers."""
from pathlib import Path
import json,re,sys
import anim_node_migration as m
from doors_migration import method
from stats_successor import current_path
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/anim-node-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
if (tree/'src/animation/AnimBlendSequence.c').exists():
    tree=root/'.diagnostics/anim-sequence-c-tests/before'
    assert (tree/'src/animation/AnimBlendNode.h').read_text()==m.header()
    assert (tree/'src/animation/AnimBlendNode.c').read_text()==m.kernel((saved/'src/animation/AnimBlendNode.cpp').read_text())
    assert (tree/'src/animation/AnimBlendNode_backend.h').read_text()==m.backend_header()
    assert (tree/'src/animation/AnimBlendSequence.h').read_text()==m.anim_node_calls((saved/'src/animation/AnimBlendSequence.h').read_text())
# Keep historical consumers on their exact pre-association schema while the current owner has its own complete fixture
if (tree/'src/animation/AnimBlendAssociation.c').exists():
    import anim_association_migration as associations
    frozen=root/'.diagnostics/anim-association-c-tests/before'
    for item in ['AnimBlendNode.h','AnimBlendNode.c','AnimBlendNode_backend.h','FrameUpdate.cpp']:
        assert current_path(tree,'src/animation/'+item).read_text()==associations.calls((frozen/'src/animation'/item).read_text()),item
    tree=frozen
for name in ['AnimBlendNode.h','AnimBlendNode.c','AnimBlendNode_backend.h','AnimBlendAssociation_flags.h','KeyFrames.h','KeyFrames.c','KeyFrames_native.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
for name,path in [('Quaternion.h','src/math/Quaternion.h'),('Quaternion.c','src/math/Quaternion.c'),('Quaternion_native.h','src/math/Quaternion_native.h'),('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('Maths.h','src/math/maths.h')]:
    (out/name).write_text((saved/path).read_text(),newline='\n')
for name in ['AnimBlendSequence.h']:
    before=(saved/'src/animation'/name).read_text();after=(tree/'src/animation'/name).read_text();(out/name).write_text('#pragma once\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
(out/'original-AnimBlendNode.h').write_text((saved/'src/animation/AnimBlendNode.h').read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
(out/'common.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
for name in ['AnimBlendAssociation.h','NodeName.h','VisibilityPlugins.h','AnimBlendClumpData.h','RpAnimBlend.h']:(out/name).write_text('#pragma once\n',newline='\n')
owners=(root/'utils/tests/anim_node_owners.h.in').read_text()
(out/'owners.h').write_text(owners,newline='\n')
entry='''#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Run_Init(void *self);
bool Run_Method(unsigned op,void *self,void *trans,void *rot,float value);
void Before_Init(void *self);
bool Before_Method(unsigned op,void *self,void *trans,void *rot,float value);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "AnimBlendNode.h"
static KeyFrameVectorOut vectorOut(void *value)
{
    // Borrow actual native vector fields only for methods that use their output
    unsigned char *p=(unsigned char*)value;const KeyFrameVectorOut result={(float*)p,(float*)(p+4),(float*)(p+8)};return result;
}
#endif
void Run_Init(void *self)
{
    // Exercise the complete public initializer from an independent C11 caller
#ifdef ORIGINAL
    Before_Init(self);
#else
    AnimBlendNode_Init((CAnimBlendNode*)self);
#endif
}
bool Run_Method(unsigned op,void *self,void *trans,void *rot,float value)
{
    // Exercise all twelve remaining node methods through the actual C boundary
#ifdef ORIGINAL
    return Before_Method(op,self,trans,rot,value);
#else
    CAnimBlendNode *node=(CAnimBlendNode*)self;
    switch(op){
    case 0:return AnimBlendNode_Update(node,vectorOut(trans),(CQuaternion*)rot,value);
    case 1:return AnimBlendNode_UpdateCompressed(node,vectorOut(trans),(CQuaternion*)rot,value);
    case 2:return AnimBlendNode_NextKeyFrame(node);
    case 3:return AnimBlendNode_NextKeyFrameCompressed(node);
    case 4:return AnimBlendNode_FindKeyFrame(node,value);
    case 5:return AnimBlendNode_SetupKeyFrameCompressed(node);
    case 6:AnimBlendNode_CalcDeltas(node);break;
    case 7:AnimBlendNode_CalcDeltasCompressed(node);break;
    case 8:AnimBlendNode_GetCurrentTranslation(node,vectorOut(trans),value);break;
    case 9:AnimBlendNode_GetCurrentTranslationCompressed(node,vectorOut(trans),value);break;
    case 10:AnimBlendNode_GetEndTranslation(node,vectorOut(trans),value);break;
    default:AnimBlendNode_GetEndTranslationCompressed(node,vectorOut(trans),value);break;
    }
    return false;
#endif
}
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/AnimBlendNode.cpp').read_text(),flags=re.M)+'\n'
original+='''extern "C" void Before_Init(void *self)
{
    // Invoke the entire original partial node initialization policy
    ((CAnimBlendNode*)self)->Init();
}
extern "C" bool Before_Method(unsigned op,void *self,void *trans,void *rot,float value)
{
    // Invoke complete original methods without translating their algorithm into a test model
    CAnimBlendNode *node=(CAnimBlendNode*)self;
    switch(op){
    case 0:return node->Update(*(CVector*)trans,*(CQuaternion*)rot,value);
    case 1:return node->UpdateCompressed(*(CVector*)trans,*(CQuaternion*)rot,value);
    case 2:return node->NextKeyFrame();
    case 3:return node->NextKeyFrameCompressed();
    case 4:return node->FindKeyFrame(value);
    case 5:return node->SetupKeyFrameCompressed();
    case 6:node->CalcDeltas();break;
    case 7:node->CalcDeltasCompressed();break;
    case 8:node->GetCurrentTranslation(*(CVector*)trans,value);break;
    case 9:node->GetCurrentTranslationCompressed(*(CVector*)trans,value);break;
    case 10:node->GetEndTranslation(*(CVector*)trans,value);break;
    default:node->GetEndTranslationCompressed(*(CVector*)trans,value);break;
    }
    return false;
}
'''
sequence=(saved/'src/animation/AnimBlendSequence.cpp').read_text();math='#include "owners.h"\n'+method(sequence,'CAnimBlendSequence::CAnimBlendSequence')+'\nCAnimBlendSequence::~CAnimBlendSequence() {}\n'+method(sequence,'CAnimBlendSequence::RemoveQuaternionFlips')+'\n'
math+='''extern "C" void *NativeNodeDefault(void *memory)
{
    // Keep the original trivial default-construction boundary opaque to fixture initialization
    return new(memory) CAnimBlendNode;
}
'''
math+='#ifndef ORIGINAL\n'+m.sequence_backend()+m.association_backend()+'\n#endif\n'
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/anim_node.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
frameTree=tree
if (tree/'src/animation/AnimBlendFrameData.h').exists():
    frameTree=root/'.diagnostics/anim-frame-c-tests/before'
    assert (frameTree/'src/animation/FrameUpdate.cpp').read_text()==m.anim_node_calls((saved/'src/animation/FrameUpdate.cpp').read_text())
before=re.sub(r'^#include[^\n]*\n','',(saved/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M);after=re.sub(r'^#include[^\n]*\n','',(frameTree/'src/animation/FrameUpdate.cpp').read_text(),flags=re.M)
(out/'FrameUpdate.cpp').write_text('#include "owners.h"\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
print('Prepared all thirteen original/C methods, native field interfaces and all complete frame callbacks')
