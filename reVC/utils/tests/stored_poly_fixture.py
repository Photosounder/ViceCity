"""Compare exact cached-validation methods with C policy and actual VU emulation."""
from pathlib import Path
import json,re,sys
import stored_poly_migration as m
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-poly-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/name).read_text()==m.calls((saved/name).read_text()),name
for name in ['StoredCollPoly.h','StoredCollPoly.c','StoredCollPoly_backend.h','StoredCollPoly_native.h','ColPoint.h','ColTriangle.h','ColTriangle.c','CompressedVector.h','CompressedVector.c','CollisionVector.h','CollisionVector_native.h']:(out/name).write_text((tree/'src/collision'/name).read_text(),newline='\n')
for name,source in [('Vector.h',saved/'src/math/Vector.h'),('Vector2D.h',saved/'src/math/Vector2D.h'),('Maths.h',saved/'src/math/Maths.h'),('VuVector.h',saved/'src/math/VuVector.h'),('VuCollision.h',saved/'src/collision/VuCollision.h'),('VuCollision.cpp',saved/'src/collision/VuCollision.cpp')]:(out/name).write_text(source.read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n#ifdef VU_COLLISION\n#define COMPRESSED_COL_VECTORS\n#endif\n#ifndef CLASSIC\n#define FIX_BUGS\n#endif\n',newline='\n')
source=(saved/'src/collision/Collision.h').read_text();start=source.index('struct CStoredCollPoly\n');end=source.index('\n};',start)+3;(out/'original-StoredCollPoly.h').write_text('#pragma once\n'+source[start:end]+'\n',newline='\n')
(out/'common.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
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
#ifdef _MSC_VER
#define TYPEALIGN(n) __declspec(align(n))
#else
#define TYPEALIGN(n) __attribute__((aligned(n)))
#endif
typedef int16_t int16;typedef int32_t int32;typedef uint8_t uint8;
struct RwV3d {float x,y,z;};
#include "Maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "VuVector.h"
#include "ColPoint.h"
#include "ColTriangle.h"
#include "CollisionVector_native.h"
#ifdef ORIGINAL
#include "original-StoredCollPoly.h"
#else
#include "StoredCollPoly_native.h"
#include "StoredCollPoly_backend.h"
#endif
#ifdef VU_COLLISION
#include "VuCollision.h"
#endif
class CCollision {
public:
#ifdef ORIGINAL
    static bool IsStoredPolyStillValidVerticalLine(const CVector &pos,float z,CColPoint &point,CStoredCollPoly *poly);
#endif
};
''',newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool Run_Validate(const void *pos,float z,void *point,void *poly);
void Run_Set(void *poly,uint32_t index,const void *vertex);
void Run_Get(const void *poly,uint32_t index,void *value);
bool Before_Validate(const void *pos,float z,void *point,void *poly);
void Before_Set(void *poly,uint32_t index,const void *vertex);
void Before_Get(const void *poly,uint32_t index,void *value);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "StoredCollPoly.h"
#include <string.h>
#endif
bool Run_Validate(const void *pos,float z,void *point,void *poly)
{
    // Preserve float references at the independent C11 boundary
#ifdef ORIGINAL
    return Before_Validate(pos,z,point,poly);
#else
    const unsigned char *p=(const unsigned char*)pos;
    return StoredPoly_IsValidVerticalLine(CollisionPoint_Ref((const float*)p,(const float*)(p+4),(const float*)(p+8)),z,(CColPoint*)point,(CStoredCollPoly*)poly);
#endif
}
void Run_Set(void *poly,uint32_t index,const void *vertex)
{
    // Exercise complete vertex writes through the public C interface
#ifdef ORIGINAL
    Before_Set(poly,index,vertex);
#else
    StoredPoly_SetVertex((CStoredCollPoly*)poly,index,(const StoredPolyVertex*)vertex);
#endif
}
void Run_Get(const void *poly,uint32_t index,void *value)
{
    // Compare exact XYZ query representations independently of native classes
#ifdef ORIGINAL
    Before_Get(poly,index,value);
#else
    CollisionVector3 result=StoredPoly_GetVertex((const CStoredCollPoly*)poly,index);memcpy(value,&result,12);
#endif
}
'''
original='#include "owners.h"\n#include "entry.h"\n'
original+='''extern "C" bool Before_Validate(const void *pos,float z,void *point,void *poly)
{
    // Invoke the entire original cached-validation policy
    return CCollision::IsStoredPolyStillValidVerticalLine(*(const CVector*)pos,z,*(CColPoint*)point,(CStoredCollPoly*)poly);
}
extern "C" void Before_Set(void *poly,uint32_t index,const void *vertex)
{
    // Invoke the original vertex assignment with an actual native vertex object
#ifdef VU_COLLISION
    ((CStoredCollPoly*)poly)->verts[index]=*(const CVuVector*)vertex;
#else
    ((CStoredCollPoly*)poly)->verts[index]=*(const CVector*)vertex;
#endif
}
extern "C" void Before_Get(const void *poly,uint32_t index,void *value)
{
    // Read the complete original XYZ representation without arithmetic
    memcpy(value,&((const CStoredCollPoly*)poly)->verts[index],12);
}
'''
vector=(saved/'src/math/Vector.cpp').read_text();math='#include "owners.h"\n'+method(vector,'CVector::Normalise')+'\n'+method(vector,'CrossProduct')+'\n'
collision=(saved/'src/collision/Collision.cpp').read_text();begin=collision.index('GetVUresult(CVuVector &point');begin=collision.rfind('\n',0,begin-1)+1
math+='#ifdef VU_COLLISION\n'+method(collision[begin:],'GetVUresult')+'\n#endif\n'
math+='#ifdef ORIGINAL\n'+method(collision,'CCollision::IsStoredPolyStillValidVerticalLine')+'\n#else\n'+m.backend()+'\n#endif\n'
math+='''extern "C" void *NativeStoredDefault(void *memory)
{
    // Keep default construction opaque to fixture storage initialization
    return new(memory) CStoredCollPoly;
}
extern "C" bool NativeStoredTemporary(float x,float y,float z,float end,void *point,void *poly)
{
    // Retain a native position temporary for the complete surrounding validation call
#ifdef ORIGINAL
    return CCollision::IsStoredPolyStillValidVerticalLine(CVector(x,y,z),end,*(CColPoint*)point,(CStoredCollPoly*)poly);
#else
    return StoredPoly_IsValidVerticalLine(StoredPoly_PositionRef(CVector(x,y,z)),end,(CColPoint*)point,(CStoredCollPoly*)poly);
#endif
}
extern "C" void NativeStoredFromXYZ(void *poly,uint32_t index,const CVector *value)
{
    // Retain the original native conversion including the unspecified VU fourth lane
#ifdef ORIGINAL
    ((CStoredCollPoly*)poly)->verts[index]=*value;
#else
    const StoredPolyVertex converted=StoredPoly_FromNative(*value);StoredPoly_SetVertex((CStoredCollPoly*)poly,index,&converted);
#endif
}
'''
for name,value in [('entry.h',entry),('caller.c',caller),('original.cpp',original),('math.cpp',math),('fixture.cpp',(root/'utils/tests/stored_poly.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared the complete original cached policy, C11 callers and the actual VU collision emulator')
