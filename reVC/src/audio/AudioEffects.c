//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioScriptObject.h"
#include "Pools.h"
#include "sampman.h"

void
AudioEffects_Service(cAudioManager *manager)
{
	// Preserve the original service order and live owner state reads
#ifdef FIX_BUGS
	if(AudioPoliceHost_LogicalFramesPassed() != 0)
#endif
	manager->m_bReduceReleasingPriority = (manager->m_FrameCounter++ % 5) == 0;
	if (manager->m_bIsPaused && !manager->m_bWasPaused) {
#ifdef GTA_PS2
		if (manager->m_bIsSurround) {
			for (uint32_t i = 0; i < NUM_CHANNELS_DTS_GENERIC; i++)
				SampleManager_StopChannel(&SampleManager, i);

			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_DTS_POLICE_RADIO, 0);
			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_DTS_MISSION_AUDIO_1, 0);
			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_DTS_MISSION_AUDIO_2, 0);
			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_DTS_PLAYER_VEHICLE_ENGINE, 0);
		} else {
			for (uint32_t i = 0; i < NUM_CHANNELS_GENERIC; i++)
				SampleManager_StopChannel(&SampleManager, i);

			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_POLICE_RADIO, 0);
			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_MISSION_AUDIO_1, 0);
			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_MISSION_AUDIO_2, 0);
			SampleManager_SetChannelFrequency(&SampleManager, CHANNEL_PLAYER_VEHICLE_ENGINE, 0);
		}
#else
		for (uint32_t i = 0; i < NUM_CHANNELS; i++)
			SampleManager_StopChannel(&SampleManager, i);
#endif
		AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
		if (manager->m_nActiveQueue) {
			manager->m_nActiveQueue = 0;
			AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
			manager->m_nActiveQueue = 1;
		} else {
			manager->m_nActiveQueue = 1;
			AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
			manager->m_nActiveQueue = 0;
		}
		AudioSoundQueue_ClearActive(manager->m_asActiveSamples, manager->m_nActiveSamples);
	}
	manager->m_nActiveQueue = manager->m_nActiveQueue == 1 ? 0 : 1;
#ifdef AUDIO_REVERB
	if(manager->m_bIsSurround) AudioEnvironment_Reverb(manager);
#endif
	AudioEnvironment_Special(manager);
	AudioSoundQueue_ClearRequested(manager->m_aRequestedOrderList[manager->m_nActiveQueue], &manager->m_nRequestedCount[manager->m_nActiveQueue], manager->m_nActiveSamples);
	AudioEffects_Interrogate(manager);
	PedComments_Process(&manager->m_sPedComments);
	AudioPolice_Service(manager);
	AudioCollisionService_Run(manager);
	AudioRelease_Add(manager);
	AudioMission_Process(manager);
#ifdef EXTERNAL_3D_SOUND
	AudioSoundQueue_AdjustVolume(manager->m_aRequestedQueue[manager->m_nActiveQueue], manager->m_aRequestedOrderList[manager->m_nActiveQueue], manager->m_nRequestedCount[manager->m_nActiveQueue]);
#endif
	AudioActive_Process(manager);
#ifdef AUDIO_OAL
	SampleManager_Service(&SampleManager);
#endif
	for (int32_t i = 0; i < manager->m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal; i++) {
		cAudioScriptObject *object = (cAudioScriptObject *)manager->m_asAudioEntities[manager->m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices[i]].m_pEntity;
		// Reset and release the pooled audio script object
		AudioScriptObject_Reset(object);
		// Access raw storage through the C store or pool API
		CPool_Delete(CPools_GetAudioScriptObjectPool(), object);
		manager->m_asAudioEntities[manager->m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices[i]].m_pEntity = NULL;
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_sAudioScriptObjectManager.m_anScriptObjectEntityIndices[i]);
	}
	// Reset the active script-object count through the C state API
	AudioScriptObjectManager_Reset(&manager->m_sAudioScriptObjectManager);
}

void
AudioEffects_Interrogate(cAudioManager *manager)
{
	// Preserve the original service order and live owner state reads
	for (uint32_t i = 0; i < manager->m_nAudioEntitiesCount; i++) {
		AudioDispatch_Entity(manager, manager->m_aAudioEntityOrderList[i]);
		manager->m_asAudioEntities[manager->m_aAudioEntityOrderList[i]].m_AudioEvents = 0;
	}
}
//- rouz edit (ChatGPT)
