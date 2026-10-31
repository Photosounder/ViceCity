//+ rouz edit (ChatGPT)
#pragma once
#include "postfx.h"
#include <stddef.h>
enum {POSTFX_STATE_TEXTUREFILTER,POSTFX_STATE_FOGENABLE,POSTFX_STATE_ZTESTENABLE,POSTFX_STATE_ZWRITEENABLE,POSTFX_STATE_TEXTURERASTER,POSTFX_STATE_VERTEXALPHAENABLE,POSTFX_STATE_SRCBLEND,POSTFX_STATE_DESTBLEND};
enum {POSTFX_BLEND_ONE,POSTFX_BLEND_SRCALPHA,POSTFX_BLEND_INVSRCALPHA,POSTFX_BLEND_DESTALPHA,POSTFX_BLEND_INVDESTALPHA};
enum {POSTFX_SHADER_DECL,POSTFX_VERTEX_HEADER,POSTFX_FRAGMENT_HEADER};
#ifdef __cplusplus
extern "C" {
#endif
void * PostFX_HostCameraRaster(void *camera);
float PostFX_HostNearClip(void *camera);
int32_t PostFX_HostWidth(void *raster);
int32_t PostFX_HostHeight(void *raster);
int32_t PostFX_HostDepth(void *raster);
void PostFX_HostDestroyRaster(void *raster);
void PostFX_HostPushContext(void *raster);
void PostFX_HostPopContext(void);
void PostFX_HostRenderFast(void *raster,int32_t x,int32_t y);
float PostFX_HostNearScreenZ(void);
float PostFX_HostHalfPixel(void);
void PostFX_HostDefinedState(void);
void PostFX_HostScreenX(void *vertex,float value);
void PostFX_HostScreenY(void *vertex,float value);
void PostFX_HostScreenZ(void *vertex,float value);
void PostFX_HostCameraZ(void *vertex,float value);
void PostFX_HostRecipCameraZ(void *vertex,float value);
void PostFX_HostU(void *vertex,float value,float reciprocal);
void PostFX_HostV(void *vertex,float value,float reciprocal);
void PostFX_HostIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha);
void *PostFX_HostVertices(int32_t set);
void *PostFX_HostVertex(void *vertices,int32_t index);
void PostFX_HostDraw(void *vertices,int32_t count,uint16_t *indices,int32_t indexCount);
void *PostFX_HostCreateRaster(int32_t width,int32_t height,int32_t depth);
void PostFX_HostState(int32_t state,void *value);
void PostFX_HostPushGroup(const char *name);
void PostFX_HostPopGroup(void);
void *PostFX_HostD3DCreate(void *bytes);
void PostFX_HostD3DDestroy(void *shader);
void PostFX_HostD3DConstants(uint32_t first,const float *values,uint32_t count);
void PostFX_HostD3DOverride(void *shader);
int32_t PostFX_HostGLRegister(const char *name);
const char *PostFX_HostGLText(int32_t kind);
void *PostFX_HostGLCreate(const char **vertices,const char **fragments);
void PostFX_HostGLDestroy(void *shader);
void PostFX_HostGLUse(void *shader);
void PostFX_HostGLOverride(void *shader);
int32_t PostFX_HostGLLocation(void *shader,int32_t uniform);
void PostFX_HostGLUniform3(int32_t location,int32_t count,const float *values);
void PostFX_HostGLUniform4(int32_t location,int32_t count,const float *values);
void re3_assert(const char *expression,const char *filename,unsigned int line,const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
