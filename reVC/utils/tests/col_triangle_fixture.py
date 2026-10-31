"""Prepare exact original triangle/plane methods and vector math with independent C11 callers."""
from pathlib import Path
import json,re,sys
import col_triangle_migration as m
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-triangle-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['ColTriangle.h','ColTriangle.c','CompressedVector.h','CompressedVector.c']:(out/name).write_text((tree/'src/collision'/name).read_text(),newline='\n')
if (tree/'src/collision/CollisionVector.h').exists():(out/'CollisionVector.h').write_text((tree/'src/collision/CollisionVector.h').read_text(),newline='\n')
for name,source in [('original-ColTriangle.h',saved/'src/collision/ColTriangle.h'),('Vector.h',saved/'src/math/Vector.h'),('Maths.h',saved/'src/math/Maths.h')]:(out/name).write_text(source.read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n#ifdef VU_COLLISION\n#define COMPRESSED_COL_VECTORS\n#endif\n',newline='\n')
(out/'owners.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"
typedef int16_t int16;typedef uint16_t uint16;typedef int32_t int32;typedef uint8_t uint8;
struct RwV3d {float x,y,z;};
#include "Maths.h"
#include "Vector.h"
#ifdef ORIGINAL
#include "original-ColTriangle.h"
#else
#include "ColTriangle.h"
#endif
''',newline='\n')
vector=(saved/'src/math/Vector.cpp').read_text();math='#include "owners.h"\n'+method(vector,'CVector::Normalise')+'\n'+method(vector,'CrossProduct')+'\n'
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/collision/ColTriangle.cpp').read_text(),flags=re.M)+'\n'
entry='''#pragma once
#include "config.h"
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "ColTriangle.h"
static CollisionPointRef point(const void *value)
{
    // Preserve references to native float subobjects without struct type punning
    const unsigned char *p=(const unsigned char*)value;
    return ColTrianglePlane_Ref((const float*)p,(const float*)(p+4),(const float*)(p+8));
}
#endif
'''
ops=[('TriangleSet','void','void *self,int32_t a,int32_t b,int32_t c,uint8_t surface','self,a,b,c,surface','((CColTriangle*)self)->Set(a,b,c,surface);','ColTriangle_Set((CColTriangle*)self,a,b,c,surface);',''),('Set','void','void *self,const void *a,const void *b,const void *c','self,a,b,c','((CColTrianglePlane*)self)->Set(*(const CVector*)a,*(const CVector*)b,*(const CVector*)c);','ColTrianglePlane_Set((CColTrianglePlane*)self,point(a),point(b),point(c));',''),('SetFromVertices','void','void *self,const void *vertices,void *triangle','self,vertices,triangle','((CColTrianglePlane*)self)->Set((const CompressedVector*)vertices,*(CColTriangle*)triangle);','ColTrianglePlane_SetFromVertices((CColTrianglePlane*)self,(const CompressedVector*)vertices,(const CColTriangle*)triangle);',''),('GetNormal','void','const void *self,void *value','self,value','((const CColTrianglePlane*)self)->GetNormal(*(CVector*)value);','ColTrianglePlane_GetNormal((const CColTrianglePlane*)self,(float*)value,(float*)((unsigned char*)value+4),(float*)((unsigned char*)value+8));',''),('CalcPoint','float','const void *self,const void *value','self,value','return ((const CColTrianglePlane*)self)->CalcPoint(*(const CVector*)value);','return ColTrianglePlane_CalcPoint((const CColTrianglePlane*)self,point(value));','')]
for axis in 'XYZ':ops.append(('GetNormal'+axis,'float','const void *self','self','return ((const CColTrianglePlane*)self)->GetNormal'+axis+'();','return ColTrianglePlane_GetNormal'+axis+'((const CColTrianglePlane*)self);','#ifndef VU_COLLISION\n'))
ops.append(('Unpack','void','const void *self,int32_t *value','self,value','((const CColTrianglePlane*)self)->Unpack(value);','ColTrianglePlane_Unpack((const CColTrianglePlane*)self,value);','#ifdef VU_COLLISION\n'))
for name,result,args,values,old,new,condition in ops:
    ending='#endif\n' if condition else ''
    for prefix in ['Run','Before']:entry+=condition+result+' '+prefix+'_'+name+'('+args+');\n'+ending
    original+=condition+'extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original triangle operation with its original references\n    '+old+'\n}\n'+ending
    caller+=condition+result+' Run_'+name+'('+args+')\n{\n    // Exercise the public C operation through an independent strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'+ending
entry+='#ifdef __cplusplus\n}\n#endif\n'
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('math.cpp',math),('fixture.cpp',(root/'utils/tests/col_triangle.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
if (tree/'src/collision/CollisionVector.h').exists():(out/'caller.c').write_text(caller.replace('ColTrianglePlane_Ref(','CollisionPoint_Ref('),newline='\n')
print('Prepared all original/C triangle methods, exact native vector normalization/cross product and live-reference callers')
