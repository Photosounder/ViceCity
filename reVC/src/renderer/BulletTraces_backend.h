//+ rouz edit (ChatGPT)
#pragma once
#include "BulletTraces.h"
#ifdef __cplusplus
extern "C" {
#endif
BulletTraceVector BulletTraces_BackendReadVector(const void *value);
const void * BulletTraces_BackendCameraPosition(void);
const void * BulletTraces_BackendCameraForward(void);
const void * BulletTraces_BackendCameraUp(void);
const void * BulletTraces_BackendCameraRight(void);
void * BulletTraces_BackendPlayerPedEntity(void);
void * BulletTraces_BackendPlayerVehicle(void);
void * BulletTraces_BackendAsVehicle(void *entity);
void * BulletTraces_BackendPlayerVehiclePhysical(void);
void * BulletTraces_BackendPlayerPedPhysical(void);
BulletTraceVector BulletTraces_BackendMoveSpeed(void *physical);
int16_t BulletTraces_BackendCameraMode(void);
bool BulletTraces_BackendEnvMap(void);
void * BulletTraces_BackendEnvCamera(void);
void * BulletTraces_BackendCameraFrame(void *camera);
void * BulletTraces_BackendFrameMatrix(void *frame);
void * BulletTraces_BackendRaster(void *texture);
void BulletTraces_BackendSetRaster(void *raster);
void BulletTraces_BackendZWrite(bool value);
void BulletTraces_BackendSrcBlend(void);
void BulletTraces_BackendDestBlend(void);
bool BulletTraces_BackendTransform(bool uv);
void BulletTraces_BackendRenderIndexed(uint16_t *indices);
void BulletTraces_BackendEnd(void);
void BulletTraces_BackendPushTxd(void);
int32_t BulletTraces_BackendFindTxd(const char *name);
void BulletTraces_BackendSetTxd(int32_t slot);
void * BulletTraces_BackendReadTexture(const char *name);
void BulletTraces_BackendPopTxd(void);
void BulletTraces_BackendDestroyTexture(void *texture);
BulletTraceVector BulletTraces_BackendMatrixPosition(void *matrix);
BulletTraceVector BulletTraces_BackendMatrixForward(void *matrix);
void BulletTraces_BackendSetU(int index,float value);
void BulletTraces_BackendSetV(int index,float value);
void BulletTraces_BackendSetRGBA(int index,int red,int green,int blue,int alpha);
void BulletTraces_BackendSetPos(int index,float x,float y,float z);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
