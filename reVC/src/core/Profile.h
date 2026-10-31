//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"
typedef enum eProfile
{
	PROFILE_FRAME_RATE,
	PROFILE_PHYSICS,
	PROFILE_COLLISION,
	PROFILE_PED_AI,
	PROFILE_PROCESSING_TIME,
	PROFILE_RENDERING_TIME,
	PROFILE_TOTAL,
	NUM_PROFILES,
} eProfile;
typedef struct ProfileColor {uint8_t red,green,blue,alpha;} ProfileColor;
#ifdef __cplusplus
extern "C" {
#endif
extern float Profile_ms_afStartTime[NUM_PROFILES];
extern float Profile_ms_afCumulativeTime[NUM_PROFILES];
extern float Profile_ms_afEndTime[NUM_PROFILES];
extern float Profile_ms_afMaxEndTime[NUM_PROFILES];
extern float Profile_ms_afMaxCumulativeTime[NUM_PROFILES];
extern const char *Profile_ms_pProfileString[NUM_PROFILES];
extern ProfileColor Profile_ms_aBarColours[NUM_PROFILES];
void Profile_Initialise(void);
void Profile_SuspendProfile(eProfile profile);
void Profile_ShowResults(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
