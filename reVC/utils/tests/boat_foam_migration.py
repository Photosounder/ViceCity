"""Move the complete special-particle/boat-foam class and animation state into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import markers_3d_migration as markers
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void BoatFoam_CreateFoamAroundObject(void *matrix,float innerFw,float innerRg,float innerUp,int32_t particles);
void BoatFoam_StartBoatFoamAnimation(void);
void BoatFoam_UpdateBoatFoamAnimation(void *matrix);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
VECTOR='typedef struct BoatFoamVector { float x,y,z; } BoatFoamVector;\n'
MATRIX='typedef struct BoatFoamMatrix { float rx,ry,rz,fx,fy,fz,ux,uy,uz,px,py,pz; } BoatFoamMatrix;\n'
READERS=[('BoatFoamVector','CameraForward','void','const CVector &v=TheCamera.GetForward();\n    BoatFoamVector result={v.x,v.y,0.0f};return result;','Read only the original two camera components used by the 2D dot product')]
for name in ['Position','Forward','Right','Up']:READERS.append(('BoatFoamVector','Matrix'+name,'void *matrix','const CVector &v=((CMatrix *)matrix)->Get'+name+'();\n    BoatFoamVector result={v.x,v.y,v.z};return result;','Copy actual typed matrix '+name+' fields at their original use'))
READERS += [
('BoatFoamMatrix','ReadMatrix','void *matrix','const CMatrix &m=*(CMatrix *)matrix;\n    BoatFoamMatrix result={m.rx,m.ry,m.rz,m.fx,m.fy,m.fz,m.ux,m.uy,m.uz,m.px,m.py,m.pz};return result;','Copy original named transform coefficients without assuming matrix layout'),
('void','Particle','int32_t type,BoatFoamVector position,float size','CParticle::AddParticle((tParticleType)type,CVector(position.x,position.y,position.z),CVector(0.0f,0.0f,0.0f),nil,size,FoamColour,1,0,0,0);','Enter the exact typed particle overload while retaining its live original color reference')]
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "BoatFoam.h"\n'+VECTOR+MATRIX+'#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' BoatFoam_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "BoatFoam_backend.h"
#include "Camera.h"
#include "Particle.h"
CRGBA FoamColour(255,255,255,255);
'''+''.join(result+' BoatFoam_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'//- rouz edit (ChatGPT)\n'

def section(source):
    # Extract every complete original method and timestamp definition without surrounding markers
    return 'uint32 CSpecialParticleStuff::BoatFromStart;\n\n'+'\n\n'.join(function(source,'CSpecialParticleStuff::',name) for name in ['CreateFoamAroundObject','StartBoatFoamAnimation','UpdateBoatFoamAnimation'])+'\n'

def record_header(source):
    # Expose the original private timestamp solely to independent test observations
    a=source.index('class CSpecialParticleStuff\n');b=source.index('\n};',a)+3;return source[a:b].replace('class CSpecialParticleStuff\n{','class CSpecialParticleStuff\n{\npublic:')

def calls(source):
    # Remove the original class and methods while retaining disabled script command regions
    if 'uint32 CSpecialParticleStuff::BoatFromStart;' in source:
        source=source.replace('CRGBA FoamColour(255, 255, 255, 255);\n','').replace('uint32 CSpecialParticleStuff::BoatFromStart;\n','')
        for name in ['CreateFoamAroundObject','StartBoatFoamAnimation','UpdateBoatFoamAnimation']:source=source.replace(function(source,'CSpecialParticleStuff::',name)+'\n','')
    if 'class CSpecialParticleStuff\n' in source:source=re.sub(r'class CSpecialParticleStuff\n\{.*?\n\};','#include "BoatFoam.h"',source,flags=re.S)
    source=source.replace('CSpecialParticleStuff::','BoatFoam_')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bBoatFoam_(?:StartBoatFoamAnimation|UpdateBoatFoamAnimation)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the C boat-foam algorithm at the original script call point\n')
        lines.append(line)
    return markers.calls(''.join(lines))

HELPERS='''#ifdef _MSC_VER
float fast_absf(float value);
#define BOAT_FOAM_ABS fast_absf
#else
#define BOAT_FOAM_ABS fabsf
#endif
static BoatFoamVector BoatFoam_Add(BoatFoamVector a,BoatFoamVector b)
{
    // Retain the original component-wise position addition
    BoatFoamVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static BoatFoamVector BoatFoam_Subtract(BoatFoamVector a,BoatFoamVector b)
{
    // Retain the original component-wise displacement calculation
    BoatFoamVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float BoatFoam_Dot(BoatFoamVector a,BoatFoamVector b)
{
    // Preserve the original three-product addition order
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float BoatFoam_Dot2D(BoatFoamVector a,BoatFoamVector b)
{
    // Preserve the original two-dimensional camera rejection expression
    return a.x*b.x+a.y*b.y;
}
static BoatFoamVector BoatFoam_RandomPosition(int rnd,float innerFw)
{
    // Preserve both original signed-byte conversions and multiply-then-divide order
    BoatFoamVector pos;pos.x=(int8_t)(rnd-128)*innerFw/110.0f;pos.y=(int8_t)((rnd>>8)-128)*innerFw/110.0f;pos.z=0.0f;return pos;
}
static BoatFoamVector BoatFoam_Transform(void *matrix,BoatFoamVector v)
{
    // Retain every original matrix-vector product and addition in C
    BoatFoamMatrix m=BoatFoam_BackendReadMatrix(matrix);
    BoatFoamVector result={m.rx*v.x+m.fx*v.y+m.ux*v.z+m.px,m.ry*v.x+m.fy*v.y+m.uy*v.z+m.py,m.rz*v.x+m.fz*v.y+m.uz*v.z+m.pz};return result;
}
'''

def kernel(source):
    # Relocate all three full methods, timestamp and eight original persistent animation values
    body=section(source).replace('uint32 CSpecialParticleStuff::BoatFromStart;','static uint32_t BoatFromStart;').replace('CSpecialParticleStuff::','BoatFoam_')
    body=body.replace('CMatrix* pMatrix','void *pMatrix').replace('BoatFoam_StartBoatFoamAnimation()','BoatFoam_StartBoatFoamAnimation(void)')
    body=re.sub(r'(?<!\w)CreateFoamAroundObject\(',r'BoatFoam_CreateFoamAroundObject(',body)
    start=body.index('\t\tpos.x = (int8)');end=body.index('\n\t\tif (DotProduct2D',start)
    body=body[:start]+'\t\tpos = BoatFoam_RandomPosition(rnd,innerFw);'+body[end:]
    body=body.replace('DotProduct2D(pos, TheCamera.GetForward())','BoatFoam_Dot2D(pos,BoatFoam_BackendCameraForward())')
    body=body.replace('pos += pMatrix->GetPosition()','pos = BoatFoam_Add(pos,BoatFoam_BackendMatrixPosition(pMatrix))')
    for name in ['Forward','Right','Up']:body=body.replace('DotProduct(pMatrix->Get'+name+'(), pos - pMatrix->GetPosition())','BoatFoam_Dot(BoatFoam_BackendMatrix'+name+'(pMatrix),BoatFoam_Subtract(pos,BoatFoam_BackendMatrixPosition(pMatrix)))')
    body=body.replace('*pMatrix * CVector(X + dX * frameInAnimation, Y + dY * frameInAnimation, Z + dZ * frameInAnimation)','BoatFoam_Transform(pMatrix,(BoatFoamVector){X + dX * frameInAnimation, Y + dY * frameInAnimation, Z + dZ * frameInAnimation})')
    body=re.sub(r'CParticle::AddParticle\((PARTICLE_STEAM2|PARTICLE_STEAM_NY), pos, CVector\(0.0f, 0.0f, 0.0f\),\s*nil, ([^,]+), FoamColour, 1, 0, 0, 0\)',r'BoatFoam_BackendParticle(\1,pos,\2)',body)
    for a,b in [('CVector','BoatFoamVector'),('uint32','uint32_t'),('int32','int32_t'),('int8','int8_t')]:body=re.sub(r'\b'+a+r'\b',b,body)
    body=body.replace('Abs(','BOAT_FOAM_ABS(')
    body=body.replace('\n{\n','\n{\n\t// Retain the complete original foam and animation operation order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve original timer, random, rejection and callback behavior\n',body,flags=re.M)
    assert not re.search(r'\b(?:CSpecialParticleStuff|CVector|CMatrix|CParticle|TheCamera|DotProduct|DotProduct2D|FoamColour)\b',body)
    return '//+ rouz edit (ChatGPT)\n#include "BoatFoam_backend.h"\n#include "Timer.h"\n#include "General.h"\n#include "ParticleType.h"\n#include <math.h>\n\n'+HELPERS+body.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile C logic and the small typed particle/matrix boundary into distinct objects
    if '..\\src\\renderer\\BoatFoam.c"' not in source:
        match=re.search(r'<ClCompile Include="\.\.\\src\\renderer\\BrightLights\.c">[\s\S]*?</ClCompile>',source)
        if match:
            entry=match[0]
            if '<Filter>' in entry:source=source.replace(entry,entry+'\n    '+entry.replace('BrightLights.c','BoatFoam.cpp')+'\n    '+entry.replace('BrightLights.c','BoatFoam.c'))
            else:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\BoatFoam.cpp" />\n    '+entry.replace('BrightLights.c','BoatFoam.c'))
    for name in ['BoatFoam.h','BoatFoam_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    return markers.project(source)
