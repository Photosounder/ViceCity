"""Move both complete sphere records and ray intersection into C with explicit native boundaries."""
import re
from rect_migration import mask,closing,arguments
from col_box_migration import value_start
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/collision/ColSphere.cpp':'src/collision/ColSphere.c'}
OPERATIONS=['Sphere_Set','ColSphere_SetBounds','ColSphere_Set','ColSphere_IntersectRay','Sphere_FromRaw16','ColSphere_FromRaw16']

def header(original):
    # Retain the quadword prefix and metadata layout without native classes or inheritance
    assert 'struct CColSphere : public CSphere' in original
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "CollisionVector.h"
#include "SurfaceTable.h"
typedef struct CSphere {
    CollisionVector3 center;
    float radius;
} CSphere;
typedef struct CColSphere {
    CSphere bounds;
    uint8_t surface,piece;
} CColSphere;
typedef struct CollisionPointOut { float *x,*y,*z; } CollisionPointOut;
static inline CollisionPointOut CollisionPoint_Output(float *x,float *y,float *z)
{
    // Retain the actual writable float subobjects without native vector type assumptions
    const CollisionPointOut value={x,y,z};return value;
}
#define COLLISION_POINT_OUT(value) CollisionPoint_Output(&(value).x,&(value).y,&(value).z)
#ifdef __cplusplus
extern "C" {
#endif
void Sphere_Set(CSphere *self,float radius,CollisionPointRef center);
void ColSphere_SetBounds(CColSphere *self,float radius,CollisionPointRef center);
void ColSphere_Set(CColSphere *self,float radius,CollisionPointRef center,uint8_t surface,uint8_t piece);
bool ColSphere_IntersectRay(CColSphere *self,CollisionPointRef from,CollisionPointRef dir,CollisionPointOut entry,CollisionPointOut exit);
CSphere Sphere_FromRaw16(const void *bytes);
CColSphere ColSphere_FromRaw16(const void *bytes);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Preserve setter order, arithmetic grouping and live ray references across the first output write
    assert 'CColSphere::IntersectRay(' in original
    return '''//+ rouz edit (ChatGPT)
#include "ColSphere.h"
#include "General.h"
#include <string.h>
static void Sphere_WriteCenter(CollisionVector3 *self,CollisionPointRef center)
{
    // Capture the complete source before assigning its destination coordinates
    CollisionVector3 value;
    memcpy(&value.x,center.x,sizeof(float));memcpy(&value.y,center.y,sizeof(float));memcpy(&value.z,center.z,sizeof(float));
    memcpy(self,&value,sizeof(value));
}
void Sphere_Set(CSphere *self,float radius,CollisionPointRef center)
{
    // Keep the base setter's original center-before-radius ordering
    Sphere_WriteCenter(&self->center,center);self->radius=radius;
}
void ColSphere_SetBounds(CColSphere *self,float radius,CollisionPointRef center)
{
    // Preserve the inherited base setter without touching collision metadata or padding
    Sphere_Set(&self->bounds,radius,center);
}
void ColSphere_Set(CColSphere *self,float radius,CollisionPointRef center,uint8_t surface,uint8_t piece)
{
    // Keep the collision setter's original radius-before-center and metadata ordering
    self->bounds.radius=radius;Sphere_WriteCenter(&self->bounds.center,center);
    self->surface=surface;self->piece=piece;
}
static void Sphere_WritePoint(CollisionPointOut destination,CollisionVector3 value)
{
    // Assign one fully evaluated point before the next ray expression is read
    memcpy(destination.x,&value.x,sizeof(float));memcpy(destination.y,&value.y,sizeof(float));memcpy(destination.z,&value.z,sizeof(float));
}
bool ColSphere_IntersectRay(CColSphere *self,CollisionPointRef from,CollisionPointRef dir,CollisionPointOut entry,CollisionPointOut exit)
{
    // Retain the original vector subtraction, squared-distance grouping and unit quadratic coefficient
    const CollisionVector3 delta={*from.x-self->bounds.center.x,*from.y-self->bounds.center.y,*from.z-self->bounds.center.z};
    const float touch=delta.x*delta.x+delta.y*delta.y+delta.z*delta.z-self->bounds.radius*self->bounds.radius;
    float root1,root2;
    if(!General_SolveQuadratic(1.0f,(delta.x**dir.x+delta.y**dir.y+delta.z**dir.z)*2.f,touch,&root1,&root2))return false;
    // Evaluate the complete entry point before writing any of its coordinates
    const CollisionVector3 first={*from.x+*dir.x*root1,*from.y+*dir.y*root1,*from.z+*dir.z*root1};
    Sphere_WritePoint(entry,first);
    // Reread live inputs after entry writes so valid input/output aliases retain their original behavior
    const CollisionVector3 second={*from.x+*dir.x*root2,*from.y+*dir.y*root2,*from.z+*dir.z*root2};
    Sphere_WritePoint(exit,second);return true;
}
CSphere Sphere_FromRaw16(const void *bytes)
{
    // Transfer only the actual center/radius quadword into a complete C geometry record
    CSphere value;memcpy(&value,bytes,sizeof(value));return value;
}
CColSphere ColSphere_FromRaw16(const void *bytes)
{
    // Supply the scratch geometry without reading nonexistent metadata beyond its original quadword
    CColSphere value;memset(&value,0,sizeof(value));memcpy(&value.bounds,bytes,sizeof(value.bounds));return value;
}
//- rouz edit (ChatGPT)
'''

def native():
    # Preserve all center/radius bits when the remaining VU interface requires its native vector class
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "ColSphere.h"
#include "CollisionVector_native.h"
#include "VuVector.h"
static inline CVuVector Sphere_ToVu(const CSphere &sphere)
{
    // Transfer the complete original quadword without an unrelated struct view
    CVuVector value;memcpy(&value,&sphere,sizeof(sphere));return value;
}
//- rouz edit (ChatGPT)
'''

def symbols(source):
    # Track record declarations within actual local and parameter lifetimes instead of rewriting unrelated Rw spheres
    code=mask(source);stack=[];scopes=[]
    for n,ch in enumerate(code):
        if ch=='{':stack.append(n)
        elif ch=='}' and stack:scopes.append((stack.pop(),n+1))
    params=[]
    for begin,end in scopes:
        before=code[:begin].rstrip();tail=re.search(r'\)\s*(?:const\s*)?$',before)
        if not tail:continue
        pos=tail.start();depth=1
        while depth and pos:
            pos-=1
            if code[pos]==')':depth+=1
            elif code[pos]=='(':depth-=1
        if depth==0:params.append((pos,end))
    records=[]
    for match in re.finditer(r'\b(CSphere|CColSphere)\b\s*(?:const\s*)?[*&]?\s*(\w+)\b',code):
        scopesHere=[s for s in scopes if s[0]<match.start()<s[1]]+[s for s in params if s[0]<match.start()<s[1]]
        end=min((s[1] for s in scopesHere),default=len(code));records.append((match[2],match[1],match.start(),end))
    return records

def kind(receiver,pos,records):
    # Resolve only the project-owned base geometry and collision records at this use site
    receiver=receiver.strip()
    if receiver.endswith(('.bounds','->bounds')):return 'CSphere' if kind(re.sub(r'(\.|->)bounds$','',receiver),pos,records)=='CColSphere' else None
    if receiver.endswith('boundingSphere') and not re.search(r'\b(?:atomic|morphTarget)->boundingSphere$',receiver):return 'CSphere'
    if re.search(r'(?:\.|->)spheres(?:\[[\s\S]*\])?$',receiver):return 'CColSphere'
    identifier=re.fullmatch(r'(\w+)(?:\[[\s\S]*\])?',receiver)
    if identifier:
        candidates=[row for row in records if row[0]==identifier[1] and row[2]<=pos<row[3]]
        if candidates:return max(candidates,key=lambda row:row[2])[1]
    return None

def centers(source):
    # Locate complete center expressions while retaining native receiver evaluation exactly once
    code=mask(source);records=symbols(source);result=[]
    for match in re.finditer(r'(\.|->)center\b',code):
        start=value_start(code,match.start())
        if kind(source[start:match.start()],match.start(),records):result.append((start,match.end()))
    return result

def transform(original):
    # Retain the exact strided assembly path and scalar matrix operation for the new C sphere prefix
    old=method(original,'TransformPoints');assembly=old[old.index('\t__asm__ __volatile__'):old.index('\n#else')]
    return '''
void TransformColSpheres(CVuVector *out,int numSpheres,const CMatrix &mat,const CColSphere *spheres)
{
#ifdef GTA_PS3
    // Keep the original point count and twenty-byte input stride for the native batch path
    const int n=numSpheres;const void *in=spheres;const int stride=sizeof(CColSphere);
'''+assembly+'''
#else
    // Preserve scalar transformation order while accessing actual C coordinate subobjects
    while(numSpheres--){*out=mat*CollisionVector_ToNative(spheres->bounds.center);out++;spheres++;}
#endif
}
'''

def col_sphere_calls(source):
    # Remove both complete native owners before adapting their consumers and geometry interfaces
    if 'struct CColSphere : public CSphere' in source:return header(source)
    if 'CColSphere::IntersectRay(' in source:return kernel(source)
    if 'static void Sphere_WriteCenter(' in source:return source
    if not any(token in source for token in ['CSphere','CColSphere','boundingSphere','spheres','TransformColLines(']):return source
    original=source
    if '#include "common.h"' in source and 'void TransformColLines(' in source and 'void TransformColSpheres(' not in source:
        source=source.replace('#include "ColLine_native.h"','#include "ColLine_native.h"\n#include "ColSphere_native.h"');source+=transform(original)
    declaration='void TransformColLines(CVuVector *out,int numLines,const CMatrix &mat,const CColLine *lines);'
    if declaration in source and 'void TransformColSpheres(' not in source:source=source.replace(declaration,declaration+'\nstruct CColSphere;\nvoid TransformColSpheres(CVuVector *out,int numSpheres,const CMatrix &mat,const CColSphere *spheres);')
    for which in ['A','B']:source=source.replace('TransformPoints(aSpheres'+which+', model'+which+'.numSpheres, mat'+('AB' if which=='A' else 'BA')+', &model'+which+'.spheres->center, sizeof(CColSphere));','TransformColSpheres(aSpheres'+which+',model'+which+'.numSpheres,mat'+('AB' if which=='A' else 'BA')+',model'+which+'.spheres);')
    source=source.replace('*(CVuVector*)&sphere','Sphere_ToVu(sphere.bounds)')
    source=re.sub(r'\*\(CColSphere\*\)&(bsphereAB|aSpheres[AB]\[i\])',r'Sphere_FromRaw16(&\1)',source)
    source=source.replace('*(CColSphere*)vusphA','ColSphere_FromRaw16(vusphA)')
    # Create native point storage only in the edge-query cases whose output is a distinct local vector
    if '&sphere.center' in source and 'CCollision::TestSphereTriangle(' in source:
        for name in ['CCollision::TestSphereTriangle','CCollision::ProcessSphereTriangle']:
            body=method(source,name);begin=body.index('\tcase 2:');end=body.index('\tcase 3:',begin)
            part=body[begin:end].replace('case 2:','case 2: {',1).replace('&sphere.center','&sphereNativeCenter')
            part=part.replace('\n\t\tif(!insideAB)', '\n\t\t// Keep a native point alive through the existing edge-distance query\n\t\tconst CVector sphereNativeCenter=CollisionVector_ToNative(sphere.center);\n\t\tif(!insideAB)',1)+'\t}\n'
            source=source.replace(body,body[:begin]+part+body[end:])
    code=mask(source);records=symbols(source)
    for match in reversed(list(re.finditer(r'(\.|->)(center|radius)\b',code))):
        start=value_start(code,match.start())
        if kind(source[start:match.start()],match.start(),records)=='CColSphere':source=source[:match.start()]+match[1]+'bounds.'+match[2]+source[match.end():]
    code=mask(source);records=symbols(source)
    for match in reversed(list(re.finditer(r'\bTestSphere(?:Box|Sphere)\(',code))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);changed=False
        for n in range(min(len(args),2 if 'TestSphereSphere(' in match[0] else 1)):
            if kind(args[n],match.start(),records)=='CColSphere':args[n]+='.bounds';changed=True
        if changed:source=source[:match.end()]+', '.join(args)+source[end-1:]
    code=mask(source);records=symbols(source);matches=[]
    for match in re.finditer(r'\.Set\(',code):
        start=value_start(code,match.start());receiver=source[start:match.start()];record=kind(receiver,match.start(),records)
        if record:matches.append((start,match,receiver,record))
    for index,(start,match,receiver,record) in reversed(list(enumerate(matches))):
        end=closing(code,match.end()-1);args=arguments(source[match.end():end-1]);assert len(args) in [2,4],args
        indent=source[source.rfind('\n',0,start)+1:start];assert not indent.strip(),receiver
        name='sphereCenter'+str(index);op='ColSphere_Set' if len(args)==4 else 'ColSphere_SetBounds' if record=='CColSphere' else 'Sphere_Set'
        replacement='{\n'+indent+'    // Keep the native center expression alive at the former member-call boundary\n'+indent+'    const CVector &'+name+'='+args[1]+';\n'+indent+'    '+op+'(&'+receiver+','+args[0]+',COLLISION_POINT_REF('+name+')'+(','+','.join(args[2:]) if len(args)>2 else '')+');\n'+indent+'}'
        source=source[:start]+replacement+source[end+1:]
    source=source.replace('followedEntSphere.IntersectRay(GetPosition(), distToDest, startVecCandidate, targetVecCandidate)','ColSphere_IntersectRay(&followedEntSphere,COLLISION_POINT_REF(GetPosition()),COLLISION_POINT_REF(distToDest),COLLISION_POINT_OUT(startVecCandidate),COLLISION_POINT_OUT(targetVecCandidate))')
    # Preserve complete native assignment values before writing the plain C centers
    for start,end in reversed(centers(source)):
        tail=source[end:];assignment=re.match(r'\s*=\s*([^;\n]+);',tail)
        if assignment and not assignment[1].startswith('CollisionVector_FromNative('):source=source[:end]+tail[:assignment.start(1)]+'CollisionVector_FromNative('+assignment[1]+')'+tail[assignment.end(1):]
    code=mask(source);protected=[]
    for match in re.finditer(r'\b(?:COLLISION_POINT_REF|CollisionVector_ToNative)\(',code):protected.append((match.start(),closing(code,match.end()-1)))
    for start,end in reversed(centers(source)):
        if any(a<=start<b for a,b in protected):continue
        if re.match(r'\s*\.[xyz]\b|\s*=',source[end:]):continue
        source=source[:start]+'CollisionVector_ToNative('+source[start:end]+')'+source[end:]
    if source==original:return source
    if '#include "common.h"' in source and '#include "ColSphere_native.h"' not in source:source=source.replace('#include "common.h"','#include "common.h"\n#include "ColSphere_native.h"',1)
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def col_sphere_project(source):
    # Rename the complete C producer while retaining explicit C11 metadata and paired object names
    source=source.replace('..\\src\\collision\\ColSphere.cpp','..\\src\\collision\\ColSphere.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\collision\\ColSphere\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated metadata while selecting C11 for the renamed producer
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\collision\\ColSphere.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    entry='<ClInclude Include="..\\src\\collision\\ColSphere.h" />'
    if '..\\src\\collision\\ColSphere_native.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\collision\\ColSphere_native.h" />')
    return objects.project(source)



def calls(source):
    # Preserve complete sphere ownership before the contact record and its native mutation boundaries
    import col_point_migration as contacts
    return contacts.calls(col_sphere_calls(source))


def project(source):
    # Retain sphere C11 integration before the complete contact producer rename
    import col_point_migration as contacts
    return contacts.project(col_sphere_project(source))
