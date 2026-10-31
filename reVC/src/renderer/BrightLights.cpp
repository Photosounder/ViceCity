//+ rouz edit (ChatGPT)
#include "common.h"
#include "BrightLights_backend.h"
#include "Camera.h"
#include "RenderBuffer.h"
#include "main.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
BrightLightVector BrightLights_BackendCameraPosition(void)
{
    // Copy the original live main-camera Position value
    const CVector &v=TheCamera.GetPosition();
    BrightLightVector result={v.x,v.y,v.z};return result;
}
BrightLightVector BrightLights_BackendCameraRight(void)
{
    // Copy the original live main-camera Right value
    const CVector &v=TheCamera.GetRight();
    BrightLightVector result={v.x,v.y,v.z};return result;
}
BrightLightVector BrightLights_BackendCameraUp(void)
{
    // Copy the original live main-camera Up value
    const CVector &v=TheCamera.GetUp();
    BrightLightVector result={v.x,v.y,v.z};return result;
}
bool BrightLights_BackendEnvMap(void)
{
    // Read the original reflection flag at its original operand
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
void * BrightLights_BackendEnvCamera(void)
{
    // Read the original live reflection-camera pointer
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::EnvMapCam;
#else
    return NULL;
#endif
}
void * BrightLights_BackendCameraFrame(void *camera)
{
    // Enter the original RenderWare camera-frame operation
    return RwCameraGetFrame((RwCamera *)camera);
}
void * BrightLights_BackendFrameMatrix(void *frame)
{
    // Retain each original RenderWare matrix query
    return RwFrameGetMatrix((RwFrame *)frame);
}
void BrightLights_BackendLittleTest(void)
{
    // Invoke the original buffer hook before live transform-count reads
    LittleTest();
}
void BrightLights_BackendClearRaster(void)
{
    // Clear the original texture raster state
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);
}
BrightLightVector BrightLights_BackendMatrixPosition(const void *matrix)
{
    // Read the actual typed matrix pos components
    const RwV3d &v=((const RwMatrix *)matrix)->pos;
    BrightLightVector result={v.x,v.y,v.z};return result;
}
BrightLightVector BrightLights_BackendMatrixRight(const void *matrix)
{
    // Read the actual typed matrix right components
    const RwV3d &v=((const RwMatrix *)matrix)->right;
    BrightLightVector result={v.x,v.y,v.z};return result;
}
BrightLightVector BrightLights_BackendMatrixUp(const void *matrix)
{
    // Read the actual typed matrix up components
    const RwV3d &v=((const RwMatrix *)matrix)->up;
    BrightLightVector result={v.x,v.y,v.z};return result;
}
void BrightLights_BackendVertexAlpha(bool enabled)
{
    // Set the original typed VertexAlpha state
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);
}
void BrightLights_BackendZWrite(bool enabled)
{
    // Set the original typed ZWrite state
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);
}
void BrightLights_BackendSrcBlend(void)
{
    // Select the original source blend factor
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);
}
void BrightLights_BackendDestBlend(void)
{
    // Select the original destination blend factor
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);
}
void BrightLights_BackendSetRGBA(int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Write the original typed shared vertex RGBA fields
    RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],r,g,b,a);
}
void BrightLights_BackendSetPos(int32_t index,float x,float y,float z)
{
    // Write the original typed shared vertex Pos fields
    RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);
}

BrightLightVector BrightLights_FromVector(CVector value)
{
    // Copy actual typed components without assuming legacy vector layout
    BrightLightVector result={value.x,value.y,value.z};return result;
}
void BrightLights_RegisterOneLegacy(CVector pos,CVector up,CVector side,CVector front,uint8_t type,uint8_t red,uint8_t green,uint8_t blue)
{
    // Retain all original by-value CVector captures before entering the C registration
    BrightLights_RegisterOne(BrightLights_FromVector(pos),BrightLights_FromVector(up),BrightLights_FromVector(side),BrightLights_FromVector(front),type,red,green,blue);
}
//- rouz edit (ChatGPT)
