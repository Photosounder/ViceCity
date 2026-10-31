"""Move the complete scrolling-sign record, messages, glyphs and rendering into C11."""
import re
from audio_controls_migration import function
import escalators_migration as escalators
FIELDS=['m_Counter','m_pMessage','m_Position','m_MessageCurrentChar','m_MessageLength','m_Size','m_fIntensity','m_MessageBar','m_Type','m_bVisible','m_uRed','m_uGreen','m_uBlue','m_fScale']
TYPES='''#define NUMSCROLLBARS 1
typedef struct ScrollBarVector { float x,y,z; } ScrollBarVector;
typedef struct ScrollBar {
    uint8_t m_Counter;
    const char *m_pMessage;
    ScrollBarVector m_Position;
    uint32_t m_MessageCurrentChar,m_MessageLength;
    ScrollBarVector m_Size;
    float m_fIntensity;
    uint8_t m_MessageBar[40];
    uint8_t m_Type;
    bool m_bVisible;
    uint8_t m_uRed,m_uGreen,m_uBlue;
    float m_fScale;
} ScrollBar;
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
'''+TYPES+'''#ifdef __cplusplus
extern "C" {
#endif
extern ScrollBar aScrollBars[NUMSCROLLBARS];
extern int32_t ScrollBar_TonightsEvent;
void ScrollBar_Init(ScrollBar *bar,ScrollBarVector first,ScrollBarVector last,uint8_t type,uint8_t r,uint8_t g,uint8_t b,float scale);
void ScrollBar_Update(ScrollBar *bar);
void ScrollBar_Render(ScrollBar *bar);
void ScrollBar_SetVisibility(ScrollBar *bar,bool visible);
bool ScrollBar_IsVisible(ScrollBar *bar);
#ifdef REVC_SOFTWARE_POLYGONS
void ScrollBar_RenderForEnvMap(ScrollBar *bar,void *camera);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
DESCRIPTORS='''typedef struct ScrollBarPoint { void *owner;float *x,*y,*z; } ScrollBarPoint;
typedef struct ScrollBarPoints { ScrollBarPoint input,output; } ScrollBarPoints;
typedef struct ScrollBarBasis { ScrollBarVector position,right,up,forward; } ScrollBarBasis;
typedef void (*ScrollBarPointsCallback)(ScrollBar *,ScrollBarPoints,uint8_t,uint8_t,uint8_t);
'''
READERS=[
('ScrollBarVector','CameraPosition','void','const CVector &v=TheCamera.GetPosition();ScrollBarVector result={v.x,v.y,v.z};return result;'),
('bool','SphereVisible','ScrollBarVector value,float radius','return TheCamera.IsSphereVisible(CVector(value.x,value.y,value.z),radius);'),
('void','InitSprites','void','CSprite::InitSpriteBuffer();'),('void','FlushSprites','void','CSprite::FlushSpriteBuffer();'),
('void','WithPoints','ScrollBar *bar,uint8_t r,uint8_t g,uint8_t b,ScrollBarPointsCallback callback','CVector input,output;ScrollBarPoints points={{&input,&input.x,&input.y,&input.z},{&output,&output.x,&output.y,&output.z}};callback(bar,points,r,g,b);'),
('bool','Project','void *input,void *output,float *width,float *height,bool farclip','return CSprite::CalcScreenCoors(*(CVector *)input,(CVector *)output,width,height,farclip);'),
('void','Sprite','float x,float y,float z,float width,float height,uint8_t r,uint8_t g,uint8_t b,int16_t intensity,float reciprocal,uint8_t alpha','CSprite::RenderBufferedOneXLUSprite(x,y,z,width,height,r,g,b,intensity,reciprocal,alpha);'),
('void *','Texture','void','return gpCoronaTexture[0];'),('void *','Raster','void *texture','return RwTextureGetRaster((RwTexture *)texture);'),
('void','SetRaster','void *raster','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);'),
('void','ZWrite','bool enabled','RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);'),
('void','ZTest','bool enabled','RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void *)(uintptr_t)enabled);'),
('void','VertexAlpha','bool enabled','RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);'),
('void','SrcBlend','bool additive','RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)(additive?rwBLENDONE:rwBLENDSRCALPHA));'),
('void','DestBlend','bool additive','RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)(additive?rwBLENDONE:rwBLENDINVSRCALPHA));'),
('void','Linear','void','RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,(void *)rwFILTERLINEAR);'),
('void','Fog','bool enabled','RwRenderStateSet(rwRENDERSTATEFOGENABLE,(void *)(uintptr_t)enabled);'),
('void','Cull','bool none','RwRenderStateSet(rwRENDERSTATECULLMODE,(void *)(none?rwCULLMODECULLNONE:rwCULLMODECULLBACK));'),
('void','Push','const char *name','PUSH_RENDERGROUP(name);'),('void','Pop','void','POP_RENDERGROUP();'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);'),('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);'),
('ScrollBarBasis','Basis','void *matrix','const RwMatrix &m=*(RwMatrix *)matrix;ScrollBarBasis result={{m.pos.x,m.pos.y,m.pos.z},{m.right.x,m.right.y,m.right.z},{m.up.x,m.up.y,m.up.z},{m.at.x,m.at.y,m.at.z}};return result;'),
('float','NearClip','void *camera','return RwCameraGetNearClipPlane((RwCamera *)camera);'),('float','FarClip','void *camera','return RwCameraGetFarClipPlane((RwCamera *)camera);')]
for name,args,code in [('Pos','float x,float y,float z','RwIm3DVertexSetPos(&v[index],x,y,z);'),('RGBA','uint8_t r,uint8_t g,uint8_t b,uint8_t a','RwIm3DVertexSetRGBA(&v[index],r,g,b,a);'),('U','float u','RwIm3DVertexSetU(&v[index],u);'),('V','float value','RwIm3DVertexSetV(&v[index],value);')]:READERS.append(('void','Set'+name,'void *vertices,int32_t index,'+args,'RwIm3DVertex *v=(RwIm3DVertex *)vertices;'+code))
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "ScrollBar.h"
'''+DESCRIPTORS+'''#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' ScrollBar_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='//+ rouz edit (ChatGPT)\n#include "ScrollBar_backend.h"\n'+''.join(result+' ScrollBar_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed owner operation and original native object lifetime\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''static ScrollBarVector ScrollBar_Add(ScrollBarVector a,ScrollBarVector b)
{
    // Preserve each original component addition and intermediate vector value
    ScrollBarVector value={a.x+b.x,a.y+b.y,a.z+b.z};return value;
}
static ScrollBarVector ScrollBar_Subtract(ScrollBarVector a,ScrollBarVector b)
{
    // Preserve each original component subtraction and intermediate vector value
    ScrollBarVector value={a.x-b.x,a.y-b.y,a.z-b.z};return value;
}
static ScrollBarVector ScrollBar_Scale(ScrollBarVector a,float factor)
{
    // Retain the original per-component multiplication before corner composition
    ScrollBarVector value={a.x*factor,a.y*factor,a.z*factor};return value;
}
static float ScrollBar_Dot(ScrollBarVector a,ScrollBarVector b)
{
    // Preserve the original left-associated component-product sum
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float ScrollBar_Magnitude(ScrollBarVector a)
{
    // Keep the exact original square-root argument and arithmetic order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
static void ScrollBar_Normalise(ScrollBarVector *a)
{
    // Retain the original normalizer including its x-only zero fallback
    float sq=ScrollBar_Dot(*a,*a);if(sq>0.0f){
        // Multiply each declared component by the original reciprocal square root
        float inv=1.0f/sqrtf(sq);a->x*=inv;a->y*=inv;a->z*=inv;
    }else a->x=1.0f;
}
static void ScrollBar_RenderPixels(ScrollBar *bar,ScrollBarPoints points,uint8_t r,uint8_t g,uint8_t b);
'''
SCROLL_BACKENDS=['ScrollCount','ScrollInit','ScrollVisibility','ScrollVisible','ScrollUpdate','ScrollRender','ScrollEnv']

def method(source,name):
    # Extract each unchanged original method with either opening-brace style
    match=re.search(r'(?m)^void\s+CScrollBar::'+name+r'\([^\n]*\)\s*\{',source);assert match,name
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]

def charset(source):
    # Include all 295 original mutable glyph bytes and every original initializer entry
    a=source.index('uint8 ScrollCharSet[59][5]');b=source.index('\n};',a)+len('\n};');return source[a:b]

def classes(source):
    # Delimit the complete original record including its inline visibility methods
    a=source.index('class CScrollBar\n');b=source.index('\n};',a)+len('\n};');return source[a:b]

def calls(source):
    # Remove the sign algorithms and storage while retaining all unrelated Fluff code
    if 'uint8 ScrollCharSet[59][5]' in source:
        source=source.replace(charset(source),'').replace('CScrollBar    aScrollBars[1];','').replace('int32 CScrollBar::TonightsEvent;','')
        source=source.replace('char String_Time[] = "THE TIME IS 12:34    ";','').replace(function(source,'','FindTimeMessage'),'')
        for name in ['Init','Update','Render','RenderForEnvMap']:source=source.replace(method(source,name),'')
        source+='\n'+HOST
    if 'class CScrollBar\n' in source:source=source.replace(classes(source),'//+ rouz edit (ChatGPT)\n#include "ScrollBar.h"\n//- rouz edit (ChatGPT)')
    for name in SCROLL_BACKENDS:
        if not re.search(r'MovingThings_Backend'+name+r'\([^\n]*\)\n\{',source):continue
        source=source.replace(function(source,'MovingThings_Backend',name),'')
    source=re.sub(r'^.*\bMovingThings_Backend(?:'+'|'.join(SCROLL_BACKENDS)+r')\([^\n]*\);\n','',source,flags=re.M) if 'extern "C"' in source and 'void MovingThings_BackendEscalatorsInit(void);' in source else source
    if 'MovingThings_BackendScrollCount()' in source:
        source=source.replace('#include "MovingThings_backend.h"','#include "MovingThings_backend.h"\n#include "ScrollBar.h"')
        source=source.replace('MovingThings_BackendScrollCount()','NUMSCROLLBARS')
        source=source.replace('MovingThings_BackendScrollInit(0,(MovingThingVector){-1069.209f,1320.126f,18.848f},(MovingThingVector){-1069.209f,1342.299f,22.612f},0,128,255,0,0.3f)','ScrollBar_Init(&aScrollBars[0],(ScrollBarVector){-1069.209f,1320.126f,18.848f},(ScrollBarVector){-1069.209f,1342.299f,22.612f},0,128,255,0,0.3f)')
        source=source.replace('MovingThings_BackendScrollVisibility(0,false)','ScrollBar_SetVisibility(&aScrollBars[0],false)')
        for old,new in [('ScrollVisible','IsVisible'),('ScrollUpdate','Update'),('ScrollRender','Render')]:source=source.replace('MovingThings_Backend'+old+'(i)','ScrollBar_'+new+'(&aScrollBars[i])')
        source=source.replace('MovingThings_BackendScrollEnv(i,camera)','ScrollBar_RenderForEnvMap(&aScrollBars[i],camera)')
    lines=[]
    for line in source.splitlines(keepends=True):
        if 'CScrollBar::TonightsEvent' in line and not line.lstrip().startswith('//'):lines.append(re.match(r'\s*',line)[0]+'// Publish the stadium event to the canonical C scrolling-sign state\n')
        lines.append(line.replace('CScrollBar::TonightsEvent','ScrollBar_TonightsEvent'))
    return escalators.calls(''.join(lines))

def kernel(source):
    # Translate every original message, bitmap, main-render and reflection-render statement
    init=method(source,'Init').replace('CScrollBar::Init(CVector pos1, CVector pos2, uint8 type, uint8 red, uint8 green, uint8 blue, float scale)','ScrollBar_Init(ScrollBar *bar,ScrollBarVector pos1,ScrollBarVector pos2,uint8_t type,uint8_t red,uint8_t green,uint8_t blue,float scale)')
    update=method(source,'Update').replace('CScrollBar::Update()','ScrollBar_Update(ScrollBar *bar)')
    render=method(source,'Render');a=render.index('\tCVector coronaCoord, screenCoord;');setup=render[:a];pixels=render[a:]
    setup=setup.replace('CScrollBar::Render()','ScrollBar_Render(ScrollBar *bar)')+'    // Keep both actual native projection vectors alive throughout the pixel loop\n    ScrollBar_BackendWithPoints(bar,r,g,b,ScrollBar_RenderPixels);\n}'
    pixels='static void\nScrollBar_RenderPixels(ScrollBar *bar,ScrollBarPoints points,uint8_t r,uint8_t g,uint8_t b)\n{\n'+pixels.replace('\tCVector coronaCoord, screenCoord;\n','')
    for name,point in [('coronaCoord','input'),('screenCoord','output')]:pixels=re.sub(r'\b'+name+r'\.([xyz])',lambda match:'(*points.'+point+'.'+match[1]+')',pixels)
    pixels=pixels.replace('CSprite::CalcScreenCoors(coronaCoord, &screenCoord,','ScrollBar_BackendProject(points.input.owner,points.output.owner,')
    env=method(source,'RenderForEnvMap').replace('CScrollBar::RenderForEnvMap(RwCamera *camera)','ScrollBar_RenderForEnvMap(ScrollBar *bar,void *camera)')
    env=env.replace('RwFrame *cameraFrame','void *cameraFrame').replace('RwMatrix *cameraMatrix','void *cameraMatrix')
    a=env.index('\tCVector cameraPos(');b=env.index('\tcameraRight.Normalise();',a)
    env=env[:a]+'    ScrollBarBasis basis=ScrollBar_BackendBasis(cameraMatrix);\n    ScrollBarVector cameraPos=basis.position,cameraRight=basis.right,cameraUp=basis.up,cameraForward=basis.forward;\n'+env[b:]
    for name in ['cameraRight','cameraUp','cameraForward']:env=env.replace(name+'.Normalise()','ScrollBar_Normalise(&'+name+')')
    env=env.replace('(cameraPos - m_Position).Magnitude()','ScrollBar_Magnitude(ScrollBar_Subtract(cameraPos,m_Position))')
    env=env.replace('CVector pixelPosition(m_Position.x + m_Size.x*i,\n\t\t\t\tm_Position.y + m_Size.y*i,\n\t\t\t\tm_Position.z + m_Size.z*j);','ScrollBarVector pixelPosition={m_Position.x+m_Size.x*i,m_Position.y+m_Size.y*i,m_Position.z+m_Size.z*j};')
    env=env.replace('DotProduct(pixelPosition-cameraPos, cameraForward)','ScrollBar_Dot(ScrollBar_Subtract(pixelPosition,cameraPos),cameraForward)')
    env=env.replace('RwImVertexIndex *indices;','uint16_t *indices;').replace('RwIm3DVertex *vertices;','void *vertices;').replace('RenderBuffer_StartStoringLegacy(6, 4, &indices, &vertices)','RenderBuffer_StartStoring(6,4,&indices,&vertices)')
    env=env.replace('CVector horizontal = cameraRight*pixelScale','ScrollBarVector horizontal=ScrollBar_Scale(cameraRight,pixelScale)').replace('CVector vertical = cameraUp*pixelScale','ScrollBarVector vertical=ScrollBar_Scale(cameraUp,pixelScale)').replace('CVector corners[4]','ScrollBarVector corners[4]')
    for old,new in [('pixelPosition-horizontal+vertical','ScrollBar_Add(ScrollBar_Subtract(pixelPosition,horizontal),vertical)'),('pixelPosition+horizontal+vertical','ScrollBar_Add(ScrollBar_Add(pixelPosition,horizontal),vertical)'),('pixelPosition-horizontal-vertical','ScrollBar_Subtract(ScrollBar_Subtract(pixelPosition,horizontal),vertical)'),('pixelPosition+horizontal-vertical','ScrollBar_Subtract(ScrollBar_Add(pixelPosition,horizontal),vertical)')]:env=env.replace(old,new)
    for name in ['Pos','RGBA','U','V']:env=re.sub(r'RwIm3DVertexSet'+name+r'\(&vertices\[([^\]]+)\],',lambda match:'ScrollBar_BackendSet'+name+'(vertices,'+match[1]+',',env)
    time=function(source,'','FindTimeMessage').replace('FindTimeMessage()','ScrollBar_FindTimeMessage(void)');time='static '+time
    body=time+'\n'+init+'\n'+update+'\n'+setup+'\n'+pixels+'\n#ifdef REVC_SOFTWARE_POLYGONS\n'+env+'\n#endif\n'
    body=body.replace('FindTimeMessage()','ScrollBar_FindTimeMessage()').replace('switch (TonightsEvent)','switch (ScrollBar_TonightsEvent)')
    body=body.replace('(TheCamera.GetPosition() - m_Position).Magnitude()','ScrollBar_Magnitude(ScrollBar_Subtract(ScrollBar_BackendCameraPosition(),m_Position))').replace('TheCamera.IsSphereVisible(','ScrollBar_BackendSphereVisible(')
    for field in FIELDS:body=re.sub(r'\b'+field+r'\b','bar->'+field,body)
    for old,new in [('CSprite::InitSpriteBuffer','InitSprites'),('CSprite::FlushSpriteBuffer','FlushSprites'),('CSprite::RenderBufferedOneXLUSprite','Sprite'),('RwTextureGetRaster','Raster'),('RwCameraGetFrame','CameraFrame'),('RwFrameGetMatrix','FrameMatrix'),('RwCameraGetNearClipPlane','NearClip'),('RwCameraGetFarClipPlane','FarClip')]:body=body.replace(old+'(','ScrollBar_Backend'+new+'(')
    body=body.replace('gpCoronaTexture[0]','ScrollBar_BackendTexture()')
    for state,name in [('ZWRITEENABLE','ZWrite'),('ZTESTENABLE','ZTest'),('VERTEXALPHAENABLE','VertexAlpha'),('FOGENABLE','Fog')]:
        body=re.sub(r'RwRenderStateSet\(rwRENDERSTATE'+state+r',\s*\(void\*\)(TRUE|FALSE)\)',lambda match:'ScrollBar_Backend'+name+'('+('true' if match[1]=='TRUE' else 'false')+')',body)
    for state,name in [('SRCBLEND','SrcBlend'),('DESTBLEND','DestBlend')]:body=re.sub(r'RwRenderStateSet\(rwRENDERSTATE'+state+r',\s*\(void\*\)(rwBLENDONE|rwBLENDSRCALPHA|rwBLENDINVSRCALPHA)\)',lambda match:'ScrollBar_Backend'+name+'('+('true' if match[1]=='rwBLENDONE' else 'false')+')',body)
    body=re.sub(r'RwRenderStateSet\(rwRENDERSTATETEXTUREFILTER,\s*\(void\*\)rwFILTERLINEAR\)','ScrollBar_BackendLinear()',body)
    body=re.sub(r'RwRenderStateSet\(rwRENDERSTATECULLMODE,\s*\(void\*\)(rwCULLMODECULLNONE|rwCULLMODECULLBACK)\)',lambda match:'ScrollBar_BackendCull('+('true' if match[1]=='rwCULLMODECULLNONE' else 'false')+')',body)
    body=re.sub(r'RwRenderStateSet\(rwRENDERSTATETEXTURERASTER,\s*','ScrollBar_BackendSetRaster(',body).replace('PUSH_RENDERGROUP(','ScrollBar_BackendPush(').replace('POP_RENDERGROUP()','ScrollBar_BackendPop()').replace('nil','NULL')
    for old,new in [('uint8','uint8_t'),('uint32','uint32_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    body=re.sub(r'(?m)^((?:static )?(?:void|const char\*)\s+(?:ScrollBar_)\w+\([^\n]*\)\s*\{\n)',r'\1    // Preserve original live reads, pointer identity and complete state-write ordering\n',body)
    body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original branch and traversal order\n',body,flags=re.M)
    code=re.sub(r'//[^\n]*|"[^"\n]*"','',body);assert not re.search(r'\b(?:CVector|CScrollBar|CSprite|TheCamera|RwIm3DVertex|RwFrame|RwMatrix|RwImVertexIndex)\b',code)
    prefix='//+ rouz edit (ChatGPT)\n#include "ScrollBar_backend.h"\n#include "General.h"\n#include "Clock.h"\n#include "RenderBuffer.h"\n#include <math.h>\n#include <string.h>\n#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))\n#define ABS(a) (((a)<0)?(-(a)):(a))\nenum { SCROLL_ARENA_STRING=0 };\nScrollBar aScrollBars[NUMSCROLLBARS];\nint32_t ScrollBar_TonightsEvent;\nstatic char String_Time[]="THE TIME IS 12:34    ";\n'
    tail='''void ScrollBar_SetVisibility(ScrollBar *bar,bool visible)
{
    // Preserve the original direct visibility field assignment
    bar->m_bVisible=visible;
}
bool ScrollBar_IsVisible(ScrollBar *bar)
{
    // Return the same live flag used by the original inline getter
    return bar->m_bVisible;
}
//- rouz edit (ChatGPT)
'''
    return prefix+charset(source).replace('uint8 ScrollCharSet','static uint8_t ScrollCharSet')+'\n'+HELPERS+body+tail

def project(source):
    # Add a uniquely named C11 unit without changing existing object-name disambiguation
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\ScrollBar.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\ScrollBar.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['ScrollBar.h','ScrollBar_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return escalators.project(source)
