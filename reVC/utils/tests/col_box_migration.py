"""Move both complete box records to C and replace inheritance with explicit composition."""
import re
from rect_migration import mask,closing,arguments
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/collision/ColBox.cpp':'src/collision/ColBox.c'}
OPERATIONS=['Box_Set','Box_GetSize','ColBox_SetBounds','ColBox_Set','ColBox_Assign']

def header(original):
    # Keep the original prefix and metadata layout using only C coordinate and record types
    assert 'struct CColBox : public CBox' in original
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct CBox {
    CollisionVector3 min,max;
} CBox;
typedef struct CColBox {
    CBox bounds;
    uint8_t surface,piece;
} CColBox;
#ifdef __cplusplus
extern "C" {
#endif
void Box_Set(CBox *self,CollisionPointRef min,CollisionPointRef max);
CollisionVector3 Box_GetSize(const CBox *self);
void ColBox_SetBounds(CColBox *self,CollisionPointRef min,CollisionPointRef max);
void ColBox_Set(CColBox *self,CollisionPointRef min,CollisionPointRef max,uint8_t surface,uint8_t piece);
CColBox *ColBox_Assign(CColBox *self,const CColBox *other);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Preserve ordered coordinate assignments, live source references and the untouched tail bytes
    assert 'CColBox::operator=' in original
    return '''//+ rouz edit (ChatGPT)
#include "ColBox.h"
#include <string.h>
void Box_Set(CBox *self,CollisionPointRef min,CollisionPointRef max)
{
    // Capture and assign the complete minimum before reading the maximum
    CollisionVector3 value;
    memcpy(&value.x,min.x,sizeof(float));memcpy(&value.y,min.y,sizeof(float));memcpy(&value.z,min.z,sizeof(float));
    memcpy(&self->min,&value,sizeof(value));
    // Retain the second reference's live values after the first destination write
    memcpy(&value.x,max.x,sizeof(float));memcpy(&value.y,max.y,sizeof(float));memcpy(&value.z,max.z,sizeof(float));
    memcpy(&self->max,&value,sizeof(value));
}
CollisionVector3 Box_GetSize(const CBox *self)
{
    // Return the same signed component differences without normalizing the bounds
    const CollisionVector3 value={self->max.x-self->min.x,self->max.y-self->min.y,self->max.z-self->min.z};
    return value;
}
void ColBox_SetBounds(CColBox *self,CollisionPointRef min,CollisionPointRef max)
{
    // Preserve the inherited two-argument operation without changing collision metadata
    Box_Set(&self->bounds,min,max);
}
void ColBox_Set(CColBox *self,CollisionPointRef min,CollisionPointRef max,uint8_t surface,uint8_t piece)
{
    // Keep the original minimum, maximum, surface and piece write order
    Box_Set(&self->bounds,min,max);self->surface=surface;self->piece=piece;
}
CColBox *ColBox_Assign(CColBox *self,const CColBox *other)
{
    // Copy only the original fields and retain both destination tail-padding bytes
    Box_Set(&self->bounds,COLLISION_POINT_REF(other->bounds.min),COLLISION_POINT_REF(other->bounds.max));
    self->surface=other->surface;self->piece=other->piece;return self;
}
//- rouz edit (ChatGPT)
'''

def value_start(code,end):
    # Walk one complete postfix receiver backwards through calls, indexes and member chains
    end=len(code[:end].rstrip());i=end
    if code[i-1] in ')]':
        closingChar=code[i-1];opening='(' if closingChar==')' else '[';depth=1;i-=1
        while depth:
            i-=1
            if code[i]==closingChar:depth+=1
            elif code[i]==opening:depth-=1
        before=len(code[:i].rstrip())
        if closingChar==']' or (before and (code[before-1].isalnum() or code[before-1] in '_)]')):i=value_start(code,before)
    else:
        while i and (code[i-1].isalnum() or code[i-1] in '_:'):i-=1
    before=len(code[:i].rstrip())
    if before and code[before-1]=='.':return value_start(code,before-1)
    if before>=2 and code[before-2:before]=='->':return value_start(code,before-2)
    return i

def coordinate_ranges(source):
    # Identify only actual box-coordinate receivers while retaining full native postfix expressions
    code=mask(source);result=[]
    for match in re.finditer(r'(\.|->)(min|max)\b',code):
        start=value_start(code,match.start());receiver=source[start:match.start()]
        if receiver.endswith('boundingBox') or receiver in ['box','box.bounds'] or re.search(r'\bboxes\[[\s\S]*\]\.bounds$',receiver):result.append((start,match.end()))
    return result

def col_box_calls(source):
    # Replace both complete owners before adapting explicit bases, setters and native vector boundaries
    if 'struct CColBox : public CBox' in source:return header(source)
    if 'CColBox::operator=' in source:return kernel(source)
    original=source
    if 'CCollision::ProcessSphereBox(' in source:
        for name in ['CCollision::ProcessSphereBox','CCollision::ProcessLineBox']:
            body=method(source,name);source=source.replace(body,re.sub(r'\bbox\.(min|max)\b',r'box.bounds.\1',body))
    source=re.sub(r'(\b(?:\w+(?:\.|->))?boxes\[[^\n;]+?\])\.(min|max)\b',r'\1.bounds.\2',source)
    source=source.replace('boxes[i] = other.boxes[i];','ColBox_Assign(&boxes[i],&other.boxes[i]);')
    source=source.replace('const CVector &v = veh->GetColModel()->boundingBox.max;','const CollisionVector3 &v = veh->GetColModel()->boundingBox.max;')
    source=source.replace('CVector *colMin = &vehCol->boundingBox.min;','CollisionVector3 *colMin = &vehCol->boundingBox.min;').replace('CVector *colMax = &vehCol->boundingBox.max;','CollisionVector3 *colMax = &vehCol->boundingBox.max;')
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bTest(?:Sphere|Line|VerticalLine)Box\(',code))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1])
        if len(args)==2 and (re.search(r'\bboxes\[',args[1]) or (args[1]=='box' and 'bool CBulletInfo::TestForSniperBullet(' in source)) and not args[1].endswith('.bounds'):
            args[1]+='.bounds';source=source[:match.end()]+', '.join(args)+source[end-1:]
    code=mask(source)
    for match in reversed(list(re.finditer(r'\.GetSize\(\)',code))):
        start=value_start(code,match.start());receiver=source[start:match.start()]
        if not receiver.endswith('boundingBox'):continue
        source=source[:start]+'CollisionVector_ToNative(Box_GetSize(&'+receiver+'))'+source[match.end():]
    code=mask(source);matches=[]
    for match in re.finditer(r'\.Set\(',code):
        start=value_start(code,match.start());receiver=source[start:match.start()]
        if receiver.endswith('boundingBox') or receiver=='box' or re.search(r'\bboxes\[',receiver):matches.append((start,match,receiver))
    for n,(start,match,receiver) in reversed(list(enumerate(matches))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);assert len(args) in [2,4],args
        indent=source[source.rfind('\n',0,start)+1:start];assert not indent.strip(),receiver
        lo='boxMinimum'+str(n);hi='boxMaximum'+str(n);operation='ColBox_Set' if len(args)==4 else 'Box_Set'
        replacement='''{
@INDENT@    // Keep native bound expressions alive through the ordered C update
@INDENT@    const CVector &@LO@=@A@;
@INDENT@    const CVector &@HI@=@B@;
@INDENT@    @OP@(&@RECEIVER@,COLLISION_POINT_REF(@LO@),COLLISION_POINT_REF(@HI@)@TAIL@);
@INDENT@}'''
        for key,value in [('INDENT',indent),('LO',lo),('HI',hi),('A',args[0]),('B',args[1]),('OP',operation),('RECEIVER',receiver),('TAIL',','+','.join(args[2:]) if len(args)>2 else '')]:replacement=replacement.replace('@'+key+'@',value)
        source=source[:start]+replacement+source[end+1:]
    # Convert complete vector writes before wrapping native whole-coordinate reads
    for start,end in reversed(coordinate_ranges(source)):
        tail=source[end:];assignment=re.match(r'\s*=\s*([^;\n]+);',tail)
        if not assignment or assignment[1].startswith('CollisionVector_FromNative('):continue
        source=source[:end]+tail[:assignment.start(1)]+'CollisionVector_FromNative('+assignment[1]+')'+tail[assignment.end(1):]
    code=mask(source);protected=[]
    for match in re.finditer(r'\b(?:COLLISION_POINT_REF|CollisionVector_ToNative)\(',code):protected.append((match.start(),closing(code,match.end()-1)))
    for start,end in reversed(coordinate_ranges(source)):
        if any(a<=start<b for a,b in protected):continue
        tail=source[end:];prefix=source[source.rfind('\n',0,start)+1:start]
        if re.match(r'\s*\.[xyz]\b|\s*=',tail):continue
        if re.search(r'(?:const CollisionVector3 &v\s*=|CollisionVector3 \*col(?:Min|Max)\s*=\s*&)\s*$',prefix):continue
        source=source[:start]+'CollisionVector_ToNative('+source[start:end]+')'+source[end:]
    if source==original:return source
    if any(token in source for token in ['CollisionVector_ToNative(','CollisionVector_FromNative(']) and '#include "CollisionVector_native.h"' not in source and '#include "common.h"' in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "CollisionVector_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def col_box_project(source):
    # Keep the C11 producer rename and all existing paired object names
    source=source.replace('..\\src\\collision\\ColBox.cpp','..\\src\\collision\\ColBox.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\collision\\ColBox\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated metadata while selecting C11 for the renamed producer
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\collision\\ColBox.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)



def calls(source):
    # Preserve complete box ownership before removing both sphere owners and their native vector members
    import col_sphere_migration as spheres
    return spheres.calls(col_box_calls(source))


def project(source):
    # Retain box C11 integration before the sphere producer rename and native boundary header
    import col_sphere_migration as spheres
    return spheres.project(col_box_project(source))
