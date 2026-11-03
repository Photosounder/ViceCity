"""Move the seven plane-trail algorithms and their complete records into C11."""
import re
from audio_controls_migration import function
import plane_banners_migration as plane_banners

TYPES='''typedef struct PlaneTrailVector { float x,y,z; } PlaneTrailVector;
typedef struct PlaneTrail {
    PlaneTrailVector m_pos[16];
    int32_t m_time[16];
} PlaneTrail;
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
'''+TYPES+'''#ifdef __cplusplus
extern "C" {
#endif
void PlaneTrails_Init(void);
void PlaneTrails_Update(void);
void PlaneTrails_Render(void);
void PlaneTrails_RegisterPoint(PlaneTrailVector pos,uint32_t id);
void PlaneTrails_RegisterPointVector(const void *pos,uint32_t id);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
DESCRIPTOR='''typedef struct PlaneTrailsPosition { void *owner;float *x,*y,*z; } PlaneTrailsPosition;
typedef void (*PlaneTrailsPositionCallback)(PlaneTrailsPosition position);
'''
READERS=[
('PlaneTrailVector','Vector','const void *position','const CVector &v=*(const CVector *)position;PlaneTrailVector result={v.x,v.y,v.z};return result;'),
('void','WithPosition','PlaneTrailsPositionCallback callback','CVector value;PlaneTrailsPosition fields={&value,&value.x,&value.y,&value.z};callback(fields);'),
('bool','RenderingEnvMap','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif'),
('bool','SphereVisible','PlaneTrailVector position,float radius','return IsEffectSphereVisible(CVector(position.x,position.y,position.z),radius);'),
('void','SetRGBA','int32_t index,uint8_t alpha','RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],255,255,255,alpha);'),
('void','SetPos','int32_t index,float x,float y,float z','RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);'),
('void','VertexAlpha','void','RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)TRUE);'),
('void','SrcBlend','void','RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);'),
('void','DestBlend','void','RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);'),
('void','RasterNull','void','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);'),
('bool','Transform','int32_t count','return RwIm3DTransform(TempBufferRenderVertices,count,nil,rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA)!=nil;'),
('void','RenderIndexed','uint16_t *indices,int32_t count','RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST,indices,count);'),
('void','End','void','RwIm3DEnd();'),
('void','RegisterCorona','uint32_t id,void *position','CCoronas::RegisterCorona(id,255,0,0,255,*(CVector *)position,5.0f,2000.0f,CCoronas::TYPE_NORMAL,CCoronas::FLARE_NONE,CCoronas::REFLECTION_OFF,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_OFF,0.0f);'),
('void','UpdateCorona','uint32_t id,void *position','CCoronas::UpdateCoronaCoors(id,*(CVector *)position,2000.0f,0.0f);')]
for name in ['Foggyness','CloudCoverage','Rain']:READERS.append(('float',name,'void','return CWeather::'+name+';'))
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "PlaneTrails.h"
'''+DESCRIPTOR+'''#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' PlaneTrails_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='//+ rouz edit (ChatGPT)\n#include "PlaneTrails_backend.h"\n'+''.join(result+' PlaneTrails_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed owner operation and original borrowed-vector lifetime\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'

def section(source):
    # Delimit exactly the original indices and all seven record/controller methods
    a=source.index('RwImVertexIndex TrailIndices[32]');last=function(source,'CPlaneTrails::','RegisterPoint');b=source.index(last,a)+len(last);return source[a:b]

def classes(source):
    # Retain the complete two original declarations for independent native layout probes
    a=source.index('class CPlaneTrail\n');b=source.index('class CPlaneBanner\n',a);return source[a:b]

def calls(source):
    # Remove only the two obsolete classes and all seven relocated implementations
    if 'RwImVertexIndex TrailIndices[32]' in source:
        source=source.replace('CPlaneTrail CPlaneTrails::aArray[6];','').replace(section(source),'')+'\n'+HOST
    if 'class CPlaneTrail\n' in source:source=source.replace(classes(source),'//+ rouz edit (ChatGPT)\n#include "PlaneTrails.h"\n//- rouz edit (ChatGPT)\n\n')
    lines=[]
    for line in source.splitlines(keepends=True):
        if line.lstrip().startswith('//'):lines.append(line);continue
        if 'CPlaneTrails::' in line:lines.append(re.match(r'\s*',line)[0]+'// Enter the C plane-trail algorithm with the original input capture\n')
        line=line.replace('CPlaneTrails::RegisterPoint(GetPosition(),','PlaneTrails_RegisterPointVector(&GetPosition(),')
        for method in ['Init','Update','Render']:line=line.replace('CPlaneTrails::'+method+'(','PlaneTrails_'+method+'(')
        lines.append(line)
    return plane_banners.calls(''.join(lines))

def kernel(source):
    # Translate every complete original record and controller statement without replacing their algorithms
    body=section(source)
    body=body.replace('RwImVertexIndex TrailIndices','static uint16_t TrailIndices')
    for method in ['Init','Render','RegisterPoint']:
        args={'Init':'void','Render':'float visibility','RegisterPoint':'CVector pos'}[method]
        newargs='PlaneTrail *trail'+(', '+args if args!='void' else '')
        body=body.replace('void\nCPlaneTrail::'+method+'('+args+')','static void\nPlaneTrail_'+method+'('+newargs+')')
    body=re.sub(r'\bm_pos\b','trail->m_pos',body);body=re.sub(r'\bm_time\b','trail->m_time',body)
    body=body.replace('CPlaneTrails::','PlaneTrails_')
    body=body.replace('CVector planePos;','')
    body=body.replace('PlaneTrails_Update(void)','PlaneTrails_UpdateWithPosition(PlaneTrailsPosition planePos)')
    body=body.replace('void\nPlaneTrails_UpdateWithPosition','static void\nPlaneTrails_UpdateWithPosition')
    body=body.replace('RegisterPoint(planePos,','PlaneTrails_RegisterPoint(PlaneTrails_ReadPosition(planePos),')
    body=body.replace('aArray[i].Init()','PlaneTrail_Init(&aArray[i])').replace('aArray[i].Render(visibility)','PlaneTrail_Render(&aArray[i],visibility)').replace('aArray[id].RegisterPoint(pos)','PlaneTrail_RegisterPoint(&aArray[id],pos)')
    body=body.replace('CVector pos','PlaneTrailVector pos')
    body=re.sub(r'planePos\.([xyz])',r'(*planePos.\1)',body)
    body=re.sub(r'CCoronas::RegisterCorona\((\d+), 255, 0, 0, 255, planePos, 5\.0f, 2000\.0f,\s*CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_OFF,\s*CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0\.0f\)',r'PlaneTrails_BackendRegisterCorona(\1,planePos.owner)',body)
    body=re.sub(r'CCoronas::UpdateCoronaCoors\((\d+), planePos, 2000\.0f, 0\.0f\)',r'PlaneTrails_BackendUpdateCorona(\1,planePos.owner)',body)
    body=body.replace('CustomPipes::bRenderingEnvMap','PlaneTrails_BackendRenderingEnvMap()').replace('IsEffectSphereVisible(','PlaneTrails_BackendSphereVisible(')
    body=body.replace('RwIm3DVertexSetRGBA(&TempBufferRenderVertices[numVerts], 255, 255, 255, (int)(alpha*fade))','PlaneTrails_BackendSetRGBA(numVerts,(int)(alpha*fade))')
    body=body.replace('RwIm3DVertexSetPos(&TempBufferRenderVertices[numVerts],','PlaneTrails_BackendSetPos(numVerts,')
    for state,value,name in [('VERTEXALPHAENABLE','TRUE','VertexAlpha'),('SRCBLEND','rwBLENDSRCALPHA','SrcBlend'),('DESTBLEND','rwBLENDINVSRCALPHA','DestBlend')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','PlaneTrails_Backend'+name+'()')
    body=body.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil)','PlaneTrails_BackendRasterNull()').replace('RwIm3DTransform(TempBufferRenderVertices, numVerts, nil, rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA)','PlaneTrails_BackendTransform(numVerts)').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST, TrailIndices,','PlaneTrails_BackendRenderIndexed(TrailIndices,').replace('RwIm3DEnd()','PlaneTrails_BackendEnd()')
    for name in ['Foggyness','CloudCoverage','Rain']:body=body.replace('CWeather::'+name,'PlaneTrails_Backend'+name+'()')
    for name in ['Red','Green','Blue']:body=body.replace('CTimeCycle::GetSkyTop'+name+'()','TimeCycle_GetSkyTop'+name+'()')
    body=body.replace('Sin(','sinf(').replace('Cos(','cosf(');body=re.sub(r'\bint32\b','int32_t',body);body=re.sub(r'\buint32\b','uint32_t',body)
    body=body.replace('\n{\n','\n{\n    // Preserve all original live reads, timestamp decisions and state-write ordering\n')
    body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original branch and traversal order\n',body,flags=re.M)
    assert not re.search(r'\b(?:CVector|CPlaneTrail|CPlaneTrails|CCoronas|CWeather|CTimeCycle|TempBufferRenderVertices|CustomPipes)\b',body)
    prefix='''//+ rouz edit (ChatGPT)
#include "PlaneTrails_backend.h"
#include "Timecycle.h"
#include "Clock.h"
#include "Timer.h"
#include "Draw_constants.h"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
static PlaneTrail aArray[6];
static PlaneTrailVector PlaneTrails_ReadPosition(PlaneTrailsPosition position)
{
    // Capture the original by-value vector before entering trail registration
    PlaneTrailVector value={*position.x,*position.y,*position.z};return value;
}
'''
    tail='''
void PlaneTrails_Update(void)
{
    // Keep one actual temporary vector alive across all three original corona calls
    PlaneTrails_BackendWithPosition(PlaneTrails_UpdateWithPosition);
}
void PlaneTrails_RegisterPointVector(const void *position,uint32_t id)
{
    // Capture the original typed input once before any timestamp reads or trail writes
    PlaneTrails_RegisterPoint(PlaneTrails_BackendVector(position),id);
}
//- rouz edit (ChatGPT)
'''
    return prefix+body+tail

def project(source):
    # Add a distinct C11 compilation unit while preserving all existing paired-object names
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\PlaneTrails.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\PlaneTrails.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['PlaneTrails.h','PlaneTrails_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return plane_banners.project(source)
