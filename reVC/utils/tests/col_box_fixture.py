"""Compare both complete box owners, independent C11 callers and exact collision consumers."""
from pathlib import Path
import json,re,sys
import col_box_migration as m
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-box-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['ColBox.h','ColBox.c','CollisionVector.h','CollisionVector_native.h','ColLine.h','ColPoint.h','ColSphere.h']:(out/name).write_text((tree/'src/collision'/name).read_text(),newline='\n')
if 'typedef struct CColPoint {' in (out/'ColPoint.h').read_text():(out/'ColPoint.c').write_text((tree/'src/collision/ColPoint.c').read_text(),newline='\n')
else:(out/'ColPoint.c').unlink(missing_ok=True)
for name,source in [('original-ColBox.h',saved/'src/collision/ColBox.h'),('Vector.h',saved/'src/math/Vector.h'),('Maths.h',saved/'src/math/Maths.h'),('SurfaceTable.h',saved/'src/core/SurfaceTable.h'),('SurfaceTypes.h',saved/'src/core/SurfaceTypes.h')]:(out/name).write_text(source.read_text(),newline='\n')
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
#ifdef ORIGINAL
#include "original-ColBox.h"
#else
#include "ColBox.h"
#include "CollisionVector_native.h"
#endif
#include "ColLine.h"
#include "ColSphere.h"
#include "ColPoint.h"
#include "CollisionVector_native.h"
class CCollision {
public:
    static bool TestSphereBox(const CSphere &sph,const CBox &box);
    static bool TestLineBox(const CColLine &line,const CBox &box);
    static bool TestVerticalLineBox(const CColLine &line,const CBox &box);
    static bool ProcessSphereBox(const CColSphere &sph,const CColBox &box,CColPoint &point,float &mindistsq);
    static bool ProcessLineBox(const CColLine &line,const CColBox &box,CColPoint &point,float &mindist);
};
''',newline='\n')
entry='''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "ColBox.h"
#include <string.h>
static CollisionPointRef point(const void *value)
{
    // Keep live float references independent of the native vector class
    const unsigned char *p=(const unsigned char*)value;
    return CollisionPoint_Ref((const float*)p,(const float*)(p+4),(const float*)(p+8));
}
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/collision/ColBox.cpp').read_text(),flags=re.M)+'\n'
ops=[('BoxSet','void','void *self,const void *a,const void *b','self,a,b','((CBox*)self)->Set(*(const CVector*)a,*(const CVector*)b);','Box_Set((CBox*)self,point(a),point(b));'),('Size','void','const void *self,void *result','self,result','CVector value=((CBox*)self)->GetSize();memcpy(result,&value,12);','CollisionVector3 value=Box_GetSize((const CBox*)self);memcpy(result,&value,12);'),('Bounds','void','void *self,const void *a,const void *b','self,a,b','((CColBox*)self)->Set(*(const CVector*)a,*(const CVector*)b);','ColBox_SetBounds((CColBox*)self,point(a),point(b));'),('Set','void','void *self,const void *a,const void *b,uint8_t surface,uint8_t piece','self,a,b,surface,piece','((CColBox*)self)->Set(*(const CVector*)a,*(const CVector*)b,surface,piece);','ColBox_Set((CColBox*)self,point(a),point(b),surface,piece);'),('Assign','void*','void *self,const void *other','self,other','return &(*(CColBox*)self=*(const CColBox*)other);','return ColBox_Assign((CColBox*)self,(const CColBox*)other);')]
for name,result,args,values,old,new in ops:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the exact original complete box operation\n    '+old+'\n}\n'
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise the public C API through a separately compiled strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
math='#include "owners.h"\n'
for phase,base in [('before',saved),('after',tree)]:
    math+=('#ifdef ORIGINAL\n' if phase=='before' else '#else\n')
    for name in ['TestSphereBox','TestLineBox','TestVerticalLineBox','ProcessSphereBox','ProcessLineBox']:
        body=method((base/'src/collision/Collision.cpp').read_text(),'CCollision::'+name)
        if (tree/'src/collision/ColSphere_native.h').exists():
            import col_sphere_migration as spheres
            body=spheres.calls(body)
        math+=body+'\n'
math+='#endif\n'
math+='''extern "C" void *NativeBoxDefault(void *memory,int collision)
{
    // Keep construction opaque to fixture-side storage initialization
    if(collision)return new(memory) CColBox;
    return new(memory) CBox;
}
extern "C" void NativeBoxSize(const CBox *box,CVector *result)
{
    // Exercise native callers of the original method or C value-return ABI
#ifdef ORIGINAL
    *result=const_cast<CBox*>(box)->GetSize();
#else
    *result=CollisionVector_ToNative(Box_GetSize(box));
#endif
}
'''
fixture=(root/'utils/tests/col_box.cpp.in').read_text()
if (tree/'src/collision/ColSphere_native.h').exists():
    import col_sphere_migration as spheres
    fixture=spheres.calls(fixture)
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',fixture)]:(out/name).write_text(value,newline='\n')
print('Prepared both complete original/C box owners and five exact native collision consumers')
