//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;class CVehicle;
extern "C" {
#else
typedef struct CEntity CEntity;
typedef struct CVehicle CVehicle;
#endif
enum {
    PED_LIGHTS_WALK,PED_LIGHTS_WALK_BLINK,PED_LIGHTS_DONT_WALK,
    CAR_LIGHTS_GREEN=0,CAR_LIGHTS_YELLOW,CAR_LIGHTS_RED,CAR_LIGHTS_NONE
};
extern bool TrafficLights_bGreenLightsCheat;
void TrafficLights_DisplayActualLight(CEntity *entity);
void TrafficLights_ScanForLightsOnMap(void);
int32_t TrafficLights_FindTrafficLightType(CEntity *entity);
uint8_t TrafficLights_LightForPeds(void);
uint8_t TrafficLights_LightForCars1(void);
uint8_t TrafficLights_LightForCars2(void);
uint8_t TrafficLights_LightForCars1_Visual(void);
uint8_t TrafficLights_LightForCars2_Visual(void);
bool TrafficLights_ShouldCarStopForLight(CVehicle *vehicle,bool alwaysStop);
bool TrafficLights_ShouldCarStopForBridge(CVehicle *vehicle);
bool TrafficLights_DoesLineSegmentIntersect(float x1,float y1,float x2,float y2,float u1,float v1,float u2,float v2);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
