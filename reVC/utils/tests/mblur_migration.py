"""Move all motion-blur, queued screen-effect and vertex policy into C."""
import re
from rect_migration import mask,closing,arguments
from mblur_schema import VARIABLES,OPERATIONS,HOST,SETTERS,RW_STATES,BLENDS,STENCIL,SDK_MAP

RENAMES={'src/renderer/MBlur.cpp':'src/renderer/MBlur.c'}

def camera_blocks(source):
    # Select the original two complete anonymous enums without inventing numeric values
    return [re.search(r'enum\n\{\n\t// TODO: find better names\n\tFADE_0,[\s\S]*?\n\};',source)[0],re.search(r'enum\n\{\n\tMOTION_BLUR_NONE = 0,[\s\S]*?\n\};',source)[0]]

def constants(source):
    # Share the original camera enum bytes with the C owner and every existing camera client
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+ '\n\n'.join(camera_blocks(source))+'\n//- rouz edit (ChatGPT)\n'

def header(original):
    # Expose complete C operations with live references to the caller's actual rectangle scalar fields
    enum=re.search(r'enum FxType\n\{[\s\S]*?\n\};',original)[0].replace('enum FxType','typedef enum FxType').replace('\n};','\n} FxType;')
    value='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n'+enum+'''
typedef struct MBlurRect {int32_t x,y,w,h;} MBlurRect;
typedef struct MBlurRectRef {int32_t *x,*y,*w,*h;} MBlurRectRef;
typedef struct MBlurColour {uint8_t red,green,blue,alpha;} MBlurColour;
static inline MBlurRectRef MBlur_RectRef(MBlurRect *rect)
{
    // Borrow the actual C rectangle fields without copying or delaying caller-visible clamping
    const MBlurRectRef value={&rect->x,&rect->y,&rect->w,&rect->h};return value;
}
#ifdef __cplusplus
extern "C" {
#endif
'''
    value+=''.join('extern '+kind+' MBlur_'+name+';\n' for name,kind in VARIABLES.items())
    value+='''int32_t MBlur_MotionBlurOpen(void *camera);
int32_t MBlur_MotionBlurClose(void);
void MBlur_CreateImmediateModeData(void *camera,MBlurRectRef rect);
void MBlur_CreateImmediateModeVertices(void *camera,MBlurRectRef rect,void *vertices,MBlurColour colour,float u1,float v1,float u2,float v2,float z,int fullTexture);
void MBlur_MotionBlurRender(void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha);
void MBlur_OverlayRender(void *camera,void *raster,MBlurColour colour,int32_t type,int32_t alpha);
void MBlur_SetDrunkBlur(float drunkness);
void MBlur_ClearDrunkBlur(void);
bool MBlur_PosInside(MBlurRectRef rect,float x1,float y1,float x2,float y2);
bool MBlur_AddRenderFx(void *camera,MBlurRectRef rect,float z,FxType type);
void MBlur_OverlayRenderFx(void *camera,void *frontBuffer);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return value

def native_header():
    # Keep SDK record borrowing typed in the remaining native clients
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "MBlur.h"
static inline MBlurRectRef MBlur_NativeRect(RwRect *rect)
{
    // Borrow the original SDK rectangle's actual integer subobjects without incompatible-record aliasing
    const MBlurRectRef value={&rect->x,&rect->y,&rect->w,&rect->h};return value;
}
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Keep SDK vertex storage opaque and expose only individual external primitives and metadata
    value='//+ rouz edit (ChatGPT)\n#pragma once\n#include "MBlur.h"\n#include <stddef.h>\n'
    value+='enum {'+','.join('MBLUR_STATE_'+name for name in RW_STATES)+'};\n'
    value+='enum {'+','.join('MBLUR_BLEND_'+name for name in BLENDS)+'};\n'
    value+='enum {'+','.join('MBLUR_'+name for name,_ in STENCIL)+'};\n'
    value+='enum {MBLUR_STENCILALWAYS,MBLUR_STENCILEQUAL,MBLUR_STENCILKEEP,MBLUR_STENCILREPLACE};\n'
    value+='enum {MBLUR_DOT,MBLUR_HEATHAZE,MBLUR_RAIN,MBLUR_DARKRAIN,MBLUR_SPLASH};\n'
    value+='enum {MBLUR_CAMERA_TEXTURE=1,MBLUR_DONT_ALLOCATE=2};\n'
    value+='typedef void (*MBlurVertexScopeCallback)(void *data,void *vertices);\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    value+=''.join(result+' MBlur_Host'+name+'('+args+');\n' for result,name,args,_ in HOST)
    for name,(args,_) in SETTERS.items():value+='void MBlur_Host'+name+'(void *vertex,'+args+');\n'
    value+='''void *MBlur_HostVertices(int32_t set);
void *MBlur_HostVertex(void *vertices,int32_t index);
void MBlur_HostWithVertices(MBlurVertexScopeCallback callback,void *data);
void MBlur_HostDraw(void *vertices,int32_t count,uint16_t *indices,int32_t indexCount);
void *MBlur_HostCreateRaster(int32_t width,int32_t height,int32_t depth,uint32_t flags);
void *MBlur_HostSubRaster(void *raster,void *parent,MBlurRect *rect);
void MBlur_HostState(int32_t state,void *value);
void MBlur_HostStencil(int32_t state,uint32_t value);
void *MBlur_HostRaster(int32_t kind,int32_t index);
void MBlur_HostPushGroup(const char *name);
void MBlur_HostPopGroup(void);
void MBlur_HostDebugMemory(uint32_t available);
void MBlur_HostDebugOpenFailure(void);
void MBlur_HostVideoInfo(uint32_t *total,uint32_t *available);
uint32_t MBlur_HostVideoMemory(void);
uint32_t MBlur_HostMaxTextureWidth(void);
uint32_t MBlur_HostMaxTextureHeight(void);
bool MBlur_HostFramePending(void);
bool MBlur_HostCapturingWorldEffects(void);
bool MBlur_HostBeginScreenTextureSampling(void);
void MBlur_HostRefraction(float left,float top,float right,float bottom,int width,int height,void *raster,float u1,float v1,float u2,float v2,int strength);
void MBlur_HostScreenTexture(float left,float top,float right,float bottom,int width,int height,void *raster,int red,int green,int blue,int alpha,bool additive);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return value

def native():
    # Keep native code limited to SDK storage, typed scalar access, external owners and a synchronous scope trampoline
    value='''//+ rouz edit (ChatGPT)
#ifndef LIBRW
#define WITHD3D
#endif
#include "common.h"
#include "main.h"
#include "MBlur_backend.h"
#include "RwHelper.h"
#include "Camera.h"
#include "Hud.h"
#include "Frontend.h"
#include "Particle.h"
#include "postfx.h"
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h"
#endif
#ifndef LIBRW
#include <d3d8caps.h>
extern "C" D3DCAPS8 _RwD3D8DeviceCaps;
extern void _GetVideoMemInfo(LPDWORD total,LPDWORD available);
extern DWORD _dwMemTotalVideo;
#endif
static RwIm2DVertex nativeVertices[2][4];
void *MBlur_HostVertices(int32_t set)
{
    // Borrow the same zero-initialized SDK vertex arrays used by the original global storage
    return nativeVertices[set];
}
void *MBlur_HostVertex(void *vertices,int32_t index)
{
    // Index actual native SDK objects without exposing their size or aliasing them as C records
    return &((RwIm2DVertex*)vertices)[index];
}
void MBlur_HostWithVertices(MBlurVertexScopeCallback callback,void *data)
{
    // Retain one actual SDK stack array per invocation including nested renderer callbacks
    RwIm2DVertex vertices[4];callback(data,vertices);
}
void MBlur_HostDraw(void *vertices,int32_t count,uint16_t *indices,int32_t indexCount)
{
    // Forward the original indexed triangle-list primitive with its actual SDK vertex storage
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST,(RwIm2DVertex*)vertices,count,indices,indexCount);
}
void *MBlur_HostCreateRaster(int32_t width,int32_t height,int32_t depth,uint32_t flags)
{
    // Translate only abstract SDK flag identities while C retains all allocation decisions
    return RwRasterCreate(width,height,depth,((flags&MBLUR_CAMERA_TEXTURE)?rwRASTERTYPECAMERATEXTURE:0)|((flags&MBLUR_DONT_ALLOCATE)?rwRASTERDONTALLOCATE:0));
}
#if defined(GTA_PS2) && !defined(EXTENDED_COLOURFILTER)
void *MBlur_HostSubRaster(void *raster,void *parent,MBlurRect *rect)
{
    // Marshal the C-owned local rectangle into the SDK primitive and preserve any returned scalar updates
    RwRect native={rect->x,rect->y,rect->w,rect->h};void *result=RwRasterSubRaster((RwRaster*)raster,(RwRaster*)parent,&native);rect->x=native.x;rect->y=native.y;rect->w=native.w;rect->h=native.h;return result;
}
#endif
void MBlur_HostState(int32_t state,void *value)
{
    // Translate SDK render-state and blend identities without making rendering choices outside C
'''
    value+='    static const RwRenderState states[]={'+','.join('rwRENDERSTATE'+name for name in RW_STATES)+'};\n'
    value+='    static const uintptr_t blends[]={'+','.join('rwBLEND'+name for name in BLENDS)+'};\n'
    value+='''    if(state==MBLUR_STATE_TEXTUREFILTER)value=(void*)rwFILTERNEAREST;
    else if(state==MBLUR_STATE_SRCBLEND||state==MBLUR_STATE_DESTBLEND)value=(void*)blends[(uintptr_t)value];
    RwRenderStateSet(states[state],value);
}
void MBlur_HostStencil(int32_t state,uint32_t value)
{
    // Translate each original backend's stencil constants and forward one native state operation
#ifdef LIBRW
'''
    value+='    static const int32_t states[]={'+','.join('rw::'+name for name,_ in STENCIL)+'};\n'
    value+='''    if(state==MBLUR_STENCILFUNCTION)value=value==MBLUR_STENCILALWAYS?rw::STENCILALWAYS:rw::STENCILEQUAL;
    else if(state==MBLUR_STENCILZFAIL||state==MBLUR_STENCILFAIL||state==MBLUR_STENCILPASS)value=value==MBLUR_STENCILKEEP?rw::STENCILKEEP:rw::STENCILREPLACE;
    rw::SetRenderState(states[state],value);
#else
'''
    value+='    static const RwUInt32 states[]={'+','.join('D3DRS_'+legacy for _,legacy in STENCIL)+'};\n'
    value+='''    if(state==MBLUR_STENCILFUNCTION)value=value==MBLUR_STENCILALWAYS?D3DCMP_ALWAYS:D3DCMP_EQUAL;
    else if(state==MBLUR_STENCILZFAIL||state==MBLUR_STENCILFAIL||state==MBLUR_STENCILPASS)value=value==MBLUR_STENCILKEEP?D3DSTENCILOP_KEEP:D3DSTENCILOP_REPLACE;
    RwD3D8SetRenderState(states[state],value);
#endif
}
void *MBlur_HostRaster(int32_t kind,int32_t index)
{
    // Borrow the original particle texture selected by the C effect policy
    switch(kind){case MBLUR_DOT:return gpDotRaster;case MBLUR_HEATHAZE:return gpHeatHazeRaster;case MBLUR_RAIN:return gpRainDripRaster[index];case MBLUR_DARKRAIN:return gpRainDripDarkRaster[index];case MBLUR_SPLASH:return gpCarSplashRaster[index];}return NULL;
}
void MBlur_HostPushGroup(const char *name)
{
    // Preserve the original debug-build rendering group primitive
    PUSH_RENDERGROUP(name);
}
void MBlur_HostPopGroup(void)
{
    // Preserve the original debug-build rendering group primitive
    POP_RENDERGROUP();
}
void MBlur_HostDebugMemory(uint32_t available)
{
    // Preserve the original debug macro's literal concatenation and available-memory argument
    debug("Available video memory %d\\n",available);
}
void MBlur_HostDebugOpenFailure(void)
{
    // Preserve the original debug macro's literal concatenation and raster-creation failure message
    debug("MBlurOpen can't create raster.");
}
#ifndef LIBRW
void MBlur_HostVideoInfo(uint32_t *total,uint32_t *available)
{
    // Marshal native DWORD outputs without aliasing them through a different C integer typedef
    DWORD nativeTotal,nativeAvailable;_GetVideoMemInfo(&nativeTotal,&nativeAvailable);*total=nativeTotal;*available=nativeAvailable;
}
uint32_t MBlur_HostVideoMemory(void)
{
    // Read the original native video-memory metadata
    return _dwMemTotalVideo;
}
uint32_t MBlur_HostMaxTextureWidth(void)
{
    // Read the original native device capability
    return _RwD3D8DeviceCaps.MaxTextureWidth;
}
uint32_t MBlur_HostMaxTextureHeight(void)
{
    // Read the original native device capability
    return _RwD3D8DeviceCaps.MaxTextureHeight;
}
#endif
#ifdef REVC_SOFTWARE_POLYGONS
'''
    for name in ['FramePending','CapturingWorldEffects','BeginScreenTextureSampling']:
        # Keep software-renderer operations in their existing external owner
        value+='bool MBlur_Host'+name+'(void)\n{\n    // Forward the original external software-renderer primitive\n    return SoftwarePolygons::'+name+'();\n}\n'
    value+='''void MBlur_HostRefraction(float left,float top,float right,float bottom,int width,int height,void *raster,float u1,float v1,float u2,float v2,int strength)
{
    // Forward one software refraction draw after C has selected geometry, offsets and strength
    SoftwarePolygons::RenderScreenRefraction(left,top,right,bottom,width,height,(RwRaster*)raster,u1,v1,u2,v2,strength);
}
void MBlur_HostScreenTexture(float left,float top,float right,float bottom,int width,int height,void *raster,int red,int green,int blue,int alpha,bool additive)
{
    // Forward one software texture draw after C has selected its tint and compositing mode
    SoftwarePolygons::RenderScreenTexture(left,top,right,bottom,width,height,(RwRaster*)raster,red,green,blue,alpha,additive);
}
#endif
'''
    for result,name,args,expression in HOST:
        # Forward individual SDK primitives and live external metadata without moving any blur policy out of C
        if name.startswith('Post'):value+='#ifdef EXTENDED_COLOURFILTER\n'
        value+=result+' MBlur_Host'+name+'('+args+')\n{\n    // Preserve the original external primitive or live owner metadata access\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
        if name.startswith('Post'):value+='#endif\n'
    for name,(args,values) in SETTERS.items():
        # Retain actual per-backend SDK vertex representation and setter behavior
        value+='void MBlur_Host'+name+'(void *vertex,'+args+')\n{\n    // Apply the original SDK vertex setter to its actual native object\n    RwIm2DVertexSet'+name+'((RwIm2DVertex*)vertex,'+values+');\n}\n'
    return value+'//- rouz edit (ChatGPT)\n'

def colour_helper():
    # Keep original channel narrowing in C instead of retaining CRGBA constructors
    return '''static MBlurColour MBlur_MakeColour(uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha)
{
    // Preserve the original constructor's four channel conversions and value result
    const MBlurColour value={red,green,blue,alpha};return value;
}
static MBlurRect MBlur_CopyRect(MBlurRectRef rect)
{
    // Copy only the current scalar values after the original caller-visible rectangle mutations
    const MBlurRect value={*rect.x,*rect.y,*rect.w,*rect.h};return value;
}
'''

def kernel(original):
    # Retain every original control-flow body while replacing class members and SDK metadata operations
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=source.replace('static RwIm2DVertex Vertex[4];\n','').replace('static RwIm2DVertex Vertex2[4];\n','')
    source=re.sub(r'extern "C" D3DCAPS8 _RwD3D8DeviceCaps;\n','',source)
    source=source.replace('extern void _GetVideoMemInfo(LPDWORD total, LPDWORD avaible);','').replace('extern DWORD _dwMemTotalVideo;','')
    source=source.replace('_RwD3D8DeviceCaps.MaxTextureWidth','MBlur_HostMaxTextureWidth()').replace('_RwD3D8DeviceCaps.MaxTextureHeight','MBlur_HostMaxTextureHeight()').replace('_dwMemTotalVideo','MBlur_HostVideoMemory()').replace('_GetVideoMemInfo(','MBlur_HostVideoInfo(')
    source=source.replace('CMBlur::CreateImmediateModeData(RwCamera *cam, RwRect *rect, RwIm2DVertex *verts,','MBlur_CreateImmediateModeVertices(RwCamera *cam, RwRect *rect, void *verts,')
    source=source.replace('RwRaster *CMBlur::pFrontBuffer;','void *MBlur_pFrontBuffer;')
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bCMBlur::',code))):source=source[:match.start()]+'MBlur_'+source[match.end():]
    for name in VARIABLES:
        # Move every static class field into equivalent zero-initialized C global storage
        source=re.sub(r'\b'+name+r'\b','MBlur_'+name,source)
    for name in OPERATIONS:
        if name=='CreateImmediateModeVertices':continue
        source=re.sub(r'(?<![\w:])'+name+r'\b','MBlur_'+name,source)
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bMBlur_CreateImmediateModeData\(',code))):
        # Give the former overloaded vertex builder a distinct C name without changing argument evaluation
        end=closing(code,match.end()-1)
        if len(arguments(source[match.end():end-1]))>2:source=source[:match.start()]+'MBlur_CreateImmediateModeVertices'+source[match.end()-1:]
    source=source.replace('RwRect *rect','MBlurRectRef rect')
    for field in ['x','y','w','h']:source=source.replace('rect->'+field,'(*rect.'+field+')')
    source=source.replace('= *rect;','= MBlur_CopyRect(rect);')
    source=re.sub(r'&(fxRect\[[^\]]+\])',r'MBlur_RectRef(&\1)',source)
    source=source.replace('MBlur_CreateImmediateModeData(cam, &rect)','MBlur_CreateImmediateModeData(cam, MBlur_RectRef(&rect))')
    for name,(_,_) in SETTERS.items():source=source.replace('RwIm2DVertexSet'+name+'(','MBlur_Host'+name+'(')
    source=re.sub(r'&(Vertex2|Vertex|verts)\[([^\]]+)\]',lambda match:'MBlur_HostVertex('+('MBlur_HostVertices(1)' if match[1]=='Vertex2' else 'MBlur_HostVertices(0)' if match[1]=='Vertex' else 'verts')+','+match[2]+')',source)
    source=re.sub(r'\bVertex2\b','MBlur_HostVertices(1)',source);source=re.sub(r'\bVertex\b','MBlur_HostVertices(0)',source)
    source=source.replace('RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST,','MBlur_HostDraw(')
    source=source.replace('RwRenderStateSet(','MBlur_HostState(')
    for name in RW_STATES:source=source.replace('rwRENDERSTATE'+name,'MBLUR_STATE_'+name)
    for name in BLENDS:source=source.replace('rwBLEND'+name,'MBLUR_BLEND_'+name)
    source=source.replace('rwFILTERNEAREST','0')
    source=source.replace('rw::SetRenderState(','MBlur_HostStencil(').replace('RwD3D8SetRenderState(','MBlur_HostStencil(')
    for name,legacy in STENCIL:source=source.replace('rw::'+name,'MBLUR_'+name).replace('D3DRS_'+legacy,'MBLUR_'+name)
    for name,legacy in [('STENCILALWAYS','D3DCMP_ALWAYS'),('STENCILEQUAL','D3DCMP_EQUAL'),('STENCILKEEP','D3DSTENCILOP_KEEP'),('STENCILREPLACE','D3DSTENCILOP_REPLACE')]:source=source.replace('rw::'+name,'MBLUR_'+name).replace(legacy,'MBLUR_'+name)
    source=source.replace('RwRasterCreate(','MBlur_HostCreateRaster(').replace('RwRasterSubRaster(','MBlur_HostSubRaster(')
    source=source.replace('rwRASTERDONTALLOCATE','MBLUR_DONT_ALLOCATE').replace('rwRASTERTYPECAMERATEXTURE','MBLUR_CAMERA_TEXTURE')
    for old,new in SDK_MAP.items():source=re.sub(r'\b'+old+r'\b','MBlur_Host'+new,source)
    for old,new in [('CPostFX::Open','MBlur_HostPostOpen'),('CPostFX::Close','MBlur_HostPostClose'),('CPostFX::Render','MBlur_HostPostRender'),('SoftwarePolygons::FramePending','MBlur_HostFramePending'),('SoftwarePolygons::CapturingWorldEffects','MBlur_HostCapturingWorldEffects'),('SoftwarePolygons::BeginScreenTextureSampling','MBlur_HostBeginScreenTextureSampling'),('SoftwarePolygons::RenderScreenRefraction','MBlur_HostRefraction'),('SoftwarePolygons::RenderScreenTexture','MBlur_HostScreenTexture')]:source=source.replace(old,new)
    for old,new in [('CHud::m_VehicleState','MBlur_HostVehicleHud()'),('CHud::m_ZoneState','MBlur_HostZoneHud()'),('TheCamera.GetScreenFadeStatus()','MBlur_HostFade()'),('FrontEndMenuManager.m_PrefsBrightness','MBlur_HostBrightness()')]:source=source.replace(old,new)
    for name,kind in [('gpCarSplashRaster','MBLUR_SPLASH'),('gpRainDripRaster','MBLUR_RAIN'),('gpRainDripDarkRaster','MBLUR_DARKRAIN')]:source=re.sub(r'\b'+name+r'\[([^\]]+)\]',r'MBlur_HostRaster('+kind+r',\1)',source)
    source=source.replace('gpDotRaster','MBlur_HostRaster(MBLUR_DOT,0)').replace('gpHeatHazeRaster','MBlur_HostRaster(MBLUR_HEATHAZE,0)')
    source=source.replace('PUSH_RENDERGROUP(','MBlur_HostPushGroup(').replace('POP_RENDERGROUP(','MBlur_HostPopGroup(')
    source=source.replace('debug("MBlurOpen can\'t create raster.");','MBlur_HostDebugOpenFailure();').replace('debug("Available video memory %d\\n", avaible);','MBlur_HostDebugMemory(avaible);')
    source=source.replace('CRGBA(','MBlur_MakeColour(').replace('Pow(','powf(')
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bint32\(',code))):
        # Preserve original functional-cast truncation of the logarithm before the power calculation
        end=closing(code,match.end()-1);source=source[:match.start()]+'(int32_t)'+source[match.end()-1:end]+source[end:]
    for old,new in [('RwBool','int32_t'),('RwCamera','void'),('RwRaster','void'),('RwRGBA','MBlurColour'),('RwRect','MBlurRect'),('RwImVertexIndex','uint16_t'),('RwUInt8','uint8_t'),('DWORD','uint32_t'),('uint8','uint8_t'),('uint32','uint32_t'),('int32','int32_t'),('nil','NULL'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('MBlur_ClearDrunkBlur()','MBlur_ClearDrunkBlur(void)')
    start=source.index('void\nMBlur_OverlayRenderFx(');tail=source[start:];tail=tail.replace('void\nMBlur_OverlayRenderFx(void *cam, void *frontBuf)','static void\nMBlur_OverlayFxInScope(void *opaque,void *vertices)',1)
    opening=tail.index('{')+1;tail=tail[:opening]+'\n    // Run the complete original effect policy while native vertex storage retains its original stack lifetime\n    MBlurFxContext *context=(MBlurFxContext*)opaque;void *cam=context->camera,*frontBuf=context->frontBuffer;\n'+tail[opening:]
    tail=tail.replace('RwIm2DVertex verts[4];','void *verts=vertices;')
    source=source[:start]+'typedef struct MBlurFxContext {void *camera,*frontBuffer;} MBlurFxContext;\n'+tail
    source+='''
void MBlur_OverlayRenderFx(void *camera,void *frontBuffer)
{
    // Borrow a distinct native SDK vertex array for each recursive hardware effect invocation
    MBlurFxContext context={camera,frontBuffer};
#ifdef REVC_SOFTWARE_POLYGONS
    MBlur_OverlayFxInScope(&context,NULL);
#else
    MBlur_HostWithVertices(MBlur_OverlayFxInScope,&context);
#endif
}
'''
    for name in OPERATIONS:
        # Describe each complete converted operation before its original control flow
        source=re.sub(r'(?m)^(MBlur_'+name+r'\([^\n]*\)\n\{\n)',lambda match:match[1]+'    // Preserve the complete original blur operation and externally visible mutation order\n',source)
    prefix='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "MBlur.h"
#include "MBlur_backend.h"
#include "Camera_effects.h"
#include "Draw.h"
#include "Draw_constants.h"
#include "General.h"
#include "Timecycle.h"
#include "Timer.h"
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#define HALFPX MBlur_HostHalfPixel()
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#ifdef GTA_PS2
#ifdef GTA_PAL
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)512)
#else
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)448)
#endif
#else
#define SCREEN_WIDTH ((float)MBlur_HostScreenWidth())
#define SCREEN_HEIGHT ((float)MBlur_HostScreenHeight())
#endif
#define SCREEN_STRETCH_X(a) ((a)*(float)SCREEN_WIDTH/DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a) ((a)*(float)SCREEN_HEIGHT/DEFAULT_SCREEN_HEIGHT)
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_BOTTOM(a) (SCREEN_HEIGHT-SCREEN_SCALE_Y(a))
#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a)*DEFAULT_ASPECT_RATIO/Draw_GetAspectRatio())
#if defined(PROPER_SCALING) && !defined(FORCE_PC_SCALING)
#undef SCREEN_SCALE_Y
#define SCREEN_SCALE_Y(a) Draw_ScaleY(SCREEN_STRETCH_Y(a))
#endif
#else
#define SCREEN_SCALE_AR(a) (a)
#endif
'''+colour_helper()
    return prefix+source+'\n//- rouz edit (ChatGPT)\n'

def mblur_calls(source):
    # Replace the entire owner, class declarations and all existing native clients
    if 'CMBlur::MotionBlurOpen(RwCamera *cam)' in source:return kernel(source)
    if 'class CMBlur\n' in source:return header(source)
    if 'class CCamera' in source and '\n\tFADE_0,' in source and '\n\tMOTION_BLUR_NONE = 0,' in source:
        blocks=camera_blocks(source);source=source.replace(blocks[0],'#include "Camera_effects.h"').replace(blocks[1],'');return '//+ rouz edit (ChatGPT)\n'+source+'//- rouz edit (ChatGPT)\n'
    original=source;code=mask(source);changes=[];needsRect=False
    for match in re.finditer(r'\bCMBlur::(\w+)',code):
        # Pass borrowed rectangle fields once at each remaining typed SDK client call
        name=match[1];assert name in VARIABLES or name in OPERATIONS,name;end=match.end();value='MBlur_'+name
        if name in ['AddRenderFx','PosInside','CreateImmediateModeData']:
            opening=match.end()
            while code[opening].isspace():opening+=1
            assert code[opening]=='('
            end=closing(code,opening);args=arguments(source[opening+1:end-1]);args[1 if name!='PosInside' else 0]='MBlur_NativeRect('+args[1 if name!='PosInside' else 0]+')';needsRect=True
            if name=='CreateImmediateModeData' and len(args)>2:value='MBlur_CreateImmediateModeVertices'
            value+='('+','.join(args)+')'
        changes.append((match.start(),end,value))
    for start,end,value in reversed(changes):source=source[:start]+value+source[end:]
    if needsRect:
        assert '#include "MBlur.h"' in source;source=source.replace('#include "MBlur.h"','#include "MBlur_native.h"')
    if source!=original:
        lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True)
        for index in reversed(range(len(lines))):
            if re.search(r'\bMBlur_\w+',code[index]):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Use the complete C motion-blur owner and its live shared state\n')
        return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'
    return source

def mblur_project(source):
    # Compile every blur policy operation as C11 and retain only SDK primitives in the native unit
    import object_names_migration as objects
    source=source.replace('src\\renderer\\MBlur.cpp','src\\renderer\\MBlur.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\renderer\\MBlur\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve prior metadata and require the actual C compiler for the whole motion-blur owner
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';native=match[1][:-len('MBlur.c')]+'MBlur_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Retain all preceding C blur policy before the complete C post-processing successor
    import postfx_migration as post
    return post.calls(mblur_calls(source))

def project(source):
    # Select C post-processing policy and forwarding after all preceding project metadata
    import postfx_migration as post
    return post.project(mblur_project(source))
