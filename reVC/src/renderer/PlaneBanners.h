//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct PlaneBannerVector { float x,y,z; } PlaneBannerVector;
typedef struct PlaneBanner { PlaneBannerVector m_pos[8]; } PlaneBanner;
#ifdef __cplusplus
extern "C" {
#endif
void PlaneBanners_Init(void);
void PlaneBanners_Update(void);
void PlaneBanners_Render(void);
void PlaneBanners_RegisterPoint(PlaneBannerVector position,uint32_t id);
void PlaneBanners_RegisterPointVector(const void *position,uint32_t id);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
