//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "sampman.h"

#ifndef GTA_PS2
#define CHANNEL_PLAYER_VEHICLE_ENGINE manager->m_nActiveSamples
#endif

void
AudioEnvironment_Reverb(cAudioManager *manager)
{
	// Preserve the original game-service gates and backend callback order
#ifdef AUDIO_REVERB
#ifdef FIX_BUGS
	const uint32_t numChannels = NUM_CHANNELS_GENERIC;
#else
	const uint32_t numChannels = NUM_CHANNELS_GENERIC+1;
#endif

	if (SampleManager_UpdateReverb(&SampleManager) && manager->m_bDynamicAcousticModelingStatus) {
#ifndef GTA_PS2
		for (uint32_t i = 0; i < numChannels; i++) {
			if (manager->m_asActiveSamples[i].m_bReverb)
				SampleManager_SetChannelReverbFlag(&SampleManager, i, 1);
		}
#endif
	}
#endif // AUDIO_REVERB
}

void
AudioEnvironment_Special(cAudioManager *manager)
{
	// Preserve the original game-service gates and backend callback order
	CPlayerPed *playerPed;
	CVehicle *remoteVehicle;

	if (manager->m_bIsPaused) {
		if (!manager->m_bWasPaused) {
			SampleManager_SetEffectsFadeVolume(&SampleManager, MAX_VOLUME);
			SampleManager_SetMusicFadeVolume(&SampleManager, MAX_VOLUME);
		}
	} else {
		if (!AudioEnvironmentHost_Replay())
			AudioEnvironmentHost_Mood(manager);
		remoteVehicle = AudioEnvironmentHost_Remote();
		playerPed = AudioPoliceHost_FindPlayer();
		if (playerPed) {
			if (AudioEnvironmentHost_EntityId(playerPed) >= 0 && manager->m_asAudioEntities[AudioEnvironmentHost_EntityId(playerPed)].m_bIsUsed) {
				if(!AudioEnvironmentHost_Entering(playerPed) && !AudioEnvironmentHost_InVehicle(playerPed) && !remoteVehicle)
					SampleManager_StopChannel(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE);
			}
		}
	}
}
#ifndef GTA_PS2
#undef CHANNEL_PLAYER_VEHICLE_ENGINE
#endif
//- rouz edit (ChatGPT)
