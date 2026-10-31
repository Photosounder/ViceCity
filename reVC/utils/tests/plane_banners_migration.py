"""Move all eight banner methods, both camera helpers and all banner records to C11."""
import re
from audio_controls_migration import function
import smoke_trails_migration as smoke_trails
TYPES='''typedef struct PlaneBannerVector { float x,y,z; } PlaneBannerVector;
typedef struct PlaneBanner { PlaneBannerVector m_pos[8]; } PlaneBanner;
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
'''+TYPES+'''#ifdef __cplusplus
extern "C" {
#endif
void PlaneBanners_Init(void);
void PlaneBanners_Update(void);
void PlaneBanners_Render(void);
void PlaneBanners_RegisterPoint(PlaneBannerVector position,uint32_t id);
void PlaneBanners_RegisterPointVector(const void *position,uint32_t id);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('PlaneBannerVector','Vector','const void *position','const CVector &v=*(const CVector *)position;PlaneBannerVector result={v.x,v.y,v.z};return result;'),
('PlaneBannerVector','CameraPosition','void','const CVector &v=TheCamera.GetPosition();PlaneBannerVector result={v.x,v.y,v.z};return result;'),
('bool','EnvMap','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif'),
('void *','EnvCamera','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::EnvMapCam;\n#else\n    return NULL;\n#endif'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);'),
('PlaneBannerVector','MatrixPosition','void *matrix','const RwV3d &v=((RwMatrix *)matrix)->pos;PlaneBannerVector result={v.x,v.y,v.z};return result;'),
('bool','SphereVisible','PlaneBannerVector position,float radius','return TheCamera.IsSphereVisible(CVector(position.x,position.y,position.z),radius);'),
('void','SetRGBA','int32_t index,uint8_t alpha','RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],255,255,255,alpha);'),
('void','SetPos','int32_t index,float x,float y,float z','RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);'),
('void','SetU','int32_t index,float u','RwIm3DVertexSetU(&TempBufferRenderVertices[index],u);'),
('void','SetV','int32_t index,float v','RwIm3DVertexSetV(&TempBufferRenderVertices[index],v);'),
('void','ZWrite','bool enabled','RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);'),
('void','ZTest','bool enabled','RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void *)(uintptr_t)enabled);'),
('void','Fog','bool enabled','RwRenderStateSet(rwRENDERSTATEFOGENABLE,(void *)(uintptr_t)enabled);'),
('void','VertexAlpha','bool enabled','RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);'),
('void','SrcBlend','void','RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);'),
('void','DestBlend','void','RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);'),
('void','Raster','void *raster','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);'),
('bool','Transform','int32_t count','return RwIm3DTransform(TempBufferRenderVertices,count,nil,\n#ifdef FIX_BUGS\n        rwIM3D_VERTEXXYZ|rwIM3D_VERTEXUV|rwIM3D_VERTEXRGBA\n#else\n        0\n#endif\n    )!=nil;'),
('void','RenderIndexed','int32_t count','RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,TempBufferRenderIndexList,count);'),
('void','End','void','RwIm3DEnd();')]
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "PlaneBanners.h"
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' PlaneBanners_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='//+ rouz edit (ChatGPT)\n#include "PlaneBanners_backend.h"\n'+''.join(result+' PlaneBanners_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed owner operation and original native field access\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''static PlaneBannerVector PlaneBanners_Subtract(PlaneBannerVector a,PlaneBannerVector b)
{
    // Preserve each original float subtraction and vector value boundary
    PlaneBannerVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static PlaneBannerVector PlaneBanners_Add(PlaneBannerVector a,PlaneBannerVector b)
{
    // Preserve each original component addition after scaled-distance evaluation
    PlaneBannerVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static PlaneBannerVector PlaneBanners_Divide(PlaneBannerVector a,float divisor)
{
    // Divide each original component before the following multiplication by eight
    PlaneBannerVector result={a.x/divisor,a.y/divisor,a.z/divisor};return result;
}
static PlaneBannerVector PlaneBanners_Scale(PlaneBannerVector a,float factor)
{
    // Keep the original separate multiplication and vector value boundary
    PlaneBannerVector result={a.x*factor,a.y*factor,a.z*factor};return result;
}
static float PlaneBanners_Magnitude(PlaneBannerVector a)
{
    // Retain the original square-root argument and left-associated sum order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
'''

def section(source):
    # Delimit the complete stored array and all eight original methods
    a=source.index('CPlaneBanner CPlaneBanners::aArray[5];');last=function(source,'CPlaneBanners::','RegisterPoint');b=source.index(last,a)+len(last);return source[a:b]

def classes(source):
    # Retain the complete two original classes for independent layout verification
    a=source.index('class CPlaneBanner\n');b=source.index('class CEscalator\n',a);return source[a:b]

def calls(source):
    # Remove both obsolete classes, the relocated methods and the now-unused camera-position helper
    if 'CPlaneBanner CPlaneBanners::aArray[5];' in source:source=source.replace(section(source),'').replace(function(source,'','GetEffectCameraPosition'),'')+'\n'+HOST
    if 'class CPlaneBanner\n' in source:source=source.replace(classes(source),'//+ rouz edit (ChatGPT)\n#include "PlaneBanners.h"\n//- rouz edit (ChatGPT)\n\n')
    lines=[]
    for line in source.splitlines(keepends=True):
        if line.lstrip().startswith('//'):lines.append(line);continue
        if 'CPlaneBanners::' in line:lines.append(re.match(r'\s*',line)[0]+'// Enter the C plane-banner algorithm with the original input capture\n')
        line=line.replace('CPlaneBanners::RegisterPoint(GetPosition(),','PlaneBanners_RegisterPointVector(&GetPosition(),')
        for method in ['Init','Update','Render']:line=line.replace('CPlaneBanners::'+method+'(','PlaneBanners_'+method+'(')
        lines.append(line)
    return smoke_trails.calls(''.join(lines))

def kernel(source):
    # Translate both complete camera helpers and every original banner statement
    helper=function(source,'','IsEffectSphereVisible')+'\n'+function(source,'','GetEffectCameraPosition')+'\n'
    helper=helper.replace('CVector','PlaneBannerVector').replace('IsEffectSphereVisible','PlaneBanners_IsSphereVisible').replace('GetEffectCameraPosition','PlaneBanners_GetCameraPosition')
    helper=helper.replace('const PlaneBannerVector &center','PlaneBannerVector center')
    helper=helper.replace('CustomPipes::bRenderingEnvMap','PlaneBanners_BackendEnvMap()').replace('CustomPipes::EnvMapCam','PlaneBanners_BackendEnvCamera()')
    helper=helper.replace('RwFrame *cameraFrame','void *cameraFrame').replace('RwCameraGetFrame(','PlaneBanners_BackendCameraFrame(').replace('RwFrameGetMatrix(','PlaneBanners_BackendFrameMatrix(')
    helper=helper.replace('PlaneBannerVector(PlaneBanners_BackendFrameMatrix(cameraFrame)->pos)','PlaneBanners_BackendMatrixPosition(PlaneBanners_BackendFrameMatrix(cameraFrame))')
    helper=helper.replace('TheCamera.GetPosition()','PlaneBanners_BackendCameraPosition()').replace('TheCamera.IsSphereVisible(','PlaneBanners_BackendSphereVisible(')
    body=section(source).replace('CPlaneBanner CPlaneBanners::aArray[5];','static PlaneBanner aArray[5];')
    for method in ['Init','Update','Render','RegisterPoint']:
        args='CVector pos' if method=='RegisterPoint' else 'void';newargs='PlaneBanner *banner'+(', '+args if args!='void' else '')
        body=body.replace('void\nCPlaneBanner::'+method+'('+args+')','static void\nPlaneBanner_'+method+'('+newargs+')')
    body=re.sub(r'\bm_pos\b','banner->m_pos',body).replace('CPlaneBanners::','PlaneBanners_').replace('CVector ','PlaneBannerVector ')
    for method in ['Init','Update','Render']:body=body.replace('aArray[i].'+method+'()','PlaneBanner_'+method+'(&aArray[i])')
    body=body.replace('aArray[id].RegisterPoint(pos)','PlaneBanner_RegisterPoint(&aArray[id],pos)')
    body=body.replace('banner->m_pos[i] - banner->m_pos[i-1]','PlaneBanners_Subtract(banner->m_pos[i],banner->m_pos[i-1])').replace('dist.Magnitude()','PlaneBanners_Magnitude(dist)')
    body=body.replace('banner->m_pos[i-1] + dist/len*8.0f','PlaneBanners_Add(banner->m_pos[i-1],PlaneBanners_Scale(PlaneBanners_Divide(dist,len),8.0f))')
    body=body.replace('float camDist = (GetEffectCameraPosition() - banner->m_pos[0]).Magnitude();','PlaneBannerVector cameraPosition=PlaneBanners_GetCameraPosition();\n        // Read the live anchor only after the original camera queries have completed\n        float camDist=PlaneBanners_Magnitude(PlaneBanners_Subtract(cameraPosition,banner->m_pos[0]));')
    body=body.replace('IsEffectSphereVisible(','PlaneBanners_IsSphereVisible(')
    body=re.sub(r'RwIm3DVertexSetRGBA\(&TempBufferRenderVertices\[([^\]]+)\], 255, 255, 255, alpha\)',r'PlaneBanners_BackendSetRGBA(\1,alpha)',body)
    for old,new in [('Pos','SetPos'),('U','SetU'),('V','SetV')]:body=re.sub(r'RwIm3DVertexSet'+old+r'\(&TempBufferRenderVertices\[([^\]]+)\],',lambda match:'PlaneBanners_Backend'+new+'('+match[1]+',',body)
    for state,name in [('ZWRITEENABLE','ZWrite'),('ZTESTENABLE','ZTest'),('FOGENABLE','Fog'),('VERTEXALPHAENABLE','VertexAlpha')]:
        for value,flag in [('TRUE','true'),('FALSE','false')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','PlaneBanners_Backend'+name+'('+flag+')')
    for state,value,name in [('SRCBLEND','rwBLENDSRCALPHA','SrcBlend'),('DESTBLEND','rwBLENDINVSRCALPHA','DestBlend')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','PlaneBanners_Backend'+name+'()')
    body=body.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER,','PlaneBanners_BackendRaster(')
    a=body.index('#ifdef FIX_BUGS');b=body.index('#endif',a)+len('#endif')
    body=body[:a]+'\t\t\tif(PlaneBanners_BackendTransform(TempBufferVerticesStored)){'+body[b:]
    body=body.replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList,','PlaneBanners_BackendRenderIndexed(').replace('RwIm3DEnd()','PlaneBanners_BackendEnd()');body=re.sub(r'\buint32\b','uint32_t',body)
    combined=helper+body
    combined=combined.replace('\n{\n','\n{\n    // Preserve original arithmetic, live owner reads and state-write ordering\n')
    combined=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original branch and traversal order\n',combined,flags=re.M)
    assert not re.search(r'\b(?:CVector|CPlaneBanner|CPlaneBanners|RwIm3D|TempBufferRenderVertices|TheCamera|CustomPipes)\b',combined)
    tail='''
void PlaneBanners_RegisterPointVector(const void *position,uint32_t id)
{
    // Capture the original typed position once before writing any destination fields
    PlaneBanners_RegisterPoint(PlaneBanners_BackendVector(position),id);
}
//- rouz edit (ChatGPT)
'''
    return '//+ rouz edit (ChatGPT)\n#include "PlaneBanners_backend.h"\n#include "Timer.h"\n#include "RenderBuffer.h"\n#include "Rubbish_backend.h"\n#include <math.h>\n#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))\n#define Max(a,b) ((a)>(b)?(a):(b))\n'+HELPERS+combined+tail

def project(source):
    # Add a uniquely named C11 unit without altering existing paired-object metadata
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\PlaneBanners.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\PlaneBanners.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['PlaneBanners.h','PlaneBanners_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return smoke_trails.project(source)
