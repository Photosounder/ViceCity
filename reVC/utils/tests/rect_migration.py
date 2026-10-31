"""Move the entire rectangle record, constructors and overloaded geometry operations into C11."""
import re
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/math/Rect.cpp':'src/math/Rect.c'}
OPERATIONS=['Init','Make','CopyValue','ContainPoint','ContainRect','IsPointInside','IsPointInsideRadius','Translate','Grow','GrowSides','GrowEdges','GetWidth','GetHeight']
WATER_INIT='''
static int InitialiseWaterRectStorage(void)
{
    // Initialize water rectangles at their former array-constructor boundary
    for(unsigned i=0;i<48;++i) Rect_Init(&CWaterLevel::ms_aWaterRects[i]);
    return 0;
}
static const int waterRectStorageInitialised = InitialiseWaterRectStorage();'''

def mask(source):
    # Keep source offsets while excluding comments and literals from syntax conversion
    return re.sub(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'',lambda m:re.sub(r'[^\n]',' ',m[0]),source)

def closing(source,start):
    # Find a complete call including nested macros and expressions
    depth=1;end=start+1
    while depth:
        if source[end]=='(':depth+=1
        elif source[end]==')':depth-=1
        end+=1
    return end

def arguments(source):
    # Split only commas at the current argument level
    result=[];start=0;depth=0
    for n,ch in enumerate(source):
        if ch in '([{':depth+=1
        elif ch in ')]}':depth-=1
        elif ch==',' and depth==0:result.append(source[start:n].strip());start=n+1
    result.append(source[start:].strip());return result

def header(original):
    # Retain the original field order and expose every operation without C++ vector dependencies
    assert 'class CRect\n' in original
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
typedef struct CRect {
    float left;
    float bottom;
    float right;
    float top;
} CRect;
#ifdef __cplusplus
extern "C" {
#endif
void Rect_Init(CRect *self);
CRect Rect_Make(float left,float top,float right,float bottom);
CRect Rect_CopyValue(CRect value);
void Rect_ContainPoint(CRect *self,const float *x,const float *y);
void Rect_ContainRect(CRect *self,const CRect *other);
bool Rect_IsPointInside(const CRect *self,const float *x,const float *y);
bool Rect_IsPointInsideRadius(const CRect *self,const float *x,const float *y,float extraRadius);
void Rect_Translate(CRect *self,float x,float y);
void Rect_Grow(CRect *self,float radius);
void Rect_GrowSides(CRect *self,float left,float right);
void Rect_GrowEdges(CRect *self,float left,float right,float top,float bottom);
float Rect_GetWidth(const CRect *self);
float Rect_GetHeight(const CRect *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Keep ordered field reads and short-circuit coordinate loads through explicit C pointers
    return '''//+ rouz edit (ChatGPT)
#include "Rect.h"
void Rect_Init(CRect *self)
{
    // Preserve the empty-bound constructor and its field-write order
    self->left=1000000.0f;
    self->top=1000000.0f;
    self->right=-1000000.0f;
    self->bottom=-1000000.0f;
}
CRect Rect_Make(float left,float top,float right,float bottom)
{
    // Construct the original left, top, right, bottom argument order
    CRect value;
    value.left=left;value.top=top;value.right=right;value.bottom=bottom;
    return value;
}
CRect Rect_CopyValue(CRect value)
{
    // Preserve explicit copy-construction sites as plain C value copies
    return value;
}
void Rect_ContainPoint(CRect *self,const float *x,const float *y)
{
    // Preserve each comparison and its live coordinate read before the next field update
    if(*x<self->left) self->left=*x;
    if(*x>self->right) self->right=*x;
    if(*y<self->top) self->top=*y;
    if(*y>self->bottom) self->bottom=*y;
}
void Rect_ContainRect(CRect *self,const CRect *other)
{
    // Retain ordered source reads even when the source and destination are the same record
    if(other->left<self->left) self->left=other->left;
    if(other->right>self->right) self->right=other->right;
    if(other->top<self->top) self->top=other->top;
    if(other->bottom>self->bottom) self->bottom=other->bottom;
}
bool Rect_IsPointInside(const CRect *self,const float *x,const float *y)
{
    // Preserve inclusive edges and short-circuit coordinate reads
    return *x>=self->left&&*x<=self->right&&*y>=self->top&&*y<=self->bottom;
}
bool Rect_IsPointInsideRadius(const CRect *self,const float *x,const float *y,float extraRadius)
{
    // Preserve the original radius arithmetic at each inclusive edge comparison
    return *x>=self->left-extraRadius&&*x<=self->right+extraRadius&&*y>=self->top-extraRadius&&*y<=self->bottom+extraRadius;
}
void Rect_Translate(CRect *self,float x,float y)
{
    // Keep the original horizontal then vertical update order
    self->left+=x;self->right+=x;self->bottom+=y;self->top+=y;
}
void Rect_Grow(CRect *self,float radius)
{
    // Apply one radius in the original field order
    self->left-=radius;self->right+=radius;self->top-=radius;self->bottom+=radius;
}
void Rect_GrowSides(CRect *self,float left,float right)
{
    // Apply the original left/top and right/bottom radius pairing
    self->left-=left;self->top-=left;self->right+=right;self->bottom+=right;
}
void Rect_GrowEdges(CRect *self,float left,float right,float top,float bottom)
{
    // Keep all four independently supplied edge radii
    self->left-=left;self->top-=top;self->right+=right;self->bottom+=bottom;
}
float Rect_GetWidth(const CRect *self)
{
    // Return the original signed horizontal extent
    return self->right-self->left;
}
float Rect_GetHeight(const CRect *self)
{
    // Return the original signed vertical extent
    return self->bottom-self->top;
}
//- rouz edit (ChatGPT)
'''

def rect_calls(source):
    # Replace the complete owner before adapting constructors, member calls and embedded lifetimes
    if 'class CRect\n' in source:return header(source)
    if 'CRect::CRect(void)' in source:return kernel(source)
    original=source;code=mask(source)
    matches=list(re.finditer(r'\bCRect\s+(rect|mouse|shad|unusedRect)\s*\(',code))
    for match in reversed(matches):
        end=closing(code,match.end()-1);args=source[match.end():end-1]
        source=source[:match.start()]+'CRect '+match[1]+' = Rect_Make('+args+')'+source[end:]
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bCRect\s*\(',code))):
        end=closing(code,match.end()-1);args=source[match.end():end-1];count=len(arguments(args))
        assert count in [1,4],args
        source=source[:match.start()]+('Rect_CopyValue' if count==1 else 'Rect_Make')+'('+args+')'+source[end:]
    # Initialize only actual local default declarations, leaving native record members explicit below
    field='struct tZonePrint\n{'
    scope=(source.index(field),source.index('};',source.index(field))) if field in source else None
    matches=list(re.finditer(r'(?m)^([\t ]*)CRect (rect|Bound);',mask(source)))
    for match in reversed(matches):
        if scope and scope[0]<match.start()<scope[1]:continue
        if not match[1]:continue
        if source[match.end():].lstrip().startswith('// Initialize rectangle bounds at the former local constructor boundary'):continue
        replacement=source[match.start():match.end()]+'\n'+match[1]+'// Initialize rectangle bounds at the former local constructor boundary\n'+match[1]+'Rect_Init(&'+match[2]+');'
        source=source[:match.start()]+replacement+source[match.end():]
    if '\tintro_script_rectangle() { }' in source:source=source.replace('\tintro_script_rectangle() { }','\tintro_script_rectangle() {\n        // Initialize the rectangle before the former empty color-constructor boundary\n        Rect_Init(&m_sRect);\n    }')
    water='CRect CWaterLevel::ms_aWaterRects[48];'
    if water in source and WATER_INIT not in source:source=source.replace(water,water+WATER_INIT)
    points=list(re.finditer(r'(?m)^([\t ]*)rect\.ContainPoint\(([^\n]+)\);',source))
    for n,match in reversed(list(enumerate(points))):
        indent=match[1];name='rectanglePoint'+str(n)
        source=source[:match.start()]+indent+'// Pass the native point once while retaining live coordinate reads\n'+indent+'const CVector &'+name+' = '+match[2]+';\n'+indent+'Rect_ContainPoint(&rect, &'+name+'.x, &'+name+'.y);'+source[match.end():]
    source=re.sub(r'\b(rect|mouse|shad)\.Translate\(',r'Rect_Translate(&\1, ',source)
    for match in reversed(list(re.finditer(r'\brect\.Grow\(',source))):
        end=closing(mask(source),match.end()-1);args=source[match.end():end-1];name={1:'Grow',2:'GrowSides',4:'GrowEdges'}[len(arguments(args))]
        source=source[:match.start()]+'Rect_'+name+'(&rect, '+args+')'+source[end:]
    source=re.sub(r'(CColStore::GetBoundingBox\(i\))\.Grow\(',r'Rect_Grow(&\1, ',source)
    old='if(col->level != 0)\n\t\t\t\tCColStore::GetBoundingBox(col->level).ContainRect(entity->GetBoundRect());'
    new='''if(col->level != 0) {
                // Keep the returned entity rectangle alive through the C containment call
                const CRect entityBounds = entity->GetBoundRect();
                Rect_ContainRect(&CColStore::GetBoundingBox(col->level), &entityBounds);
            }'''
    source=source.replace(old,new)
    entity='if (GetBoundingBox(i).IsPointInside(pEntity->GetPosition(), -80.0f))'
    if entity in source:source=source.replace(entity,'// Retain the original conversion before the collision point test\n                    const CVector2D collisionPoint(pEntity->GetPosition());\n                    if (Rect_IsPointInsideRadius(&GetBoundingBox(i), &collisionPoint.x, &collisionPoint.y, -80.0f))')
    source=re.sub(r'GetBoundingBox\(i\)\.IsPointInside\((pos|secondPosition), ([^)]+)\)',r'Rect_IsPointInsideRadius(&GetBoundingBox(i), &\1.x, &\1.y, \2)',source)
    source=re.sub(r'GetBoundingBox\(i\)\.IsPointInside\((pos|secondPosition)\)',r'Rect_IsPointInside(&GetBoundingBox(i), &\1.x, &\1.y)',source)
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def rect_project(source):
    # Rename the complete native producer and explicitly retain C11 compilation
    source=source.replace('..\\src\\math\\Rect.cpp','..\\src\\math\\Rect.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\math\\Rect\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated options while selecting the C language
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\math\\Rect.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Preserve complete rectangle ownership before moving collision vertex packing and all consumers into C
    import compressed_vector_migration as vertex
    return vertex.calls(rect_calls(source))

def project(source):
    # Retain the rectangle rename before registering the complete C11 collision vertex producer
    import compressed_vector_migration as vertex
    return vertex.project(rect_project(source))
