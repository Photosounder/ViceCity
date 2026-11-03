//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "Camera.h"
#include "World.h"
#include "PlayerPed.h"
#include "Vehicle.h"
#include "TxdStore.h"
#include "BulletTraces_backend.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
static RwIm3DVertex TraceVertices[10];
BulletTraceVector BulletTraces_BackendReadVector(const void *value)
{
    // Read actual legacy vector components at their original copy point
    const CVector &v=*(const CVector *)value;
    BulletTraceVector result={v.x,v.y,v.z};return result;
}
const void * BulletTraces_BackendCameraPosition(void)
{
    // Borrow the original live camera position reference
    return &TheCamera.GetPosition();
}
const void * BulletTraces_BackendCameraForward(void)
{
    // Borrow the original live forward-vector reference
    return &TheCamera.GetForward();
}
const void * BulletTraces_BackendCameraUp(void)
{
    // Borrow the original live up-vector reference
    return &TheCamera.GetUp();
}
const void * BulletTraces_BackendCameraRight(void)
{
    // Borrow the original live right-vector reference
    return &TheCamera.GetRight();
}
void * BulletTraces_BackendPlayerPedEntity(void)
{
    // Preserve the original typed player-to-entity conversion and query
    return (CEntity *)FindPlayerPed();
}
void * BulletTraces_BackendPlayerVehicle(void)
{
    // Preserve each original live player-vehicle query
    return FindPlayerVehicle();
}
void * BulletTraces_BackendAsVehicle(void *entity)
{
    // Preserve the original typed entity-to-vehicle address conversion
    return (CVehicle *)(CEntity *)entity;
}
void * BulletTraces_BackendPlayerVehiclePhysical(void)
{
    // Preserve the original repeated vehicle query and physical-base conversion
    return (CPhysical *)FindPlayerVehicle();
}
void * BulletTraces_BackendPlayerPedPhysical(void)
{
    // Preserve the original repeated ped query and physical-base conversion
    return (CPhysical *)FindPlayerPed();
}
BulletTraceVector BulletTraces_BackendMoveSpeed(void *physical)
{
    // Read the actual physical-owner velocity before C computes speed
    return BulletTraces_BackendReadVector(&((CPhysical *)physical)->m_vecMoveSpeed);
}
int16_t BulletTraces_BackendCameraMode(void)
{
    // Read the original active camera mode only after shooter eligibility
    return TheCamera.Cams[TheCamera.ActiveCam].Mode;
}
bool BulletTraces_BackendEnvMap(void)
{
    // Read the original conditional reflection-pass flag
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
void * BulletTraces_BackendEnvCamera(void)
{
    // Read the actual auxiliary camera at every original use
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::EnvMapCam;
#else
    return NULL;
#endif
}
void * BulletTraces_BackendCameraFrame(void *camera)
{
    // Enter the original typed camera-frame operation
    return RwCameraGetFrame((RwCamera *)camera);
}
void * BulletTraces_BackendFrameMatrix(void *frame)
{
    // Enter each original repeated frame-matrix query
    return RwFrameGetMatrix((RwFrame *)frame);
}
void BulletTraces_BackendZWrite(bool value)
{
    // Set the original typed depth-write state
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)value);
}
void BulletTraces_BackendSrcBlend(void)
{
    // Set the original source blend factor
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);
}
void BulletTraces_BackendDestBlend(void)
{
    // Set the original destination blend factor
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);
}
bool BulletTraces_BackendTransform(bool uv)
{
    // Transform the actual shared ten-vertex array with the original flags
    return RwIm3DTransform(TraceVertices,ARRAY_SIZE(TraceVertices),nil,uv?rwIM3D_VERTEXUV:1)!=nil;
}
void BulletTraces_BackendRenderIndexed(uint16_t *indices)
{
    // Render the entire original typed triangle table
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,indices,48);
}
void BulletTraces_BackendEnd(void)
{
    // Finish the original successful immediate draw
    RwIm3DEnd();
}



BulletTraceVector BulletTraces_BackendMatrixPosition(void *matrix)
{
    // Read actual matrix pos components
    const RwV3d &v=((RwMatrix *)matrix)->pos;
    BulletTraceVector result={v.x,v.y,v.z};return result;
}
BulletTraceVector BulletTraces_BackendMatrixForward(void *matrix)
{
    // Read actual matrix at components
    const RwV3d &v=((RwMatrix *)matrix)->at;
    BulletTraceVector result={v.x,v.y,v.z};return result;
}
void BulletTraces_BackendSetU(int index,float value)
{
    // Write actual shared vertex U fields
    RwIm3DVertexSetU(&TraceVertices[index],value);
}
void BulletTraces_BackendSetV(int index,float value)
{
    // Write actual shared vertex V fields
    RwIm3DVertexSetV(&TraceVertices[index],value);
}
void BulletTraces_BackendSetRGBA(int index,int red,int green,int blue,int alpha)
{
    // Write actual shared vertex RGBA fields
    RwIm3DVertexSetRGBA(&TraceVertices[index],red,green,blue,alpha);
}
void BulletTraces_BackendSetPos(int index,float x,float y,float z)
{
    // Write actual shared vertex Pos fields
    RwIm3DVertexSetPos(&TraceVertices[index],x,y,z);
}

BulletTraceVector BulletTraces_FromVector(CVector value)
{
    // Copy replay input coordinates through their actual typed owner
    BulletTraceVector result={value.x,value.y,value.z};return result;
}
CVector BulletTraces_ToVector(BulletTraceVector value)
{
    // Return a typed replay packet value without aliasing a C record
    return CVector(value.x,value.y,value.z);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
