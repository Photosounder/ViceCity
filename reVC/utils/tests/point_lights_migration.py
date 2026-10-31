"""Move complete point-light and fog algorithms into C with typed owner operations."""
import re
import object_names_migration as objects
import render_buffer_migration as render_buffer

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct PointLightsVector { float x,y,z; } PointLightsVector;
typedef struct PointLight {
    PointLightsVector coors,dir;
    float radius,red,green,blue;
    int8_t type,fogType;
    bool castExtraShadows;
} PointLight;
enum {
    POINTLIGHTS_LIGHT_POINT, POINTLIGHTS_LIGHT_DIRECTIONAL, POINTLIGHTS_LIGHT_DARKEN,
    POINTLIGHTS_LIGHT_FOGONLY_ALWAYS, POINTLIGHTS_LIGHT_FOGONLY
};
enum { POINTLIGHTS_FOG_NONE, POINTLIGHTS_FOG_NORMAL, POINTLIGHTS_FOG_ALWAYS };
#ifdef __cplusplus
extern "C" {
#endif
extern int16_t PointLights_NumLights;
extern PointLight PointLights_aLights[NUMPOINTLIGHTS];
extern PointLightsVector PointLights_aCachedMapReads[32];
extern float PointLights_aCachedMapReadResults[32];
extern int32_t PointLights_NextCachedValue;
extern float FogSizes[8];
void PointLights_Init(void);
void PointLights_InitPerFrame(void);
void PointLights_ResetEnvMapLights(void);
void PointLights_AddLight(uint8_t type, PointLightsVector coors, PointLightsVector dir, float radius, float red, float green, float blue, uint8_t fogType, bool castExtraShadows);
float PointLights_GenerateLightsAffectingObject(const PointLightsVector *objCoors);
float PointLights_GenerateLightsAffectingObjectLegacy(const void *objCoors);
void PointLights_RemoveLightsAffectingObject(void);
void PointLights_RenderFogEffect(void);
void PointLights_RenderFogEffectForEnvMap(void *camera);
void PointLights_RenderFogEffectForEnvMapLights(void *camera, PointLight *lights, int32_t lightCount);
bool PointLights_ProcessVerticalLineUsingCache(PointLightsVector coors,float *groundZ);
#ifdef __cplusplus
}
class CVector;
void PointLights_AddLightLegacy(uint8_t type,CVector coors,CVector dir,float radius,float red,float green,float blue,uint8_t fogType,bool castExtraShadows);
CVector PointLights_VectorToLegacy(PointLightsVector value);
#endif
//- rouz edit (ChatGPT)
'''

READERS=[
('PointLightsVector','ReadVector','const void *value','const CVector *v=(const CVector *)value;\n    PointLightsVector result={v->x,v->y,v->z};\n    return result;','Read actual legacy vector components only at their original use'),
('PointLightsVector','Camera','void','return PointLights_BackendReadVector(&TheCamera.GetPosition());','Read the original live main-camera position'),
('bool','EnvMap','void','#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)\n    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif','Read the original active reflection-pass flag'),
('void *','EnvCamera','void','#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)\n    return CustomPipes::EnvMapCam;\n#else\n    return NULL;\n#endif','Read the actual auxiliary camera pointer'),
('void *','EnvFrame','void *camera','return ((RwCamera *)camera)->getFrame();','Query the original typed frame method'),
('void *','FrameLTM','void *frame','return ((RwFrame *)frame)->getLTM();','Query the original typed local-to-world matrix method'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);','Enter the original RenderWare camera-frame operation'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);','Enter the original RenderWare frame-matrix operation'),
('bool','Cutscene','void','return CCutsceneMgr::IsRunning();','Read the live cutscene eligibility gate'),
('float','Fogginess','void','return CWeather::Foggyness;','Read original live weather intensity'),
('void *','Raster','void','return gpPointlightRaster;','Read the live original fog texture'),
('bool','VerticalLine','PointLightsVector coors,float z2,float *groundZ','CColPoint point;\n    CEntity *entity;\n    bool result=CWorld::ProcessVerticalLine(CVector(coors.x,coors.y,coors.z),z2,point,entity,true,false,false,false,true,false,nil);\n    if(result){\n        // Publish the original collision height only on success\n        *groundZ=point.point.z;\n    }\n    return result;','Use the actual collision record and original vertical-line flags'),
('void','ExtraLight','float x,float y,float z,float red,float green,float blue','AddAnExtraDirectionalLight(Scene.world,x,y,z,red,green,blue);','Enter the original typed directional-light operation'),
('void','RemoveExtra','void','RemoveExtraDirectionalLights(Scene.world);','Remove lights from the original live world'),
('void','SpriteInit','void','CSprite::InitSpriteBuffer();','Initialize the original sprite buffer'),
('void','SpriteFlush','void','CSprite::FlushSpriteBuffer();','Flush the original sprite buffer'),
('bool','Screen','PointLightsVector coors,PointLightsVector *screen,float *width,float *height,bool farclip','CVector result;\n    bool visible=CSprite::CalcScreenCoors(CVector(coors.x,coors.y,coors.z),&result,width,height,farclip);\n    if(visible){\n        // Publish screen coordinates only where the original caller uses them\n        *screen=PointLights_BackendReadVector(&result);\n    }\n    return visible;','Project using actual vector and sprite types'),
('void','Sprite','float x,float y,float z,float width,float height,uint8_t red,uint8_t green,uint8_t blue,int16_t intensity,float reciprocal,float rotation,uint8_t alpha','CSprite::RenderBufferedOneXLUSprite_Rotate_Aspect(x,y,z,width,height,red,green,blue,intensity,reciprocal,rotation,alpha);','Retain the exact original narrowed sprite signature'),
('void','BufferRender','void','RenderBuffer::RenderStuffInBuffer();','Flush pending shared geometry at its original point'),
('void','BufferClear','void','RenderBuffer::ClearRenderBuffer();','Clear the original shared render buffer'),
('void','Start','int indicesCount,int verticesCount,uint16_t **indices,void **vertices','RwIm3DVertex *storage;\n    RenderBuffer::StartStoring(indicesCount,verticesCount,indices,&storage);\n    *vertices=storage;','Return the original typed vertex allocation as an opaque handle'),
('void','Stop','void','RenderBuffer::StopStoring();','Commit the original pending geometry'),
('void','Push','const char *name','PUSH_RENDERGROUP(name);','Retain original debug labels and configuration'),
('void','Pop','void','POP_RENDERGROUP();','Close the original debug group'),
('void','SetRaster','void *raster','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);','Set the original typed texture state')]
for name,field in [('Position','pos'),('Forward','at'),('Right','right'),('Up','up')]:
    READERS.append(('PointLightsVector','Matrix'+name,'void *matrix','const RwV3d &v=((RwMatrix *)matrix)->'+field+';\n    PointLightsVector result={v.x,v.y,v.z};\n    return result;','Read actual matrix '+field+' components'))
for name,state in [('ZWrite','ZWRITEENABLE'),('ZTest','ZTESTENABLE'),('VertexAlpha','VERTEXALPHAENABLE'),('Fog','FOGENABLE')]:
    READERS.append(('void',name,'bool enabled','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)enabled);','Set the original typed '+name+' state'))
for name,state,a,b in [('SrcBlend','SRCBLEND','rwBLENDONE','rwBLENDSRCALPHA'),('DestBlend','DESTBLEND','rwBLENDONE','rwBLENDINVSRCALPHA'),('Cull','CULLMODE','rwCULLMODECULLNONE','rwCULLMODECULLBACK')]:
    READERS.append(('void',name,'bool special','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)(special?'+a+':'+b+'));','Set the original typed '+name+' value'))
for name,args in [('Pos','float x,float y,float z'),('RGBA','uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha'),('U','float value'),('V','float value')]:
    actual={'Pos':'x,y,z','RGBA':'red,green,blue,alpha','U':'value','V':'value'}[name]
    READERS.append(('void','Set'+name,'void *vertices,int index,'+args,'RwIm3DVertexSet'+name+'(&((RwIm3DVertex *)vertices)[index],'+actual+');','Write the original allocated vertex through its typed setter'))

BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "PointLights.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' PointLights_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "CutsceneMgr.h"
#include "Lights.h"
#include "Camera.h"
#include "Weather.h"
#include "World.h"
#include "Collision.h"
#include "Sprite.h"
#include "RenderBuffer.h"
#include "PointLights_backend.h"
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
#include "custompipes.h"
#endif
extern RwRaster *gpPointlightRaster;
'''+''.join(result+' PointLights_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'''
void PointLights_AddLightLegacy(uint8_t type,CVector coors,CVector dir,float radius,float red,float green,float blue,uint8_t fogType,bool castExtraShadows)
{
    // Preserve original by-value legacy inputs before entering the C algorithm
    PointLights_AddLight(type,PointLights_BackendReadVector(&coors),PointLights_BackendReadVector(&dir),radius,red,green,blue,fogType,castExtraShadows);
}
CVector PointLights_VectorToLegacy(PointLightsVector value)
{
    // Return a typed value to the remaining shadow consumer
    return CVector(value.x,value.y,value.z);
}
//- rouz edit (ChatGPT)
'''

HELPERS='''#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define sq(a) ((a)*(a))
#if defined(_MSC_VER)
float fast_absf(float value);
#define Abs fast_absf
#else
#define Abs fabsf
#endif
#define Sqrt sqrtf
#define Sin sinf
#define Cos cosf
typedef PointLightsVector (*PointLightsRead)(const void *);
static PointLightsVector PointLights_Read(const void *value)
{
    // Read actual C vector storage at the original object-coordinate use
    return *(const PointLightsVector *)value;
}
static PointLightsVector PointLights_Add(PointLightsVector a,PointLightsVector b)
{
    // Retain component-wise original addition
    PointLightsVector r={a.x+b.x,a.y+b.y,a.z+b.z};return r;
}
static PointLightsVector PointLights_Subtract(PointLightsVector a,PointLightsVector b)
{
    // Retain component-wise original subtraction
    PointLightsVector r={a.x-b.x,a.y-b.y,a.z-b.z};return r;
}
static PointLightsVector PointLights_Scale(PointLightsVector a,float b)
{
    // Retain original vector-times-scalar order
    PointLightsVector r={a.x*b,a.y*b,a.z*b};return r;
}
static PointLightsVector PointLights_Divide(PointLightsVector a,float b)
{
    // Retain three original divisions rather than multiplying by a reciprocal
    PointLightsVector r={a.x/b,a.y/b,a.z/b};return r;
}
static float PointLights_Magnitude(PointLightsVector a)
{
    // Retain original float square root and product/sum order
    return Sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}
static float PointLights_Dot(PointLightsVector a,PointLightsVector b)
{
    // Retain the three original dot products and additions
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static bool PointLights_Equal(PointLightsVector a,PointLightsVector b)
{
    // Retain original exact component comparisons and short-circuit order
    return a.x==b.x && a.y==b.y && a.z==b.z;
}
static void PointLights_Normalise(PointLightsVector *a)
{
    // Preserve the original reciprocal float root and x-only fallback
    float square=a->x*a->x+a->y*a->y+a->z*a->z;
    if(square>0.0f){
        // Scale each original component in order
        float scale=1.0f/Sqrt(square);a->x*=scale;a->y*=scale;a->z*=scale;
    }else a->x=1.0f;
}
'''
WRAPPERS='''float PointLights_GenerateLightsAffectingObject(const PointLightsVector *objCoors)
{
    // Preserve live C coordinate reads at every original light iteration
    return PointLights_Generate(objCoors,PointLights_Read);
}
float PointLights_GenerateLightsAffectingObjectLegacy(const void *objCoors)
{
    // Read actual legacy vector storage lazily at every original use
    return PointLights_Generate(objCoors,PointLights_BackendReadVector);
}
'''

def calls(source):
    # Replace entire owners or their explicit call and field references
    if 'class CRegisteredPointLight\n' in source:return HEADER
    if 'int16 CPointLights::NumLights;' in source:return render_buffer.calls(HOST)
    source=source.replace('CPointLights::aLights[i].coors - FindPlayerCoors()','PointLights_VectorToLegacy(PointLights_aLights[i].coors) - FindPlayerCoors()')
    changed='CPointLights::' in source
    source=source.replace('CPointLights::AddLight(','PointLights_AddLightLegacy(').replace('CPointLights::GenerateLightsAffectingObject(','PointLights_GenerateLightsAffectingObjectLegacy(')
    source=source.replace('CRegisteredPointLight','PointLight')
    source=re.sub(r'\bCPointLights::((?:LIGHT|FOG)_\w+)',r'POINTLIGHTS_\1',source)
    source=source.replace('CPointLights::','PointLights_')
    if changed:
        lines=[]
        for line in source.splitlines(keepends=True):
            if re.search(r'\bPointLights_(?:AddLightLegacy|GenerateLightsAffectingObjectLegacy|Init|InitPerFrame|RenderFogEffect|RenderFogEffectForEnvMap|ResetEnvMapLights|RemoveLightsAffectingObject)\(',line):
                lines.append(re.match(r'\s*',line)[0]+'// Enter the C point-light algorithm with the original owner expressions\n')
            lines.append(line)
        source=''.join(lines)
    return render_buffer.calls(source)

def kernel(source):
    # Relocate all ten public methods and the complete billboard algorithm
    source=source[source.index('int16 CPointLights::NumLights;'):]
    source=source.replace('CRegisteredPointLight','PointLight')
    source=re.sub(r'\bCPointLights::((?:LIGHT|FOG)_\w+)',r'POINTLIGHTS_\1',source)
    source=source.replace('float\nCPointLights::GenerateLightsAffectingObject(Const CVector *objCoors)','static float\nPointLights_Generate(const void *objCoors, PointLightsRead read)')
    source=source.replace('CPointLights::','PointLights_')
    for name in ['NumLights','aLights','aCachedMapReads','aCachedMapReadResults','NextCachedValue']:
        source=re.sub(r'(?<![\w])'+name+r'\b','PointLights_'+name,source)
    # Retain public diagnostic labels after renaming actual functions
    source=source.replace('"PointLights_RenderFogEffect"','"CPointLights::RenderFogEffect"').replace('"PointLights_RenderFogEffectForEnvMap"','"CPointLights::RenderFogEffectForEnvMap"')
    source=source.replace('extern RwRaster *gpPointlightRaster;\n','')
    source=source.replace('const CVector &','PointLightsVector ').replace('CVector ','PointLightsVector ')
    source=source.replace('RwCamera *camera','void *camera').replace('RwFrame *cameraFrame','void *cameraFrame').replace('RwMatrix *cameraMatrix','void *cameraMatrix')
    source=source.replace('PointLightsVector dist;','PointLightsVector dist;')
    source=source.replace('CVector(0.0f, 0.0f, 0.0f)','(PointLightsVector){0.0f,0.0f,0.0f}')
    source=source.replace('cameraPosition = TheCamera.GetPosition();','cameraPosition = PointLights_BackendCamera();')
    source=source.replace('CustomPipes::EnvMapCam->getFrame()->getLTM()','PointLights_BackendFrameLTM(PointLights_BackendEnvFrame(PointLights_BackendEnvCamera()))')
    source=source.replace('CustomPipes::EnvMapCam->getFrame()','PointLights_BackendEnvFrame(PointLights_BackendEnvCamera())')
    source=source.replace('CVector(PointLights_BackendFrameLTM(PointLights_BackendEnvFrame(PointLights_BackendEnvCamera()))->pos)','PointLights_BackendMatrixPosition(PointLights_BackendFrameLTM(PointLights_BackendEnvFrame(PointLights_BackendEnvCamera())))')
    source=source.replace('CustomPipes::bRenderingEnvMap','PointLights_BackendEnvMap()').replace('CustomPipes::EnvMapCam','PointLights_BackendEnvCamera()')
    source=source.replace('dist = coors - cameraPosition;','dist = PointLights_Subtract(coors,cameraPosition);').replace('dist = light->coors - *objCoors;','dist = PointLights_Subtract(light->coors,read(objCoors));')
    source=source.replace('dist.Magnitude()','PointLights_Magnitude(dist)').replace('PointLightsVector dir = dist / distance;','PointLightsVector dir = PointLights_Divide(dist,distance);').replace('DotProduct(dir, light->dir)','PointLights_Dot(dir,light->dir)')
    source=source.replace('AddAnExtraDirectionalLight(Scene.world,','PointLights_BackendExtraLight(').replace('RemoveExtraDirectionalLights(Scene.world)','PointLights_BackendRemoveExtra()')
    source=source.replace('PointLightsVector horizontal = cameraRight*(size*cosine) + cameraUp*(size*sine);','PointLightsVector horizontal = PointLights_Add(PointLights_Scale(cameraRight,(size*cosine)),PointLights_Scale(cameraUp,(size*sine)));')
    source=source.replace('PointLightsVector vertical = cameraUp*(size*cosine) - cameraRight*(size*sine);','PointLightsVector vertical = PointLights_Subtract(PointLights_Scale(cameraUp,(size*cosine)),PointLights_Scale(cameraRight,(size*sine)));')
    for expression,replacement in [('position-horizontal+vertical','PointLights_Add(PointLights_Subtract(position,horizontal),vertical)'),('position+horizontal+vertical','PointLights_Add(PointLights_Add(position,horizontal),vertical)'),('position-horizontal-vertical','PointLights_Subtract(PointLights_Subtract(position,horizontal),vertical)'),('position+horizontal-vertical','PointLights_Subtract(PointLights_Add(position,horizontal),vertical)')]:source=source.replace(expression,replacement)
    source=source.replace('RwIm3DVertex *vertices;','void *vertices;').replace('RwImVertexIndex *indices;','uint16_t *indices;')
    source=source.replace('RenderBuffer::StartStoring(','PointLights_BackendStart(').replace('RenderBuffer::StopStoring()','PointLights_BackendStop()').replace('RenderBuffer::RenderStuffInBuffer()','PointLights_BackendBufferRender()').replace('RenderBuffer::ClearRenderBuffer()','PointLights_BackendBufferClear()')
    for operation in ['Pos','RGBA','U','V']:source=re.sub(r'RwIm3DVertexSet'+operation+r'\(&vertices\[([^\]]+)\],',r'PointLights_BackendSet'+operation+r'(vertices,\1,',source)
    source=source.replace('CColPoint point;','float pointZ;').replace('point.point.z','pointZ')
    source=re.sub(r'^[\t ]*CEntity \*entity;\n','',source,flags=re.M)
    source=re.sub(r'CWorld::ProcessVerticalLine\((fogcoors|fogPosition|coors), ([^,\n]+),\s*point, entity, true, false, false, false, true, false, nil\)',r'PointLights_BackendVerticalLine(\1,\2,&pointZ)',source)
    source=re.sub(r'PointLightsVector (fogcoors|fogPosition)\(xi, yi, ([^;]+)\);',r'PointLightsVector \1={(float)xi,(float)yi,\2};',source)
    source=source.replace('CCutsceneMgr::IsRunning()','PointLights_BackendCutscene()').replace('CWeather::Foggyness','PointLights_BackendFogginess()')
    source=source.replace('CSprite::InitSpriteBuffer()','PointLights_BackendSpriteInit()').replace('CSprite::FlushSpriteBuffer()','PointLights_BackendSpriteFlush()').replace('CSprite::CalcScreenCoors(','PointLights_BackendScreen(').replace('CSprite::RenderBufferedOneXLUSprite_Rotate_Aspect(','PointLights_BackendSprite(')
    source=source.replace('TheCamera.GetPosition().x','PointLights_BackendCamera().x').replace('TheCamera.GetPosition().y','PointLights_BackendCamera().y')
    source=source.replace('RwCameraGetFrame(camera)','PointLights_BackendCameraFrame(camera)').replace('RwFrameGetMatrix(cameraFrame)','PointLights_BackendFrameMatrix(cameraFrame)')
    for name,field in [('cameraPos','Position'),('cameraForward','Forward'),('cameraRight','Right'),('cameraUp','Up')]:
        oldfield={'Position':'pos','Forward':'at','Right':'right','Up':'up'}[field]
        source=source.replace('PointLightsVector '+name+'(cameraMatrix->'+oldfield+');','PointLightsVector '+name+' = PointLights_BackendMatrix'+field+'(cameraMatrix);')
        source=source.replace(name+'.Normalise();','PointLights_Normalise(&'+name+');')
    source=source.replace('PointLights_aCachedMapReads[i] == coors','PointLights_Equal(PointLights_aCachedMapReads[i],coors)')
    source=re.sub(r'(?<![\w])ProcessVerticalLineUsingCache\(',r'PointLights_ProcessVerticalLineUsingCache(',source)
    source=re.sub(r'(?<![\w])RenderFogEffectForEnvMapLights\(',r'PointLights_RenderFogEffectForEnvMapLights(',source)
    for state,name in [('ZWRITEENABLE','ZWrite'),('ZTESTENABLE','ZTest'),('VERTEXALPHAENABLE','VertexAlpha'),('FOGENABLE','Fog')]:
        source=re.sub(r'RwRenderStateSet\(rwRENDERSTATE'+state+r', \(void\*\)(TRUE|FALSE)\)',lambda match:'PointLights_Backend'+name+'('+('true' if match[1]=='TRUE' else 'false')+')',source)
    for state,name,value in [('SRCBLEND','SrcBlend','rwBLENDSRCALPHA'),('DESTBLEND','DestBlend','rwBLENDINVSRCALPHA'),('CULLMODE','Cull','rwCULLMODECULLBACK')]:
        special='rwCULLMODECULLNONE' if state=='CULLMODE' else 'rwBLENDONE'
        for token,boolean in [(special,'true'),(value,'false')]:source=source.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+token+')','PointLights_Backend'+name+'('+boolean+')')
    source=source.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpPointlightRaster)','PointLights_BackendSetRaster(PointLights_BackendRaster())').replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil)','PointLights_BackendSetRaster(NULL)').replace('gpPointlightRaster == nil','PointLights_BackendRaster() == NULL')
    source=source.replace('PUSH_RENDERGROUP(','PointLights_BackendPush(').replace('POP_RENDERGROUP()','PointLights_BackendPop()').replace('nil','NULL')
    for a,b in [('int16','int16_t'),('int32','int32_t'),('int8','int8_t'),('uint8','uint8_t')]:source=re.sub(r'\b'+a+r'\b',b,source)
    for prefix in ['LIGHT','FOG']:source=re.sub(r'(?<![\w])'+prefix+r'_(POINT|DIRECTIONAL|DARKEN|FOGONLY_ALWAYS|FOGONLY|NONE|NORMAL|ALWAYS)\b',r'POINTLIGHTS_'+prefix+r'_\1',source)
    source=re.sub(r'\t*//[+-] rouz edit \(ChatGPT\)\n','',source)
    source=re.sub(r'(\n\{\n)',r'\1\t// Preserve the complete original point-light traversal and operation order\n',source)
    source=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Retain the original eligibility, arithmetic and write order\n',source,flags=re.M)
    assert not re.search(r'\b(?:CVector|CPointLights|CRegisteredPointLight|CColPoint|CEntity|RwCamera|RwFrame|RwMatrix|RwIm3DVertex|RenderBuffer|CSprite|CWorld)\b',re.sub(r'"[^"\n]*"','',source))
    return '//+ rouz edit (ChatGPT)\n#include "PointLights_backend.h"\n#include "Timer.h"\n#include "Draw_constants.h"\n#include <stddef.h>\n#include <math.h>\n#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n\n'+HELPERS+source.rstrip()+'\n\n'+WRAPPERS+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Compile C11 beside typed C++ operations with separate object names
    if '..\\src\\renderer\\PointLights.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\PointLights.cpp" />';addition='<ClCompile Include="..\\src\\renderer\\PointLights.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\renderer\\PointLights.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('PointLights.cpp','PointLights.c'))
    if '..\\src\\renderer\\PointLights_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\PointLights.h" />','<ClInclude Include="..\\src\\renderer\\PointLights.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('PointLights.h','PointLights_backend.h'))
    return render_buffer.project(objects.project(source))
