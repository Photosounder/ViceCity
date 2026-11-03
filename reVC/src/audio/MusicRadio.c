//+ rouz edit (ChatGPT)
#include "ReplayState.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"
#include "oal/AudioSampleHost.h"
#include "../modelinfo/ModelIds.h"
#include "../peds/PedStates.h"
#include "../entities/EntityStatus.h"
#include <stddef.h>

uint8_t
MusicRadio_PlayerInCar(cMusicManager *manager)
{
	// Preserve the original radio gates, live vehicle queries and station writes
	CVehicle *vehicle = AudioGame_FindVehicle();
	if(!vehicle)
		return 0;

	int32_t State = AudioPedTalkHost_PlayerState();

	if(State == PED_DRAG_FROM_CAR || State == PED_EXIT_CAR || State == PED_ARRESTED)
		return 0;

	if (AudioVehicleEngineHost_Status(vehicle) == STATUS_WRECKED)
		return 0;

	return 1;
}

uint32_t
MusicRadio_GetRadioInCar(cMusicManager *manager)
{
	// Preserve the original radio gates, live vehicle queries and station writes
	if (!manager->m_bIsInitialised) return WILDSTYLE;
	if (MusicRadio_PlayerInCar(manager)) {
		CVehicle* veh = AudioGame_FindVehicle();
		if (veh != NULL) {
			if (MusicRadio_UsesPoliceRadio(veh) || MusicRadio_UsesTaxiRadio(veh)) {
				if (manager->m_nRadioInCar == NO_TRACK || (Replay_IsPlayingBack() && !AudioManager.m_bIsPaused))
					return STREAMED_SOUND_RADIO_POLICE;
				return manager->m_nRadioInCar;
			}
			else return (*MusicRadioHost_Station(veh));
		}
	}

	if (manager->m_nRadioInCar == NO_TRACK || (Replay_IsPlayingBack() && !AudioManager.m_bIsPaused))
		return RADIO_OFF;
	return manager->m_nRadioInCar;
}

void
MusicRadio_SetRadioInCar(cMusicManager *manager, uint32_t station)
{
	// Preserve the original radio gates, live vehicle queries and station writes
	if (manager->m_bIsInitialised) {
		if (!MusicRadio_PlayerInCar(manager)) {
			manager->m_nRadioInCar = station;
			return;
		}
		CVehicle* veh = AudioGame_FindVehicle();
		if (veh == NULL) return;
		if (MusicRadio_UsesPoliceRadio(veh) || MusicRadio_UsesTaxiRadio(veh))
			manager->m_nRadioInCar = station;
		else
			(*MusicRadioHost_Station(veh)) = station;
	}
}

uint32_t
MusicRadio_GetNextCarTuning(cMusicManager *manager)
{
	// Preserve the original radio gates, live vehicle queries and station writes
	CVehicle *veh = AudioGame_FindVehicle();
	if (veh == NULL) return STREAMED_SOUND_CITY_AMBIENT;
	if (MusicRadio_UsesPoliceRadio(veh)) return STREAMED_SOUND_RADIO_POLICE;
	if (MusicRadio_UsesTaxiRadio(veh)) return STREAMED_SOUND_RADIO_TAXI;
	if (gNumRetunePresses != 0) {
#ifdef RADIO_SCROLL_TO_PREV_STATION
		// manager->m_nRadioStation is unsigned, so...
		int station = (*MusicRadioHost_Station(veh)) + gNumRetunePresses;
		while(station < 0) station += NUM_RADIOS + 1;
		while(station >= NUM_RADIOS + 1) station -= NUM_RADIOS + 1;
		(*MusicRadioHost_Station(veh)) = station;
#else
		(*MusicRadioHost_Station(veh)) += gNumRetunePresses;
		while((*MusicRadioHost_Station(veh)) >= NUM_RADIOS + 1)
			(*MusicRadioHost_Station(veh)) -= NUM_RADIOS + 1;
#endif
		DMAudio_IsMP3RadioChannelAvailable(); // woof, just call and do nothing =P they manipulate gNumRetunePresses on DisplayRadioStationName in this case
		gNumRetunePresses = 0;
	}
	return (*MusicRadioHost_Station(veh));
}

uint32_t
MusicRadio_GetCarTuning(cMusicManager *manager)
{
	// Preserve the original radio gates, live vehicle queries and station writes
	CVehicle* veh = AudioGame_FindVehicle();
	if (veh == NULL) return STREAMED_SOUND_CITY_AMBIENT;
	if (MusicRadio_UsesPoliceRadio(veh)) return STREAMED_SOUND_RADIO_POLICE;
	if (MusicRadio_UsesTaxiRadio(veh)) return STREAMED_SOUND_RADIO_TAXI;
	if ((*MusicRadioHost_Station(veh)) == USERTRACK && !SampleManager_IsMP3RadioChannelAvailable(&SampleManager))
		(*MusicRadioHost_Station(veh)) = AudioManager.m_anRandomTable[2] % USERTRACK;
	return (*MusicRadioHost_Station(veh));
}

uint8_t
MusicRadio_UsesPoliceRadio(CVehicle *veh)
{
	// Preserve the original radio gates, live vehicle queries and station writes
	switch (AudioVehicleModelHost_Model(veh))
	{
	case MI_VCNMAV:
	case MI_POLMAV:
	case MI_COASTG:
	case MI_RHINO:
	case MI_BARRACKS:
		return 1;
	case MI_MRWHOOP:
	case MI_HUNTER:
		return 0;
	}
	return MusicRadioHost_UsesSiren(veh);
}

uint8_t
MusicRadio_UsesTaxiRadio(CVehicle *veh)
{
	// Preserve the original radio gates, live vehicle queries and station writes
	if (AudioVehicleModelHost_Model(veh) != MI_KAUFMAN) return 0;
	return MusicRadioHost_HasMetDebbie();
}

uint8_t
DMAudio_GetRadioInCar(void)
{
	// Preserve the original game audio operation through the C interface
	return MusicRadio_GetRadioInCar(&MusicManager);
}

void
DMAudio_SetRadioInCar(uint32_t radio)
{
	// Preserve the original game audio operation through the C interface
	MusicRadio_SetRadioInCar(&MusicManager, radio);
}

#if defined(AUDIO_OAL) || defined(AUDIO_MSS)
int
AudioSample_GetRadioInCar(void)
{
    // Query car radio selection at the existing stream-worker boundary
    return MusicRadio_GetRadioInCar(&MusicManager);
}
#endif
//- rouz edit (ChatGPT)
