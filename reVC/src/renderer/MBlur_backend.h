//+ rouz edit (ChatGPT)
#pragma once
#include "MBlur.h"
#include <stddef.h>
enum {MBLUR_STATE_TEXTUREFILTER,MBLUR_STATE_FOGENABLE,MBLUR_STATE_ZTESTENABLE,MBLUR_STATE_ZWRITEENABLE,MBLUR_STATE_TEXTURERASTER,MBLUR_STATE_VERTEXALPHAENABLE,MBLUR_STATE_SRCBLEND,MBLUR_STATE_DESTBLEND};
enum {MBLUR_BLEND_ONE,MBLUR_BLEND_SRCALPHA,MBLUR_BLEND_INVSRCALPHA,MBLUR_BLEND_DESTALPHA,MBLUR_BLEND_INVDESTALPHA};
enum {MBLUR_STENCILENABLE,MBLUR_STENCILFUNCTION,MBLUR_STENCILFUNCTIONREF,MBLUR_STENCILFUNCTIONMASK,MBLUR_STENCILFUNCTIONWRITEMASK,MBLUR_STENCILZFAIL,MBLUR_STENCILFAIL,MBLUR_STENCILPASS};
enum {MBLUR_STENCILALWAYS,MBLUR_STENCILEQUAL,MBLUR_STENCILKEEP,MBLUR_STENCILREPLACE};
enum {MBLUR_DOT,MBLUR_HEATHAZE,MBLUR_RAIN,MBLUR_DARKRAIN,MBLUR_SPLASH};
enum {MBLUR_CAMERA_TEXTURE=1,MBLUR_DONT_ALLOCATE=2};
typedef void (*MBlurVertexScopeCallback)(void *data,void *vertices);
#ifdef __cplusplus
extern "C" {
#endif
void * MBlur_HostCameraRaster(void *camera);
float MBlur_HostNearClip(void *camera);
int32_t MBlur_HostWidth(void *raster);
int32_t MBlur_HostHeight(void *raster);
int32_t MBlur_HostDepth(void *raster);
void MBlur_HostDestroyRaster(void *raster);
void MBlur_HostPushContext(void *raster);
void MBlur_HostPopContext(void);
void MBlur_HostRenderFast(void *raster,int32_t x,int32_t y);
float MBlur_HostNearScreenZ(void);
float MBlur_HostHalfPixel(void);
int32_t MBlur_HostScreenWidth(void);
int32_t MBlur_HostScreenHeight(void);
int32_t MBlur_HostVehicleHud(void);
int32_t MBlur_HostZoneHud(void);
int32_t MBlur_HostFade(void);
int32_t MBlur_HostBrightness(void);
void MBlur_HostDefinedState(void);
void MBlur_HostPostOpen(void *camera);
void MBlur_HostPostClose(void);
void MBlur_HostPostRender(void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha);
void MBlur_HostScreenX(void *vertex,float value);
void MBlur_HostScreenY(void *vertex,float value);
void MBlur_HostScreenZ(void *vertex,float value);
void MBlur_HostCameraZ(void *vertex,float value);
void MBlur_HostRecipCameraZ(void *vertex,float value);
void MBlur_HostU(void *vertex,float value,float reciprocal);
void MBlur_HostV(void *vertex,float value,float reciprocal);
void MBlur_HostIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha);
void *MBlur_HostVertices(int32_t set);
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
