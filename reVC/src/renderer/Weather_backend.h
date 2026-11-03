//+ rouz edit (ChatGPT)
#pragma once
#include "Weather.h"
#include "ParticleType.h"
#ifdef __cplusplus
class CVehicle;class CEntity;
extern "C" {
#else
typedef struct CVehicle CVehicle;typedef struct CEntity CEntity;
#endif
enum {WEATHER_TRIG_TABLE_SIZE=1024,WEATHER_rwRENDERSTATETEXTURERASTER=1,WEATHER_rwRENDERSTATEZTESTENABLE=6,WEATHER_rwRENDERSTATEZWRITEENABLE=8,WEATHER_rwRENDERSTATESRCBLEND=10,WEATHER_rwRENDERSTATEDESTBLEND=11,WEATHER_rwRENDERSTATEVERTEXALPHAENABLE=12,WEATHER_rwRENDERSTATEFOGENABLE=14,WEATHER_rwRENDERSTATEFOGTYPE=16,WEATHER_rwFOGTYPELINEAR=1,WEATHER_rwBLENDONE=2,WEATHER_rwBLENDSRCALPHA=5,WEATHER_rwBLENDINVSRCALPHA=6};
bool Weather_BackendNoHurricanes(void);
bool Weather_BackendShockDown(void);
void Weather_BackendShake(int16_t duration,uint8_t frequency);
float Weather_BackendSunZ(void);
bool Weather_BackendInterior(void);
bool Weather_BackendCutscene(void);
bool Weather_BackendPlayerExists(void);
bool Weather_BackendPlayerInAir(void);
float Weather_BackendPlayerZ(void);
float Weather_BackendPlayerSpeedX(void);
float Weather_BackendPlayerSpeedY(void);
CVehicle * Weather_BackendPlayerVehicle(void);
bool Weather_BackendVehicleRoof(CVehicle *car);
int16_t Weather_BackendCameraMode(void);
bool Weather_BackendCameraForwardFirstPerson(void);
bool Weather_BackendCameraLRBFirstPerson(void);
float Weather_BackendScreenWidth(void);
float Weather_BackendScreenHeight(void);
float Weather_BackendCosValue(int32_t angle);
float Weather_BackendSinValue(int32_t angle);
float Weather_BackendCameraOrientation(void);
float Weather_BackendCameraSpeed(void);
CEntity * Weather_BackendTarget(void);
WeatherVector Weather_BackendEntityPosition(CEntity *entity);
float Weather_BackendGroundZ(float x,float y,float z,bool *found);
void Weather_BackendRemoveParticles(enum tParticleType type);
void Weather_BackendParticleSimple(enum tParticleType type,WeatherVector position,WeatherVector direction);
void Weather_BackendParticleColor(enum tParticleType type,WeatherVector position,WeatherVector direction,float size,WeatherColor value);
void Weather_BackendParticleDrop(enum tParticleType type,WeatherVector position,WeatherVector direction,float size,int32_t rotationSpeed,int32_t rotation,int32_t frame,int32_t lifespan);
bool Weather_BackendVerticalLine(WeatherVector value,float *height);
bool Weather_BackendEnvMap(void);
void * Weather_BackendEnvCamera(void);
void * Weather_BackendCameraFrame(void *camera);
void * Weather_BackendFrameMatrix(void *frame);
void Weather_BackendVertexRGBA(int32_t index,int32_t red,int32_t green,int32_t blue,int32_t alpha);
void Weather_BackendVertexPos(int32_t index,float x,float y,float z);
void Weather_BackendVertexU(int32_t index,float u);
void Weather_BackendVertexV(int32_t index,float v);
void Weather_BackendRenderState(int32_t state,uintptr_t value);
void * Weather_BackendRaster(void);
bool Weather_BackendTransform(int32_t vertices);
void Weather_BackendRenderIndexed(int32_t indices);
void Weather_BackendEnd(void);
WeatherVector Weather_BackendCameraPosition(void);
WeatherVector Weather_BackendCameraForward(void);
WeatherVector Weather_BackendCameraRight(void);
WeatherVector Weather_BackendCameraUp(void);
WeatherVector Weather_BackendCameraPrevious(void);
WeatherVector Weather_BackendMatrixPosition(void *matrix);
WeatherVector Weather_BackendMatrixRight(void *matrix);
WeatherVector Weather_BackendMatrixUp(void *matrix);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
