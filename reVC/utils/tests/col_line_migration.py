"""Move the complete collision-line record to C and make native vector boundaries explicit."""
import re
from rect_migration import mask,closing,arguments
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/collision/ColLine.cpp':'src/collision/ColLine.c'}
POINT=r'\b((?:line|ourLine|shootLine)\.p[01]|\w+(?:->|\.)lines\[[^\]\n]+\]\.p[01])'

def shared():
    # Share plain coordinates and live references between all C collision primitives
    return '''//+ rouz edit (ChatGPT)
#pragma once
typedef struct CollisionVector3 { float x, y, z; } CollisionVector3;
typedef struct CollisionPointRef { const float *x,*y,*z; } CollisionPointRef;
static inline CollisionPointRef CollisionPoint_Ref(const float *x,const float *y,const float *z)
{
    // Retain live references to float subobjects without depending on a native vector type
    const CollisionPointRef value={x,y,z};return value;
}
#define COLLISION_POINT_REF(value) CollisionPoint_Ref(&(value).x,&(value).y,&(value).z)
//- rouz edit (ChatGPT)
'''

def native_vectors():
    # Keep the remaining C++ vector representation behind explicit bit-preserving value conversions
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "CollisionVector.h"
#include "Vector.h"
#include <string.h>
static inline CVector CollisionVector_ToNative(const CollisionVector3 &value)
{
    // Copy coordinate representations without arithmetic or struct type punning
    CVector result;
    memcpy(&result.x,&value.x,sizeof(float));memcpy(&result.y,&value.y,sizeof(float));memcpy(&result.z,&value.z,sizeof(float));
    return result;
}
static inline CollisionVector3 CollisionVector_FromNative(const CVector &value)
{
    // Preserve native coordinate bits in the plain C representation
    CollisionVector3 result;
    memcpy(&result.x,&value.x,sizeof(float));memcpy(&result.y,&value.y,sizeof(float));memcpy(&result.z,&value.z,sizeof(float));
    return result;
}
//- rouz edit (ChatGPT)
'''

def native_lines():
    # Replace the old sixteen-byte native views with an explicit representation transfer
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "ColLine.h"
#include "CollisionVector_native.h"
#include "VuVector.h"
static inline CVuVector ColLine_ToVu(const CColLine &line,unsigned endpoint)
{
    // Preserve every quadword bit including the original fourth lane
    CVuVector value;
    memcpy(&value,(const unsigned char*)&line+endpoint*16,16);
    return value;
}
//- rouz edit (ChatGPT)
'''

def header(original):
    # Preserve the complete two-quadword layout without C++ vector members or constructors
    assert 'CColLine(const CVector &p0' in original
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct CColLine {
    CollisionVector3 p0;
    int32_t pad0;
    CollisionVector3 p1;
    int32_t pad1;
} CColLine;
#ifdef __cplusplus
extern "C" {
#endif
void ColLine_Init(CColLine *self,CollisionPointRef p0,CollisionPointRef p1);
void ColLine_Set(CColLine *self,CollisionPointRef p0,CollisionPointRef p1);
CColLine ColLine_Make(CollisionPointRef p0,CollisionPointRef p1);
CColLine ColLine_FromRaw32(const void *bytes);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Retain the original first-endpoint then second-endpoint copy contract and untouched padding
    return '''//+ rouz edit (ChatGPT)
#include "ColLine.h"
#include <string.h>
static void ColLine_Write(CColLine *self,CollisionPointRef p0,CollisionPointRef p1)
{
    // Capture the complete first vector before writing its destination
    CollisionVector3 first;
    memcpy(&first.x,p0.x,sizeof(float));memcpy(&first.y,p0.y,sizeof(float));memcpy(&first.z,p0.z,sizeof(float));
    memcpy(&self->p0,&first,sizeof(first));
    // Read the second vector only after the first destination has been updated
    CollisionVector3 second;
    memcpy(&second.x,p1.x,sizeof(float));memcpy(&second.y,p1.y,sizeof(float));memcpy(&second.z,p1.z,sizeof(float));
    memcpy(&self->p1,&second,sizeof(second));
}
void ColLine_Init(CColLine *self,CollisionPointRef p0,CollisionPointRef p1)
{
    // Preserve parameterized construction without writing either padding word
    ColLine_Write(self,p0,p1);
}
void ColLine_Set(CColLine *self,CollisionPointRef p0,CollisionPointRef p1)
{
    // Preserve the original ordered member assignment and valid endpoint aliases
    ColLine_Write(self,p0,p1);
}
CColLine ColLine_Make(CollisionPointRef p0,CollisionPointRef p1)
{
    // Construct a temporary line while retaining the original unspecified padding-word values
    CColLine value;ColLine_Write(&value,p0,p1);return value;
}
CColLine ColLine_FromRaw32(const void *bytes)
{
    // Transfer both complete quadwords into an actual C record instead of creating a typed view
    CColLine value;memcpy(&value,bytes,sizeof(value));return value;
}
//- rouz edit (ChatGPT)
'''

def transform(original):
    # Keep the original batch assembly and scalar matrix operation with typed line access
    source=method(original,'TransformPoints');assembly=source[source.index('\t__asm__ __volatile__'):source.index('\n#else')]
    return '''
void TransformColLines(CVuVector *out,int numLines,const CMatrix &mat,const CColLine *lines)
{
#ifdef GTA_PS3
    // Retain the existing native batch path with the same two sixteen-byte endpoints per line
    int n=numLines*2;const void *in=lines;const int stride=sizeof(CColLine)/2;
'''+assembly+'''
#else
    // Preserve the original point order and native matrix multiplication without unrelated struct aliases
    while(numLines--){
        *out=mat*CollisionVector_ToNative(lines->p0);out++;
        *out=mat*CollisionVector_ToNative(lines->p1);out++;
        lines++;
    }
#endif
}
'''

def lvalue(value):
    # Recognize native or C coordinate lvalues that can retain live references directly
    return bool(re.fullmatch(r'\*?\w+(?:(?:\.|->)\w+|\[[^\]]+\])*',value.strip()))

def col_line_calls(source):
    # Convert the complete owner, shared coordinate definitions and every native line consumer
    original=source
    if 'CColLine(const CVector &p0' in source:return header(source)
    if 'CColLine::Set(' in source:return kernel(source)
    old='typedef struct CollisionVector3 { float x, y, z; } CollisionVector3;'
    if old in source and '} CompressedVector;' in source:source=source.replace(old,'#include "CollisionVector.h"')
    if 'static inline CollisionPointRef ColTrianglePlane_Ref(' in source:
        source=re.sub(r'typedef struct CollisionPointRef \{[\s\S]*?\} CollisionPointRef;\n','',source,count=1)
        source=re.sub(r'static inline CollisionPointRef ColTrianglePlane_Ref\([\s\S]*?\n\}\n#define COLLISION_POINT_REF\(value\)[^\n]*\n','',source,count=1)
    if 'void TransformPoints(CVuVector *out, int n, const CMatrix &mat, const RwV3d *in, int stride)' in source and '#include "common.h"' in source and 'void TransformColLines(' not in source:
        source=source.replace('#include "VuVector.h"','#include "VuVector.h"\n#include "ColLine_native.h"')
        source+=transform(original)
    prototype='void TransformPoints(CVuVector *out, int n, const CMatrix &mat, const RwV3d *in, int stride);'
    if prototype in source and 'void TransformColLines(' not in source:source=source.replace(prototype,prototype+'\nstruct CColLine;\nvoid TransformColLines(CVuVector *out,int numLines,const CMatrix &mat,const CColLine *lines);')
    source=source.replace('TransformPoints(newline, 2, matTransform, &line.p0, sizeof(CColLine)/2);','TransformColLines(newline,1,matTransform,&line);')
    source=source.replace('TransformPoints(aLinesA, modelA.numLines*2, matAB, &modelA.lines->p0, sizeof(CColLine)/2);','TransformColLines(aLinesA,modelA.numLines,matAB,modelA.lines);')
    source=source.replace('const CVector &p0 = line.p0;','const CollisionVector3 &p0 = line.p0;').replace('(line.p1 - p0).z','(line.p1 - CollisionVector_ToNative(p0)).z')
    source=re.sub(r'\*\(CVuVector\*\)&line\.p([01])',r'ColLine_ToVu(line,\1)',source)
    source=re.sub(r'\*\(CColLine\*\)(newline|lineA)',r'ColLine_FromRaw32(\1)',source)
    code=mask(source);constructors=list(re.finditer(r'\bCColLine\s+(shootLine|line|newline|groundLine)\s*\(',code))
    for index,match in reversed(list(enumerate(constructors))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);assert len(args)==2,args
        start=source.rfind('\n',0,match.start())+1;indent=source[start:match.start()];assert not indent.strip(),source[start:match.start()]
        lines=['// Retain the line record at its original construction boundary','CColLine '+match[1]+';'];references=[]
        for n,arg in enumerate(args):
            if lvalue(arg):references.append('COLLISION_POINT_REF('+arg+')')
            else:
                name='lineConstructorPoint'+str(index)+'_'+str(n);lines+=['// Capture the complete native expression before initializing line endpoints','const CVector &'+name+'='+arg+';'];references.append('COLLISION_POINT_REF('+name+')')
        lines+=['// Initialize the line at the former parameterized constructor boundary','ColLine_Init(&'+match[1]+','+','.join(references)+')']
        source=source[:match.start()]+('\n'+indent).join(lines)+source[end:]
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bCColLine\s*\(',code))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);assert len(args)==2 and all(lvalue(arg) for arg in args),args
        source=source[:match.start()]+'ColLine_Make('+','.join('COLLISION_POINT_REF('+arg+')' for arg in args)+')'+source[end:]
    old='aLinesA[i].Set(matAB * modelA.lines[i].p0, matAB * modelA.lines[i].p1);'
    new='''{
        // Capture both transformed values before the ordered C line update
        const CVector transformedLineStart=matAB * modelA.lines[i].p0;
        const CVector transformedLineEnd=matAB * modelA.lines[i].p1;
        ColLine_Set(&aLinesA[i],COLLISION_POINT_REF(transformedLineStart),COLLISION_POINT_REF(transformedLineEnd));
    }'''
    source=source.replace(old,new)
    # Convert whole-coordinate assignments before adapting their native read expressions
    assignments=list(re.finditer(r'(?m)^([\t ]*)('+POINT+r') = ([^;\n]+);',source))
    for match in reversed(assignments):
        indent,target,value=match[1],match[2],match[4]
        if value.startswith('CollisionVector_FromNative('):continue
        source=source[:match.start()]+indent+target+' = CollisionVector_FromNative('+value+');'+source[match.end():]
    code=mask(source);protected=[]
    for match in re.finditer(r'\b(?:COLLISION_POINT_REF|CollisionVector_ToNative)\(',code):protected.append((match.start(),closing(code,match.end()-1)))
    for match in reversed(list(re.finditer(POINT,code))):
        if any(a<=match.start()<b for a,b in protected):continue
        tail=source[match.end():]
        if re.match(r'\s*\.[xyz]\b',tail):continue
        before=source[source.rfind('\n',0,match.start())+1:match.start()]
        if before.strip()=='const CollisionVector3 &p0 =':continue
        if re.match(r'\s*=',tail):continue
        source=source[:match.start()]+'CollisionVector_ToNative('+match[1]+')'+source[match.end():]
    if source!=original and any(token in source for token in ['CollisionVector_ToNative(','CollisionVector_FromNative(','ColLine_ToVu(']) and '#include "common.h"' in source and '#include "ColLine_native.h"' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "ColLine_native.h"',1)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def col_line_project(source):
    # Rename the complete C producer and register the shared C/native integration headers
    source=source.replace('..\\src\\collision\\ColLine.cpp','..\\src\\collision\\ColLine.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\collision\\ColLine\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing metadata while requiring C11 compilation
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\collision\\ColLine.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    for name in ['CollisionVector.h','CollisionVector_native.h','ColLine_native.h']:
        entry='<ClInclude Include="..\\src\\collision\\ColLine.h" />'
        if '..\\src\\collision\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\collision\\'+name+'" />')
    return objects.project(source)



def calls(source):
    # Preserve complete line ownership before removing both native box owners and their inheritance
    import col_box_migration as boxes
    return boxes.calls(col_line_calls(source))


def project(source):
    # Retain the complete line producer before the box producer rename and C11 metadata
    import col_box_migration as boxes
    return boxes.project(col_line_project(source))
