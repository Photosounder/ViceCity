//+ rouz edit (ChatGPT)
#pragma once
#include "WaterCannon.h"
#include "Pool.h"
#ifdef __cplusplus
class CPed;
extern "C" {
#else
typedef struct CPed CPed;
#endif
WaterCannonVector WaterCannon_BackendReadVector(const void *value);
WaterCannonVector WaterCannon_BackendCameraForward(void);
bool WaterCannon_BackendEnvMap(void);
void * WaterCannon_BackendEnvCamera(void);
void * WaterCannon_BackendCameraFrame(void *camera);
void * WaterCannon_BackendFrameMatrix(void *frame);
WaterCannonVector WaterCannon_BackendMatrixForward(void *matrix);
bool WaterCannon_BackendExtinguish(WaterCannonVector pos, float range);
CPool * WaterCannon_BackendPedPool(void);
CPed * WaterCannon_BackendPed(void *slot);
float WaterCannon_BackendPedX(CPed *ped);
float WaterCannon_BackendPedY(CPed *ped);
float WaterCannon_BackendPedZ(CPed *ped);
WaterCannonVector WaterCannon_BackendPedPosition(CPed *ped);
int32_t WaterCannon_BackendLocalDirection(CPed *ped);
void WaterCannon_BackendStanding(CPed *ped);
void WaterCannon_BackendForce(CPed *ped, float z);
float WaterCannon_BackendSpeedX(CPed *ped);
float WaterCannon_BackendSpeedY(CPed *ped);
void WaterCannon_BackendSetSpeedX(CPed *ped, float value);
void WaterCannon_BackendSetSpeedY(CPed *ped, float value);
WaterCannonVector WaterCannon_BackendVelocity(CPed *ped);
void WaterCannon_BackendFall(CPed *ped, int32_t animation);
void WaterCannon_BackendParticle(int32_t kind, WaterCannonVector pos, WaterCannonVector dir, int32_t rotationSpeed, int32_t rotation, int32_t frame);
void WaterCannon_BackendSetU(int index, float value);
void WaterCannon_BackendSetV(int index, float value);
void WaterCannon_BackendSetRGBA(int index, int32_t r, int32_t g, int32_t b, int32_t a);
void WaterCannon_BackendSetPos(int index, float x, float y, float z);
void WaterCannon_BackendZWrite(bool enabled);
void WaterCannon_BackendVertexAlpha(bool enabled);
void WaterCannon_BackendFog(bool enabled);
void WaterCannon_BackendRaster(void);
void WaterCannon_BackendLittleTest(void);
bool WaterCannon_BackendTransform(void);
void WaterCannon_BackendRenderIndexed(uint16_t *indices);
void WaterCannon_BackendEnd(void);
void WaterCannon_BackendPush(const char *name);
void WaterCannon_BackendPop(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
