//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioPoliceState.h"
#include "DMAudio.h"
#include "sampman.h"
#include <string.h>
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

tPoliceRadioZone ZoneSfx[NUMAUDIOZONES];

uint32_t g_nMissionAudioSfx = TOTAL_AUDIO_SAMPLES;
int8_t g_nMissionAudioPlayingStatus = PLAY_STATUS_FINISHED;
uint8_t gSpecialSuspectLastSeenReport;
uint32_t gMinTimeToNextReport[NUM_CRIME_TYPES];


void
AudioPolice_InitZones(void)
{
	// Preserve the original police state gates, partial writes, and backend order
	for (int32_t i = 0; i < NUMAUDIOZONES; i++)
		memset(ZoneSfx[i].m_aName, 0, 8);

#define SETZONESFX(i, name, sample) \
	strcpy(ZoneSfx[i].m_aName, name); \
	ZoneSfx[i].m_nSampleIndex = sample;

	SETZONESFX(0, "VICE_C", SFX_POLICE_RADIO_VICE_CITY);
	SETZONESFX(1, "IND_ZON", SFX_POLICE_RADIO_VICE_CITY_BEACH);
	SETZONESFX(2, "COM_ZON", SFX_POLICE_RADIO_VICE_CITY_MAINLAND);
	SETZONESFX(3, "BEACH1", SFX_POLICE_RADIO_OCEAN_BEACH);
	SETZONESFX(4, "BEACH2", SFX_POLICE_RADIO_WASHINGTON_BEACH);
	SETZONESFX(5, "BEACH3", SFX_POLICE_RADIO_VICE_POINT);
	SETZONESFX(6, "GOLFC", SFX_POLICE_RADIO_LEAF_LINKS);
	SETZONESFX(7, "STARI", SFX_POLICE_RADIO_STARFISH_ISLAND);
	SETZONESFX(8, "DOCKS", SFX_POLICE_RADIO_VICEPORT);
	SETZONESFX(9, "HAVANA", SFX_POLICE_RADIO_LITTLE_HAVANA);
	SETZONESFX(10, "HAITI", SFX_POLICE_RADIO_LITTLE_HAITI);
	SETZONESFX(11, "PORNI", SFX_POLICE_RADIO_PRAWN_ISLAND);
	SETZONESFX(12, "DTOWN", SFX_POLICE_RADIO_DOWNTOWN);
	SETZONESFX(13, "A_PORT", SFX_POLICE_RADIO_ESCOBAR_INTERNATIONAL);

#undef SETZONESFX
}

void
AudioPolice_Init(cAudioManager *manager)
{
	// Preserve the original police state gates, partial writes, and backend order
	// Reset the radio queue through its C interface
	PoliceRadioQueue_Reset(&manager->m_sPoliceRadioQueue);
	for (int32_t i = 0; i < ARRAY_SIZE(manager->m_aCrimes); i++)
		manager->m_aCrimes[i].type = CRIME_NONE;
#if !defined(GTA_PS2) || defined(AUDIO_REVERB)
	SampleManager_SetChannelReverbFlag(&SampleManager, CHANNEL_POLICE_RADIO, 0);
#endif
	gSpecialSuspectLastSeenReport = 0;
	for (int32_t i = 0; i < ARRAY_SIZE(gMinTimeToNextReport); i++)
		gMinTimeToNextReport[i] = manager->m_FrameCounter;
}

void
AudioPolice_Reset(cAudioManager *manager)
{
	// Preserve the original police state gates, partial writes, and backend order
	if (!manager->m_bIsInitialised) return;
	if (SampleManager_GetChannelUsedFlag(&SampleManager, CHANNEL_POLICE_RADIO)) SampleManager_StopChannel(&SampleManager, CHANNEL_POLICE_RADIO);
	AudioPolice_Init(manager);
}

void
AudioPolice_SetMission(uint8_t initialized, uint32_t sfx)
{
	// Preserve the original police state gates, partial writes, and backend order
	if (!initialized) return;
	if (g_nMissionAudioPlayingStatus != PLAY_STATUS_PLAYING) {
		g_nMissionAudioPlayingStatus = PLAY_STATUS_STOPPED;
		g_nMissionAudioSfx = sfx;
	}
}

int8_t
AudioPolice_GetMissionStatus(void)
{
	// Preserve the original police state gates, partial writes, and backend order
	return g_nMissionAudioPlayingStatus;
}

void
DMAudio_ResetPoliceRadio(void)
{
	// Reset the live police radio through its C interface
	AudioPolice_Reset(&AudioManager);
}
//- rouz edit (ChatGPT)
