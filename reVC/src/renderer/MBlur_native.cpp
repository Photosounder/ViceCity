//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
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
    static const RwRenderState states[]={rwRENDERSTATETEXTUREFILTER,rwRENDERSTATEFOGENABLE,rwRENDERSTATEZTESTENABLE,rwRENDERSTATEZWRITEENABLE,rwRENDERSTATETEXTURERASTER,rwRENDERSTATEVERTEXALPHAENABLE,rwRENDERSTATESRCBLEND,rwRENDERSTATEDESTBLEND};
    static const uintptr_t blends[]={rwBLENDONE,rwBLENDSRCALPHA,rwBLENDINVSRCALPHA,rwBLENDDESTALPHA,rwBLENDINVDESTALPHA};
    if(state==MBLUR_STATE_TEXTUREFILTER)value=(void*)rwFILTERNEAREST;
    else if(state==MBLUR_STATE_SRCBLEND||state==MBLUR_STATE_DESTBLEND)value=(void*)blends[(uintptr_t)value];
    RwRenderStateSet(states[state],value);
}
void MBlur_HostStencil(int32_t state,uint32_t value)
{
    // Translate each original backend's stencil constants and forward one native state operation
#ifdef LIBRW
    static const int32_t states[]={rw::STENCILENABLE,rw::STENCILFUNCTION,rw::STENCILFUNCTIONREF,rw::STENCILFUNCTIONMASK,rw::STENCILFUNCTIONWRITEMASK,rw::STENCILZFAIL,rw::STENCILFAIL,rw::STENCILPASS};
    if(state==MBLUR_STENCILFUNCTION)value=value==MBLUR_STENCILALWAYS?rw::STENCILALWAYS:rw::STENCILEQUAL;
    else if(state==MBLUR_STENCILZFAIL||state==MBLUR_STENCILFAIL||state==MBLUR_STENCILPASS)value=value==MBLUR_STENCILKEEP?rw::STENCILKEEP:rw::STENCILREPLACE;
    rw::SetRenderState(states[state],value);
#else
    static const RwUInt32 states[]={D3DRS_STENCILENABLE,D3DRS_STENCILFUNC,D3DRS_STENCILREF,D3DRS_STENCILMASK,D3DRS_STENCILWRITEMASK,D3DRS_STENCILZFAIL,D3DRS_STENCILFAIL,D3DRS_STENCILPASS};
    if(state==MBLUR_STENCILFUNCTION)value=value==MBLUR_STENCILALWAYS?D3DCMP_ALWAYS:D3DCMP_EQUAL;
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
    debug("Available video memory %d\n",available);
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
bool MBlur_HostFramePending(void)
{
    // Forward the original external software-renderer primitive
    return SoftwarePolygons::FramePending();
}
bool MBlur_HostCapturingWorldEffects(void)
{
    // Forward the original external software-renderer primitive
    return SoftwarePolygons::CapturingWorldEffects();
}
bool MBlur_HostBeginScreenTextureSampling(void)
{
    // Forward the original external software-renderer primitive
    return SoftwarePolygons::BeginScreenTextureSampling();
}
void MBlur_HostRefraction(float left,float top,float right,float bottom,int width,int height,void *raster,float u1,float v1,float u2,float v2,int strength)
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
void * MBlur_HostCameraRaster(void *camera)
{
    // Preserve the original external primitive or live owner metadata access
    return RwCameraGetRaster((RwCamera*)camera);
}
float MBlur_HostNearClip(void *camera)
{
    // Preserve the original external primitive or live owner metadata access
    return RwCameraGetNearClipPlane((RwCamera*)camera);
}
int32_t MBlur_HostWidth(void *raster)
{
    // Preserve the original external primitive or live owner metadata access
    return RwRasterGetWidth((RwRaster*)raster);
}
int32_t MBlur_HostHeight(void *raster)
{
    // Preserve the original external primitive or live owner metadata access
    return RwRasterGetHeight((RwRaster*)raster);
}
int32_t MBlur_HostDepth(void *raster)
{
    // Preserve the original external primitive or live owner metadata access
    return RwRasterGetDepth((RwRaster*)raster);
}
void MBlur_HostDestroyRaster(void *raster)
{
    // Preserve the original external primitive or live owner metadata access
    RwRasterDestroy((RwRaster*)raster);
}
void MBlur_HostPushContext(void *raster)
{
    // Preserve the original external primitive or live owner metadata access
    RwRasterPushContext((RwRaster*)raster);
}
void MBlur_HostPopContext(void)
{
    // Preserve the original external primitive or live owner metadata access
    RwRasterPopContext();
}
void MBlur_HostRenderFast(void *raster,int32_t x,int32_t y)
{
    // Preserve the original external primitive or live owner metadata access
    RwRasterRenderFast((RwRaster*)raster,x,y);
}
float MBlur_HostNearScreenZ(void)
{
    // Preserve the original external primitive or live owner metadata access
    return RwIm2DGetNearScreenZ();
}
float MBlur_HostHalfPixel(void)
{
    // Preserve the original external primitive or live owner metadata access
    return HALFPX;
}
int32_t MBlur_HostScreenWidth(void)
{
    // Preserve the original external primitive or live owner metadata access
    return RsGlobal.width;
}
int32_t MBlur_HostScreenHeight(void)
{
    // Preserve the original external primitive or live owner metadata access
    return RsGlobal.height;
}
int32_t MBlur_HostVehicleHud(void)
{
    // Preserve the original external primitive or live owner metadata access
    return CHud::m_VehicleState;
}
int32_t MBlur_HostZoneHud(void)
{
    // Preserve the original external primitive or live owner metadata access
    return CHud::m_ZoneState;
}
int32_t MBlur_HostFade(void)
{
    // Preserve the original external primitive or live owner metadata access
    return TheCamera.GetScreenFadeStatus();
}
int32_t MBlur_HostBrightness(void)
{
    // Preserve the original external primitive or live owner metadata access
    return FrontEndMenuManager.m_PrefsBrightness;
}
void MBlur_HostDefinedState(void)
{
    // Preserve the original external primitive or live owner metadata access
    DefinedState();
}
#ifdef EXTENDED_COLOURFILTER

#endif
#ifdef EXTENDED_COLOURFILTER

#endif
#ifdef EXTENDED_COLOURFILTER

#endif
void MBlur_HostScreenX(void *vertex,float value)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetScreenX((RwIm2DVertex*)vertex,value);
}
void MBlur_HostScreenY(void *vertex,float value)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetScreenY((RwIm2DVertex*)vertex,value);
}
void MBlur_HostScreenZ(void *vertex,float value)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetScreenZ((RwIm2DVertex*)vertex,value);
}
void MBlur_HostCameraZ(void *vertex,float value)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetCameraZ((RwIm2DVertex*)vertex,value);
}
void MBlur_HostRecipCameraZ(void *vertex,float value)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetRecipCameraZ((RwIm2DVertex*)vertex,value);
}
void MBlur_HostU(void *vertex,float value,float reciprocal)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetU((RwIm2DVertex*)vertex,value,reciprocal);
}
void MBlur_HostV(void *vertex,float value,float reciprocal)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetV((RwIm2DVertex*)vertex,value,reciprocal);
}
void MBlur_HostIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha)
{
    // Apply the original SDK vertex setter to its actual native object
    RwIm2DVertexSetIntRGBA((RwIm2DVertex*)vertex,red,green,blue,alpha);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
