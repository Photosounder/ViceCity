"""Move complete compressed/uncompressed collision vertex storage and conversion policy into C11."""
import re
import object_names_migration as objects
OPERATIONS=['Get','Set','Unpack']
RENAMES={}

def header(original):
    # Preserve both native representations without exposing C++ vector or reference types
    assert 'CVector Get(void) const' in original
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"
typedef struct CollisionVector3 { float x, y, z; } CollisionVector3;
typedef struct CompressedVector {
#ifdef COMPRESSED_COL_VECTORS
    int16_t x, y, z;
#else
    float x, y, z;
#endif
} CompressedVector;
#ifdef __cplusplus
extern "C" {
#endif
CollisionVector3 CompressedVector_Get(const CompressedVector *self);
void CompressedVector_Set(CompressedVector *self,float x,float y,float z);
#ifdef COMPRESSED_COL_VECTORS
void CompressedVector_Unpack(const CompressedVector *self,void *quadword);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Keep exact scaling, ordered narrowing and the existing PS2 instruction sequence
    assembly=original[original.index('\t\t__asm__ volatile ('):original.index('\n\t}',original.index('\t\t__asm__ volatile ('))]
    assembly=assembly.replace('"=m" (qword)','"=m" (*(CollisionQuadWord*)quadword)').replace('"r" (this)','"r" (self)')
    return '''//+ rouz edit (ChatGPT)
#include "CompressedVector.h"
CollisionVector3 CompressedVector_Get(const CompressedVector *self)
{
    // Decode every coordinate before returning the original by-value result
#ifdef COMPRESSED_COL_VECTORS
    const CollisionVector3 value={self->x/128.0f,self->y/128.0f,self->z/128.0f};
#else
    const CollisionVector3 value={self->x,self->y,self->z};
#endif
    return value;
}
void CompressedVector_Set(CompressedVector *self,float x,float y,float z)
{
    // Preserve coordinate assignment order and the original floating point conversion contract
#ifdef COMPRESSED_COL_VECTORS
    self->x=x*128.0f;self->y=y*128.0f;self->z=z*128.0f;
#else
    self->x=x;self->y=y;self->z=z;
#endif
}
#ifdef COMPRESSED_COL_VECTORS
void CompressedVector_Unpack(const CompressedVector *self,void *quadword)
{
    // Retain the native quadword unpack operation and its original fourth-lane behavior
#ifdef GTA_PS2
    typedef unsigned int CollisionQuadWord __attribute__((mode(TI)));
'''+assembly+'''
#else
    int32_t *qword=(int32_t*)quadword;
    qword[0]=self->x;qword[1]=self->y;qword[2]=self->z;qword[3]=0;
#endif
}
#endif
//- rouz edit (ChatGPT)
'''

def compressed_vector_calls(source):
    # Remove the complete owner methods and adapt every typed native collision consumer
    if 'CVector Get(void) const' in source and 'struct CompressedVector\n' in source:return header(source)
    original=source
    old='void Set(const CompressedVector *v, CColTriangle &tri) { Set(v[tri.a].Get(), v[tri.b].Get(), v[tri.c].Get()); }'
    new='''void Set(const CompressedVector *v, CColTriangle &tri) {
        // Decode all vertex values before invoking the native plane operation
        const CollisionVector3 a=CompressedVector_Get(&v[tri.a]);
        const CollisionVector3 b=CompressedVector_Get(&v[tri.b]);
        const CollisionVector3 c=CompressedVector_Get(&v[tri.c]);
        Set(CVector(a.x,a.y,a.z),CVector(b.x,b.y,b.z),CVector(c.x,c.y,c.z));
    }'''
    source=source.replace(old,new)
    if source==original and not re.search(r'\b(?:vertices|verts)\[[^\]\n]+\]\.(?:Get|Set|Unpack)\(',source):return source
    receiver=r'((?:(?:modelB|model)\.vertices|(?:col|colmodel)->vertices|vertices|verts)\[[^\]\n]+\])'
    pattern=r'(?m)^([\t ]*)([^;\n]+?) = ([^;\n]*'+receiver+r'\.Get\(\)[^;\n]*);'
    matches=list(re.finditer(pattern,source))
    for index,match in reversed(list(enumerate(matches))):
        indent,target,expression=match[1],match[2],match[3];declarations=[]
        getters=list(re.finditer(receiver+r'\.Get\(\)',expression))
        for n,getter in reversed(list(enumerate(getters))):
            name='collisionVertexValue'+str(index)+'_'+str(n)
            declarations.insert(0,indent+'const CollisionVector3 '+name+'=CompressedVector_Get(&'+getter[1]+');')
            expression=expression[:getter.start()]+'CVector('+name+'.x,'+name+'.y,'+name+'.z)'+expression[getter.end():]
        replacement=indent+'// Copy complete decoded values before the original native vector expression\n'+'\n'.join(declarations)+'\n'+indent+target+' = '+expression+';'
        source=source[:match.start()]+replacement+source[match.end():]
    source=re.sub(receiver+r'\.Set\(',r'CompressedVector_Set(&\1,',source)
    source=re.sub(receiver+r'\.Unpack\((vutri\.v[012])\)',r'CompressedVector_Unpack(&\1,&\2)',source)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def compressed_vector_project(source):
    # Register the standalone C producer alongside its existing public header
    tag='ClCompile';old='ColModel.cpp';new='CompressedVector.c';entry='<'+tag+' Include="..\\src\\collision\\'+old+'" />'
    addition='<ClCompile Include="..\\src\\collision\\CompressedVector.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
    if '..\\src\\collision\\'+new+'"' not in source:
        source=source.replace(entry,entry+'\n    '+addition)
        pattern=r'(<ClCompile Include="\.\.\\src\\collision\\ColModel\.cpp">\n\s*<Filter>collision</Filter>\n\s*</ClCompile>)'
        source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)


def calls(source):
    # Preserve collision vertex storage before the complete triangle and plane owners and their consumers
    import col_triangle_migration as triangles
    return triangles.calls(compressed_vector_calls(source))

def project(source):
    # Retain the vertex producer before the complete C11 triangle producer rename
    import col_triangle_migration as triangles
    return triangles.project(compressed_vector_project(source))
