//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct PointLightsVector { float x,y,z; } PointLightsVector;
typedef struct PointLight {
    PointLightsVector coors,dir;
    float radius,red,green,blue;
    int8_t type,fogType;
    bool castExtraShadows;
} PointLight;
enum {
    POINTLIGHTS_LIGHT_POINT, POINTLIGHTS_LIGHT_DIRECTIONAL, POINTLIGHTS_LIGHT_DARKEN,
    POINTLIGHTS_LIGHT_FOGONLY_ALWAYS, POINTLIGHTS_LIGHT_FOGONLY
};
enum { POINTLIGHTS_FOG_NONE, POINTLIGHTS_FOG_NORMAL, POINTLIGHTS_FOG_ALWAYS };
#ifdef __cplusplus
extern "C" {
#endif
extern int16_t PointLights_NumLights;
extern PointLight PointLights_aLights[NUMPOINTLIGHTS];
extern PointLightsVector PointLights_aCachedMapReads[32];
extern float PointLights_aCachedMapReadResults[32];
extern int32_t PointLights_NextCachedValue;
extern float FogSizes[8];
void PointLights_Init(void);
void PointLights_InitPerFrame(void);
void PointLights_ResetEnvMapLights(void);
void PointLights_AddLight(uint8_t type, PointLightsVector coors, PointLightsVector dir, float radius, float red, float green, float blue, uint8_t fogType, bool castExtraShadows);
float PointLights_GenerateLightsAffectingObject(const PointLightsVector *objCoors);
float PointLights_GenerateLightsAffectingObjectLegacy(const void *objCoors);
void PointLights_RemoveLightsAffectingObject(void);
void PointLights_RenderFogEffect(void);
void PointLights_RenderFogEffectForEnvMap(void *camera);
void PointLights_RenderFogEffectForEnvMapLights(void *camera, PointLight *lights, int32_t lightCount);
bool PointLights_ProcessVerticalLineUsingCache(PointLightsVector coors,float *groundZ);
#ifdef __cplusplus
}
class CVector;
void PointLights_AddLightLegacy(uint8_t type,CVector coors,CVector dir,float radius,float red,float green,float blue,uint8_t fogType,bool castExtraShadows);
CVector PointLights_VectorToLegacy(PointLightsVector value);
#endif
//- rouz edit (ChatGPT)
