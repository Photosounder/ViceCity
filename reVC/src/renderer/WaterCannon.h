//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#define WATERCANNON_GRAVITY (0.009f)
#define WATERCANNON_LIFETIME (150)
enum { WATERCANNON_SEGMENTS = 16 };
typedef struct WaterCannonVector { float x, y, z; } WaterCannonVector;
typedef struct WaterCannon {
	int32_t m_nId;
	int16_t m_nCur;
	uint32_t m_nTimeCreated;
	WaterCannonVector m_avecPos[WATERCANNON_SEGMENTS];
	WaterCannonVector m_avecVelocity[WATERCANNON_SEGMENTS];
	bool m_abUsed[WATERCANNON_SEGMENTS];
} WaterCannon;
#ifdef __cplusplus
extern "C" {
#endif
extern WaterCannon WaterCannons_aCannons[NUM_WATERCANNONS];
extern uint16_t WaterCannonIndexList[12];
void WaterCannon_Init(WaterCannon *cannon);
void WaterCannon_Update_OncePerFrame(WaterCannon *cannon, int16_t index);
void WaterCannon_Update_NewInput(WaterCannon *cannon, const WaterCannonVector *pos, const WaterCannonVector *dir);
void WaterCannon_Update_NewInputLegacy(WaterCannon *cannon, const void *pos, const void *dir);
void WaterCannon_Render(WaterCannon *cannon);
void WaterCannon_PushPeds(WaterCannon *cannon);
void WaterCannons_Init(void);
void WaterCannons_UpdateOne(uint32_t id, const WaterCannonVector *pos, const WaterCannonVector *dir);
void WaterCannons_UpdateOneLegacy(uint32_t id, const void *pos, const void *dir);
void WaterCannons_Update(void);
void WaterCannons_Render(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
