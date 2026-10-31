//+ rouz edit (ChatGPT)
#pragma once
#include "PlaneTrails.h"
typedef struct PlaneTrailsPosition { void *owner;float *x,*y,*z; } PlaneTrailsPosition;
typedef void (*PlaneTrailsPositionCallback)(PlaneTrailsPosition position);
#ifdef __cplusplus
extern "C" {
#endif
PlaneTrailVector PlaneTrails_BackendVector(const void *position);
void PlaneTrails_BackendWithPosition(PlaneTrailsPositionCallback callback);
bool PlaneTrails_BackendRenderingEnvMap(void);
bool PlaneTrails_BackendSphereVisible(PlaneTrailVector position,float radius);
void PlaneTrails_BackendSetRGBA(int32_t index,uint8_t alpha);
void PlaneTrails_BackendSetPos(int32_t index,float x,float y,float z);
void PlaneTrails_BackendVertexAlpha(void);
void PlaneTrails_BackendSrcBlend(void);
void PlaneTrails_BackendDestBlend(void);
void PlaneTrails_BackendRasterNull(void);
bool PlaneTrails_BackendTransform(int32_t count);
void PlaneTrails_BackendRenderIndexed(uint16_t *indices,int32_t count);
void PlaneTrails_BackendEnd(void);
void PlaneTrails_BackendRegisterCorona(uint32_t id,void *position);
void PlaneTrails_BackendUpdateCorona(uint32_t id,void *position);
float PlaneTrails_BackendFoggyness(void);
float PlaneTrails_BackendCloudCoverage(void);
float PlaneTrails_BackendRain(void);
int32_t PlaneTrails_BackendSkyTopRed(void);
int32_t PlaneTrails_BackendSkyTopGreen(void);
int32_t PlaneTrails_BackendSkyTopBlue(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
