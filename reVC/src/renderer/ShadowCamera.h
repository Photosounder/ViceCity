//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
typedef struct ShadowVector {float x,y,z;} ShadowVector;
typedef struct ShadowSphere {ShadowVector center;float radius;} ShadowSphere;
typedef struct ShadowColour {uint8_t red,green,blue,alpha;} ShadowColour;
typedef struct ShadowVectorRef {const float *x,*y,*z;} ShadowVectorRef;
typedef struct ShadowCamera {void *m_pCamera,*m_pTexture;} ShadowCamera;
static inline ShadowVectorRef Shadow_VectorRef(const ShadowVector *vector)
{
    // Borrow live scalar fields without treating a C record as an SDK vector object
    ShadowVectorRef value={NULL,NULL,NULL};if(vector){value.x=&vector->x;value.y=&vector->y;value.z=&vector->z;}return value;
}
#ifdef __cplusplus
extern "C" {
#endif
void ShadowCamera_Init(ShadowCamera *self);
void ShadowCamera_Deinit(ShadowCamera *self);
void *ShadowCamera_Create(ShadowCamera *self,int32_t rasterSize);
void ShadowCamera_Destroy(ShadowCamera *self);
void *ShadowCamera_SetFrustum(ShadowCamera *self,float objectRadius);
void *ShadowCamera_SetLight(ShadowCamera *self,void *light);
void *ShadowCamera_SetCenter(ShadowCamera *self,ShadowVectorRef center);
void *ShadowCamera_UpdateClump(ShadowCamera *self,void *clump);
void *ShadowCamera_UpdateAtomic(ShadowCamera *self,void *atomic);
void ShadowCamera_InvertRaster(ShadowCamera *self);
void *ShadowCamera_GetRwRenderRaster(const ShadowCamera *self);
void *ShadowCamera_MakeGradientRaster(ShadowCamera *self);
void *ShadowCamera_GetRwRenderTexture(const ShadowCamera *self);
void *ShadowCamera_GetRwZRaster(const ShadowCamera *self);
void *ShadowCamera_RasterResample(ShadowCamera *self,void *destination);
void *ShadowCamera_RasterBlur(ShadowCamera *self,void *destination,int32_t passes);
void *ShadowCamera_RasterGradient(ShadowCamera *self,void *destination);
void *ShadowCamera_DrawOutlineBorder(ShadowCamera *self,const ShadowColour *colour);
void *ShadowCamera_GetRwCamera(const ShadowCamera *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
