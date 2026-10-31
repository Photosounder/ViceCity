"""Move both complete collision triangle owners and scalar/VU plane policies into C11."""
import re
import object_names_migration as objects
RENAMES={'src/collision/ColTriangle.cpp':'src/collision/ColTriangle.c'}
OPERATIONS=['Set','SetFromVertices','GetNormal','GetNormalX','GetNormalY','GetNormalZ','CalcPoint','Unpack']
PLANE=r'(plane|(?:modelB|model)\.trianglePlanes\[[^\]\n]+\]|(?:triPlanes|pColTriPlanes)\[[^\]\n]+\]|collision->trianglePlanes\[[^\]\n]+\])'

def header(original):
    # Keep both record layouts, all direction values and live float references without C++ types
    enums=original[original.index('enum Direction'):original.index('struct CColTriangle')]
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "CompressedVector.h"
'''+enums+'''typedef struct CColTriangle {
    uint16_t a,b,c;
    uint8_t surface;
} CColTriangle;
typedef struct CColTrianglePlane {
#ifdef VU_COLLISION
    CompressedVector normal;
    int16_t dist;
#else
    CollisionVector3 normal;
    float dist;
    uint8_t dir;
#endif
} CColTrianglePlane;
typedef struct CollisionPointRef {
    const float *x,*y,*z;
} CollisionPointRef;
#ifdef __cplusplus
extern "C" {
#endif
static inline CollisionPointRef ColTrianglePlane_Ref(const float *x,const float *y,const float *z)
{
    // Retain live component references across C and native vector types
    const CollisionPointRef value={x,y,z};return value;
}
#define COLLISION_POINT_REF(value) ColTrianglePlane_Ref(&(value).x,&(value).y,&(value).z)
void ColTriangle_Set(CColTriangle *self,int32_t a,int32_t b,int32_t c,uint8_t surface);
void ColTrianglePlane_Set(CColTrianglePlane *self,CollisionPointRef a,CollisionPointRef b,CollisionPointRef c);
void ColTrianglePlane_SetFromVertices(CColTrianglePlane *self,const CompressedVector *vertices,const CColTriangle *triangle);
void ColTrianglePlane_GetNormal(const CColTrianglePlane *self,float *x,float *y,float *z);
float ColTrianglePlane_CalcPoint(const CColTrianglePlane *self,CollisionPointRef point);
#ifdef VU_COLLISION
void ColTrianglePlane_Unpack(const CColTrianglePlane *self,void *quadword);
#else
float ColTrianglePlane_GetNormalX(const CColTrianglePlane *self);
float ColTrianglePlane_GetNormalY(const CColTrianglePlane *self);
float ColTrianglePlane_GetNormalZ(const CColTrianglePlane *self);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original_header,original_source):
    # Preserve the original vector math ordering, partial fallback and both output representations
    assembly=original_header[original_header.index('\t\t__asm__ volatile ('):original_header.index('\n\t}',original_header.index('\t\t__asm__ volatile ('))]
    assembly=assembly.replace('"=m" (qword)','"=m" (*(TriangleQuadWord*)quadword)').replace('"r" (this)','"r" (self)')
    return '''//+ rouz edit (ChatGPT)
#include "ColTriangle.h"
#include <math.h>

static CollisionVector3 Triangle_Cross(CollisionPointRef a,CollisionPointRef b,CollisionPointRef c)
{
    // Capture both complete differences before writing any destination normal component
    const CollisionVector3 first={*c.x-*a.x,*c.y-*a.y,*c.z-*a.z};
    const CollisionVector3 second={*b.x-*a.x,*b.y-*a.y,*b.z-*a.z};
    const CollisionVector3 result={first.y*second.z-first.z*second.y,first.z*second.x-first.x*second.z,first.x*second.y-first.y*second.x};
    return result;
}
static void Triangle_Normalise(CollisionVector3 *value)
{
    // Preserve CVector normalization including the fallback that writes only X
    const float square=value->x*value->x+value->y*value->y+value->z*value->z;
    if(square>0.0f){
        // Use the original reciprocal square root and ordered component updates
        const float inverse=1.0f/sqrtf(square);
        value->x*=inverse;value->y*=inverse;value->z*=inverse;
    }else value->x=1.0f;
}
void ColTriangle_Set(CColTriangle *self,int32_t a,int32_t b,int32_t c,uint8_t surface)
{
    // Preserve index narrowing, member order and untouched triangle padding
    self->a=a;self->b=b;self->c=c;self->surface=surface;
}
void ColTrianglePlane_Set(CColTrianglePlane *self,CollisionPointRef a,CollisionPointRef b,CollisionPointRef c)
{
#ifdef VU_COLLISION
    // Compute the complete normal and distance before narrowing the packed outputs
    CollisionVector3 normal=Triangle_Cross(a,b,c);
    Triangle_Normalise(&normal);
    const float distance=normal.x * *a.x+normal.y * *a.y+normal.z * *a.z;
    self->normal.x=normal.x*4096.0f;
    self->normal.y=normal.y*4096.0f;
    self->normal.z=normal.z*4096.0f;
    self->dist=distance*128.0f;
#else
    // Keep source references live after the destination normal has been overwritten and normalized
    self->normal=Triangle_Cross(a,b,c);
    Triangle_Normalise(&self->normal);
    self->dist=self->normal.x * *a.x+self->normal.y * *a.y+self->normal.z * *a.z;
    const CollisionVector3 absolute={fabsf(self->normal.x),fabsf(self->normal.y),fabsf(self->normal.z)};
    if(absolute.x>absolute.y&&absolute.x>absolute.z)
        self->dir=self->normal.x<0.0f?DIR_X_NEG:DIR_X_POS;
    else if(absolute.y>absolute.z)
        self->dir=self->normal.y<0.0f?DIR_Y_NEG:DIR_Y_POS;
    else
        self->dir=self->normal.z<0.0f?DIR_Z_NEG:DIR_Z_POS;
#endif
}
void ColTrianglePlane_SetFromVertices(CColTrianglePlane *self,const CompressedVector *vertices,const CColTriangle *triangle)
{
    // Decode all three original temporaries before invoking the complete plane operation
    const CollisionVector3 a=CompressedVector_Get(&vertices[triangle->a]);
    const CollisionVector3 b=CompressedVector_Get(&vertices[triangle->b]);
    const CollisionVector3 c=CompressedVector_Get(&vertices[triangle->c]);
    ColTrianglePlane_Set(self,COLLISION_POINT_REF(a),COLLISION_POINT_REF(b),COLLISION_POINT_REF(c));
}
void ColTrianglePlane_GetNormal(const CColTrianglePlane *self,float *x,float *y,float *z)
{
    // Preserve ordered output writes and the original normal scaling
#ifdef VU_COLLISION
    *x=self->normal.x/4096.0f;*y=self->normal.y/4096.0f;*z=self->normal.z/4096.0f;
#else
    *x=self->normal.x;*y=self->normal.y;*z=self->normal.z;
#endif
}
float ColTrianglePlane_CalcPoint(const CColTrianglePlane *self,CollisionPointRef point)
{
    // Retain the original dot-product grouping and representation-specific distance scale
#ifdef VU_COLLISION
    CollisionVector3 normal;
    ColTrianglePlane_GetNormal(self,&normal.x,&normal.y,&normal.z);
    return normal.x * *point.x+normal.y * *point.y+normal.z * *point.z-self->dist/128.0f;
#else
    return self->normal.x * *point.x+self->normal.y * *point.y+self->normal.z * *point.z-self->dist;
#endif
}
#ifdef VU_COLLISION
void ColTrianglePlane_Unpack(const CColTrianglePlane *self,void *quadword)
{
    // Preserve all four packed plane lanes and the native PS2 instruction sequence
#ifdef GTA_PS2
    typedef unsigned int TriangleQuadWord __attribute__((mode(TI)));
'''+assembly+'''
#else
    int32_t *qword=(int32_t*)quadword;
    qword[0]=self->normal.x;qword[1]=self->normal.y;qword[2]=self->normal.z;qword[3]=self->dist;
#endif
}
#else
float ColTrianglePlane_GetNormalX(const CColTrianglePlane *self)
{
    // Return the original X component
    return self->normal.x;
}
float ColTrianglePlane_GetNormalY(const CColTrianglePlane *self)
{
    // Return the original Y component
    return self->normal.y;
}
float ColTrianglePlane_GetNormalZ(const CColTrianglePlane *self)
{
    // Return the original Z component
    return self->normal.z;
}
#endif
//- rouz edit (ChatGPT)
'''

def col_triangle_calls(source):
    # Remove both complete native owners and replace every typed triangle or plane consumer
    original=source
    if 'struct CColTriangle\n' in source and 'void Set(int a, int b, int c' in source:return header(source)
    if 'CColTrianglePlane::Set(' in source:
        from pathlib import Path
        root=Path(__file__).resolve().parents[2];saved=root/'.diagnostics/col-triangle-c-tests/before/src/collision/ColTriangle.h'
        return kernel(saved.read_text() if saved.exists() else (root/'src/collision/ColTriangle.h').read_text(),source)
    source=re.sub(r'(model\.triangles\[i\])\.Set\(',r'ColTriangle_Set(&\1,',source)
    source=source.replace('trianglePlanes[i].Set(vertices, triangles[i]);','ColTrianglePlane_SetFromVertices(&trianglePlanes[i],vertices,&triangles[i]);')
    source=source.replace('plane.Set(poly->verts[0], poly->verts[1], poly->verts[2]);','ColTrianglePlane_Set(&plane,COLLISION_POINT_REF(poly->verts[0]),COLLISION_POINT_REF(poly->verts[1]),COLLISION_POINT_REF(poly->verts[2]));')
    source=source.replace('CVector n(collision->trianglePlanes[i].GetNormalX(), collision->trianglePlanes[i].GetNormalY(), collision->trianglePlanes[i].GetNormalZ());','CVector n;\n                // Read the complete normal through the representation-independent C operation\n                ColTrianglePlane_GetNormal(&collision->trianglePlanes[i],&n.x,&n.y,&n.z);')
    source=re.sub(PLANE+r'\.GetNormal\((\w+)\)',lambda m:'ColTrianglePlane_GetNormal(&'+m[1]+',&'+m[2]+'.x,&'+m[2]+'.y,&'+m[2]+'.z)',source)
    source=re.sub(PLANE+r'\.GetNormal([XYZ])\(\)',r'ColTrianglePlane_GetNormal\2(&\1)',source)
    source=re.sub(PLANE+r'\.CalcPoint\(([^()]+)\)',r'ColTrianglePlane_CalcPoint(&\1,COLLISION_POINT_REF(\2))',source)
    source=re.sub(PLANE+r'\.Unpack\((vutri\.plane)\)',r'ColTrianglePlane_Unpack(&\1,&\2)',source)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def col_triangle_project(source):
    # Rename the complete producer while retaining explicit C11 native-project metadata
    source=source.replace('..\\src\\collision\\ColTriangle.cpp','..\\src\\collision\\ColTriangle.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\collision\\ColTriangle\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve other options while selecting the C language
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\collision\\ColTriangle.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)



def calls(source):
    # Retain complete triangle ownership before the line owner and shared coordinate boundaries
    import col_line_migration as lines
    return lines.calls(col_triangle_calls(source))


def project(source):
    # Retain triangle C11 integration before the complete line producer rename
    import col_line_migration as lines
    return lines.project(col_triangle_project(source))
