"""Compare complete sphere owners, live ray aliases, exact native collision consumers and batch math."""
from pathlib import Path
import json,re,sys
import col_sphere_migration as m
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-sphere-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['ColSphere.h','ColSphere.c','ColSphere_native.h','CollisionVector.h','CollisionVector_native.h','ColBox.h','ColLine.h','ColPoint.h']:(out/name).write_text((tree/'src/collision'/name).read_text(),newline='\n')
hasCContact='typedef struct CColPoint {' in (out/'ColPoint.h').read_text()
if hasCContact:(out/'ColPoint.c').write_text((tree/'src/collision/ColPoint.c').read_text(),newline='\n')
else:(out/'ColPoint.c').unlink(missing_ok=True)
for name,source in [('original-ColSphere.h',saved/'src/collision/ColSphere.h'),('Vector.h',saved/'src/math/Vector.h'),('Maths.h',saved/'src/math/Maths.h'),('Matrix.h',saved/'src/math/Matrix.h'),('VuVector.h',tree/'src/math/VuVector.h'),('SurfaceTable.h',saved/'src/core/SurfaceTable.h'),('SurfaceTypes.h',saved/'src/core/SurfaceTypes.h'),('General.h',saved/'src/core/General.h')]:(out/name).write_text(source.read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
(out/'owners.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>
#define nil nullptr
#define sq(a) ((a)*(a))
#ifdef _MSC_VER
#define TYPEALIGN(n) __declspec(align(n))
#else
#define TYPEALIGN(n) __attribute__((aligned(n)))
#endif
typedef int8_t int8;typedef int16_t int16;typedef uint16_t uint16;typedef int32_t int32;typedef uint8_t uint8;
struct RwV3d {float x,y,z;};struct RwMatrix;
#include "Maths.h"
#include "Vector.h"
#include "Matrix.h"
#include "General.h"
#include "CollisionVector_native.h"
#ifdef ORIGINAL
#include "original-ColSphere.h"
#else
#include "ColSphere_native.h"
#endif
#include "ColLine.h"
#include "ColBox.h"
#include "ColPoint.h"
#include "VuVector.h"
class CCollision {
public:
    static bool TestSphereSphere(const CSphere &s1,const CSphere &s2);
    static bool TestSphereBox(const CSphere &sph,const CBox &box);
    static bool TestLineSphere(const CColLine &line,const CColSphere &sph);
    static bool ProcessSphereSphere(const CColSphere &s1,const CColSphere &s2,CColPoint &point,float &mindistsq);
    static bool ProcessSphereBox(const CColSphere &sph,const CColBox &box,CColPoint &point,float &mindistsq);
    static bool ProcessLineSphere(const CColLine &line,const CColSphere &sphere,CColPoint &point,float &mindist);
};
''',newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "ColSphere.h"
#include <string.h>
static CollisionPointRef point(const void *value)
{
    // Refer to actual float coordinates without forming an unrelated vector-class view
    const unsigned char *p=(const unsigned char*)value;
    return CollisionPoint_Ref((const float*)p,(const float*)(p+4),(const float*)(p+8));
}
static CollisionPointOut output(void *value)
{
    // Retain live writable coordinates for input/output aliases
    unsigned char *p=(unsigned char*)value;
    return CollisionPoint_Output((float*)p,(float*)(p+4),(float*)(p+8));
}
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/collision/ColSphere.cpp').read_text(),flags=re.M)+'\n'
ops=[('BaseSet','void','void *self,float radius,const void *center','self,radius,center','((CSphere*)self)->Set(radius,*(const CVector*)center);','Sphere_Set((CSphere*)self,radius,point(center));'),('Bounds','void','void *self,float radius,const void *center','self,radius,center','((CColSphere*)self)->Set(radius,*(const CVector*)center);','ColSphere_SetBounds((CColSphere*)self,radius,point(center));'),('Set','void','void *self,float radius,const void *center,uint8_t surface,uint8_t piece','self,radius,center,surface,piece','((CColSphere*)self)->Set(radius,*(const CVector*)center,surface,piece);','ColSphere_Set((CColSphere*)self,radius,point(center),surface,piece);'),('Ray','bool','void *self,const void *from,const void *dir,void *entry,void *exit','self,from,dir,entry,exit','return ((CColSphere*)self)->IntersectRay(*(const CVector*)from,*(const CVector*)dir,*(CVector*)entry,*(CVector*)exit);','return ColSphere_IntersectRay((CColSphere*)self,point(from),point(dir),output(entry),output(exit));'),('RawBase','void','void *self,const void *bytes','self,bytes','memcpy(self,bytes,16);','*(CSphere*)self=Sphere_FromRaw16(bytes);'),('RawCol','void','void *self,const void *bytes','self,bytes','memset(self,0,20);memcpy(self,bytes,16);','*(CColSphere*)self=ColSphere_FromRaw16(bytes);')]
for name,result,args,values,old,new in ops:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original method or its original quadword representation\n    '+old+'\n}\n'
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise each public C operation through an independently compiled strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
quadratic='#include "General.h"\n#include <math.h>\n'+method((saved/'src/core/General.c').read_text(),'General_SolveQuadratic')+'\n'
vector=(saved/'src/math/Vector.cpp').read_text();start=vector.index('operator*(const CMatrix &mat, const CVector &vec)');start=vector.rfind('\n',0,start-1)+1;end=vector.index('\n}',start)+2
math='#include "owners.h"\nCMatrix::CMatrix() {}\nCMatrix::~CMatrix() {}\n'+vector[start:end]+'\n'+method(vector,'CVector::Normalise')+'\n'
for phase,base in [('before',saved),('after',tree)]:
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')
    for name in ['TestSphereSphere','TestSphereBox','TestLineSphere','ProcessSphereSphere','ProcessSphereBox','ProcessLineSphere']:
        body=method((base/'src/collision/Collision.cpp').read_text(),'CCollision::'+name)
        if hasCContact:
            import col_point_migration as contacts
            body=contacts.calls(body)
        math+=body+'\n'
    math+=method((base/'src/math/math.cpp').read_text(),'TransformPoints' if phase=='before' else 'TransformColSpheres')+'\n'
math+='#endif\n'
math+='''extern "C" void *NativeSphereDefault(void *memory,int collision)
{
    // Keep default construction opaque to fixture storage initialization
    if(collision)return new(memory) CColSphere;
    return new(memory) CSphere;
}
extern "C" bool NativeSphereRay(void *self,const void *from,const void *dir,void *entry,void *exit)
{
    // Exercise the native ABI while preserving references to each actual float subobject
#ifdef ORIGINAL
    return ((CColSphere*)self)->IntersectRay(*(const CVector*)from,*(const CVector*)dir,*(CVector*)entry,*(CVector*)exit);
#else
    const unsigned char *a=(const unsigned char*)from,*b=(const unsigned char*)dir;
    unsigned char *c=(unsigned char*)entry,*d=(unsigned char*)exit;
    return ColSphere_IntersectRay((CColSphere*)self,CollisionPoint_Ref((const float*)a,(const float*)(a+4),(const float*)(a+8)),CollisionPoint_Ref((const float*)b,(const float*)(b+4),(const float*)(b+8)),CollisionPoint_Output((float*)c,(float*)(c+4),(float*)(c+8)),CollisionPoint_Output((float*)d,(float*)(d+4),(float*)(d+8)));
#endif
}
'''
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('quadratic.c',quadratic),('math.cpp',math),('fixture.cpp',(root/'utils/tests/col_sphere.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared both complete original/C sphere owners, the exact quadratic solver, six collision consumers and batch transforms')
