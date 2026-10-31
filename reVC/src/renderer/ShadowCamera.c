//+ rouz edit (ChatGPT)
#include "ShadowCamera.h"
#include "Shadow_backend.h"
#include <string.h>
#define nil NULL
#ifndef MASTER
#define ASSERT(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define ASSERT(expression) (expression)
#endif

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

//+ rouz edit (ChatGPT)
void ShadowCamera_Destroy(ShadowCamera *self)
{
    // Retain the complete original resource or frame operation and its canonical instance fields

	if ( self->m_pCamera )
	{
		void		   *raster;
		void			   *frame;
	
		frame = RwHelper_HostCameraFrame(self->m_pCamera);
	
		if ( frame )
		{
			RwHelper_HostSetCameraFrame(self->m_pCamera, nil);
			RwHelper_HostDestroyFrame(frame);
		}
	
		raster = RwHelper_HostCameraZRaster(self->m_pCamera);
		if ( raster )
		{
			RwHelper_HostSetCameraZRaster(self->m_pCamera, nil);
			RwHelper_HostDestroyRaster(raster);
		}

		raster = RwHelper_HostCameraRaster(self->m_pCamera);
		if ( raster )
		{
			RwHelper_HostSetCameraRaster(self->m_pCamera, nil);
			RwHelper_HostDestroyRaster(raster);
		}
		
		if ( self->m_pTexture )
		{
			Shadow_HostTextureRaster(self->m_pTexture, nil);
			Shadow_HostTextureDestroy(self->m_pTexture);
			self->m_pTexture = nil;
		}
		
		RwHelper_HostDestroyCamera(self->m_pCamera);
		self->m_pCamera = nil;
	}
    return;

}

void * ShadowCamera_Create(ShadowCamera *self,int32_t rasterSize)
{
    // Retain the complete original resource or frame operation and its canonical instance fields

	int32_t size = 1 << rasterSize;
	
	self->m_pCamera = RwHelper_HostCreateCamera();
	ASSERT(self->m_pCamera != nil);
	
	if ( self->m_pCamera )
	{
		RwHelper_HostSetCameraFrame(self->m_pCamera, RwHelper_HostCreateFrame());

		if( RwHelper_HostCameraFrame(self->m_pCamera) )
		{
			void *zRaster = RwHelper_HostCreateRaster(size, size, 0, Shadow_HostConstant(SHADOW_CONST_rwRASTERTYPEZBUFFER));
			ASSERT(zRaster != nil);
			
			if ( zRaster )
			{
				RwHelper_HostSetCameraZRaster(self->m_pCamera, zRaster);

				void *raster = RwHelper_HostCreateRaster(size, size, 0, Shadow_HostConstant(SHADOW_CONST_rwRASTERTYPECAMERATEXTURE));
				ASSERT(raster != nil);

				if ( raster )
				{
					RwHelper_HostSetCameraRaster(self->m_pCamera, raster);
					self->m_pTexture = Shadow_HostTextureCreate(raster);
					ASSERT(self->m_pTexture != nil);
					
					if ( self->m_pTexture )
					{
						Shadow_HostTextureAddress(self->m_pTexture, Shadow_HostConstant(SHADOW_CONST_rwTEXTUREADDRESSCLAMP));
						Shadow_HostTextureFilter(self->m_pTexture, Shadow_HostConstant(SHADOW_CONST_rwFILTERLINEAR));
						Shadow_HostProjection(self->m_pCamera,   Shadow_HostConstant(SHADOW_CONST_rwPARALLEL));
						return (self->m_pCamera);
					}
				}
			}
		}
	}

	ShadowCamera_Destroy(self);

	return (nil);

}

void * ShadowCamera_SetFrustum(ShadowCamera *self,float objectRadius)
{
    // Retain the complete original resource or frame operation and its canonical instance fields

	ASSERT(self->m_pCamera != nil);
	
	ShadowVector2 vw;

	Shadow_HostFarClip (self->m_pCamera, 2.0f   * objectRadius);
	Shadow_HostNearClip(self->m_pCamera, 0.001f * objectRadius);

	vw.x = objectRadius;
	vw.y = objectRadius;
	Shadow_HostViewWindow(self->m_pCamera, &vw);

	return self->m_pCamera;

}

void * ShadowCamera_SetLight(ShadowCamera *self,void *light)
{
    // Retain the complete original resource or frame operation and its canonical instance fields

	ASSERT(light != nil);
	ASSERT(self->m_pCamera != nil);
	
	void  *camFrame = RwHelper_HostCameraFrame(self->m_pCamera);
	void *camMatrix = RwHelper_HostFrameMatrix(camFrame);
	void  *lightFrame = Shadow_HostLightFrame(light);
	void *lightMatrix = RwHelper_HostFrameMatrix(lightFrame);

	Shadow_HostCopyVector(Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_RIGHT),Shadow_HostMatrixVector(lightMatrix,SHADOW_MATRIX_RIGHT));
	Shadow_HostCopyVector(Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_UP),Shadow_HostMatrixVector(lightMatrix,SHADOW_MATRIX_UP));
	Shadow_HostCopyVector(Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_AT),Shadow_HostMatrixVector(lightMatrix,SHADOW_MATRIX_AT));
	
	Shadow_HostMatrixUpdate(camMatrix);
	Shadow_HostFrameUpdate(camFrame);

	return self->m_pCamera;

}

void * ShadowCamera_SetCenter(ShadowCamera *self,ShadowVectorRef center)
{
    // Retain the complete original resource or frame operation and its canonical instance fields

	ASSERT(center.x != nil);
	ASSERT(self->m_pCamera != nil);
	
	void            *camFrame = RwHelper_HostCameraFrame(self->m_pCamera);
	void           *camMatrix = RwHelper_HostFrameMatrix(camFrame);
	
	ShadowVectorOut position=Shadow_HostVectorOut(Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_POS));
    memcpy(position.x,center.x,sizeof(float));memcpy(position.y,center.y,sizeof(float));memcpy(position.z,center.z,sizeof(float));
	
	Shadow_HostIncrementScaled(Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_POS), Shadow_HostMatrixVector(camMatrix,SHADOW_MATRIX_AT), -0.5f * Shadow_HostGetFarClip(self->m_pCamera));  

	Shadow_HostMatrixUpdate(camMatrix);
	Shadow_HostFrameUpdate(camFrame);
	Shadow_HostFrameOrtho(camFrame);

	return self->m_pCamera;

}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#define TEXELOFFSET 0.5f
#define TRUE 1
#define FALSE 0
bool Shadow_BeginSoftwareClear(void *camera,const ShadowColour *colour)
{
    // Select the original software colour overload only when software rasterization is configured
#ifdef REVC_SOFTWARE_POLYGONS
    return Shadow_HostSoftwareClear(camera,colour);
#else
    (void)camera;(void)colour;return false;
#endif
}
bool Shadow_BeginSoftwareRaster(void *camera,void *raster)
{
    // Select the original software initial-image overload without changing raster ownership
#ifdef REVC_SOFTWARE_POLYGONS
    return Shadow_HostSoftwareRaster(camera,raster);
#else
    (void)camera;(void)raster;return false;
#endif
}
void Shadow_EndSoftware(void *camera,bool active)
{
    // Query the current camera raster only at the original active-pass upload boundary
#ifdef REVC_SOFTWARE_POLYGONS
    if(active)Shadow_HostSoftwareEnd(RwHelper_HostCameraRaster(camera));
#else
    (void)camera;(void)active;
#endif
}
void *Shadow_RenderAtomic(void *atomic,void *data)
{
    // Save and restore the actual SDK render callback without converting a function pointer into a data pointer
    (void)data;ShadowRenderToken saved=Shadow_HostGetRenderCallback(atomic);Shadow_HostSetRenderCallback(atomic,Shadow_HostDefaultRenderCallback());Shadow_HostAtomicRender(atomic);Shadow_HostSetRenderCallback(atomic,saved);return atomic;
}

void * ShadowCamera_UpdateClump(ShadowCamera *self,void *clump)
{
    // Preserve the complete original render or raster-effect operation and its canonical camera handle

	ASSERT(clump != nil);
	ASSERT(self->m_pCamera != nil);
	
	uint32_t flags;
	void *geometry;
	
	ShadowColour              bgColor = { 255, 255, 255, 0 };
	//+ rouz edit (ChatGPT)
	// Start CPU rasterization for the auxiliary shadow texture
	bool softwarePass = Shadow_BeginSoftwareClear(self->m_pCamera,&bgColor);
	//- rouz edit (ChatGPT)
	
	Shadow_HostClear(self->m_pCamera, &bgColor, Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwCAMERACLEARZ) | Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwCAMERACLEARIMAGE));

	if ( Shadow_HostBeginUpdate(self->m_pCamera) )
	{
		// Use the C helper and retain the original typed SDK result
		geometry = RwHelper_HostAtomicGeometry(((void*)GetFirstAtomic(clump)));
		ASSERT(geometry != nil);
		
		flags = Shadow_HostGeometryFlags(geometry);

		Shadow_HostGeometrySetFlags(geometry, flags & ~(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYPRELIT)|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYLIGHT)
				|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED)|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED2)|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYMODULATEMATERIALCOLOR)));
		
		RwHelper_HostAtomics(clump, Shadow_RenderAtomic, nil);				
		
		Shadow_HostGeometrySetFlags(geometry, flags);
		
		ShadowCamera_InvertRaster(self);
		Shadow_HostEndUpdate(self->m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Store the software silhouette in the shadow camera raster
	Shadow_EndSoftware(self->m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	return self->m_pCamera;

}

void * ShadowCamera_UpdateAtomic(ShadowCamera *self,void *atomic)
{
    // Preserve the complete original render or raster-effect operation and its canonical camera handle

	ASSERT(atomic != nil);
	ASSERT(self->m_pCamera != nil);
	
	uint32_t flags;
	void *geometry;

	ShadowColour              bgColor = { 255, 255, 255, 0 };
	//+ rouz edit (ChatGPT)
	// Start CPU rasterization for the auxiliary shadow texture
	bool softwarePass = Shadow_BeginSoftwareClear(self->m_pCamera,&bgColor);
	//- rouz edit (ChatGPT)

	Shadow_HostClear(self->m_pCamera, &bgColor, Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwCAMERACLEARZ) | Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwCAMERACLEARIMAGE));
  
	if ( Shadow_HostBeginUpdate(self->m_pCamera) )
	{
		geometry = RwHelper_HostAtomicGeometry(atomic);	
		ASSERT(geometry != nil);
		flags = Shadow_HostGeometryFlags(geometry);
		
		Shadow_HostGeometrySetFlags(geometry, flags & ~(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYPRELIT)|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYLIGHT)
				|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED)|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYTEXTURED2)|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYMODULATEMATERIALCOLOR)|Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rpGEOMETRYNORMALS)));
		
		Shadow_RenderAtomic(atomic, nil);
		
		Shadow_HostGeometrySetFlags(geometry, flags);

		ShadowCamera_InvertRaster(self);
		Shadow_HostEndUpdate(self->m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Store the software silhouette in the shadow camera raster
	Shadow_EndSoftware(self->m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	return self->m_pCamera;

}

typedef struct Shadow_InvertRasterScope {ShadowCamera *self;} Shadow_InvertRasterScope;
static void Shadow_InvertRasterBody(void *data,void *vx)
{
    // Execute complete original effect policy with native vertex storage alive throughout the callback
    Shadow_InvertRasterScope *scope=(Shadow_InvertRasterScope*)data;ShadowCamera *self=scope->self;
	ASSERT(self->m_pCamera != nil);
	
	
	float crw, crh;
	void *raster;
	float recipZ;
  
	raster = RwHelper_HostCameraRaster(self->m_pCamera);
	ASSERT(raster != nil);
	
	crw = (float)RwHelper_HostRasterWidth(raster);
	crh = (float)RwHelper_HostRasterHeight(raster);

	recipZ = 1.0f / Shadow_HostGetNearClip(self->m_pCamera);

	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,0), 0.0f);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,0), 0.0f);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,0), Shadow_HostNearScreenZ());
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,0), recipZ);
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,0), 255, 255, 255, 255);

	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,1), 0.0f);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,1), crh);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,1), Shadow_HostNearScreenZ());
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,1), recipZ);
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,1), 255, 255, 255, 255);

	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,2), crw);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,2), 0.0f);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,2), Shadow_HostNearScreenZ());
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,2), recipZ);
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,2), 255, 255, 255, 255);

	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,3), crw);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,3), crh);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,3), Shadow_HostNearScreenZ());
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,3), recipZ);
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,3), 255, 255, 255, 255);


	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),       (void *)FALSE);
	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER),     (void *)nil);
	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void *)TRUE);
	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),          (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDINVDESTCOLOR));
	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),         (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDZERO));

	Shadow_HostDrawStrip(vx,4);

	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),       (void *)TRUE);
	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),          (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDSRCALPHA));
	RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),         (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDINVSRCALPHA));

}
void ShadowCamera_InvertRaster(ShadowCamera *self)
{
    // Preserve distinct per-call SDK vertices and the original conditional return control flow
    Shadow_InvertRasterScope scope={self};RwHelper_HostQuadVertices(Shadow_InvertRasterBody,&scope);
}

typedef struct Shadow_MakeGradientRasterScope {ShadowCamera *self;void * result;} Shadow_MakeGradientRasterScope;
static void Shadow_MakeGradientRasterBody(void *data,void *vx)
{
    // Execute complete original effect policy with native vertex storage alive throughout the callback
    Shadow_MakeGradientRasterScope *scope=(Shadow_MakeGradientRasterScope*)data;ShadowCamera *self=scope->self;
	ASSERT(self->m_pCamera != nil);
	
	

	if ( !self->m_pCamera )
		{scope->result=nil;return;}

	float recipCamZ = 1.0f / Shadow_HostGetNearClip(self->m_pCamera);

	void *raster = RwHelper_HostCameraRaster(self->m_pCamera);
	ASSERT(raster != nil);
	
	float width = (float)RwHelper_HostRasterWidth(raster);
	float height = (float)RwHelper_HostRasterHeight(raster);

	if ( height < 1 )
		{scope->result=nil;return;}
	//+ rouz edit (ChatGPT)
	// Render the generated cutscene shadow gradient into its CPU raster
	ShadowColour clearColor = { 0, 0, 0, 0 };
	bool softwarePass = Shadow_BeginSoftwareClear(self->m_pCamera,&clearColor);
	//- rouz edit (ChatGPT)
      
	if ( Shadow_HostBeginUpdate(self->m_pCamera) )
	{
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER),     (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwFILTERNAFILTERMODE));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),       (void *)FALSE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void *)FALSE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),         (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDZERO));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),          (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDINVDESTCOLOR));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESHADEMODE),         (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwSHADEMODEFLAT));

		float color = 255.0f;
		float step = (-191.0f / height);

		for ( int32_t i = 0; i < height; i++ )
		{
			Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,0), 0.0f);
			Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,0), i);
			Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,0), Shadow_HostNearScreenZ());
			Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,0), recipCamZ);
			Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,0), (uint32_t)color, (uint32_t)color, (uint32_t)color, (uint32_t)color);
			
			Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,1), width - 1);
			Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,1), i);
			Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,1), Shadow_HostNearScreenZ());
			Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,1), recipCamZ);
			Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,1), (uint32_t)color, (uint32_t)color, (uint32_t)color, (uint32_t)color);
			
			Shadow_HostDrawLine(vx,2,0,1);
			
			color += step;
		}
		
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE), (void *)TRUE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),   (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDINVSRCALPHA));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),    (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDSRCALPHA));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESHADEMODE),   (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwSHADEMODEGOURAUD));
		
		Shadow_HostEndUpdate(self->m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Upload the generated gradient before returning its raster
	Shadow_EndSoftware(self->m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	{scope->result=raster;return;}

}
void * ShadowCamera_MakeGradientRaster(ShadowCamera *self)
{
    // Preserve distinct per-call SDK vertices and the original conditional return control flow
    Shadow_MakeGradientRasterScope scope={self,NULL};Shadow_HostTwoVertices(Shadow_MakeGradientRasterBody,&scope);return scope.result;
}

void * ShadowCamera_RasterResample(ShadowCamera *self,void *dstRaster)
{
    // Preserve the complete original render or raster-effect operation and its canonical camera handle

	ASSERT(dstRaster != nil);
	ASSERT(self->m_pCamera != nil);
	
	if ( !self->m_pCamera )
		return nil;

	void *raster = RwHelper_HostCameraRaster(self->m_pCamera);
	ASSERT(raster != nil);
	
	float size = (float) RwHelper_HostRasterWidth(raster);
	float uvOffset = TEXELOFFSET / size;
	float recipCamZ = 1.0f / Shadow_HostGetNearClip(self->m_pCamera);
	//+ rouz edit (ChatGPT)
	// Prepare a CPU target for the full-screen shadow resample
	ShadowColour clearColor = { 0, 0, 0, 0 };
	bool softwarePass = Shadow_BeginSoftwareClear(self->m_pCamera,&clearColor);
	//- rouz edit (ChatGPT)

	if ( Shadow_HostBeginUpdate(self->m_pCamera) )
	{
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),      (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDONE));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),     (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDZERO));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),   (void *)FALSE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTUREFILTER), (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwFILTERLINEAR));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER), (void *)dstRaster);

		Im2DRenderQuad(0.0f, 0.0f, size, size, Shadow_HostNearScreenZ(), recipCamZ, uvOffset);
    
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),   (void *)TRUE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),      (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDSRCALPHA));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),     (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDINVSRCALPHA));
		
		Shadow_HostEndUpdate(self->m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Upload the resampled shadow raster
	Shadow_EndSoftware(self->m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	return raster;

}

void * ShadowCamera_RasterBlur(ShadowCamera *self,void *dstRaster,int32_t numPasses)
{
    // Preserve the complete original render or raster-effect operation and its canonical camera handle

	ASSERT(dstRaster != nil);
	ASSERT(self->m_pCamera != nil);
	
	if ( !self->m_pCamera )
		return nil;

	void *raster = RwHelper_HostCameraRaster(self->m_pCamera);
	ASSERT(raster != nil);
	
	float size = (float) RwHelper_HostRasterWidth(dstRaster);
	float recipCamZ = 1.0f / Shadow_HostGetNearClip(self->m_pCamera);

	for (int i = 0; i < numPasses; i++ )
	{
		RwHelper_HostSetCameraRaster(self->m_pCamera, raster);
		//+ rouz edit (ChatGPT)
		// Rasterize the first half of each shadow blur pass on the CPU
		ShadowColour clearColor = { 0, 0, 0, 0 };
		bool softwarePass = Shadow_BeginSoftwareClear(self->m_pCamera,&clearColor);
		//- rouz edit (ChatGPT)
		
		if ( Shadow_HostBeginUpdate(self->m_pCamera) )
		{
			if ( i == 0 )
			{
				RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),      (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDONE));
				RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),     (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDZERO));
				RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),   (void *)FALSE);
				RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTUREFILTER), (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwFILTERLINEAR));
			}
			
			RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER),     (void *)dstRaster);
			Im2DRenderQuad(0.0f, 0.0f, size, size, Shadow_HostNearScreenZ(), recipCamZ, 1.0f / size);
			Shadow_HostEndUpdate(self->m_pCamera);
		}
		//+ rouz edit (ChatGPT)
		// Save the first blur half into its active target raster
		Shadow_EndSoftware(self->m_pCamera, softwarePass);
		//- rouz edit (ChatGPT)
		
		RwHelper_HostSetCameraRaster(self->m_pCamera, dstRaster);
		//+ rouz edit (ChatGPT)
		// Rasterize the second half of each shadow blur pass on the CPU
		softwarePass = Shadow_BeginSoftwareClear(self->m_pCamera,&clearColor);
		//- rouz edit (ChatGPT)
      
		if ( Shadow_HostBeginUpdate(self->m_pCamera) )
		{
			RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER),      (void *)raster);
			Im2DRenderQuad(0.0f, 0.0f, size, size, Shadow_HostNearScreenZ(), recipCamZ, 0);
        
			if ( i == numPasses - 1 )
			{
				RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),    (void *)TRUE);
				RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),       (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDSRCALPHA));
				RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),      (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDINVSRCALPHA));
			}
			
			Shadow_HostEndUpdate(self->m_pCamera);
		}
		//+ rouz edit (ChatGPT)
		// Save the second blur half into its active target raster
		Shadow_EndSoftware(self->m_pCamera, softwarePass);
		//- rouz edit (ChatGPT)
	}

	RwHelper_HostSetCameraRaster(self->m_pCamera, raster);

	return dstRaster;

}

void * ShadowCamera_RasterGradient(ShadowCamera *self,void *dstRaster)
{
    // Preserve the complete original render or raster-effect operation and its canonical camera handle

	ASSERT(dstRaster != nil);
	ASSERT(self->m_pCamera != nil);
	
	void *raster = RwHelper_HostCameraRaster(self->m_pCamera);
	ASSERT(raster != nil);
	
	float size = (float)RwHelper_HostRasterWidth(dstRaster);
	float recipCamZ = 1.0f / Shadow_HostGetNearClip(self->m_pCamera);
	
	RwHelper_HostSetCameraRaster(self->m_pCamera, dstRaster);
	//+ rouz edit (ChatGPT)
	// Prepare a CPU target for the full-screen shadow gradient blend
	ShadowColour clearColor = { 0, 0, 0, 0 };
	bool softwarePass = Shadow_BeginSoftwareClear(self->m_pCamera,&clearColor);
	//- rouz edit (ChatGPT)
	
	if ( Shadow_HostBeginUpdate(self->m_pCamera) )
	{
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),      (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDZERO));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),     (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDSRCCOLOR));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),   (void *)FALSE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTUREFILTER), (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwFILTERLINEAR));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER), (void *)raster);
		
		Im2DRenderQuad(0, 0, size, size, Shadow_HostNearScreenZ(), recipCamZ, 0);
		
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),   (void *)TRUE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATESRCBLEND),      (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDSRCALPHA));
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEDESTBLEND),     (void*)(uintptr_t)Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwBLENDINVSRCALPHA));
		
		Shadow_HostEndUpdate(self->m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Upload the gradient shadow blend into its destination raster
	Shadow_EndSoftware(self->m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)

	RwHelper_HostSetCameraRaster(self->m_pCamera, raster);

	return dstRaster;

}

typedef struct Shadow_DrawOutlineBorderScope {ShadowCamera *self;const ShadowColour *color;void * result;} Shadow_DrawOutlineBorderScope;
static void Shadow_DrawOutlineBorderBody(void *data,void *vx)
{
    // Execute complete original effect policy with native vertex storage alive throughout the callback
    Shadow_DrawOutlineBorderScope *scope=(Shadow_DrawOutlineBorderScope*)data;ShadowCamera *self=scope->self;const ShadowColour *color=scope->color;
	ASSERT(self->m_pCamera != nil);
	
	
	uint16_t ix[5];

	void *raster = RwHelper_HostCameraRaster(self->m_pCamera);
	ASSERT(raster != nil);

	float size = (float)RwHelper_HostRasterWidth(raster) - 1.0f;
	float recipCamZ = 1.0f / Shadow_HostGetNearClip(self->m_pCamera);
	//+ rouz edit (ChatGPT)
	// Preserve the generated cutscene shadow while capturing its border
	bool softwarePass = Shadow_BeginSoftwareRaster(self->m_pCamera,raster);
	//- rouz edit (ChatGPT)
	
	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,0), 0.0f);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,0), 0.0f);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,0), Shadow_HostNearScreenZ());
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,0), color->red, color->green, color->blue, color->alpha);
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,0), recipCamZ);

	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,1), size);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,1), 0.0f);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,1), Shadow_HostNearScreenZ());
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,1), color->red, color->green, color->blue, color->alpha);
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,1), recipCamZ);

	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,2), size);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,2), size);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,2), Shadow_HostNearScreenZ());
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,2), color->red, color->green, color->blue, color->alpha);
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,2), recipCamZ);

	Shadow_HostVertexScreenX     (Shadow_HostVertex(vx,3), 0.0f);
	Shadow_HostVertexScreenY     (Shadow_HostVertex(vx,3), size);
	Shadow_HostVertexScreenZ     (Shadow_HostVertex(vx,3), Shadow_HostNearScreenZ());
	Shadow_HostVertexIntRGBA     (Shadow_HostVertex(vx,3), color->red, color->green, color->blue, color->alpha);
	Shadow_HostVertexRecipCameraZ(Shadow_HostVertex(vx,3), recipCamZ);

	ix[0] = 0;
	ix[4] = 0;
	ix[1] = 1;
	ix[2] = 2;
	ix[3] = 3;

	if ( Shadow_HostBeginUpdate(self->m_pCamera) )
	{
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),       (void *)FALSE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void *)FALSE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATETEXTURERASTER),     (void *)nil);
		
		Shadow_HostDrawPolyline(vx,4,ix,5);
		
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEZTESTENABLE),       (void *)TRUE);
		RwHelper_HostRenderState(Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void *)TRUE);
		
		Shadow_HostEndUpdate(self->m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Store the completed outline in the camera texture
	Shadow_EndSoftware(self->m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)

	{scope->result=raster;return;}

}
void * ShadowCamera_DrawOutlineBorder(ShadowCamera *self,const ShadowColour *color)
{
    // Preserve distinct per-call SDK vertices and the original conditional return control flow
    Shadow_DrawOutlineBorderScope scope={self,color,NULL};RwHelper_HostQuadVertices(Shadow_DrawOutlineBorderBody,&scope);return scope.result;
}
//- rouz edit (ChatGPT)
