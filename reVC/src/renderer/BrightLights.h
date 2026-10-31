//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct BrightLightVector { float x,y,z; } BrightLightVector;
typedef struct BrightLight {
    BrightLightVector m_pos,m_up,m_side,m_front;
    float m_camDist;
    uint8_t m_type,m_red,m_green,m_blue;
} BrightLight;
enum {
    BRIGHTLIGHT_INVALID,
    BRIGHTLIGHT_TRAFFIC_GREEN,BRIGHTLIGHT_TRAFFIC_YELLOW,BRIGHTLIGHT_TRAFFIC_RED,
    BRIGHTLIGHT_FRONT_LONG,BRIGHTLIGHT_FRONT_SMALL,BRIGHTLIGHT_FRONT_BIG,BRIGHTLIGHT_FRONT_TALL,
    BRIGHTLIGHT_REAR_LONG,BRIGHTLIGHT_REAR_SMALL,BRIGHTLIGHT_REAR_BIG,BRIGHTLIGHT_REAR_TALL,
    BRIGHTLIGHT_SIREN,
    BRIGHTLIGHT_FRONT=BRIGHTLIGHT_FRONT_LONG,BRIGHTLIGHT_REAR=BRIGHTLIGHT_REAR_LONG
};
#ifdef __cplusplus
extern "C" {
#endif
void BrightLights_Init(void);
void BrightLights_RegisterOne(BrightLightVector pos,BrightLightVector up,BrightLightVector side,BrightLightVector front,uint8_t type,uint8_t red,uint8_t green,uint8_t blue);
void BrightLights_Render(bool reuseFlicker);
void BrightLights_RenderOutGeometryBuffer(void);
#ifdef __cplusplus
}
class CVector;
BrightLightVector BrightLights_FromVector(CVector value);
void BrightLights_RegisterOneLegacy(CVector pos,CVector up,CVector side,CVector front,uint8_t type,uint8_t red=0,uint8_t green=0,uint8_t blue=0);
#endif
//- rouz edit (ChatGPT)
