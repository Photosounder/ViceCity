//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "CutsceneMgr.h"
#include "Lights.h"
#include "Camera.h"
#include "Weather.h"
#include "World.h"
#include "Collision.h"
#include "Sprite.h"
#include "RenderBuffer.h"
#include "PointLights_backend.h"
#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
#include "custompipes.h"
#endif
extern RwRaster *gpPointlightRaster;
PointLightsVector PointLights_BackendReadVector(const void *value)
{
    // Read actual legacy vector components only at their original use
    const CVector *v=(const CVector *)value;
    PointLightsVector result={v->x,v->y,v->z};
    return result;
}
PointLightsVector PointLights_BackendCamera(void)
{
    // Read the original live main-camera position
    return PointLights_BackendReadVector(&TheCamera.GetPosition());
}
bool PointLights_BackendEnvMap(void)
{
    // Read the original active reflection-pass flag
    #if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
void * PointLights_BackendEnvCamera(void)
{
    // Read the actual auxiliary camera pointer
    #if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)
    return CustomPipes::EnvMapCam;
#else
    return NULL;
#endif
}
void * PointLights_BackendEnvFrame(void *camera)
{
    // Query the original typed frame method
    return ((RwCamera *)camera)->getFrame();
}
void * PointLights_BackendFrameLTM(void *frame)
{
    // Query the original typed local-to-world matrix method
    return ((RwFrame *)frame)->getLTM();
}
void * PointLights_BackendCameraFrame(void *camera)
{
    // Enter the original RenderWare camera-frame operation
    return RwCameraGetFrame((RwCamera *)camera);
}
void * PointLights_BackendFrameMatrix(void *frame)
{
    // Enter the original RenderWare frame-matrix operation
    return RwFrameGetMatrix((RwFrame *)frame);
}
bool PointLights_BackendCutscene(void)
{
    // Read the live cutscene eligibility gate
    return CCutsceneMgr::IsRunning();
}
float PointLights_BackendFogginess(void)
{
    // Read original live weather intensity
    return Weather_Foggyness;
}
void * PointLights_BackendRaster(void)
{
    // Read the live original fog texture
    return gpPointlightRaster;
}
bool PointLights_BackendVerticalLine(PointLightsVector coors,float z2,float *groundZ)
{
    // Use the actual collision record and original vertical-line flags
    CColPoint point;
    CEntity *entity;
    bool result=CWorld::ProcessVerticalLine(CVector(coors.x,coors.y,coors.z),z2,point,entity,true,false,false,false,true,false,nil);
    if(result){
        // Publish the original collision height only on success
        *groundZ=point.point.z;
    }
    return result;
}
void PointLights_BackendExtraLight(float x,float y,float z,float red,float green,float blue)
{
    // Enter the original typed directional-light operation
    // Use the complete C lighting owner and its borrowed SDK handles
    Lights_AddAnExtraDirectionalLight(Scene.world,x,y,z,red,green,blue);
}
void PointLights_BackendRemoveExtra(void)
{
    // Remove lights from the original live world
    // Use the complete C lighting owner and its borrowed SDK handles
    Lights_RemoveExtraDirectionalLights(Scene.world);
}




void PointLights_BackendBufferRender(void)
{
    // Flush pending shared geometry at its original point
    // Enter the C shared-buffer algorithm with the original arguments
    RenderBuffer_RenderStuffInBuffer();
}
void PointLights_BackendBufferClear(void)
{
    // Clear the original shared render buffer
    // Enter the C shared-buffer algorithm with the original arguments
    RenderBuffer_ClearRenderBuffer();
}
void PointLights_BackendStart(int indicesCount,int verticesCount,uint16_t **indices,void **vertices)
{
    // Return the original typed vertex allocation as an opaque handle
    RwIm3DVertex *storage;
    // Enter the C shared-buffer algorithm with the original arguments
    RenderBuffer_StartStoringLegacy(indicesCount,verticesCount,indices,&storage);
    *vertices=storage;
}
void PointLights_BackendStop(void)
{
    // Commit the original pending geometry
    // Enter the C shared-buffer algorithm with the original arguments
    RenderBuffer_StopStoring();
}
void PointLights_BackendPush(const char *name)
{
    // Retain original debug labels and configuration
    PUSH_RENDERGROUP(name);
}
void PointLights_BackendPop(void)
{
    // Close the original debug group
    POP_RENDERGROUP();
}
void PointLights_BackendSetRaster(void *raster)
{
    // Set the original typed texture state
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);
}
PointLightsVector PointLights_BackendMatrixPosition(void *matrix)
{
    // Read actual matrix pos components
    const RwV3d &v=((RwMatrix *)matrix)->pos;
    PointLightsVector result={v.x,v.y,v.z};
    return result;
}
PointLightsVector PointLights_BackendMatrixForward(void *matrix)
{
    // Read actual matrix at components
    const RwV3d &v=((RwMatrix *)matrix)->at;
    PointLightsVector result={v.x,v.y,v.z};
    return result;
}
PointLightsVector PointLights_BackendMatrixRight(void *matrix)
{
    // Read actual matrix right components
    const RwV3d &v=((RwMatrix *)matrix)->right;
    PointLightsVector result={v.x,v.y,v.z};
    return result;
}
PointLightsVector PointLights_BackendMatrixUp(void *matrix)
{
    // Read actual matrix up components
    const RwV3d &v=((RwMatrix *)matrix)->up;
    PointLightsVector result={v.x,v.y,v.z};
    return result;
}
void PointLights_BackendZWrite(bool enabled)
{
    // Set the original typed ZWrite state
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);
}
void PointLights_BackendZTest(bool enabled)
{
    // Set the original typed ZTest state
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void *)(uintptr_t)enabled);
}
void PointLights_BackendVertexAlpha(bool enabled)
{
    // Set the original typed VertexAlpha state
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);
}
void PointLights_BackendFog(bool enabled)
{
    // Set the original typed Fog state
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,(void *)(uintptr_t)enabled);
}
void PointLights_BackendSrcBlend(bool special)
{
    // Set the original typed SrcBlend value
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)(uintptr_t)(special?rwBLENDONE:rwBLENDSRCALPHA));
}
void PointLights_BackendDestBlend(bool special)
{
    // Set the original typed DestBlend value
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)(uintptr_t)(special?rwBLENDONE:rwBLENDINVSRCALPHA));
}
void PointLights_BackendCull(bool special)
{
    // Set the original typed Cull value
    RwRenderStateSet(rwRENDERSTATECULLMODE,(void *)(uintptr_t)(special?rwCULLMODECULLNONE:rwCULLMODECULLBACK));
}
void PointLights_BackendSetPos(void *vertices,int index,float x,float y,float z)
{
    // Write the original allocated vertex through its typed setter
    RwIm3DVertexSetPos(&((RwIm3DVertex *)vertices)[index],x,y,z);
}
void PointLights_BackendSetRGBA(void *vertices,int index,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha)
{
    // Write the original allocated vertex through its typed setter
    RwIm3DVertexSetRGBA(&((RwIm3DVertex *)vertices)[index],red,green,blue,alpha);
}
void PointLights_BackendSetU(void *vertices,int index,float value)
{
    // Write the original allocated vertex through its typed setter
    RwIm3DVertexSetU(&((RwIm3DVertex *)vertices)[index],value);
}
void PointLights_BackendSetV(void *vertices,int index,float value)
{
    // Write the original allocated vertex through its typed setter
    RwIm3DVertexSetV(&((RwIm3DVertex *)vertices)[index],value);
}

void PointLights_AddLightLegacy(uint8_t type,CVector coors,CVector dir,float radius,float red,float green,float blue,uint8_t fogType,bool castExtraShadows)
{
    // Preserve original by-value legacy inputs before entering the C algorithm
    PointLights_AddLight(type,PointLights_BackendReadVector(&coors),PointLights_BackendReadVector(&dir),radius,red,green,blue,fogType,castExtraShadows);
}
CVector PointLights_VectorToLegacy(PointLightsVector value)
{
    // Return a typed value to the remaining shadow consumer
    return CVector(value.x,value.y,value.z);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
