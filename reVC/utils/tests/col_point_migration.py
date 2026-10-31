"""Move the entire contact record and selective assignment into C while retaining native mutation order."""
import re
from rect_migration import mask,closing,arguments
from col_box_migration import value_start
from col_sphere_migration import symbols as sphere_symbols
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/collision/ColPoint.cpp':'src/collision/ColPoint.c'}
OPERATIONS=['GetNormal','GetDepth','Set','SetSurfaces','Assign','CopyValue','NormaliseNormal','ReverseNormal']

def header(original):
    # Retain the complete forty-byte record with explicit gap words and C coordinates
    assert 'CColPoint &operator=' in original
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct CColPoint {
    CollisionVector3 point;
    int32_t pad1;
    CollisionVector3 normal;
    int32_t pad2;
    uint8_t surfaceA,pieceA,surfaceB,pieceB;
    float depth;
} CColPoint;
#ifdef __cplusplus
extern "C" {
#endif
const CollisionVector3 *ColPoint_GetNormal(const CColPoint *self);
float ColPoint_GetDepth(const CColPoint *self);
void ColPoint_Set(CColPoint *self,float depth,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB);
void ColPoint_SetSurfaces(CColPoint *self,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB);
CColPoint *ColPoint_Assign(CColPoint *self,const CColPoint *other);
CColPoint ColPoint_CopyValue(CColPoint value);
void ColPoint_NormaliseNormal(CColPoint *self);
void ColPoint_ReverseNormal(CColPoint *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Keep the original selective copy and metadata writes while moving normal mutations into C
    assert 'CColPoint::operator=' in original
    return '''//+ rouz edit (ChatGPT)
#include "ColPoint.h"
#include <string.h>
#include <math.h>
static void ColPoint_CopyCoordinates(CollisionVector3 *self,const CollisionVector3 *other)
{
    // Capture one complete vector before assigning its destination
    CollisionVector3 value;memcpy(&value,other,sizeof(value));memcpy(self,&value,sizeof(value));
}
const CollisionVector3 *ColPoint_GetNormal(const CColPoint *self)
{
    // Borrow the actual normal so later contact updates remain visible through the query result
    return &self->normal;
}
float ColPoint_GetDepth(const CColPoint *self)
{
    // Return the original stored depth without interpretation
    return self->depth;
}
void ColPoint_SetSurfaces(CColPoint *self,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB)
{
    // Retain the original surface and piece write order without changing depth or geometry
    self->surfaceA=surfaceA;self->pieceA=pieceA;self->surfaceB=surfaceB;self->pieceB=pieceB;
}
void ColPoint_Set(CColPoint *self,float depth,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB)
{
    // Write depth before the original four metadata fields
    self->depth=depth;ColPoint_SetSurfaces(self,surfaceA,pieceA,surfaceB,pieceB);
}
CColPoint *ColPoint_Assign(CColPoint *self,const CColPoint *other)
{
    // Copy point, normal and metadata while leaving both gap words and depth untouched
    ColPoint_CopyCoordinates(&self->point,&other->point);ColPoint_CopyCoordinates(&self->normal,&other->normal);
    self->surfaceA=other->surfaceA;self->pieceA=other->pieceA;self->surfaceB=other->surfaceB;self->pieceB=other->pieceB;
    return self;
}
CColPoint ColPoint_CopyValue(CColPoint value)
{
    // Preserve full copy construction including depth and both explicit gap words
    return value;
}
void ColPoint_NormaliseNormal(CColPoint *self)
{
    // Retain the original native squared-length and reciprocal-square-root operation grouping
    const float sq=self->normal.x*self->normal.x+self->normal.y*self->normal.y+self->normal.z*self->normal.z;
    if(sq>0.0f){
        // Scale all three components in their original order for a positive squared length
        const float inverse=1.0f/sqrtf(sq);self->normal.x*=inverse;self->normal.y*=inverse;self->normal.z*=inverse;
    }else{
        // Preserve the original fallback that changes only X even for a nonfinite squared length
        self->normal.x=1.0f;
    }
}
void ColPoint_ReverseNormal(CColPoint *self)
{
    // Retain the original ordered multiplication by negative one before suspension forces are applied
    self->normal.x*=-1.0f;self->normal.y*=-1.0f;self->normal.z*=-1.0f;
}
//- rouz edit (ChatGPT)
'''

def symbols(source):
    # Reuse lexical lifetime tracking with equal-length type spelling and retain pointer/array distinctions
    if 'CColPoint' not in source:return []
    rows=[]
    for name,record,start,end in sphere_symbols(source.replace('CColPoint','CSphere  ')):
        found=re.match(r'CColPoint\b\s*(?:const\s*)?([*&]?)\s*(\w+)',source[start:])
        if not found:continue
        tail=source[start+found.end():];rows.append((name,found[1]=='*',tail.lstrip().startswith('['),start,end))
        extra=re.match(r'\s*,\s*([*&]?)\s*(\w+)\s*([,;])',tail)
        if extra:rows.append((extra[2],extra[1]=='*',False,start,end))
    return rows

def record(receiver,pos,rows,pointers=False):
    # Recognize actual contact records and the two declared native wheel/world contact arrays
    receiver=receiver.strip()
    if receiver.startswith('(') and receiver.endswith(')'):receiver=receiver[1:-1].strip()
    if re.search(r'(?:^|\.|->|::)(?:m_aWheelColPoints|m_aTempColPts|gaTempSphereColPoints)\[[\s\S]*\]$',receiver):return True
    found=re.fullmatch(r'(\*)?(\w+)(\[[\s\S]*\])?',receiver)
    if not found:return False
    candidates=[row for row in rows if row[0]==found[2] and row[3]<=pos<row[4]]
    if not candidates:return False
    value=max(candidates,key=lambda row:row[3])
    if found[1] or found[3]:return value[1] or value[2]
    return pointers or (not value[1] and not value[2])

def fields(source):
    # Identify contact coordinates without confusing other native normal or point records
    code=mask(source);rows=symbols(source);result=[]
    for match in re.finditer(r'(\.|->)(point|normal)\b',code):
        start=value_start(code,match.start());receiver=source[start:match.start()]
        if record(receiver,match.start(),rows,match[1]=='->'):result.append((start,match.end(),receiver,match[1],match[2]))
    return result

def address(receiver,separator):
    # Pass the actual record pointer at its original evaluation point
    return receiver if separator=='->' else '&'+receiver

def col_point_calls(source):
    # Replace the complete owner before adapting native expressions and selective assignment sites
    if 'CColPoint &operator=' in source:return header(source)
    if 'CColPoint::operator=' in source:return kernel(source)
    if 'static void ColPoint_CopyCoordinates(' in source:return source
    if 'typedef struct CColPoint {' in source:return source
    if not any(token in source for token in ['CColPoint','m_aWheelColPoints','m_aTempColPts','gaTempSphereColPoints']):return source
    original=source
    source=source.replace('CVector &forceDir);','CColPoint &contact);') if 'ApplySpringCollisionAlt(' in source else source
    if 'CPhysical::ApplySpringCollisionAlt(' in source:
        body=method(source,'CPhysical::ApplySpringCollisionAlt')
        updated=body.replace('CVector &forceDir)','CColPoint &contact)').replace('forceDir *= -1.0f;','ColPoint_ReverseNormal(&contact);').replace('forceDir','contact.normal')
        if '// Update the actual contact normal before either force application' not in updated:updated=updated.replace('{\n','{\n    // Update the actual contact normal before either force application\n',1)
        source=source.replace(body,updated)
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bApplySpringCollisionAlt\(',code))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1])
        if len(args)==6 and args[-1].endswith('.normal'):args[-1]=args[-1][:-7];source=source[:match.end()]+', '.join(args)+source[end-1:]
    # Retain a genuine native end vector for the legacy bullet backend's CVector read
    old='BulletTraces_AddWeaponLegacy(fireSource, &point.point, m_eWeaponType, shooter);'
    if old in source:source=source.replace(old,'''{
                // Keep the native endpoint alive through the legacy vector reader
                const CVector traceEnd=CollisionVector_ToNative(point.point);
                BulletTraces_AddWeaponLegacy(fireSource,&traceEnd,m_eWeaponType,shooter);
            }''')
    code=mask(source);rows=symbols(source)
    for match in reversed(list(re.finditer(r'\bCColPoint\s+(origPoint)\s*\(',code))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);assert len(args)==1,args
        source=source[:match.start()]+'CColPoint '+match[1]+'=ColPoint_CopyValue('+args[0]+')'+source[end:]
    code=mask(source);rows=symbols(source)
    for match in reversed(list(re.finditer(r'(\.|->)(GetNormal|GetDepth|Set)\(',code))):
        start=value_start(code,match.start());receiver=source[start:match.start()]
        if not record(receiver,match.start(),rows,match[1]=='->'):continue
        end=closing(code,match.end()-1);self=address(receiver,match[1]);args=arguments(source[match.end():end-1])
        if match[2]=='GetNormal':replacement='CollisionVector_ToNative(*ColPoint_GetNormal('+self+'))'
        elif match[2]=='GetDepth':replacement='ColPoint_GetDepth('+self+')'
        else:
            assert len(args) in [4,5],args;replacement='ColPoint_'+('Set' if len(args)==5 else 'SetSurfaces')+'('+self+','+source[match.end():end-1]+')'
        source=source[:start]+replacement+source[end:]
    code=mask(source)
    for start,end,receiver,separator,field in reversed(fields(source)):
        if field=='normal' and source[end:].startswith('.Normalise()'):source=source[:start]+'ColPoint_NormaliseNormal('+address(receiver,separator)+')'+source[end+12:]
    # Redirect actual record assignment while leaving full copy initialization and pointer assignment intact
    code=mask(source);rows=symbols(source)
    for match in reversed(list(re.finditer(r'(?<![=!<>+*/-])=(?!=)',code))):
        start=value_start(code,match.start());left=source[start:match.start()].strip()
        if start and code[start-1]=='*':start-=1;left='*'+left
        if not record(left,match.start(),rows):continue
        before=source[source.rfind('\n',0,start)+1:start]
        if re.search(r'\bCColPoint\s*$',before):continue
        end=source.find(';',match.end());right=source[match.end():end].strip()
        if end<0 or not record(right,match.start(),rows):continue
        source=source[:start]+'ColPoint_Assign(&'+left+',&'+right+')'+source[end:]
    # Convert complete native vector writes before wrapping reads in the original expressions
    code=mask(source)
    for start,end,receiver,separator,field in reversed(fields(source)):
        suffix=source[end:];assignment=re.match(r'\s*=\s*(?!=)',suffix)
        if not assignment:continue
        valueStart=end+assignment.end();valueEnd=source.find(';',valueStart);value=source[valueStart:valueEnd]
        if value.startswith('CollisionVector_FromNative('):continue
        source=source[:valueStart]+'CollisionVector_FromNative('+value+')'+source[valueEnd:]
    code=mask(source);protected=[]
    for match in re.finditer(r'\b(?:COLLISION_POINT_REF|COLLISION_POINT_OUT|CollisionVector_ToNative)\(',code):protected.append((match.start(),closing(code,match.end()-1)))
    for start,end,receiver,separator,field in reversed(fields(source)):
        if any(a<=start<b for a,b in protected):continue
        if re.match(r'\s*\.[xyz]\b|\s*=',source[end:]):continue
        source=source[:start]+'CollisionVector_ToNative('+source[start:end]+')'+source[end:]
    if source==original:return source
    if '#include "common.h"' in source and '#include "CollisionVector_native.h"' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "CollisionVector_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def col_point_project(source):
    # Keep the complete producer in C11 while retaining all native object-name distinctions
    source=source.replace('..\\src\\collision\\ColPoint.cpp','..\\src\\collision\\ColPoint.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\collision\\ColPoint\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated metadata while requiring C compilation for the renamed producer
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\collision\\ColPoint.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)



def calls(source):
    # Preserve complete contact ownership before the cached polygon record and validation policy
    import stored_poly_migration as stored
    return stored.calls(col_point_calls(source))


def project(source):
    # Retain contact C11 integration before registering the complete stored-polygon producer
    import stored_poly_migration as stored
    return stored.project(col_point_project(source))
