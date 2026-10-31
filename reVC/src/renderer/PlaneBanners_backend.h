//+ rouz edit (ChatGPT)
#pragma once
#include "PlaneBanners.h"
#ifdef __cplusplus
extern "C" {
#endif
PlaneBannerVector PlaneBanners_BackendVector(const void *position);
PlaneBannerVector PlaneBanners_BackendCameraPosition(void);
bool PlaneBanners_BackendEnvMap(void);
void * PlaneBanners_BackendEnvCamera(void);
void * PlaneBanners_BackendCameraFrame(void *camera);
void * PlaneBanners_BackendFrameMatrix(void *frame);
PlaneBannerVector PlaneBanners_BackendMatrixPosition(void *matrix);
bool PlaneBanners_BackendSphereVisible(PlaneBannerVector position,float radius);
void PlaneBanners_BackendSetRGBA(int32_t index,uint8_t alpha);
void PlaneBanners_BackendSetPos(int32_t index,float x,float y,float z);
void PlaneBanners_BackendSetU(int32_t index,float u);
void PlaneBanners_BackendSetV(int32_t index,float v);
void PlaneBanners_BackendZWrite(bool enabled);
void PlaneBanners_BackendZTest(bool enabled);
void PlaneBanners_BackendFog(bool enabled);
void PlaneBanners_BackendVertexAlpha(bool enabled);
void PlaneBanners_BackendSrcBlend(void);
void PlaneBanners_BackendDestBlend(void);
void PlaneBanners_BackendRaster(void *raster);
bool PlaneBanners_BackendTransform(int32_t count);
void PlaneBanners_BackendRenderIndexed(int32_t count);
void PlaneBanners_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
