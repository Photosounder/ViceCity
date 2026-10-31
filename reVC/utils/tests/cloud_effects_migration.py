"""Move cloud resources, animation, sprites and reflection geometry into C11."""
import re
from audio_controls_migration import function
import cloud_background_migration as background
SCALARS=[('float','CloudRotation'),('uint32_t','IndividualRotation'),('float','ms_cameraRoll'),('float','ms_horizonZ'),('float','ms_HorizonTilt')]
HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "config.h"\ntypedef struct CloudsVector { float x,y,z; } CloudsVector;\ntypedef struct CloudsColor { uint8_t r,g,b,a; } CloudsColor;\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join('extern '+kind+' Clouds_'+name+';\n' for kind,name in SCALARS)+'''void Clouds_Init(void);
void Clouds_Shutdown(void);
void Clouds_Update(void);
void Clouds_Render(void);
#ifdef REVC_SOFTWARE_POLYGONS
void Clouds_RenderForEnvMap(void *camera);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('void','PushTxd','void','CTxdStore::PushCurrentTxd();'),('int32_t','FindTxd','const char *name','return CTxdStore::FindTxdSlot(name);'),('void','SetTxd','int32_t slot','CTxdStore::SetCurrentTxd(slot);'),('void','PopTxd','void','CTxdStore::PopCurrentTxd();'),
('void *','ReadTexture','const char *name,const char *mask','return RwTextureRead(name,mask);'),('void','DestroyTexture','void *texture','RwTextureDestroy((RwTexture *)texture);'),('void *','Raster','void *texture','return RwTextureGetRaster((RwTexture *)texture);'),
('float','Orientation','void','return TheCamera.Orientation;'),('bool','Outside','void','return CGame::CanSeeOutSideFromCurrArea();'),('float','ScreenWidth','void','return SCREEN_WIDTH;'),
('CloudsVector','CameraPosition','void','const CVector &v=TheCamera.GetPosition();CloudsVector result={v.x,v.y,v.z};return result;'),('bool *','SunBlocked','void','return &CCoronas::SunBlockedByClouds;'),('float','MoonSize','void','return CCoronas::MoonSize;'),('float','SunX','void','return CCoronas::SunScreenX;'),('float','SunY','void','return CCoronas::SunScreenY;'),('void *','CoronaTexture','int32_t index','return gpCoronaTexture[index];'),
('bool','Project','CloudsVector value,CloudsVector *out,float *width,float *height,bool farclip','RwV3d projected;RwV3d input={value.x,value.y,value.z};bool visible=CSprite::CalcScreenCoors(input,&projected,width,height,farclip);if(visible){out->x=projected.x;out->y=projected.y;out->z=projected.z;}return visible;'),
('void','InitSprites','void','CSprite::InitSpriteBuffer();'),('void','FlushSprites','void','CSprite::FlushSpriteBuffer();'),
('void','SetRaster','void *raster','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);'),('void','Push','const char *name','PUSH_RENDERGROUP(name);'),('void','Pop','void','POP_RENDERGROUP();')]
for name in ['Wind','Foggyness','CloudCoverage','ExtraSunnyness','Rainbow']:READERS.append(('float',name,'void','return CWeather::'+name+';'))
for name in ['LowCloudsRed','LowCloudsGreen','LowCloudsBlue','FluffyCloudsTopRed','FluffyCloudsTopGreen','FluffyCloudsTopBlue','FluffyCloudsBottomRed','FluffyCloudsBottomGreen','FluffyCloudsBottomBlue']:READERS.append(('int32_t',name,'void','return CTimeCycle::Get'+name+'();'))
for name,state in [('ZWrite','ZWRITEENABLE'),('ZTest','ZTESTENABLE'),('VertexAlpha','VERTEXALPHAENABLE'),('Fog','FOGENABLE')]:READERS.append(('void',name,'bool enabled','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)enabled);'))
for name,state,factor in [('SrcBlend','SRCBLEND','rwBLENDSRCALPHA'),('DestBlend','DESTBLEND','rwBLENDINVSRCALPHA')]:READERS.append(('void',name,'bool additive','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)(additive?rwBLENDONE:'+factor+'));'))
READERS.append(('void','Cull','bool none','RwRenderStateSet(rwRENDERSTATECULLMODE,(void *)(uintptr_t)(none?rwCULLMODECULLNONE:rwCULLMODECULLBACK));'))
SPRITE_ARGS='float x,float y,float z,float w,float h,uint8_t r,uint8_t g,uint8_t b,int16_t intens,float recipz'
for name in ['RenderOneXLUSprite','RenderBufferedOneXLUSprite','RenderBufferedOneXLUSprite_Rotate_Dimension','RenderBufferedOneXLUSprite_Rotate_Aspect']:
    args=SPRITE_ARGS+(',float roll' if '_Rotate_' in name else '')+',uint8_t a'
    READERS.append(('void',name,args,'CSprite::'+name+'('+','.join(arg.split()[-1] for arg in args.split(','))+');'))
args='float x,float y,float z,float w,float h,uint8_t r1,uint8_t g1,uint8_t b1,uint8_t r2,uint8_t g2,uint8_t b2,float cx,float cy,float recipz,float rotation,uint8_t a'
READERS.append(('void','RenderBufferedOneXLUSprite_Rotate_2Colours',args,'CSprite::RenderBufferedOneXLUSprite_Rotate_2Colours('+','.join(arg.split()[-1] for arg in args.split(','))+');'))
REFLECTION=[('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);'),('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);'),('float','NearClip','void *camera','return RwCameraGetNearClipPlane((RwCamera *)camera);'),('float','FarClip','void *camera','return RwCameraGetFarClipPlane((RwCamera *)camera);'),('CloudsVector','SunDirection','void','CVector v=CTimeCycle::GetSunDirection();CloudsVector result={v.x,v.y,v.z};return result;')]
for name,field in [('Position','pos'),('Right','right'),('Up','up'),('Forward','at')]:REFLECTION.append(('CloudsVector','Matrix'+name,'void *matrix','const RwV3d &v=((RwMatrix *)matrix)->'+field+';CloudsVector result={v.x,v.y,v.z};return result;'))
for name,args,call in [('SetPos','float x,float y,float z','RwIm3DVertexSetPos(&v[index],x,y,z);'),('SetRGBA','uint8_t r,uint8_t g,uint8_t b,uint8_t a','RwIm3DVertexSetRGBA(&v[index],r,g,b,a);'),('SetU','float value','RwIm3DVertexSetU(&v[index],value);'),('SetV','float value','RwIm3DVertexSetV(&v[index],value);')]:REFLECTION.append(('void',name,'void *vertices,int32_t index,'+args,'RwIm3DVertex *v=(RwIm3DVertex *)vertices;'+call))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Clouds_control.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' Clouds_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'#ifdef REVC_SOFTWARE_POLYGONS\n'+''.join(result+' Clouds_Backend'+name+'('+args+');\n' for result,name,args,body in REFLECTION)+'#endif\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "Clouds_backend.h"
'''+''.join(result+' Clouds_Backend'+name+'('+args+')\n{\n    // Retain the original typed owner operation at its original C call point\n    '+body+'\n}\n' for result,name,args,body in READERS)+'#ifdef REVC_SOFTWARE_POLYGONS\n'+''.join(result+' Clouds_Backend'+name+'('+args+')\n{\n    // Retain the actual reflection camera or vertex field type without assuming its layout\n    '+body+'\n}\n' for result,name,args,body in REFLECTION)+'#endif\n//- rouz edit (ChatGPT)\n'
HELPERS='''static CloudsVector Clouds_Add(CloudsVector a,CloudsVector b)
{
    // Preserve original component addition
    CloudsVector r={a.x+b.x,a.y+b.y,a.z+b.z};return r;
}
static CloudsVector Clouds_Subtract(CloudsVector a,CloudsVector b)
{
    // Preserve original component subtraction
    CloudsVector r={a.x-b.x,a.y-b.y,a.z-b.z};return r;
}
static CloudsVector Clouds_Scale(CloudsVector a,float b)
{
    // Preserve original vector-times-scalar operand order
    CloudsVector r={a.x*b,a.y*b,a.z*b};return r;
}
static float Clouds_Dot(CloudsVector a,CloudsVector b)
{
    // Preserve the original three-product addition order
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static void Clouds_Normalise(CloudsVector *v)
{
    // Retain the original reciprocal-square-root scale and x-only zero fallback
    float sq=Clouds_Dot(*v,*v);if(sq>0.f){float inv=1.f/sqrtf(sq);v->x*=inv;v->y*=inv;v->z*=inv;}else v->x=1.f;
}
static float Clouds_Max(float a,float b)
{
    // Preserve the original comparison and selected operand
    return a>b?a:b;
}
static float Clouds_Square(float value)
{
    // Evaluate each original squared operand once
    return value*value;
}
static CloudsColor Clouds_Color(uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Narrow original color arguments before storing their four bytes
    CloudsColor c={r,g,b,a};return c;
}
'''

def section(source):
    # Delimit the complete lifecycle, sprite and reflection methods plus their live tables
    a=source.index('void\nCClouds::Init');b=source.index('bool\nUseDarkBackground',a);return source[a:b]

def calls(source):
    # Retain the background/horizon owner while moving all five effect methods and scalar state
    if 'RwTexture *gpCloudTex[5];' in source:
        source=source.replace(section(source),'').replace('RwTexture *gpCloudTex[5];\n','')
        for kind,name in SCALARS:source=re.sub(r'^(?:float|uint32) CClouds::'+name+r';\n','',source,flags=re.M)
        source+='\n'+HOST
    if 'class CClouds\n' in source:
        for kind,name in SCALARS:source=re.sub(r'^\tstatic (?:float|uint32) '+name+r';\n','',source,flags=re.M)
        for name in ['Init','Shutdown','Update','Render','RenderForEnvMap']:source=re.sub(r'^\tstatic void '+name+r'\([^\n]*\);[^\n]*\n','',source,flags=re.M)
        source=source.replace('#pragma once','#pragma once\n#include "Clouds_control.h"',1)
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'CClouds::(?:Init|Shutdown|Update|Render|RenderForEnvMap)\(',line) and not line.lstrip().startswith('//'):lines.append(re.match(r'\s*',line)[0]+'// Enter the complete C cloud effect with the original arguments\n')
        lines.append(line)
    source=''.join(lines)
    for name in ['Init','Shutdown','Update','Render','RenderForEnvMap']:source=source.replace('CClouds::'+name+'(','Clouds_'+name+'(')
    for kind,name in SCALARS:
        source=source.replace('CClouds::'+name,'Clouds_'+name);source=re.sub(r'(?<![\w:])'+name+r'\b','Clouds_'+name,source)
    return background.calls(source)

def kernel(source):
    # Translate all original effect statements and private geometry without changing their ordering
    body=section(source)
    body=body.replace('CClouds::ms_cameraRoll','ms_cameraRoll')
    for name in ['Init','Shutdown','Update','Render','RenderForEnvMap']:body=body.replace('CClouds::'+name+'(','Clouds_'+name+'(')
    for kind,name in SCALARS:body=re.sub(r'\b'+name+r'\b','Clouds_'+name,body)
    body=body.replace('RwCamera *camera','void *camera').replace('RwFrame *cameraFrame','void *cameraFrame').replace('RwMatrix *cameraMatrix','void *cameraMatrix')
    body=body.replace('const CVector &','const CloudsVector *').replace('const CRGBA &','const CloudsColor *')
    body=body.replace('RwImVertexIndex *indices','uint16_t *indices').replace('RwIm3DVertex *vertices','void *vertices').replace('RenderBuffer_StartStoringLegacy(','RenderBuffer_StartStoring(')
    # Translate the helper's borrowed vectors separately from the owning reflection locals
    a=body.index('static void\nStoreEnvMapCloudQuad');b=body.index('void\nClouds_RenderForEnvMap',a);quad=body[a:b]
    quad=quad.replace('cameraRight*(halfWidth*cosine) + cameraUp*(halfWidth*sine)','Clouds_Add(Clouds_Scale(*cameraRight,halfWidth*cosine),Clouds_Scale(*cameraUp,halfWidth*sine))').replace('cameraUp*(halfHeight*cosine) - cameraRight*(halfHeight*sine)','Clouds_Subtract(Clouds_Scale(*cameraUp,halfHeight*cosine),Clouds_Scale(*cameraRight,halfHeight*sine))')
    for expr,new in [('center-horizontal+vertical','Clouds_Add(Clouds_Subtract(*center,horizontal),vertical)'),('center+horizontal+vertical','Clouds_Add(Clouds_Add(*center,horizontal),vertical)'),('center-horizontal-vertical','Clouds_Subtract(Clouds_Subtract(*center,horizontal),vertical)'),('center+horizontal-vertical','Clouds_Subtract(Clouds_Add(*center,horizontal),vertical)')]:quad=quad.replace(expr,new)
    for field in ['r','g','b','a']:quad=quad.replace('color.'+field,'color->'+field)
    body=body[:a]+quad+body[b:]
    expressions={
    'cameraPos + cameraForward*190.0f + cameraUp*45.0f':'Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraUp,45.0f))',
    'cameraPos + cameraForward*190.0f + cameraRight*horizontal + cameraUp*vertical':'Clouds_Add(Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraRight,horizontal)),Clouds_Scale(cameraUp,vertical))',
    'cameraPos + cameraForward*190.0f + cameraRight*95.0f + cameraUp*18.0f':'Clouds_Add(Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraRight,95.0f)),Clouds_Scale(cameraUp,18.0f))',
    'cameraPos + cameraForward*190.0f + cameraUp*35.0f + cameraRight*(i*1.5f-3.75f)':'Clouds_Add(Clouds_Add(Clouds_Add(cameraPos,Clouds_Scale(cameraForward,190.0f)),Clouds_Scale(cameraUp,35.0f)),Clouds_Scale(cameraRight,i*1.5f-3.75f))',
    'moonPosition-cameraPos':'Clouds_Subtract(moonPosition,cameraPos)','starPosition-cameraPos':'Clouds_Subtract(starPosition,cameraPos)','brightStarPosition-cameraPos':'Clouds_Subtract(brightStarPosition,cameraPos)','cloudPosition-cameraPos':'Clouds_Subtract(cloudPosition,cameraPos)'}
    for old,new in expressions.items():assert old in body,old;body=body.replace(old,new)
    for name,field in [('cameraPos','Position'),('cameraRight','Right'),('cameraUp','Up'),('cameraForward','Forward')]:body=body.replace('CVector '+name+'(cameraMatrix->'+{'Position':'pos','Right':'right','Up':'up','Forward':'at'}[field]+');','CloudsVector '+name+'=Clouds_BackendMatrix'+field+'(cameraMatrix);')
    body=re.sub(r'CVector (\w+)\(([^;]+)\);',r'CloudsVector \1={\2};',body)
    body=re.sub(r'CRGBA (\w+)\(([^;]+)\);',r'CloudsColor \1=Clouds_Color(\2);',body)
    body=body.replace('CVector ','CloudsVector ').replace('RwV3d ','CloudsVector ')
    body=re.sub(r'(\w+).Normalise\(\)',r'Clouds_Normalise(&\1)',body).replace('DotProduct(','Clouds_Dot(')
    body=re.sub(r'RwV3dAdd\(&worldpos, &campos, &pos\)',r'(worldpos=Clouds_Add(campos,pos))',body)
    body=re.sub(r'StoreEnvMapCloudQuad\((\w+), cameraRight, cameraUp,([^;]+),\s+(\w+), (\w+)\);',r'StoreEnvMapCloudQuad(&\1,&cameraRight,&cameraUp,\2,&\3,&\4);',body)
    for name in ['SetPos','SetRGBA','SetU','SetV']:body=re.sub(r'RwIm3DVertex'+name+r'\(&vertices\[([^]]+)\],',r'Clouds_Backend'+name+r'(vertices,\1,',body)
    for name,state in [('ZWrite','ZWRITEENABLE'),('ZTest','ZTESTENABLE'),('VertexAlpha','VERTEXALPHAENABLE'),('Fog','FOGENABLE')]:
        for value,c in [('FALSE','false'),('TRUE','true')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','Clouds_Backend'+name+'('+c+')')
    for name,state,factor in [('SrcBlend','SRCBLEND','rwBLENDSRCALPHA'),('DestBlend','DESTBLEND','rwBLENDINVSRCALPHA')]:
        for value,c in [('rwBLENDONE','true'),(factor,'false')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','Clouds_Backend'+name+'('+c+')')
    for value,c in [('rwCULLMODECULLNONE','true'),('rwCULLMODECULLBACK','false')]:body=body.replace('RwRenderStateSet(rwRENDERSTATECULLMODE, (void*)'+value+')','Clouds_BackendCull('+c+')')
    body=body.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER,','Clouds_BackendSetRaster(')
    for old,new in [('CTxdStore::PushCurrentTxd','PushTxd'),('CTxdStore::FindTxdSlot','FindTxd'),('CTxdStore::SetCurrentTxd','SetTxd'),('CTxdStore::PopCurrentTxd','PopTxd'),('RwTextureRead','ReadTexture'),('RwTextureDestroy','DestroyTexture'),('RwTextureGetRaster','Raster'),('CGame::CanSeeOutSideFromCurrArea','Outside'),('RwCameraGetFrame','CameraFrame'),('RwFrameGetMatrix','FrameMatrix'),('RwCameraGetNearClipPlane','NearClip'),('RwCameraGetFarClipPlane','FarClip')]:body=body.replace(old+'(','Clouds_Backend'+new+'(')
    body=body.replace('TheCamera.GetPosition()','Clouds_BackendCameraPosition()').replace('TheCamera.Orientation','Clouds_BackendOrientation()')
    for name in ['Wind','Foggyness','CloudCoverage','ExtraSunnyness','Rainbow']:body=body.replace('CWeather::'+name,'Clouds_Backend'+name+'()')
    for result,name,args,code in READERS:
        if name.startswith(('LowClouds','FluffyClouds')):body=body.replace('CTimeCycle::Get'+name+'()','Clouds_Backend'+name+'()')
    body=body.replace('CTimeCycle::GetSunDirection()','Clouds_BackendSunDirection()').replace('CCoronas::SunBlockedByClouds','(*Clouds_BackendSunBlocked())').replace('CCoronas::MoonSize','Clouds_BackendMoonSize()').replace('CCoronas::SunScreenX','Clouds_BackendSunX()').replace('CCoronas::SunScreenY','Clouds_BackendSunY()')
    body=re.sub(r'gpCoronaTexture\[(\d+)\]',r'Clouds_BackendCoronaTexture(\1)',body)
    for old,new in [('CalcScreenCoors','Project'),('InitSpriteBuffer','InitSprites'),('FlushSpriteBuffer','FlushSprites')]+[(name,name) for result,name,args,code in READERS if name.startswith('Render')]:body=body.replace('CSprite::'+old+'(','Clouds_Backend'+new+'(')
    body=body.replace('SCREEN_WIDTH','Clouds_BackendScreenWidth()').replace('PUSH_RENDERGROUP(','Clouds_BackendPush(').replace('POP_RENDERGROUP()','Clouds_BackendPop()')
    for name,target in [('Sin','sinf'),('Cos','cosf'),('Sqrt','sqrtf'),('Atan2','atan2f'),('Abs','fabsf'),('Max','Clouds_Max')]:body=body.replace(name+'(',target+'(')
    body=body.replace('sq(','Clouds_Square(')
    for old,new in [('uint8','uint8_t'),('uint16','uint16_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    body=body.replace('nil','NULL')
    body=body.replace('\n{\n','\n{\n    // Preserve the complete original effect operation and live state-write order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve original conditions, arithmetic and owner call ordering\n',body,flags=re.M)
    return background.kernel('//+ rouz edit (ChatGPT)\n#include "Clouds_backend.h"\n#include "Clock.h"\n#include "Timer.h"\n#include "General.h"\n#include "RenderBuffer.h"\n#include "Draw_constants.h"\n#include <math.h>\n#include <stddef.h>\n#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n'+''.join(kind+' Clouds_'+name+';\n' for kind,name in SCALARS)+'static void *gpCloudTex[5];\n'+HELPERS+body+'//- rouz edit (ChatGPT)\n',source)

def project(source):
    # Compile complete C effects beside the remaining typed background/horizon owner
    if '..\\src\\renderer\\Clouds.c"' not in source:
        match=re.search(r'<ClCompile Include="\.\.\\src\\renderer\\Clouds\.cpp"\s*(?:/>|>[\s\S]*?</ClCompile>)',source)
        if match:
            entry=match[0];addition=entry.replace('Clouds.cpp','Clouds.c') if '<Filter>' in entry else '<ClCompile Include="..\\src\\renderer\\Clouds.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
            source=source.replace(entry,entry+'\n    '+addition)
    for name in ['Clouds_control.h','Clouds_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\Clouds.h" />','<ClInclude Include="..\\src\\renderer\\Clouds.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('Clouds.h',name))
    return background.project(source)
