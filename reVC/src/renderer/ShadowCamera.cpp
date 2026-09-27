#include "common.h"
#include "rwcore.h"
#include "ShadowCamera.h"
#include "RwHelper.h"
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h"
#endif
//- rouz edit (ChatGPT)

#define TEXELOFFSET 0.5f

//+ rouz edit (ChatGPT)
static bool BeginSoftwareShadowPass(RwCamera *camera, const RwRGBA &clearColor)
{
	// Convert the RenderWare clear color and open a CPU target for this shadow camera
#ifdef REVC_SOFTWARE_POLYGONS
	const rw::RGBA softwareClearColor = {
		clearColor.red, clearColor.green, clearColor.blue, clearColor.alpha
	};
	return SoftwarePolygons::BeginOffscreenFrame(camera, softwareClearColor);
#else
	return false;
#endif
}

//+ rouz edit (ChatGPT)
static bool BeginSoftwareShadowPass(RwCamera *camera, RwRaster *initialRaster)
{
	// Preserve the current shadow image before rasterizing a partial effect
#ifdef REVC_SOFTWARE_POLYGONS
	return SoftwarePolygons::BeginOffscreenFrame(camera, initialRaster);
#else
	(void)camera;
	(void)initialRaster;
	return false;
#endif
}
//- rouz edit (ChatGPT)

static void EndSoftwareShadowPass(RwCamera *camera, bool active)
{
	// Upload the completed CPU shadow image into the camera texture
#ifdef REVC_SOFTWARE_POLYGONS
	if(active)
		SoftwarePolygons::EndOffscreenFrame(RwCameraGetRaster(camera));
#else
	(void)camera;
	(void)active;
#endif
}
//- rouz edit (ChatGPT)

RpAtomic *ShadowRenderCallBack(RpAtomic *atomic, void *data)
{
	RpAtomicCallBackRender savedCB = RpAtomicGetRenderCallBack(atomic);
	RpAtomicSetRenderCallBack(atomic, AtomicDefaultRenderCallBack);
	RpAtomicRender(atomic);
	RpAtomicSetRenderCallBack(atomic, savedCB);	
	return atomic;
}

CShadowCamera::CShadowCamera()
{
	m_pCamera  = nil;
	m_pTexture = nil;
}

CShadowCamera::~CShadowCamera()
{
	Destroy();
}

void
CShadowCamera::Destroy()
{
	if ( m_pCamera )
	{
		RwRaster		   *raster;
		RwFrame			   *frame;
	
		frame = RwCameraGetFrame(m_pCamera);
	
		if ( frame )
		{
			RwCameraSetFrame(m_pCamera, nil);
			RwFrameDestroy(frame);
		}
	
		raster = RwCameraGetZRaster(m_pCamera);
		if ( raster )
		{
			RwCameraSetZRaster(m_pCamera, nil);
			RwRasterDestroy(raster);
		}

		raster = RwCameraGetRaster(m_pCamera);
		if ( raster )
		{
			RwCameraSetRaster(m_pCamera, nil);
			RwRasterDestroy(raster);
		}
		
		if ( m_pTexture )
		{
			RwTextureSetRaster(m_pTexture, nil);
			RwTextureDestroy(m_pTexture);
			m_pTexture = nil;
		}
		
		RwCameraDestroy(m_pCamera);
		m_pCamera = nil;
	}
    return;
}

RwCamera *
CShadowCamera::Create(int32 rasterSize)
{
	int32 size = 1 << rasterSize;
	
	m_pCamera = RwCameraCreate();
	ASSERT(m_pCamera != nil);
	
	if ( m_pCamera )
	{
		RwCameraSetFrame(m_pCamera, RwFrameCreate());

		if( RwCameraGetFrame(m_pCamera) )
		{
			RwRaster *zRaster = RwRasterCreate(size, size, 0, rwRASTERTYPEZBUFFER);
			ASSERT(zRaster != nil);
			
			if ( zRaster )
			{
				RwCameraSetZRaster(m_pCamera, zRaster);

				RwRaster *raster = RwRasterCreate(size, size, 0, rwRASTERTYPECAMERATEXTURE);
				ASSERT(raster != nil);

				if ( raster )
				{
					RwCameraSetRaster(m_pCamera, raster);
					m_pTexture = RwTextureCreate(raster);
					ASSERT(m_pTexture != nil);
					
					if ( m_pTexture )
					{
						RwTextureSetAddressing(m_pTexture, rwTEXTUREADDRESSCLAMP);
						RwTextureSetFilterMode(m_pTexture, rwFILTERLINEAR);
						RwCameraSetProjection(m_pCamera,   rwPARALLEL);
						return (m_pCamera);
					}
				}
			}
		}
	}

	Destroy();

	return (nil);
}

RwCamera *
CShadowCamera::SetFrustum(float objectRadius)
{
	ASSERT(m_pCamera != nil);
	
	RwV2d vw;

	RwCameraSetFarClipPlane (m_pCamera, 2.0f   * objectRadius);
	RwCameraSetNearClipPlane(m_pCamera, 0.001f * objectRadius);

	vw.x = objectRadius;
	vw.y = objectRadius;
	RwCameraSetViewWindow(m_pCamera, &vw);

	return m_pCamera;
}

RwCamera *
CShadowCamera::SetLight(RpLight *light)
{
	ASSERT(light != nil);
	ASSERT(m_pCamera != nil);
	
	RwFrame  *camFrame = RwCameraGetFrame(m_pCamera);
	RwMatrix *camMatrix = RwFrameGetMatrix(camFrame);
	RwFrame  *lightFrame = RpLightGetFrame(light);
	RwMatrix *lightMatrix = RwFrameGetMatrix(lightFrame);

	*RwMatrixGetRight(camMatrix) = *RwMatrixGetRight(lightMatrix);
	*RwMatrixGetUp(camMatrix)    = *RwMatrixGetUp(lightMatrix);
	*RwMatrixGetAt(camMatrix)    = *RwMatrixGetAt(lightMatrix);
	
	RwMatrixUpdate(camMatrix);
	RwFrameUpdateObjects(camFrame);

	return m_pCamera;
}

RwCamera *
CShadowCamera::SetCenter(RwV3d *center)
{
	ASSERT(center != nil);
	ASSERT(m_pCamera != nil);
	
	RwFrame            *camFrame = RwCameraGetFrame(m_pCamera);
	RwMatrix           *camMatrix = RwFrameGetMatrix(camFrame);
	
	*RwMatrixGetPos(camMatrix) = *center;
	
	RwV3dIncrementScaled(RwMatrixGetPos(camMatrix), RwMatrixGetAt(camMatrix), -0.5f * RwCameraGetFarClipPlane(m_pCamera));  

	RwMatrixUpdate(camMatrix);
	RwFrameUpdateObjects(camFrame);
	RwFrameOrthoNormalize(camFrame);

	return m_pCamera;
}

RwCamera *
CShadowCamera::Update(RpClump *clump)
{
	ASSERT(clump != nil);
	ASSERT(m_pCamera != nil);
	
	RwUInt32 flags;
	RpGeometry *geometry;
	
	RwRGBA              bgColor = { 255, 255, 255, 0 };
	//+ rouz edit (ChatGPT)
	// Start CPU rasterization for the auxiliary shadow texture
	bool softwarePass = BeginSoftwareShadowPass(m_pCamera, bgColor);
	//- rouz edit (ChatGPT)
	
	RwCameraClear(m_pCamera, &bgColor, rwCAMERACLEARZ | rwCAMERACLEARIMAGE);

	if ( RwCameraBeginUpdate(m_pCamera) )
	{
		geometry = RpAtomicGetGeometry(GetFirstAtomic(clump));
		ASSERT(geometry != nil);
		
		flags = RpGeometryGetFlags(geometry);

		RpGeometrySetFlags(geometry, flags & ~(rpGEOMETRYPRELIT|rpGEOMETRYLIGHT
				|rpGEOMETRYTEXTURED|rpGEOMETRYTEXTURED2|rpGEOMETRYMODULATEMATERIALCOLOR));
		
		RpClumpForAllAtomics(clump, ShadowRenderCallBack, nil);				
		
		RpGeometrySetFlags(geometry, flags);
		
		InvertRaster();
		RwCameraEndUpdate(m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Store the software silhouette in the shadow camera raster
	EndSoftwareShadowPass(m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	return m_pCamera;
}

RwCamera *
CShadowCamera::Update(RpAtomic *atomic)
{
	ASSERT(atomic != nil);
	ASSERT(m_pCamera != nil);
	
	RwUInt32 flags;
	RpGeometry *geometry;

	RwRGBA              bgColor = { 255, 255, 255, 0 };
	//+ rouz edit (ChatGPT)
	// Start CPU rasterization for the auxiliary shadow texture
	bool softwarePass = BeginSoftwareShadowPass(m_pCamera, bgColor);
	//- rouz edit (ChatGPT)

	RwCameraClear(m_pCamera, &bgColor, rwCAMERACLEARZ | rwCAMERACLEARIMAGE);
  
	if ( RwCameraBeginUpdate(m_pCamera) )
	{
		geometry = RpAtomicGetGeometry(atomic);	
		ASSERT(geometry != nil);
		flags = RpGeometryGetFlags(geometry);
		
		RpGeometrySetFlags(geometry, flags & ~(rpGEOMETRYPRELIT|rpGEOMETRYLIGHT
				|rpGEOMETRYTEXTURED|rpGEOMETRYTEXTURED2|rpGEOMETRYMODULATEMATERIALCOLOR|rpGEOMETRYNORMALS));
		
		ShadowRenderCallBack(atomic, nil);
		
		RpGeometrySetFlags(geometry, flags);

		InvertRaster();
		RwCameraEndUpdate(m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Store the software silhouette in the shadow camera raster
	EndSoftwareShadowPass(m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	return m_pCamera;
}

void
CShadowCamera::InvertRaster()
{
	ASSERT(m_pCamera != nil);
	
	RwIm2DVertex vx[4];
	float crw, crh;
	RwRaster *raster;
	float recipZ;
  
	raster = RwCameraGetRaster(m_pCamera);
	ASSERT(raster != nil);
	
	crw = (float)RwRasterGetWidth(raster);
	crh = (float)RwRasterGetHeight(raster);

	recipZ = 1.0f / RwCameraGetNearClipPlane(m_pCamera);

	RwIm2DVertexSetScreenX     (&vx[0], 0.0f);
	RwIm2DVertexSetScreenY     (&vx[0], 0.0f);
	RwIm2DVertexSetScreenZ     (&vx[0], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetRecipCameraZ(&vx[0], recipZ);
	RwIm2DVertexSetIntRGBA     (&vx[0], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX     (&vx[1], 0.0f);
	RwIm2DVertexSetScreenY     (&vx[1], crh);
	RwIm2DVertexSetScreenZ     (&vx[1], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetRecipCameraZ(&vx[1], recipZ);
	RwIm2DVertexSetIntRGBA     (&vx[1], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX     (&vx[2], crw);
	RwIm2DVertexSetScreenY     (&vx[2], 0.0f);
	RwIm2DVertexSetScreenZ     (&vx[2], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetRecipCameraZ(&vx[2], recipZ);
	RwIm2DVertexSetIntRGBA     (&vx[2], 255, 255, 255, 255);

	RwIm2DVertexSetScreenX     (&vx[3], crw);
	RwIm2DVertexSetScreenY     (&vx[3], crh);
	RwIm2DVertexSetScreenZ     (&vx[3], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetRecipCameraZ(&vx[3], recipZ);
	RwIm2DVertexSetIntRGBA     (&vx[3], 255, 255, 255, 255);


	RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void *)FALSE);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void *)nil);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void *)rwBLENDINVDESTCOLOR);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void *)rwBLENDZERO);

	RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, vx, 4);

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void *)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void *)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void *)rwBLENDINVSRCALPHA);
}

RwRaster *
CShadowCamera::MakeGradientRaster()
{
	ASSERT(m_pCamera != nil);
	
	RwIm2DVertex vx[2];

	if ( !m_pCamera )
		return nil;

	float recipCamZ = 1.0f / RwCameraGetNearClipPlane(m_pCamera);

	RwRaster *raster = RwCameraGetRaster(m_pCamera);
	ASSERT(raster != nil);
	
	float width = (float)RwRasterGetWidth(raster);
	float height = (float)RwRasterGetHeight(raster);

	if ( height < 1 )
		return nil;
	//+ rouz edit (ChatGPT)
	// Render the generated cutscene shadow gradient into its CPU raster
	RwRGBA clearColor = { 0, 0, 0, 0 };
	bool softwarePass = BeginSoftwareShadowPass(m_pCamera, clearColor);
	//- rouz edit (ChatGPT)
      
	if ( RwCameraBeginUpdate(m_pCamera) )
	{
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void *)rwFILTERNAFILTERMODE);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,         (void *)rwBLENDZERO);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,          (void *)rwBLENDINVDESTCOLOR);
		RwRenderStateSet(rwRENDERSTATESHADEMODE,         (void *)rwSHADEMODEFLAT);

		float color = 255.0f;
		float step = (-191.0f / height);

		for ( int32 i = 0; i < height; i++ )
		{
			RwIm2DVertexSetScreenX     (&vx[0], 0.0f);
			RwIm2DVertexSetScreenY     (&vx[0], i);
			RwIm2DVertexSetScreenZ     (&vx[0], RwIm2DGetNearScreenZ());
			RwIm2DVertexSetRecipCameraZ(&vx[0], recipCamZ);
			RwIm2DVertexSetIntRGBA     (&vx[0], (uint32)color, (uint32)color, (uint32)color, (uint32)color);
			
			RwIm2DVertexSetScreenX     (&vx[1], width - 1);
			RwIm2DVertexSetScreenY     (&vx[1], i);
			RwIm2DVertexSetScreenZ     (&vx[1], RwIm2DGetNearScreenZ());
			RwIm2DVertexSetRecipCameraZ(&vx[1], recipCamZ);
			RwIm2DVertexSetIntRGBA     (&vx[1], (uint32)color, (uint32)color, (uint32)color, (uint32)color);
			
			RwIm2DRenderLine(vx, 2, 0, 1);
			
			color += step;
		}
		
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void *)TRUE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,   (void *)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,    (void *)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATESHADEMODE,   (void *)rwSHADEMODEGOURAUD);
		
		RwCameraEndUpdate(m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Upload the generated gradient before returning its raster
	EndSoftwareShadowPass(m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	return raster;
}

RwRaster *
CShadowCamera::RasterResample(RwRaster *dstRaster)
{
	ASSERT(dstRaster != nil);
	ASSERT(m_pCamera != nil);
	
	if ( !m_pCamera )
		return nil;

	RwRaster *raster = RwCameraGetRaster(m_pCamera);
	ASSERT(raster != nil);
	
	float size = (float) RwRasterGetWidth(raster);
	float uvOffset = TEXELOFFSET / size;
	float recipCamZ = 1.0f / RwCameraGetNearClipPlane(m_pCamera);
	//+ rouz edit (ChatGPT)
	// Prepare a CPU target for the full-screen shadow resample
	RwRGBA clearColor = { 0, 0, 0, 0 };
	bool softwarePass = BeginSoftwareShadowPass(m_pCamera, clearColor);
	//- rouz edit (ChatGPT)

	if ( RwCameraBeginUpdate(m_pCamera) )
	{
		RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDONE);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDZERO);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,   (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)dstRaster);

		Im2DRenderQuad(0.0f, 0.0f, size, size, RwIm2DGetNearScreenZ(), recipCamZ, uvOffset);
    
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,   (void *)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDINVSRCALPHA);
		
		RwCameraEndUpdate(m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Upload the resampled shadow raster
	EndSoftwareShadowPass(m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)
	
	return raster;
}

RwRaster *
CShadowCamera::RasterBlur(RwRaster *dstRaster, int32 numPasses)
{
	ASSERT(dstRaster != nil);
	ASSERT(m_pCamera != nil);
	
	if ( !m_pCamera )
		return nil;

	RwRaster *raster = RwCameraGetRaster(m_pCamera);
	ASSERT(raster != nil);
	
	float size = (float) RwRasterGetWidth(dstRaster);
	float recipCamZ = 1.0f / RwCameraGetNearClipPlane(m_pCamera);

	for (int i = 0; i < numPasses; i++ )
	{
		RwCameraSetRaster(m_pCamera, raster);
		//+ rouz edit (ChatGPT)
		// Rasterize the first half of each shadow blur pass on the CPU
		RwRGBA clearColor = { 0, 0, 0, 0 };
		bool softwarePass = BeginSoftwareShadowPass(m_pCamera, clearColor);
		//- rouz edit (ChatGPT)
		
		if ( RwCameraBeginUpdate(m_pCamera) )
		{
			if ( i == 0 )
			{
				RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDONE);
				RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDZERO);
				RwRenderStateSet(rwRENDERSTATEZTESTENABLE,   (void *)FALSE);
				RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
			}
			
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void *)dstRaster);
			Im2DRenderQuad(0.0f, 0.0f, size, size, RwIm2DGetNearScreenZ(), recipCamZ, 1.0f / size);
			RwCameraEndUpdate(m_pCamera);
		}
		//+ rouz edit (ChatGPT)
		// Save the first blur half into its active target raster
		EndSoftwareShadowPass(m_pCamera, softwarePass);
		//- rouz edit (ChatGPT)
		
		RwCameraSetRaster(m_pCamera, dstRaster);
		//+ rouz edit (ChatGPT)
		// Rasterize the second half of each shadow blur pass on the CPU
		softwarePass = BeginSoftwareShadowPass(m_pCamera, clearColor);
		//- rouz edit (ChatGPT)
      
		if ( RwCameraBeginUpdate(m_pCamera) )
		{
			RwRenderStateSet(rwRENDERSTATETEXTURERASTER,      (void *)raster);
			Im2DRenderQuad(0.0f, 0.0f, size, size, RwIm2DGetNearScreenZ(), recipCamZ, 0);
        
			if ( i == numPasses - 1 )
			{
				RwRenderStateSet(rwRENDERSTATEZTESTENABLE,    (void *)TRUE);
				RwRenderStateSet(rwRENDERSTATESRCBLEND,       (void *)rwBLENDSRCALPHA);
				RwRenderStateSet(rwRENDERSTATEDESTBLEND,      (void *)rwBLENDINVSRCALPHA);
			}
			
			RwCameraEndUpdate(m_pCamera);
		}
		//+ rouz edit (ChatGPT)
		// Save the second blur half into its active target raster
		EndSoftwareShadowPass(m_pCamera, softwarePass);
		//- rouz edit (ChatGPT)
	}

	RwCameraSetRaster(m_pCamera, raster);

	return dstRaster;
}

RwRaster *
CShadowCamera::RasterGradient(RwRaster *dstRaster)
{
	ASSERT(dstRaster != nil);
	ASSERT(m_pCamera != nil);
	
	RwRaster *raster = RwCameraGetRaster(m_pCamera);
	ASSERT(raster != nil);
	
	float size = (float)RwRasterGetWidth(dstRaster);
	float recipCamZ = 1.0f / RwCameraGetNearClipPlane(m_pCamera);
	
	RwCameraSetRaster(m_pCamera, dstRaster);
	//+ rouz edit (ChatGPT)
	// Prepare a CPU target for the full-screen shadow gradient blend
	RwRGBA clearColor = { 0, 0, 0, 0 };
	bool softwarePass = BeginSoftwareShadowPass(m_pCamera, clearColor);
	//- rouz edit (ChatGPT)
	
	if ( RwCameraBeginUpdate(m_pCamera) )
	{
		RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDZERO);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDSRCCOLOR);
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,   (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void *)raster);
		
		Im2DRenderQuad(0, 0, size, size, RwIm2DGetNearScreenZ(), recipCamZ, 0);
		
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,   (void *)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND,      (void *)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND,     (void *)rwBLENDINVSRCALPHA);
		
		RwCameraEndUpdate(m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Upload the gradient shadow blend into its destination raster
	EndSoftwareShadowPass(m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)

	RwCameraSetRaster(m_pCamera, raster);

	return dstRaster;
}

RwRaster *CShadowCamera::DrawOutlineBorder(RwRGBA const& color)
{
	ASSERT(m_pCamera != nil);
	
	RwIm2DVertex vx[4];
	RwImVertexIndex ix[5];

	RwRaster *raster = RwCameraGetRaster(m_pCamera);
	ASSERT(raster != nil);

	float size = (float)RwRasterGetWidth(raster) - 1.0f;
	float recipCamZ = 1.0f / RwCameraGetNearClipPlane(m_pCamera);
	//+ rouz edit (ChatGPT)
	// Preserve the generated cutscene shadow while capturing its border
	bool softwarePass = BeginSoftwareShadowPass(m_pCamera, raster);
	//- rouz edit (ChatGPT)
	
	RwIm2DVertexSetScreenX     (&vx[0], 0.0f);
	RwIm2DVertexSetScreenY     (&vx[0], 0.0f);
	RwIm2DVertexSetScreenZ     (&vx[0], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetIntRGBA     (&vx[0], color.red, color.green, color.blue, color.alpha);
	RwIm2DVertexSetRecipCameraZ(&vx[0], recipCamZ);

	RwIm2DVertexSetScreenX     (&vx[1], size);
	RwIm2DVertexSetScreenY     (&vx[1], 0.0f);
	RwIm2DVertexSetScreenZ     (&vx[1], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetIntRGBA     (&vx[1], color.red, color.green, color.blue, color.alpha);
	RwIm2DVertexSetRecipCameraZ(&vx[1], recipCamZ);

	RwIm2DVertexSetScreenX     (&vx[2], size);
	RwIm2DVertexSetScreenY     (&vx[2], size);
	RwIm2DVertexSetScreenZ     (&vx[2], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetIntRGBA     (&vx[2], color.red, color.green, color.blue, color.alpha);
	RwIm2DVertexSetRecipCameraZ(&vx[2], recipCamZ);

	RwIm2DVertexSetScreenX     (&vx[3], 0.0f);
	RwIm2DVertexSetScreenY     (&vx[3], size);
	RwIm2DVertexSetScreenZ     (&vx[3], RwIm2DGetNearScreenZ());
	RwIm2DVertexSetIntRGBA     (&vx[3], color.red, color.green, color.blue, color.alpha);
	RwIm2DVertexSetRecipCameraZ(&vx[3], recipCamZ);

	ix[0] = 0;
	ix[4] = 0;
	ix[1] = 1;
	ix[2] = 2;
	ix[3] = 3;

	if ( RwCameraBeginUpdate(m_pCamera) )
	{
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)FALSE);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     (void *)nil);
		
		RwIm2DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, vx, 4, ix, 5);
		
		RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       (void *)TRUE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
		
		RwCameraEndUpdate(m_pCamera);
	}
	//+ rouz edit (ChatGPT)
	// Store the completed outline in the camera texture
	EndSoftwareShadowPass(m_pCamera, softwarePass);
	//- rouz edit (ChatGPT)

	return raster;
}
