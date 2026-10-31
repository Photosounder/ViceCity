"""Move the complete stored-polygon record and cached vertical-line validation into C."""
import re
from rect_migration import mask,closing,arguments
from doors_migration import method
import object_names_migration as objects
RENAMES={}

def header():
    # Preserve both historical vertex strides and aggregate alignment using C-compatible records
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#include "ColPoint.h"
#ifdef VU_COLLISION
typedef struct StoredPolyVertex {
#ifdef __cplusplus
    alignas(16) float x;
#else
    _Alignas(16) float x;
#endif
    float y,z,w;
} StoredPolyVertex;
#else
typedef CollisionVector3 StoredPolyVertex;
#endif
typedef struct CStoredCollPoly {
    StoredPolyVertex verts[3];
    bool valid;
} CStoredCollPoly;
#ifdef __cplusplus
extern "C" {
#endif
void StoredPoly_SetVertex(CStoredCollPoly *self,uint32_t index,const StoredPolyVertex *value);
CollisionVector3 StoredPoly_GetVertex(const CStoredCollPoly *self,uint32_t index);
bool StoredPoly_IsValidVerticalLine(CollisionPointRef pos,float z,CColPoint *point,CStoredCollPoly *poly);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def native():
    # Retain the original native conversion boundary including the unspecified VU fourth lane
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "StoredCollPoly.h"
#include "CollisionVector_native.h"
#include "VuVector.h"
static inline StoredPolyVertex StoredPoly_FromNative(const CVector &value)
{
    // Keep the original native vertex conversion before transferring its complete representation
#ifdef VU_COLLISION
    const CVuVector converted(value);
    StoredPolyVertex result;memcpy(&result,&converted,sizeof(result));return result;
#else
    return CollisionVector_FromNative(value);
#endif
}
static inline CVector StoredPoly_ToNative(const StoredPolyVertex &value)
{
    // Copy only the XYZ coordinates used by the remaining native scalar expressions
    CVector result;memcpy(&result.x,&value.x,sizeof(float));memcpy(&result.y,&value.y,sizeof(float));memcpy(&result.z,&value.z,sizeof(float));return result;
}
static inline CollisionPointRef StoredPoly_PositionRef(const CVector &value)
{
    // Evaluate a native position once and retain its temporary through the surrounding C call
    return COLLISION_POINT_REF(value);
}
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Expose only the existing VU execution and result capture through a C interface
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "StoredCollPoly.h"
#ifdef VU_COLLISION
#ifdef __cplusplus
extern "C" {
#endif
bool StoredPoly_BackendLine(const StoredPolyVertex *p0,const StoredPolyVertex *p1,const CStoredCollPoly *poly,const StoredPolyVertex *plane,CollisionVector3 *point);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
'''

def backend():
    # Keep actual VU calls and the original result-fetch helper in their existing compilation unit
    return '''
#ifdef VU_COLLISION
bool StoredPoly_BackendLine(const StoredPolyVertex *p0,const StoredPolyVertex *p1,const CStoredCollPoly *poly,const StoredPolyVertex *plane,CollisionVector3 *point)
{
    // Transfer aligned C quadwords into actual native VU vector objects
    CVuVector a,b,v0,v1,v2,n;
    memcpy(&a,p0,16);memcpy(&b,p1,16);memcpy(&v0,&poly->verts[0],16);memcpy(&v1,&poly->verts[1],16);memcpy(&v2,&poly->verts[2],16);memcpy(&n,plane,16);
    LineToTriangleCollision(a,b,v0,v1,v2,n);
    // Capture the original VU outputs even on failure and publish XYZ only for a successful result
    CVuVector result;float distance;const int hit=GetVUresult(result,n,distance);
    if(hit){memcpy(&point->x,&result.x,sizeof(float));memcpy(&point->y,&result.y,sizeof(float));memcpy(&point->z,&result.z,sizeof(float));}
    return hit!=0;
}
#endif
'''

def scalar_branch(original):
    # Select the outer scalar branch while retaining nested fixed/classic VU alternatives separately
    body=method(original,'CCollision::IsStoredPolyStillValidVerticalLine');depth=0;begin=None
    for line in re.finditer(r'(?m)^#(if\w*|else|endif)[^\n]*\n?',body):
        if line[1].startswith('if'):depth+=1
        elif line[1]=='else' and depth==1:begin=line.end()
        elif line[1]=='endif':
            depth-=1
            if depth==0 and begin is not None:return body[begin:line.start()]
    raise AssertionError('Missing scalar cached-polygon branch')

def kernel(original):
    # Retain the complete scalar policy and the exact uncompressed VU preparation in C
    scalar=scalar_branch(original)
    for name,index in [('va',0),('vb',1),('vc',2)]:scalar=scalar.replace('const CVector &'+name+' = poly->verts['+str(index)+'];','const StoredPolyVertex *'+name+' = &poly->verts['+str(index)+'];')
    scalar=scalar.replace('CVector p0 = pos;','const CollisionVector3 p0={*pos.x,*pos.y,*pos.z};').replace('CVector p1(pos.x, pos.y, z);','const CollisionVector3 p1={*pos.x,*pos.y,z};')
    scalar=scalar.replace('CVector normal;','CollisionVector3 normal;').replace('DotProduct(p1 - p0, normal)','StoredPoly_Dot(StoredPoly_Sub(p1,p0),normal)')
    scalar=scalar.replace('CVector p = p0 + (p1-p0)*t;','const CollisionVector3 p=StoredPoly_Add(p0,StoredPoly_Scale(StoredPoly_Sub(p1,p0),t));')
    scalar=scalar.replace('CVector2D vec1, vec2, vec3, vect;','StoredPolyVector2 vec1, vec2, vec3, vect;')
    scalar=re.sub(r'\b(va|vb|vc)\.',r'\1->',scalar)
    scalar=re.sub(r'CrossProduct2D\((vec[123])-(vec[123]), (vect)-(vec[123])\)',r'StoredPoly_Cross2(StoredPoly_Sub2(\1,\2),StoredPoly_Sub2(\3,\4))',scalar)
    scalar=scalar.replace('point.point = CollisionVector_FromNative(p);','point->point=p;')
    return '''//+ rouz edit (ChatGPT)
#include "StoredCollPoly.h"
#include "StoredCollPoly_backend.h"
#include "ColTriangle.h"
#include <string.h>
#include <math.h>
#include <assert.h>
static CollisionVector3 StoredPoly_Sub(CollisionVector3 a,CollisionVector3 b)
{
    // Retain complete native vector subtraction before subsequent arithmetic
    const CollisionVector3 value={a.x-b.x,a.y-b.y,a.z-b.z};return value;
}
static float StoredPoly_Dot(CollisionVector3 a,CollisionVector3 b)
{
    // Preserve left-associated native dot-product grouping
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
#ifndef VU_COLLISION
typedef struct StoredPolyVector2 {float x,y;} StoredPolyVector2;
static CollisionVector3 StoredPoly_Add(CollisionVector3 a,CollisionVector3 b)
{
    // Retain complete native addition after the scaled line difference
    const CollisionVector3 value={a.x+b.x,a.y+b.y,a.z+b.z};return value;
}
static CollisionVector3 StoredPoly_Scale(CollisionVector3 a,float scale)
{
    // Keep the original component multiplication before adding the line origin
    const CollisionVector3 value={a.x*scale,a.y*scale,a.z*scale};return value;
}
static StoredPolyVector2 StoredPoly_Sub2(StoredPolyVector2 a,StoredPolyVector2 b)
{
    // Preserve the original two-dimensional difference temporaries
    const StoredPolyVector2 value={a.x-b.x,a.y-b.y};return value;
}
static float StoredPoly_Cross2(StoredPolyVector2 a,StoredPolyVector2 b)
{
    // Retain the original signed edge test
    return a.x*b.y-a.y*b.x;
}
#endif
void StoredPoly_SetVertex(CStoredCollPoly *self,uint32_t index,const StoredPolyVertex *value)
{
    // Capture the complete original vertex before writing its destination including any VU fourth lane
    StoredPolyVertex copy;memcpy(&copy,value,sizeof(copy));memcpy(&self->verts[index],&copy,sizeof(copy));
}
CollisionVector3 StoredPoly_GetVertex(const CStoredCollPoly *self,uint32_t index)
{
    // Return exact XYZ representations without a native class view or arithmetic
    CollisionVector3 value;memcpy(&value.x,&self->verts[index].x,sizeof(float));memcpy(&value.y,&self->verts[index].y,sizeof(float));memcpy(&value.z,&self->verts[index].z,sizeof(float));return value;
}
bool StoredPoly_IsValidVerticalLine(CollisionPointRef pos,float z,CColPoint *point,CStoredCollPoly *poly)
{
#ifdef VU_COLLISION
    // Retain the original invalid-cache early exit before reading any position coordinates
    if(!poly->valid)return false;
    // Snapshot the original two line endpoints and fill unused scratch lanes consumed only as XYZ
    const StoredPolyVertex p0={*pos.x,*pos.y,*pos.z,0.0f};
    const StoredPolyVertex p1={*pos.x,*pos.y,z,0.0f};
    const CollisionVector3 a=StoredPoly_GetVertex(poly,0),b=StoredPoly_GetVertex(poly,1),c=StoredPoly_GetVertex(poly,2);
    const CollisionVector3 v01=StoredPoly_Sub(b,a),v02=StoredPoly_Sub(c,a);
    // Keep the original cross-product direction and native normalization including its X-only fallback
    CollisionVector3 normal={v02.y*v01.z-v02.z*v01.y,v02.z*v01.x-v02.x*v01.z,v02.x*v01.y-v02.y*v01.x};
    const float square=StoredPoly_Dot(normal,normal);
    if(square>0.0f){
        // Apply the same reciprocal square root and ordered component writes
        const float inverse=1.0f/sqrtf(square);normal.x*=inverse;normal.y*=inverse;normal.z*=inverse;
    }else normal.x=1.0f;
    // Execute the existing VU routine with the original uncompressed plane distance
    const StoredPolyVertex plane={normal.x,normal.y,normal.z,StoredPoly_Dot(normal,a)};
    CollisionVector3 result;
    if(!StoredPoly_BackendLine(&p0,&p1,poly,&plane,&result)){
#ifdef FIX_BUGS
        // Preserve the fixed build's cache invalidation on a failed VU result
        return poly->valid=false;
#else
        // Preserve the classic build's unchanged valid flag on failure
        return false;
#endif
    }
    // Publish only the point coordinates while leaving every other contact field and valid flag unchanged
    point->point=result;return true;
#else
    // Preserve the full original scalar cached-polygon policy and all six projection directions
'''+scalar+'''#endif
}
//- rouz edit (ChatGPT)
'''

def stored_poly_calls(source):
    # Remove the complete native data definition and cached-validation class method before adapting consumers
    original=source
    if 'struct CStoredCollPoly\n' in source:
        start=source.index('struct CStoredCollPoly\n');end=source.index('\n};',start)+3
        source=source[:start]+'#include "StoredCollPoly.h"'+source[end:]
        source=source.replace('\tstatic bool IsStoredPolyStillValidVerticalLine(const CVector &pos, float z, CColPoint &point, CStoredCollPoly *poly);\n','')
    if re.search(r'(?m)^CCollision::IsStoredPolyStillValidVerticalLine\(',source):
        source=source.replace(method(source,'CCollision::IsStoredPolyStillValidVerticalLine'),'')
    if '#include "common.h"' in source and 'int\nGetVUresult(CVuVector &point' in source and 'bool StoredPoly_BackendLine(' not in source:
        position=source.index('\neLevelName CCollision::ms_collisionInMemory;');source=source[:position]+backend()+source[position:]
        source=source.replace('#include "common.h"','#include "common.h"\n#include "StoredCollPoly_backend.h"',1)
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bCCollision::IsStoredPolyStillValidVerticalLine\(',code))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);assert len(args)==4,args
        source=source[:match.start()]+'StoredPoly_IsValidVerticalLine(StoredPoly_PositionRef('+args[0]+'),'+args[1]+',&'+args[2]+','+args[3]+')'+source[end:]
    # Adapt each complete native vertex expression before the original individual vertex write
    pattern=r'(?m)^([\t ]*)((?:poly->|TempStoredPoly\.)verts\[([012])\]) = ([^;]+);'
    for match in reversed(list(re.finditer(pattern,source))):
        indent,target,index,value=match[1],match[2],match[3],match[4]
        value=re.sub(r'CVector\((poly->verts\[[012]\])\)',r'StoredPoly_ToNative(\1)',value)
        value=re.sub(r'(?<!\w)(poly->verts\[[012]\])(?!\))',r'StoredPoly_ToNative(\1)',value)
        owner='poly' if target.startswith('poly->') else '&TempStoredPoly'
        replacement='''@INDENT@{
@INDENT@    // Preserve the complete native conversion before the individual stored-vertex update
@INDENT@    const StoredPolyVertex storedVertex=StoredPoly_FromNative(@VALUE@);
@INDENT@    StoredPoly_SetVertex(@OWNER@,@INDEX@,&storedVertex);
@INDENT@}'''
        for key,valueText in [('INDENT',indent),('VALUE',value),('OWNER',owner),('INDEX',index)]:replacement=replacement.replace('@'+key+'@',valueText)
        source=source[:match.start()]+replacement+source[match.end():]
    if source==original:return source
    if '#include "common.h"' in source and '#include "StoredCollPoly_native.h"' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "StoredCollPoly_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def stored_poly_project(source):
    # Register the C record, complete policy producer and native VU interface in legacy project projections
    anchor='<ClInclude Include="..\\src\\collision\\Collision.h" />'
    for name in ['StoredCollPoly.h','StoredCollPoly_backend.h','StoredCollPoly_native.h']:
        entry='<ClInclude Include="..\\src\\collision\\'+name+'" />'
        if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    anchor='<ClCompile Include="..\\src\\collision\\Collision.cpp"'
    if 'Microsoft.Cpp.targets' in source and '..\\src\\collision\\StoredCollPoly.c"' not in source:
        entry='<ClCompile Include="..\\src\\collision\\StoredCollPoly.c"><CompileAs>CompileAsC</CompileAs><LanguageStandard_C>stdc11</LanguageStandard_C></ClCompile>\n    '
        source=source.replace(anchor,entry+anchor,1)
    return objects.project(source)



def calls(source):
    # Preserve complete cached-polygon ownership before the quaternion record and its math
    import quaternion_migration as quaternions
    return quaternions.calls(stored_poly_calls(source))


def project(source):
    # Retain cached-polygon C11 integration before the complete quaternion producer rename
    import quaternion_migration as quaternions
    return quaternions.project(stored_poly_project(source))
