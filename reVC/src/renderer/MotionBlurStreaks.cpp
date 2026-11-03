//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Timecycle.h"
#include "MotionBlurStreaks_backend.h"
static RwIm3DVertex StreakVertices[4];
void MotionBlurStreaks_BackendFogColor(uint32_t color)
{
    // Set the original typed packed fog-color state
    RwRenderStateSet(rwRENDERSTATEFOGCOLOR,(void *)(uintptr_t)color);
}
void MotionBlurStreaks_BackendSrcBlend(void)
{
    // Select the original source blend factor
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);
}
void MotionBlurStreaks_BackendDestBlend(void)
{
    // Select the original destination blend factor
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);
}
void MotionBlurStreaks_BackendRaster(void)
{
    // Clear the original typed texture-raster state
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,(void *)FALSE);
}
void MotionBlurStreaks_BackendLittleTest(void)
{
    // Preserve the original shared-buffer hook
    LittleTest();
}
bool MotionBlurStreaks_BackendTransform(void)
{
    // Transform the original typed four-vertex array with UV flags
    return RwIm3DTransform(StreakVertices,4,nil,rwIM3D_VERTEXUV)!=nil;
}
void MotionBlurStreaks_BackendRenderIndexed(uint16_t *indices)
{
    // Render all twelve C-owned indices through the original typed primitive
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,indices,12);
}
void MotionBlurStreaks_BackendEnd(void)
{
    // Complete the original successful immediate draw
    RwIm3DEnd();
}
void MotionBlurStreaks_BackendZWrite(bool value)
{
    // Set the original typed ZWrite state
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)value);
}
void MotionBlurStreaks_BackendVertexAlpha(bool value)
{
    // Set the original typed VertexAlpha state
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)value);
}
void MotionBlurStreaks_BackendFog(bool value)
{
    // Set the original typed Fog state
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,(void *)(uintptr_t)value);
}
void MotionBlurStreaks_BackendSetU(int index,float value)
{
    // Write the shared typed vertex U fields
    RwIm3DVertexSetU(&StreakVertices[index],value);
}
void MotionBlurStreaks_BackendSetV(int index,float value)
{
    // Write the shared typed vertex V fields
    RwIm3DVertexSetV(&StreakVertices[index],value);
}
void MotionBlurStreaks_BackendSetRGBA(int index,int r,int g,int b,int a)
{
    // Write the shared typed vertex RGBA fields
    RwIm3DVertexSetRGBA(&StreakVertices[index],r,g,b,a);
}
void MotionBlurStreaks_BackendSetPos(int index,float x,float y,float z)
{
    // Write the shared typed vertex Pos fields
    RwIm3DVertexSetPos(&StreakVertices[index],x,y,z);
}

MotionBlurStreakVector MotionBlurStreaks_FromVector(CVector value)
{
    // Copy actual typed components without assuming legacy object layout
    MotionBlurStreakVector result={value.x,value.y,value.z};return result;
}
void MotionBlurStreaks_RegisterStreakLegacy(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,CVector p1,CVector p2)
{
    // Retain original by-value input copies before entering the complete C registration
    MotionBlurStreaks_RegisterStreak(id,r,g,b,MotionBlurStreaks_FromVector(p1),MotionBlurStreaks_FromVector(p2));
}
//- rouz edit (ChatGPT)
