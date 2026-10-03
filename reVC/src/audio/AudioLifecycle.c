//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"

void
AudioLifecycle_Initialise(cAudioManager *manager)
{
	// Preserve the original lifecycle gates, partial writes and callback order
	if (!manager->m_bIsInitialised) {
		AudioLifecycleHost_PreInitialise(manager);
		manager->m_bIsInitialised = SampleManager_Initialise(&SampleManager);
		if (manager->m_bIsInitialised) {
#ifdef EXTERNAL_3D_SOUND
			manager->m_nActiveSamples = SampleManager_GetMaximumSupportedChannels(&SampleManager);
			if (manager->m_nActiveSamples <= 1) {
				AudioLifecycle_Terminate(manager);
			} else {
				manager->m_nActiveSamples--;
#else
			{
				manager->m_nActiveSamples = NUM_CHANNELS_GENERIC;
#endif
				AudioLifecycleHost_PostInitialise(manager);
				AudioPolice_InitZones();
				AudioPolice_Init(manager);
				AudioLifecycleHost_MusicInitialise();
			}
		}
	}
}

void
AudioLifecycle_Terminate(cAudioManager *manager)
{
	// Preserve the original lifecycle gates, partial writes and callback order
	if (manager->m_bIsInitialised) {
		AudioLifecycleHost_MusicTerminate();

		for (uint32_t i = 0; i < NUM_AUDIOENTITIES; i++) {
			manager->m_asAudioEntities[i].m_bIsUsed = 0;
			manager->m_aAudioEntityOrderList[i] = (sizeof(manager->m_aAudioEntityOrderList) / sizeof(manager->m_aAudioEntityOrderList[0]));
		}

		manager->m_nAudioEntitiesCount = 0;
		// Reset the active script-object count through the C state API
		AudioScriptObjectManager_Reset(&manager->m_sAudioScriptObjectManager);
		AudioLifecycleHost_PreTerminate(manager);

		for (uint32_t i = 0; i < MAX_SFX_BANKS; i++) {
			if (SampleManager_IsSampleBankLoaded(&SampleManager, i))
				SampleManager_UnloadSampleBank(&SampleManager, i);
		}

		SampleManager_Terminate(&SampleManager);

		manager->m_bIsInitialised = 0;
		AudioLifecycleHost_PostTerminate(manager);
	}
}

void
DMAudio_Initialise(void)
{
	// Apply the lifecycle operation to the live global owner through C
	AudioLifecycle_Initialise(&AudioManager);
}

void
DMAudio_Terminate(void)
{
	// Apply the lifecycle operation to the live global owner through C
	AudioLifecycle_Terminate(&AudioManager);
}

//- rouz edit (ChatGPT)
