//+ rouz edit (ChatGPT)
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
