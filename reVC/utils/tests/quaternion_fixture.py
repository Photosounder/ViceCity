"""Compare every original quaternion operation and complete animation consumers with strict C11 callers."""
from pathlib import Path
import json,re,sys
import quaternion_migration as m
from stats_successor import current_path
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/quaternion-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
# Keep this historical quaternion fixture on its original keyframe schema; the keyframe fixture covers current consumers
animationTree=root/'.diagnostics/keyframes-c-tests/before' if (tree/'src/animation/KeyFrames.h').exists() else tree
if animationTree!=tree:
    for name in ['AnimBlendNode.cpp','FrameUpdate.cpp','AnimBlendSequence.cpp']:
        assert (animationTree/'src/animation'/name).read_text()==m.quaternion_calls((saved/'src/animation'/name).read_text()),name
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert current_path(root,m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['Quaternion.h','Quaternion.c','Quaternion_native.h']:(out/name).write_text((tree/'src/math'/name).read_text(),newline='\n')
for name,path in [('original-Quaternion.h','src/math/Quaternion.h'),('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('Maths.h','src/math/maths.h'),('AnimBlendSequence.h','src/animation/AnimBlendSequence.h'),('AnimBlendNode.h','src/animation/AnimBlendNode.h')]:
    source=(saved/path).read_text()
    if name=='AnimBlendSequence.h':source=source.replace('#include "Quaternion.h"','')
    (out/name).write_text(source,newline='\n')
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
struct RwV3d {float x,y,z;};
struct RwMatrix {RwV3d right;uint32_t flags;RwV3d up;uint32_t pad1;RwV3d at;uint32_t pad2;RwV3d pos;uint32_t pad3;};
#include "Maths.h"
#include "Vector.h"
#include "Vector2D.h"
#ifdef ORIGINAL
#include "original-Quaternion.h"
#else
#include "Quaternion_native.h"
#endif
#include "AnimBlendSequence.h"
#include "AnimBlendNode.h"
class CAnimBlendAssociation {
public:
    float blendAmount,timeStep;unsigned flags;
    bool IsRunning() {
        // Supply the native association flag at the original animation boundary
        return (flags&1)!=0;
    }
    bool IsRepeating() {
        // Supply the native association flag at the original animation boundary
        return (flags&2)!=0;
    }
    bool IsPartial() {
        // Supply the native association flag at the original animation boundary
        return (flags&4)!=0;
    }
    bool HasTranslation() {
        // Supply the native association flag at the original animation boundary
        return (flags&8)!=0;
    }
    bool HasXTranslation() {
        // Supply the native association flag at the original animation boundary
        return (flags&16)!=0;
    }
    float GetBlendAmount(float weight) {
        // Supply deterministic partial and weighted association amounts
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
    // Retain the native frame lookup boundary used by complete frame callbacks
    return &frame->matrix;
}
static inline void RwMatrixSetIdentity(RwMatrix *matrix)
{
    // Supply deterministic native identity behavior without changing unrelated padding
    matrix->right={1,0,0};matrix->up={0,1,0};matrix->at={0,0,1};matrix->pos={0,0,0};matrix->flags=1;
}
static inline void RwMatrixUpdate(RwMatrix *matrix)
{
    // Record the original callback's final native matrix update
    matrix->flags+=16;
}
''',newline='\n')
entry='''#pragma once
#ifdef __cplusplus
extern "C" {
#endif
void Run_Value(unsigned op,void *out,const void *a,const void *b,float scalar,float x,float y,float z,float w);
float Run_Scalar(unsigned op,const void *a,const void *b);
const void *Run_Mutate(unsigned op,void *out,const void *a,const void *b,float scalar,float theta,float inverse,float t);
void Run_Axis(unsigned op,void *self,void *axis,float *angle);
void Run_Matrix(unsigned op,void *self,void *matrix);
void Run_Euler(unsigned op,void *self,float *a,float *b,float *c);
void Before_Value(unsigned op,void *out,const void *a,const void *b,float scalar,float x,float y,float z,float w);
float Before_Scalar(unsigned op,const void *a,const void *b);
const void *Before_Mutate(unsigned op,void *out,const void *a,const void *b,float scalar,float theta,float inverse,float t);
void Before_Axis(unsigned op,void *self,void *axis,float *angle);
void Before_Matrix(unsigned op,void *self,void *matrix);
void Before_Euler(unsigned op,void *self,float *a,float *b,float *c);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "Quaternion.h"
#include <string.h>
static QuaternionVectorRef vectorRef(const void *value)
{
    // Borrow actual independent XYZ field addresses at the strict C11 boundary
    const unsigned char *p=(const unsigned char*)value;const QuaternionVectorRef result={(const float*)p,(const float*)(p+4),(const float*)(p+8)};return result;
}
static QuaternionVectorOut vectorOut(void *value)
{
    // Preserve output aliases instead of introducing a temporary native vector
    unsigned char *p=(unsigned char*)value;const QuaternionVectorOut result={(float*)p,(float*)(p+4),(float*)(p+8)};return result;
}
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/math/Quaternion.cpp').read_text(),flags=re.M)+'\n'
groups=[
('Value','void','unsigned op,void *out,const void *a,const void *b,float scalar,float x,float y,float z,float w','op,out,a,b,scalar,x,y,z,w',
'CQuaternion value;switch(op){case 0:value=CQuaternion(x,y,z,w);break;case 1:value=-*(const CQuaternion*)a;break;case 2:value=*(const CQuaternion*)a+*(const CQuaternion*)b;break;case 3:value=*(const CQuaternion*)a-*(const CQuaternion*)b;break;case 4:value=*(const CQuaternion*)a*scalar;break;case 5:value=scalar**(const CQuaternion*)a;break;default:value=*(const CQuaternion*)a/scalar;}memcpy(out,&value,16);',
'CQuaternion value;switch(op){case 0:value=Quaternion_Make(x,y,z,w);break;case 1:value=Quaternion_Negate(*(const CQuaternion*)a);break;case 2:value=Quaternion_Add(*(const CQuaternion*)a,*(const CQuaternion*)b);break;case 3:value=Quaternion_Sub(*(const CQuaternion*)a,*(const CQuaternion*)b);break;case 4:value=Quaternion_Scale(*(const CQuaternion*)a,scalar);break;case 5:value=Quaternion_LeftScale(scalar,*(const CQuaternion*)a);break;default:value=Quaternion_Divide(*(const CQuaternion*)a,scalar);}memcpy(out,&value,16);'),
('Scalar','float','unsigned op,const void *a,const void *b','op,a,b',
'const CQuaternion *value=(const CQuaternion*)a;return op==0?value->Magnitude():op==1?value->MagnitudeSqr():DotProduct(*value,*(const CQuaternion*)b);',
'return op==0?Quaternion_Magnitude((const CQuaternion*)a):op==1?Quaternion_MagnitudeSqr((const CQuaternion*)a):Quaternion_Dot(*(const CQuaternion*)a,*(const CQuaternion*)b);'),
('Mutate','const void *','unsigned op,void *out,const void *a,const void *b,float scalar,float theta,float inverse,float t','op,out,a,b,scalar,theta,inverse,t',
'CQuaternion *q=(CQuaternion*)out;switch(op){case 0:q->Normalise();break;case 1:q->Multiply(*(const CQuaternion*)a,*(const CQuaternion*)b);break;case 2:q->Invert();break;case 3:return &(*q+=*(const CQuaternion*)a);case 4:return &(*q-=*(const CQuaternion*)a);case 5:return &(*q*=scalar);case 6:return &(*q/=scalar);default:q->Slerp(*(const CQuaternion*)a,*(const CQuaternion*)b,theta,inverse,t);}return q;',
'CQuaternion *q=(CQuaternion*)out;switch(op){case 0:Quaternion_Normalise(q);break;case 1:Quaternion_Multiply(q,(const CQuaternion*)a,(const CQuaternion*)b);break;case 2:Quaternion_Invert(q);break;case 3:return Quaternion_AddAssign(q,(const CQuaternion*)a);case 4:return Quaternion_SubAssign(q,(const CQuaternion*)a);case 5:return Quaternion_ScaleAssign(q,scalar);case 6:return Quaternion_DivideAssign(q,scalar);default:Quaternion_Slerp(q,(const CQuaternion*)a,(const CQuaternion*)b,theta,inverse,t);}return q;'),
('Axis','void','unsigned op,void *self,void *axis,float *angle','op,self,axis,angle',
'if(op)((CQuaternion*)self)->Set((RwV3d*)axis,*angle);else ((CQuaternion*)self)->Get((RwV3d*)axis,angle);',
'if(op)Quaternion_SetAxisAngle((CQuaternion*)self,vectorRef(axis),*angle);else Quaternion_GetAxisAngle((const CQuaternion*)self,vectorOut(axis),angle);'),
('Matrix','void','unsigned op,void *self,void *matrix','op,self,matrix',
'if(op)((CQuaternion*)self)->Set(*(RwMatrix*)matrix);else ((CQuaternion*)self)->Get((RwMatrix*)matrix);',
'unsigned char *p=(unsigned char*)matrix;if(op){const QuaternionMatrixRef ref={vectorRef(p),vectorRef(p+16),vectorRef(p+32)};Quaternion_SetMatrix((CQuaternion*)self,ref);}else{const QuaternionMatrixOut ref={vectorOut(p),vectorOut(p+16),vectorOut(p+32)};Quaternion_GetMatrix((const CQuaternion*)self,ref);}'),
('Euler','void','unsigned op,void *self,float *a,float *b,float *c','op,self,a,b,c',
'if(op)((CQuaternion*)self)->Set(*a,*b,*c);else ((CQuaternion*)self)->Get(a,b,c);',
'if(op)Quaternion_SetEuler((CQuaternion*)self,*a,*b,*c);else Quaternion_GetEuler((const CQuaternion*)self,a,b,c);')]
for name,result,args,values,old,new in groups:
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original quaternion operation without replacing its policy\n    '+old+'\n}\n'
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise the complete public operation through an independently compiled C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
math='#include "owners.h"\nextern "C" void *NativeQuaternionDefault(void *memory)\n{\n    // Keep no-write construction opaque to the fixture storage initializer\n    return new(memory) CQuaternion;\n}\n'
sequence=(saved/'src/animation/AnimBlendSequence.cpp').read_text();math+=method(sequence,'CAnimBlendSequence::CAnimBlendSequence')+'\nCAnimBlendSequence::~CAnimBlendSequence() {}\n'
for phase,base in [('before',saved),('after',animationTree)]:
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')+method((base/'src/animation/AnimBlendSequence.cpp').read_text(),'CAnimBlendSequence::RemoveQuaternionFlips')+'\n'
math+='''#endif
extern "C" void NativeQuaternionMatrix(CQuaternion *value,RwMatrix *matrix)
{
    // Exercise the actual native field-reference adapter independently of the C11 caller
#ifdef ORIGINAL
    value->Get(matrix);
#else
    Quaternion_GetMatrix(value,Quaternion_MatrixOut(*matrix));
#endif
}
extern "C" void NativeQuaternionAxis(CQuaternion *value,RwV3d *axis,float angle)
{
    // Exercise native axis borrowing while retaining the original angle-value boundary
#ifdef ORIGINAL
    value->Set(axis,angle);
#else
    Quaternion_SetAxisAngle(value,Quaternion_VectorRef(*axis),angle);
#endif
}
'''
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/quaternion.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
for name in ['AnimBlendNode.cpp','FrameUpdate.cpp']:
    before=re.sub(r'^#include[^\n]*\n','',(saved/'src/animation'/name).read_text(),flags=re.M);after=re.sub(r'^#include[^\n]*\n','',(animationTree/'src/animation'/name).read_text(),flags=re.M)
    (out/name).write_text('#include "owners.h"\n#ifdef ORIGINAL\n'+before+'\n#else\n'+after+'\n#endif\n',newline='\n')
print('Prepared all 24 quaternion operations, complete blend-node methods and all nine frame callbacks')
