//+ rouz edit (ChatGPT)
#pragma once
#include "Rubbish.h"
#ifdef __cplusplus
extern "C" {
#endif
RubbishVector Rubbish_BackendCameraPosition(void);
float Rubbish_BackendCameraX(void);
float Rubbish_BackendCameraY(void);
float Rubbish_BackendCameraZ(void);
float Rubbish_BackendOrientation(void);
bool Rubbish_BackendEnvMap(void);
void * Rubbish_BackendEnvCamera(void);
void * Rubbish_BackendCameraFrame(void *camera);
void * Rubbish_BackendFrameMatrix(void *frame);
RubbishVector Rubbish_BackendMatrixPosition(void *matrix);
uint16_t Rubbish_BackendVehicleSeed(CVehicle *veh);
float Rubbish_BackendVehicleX(CVehicle *veh);
float Rubbish_BackendVehicleY(CVehicle *veh);
RubbishVector Rubbish_BackendVehiclePosition(CVehicle *veh);
float Rubbish_BackendSpeedX(CVehicle *veh);
float Rubbish_BackendSpeedY(CVehicle *veh);
RubbishVector2 Rubbish_BackendSpeed(CVehicle *veh);
RubbishVector2 Rubbish_BackendForward(CVehicle *veh);
RubbishVector2 Rubbish_BackendRight(CVehicle *veh);
RubbishVector2 Rubbish_BackendBounds(CVehicle *veh);
float Rubbish_BackendGround(float x, float y, float z, bool *found);
float Rubbish_BackendWind(void);
int32_t Rubbish_BackendPamphlet(void);
int32_t * Rubbish_BackendVertexCount(void);
int32_t * Rubbish_BackendIndexCount(void);
void Rubbish_BackendSetIndex(int index, uint32_t value);
void Rubbish_BackendTxdPush(void);
int Rubbish_BackendTxdFind(const char *name);
void Rubbish_BackendTxdSet(int slot);
void Rubbish_BackendTxdPop(void);
void Rubbish_BackendZWrite(bool enabled);
void Rubbish_BackendVertexAlpha(bool enabled);
void Rubbish_BackendFog(bool enabled);
void Rubbish_BackendSetPos(int index, float x, float y, float z);
void Rubbish_BackendSetRGBA(int index, uint8_t r, uint8_t g, uint8_t b, uint32_t alpha);
void Rubbish_BackendSetU(int index, float value);
void Rubbish_BackendSetV(int index, float value);
void Rubbish_BackendLittleTest(void);
bool Rubbish_BackendTransform(int count);
void Rubbish_BackendRenderIndexed(int count);
void Rubbish_BackendEnd(void);
void Rubbish_BackendPush(const char *name);
void Rubbish_BackendPop(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
