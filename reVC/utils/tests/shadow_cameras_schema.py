"""Describe the complete shadow-camera pair and explicit C lifecycle boundary."""
RENAMES={'src/renderer/ShadowCamera.cpp':'src/renderer/ShadowCamera.c','src/renderer/CutsceneShadow.cpp':'src/renderer/CutsceneShadow.c'}
CAMERA_OPERATIONS=['Init','Deinit','Create','Destroy','SetFrustum','SetLight','SetCenter','UpdateClump','UpdateAtomic','InvertRaster','GetRwRenderRaster','MakeGradientRaster','GetRwRenderTexture','GetRwZRaster','RasterResample','RasterBlur','RasterGradient','DrawOutlineBorder','GetRwCamera']
CUTSCENE_OPERATIONS=['Init','Deinit','GetBaseSphere','Create','SetLightProperties','IsInitialized','Destroy','Update','UpdateForCutscene','GetShadowCamera','GetShadowRwTexture','DrawBorderAroundTexture']
HELPERS=['BeginSoftwareClear','BeginSoftwareRaster','EndSoftware','RenderAtomic']
CAMERA_FIELDS=['m_pCamera','m_pTexture']
CUTSCENE_FIELDS=['m_Camera','m_bResample','m_ResampleCamera','m_nBlurPasses','m_BlurCamera','m_bGradient','m_GradientCamera','m_pObject','m_nRwObjectType','m_pLight','m_BoundingSphere','m_BaseSphere']

def camera_header():
    # Expose all camera operations through C records and preserve live input components through scalar references
    return '''//+ rouz edit (ChatGPT)
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
'''

def cutscene_header():
    # Preserve original member order and embedded cameras while replacing C++ unions and methods with a C API
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "ShadowCamera.h"
enum {CUTSCENE_SHADOW_RASTER,CUTSCENE_SHADOW_RESAMPLE,CUTSCENE_SHADOW_BLUR,CUTSCENE_SHADOW_GRADIENT};
typedef struct CutsceneShadow {
    ShadowCamera m_Camera;
    bool m_bResample;
    ShadowCamera m_ResampleCamera;
    int32_t m_nBlurPasses;
    ShadowCamera m_BlurCamera;
    bool m_bGradient;
    ShadowCamera m_GradientCamera;
    union {void *m_pObject;void *m_pAtomic;void *m_pClump;};
    int32_t m_nRwObjectType;
    void *m_pLight;
    ShadowSphere m_BoundingSphere,m_BaseSphere;
} CutsceneShadow;
#ifdef __cplusplus
extern "C" {
#endif
void CutsceneShadow_Init(CutsceneShadow *self);
void CutsceneShadow_Deinit(CutsceneShadow *self);
ShadowSphere CutsceneShadow_GetBaseSphere(const CutsceneShadow *self);
bool CutsceneShadow_Create(CutsceneShadow *self,void *object,int32_t rasterSize,bool resample,int32_t blurPasses,bool gradient);
void *CutsceneShadow_SetLightProperties(CutsceneShadow *self,float angleY,float angleX,bool setLight);
bool CutsceneShadow_IsInitialized(const CutsceneShadow *self);
void CutsceneShadow_Destroy(CutsceneShadow *self);
void *CutsceneShadow_Update(CutsceneShadow *self);
void *CutsceneShadow_UpdateForCutscene(CutsceneShadow *self);
ShadowCamera *CutsceneShadow_GetShadowCamera(CutsceneShadow *self,int32_t type);
void *CutsceneShadow_GetShadowRwTexture(const CutsceneShadow *self);
void CutsceneShadow_DrawBorderAroundTexture(CutsceneShadow *self,const ShadowColour *colour);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def camera_lifecycle():
    # Move original construction, destruction entry and all four getters into genuine C
    return '''//+ rouz edit (ChatGPT)
#include "ShadowCamera.h"
#include "RwHelper_backend.h"
void ShadowCamera_Init(ShadowCamera *self)
{
    // Preserve the original two pointer stores without initializing unrelated caller storage
    self->m_pCamera=NULL;self->m_pTexture=NULL;
}
void ShadowCamera_Deinit(ShadowCamera *self)
{
    // Retain the original destructor's complete cleanup call
    ShadowCamera_Destroy(self);
}
void *ShadowCamera_GetRwCamera(const ShadowCamera *self)
{
    // Return the canonical current camera handle
    return self->m_pCamera;
}
void *ShadowCamera_GetRwRenderTexture(const ShadowCamera *self)
{
    // Return the canonical current texture handle
    return self->m_pTexture;
}
void *ShadowCamera_GetRwRenderRaster(const ShadowCamera *self)
{
    // Query the actual SDK framebuffer at the original inline getter boundary
    return RwHelper_HostCameraRaster(self->m_pCamera);
}
void *ShadowCamera_GetRwZRaster(const ShadowCamera *self)
{
    // Query the actual SDK depth raster at the original inline getter boundary
    return RwHelper_HostCameraZRaster(self->m_pCamera);
}
//- rouz edit (ChatGPT)
'''

def cutscene_lifecycle():
    # Make implicit member lifecycle order explicit and retain all original getters and routing decisions
    return '''//+ rouz edit (ChatGPT)
#include "CutsceneShadow.h"
void CutsceneShadow_Init(CutsceneShadow *self)
{
    // Construct embedded cameras in their original declaration order before the parent constructor body
    ShadowCamera_Init(&self->m_Camera);ShadowCamera_Init(&self->m_ResampleCamera);ShadowCamera_Init(&self->m_BlurCamera);ShadowCamera_Init(&self->m_GradientCamera);
    // Retain original field initialization and leave both sphere records unspecified until their original assignments
    self->m_pAtomic=NULL;self->m_nRwObjectType=-1;self->m_pLight=NULL;self->m_nBlurPasses=0;self->m_bResample=false;self->m_bGradient=false;
}
void CutsceneShadow_Deinit(CutsceneShadow *self)
{
    // Execute the parent destructor body before the original reverse member destruction order
    CutsceneShadow_Destroy(self);ShadowCamera_Deinit(&self->m_GradientCamera);ShadowCamera_Deinit(&self->m_BlurCamera);ShadowCamera_Deinit(&self->m_ResampleCamera);ShadowCamera_Deinit(&self->m_Camera);
}
ShadowSphere CutsceneShadow_GetBaseSphere(const CutsceneShadow *self)
{
    // Return the same by-value sphere snapshot as the original inline getter
    return self->m_BaseSphere;
}
bool CutsceneShadow_IsInitialized(const CutsceneShadow *self)
{
    // Preserve the original object-pointer predicate independently of camera allocation state
    return self->m_pObject!=NULL;
}
ShadowCamera *CutsceneShadow_GetShadowCamera(CutsceneShadow *self,int32_t type)
{
    // Preserve camera selection and the fallback for every unrecognized selector
    switch(type){case CUTSCENE_SHADOW_RESAMPLE:return &self->m_ResampleCamera;case CUTSCENE_SHADOW_BLUR:return &self->m_BlurCamera;case CUTSCENE_SHADOW_GRADIENT:return &self->m_GradientCamera;}return &self->m_Camera;
}
void *CutsceneShadow_GetShadowRwTexture(const CutsceneShadow *self)
{
    // Preserve the original resample preference without substituting blur or gradient textures
    if(self->m_bResample)return ShadowCamera_GetRwRenderTexture(&self->m_ResampleCamera);return ShadowCamera_GetRwRenderTexture(&self->m_Camera);
}
void *CutsceneShadow_UpdateForCutscene(CutsceneShadow *self)
{
    // Discard the update raster and read the selected texture after all update side effects
    CutsceneShadow_Update(self);return CutsceneShadow_GetShadowRwTexture(self);
}
//- rouz edit (ChatGPT)
'''
