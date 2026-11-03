//+ rouz edit (ChatGPT)
#include "common.h"
#include "ShinyTexts_backend.h"
#include "Camera.h"
#include "RenderBuffer.h"
#include "Shadows.h"
#include "main.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
ShinyTextVector ShinyTexts_BackendCamera(void)
{
    // Copy the original live main-camera position at its original query
    const CVector &v=TheCamera.GetPosition();
    ShinyTextVector result={v.x,v.y,v.z};return result;
}
bool ShinyTexts_BackendEnvMap(void)
{
    // Read the original optional reflection-pass flag
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
void * ShinyTexts_BackendEnvCamera(void)
{
    // Read the live reflection camera on each original use
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::EnvMapCam;
#else
    return NULL;
#endif
}
void * ShinyTexts_BackendEnvFrame(void *camera)
{
    // Enter the original typed reflection frame method
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return ((RwCamera *)camera)->getFrame();
#else
    return NULL;
#endif
}
void * ShinyTexts_BackendFrameLTM(void *frame)
{
    // Enter the original typed reflection local-to-world method
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return ((RwFrame *)frame)->getLTM();
#else
    return NULL;
#endif
}
ShinyTextVector ShinyTexts_BackendMatrixPosition(void *matrix)
{
    // Copy actual matrix position fields without assuming its layout
    const RwV3d &v=((RwMatrix *)matrix)->pos;
    ShinyTextVector result={v.x,v.y,v.z};return result;
}
void * ShinyTexts_BackendWalkTexture(void)
{
    // Read the current walk texture at every original use
    return gpWalkDontTex;
}
void ShinyTexts_BackendLittleTest(void)
{
    // Retain the original shared-buffer hook before transformation
    LittleTest();
}
void ShinyTexts_BackendVertexAlpha(bool enabled)
{
    // Set the original typed VertexAlpha state
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);
}
void ShinyTexts_BackendZWrite(bool enabled)
{
    // Set the original typed ZWrite state
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);
}
void ShinyTexts_BackendSrcBlend(bool additive)
{
    // Set the original typed SrcBlend factor
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)(uintptr_t)(additive?rwBLENDONE:rwBLENDSRCALPHA));
}
void ShinyTexts_BackendDestBlend(bool additive)
{
    // Set the original typed DestBlend factor
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)(uintptr_t)(additive?rwBLENDONE:rwBLENDINVSRCALPHA));
}
void ShinyTexts_BackendSetRGBA(int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Write the actual typed shared vertex RGBA fields
    RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],r,g,b,a);
}
void ShinyTexts_BackendSetPos(int32_t index,float x,float y,float z)
{
    // Write the actual typed shared vertex Pos fields
    RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);
}
void ShinyTexts_BackendSetU(int32_t index,float value)
{
    // Write the actual typed shared vertex U fields
    RwIm3DVertexSetU(&TempBufferRenderVertices[index],value);
}
void ShinyTexts_BackendSetV(int32_t index,float value)
{
    // Write the actual typed shared vertex V fields
    RwIm3DVertexSetV(&TempBufferRenderVertices[index],value);
}
//- rouz edit (ChatGPT)
