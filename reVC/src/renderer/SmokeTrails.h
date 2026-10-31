//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct SmokeTrailVector { float x,y,z; } SmokeTrailVector;
typedef struct SmokeTrail {
    SmokeTrailVector m_pos[16];
    float m_opacity[16];
    int32_t m_time[16];
    char m_unused[536];
    int32_t m_seed;
} SmokeTrail;
#ifdef __cplusplus
extern "C" {
#endif
extern bool SmokeTrails_CigOn;
void SmokeTrails_Init(void);
void SmokeTrails_Update(void);
void SmokeTrails_Render(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
