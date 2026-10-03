#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "../core/config.h"
#include "AudioCrimes.h"
typedef struct tPoliceRadioZone {
	char m_aName[8];
	uint32_t m_nSampleIndex;
	int32_t field_12;
} tPoliceRadioZone;
#ifdef __cplusplus
extern "C" {
#endif
extern tPoliceRadioZone ZoneSfx[NUMAUDIOZONES];
extern uint32_t g_nMissionAudioSfx;
extern int8_t g_nMissionAudioPlayingStatus;
extern uint8_t gSpecialSuspectLastSeenReport;
extern uint32_t gMinTimeToNextReport[NUM_CRIME_TYPES];
void AudioPolice_SetMission(uint8_t initialized, uint32_t sfx);
int8_t AudioPolice_GetMissionStatus(void);
void AudioPolice_InitZones(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
