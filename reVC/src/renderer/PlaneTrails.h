//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct PlaneTrailVector { float x,y,z; } PlaneTrailVector;
typedef struct PlaneTrail {
    PlaneTrailVector m_pos[16];
    int32_t m_time[16];
} PlaneTrail;
#ifdef __cplusplus
extern "C" {
#endif
void PlaneTrails_Init(void);
void PlaneTrails_Update(void);
void PlaneTrails_Render(void);
void PlaneTrails_RegisterPoint(PlaneTrailVector pos,uint32_t id);
void PlaneTrails_RegisterPointVector(const void *pos,uint32_t id);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
