"""Compare the complete contact owner, exact collision consumers and suspension mutation timing."""
from pathlib import Path
import json,re,sys
import col_point_migration as m
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-point-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['ColPoint.h','ColPoint.c','ColSphere.h','CollisionVector.h','CollisionVector_native.h','ColBox.h','ColLine.h']:(out/name).write_text((tree/'src/collision'/name).read_text(),newline='\n')
for name,source in [('original-ColPoint.h',saved/'src/collision/ColPoint.h'),('Vector.h',saved/'src/math/Vector.h'),('Maths.h',saved/'src/math/Maths.h'),('SurfaceTable.h',saved/'src/core/SurfaceTable.h'),('SurfaceTypes.h',saved/'src/core/SurfaceTypes.h')]:(out/name).write_text(source.read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
(out/'owners.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>
typedef int8_t int8;typedef int16_t int16;typedef uint16_t uint16;typedef int32_t int32;typedef uint8_t uint8;
struct RwV3d {float x,y,z;};
#include "Maths.h"
#include "Vector.h"
#include "CollisionVector_native.h"
#ifdef ORIGINAL
#include "original-ColPoint.h"
#else
#include "ColPoint.h"
#endif
#include "ColSphere.h"
#include "ColLine.h"
#include "ColBox.h"
class CCollision {
public:
    static bool TestSphereSphere(const CSphere &s1,const CSphere &s2);
    static bool TestSphereBox(const CSphere &sph,const CBox &box);
    static bool TestLineSphere(const CColLine &line,const CColSphere &sph);
    static bool ProcessSphereSphere(const CColSphere &s1,const CColSphere &s2,CColPoint &point,float &mindistsq);
    static bool ProcessSphereBox(const CColSphere &sph,const CColBox &box,CColPoint &point,float &mindistsq);
    static bool ProcessLineSphere(const CColLine &line,const CColSphere &sphere,CColPoint &point,float &mindist);
};
extern float fixtureStep;
static inline const float *Timer_GetTimeStep() {return &fixtureStep;}
template<class T> static T Min(T a,T b) {return a<b?a:b;}
#define GRAVITY 0.008f
class CPhysical {
public:
    float m_fMass;
    bool bIsHeavy;
    CColPoint *observed;
    unsigned count;
    CVector move,turn,position;
    unsigned char moveNormal[12],turnNormal[12];
    void ApplyMoveForce(const CVector &value) {
        // Observe the contact at the original force-application boundary
        move=value;memcpy(moveNormal,&observed->normal,12);++count;
    }
    void ApplyTurnForce(const CVector &value,const CVector &point) {
        // Observe the contact again before recording the original turn-force inputs
        turn=value;position=point;memcpy(turnNormal,&observed->normal,12);++count;
    }
#ifdef ORIGINAL
    bool ApplySpringCollisionAlt(float springConst,CVector &springDir,CVector &point,float springRatio,float bias,CVector &forceDir);
#else
    bool ApplySpringCollisionAlt(float springConst,CVector &springDir,CVector &point,float springRatio,float bias,CColPoint &contact);
#endif
};
''',newline='\n')
entry='''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
'''
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "ColPoint.h"\n#endif\n'
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/collision/ColPoint.cpp').read_text(),flags=re.M)+'\n'
ops=[('Normal','const void*','const void *self','self','return &((CColPoint*)self)->GetNormal();','return ColPoint_GetNormal((const CColPoint*)self);'),('Depth','float','const void *self','self','return ((CColPoint*)self)->GetDepth();','return ColPoint_GetDepth((const CColPoint*)self);'),('Set','void','void *self,float depth,uint8_t a,uint8_t b,uint8_t c,uint8_t d','self,depth,a,b,c,d','((CColPoint*)self)->Set(depth,a,b,c,d);','ColPoint_Set((CColPoint*)self,depth,a,b,c,d);'),('Surfaces','void','void *self,uint8_t a,uint8_t b,uint8_t c,uint8_t d','self,a,b,c,d','((CColPoint*)self)->Set(a,b,c,d);','ColPoint_SetSurfaces((CColPoint*)self,a,b,c,d);'),('Assign','void*','void *self,const void *other','self,other','return &(*(CColPoint*)self=*(const CColPoint*)other);','return ColPoint_Assign((CColPoint*)self,(const CColPoint*)other);'),('Copy','void','void *self,const void *other','self,other','new(self) CColPoint(*(const CColPoint*)other);','*(CColPoint*)self=ColPoint_CopyValue(*(const CColPoint*)other);'),('Normalise','void','void *self','self','((CColPoint*)self)->normal.Normalise();','ColPoint_NormaliseNormal((CColPoint*)self);'),('Reverse','void','void *self','self','((CColPoint*)self)->normal*=-1.0f;','ColPoint_ReverseNormal((CColPoint*)self);')]
for name,result,args,values,old,new in ops:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original contact operation or original normal mutation\n    '+old+'\n}\n'
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise the public C operation through a separately compiled strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
math='#include "owners.h"\nfloat fixtureStep=1.0f;\n'+method((saved/'src/math/Vector.cpp').read_text(),'CVector::Normalise')+'\n'
for phase,base in [('before',saved),('after',tree)]:
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')
    for name in ['TestSphereSphere','TestSphereBox','TestLineSphere','ProcessSphereSphere','ProcessSphereBox','ProcessLineSphere']:math+=method((base/'src/collision/Collision.cpp').read_text(),'CCollision::'+name)+'\n'
    math+=method((base/'src/entities/Physical.cpp').read_text(),'CPhysical::ApplySpringCollisionAlt')+'\n'
math+='#endif\n'
math+='''extern "C" void *NativeContactDefault(void *memory)
{
    // Keep construction opaque to fixture storage initialization
    return new(memory) CColPoint;
}
extern "C" void NativeContactCopy(void *memory,const CColPoint *source)
{
    // Exercise full copy construction from the native caller independently of the C11 adapter
#ifdef ORIGINAL
    new(memory) CColPoint(*source);
#else
    new(memory) CColPoint(ColPoint_CopyValue(*source));
#endif
}
'''
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/col_point.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all original/C contact operations, six exact collision consumers and the original/updated suspension body')
