//+ rouz edit (ChatGPT)
#include "Profile.h"
#define Max(a,b) ((a)>(b)?(a):(b))

#ifndef MASTER
float Profile_ms_afStartTime[NUM_PROFILES];
float Profile_ms_afCumulativeTime[NUM_PROFILES];
float Profile_ms_afEndTime[NUM_PROFILES];
float Profile_ms_afMaxEndTime[NUM_PROFILES];
float Profile_ms_afMaxCumulativeTime[NUM_PROFILES];
const char *Profile_ms_pProfileString[NUM_PROFILES];
ProfileColor Profile_ms_aBarColours[NUM_PROFILES];

void Profile_Initialise(void)
{
    // Preserve the complete original Initialise operation and every intentionally omitted reset
	Profile_ms_afMaxEndTime[PROFILE_FRAME_RATE] = 0.0f;
	Profile_ms_afMaxEndTime[PROFILE_PHYSICS] = 0.0f;
	Profile_ms_afMaxEndTime[PROFILE_COLLISION] = 0.0f;
	Profile_ms_afMaxEndTime[PROFILE_PED_AI] = 0.0f;
	Profile_ms_afMaxEndTime[PROFILE_PROCESSING_TIME] = 0.0f;
	Profile_ms_afMaxEndTime[PROFILE_RENDERING_TIME] = 0.0f;
	Profile_ms_afMaxEndTime[PROFILE_TOTAL] = 0.0f;

	Profile_ms_pProfileString[PROFILE_FRAME_RATE] = "Frame rate";
	Profile_ms_pProfileString[PROFILE_PHYSICS] = "Physics";
	Profile_ms_pProfileString[PROFILE_COLLISION] = "Collision";
	Profile_ms_pProfileString[PROFILE_PED_AI] = "Ped AI";
	Profile_ms_pProfileString[PROFILE_PROCESSING_TIME] = "Processing time";
	Profile_ms_pProfileString[PROFILE_RENDERING_TIME] = "Rendering time";
	Profile_ms_pProfileString[PROFILE_TOTAL] = "Total";

	Profile_ms_afMaxCumulativeTime[PROFILE_FRAME_RATE] = 0.0f;
	Profile_ms_afMaxCumulativeTime[PROFILE_PHYSICS] = 0.0f;
	Profile_ms_afMaxCumulativeTime[PROFILE_COLLISION] = 0.0f;
	Profile_ms_afMaxCumulativeTime[PROFILE_PED_AI] = 0.0f;
	Profile_ms_afMaxCumulativeTime[PROFILE_PROCESSING_TIME] = 0.0f;
	Profile_ms_afMaxCumulativeTime[PROFILE_RENDERING_TIME] = 0.0f;
	Profile_ms_afMaxCumulativeTime[PROFILE_TOTAL] = 0.0f;

	Profile_ms_aBarColours[PROFILE_PHYSICS] = (ProfileColor){ 0, 127, 255, 255 };
	Profile_ms_aBarColours[PROFILE_COLLISION] = (ProfileColor){ 0, 255, 255, 255 };
	Profile_ms_aBarColours[PROFILE_PED_AI] = (ProfileColor){ 255, 0, 0, 255 };
	Profile_ms_aBarColours[PROFILE_PROCESSING_TIME] = (ProfileColor){ 0, 255, 0, 255 };
	Profile_ms_aBarColours[PROFILE_RENDERING_TIME] = (ProfileColor){ 0, 0, 255, 255 };
	Profile_ms_aBarColours[PROFILE_TOTAL] = (ProfileColor){ 255, 255, 255, 255 };
}

void Profile_SuspendProfile(eProfile profile)
{
    // Preserve the complete original SuspendProfile operation and every intentionally omitted reset
	Profile_ms_afEndTime[profile] = -Profile_ms_afStartTime[profile];
	Profile_ms_afCumulativeTime[profile] -= Profile_ms_afStartTime[profile];
}

void Profile_ShowResults(void)
{
    // Preserve the complete original ShowResults operation and every intentionally omitted reset
	Profile_ms_afMaxEndTime[PROFILE_FRAME_RATE] = Max(Profile_ms_afMaxEndTime[PROFILE_FRAME_RATE], Profile_ms_afEndTime[PROFILE_FRAME_RATE]);
	Profile_ms_afMaxEndTime[PROFILE_PHYSICS] = Max(Profile_ms_afMaxEndTime[PROFILE_PHYSICS], Profile_ms_afEndTime[PROFILE_PHYSICS]);
	Profile_ms_afMaxEndTime[PROFILE_COLLISION] = Max(Profile_ms_afMaxEndTime[PROFILE_COLLISION], Profile_ms_afEndTime[PROFILE_COLLISION]);
	Profile_ms_afMaxEndTime[PROFILE_PED_AI] = Max(Profile_ms_afMaxEndTime[PROFILE_PED_AI], Profile_ms_afEndTime[PROFILE_PED_AI]);
	Profile_ms_afMaxEndTime[PROFILE_PROCESSING_TIME] = Max(Profile_ms_afMaxEndTime[PROFILE_PROCESSING_TIME], Profile_ms_afEndTime[PROFILE_PROCESSING_TIME]);
	Profile_ms_afMaxEndTime[PROFILE_RENDERING_TIME] = Max(Profile_ms_afMaxEndTime[PROFILE_RENDERING_TIME], Profile_ms_afEndTime[PROFILE_RENDERING_TIME]);
	Profile_ms_afMaxEndTime[PROFILE_TOTAL] = Max(Profile_ms_afMaxEndTime[PROFILE_TOTAL], Profile_ms_afEndTime[PROFILE_TOTAL]);

	Profile_ms_afMaxCumulativeTime[PROFILE_FRAME_RATE] = Max(Profile_ms_afMaxCumulativeTime[PROFILE_FRAME_RATE], Profile_ms_afCumulativeTime[PROFILE_FRAME_RATE]);
	Profile_ms_afMaxCumulativeTime[PROFILE_PHYSICS] = Max(Profile_ms_afMaxCumulativeTime[PROFILE_PHYSICS], Profile_ms_afCumulativeTime[PROFILE_PHYSICS]);
	Profile_ms_afMaxCumulativeTime[PROFILE_COLLISION] = Max(Profile_ms_afMaxCumulativeTime[PROFILE_COLLISION], Profile_ms_afCumulativeTime[PROFILE_COLLISION]);
	Profile_ms_afMaxCumulativeTime[PROFILE_PED_AI] = Max(Profile_ms_afMaxCumulativeTime[PROFILE_PED_AI], Profile_ms_afCumulativeTime[PROFILE_PED_AI]);
	Profile_ms_afMaxCumulativeTime[PROFILE_PROCESSING_TIME] = Max(Profile_ms_afMaxCumulativeTime[PROFILE_PROCESSING_TIME], Profile_ms_afCumulativeTime[PROFILE_PROCESSING_TIME]);
	Profile_ms_afMaxCumulativeTime[PROFILE_RENDERING_TIME] = Max(Profile_ms_afMaxCumulativeTime[PROFILE_RENDERING_TIME], Profile_ms_afCumulativeTime[PROFILE_RENDERING_TIME]);
	Profile_ms_afMaxCumulativeTime[PROFILE_TOTAL] = Max(Profile_ms_afMaxCumulativeTime[PROFILE_TOTAL], Profile_ms_afCumulativeTime[PROFILE_TOTAL]);
}
#endif
//- rouz edit (ChatGPT)
