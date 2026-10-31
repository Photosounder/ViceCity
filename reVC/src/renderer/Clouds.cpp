//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "main.h"
#include "Sprite.h"
#include "Sprite2d.h"
#include "General.h"
#include "Game.h"
#include "Coronas.h"
#include "Camera.h"
#include "TxdStore.h"
#include "Weather.h"
#include "Clock.h"
#include "Timer.h"
#include "Timecycle.h"
#include "Renderer.h"
#include "Clouds.h"
#ifdef REVC_SOFTWARE_POLYGONS
#include "RenderBuffer.h" // rouz edit (ChatGPT)
#include "SoftwarePolygons.h" // rouz edit (ChatGPT)
#endif

#define SMALLSTRIPHEIGHT 4.0f
#define HORIZSTRIPHEIGHT 48.0f




//+ rouz edit (ChatGPT)

//- rouz edit (ChatGPT)



//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "Clouds_backend.h"




void * Clouds_BackendReadTexture(const char *name,const char *mask)
{
    // Retain the original typed owner operation at its original C call point
    return RwTextureRead(name,mask);
}
void Clouds_BackendDestroyTexture(void *texture)
{
    // Retain the original typed owner operation at its original C call point
    RwTextureDestroy((RwTexture *)texture);
}
void * Clouds_BackendRaster(void *texture)
{
    // Retain the original typed owner operation at its original C call point
    return RwTextureGetRaster((RwTexture *)texture);
}
float Clouds_BackendOrientation(void)
{
    // Retain the original typed owner operation at its original C call point
    return TheCamera.Orientation;
}
bool Clouds_BackendOutside(void)
{
    // Retain the original typed owner operation at its original C call point
    return CGame::CanSeeOutSideFromCurrArea();
}
float Clouds_BackendScreenWidth(void)
{
    // Retain the original typed owner operation at its original C call point
    return SCREEN_WIDTH;
}
CloudsVector Clouds_BackendCameraPosition(void)
{
    // Retain the original typed owner operation at its original C call point
    const CVector &v=TheCamera.GetPosition();CloudsVector result={v.x,v.y,v.z};return result;
}
bool * Clouds_BackendSunBlocked(void)
{
    // Retain the original typed owner operation at its original C call point
    return &CCoronas::SunBlockedByClouds;
}
float Clouds_BackendMoonSize(void)
{
    // Retain the original typed owner operation at its original C call point
    return CCoronas::MoonSize;
}
float Clouds_BackendSunX(void)
{
    // Retain the original typed owner operation at its original C call point
    return CCoronas::SunScreenX;
}
float Clouds_BackendSunY(void)
{
    // Retain the original typed owner operation at its original C call point
    return CCoronas::SunScreenY;
}
void * Clouds_BackendCoronaTexture(int32_t index)
{
    // Retain the original typed owner operation at its original C call point
    return gpCoronaTexture[index];
}



void Clouds_BackendSetRaster(void *raster)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);
}
void Clouds_BackendPush(const char *name)
{
    // Retain the original typed owner operation at its original C call point
    PUSH_RENDERGROUP(name);
}
void Clouds_BackendPop(void)
{
    // Retain the original typed owner operation at its original C call point
    POP_RENDERGROUP();
}
float Clouds_BackendWind(void)
{
    // Retain the original typed owner operation at its original C call point
    return Weather_Wind;
}
float Clouds_BackendFoggyness(void)
{
    // Retain the original typed owner operation at its original C call point
    return Weather_Foggyness;
}
float Clouds_BackendCloudCoverage(void)
{
    // Retain the original typed owner operation at its original C call point
    return Weather_CloudCoverage;
}
float Clouds_BackendExtraSunnyness(void)
{
    // Retain the original typed owner operation at its original C call point
    return Weather_ExtraSunnyness;
}
float Clouds_BackendRainbow(void)
{
    // Retain the original typed owner operation at its original C call point
    return Weather_Rainbow;
}
int32_t Clouds_BackendLowCloudsRed(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetLowCloudsRed();
}
int32_t Clouds_BackendLowCloudsGreen(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetLowCloudsGreen();
}
int32_t Clouds_BackendLowCloudsBlue(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetLowCloudsBlue();
}
int32_t Clouds_BackendFluffyCloudsTopRed(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetFluffyCloudsTopRed();
}
int32_t Clouds_BackendFluffyCloudsTopGreen(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetFluffyCloudsTopGreen();
}
int32_t Clouds_BackendFluffyCloudsTopBlue(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetFluffyCloudsTopBlue();
}
int32_t Clouds_BackendFluffyCloudsBottomRed(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetFluffyCloudsBottomRed();
}
int32_t Clouds_BackendFluffyCloudsBottomGreen(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetFluffyCloudsBottomGreen();
}
int32_t Clouds_BackendFluffyCloudsBottomBlue(void)
{
    // Retain the original typed owner operation at its original C call point
    return TimeCycle_GetFluffyCloudsBottomBlue();
}
void Clouds_BackendZWrite(bool enabled)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);
}
void Clouds_BackendZTest(bool enabled)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void *)(uintptr_t)enabled);
}
void Clouds_BackendVertexAlpha(bool enabled)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);
}
void Clouds_BackendFog(bool enabled)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,(void *)(uintptr_t)enabled);
}
void Clouds_BackendSrcBlend(bool additive)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)(uintptr_t)(additive?rwBLENDONE:rwBLENDSRCALPHA));
}
void Clouds_BackendDestBlend(bool additive)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)(uintptr_t)(additive?rwBLENDONE:rwBLENDINVSRCALPHA));
}
void Clouds_BackendCull(bool none)
{
    // Retain the original typed owner operation at its original C call point
    RwRenderStateSet(rwRENDERSTATECULLMODE,(void *)(uintptr_t)(none?rwCULLMODECULLNONE:rwCULLMODECULLBACK));
}





#ifdef REVC_SOFTWARE_POLYGONS
void * Clouds_BackendCameraFrame(void *camera)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    return RwCameraGetFrame((RwCamera *)camera);
}
void * Clouds_BackendFrameMatrix(void *frame)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    return RwFrameGetMatrix((RwFrame *)frame);
}
float Clouds_BackendNearClip(void *camera)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    return RwCameraGetNearClipPlane((RwCamera *)camera);
}
float Clouds_BackendFarClip(void *camera)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    return RwCameraGetFarClipPlane((RwCamera *)camera);
}
CloudsVector Clouds_BackendSunDirection(void)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    CVector v=TimeCycle_ToNativeVector(*TimeCycle_GetSunDirection());CloudsVector result={v.x,v.y,v.z};return result;
}
CloudsVector Clouds_BackendMatrixPosition(void *matrix)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    const RwV3d &v=((RwMatrix *)matrix)->pos;CloudsVector result={v.x,v.y,v.z};return result;
}
CloudsVector Clouds_BackendMatrixRight(void *matrix)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    const RwV3d &v=((RwMatrix *)matrix)->right;CloudsVector result={v.x,v.y,v.z};return result;
}
CloudsVector Clouds_BackendMatrixUp(void *matrix)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    const RwV3d &v=((RwMatrix *)matrix)->up;CloudsVector result={v.x,v.y,v.z};return result;
}
CloudsVector Clouds_BackendMatrixForward(void *matrix)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    const RwV3d &v=((RwMatrix *)matrix)->at;CloudsVector result={v.x,v.y,v.z};return result;
}
void Clouds_BackendSetPos(void *vertices,int32_t index,float x,float y,float z)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetPos(&v[index],x,y,z);
}
void Clouds_BackendSetRGBA(void *vertices,int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetRGBA(&v[index],r,g,b,a);
}
void Clouds_BackendSetU(void *vertices,int32_t index,float value)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetU(&v[index],value);
}
void Clouds_BackendSetV(void *vertices,int32_t index,float value)
{
    // Retain the actual reflection camera or vertex field type without assuming its layout
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetV(&v[index],value);
}
#endif
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
static CRGBA BackgroundColors[3];
CloudsColorFields Clouds_BackendColorFields(int32_t index)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    CRGBA &c=BackgroundColors[index];CloudsColorFields result={&c.r,&c.g,&c.b,&c.a};return result;
}
CloudsVector Clouds_BackendCameraUp(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    const CVector &v=TheCamera.GetUp();CloudsVector result={v.x,v.y,v.z};return result;
}
CloudsVector Clouds_BackendCameraForward(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    const CVector &v=TheCamera.GetForward();CloudsVector result={v.x,v.y,v.z};return result;
}
float Clouds_BackendForwardAxis(int32_t axis)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    const CVector &v=TheCamera.GetForward();return axis==0?v.x:axis==1?v.y:v.z;
}
bool Clouds_BackendCollisionPolys(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    return gbShowCollisionPolys;
}
float Clouds_BackendScreenHeight(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    return SCREEN_HEIGHT;
}
CloudsViewMatrix Clouds_BackendViewMatrix(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    const CMatrix &m=TheCamera.m_viewMatrix;CloudsViewMatrix result={m.rx,m.ry,m.rz,m.fx,m.fy,m.fz,m.ux,m.uy,m.uz,m.px,m.py,m.pz};return result;
}
bool Clouds_BackendCapturingWorldEffects(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    #ifdef REVC_SOFTWARE_POLYGONS
    return SoftwarePolygons::CapturingWorldEffects();
#else
    return false;
#endif
}
void Clouds_BackendDrawBackgroundQuad(float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,int32_t top,int32_t bottom)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    ([](float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,const CRGBA &c0,const CRGBA &c1,const CRGBA &c2,const CRGBA &c3){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawAnyRect(x1,y1,x2,y2,x3,y3,x4,y4,Sprite_ColourRef(&c0.r,&c0.g,&c0.b,&c0.a),Sprite_ColourRef(&c1.r,&c1.g,&c1.b,&c1.a),Sprite_ColourRef(&c2.r,&c2.g,&c2.b,&c2.a),Sprite_ColourRef(&c3.r,&c3.g,&c3.b,&c3.a));
})(x1,y1,x2,y2,x3,y3,x4,y4,BackgroundColors[top],BackgroundColors[top],BackgroundColors[bottom],BackgroundColors[bottom]);
}
void Clouds_BackendDrawBackgroundRect(float left,float top,float right,float bottom,int32_t lower,int32_t upper)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    ([](const CRect &r,const CRGBA &c0,const CRGBA &c1,const CRGBA &c2,const CRGBA &c3){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawFourColourRect(&r,Sprite_ColourRef(&c0.r,&c0.g,&c0.b,&c0.a),Sprite_ColourRef(&c1.r,&c1.g,&c1.b,&c1.a),Sprite_ColourRef(&c2.r,&c2.g,&c2.b,&c2.a),Sprite_ColourRef(&c3.r,&c3.g,&c3.b,&c3.a));
})(Rect_Make(left,top,right,bottom),BackgroundColors[lower],BackgroundColors[lower],BackgroundColors[upper],BackgroundColors[upper]);
}
float Clouds_BackendAmbientRed(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    return TimeCycle_GetAmbientRed();
}
float Clouds_BackendAmbientGreen(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    return TimeCycle_GetAmbientGreen();
}
float Clouds_BackendAmbientBlue(void)
{
    // Preserve the original actual owner fields, references and typed drawing operation
    return TimeCycle_GetAmbientBlue();
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
