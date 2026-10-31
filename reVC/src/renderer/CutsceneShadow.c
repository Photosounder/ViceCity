//+ rouz edit (ChatGPT)
#include "CutsceneShadow.h"
#include "Shadow_backend.h"
#include <string.h>
#define nil NULL
#ifndef MASTER
#define ASSERT(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define ASSERT(expression) (expression)
#endif

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

//+ rouz edit (ChatGPT)
void CutsceneShadow_Destroy(CutsceneShadow *self)
{
    // Preserve original light lifetime, rotation order and camera propagation

	ShadowCamera_Destroy(&self->m_Camera);
	ShadowCamera_Destroy(&self->m_ResampleCamera);
	ShadowCamera_Destroy(&self->m_BlurCamera);
	ShadowCamera_Destroy(&self->m_GradientCamera);
	
	self->m_pAtomic = nil;
	
	self->m_nRwObjectType = -1;
  
	if (self->m_pLight)
	{
		void *frame = Shadow_HostLightFrame(self->m_pLight);
		Shadow_HostLightSetFrame(self->m_pLight, nil);
		RwHelper_HostDestroyFrame(frame);
		Shadow_HostLightDestroy(self->m_pLight);
		self->m_pLight = nil;
	}

}

void * CutsceneShadow_SetLightProperties(CutsceneShadow *self,float angleY,float angleX,bool setLight)
{
    // Preserve original light lifetime, rotation order and camera propagation

	ASSERT(self->m_pLight != nil);
	
	void *frame;
	static ShadowVector        Xaxis = { 1.0f, 0.0f, 0.0f };
	static ShadowVector        Yaxis = { 0.0f, 1.0f, 0.0f };

	frame = Shadow_HostLightFrame(self->m_pLight);
	ASSERT(frame != nil);
	
	if ( !frame )
		return nil;
	
	Shadow_HostFrameRotate(frame, &Yaxis, angleY, Shadow_HostConstant(SHADOW_CONST_rwCOMBINEREPLACE));
	Shadow_HostFrameRotate(frame, &Xaxis, angleX, Shadow_HostConstant(SHADOW_CONST_rwCOMBINEPOSTCONCAT));
	
	if ( setLight )
		ShadowCamera_SetLight(&self->m_Camera,self->m_pLight);
		
	return frame;

}

void CutsceneShadow_DrawBorderAroundTexture(CutsceneShadow *self,const ShadowColour *color)
{
    // Preserve original light lifetime, rotation order and camera propagation

	if ( self->m_bResample )
		ShadowCamera_DrawOutlineBorder(&self->m_ResampleCamera,color);
	else
		ShadowCamera_DrawOutlineBorder(&self->m_Camera,color);

}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
bool CutsceneShadow_Create(CutsceneShadow *self,void *object,int32_t rasterSize,bool resample,int32_t blurPasses,bool gradient)
{
    // Retain the complete original cutscene lifecycle or update policy with canonical C sphere records

	ASSERT(object != nil);
	
	ShadowLightColour color;
	void *frame;

	if (!object)
		return false;

	self->m_pLight = Shadow_HostLightCreate(Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpLIGHTDIRECTIONAL));
	ASSERT(self->m_pLight != nil);
	
	if (!self->m_pLight)
		return false;

	color.red = color.green = color.blue = 0.8f;
	color.alpha = 0.0f;
	
	Shadow_HostLightColour(self->m_pLight, &color);
	
	frame = RwHelper_HostCreateFrame();
	ASSERT(frame != nil);
	
	Shadow_HostLightSetFrame(self->m_pLight, frame);
	
	CutsceneShadow_SetLightProperties(self,180.0f,90.0f,false);

	self->m_pObject = object;
	self->m_nRwObjectType = Shadow_HostObjectType(self->m_pObject);
	
	{
    // Capture the canonical object type once at the original switch boundary
    int32_t objectType=self->m_nRwObjectType;
if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpCLUMP))
		{
			// Use the C helper and retain the original typed SDK result
			(void)(RwHelper_ClumpSphereView(self->m_pClump,Shadow_SphereRef(&self->m_BoundingSphere),true));
			self->m_BaseSphere.radius = self->m_BoundingSphere.radius;
			Shadow_HostTransformVector(&self->m_BaseSphere.center,&self->m_BoundingSphere.center,RwHelper_HostFrameMatrix(RwHelper_HostClumpFrame(self->m_pClump)));
			
		}

		else if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpATOMIC))
		{
			self->m_BoundingSphere=Shadow_HostReadSphere(RwHelper_HostAtomicSphere(self->m_pAtomic));
			self->m_BaseSphere.radius = self->m_BoundingSphere.radius;
			Shadow_HostTransformVector(&self->m_BaseSphere.center,&self->m_BoundingSphere.center,RwHelper_HostFrameMatrix(Shadow_HostAtomicFrame(self->m_pAtomic)));
			
		}

		else
		{
			CutsceneShadow_Destroy(self);
			return false;
			
		}
	
}

	if ( !ShadowCamera_Create(&self->m_Camera ,rasterSize) )
	{
		CutsceneShadow_Destroy(self);
		return false;
	}
  
	self->m_nBlurPasses = blurPasses;
	self->m_bResample = resample;
	self->m_bGradient = gradient;

	if ( self->m_bResample && !ShadowCamera_Create(&self->m_ResampleCamera ,rasterSize - 1) )
	{
		CutsceneShadow_Destroy(self);
		return false;
	}
  
	if ( self->m_nBlurPasses != 0 )
	{
		if ( !ShadowCamera_Create(&self->m_BlurCamera ,resample ? rasterSize - 1 : rasterSize) )
		{
			CutsceneShadow_Destroy(self);
			return false;
		}
	}
  
	if ( self->m_bGradient )
	{
		if ( !ShadowCamera_Create(&self->m_GradientCamera ,resample ? rasterSize - 1 : rasterSize) )
		{
			CutsceneShadow_Destroy(self);
			return false;
		}
		
		ShadowCamera_MakeGradientRaster(&self->m_GradientCamera);
	}
  
	ShadowCamera_SetLight(&self->m_Camera ,self->m_pLight);
  
	{
    // Capture the canonical object type once at the original switch boundary
    int32_t objectType=self->m_nRwObjectType;
if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpATOMIC)){
			ShadowCamera_SetFrustum(&self->m_Camera ,1.1f * self->m_BoundingSphere.radius);
			
			
		}else if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpCLUMP)){
			ShadowCamera_SetFrustum(&self->m_Camera ,1.1f * self->m_BoundingSphere.radius);
			
	}
}
  
	ShadowCamera_SetCenter(&self->m_Camera ,Shadow_VectorRef(&self->m_BaseSphere.center));
	return true;

}

void * CutsceneShadow_Update(CutsceneShadow *self)
{
    // Retain the complete original cutscene lifecycle or update policy with canonical C sphere records

	{
    // Capture the canonical object type once at the original switch boundary
    int32_t objectType=self->m_nRwObjectType;
if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpCLUMP)){
			ASSERT(self->m_pClump != nil);
			Shadow_HostTransformVector(&self->m_BaseSphere.center,&self->m_BoundingSphere.center,RwHelper_HostFrameMatrix(RwHelper_HostClumpFrame(self->m_pClump)));
			

		}else if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpATOMIC)){
			ASSERT(self->m_pAtomic != nil);
			Shadow_HostTransformVector(&self->m_BaseSphere.center,&self->m_BoundingSphere.center,RwHelper_HostFrameMatrix(Shadow_HostAtomicFrame(self->m_pAtomic)));
			
	}
}

	ShadowCamera_SetCenter(&self->m_Camera ,Shadow_VectorRef(&self->m_BaseSphere.center));

	{
    // Capture the canonical object type once at the original switch boundary
    int32_t objectType=self->m_nRwObjectType;
if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpCLUMP)){
			ShadowCamera_UpdateClump(&self->m_Camera ,self->m_pClump);
			

		}else if(objectType==Shadow_HostCutsceneConstant(SHADOW_CUT_CONST_rpATOMIC)){
			ShadowCamera_UpdateAtomic(&self->m_Camera ,self->m_pAtomic);
			
	}
}
	
	void *raster = ShadowCamera_GetRwRenderRaster(&self->m_Camera);
	ASSERT(raster != nil);

	if ( self->m_bResample )
		return ShadowCamera_RasterResample(&self->m_ResampleCamera ,raster);

	if ( self->m_nBlurPasses )
		return ShadowCamera_RasterBlur(&self->m_BlurCamera ,raster,self->m_nBlurPasses);

	if ( self->m_bGradient )
		return ShadowCamera_RasterGradient(&self->m_GradientCamera ,raster);
	
	return raster;

}
//- rouz edit (ChatGPT)
