"""Compare all packed methods, actual sequence operations, blend nodes and complete frame callbacks."""
from pathlib import Path
import json,re,sys
import keyframes_migration as m
from stats_successor import current_path
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/keyframes-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
# Keep this historical keyframe fixture on the native node schema; the node fixture covers current consumers
animationTree=root/'.diagnostics/anim-node-c-tests/before' if (tree/'src/animation/AnimBlendNode.c').exists() else tree
if animationTree!=tree:
    for name in ['AnimBlendNode.cpp','FrameUpdate.cpp']:
        assert (animationTree/'src/animation'/name).read_text()==m.keyframes_calls((saved/'src/animation'/name).read_text()),name
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,name).read_text()==m.calls((saved/name).read_text()),name
sequenceTree=tree
if (tree/'src/animation/AnimBlendSequence.c').exists():
    import anim_node_migration as nodes
    sequenceTree=root/'.diagnostics/anim-sequence-c-tests/before'
    for name in ['AnimBlendSequence.h','AnimBlendSequence.cpp']:assert (sequenceTree/'src/animation'/name).read_text()==nodes.anim_node_calls(m.keyframes_calls((saved/'src/animation'/name).read_text())),name
for name in ['KeyFrames.h','KeyFrames.c','KeyFrames_native.h']:(out/name).write_text((tree/'src/animation'/name).read_text(),newline='\n')
for name,path in [('Quaternion.h','src/math/Quaternion.h'),('Quaternion.c','src/math/Quaternion.c'),('Quaternion_native.h','src/math/Quaternion_native.h'),('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('Maths.h','src/math/maths.h'),('AnimBlendNode.h','src/animation/AnimBlendNode.h')]:
    (out/name).write_text((saved/path).read_text(),newline='\n')
before=(saved/'src/animation/AnimBlendSequence.h').read_text();after=(sequenceTree/'src/animation/AnimBlendSequence.h').read_text();(out/'AnimBlendSequence.h').write_text('#pragma once\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
(out/'config.h').write_text('#pragma once\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
(out/'common.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
for name in ['AnimBlendAssociation.h','NodeName.h','VisibilityPlugins.h','AnimBlendClumpData.h','RpAnimBlend.h']:(out/name).write_text('#pragma once\n',newline='\n')
(out/'owners.h').write_text('''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>
#include <assert.h>
typedef int16_t int16;typedef int32_t int32;typedef uint8_t uint8;
#define VALIDATE_SIZE(a,b)
#define nil NULL
#define PI ((float)3.1415926535897932384626433832795)
#define TWOPI (PI*2)
#define PUSH_MEMID(x)
#define POP_MEMID()
#define REGISTER_MEMPTR(x)
struct RwV3d {float x,y,z;};
struct RwMatrix {RwV3d right;uint32_t flags;RwV3d up;uint32_t pad1;RwV3d at;uint32_t pad2;RwV3d pos;uint32_t pad3;};
#include "Maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "Quaternion_native.h"
#include "AnimBlendSequence.h"
#include "AnimBlendNode.h"
#ifndef ORIGINAL
#include "KeyFrames_native.h"
#endif
extern unsigned fixtureAllocations,fixtureFrees;
extern size_t fixtureBytes;
void *RwMalloc(size_t size);
void RwFree(void *memory);
template<class T> static T Max(T a,T b)
{
    // Retain the original maximum comparison in sequence-time consumers
    return a>b?a:b;
}
class CAnimBlendAssociation {
public:
    float blendAmount,timeStep;unsigned flags;
    bool IsRunning() {
        // Supply the original native association flag boundary
        return (flags&1)!=0;
    }
    bool IsRepeating() {
        // Supply the original native association repeat boundary
        return (flags&2)!=0;
    }
    bool IsPartial() {
        // Supply the original native association partial boundary
        return (flags&4)!=0;
    }
    bool HasTranslation() {
        // Supply the original native association translation boundary
        return (flags&8)!=0;
    }
    bool HasXTranslation() {
        // Supply the original native association X translation boundary
        return (flags&16)!=0;
    }
    float GetBlendAmount(float weight) {
        // Supply deterministic native partial and weighted association amounts
        return IsPartial()?blendAmount:blendAmount*weight;
    }
};
struct RwFrame {RwMatrix matrix;};
struct RpHAnimStdInterpFrame {struct {RwV3d imag;float real;} q;RwV3d t;};
struct AnimBlendFrameData {
    enum {VELOCITY_EXTRACTION=1,VELOCITY_EXTRACTION_3D=2,IGNORE_ROTATION=4,IGNORE_TRANSLATION=8};
    unsigned flag;RwFrame *frame;RpHAnimStdInterpFrame *hanimFrame;CVector resetPos;
};
struct AnimBlendFrameUpdateData {bool foobar;CAnimBlendNode **nodes;};
class CAnimBlendClumpData {public:CVector2D *velocity2d;CVector *velocity3d;};
extern CAnimBlendClumpData *gpAnimBlendClump;
static inline RwMatrix *RwFrameGetMatrix(RwFrame *frame)
{
    // Keep the complete callback's native frame lookup boundary
    return &frame->matrix;
}
static inline void RwMatrixSetIdentity(RwMatrix *matrix)
{
    // Provide deterministic native identity behavior while retaining unrelated padding
    matrix->right={1,0,0};matrix->up={0,1,0};matrix->at={0,0,1};matrix->pos={0,0,0};matrix->flags=1;
}
static inline void RwMatrixUpdate(RwMatrix *matrix)
{
    // Record the original final native matrix update
    matrix->flags+=16;
}
''',newline='\n')
entry='''#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void Run_GetRotation(const void *self,void *value);
void Run_SetRotation(void *self,const void *value);
float Run_GetTime(const void *self);
void Run_SetTime(void *self,float value);
void Run_GetTranslation(const void *self,void *value);
void Run_SetTranslation(void *self,const void *value);
void Before_GetRotation(const void *self,void *value);
void Before_SetRotation(void *self,const void *value);
float Before_GetTime(const void *self);
void Before_SetTime(void *self,float value);
void Before_GetTranslation(const void *self,void *value);
void Before_SetTranslation(void *self,const void *value);
#ifdef __cplusplus
}
#endif
'''
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "KeyFrames.h"\n#endif\n';original='#include "owners.h"\n#include "entry.h"\n'
ops=[('GetRotation','void','const void *self,void *value','self,value','((const KeyFrameCompressed*)self)->GetRotation((CQuaternion*)value);','KeyFrameCompressed_GetRotation((const KeyFrameCompressed*)self,(CQuaternion*)value);'),('SetRotation','void','void *self,const void *value','self,value','((KeyFrameCompressed*)self)->SetRotation(*(const CQuaternion*)value);','KeyFrameCompressed_SetRotation((KeyFrameCompressed*)self,(const CQuaternion*)value);'),('GetTime','float','const void *self','self','return ((KeyFrameCompressed*)self)->GetDeltaTime();','return KeyFrameCompressed_GetDeltaTime((const KeyFrameCompressed*)self);'),('SetTime','void','void *self,float value','self,value','((KeyFrameCompressed*)self)->SetTime(value);','KeyFrameCompressed_SetTime((KeyFrameCompressed*)self,value);'),('GetTranslation','void','const void *self,void *value','self,value','((KeyFrameTransCompressed*)self)->GetTranslation((CVector*)value);','unsigned char *p=(unsigned char*)value;const KeyFrameVectorOut ref={(float*)p,(float*)(p+4),(float*)(p+8)};KeyFrameTransCompressed_GetTranslation((const KeyFrameTransCompressed*)self,ref);'),('SetTranslation','void','void *self,const void *value','self,value','((KeyFrameTransCompressed*)self)->SetTranslation(*(const CVector*)value);','const unsigned char *p=(const unsigned char*)value;const KeyFrameVectorRef ref={(const float*)p,(const float*)(p+4),(const float*)(p+8)};KeyFrameTransCompressed_SetTranslation((KeyFrameTransCompressed*)self,ref);')]
for name,result,args,values,old,new in ops:
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original packed keyframe method\n    '+old.replace('((const KeyFrameCompressed*)self)->GetRotation','((KeyFrameCompressed*)self)->GetRotation')+'\n}\n'
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise the public operation through an independently compiled strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
sequence=(saved/'src/animation/AnimBlendSequence.cpp').read_text();math='#include "owners.h"\nunsigned fixtureAllocations=0,fixtureFrees=0;size_t fixtureBytes=0;\n'
math+='''void *RwMalloc(size_t size)
{
    // Record original allocation sizes and initialize only the fixture allocator's storage
    ++fixtureAllocations;fixtureBytes+=size;void *memory=malloc(size?size:1);assert(memory);memset(memory,0xa5,size);return memory;
}
void RwFree(void *memory)
{
    // Observe the original native free boundary without changing sequence policy
    if(memory)++fixtureFrees;free(memory);
}
extern "C" void *NativeKeyFrameDefault(unsigned type,void *memory)
{
    // Keep original default construction opaque to fixture storage initialization
    switch(type){case 0:return new(memory) KeyFrame;case 1:return new(memory) KeyFrameTrans;case 2:return new(memory) KeyFrameCompressed;default:return new(memory) KeyFrameTransCompressed;}
}
'''+method(sequence,'CAnimBlendSequence::CAnimBlendSequence')+'\nCAnimBlendSequence::~CAnimBlendSequence() {}\n'
for phase,base in [('before',saved),('after',sequenceTree)]:
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')
    for name in ['RemoveQuaternionFlips','SetNumFrames','CompressKeyframes','Uncompress','RemoveUncompressedData']:math+=method((base/'src/animation/AnimBlendSequence.cpp').read_text(),'CAnimBlendSequence::'+name)+'\n'
math+='#endif\n'
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/keyframes.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
for name in ['AnimBlendNode.cpp','FrameUpdate.cpp']:
    before=re.sub(r'^#include[^\n]*\n','',(saved/'src/animation'/name).read_text(),flags=re.M);after=re.sub(r'^#include[^\n]*\n','',(animationTree/'src/animation'/name).read_text(),flags=re.M)
    (out/name).write_text('#include "owners.h"\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
print('Prepared all six original/C packed methods, actual sequence allocation/conversion bodies, blend nodes and frame callbacks')
