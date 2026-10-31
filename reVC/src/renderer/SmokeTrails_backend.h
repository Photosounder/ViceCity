//+ rouz edit (ChatGPT)
#pragma once
#include "SmokeTrails.h"
typedef struct SmokeTrailPoint { void *owner;float *x,*y,*z; } SmokeTrailPoint;
typedef struct SmokeTrailPoints { SmokeTrailPoint start,end; } SmokeTrailPoints;
typedef void (*SmokeTrailPointsCallback)(SmokeTrailPoints points);
#ifdef __cplusplus
extern "C" {
#endif
bool SmokeTrails_BackendEnvMap(void);
bool SmokeTrails_BackendSphereVisible(SmokeTrailVector position,float radius);
bool SmokeTrails_BackendFirstPerson(void);
void * SmokeTrails_BackendPlayerPed(void);
void * SmokeTrails_BackendPlayerVehicle(void);
bool SmokeTrails_BackendCutscene(void);
void * SmokeTrails_BackendClump(void *ped);
void * SmokeTrails_BackendHierarchy(void *clump);
int32_t SmokeTrails_BackendHeadTag(void);
int32_t SmokeTrails_BackendBoneIndex(void *hier,int32_t tag);
void * SmokeTrails_BackendMatrixAt(void *hier,int32_t index);
void SmokeTrails_BackendWithPoints(SmokeTrailPointsCallback callback);
void SmokeTrails_BackendTransformPoint(void *point,void *matrix);
float SmokeTrails_BackendWind(void);
void SmokeTrails_BackendSetRGBA(int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void SmokeTrails_BackendSetPos(int32_t index,float x,float y,float z);
void SmokeTrails_BackendVertexAlpha(bool enabled);
void SmokeTrails_BackendSrcBlend(void);
void SmokeTrails_BackendDestBlend(void);
void SmokeTrails_BackendRasterNull(void);
bool SmokeTrails_BackendTransform(int32_t count);
void SmokeTrails_BackendRenderIndexed(uint16_t *indices,int32_t count);
void SmokeTrails_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
