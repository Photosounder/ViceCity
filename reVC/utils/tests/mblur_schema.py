"""Describe external SDK primitives for the complete C motion-blur owner."""
VARIABLES={'pFrontBuffer':'void *','ms_bJustInitialised':'bool','ms_bScaledBlur':'bool','BlurOn':'bool','Drunkness':'float','pBufVertCount':'int32_t'}
OPERATIONS=['MotionBlurOpen','MotionBlurClose','CreateImmediateModeData','CreateImmediateModeVertices','MotionBlurRender','OverlayRender','SetDrunkBlur','ClearDrunkBlur','PosInside','AddRenderFx','OverlayRenderFx']
HOST=[
    ('void *','CameraRaster','void *camera','RwCameraGetRaster((RwCamera*)camera)'),
    ('float','NearClip','void *camera','RwCameraGetNearClipPlane((RwCamera*)camera)'),
    ('int32_t','Width','void *raster','RwRasterGetWidth((RwRaster*)raster)'),
    ('int32_t','Height','void *raster','RwRasterGetHeight((RwRaster*)raster)'),
    ('int32_t','Depth','void *raster','RwRasterGetDepth((RwRaster*)raster)'),
    ('void','DestroyRaster','void *raster','RwRasterDestroy((RwRaster*)raster)'),
    ('void','PushContext','void *raster','RwRasterPushContext((RwRaster*)raster)'),
    ('void','PopContext','void','RwRasterPopContext()'),
    ('void','RenderFast','void *raster,int32_t x,int32_t y','RwRasterRenderFast((RwRaster*)raster,x,y)'),
    ('float','NearScreenZ','void','RwIm2DGetNearScreenZ()'),
    ('float','HalfPixel','void','HALFPX'),
    ('int32_t','ScreenWidth','void','RsGlobal.width'),
    ('int32_t','ScreenHeight','void','RsGlobal.height'),
    ('int32_t','VehicleHud','void','CHud::m_VehicleState'),
    ('int32_t','ZoneHud','void','CHud::m_ZoneState'),
    ('int32_t','Fade','void','TheCamera.GetScreenFadeStatus()'),
    ('int32_t','Brightness','void','FrontEndMenuManager.m_PrefsBrightness'),
    ('void','DefinedState','void','DefinedState()'),
    ('void','PostOpen','void *camera','CPostFX::Open((RwCamera*)camera)'),
    ('void','PostClose','void','CPostFX::Close()'),
    ('void','PostRender','void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha','CPostFX::Render((RwCamera*)camera,red,green,blue,blur,type,alpha)'),
]
SETTERS={'ScreenX':('float value','value'),'ScreenY':('float value','value'),'ScreenZ':('float value','value'),'CameraZ':('float value','value'),'RecipCameraZ':('float value','value'),'U':('float value,float reciprocal','value,reciprocal'),'V':('float value,float reciprocal','value,reciprocal'),'IntRGBA':('uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha','red,green,blue,alpha')}
RW_STATES=['TEXTUREFILTER','FOGENABLE','ZTESTENABLE','ZWRITEENABLE','TEXTURERASTER','VERTEXALPHAENABLE','SRCBLEND','DESTBLEND']
BLENDS=['ONE','SRCALPHA','INVSRCALPHA','DESTALPHA','INVDESTALPHA']
STENCIL=[('STENCILENABLE','STENCILENABLE'),('STENCILFUNCTION','STENCILFUNC'),('STENCILFUNCTIONREF','STENCILREF'),('STENCILFUNCTIONMASK','STENCILMASK'),('STENCILFUNCTIONWRITEMASK','STENCILWRITEMASK'),('STENCILZFAIL','STENCILZFAIL'),('STENCILFAIL','STENCILFAIL'),('STENCILPASS','STENCILPASS')]
SDK_MAP={'RwCameraGetRaster':'CameraRaster','RwCameraGetNearClipPlane':'NearClip','RwRasterGetWidth':'Width','RwRasterGetHeight':'Height','RwRasterGetDepth':'Depth','RwRasterDestroy':'DestroyRaster','RwRasterPushContext':'PushContext','RwRasterPopContext':'PopContext','RwRasterRenderFast':'RenderFast','RwIm2DGetNearScreenZ':'NearScreenZ','DefinedState':'DefinedState'}
