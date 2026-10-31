//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "sampman.h"

void
AudioGameSetup_PreInitialise(cAudioManager *manager)
{
	// Preserve the complete original setup or shutdown operation and live write order
	(void)manager;
	BankStartOffset[SFX_BANK_0] = SAMPLEBANK_START;
#ifdef GTA_PS2
	BankStartOffset[SAMPLEBANK_CAR_PACARD] = SFX_CAR_ACCEL_1;
	BankStartOffset[SAMPLEBANK_CAR_PATHFINDER] = SFX_CAR_ACCEL_2;
	BankStartOffset[SAMPLEBANK_CAR_PORSCHE] = SFX_CAR_ACCEL_3;
	BankStartOffset[SAMPLEBANK_CAR_SPIDER] = SFX_CAR_ACCEL_4;
	BankStartOffset[SAMPLEBANK_CAR_MERC] = SFX_CAR_ACCEL_5;
	BankStartOffset[SAMPLEBANK_CAR_MACKTRUCK] = SFX_CAR_ACCEL_6;
	BankStartOffset[SAMPLEBANK_CAR_HOTROD] = SFX_CAR_ACCEL_7;
	BankStartOffset[SAMPLEBANK_CAR_COBRA] = SFX_CAR_ACCEL_8;
	BankStartOffset[SAMPLEBANK_CAR_NONE] = SFX_CAR_ACCEL_9;
	BankStartOffset[SAMPLEBANK_FRONTEND] = SFX_PAGE_CHANGE_AND_BACK_LEFT;
	BankStartOffset[SAMPLEBANK_TRAIN] = SFX_TRAIN_STATION_AMBIENCE_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_1] = SFX_CLUB_1;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_2] = SFX_CLUB_2;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_3] = SFX_CLUB_3;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_4] = SFX_CLUB_4;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_5] = SFX_CLUB_5;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_6] = SFX_CLUB_6;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_7] = SFX_CLUB_7;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_8] = SFX_CLUB_8;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_9] = SFX_CLUB_9;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_10] = SFX_CLUB_10;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_11] = SFX_CLUB_11;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_12] = SFX_CLUB_12;
	BankStartOffset[SAMPLEBANK_BUILDING_CLUB_RAGGA] = SFX_CLUB_RAGGA;
	BankStartOffset[SAMPLEBANK_BUILDING_STRIP_CLUB_1] = SFX_STRIP_CLUB_1;
	BankStartOffset[SAMPLEBANK_BUILDING_STRIP_CLUB_2] = SFX_STRIP_CLUB_2;
	BankStartOffset[SAMPLEBANK_BUILDING_WORKSHOP] = SFX_WORKSHOP_1;
	BankStartOffset[SAMPLEBANK_BUILDING_PIANO_BAR] = SFX_PIANO_BAR_1;
	BankStartOffset[SAMPLEBANK_BUILDING_SAWMILL] = SFX_SAWMILL_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_DOG_FOOD_FACTORY] = SFX_DOG_FOOD_FACTORY;
	BankStartOffset[SAMPLEBANK_BUILDING_LAUNDERETTE] = SFX_LAUNDERETTE_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_CHINATOWN] = SFX_RESTAURANT_CHINATOWN;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_ITALY] = SFX_RESTAURANT_ITALY;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_GENERIC_1] = SFX_RESTAURANT_GENERIC_1;
	BankStartOffset[SAMPLEBANK_BUILDING_RESTAURANT_GENERIC_2] = SFX_RESTAURANT_GENERIC_2;
	BankStartOffset[SAMPLEBANK_BUILDING_AIRPORT] = SFX_AIRPORT_ANNOUNCEMENT_1;
	BankStartOffset[SAMPLEBANK_BUILDING_SHOP] = SFX_SHOP_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_CINEMA] = SFX_CINEMA_BASS_1;
	BankStartOffset[SAMPLEBANK_BUILDING_DOCKS] = SFX_DOCKS_FOGHORN;
	BankStartOffset[SAMPLEBANK_BUILDING_HOME] = SFX_HOME_1;
	BankStartOffset[SAMPLEBANK_BUILDING_PORN_1] = SFX_PORN_1_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_PORN_2] = SFX_PORN_2_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_PORN_3] = SFX_PORN_3_LOOP;
	BankStartOffset[SAMPLEBANK_BUILDING_POLICE_BALL] = SFX_POLICE_BALL_1;
	BankStartOffset[SAMPLEBANK_BUILDING_BANK_ALARM] = SFX_BANK_ALARM_1;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_INDUSTRIAL] = SFX_RAVE_INDUSTRIAL;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_COMMERCIAL] = SFX_RAVE_COMMERCIAL;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_SUBURBAN] = SFX_RAVE_SUBURBAN;
	BankStartOffset[SAMPLEBANK_BUILDING_RAVE_COMMERCIAL_2] = SFX_RAVE_COMMERCIAL_2;
	BankStartOffset[SAMPLEBANK_BUILDING_39] = SFX_CLUB_1_1;
	BankStartOffset[SAMPLEBANK_BUILDING_40] = SFX_CLUB_1_2;
	BankStartOffset[SAMPLEBANK_BUILDING_41] = SFX_CLUB_1_3;
	BankStartOffset[SAMPLEBANK_BUILDING_42] = SFX_CLUB_1_4;
	BankStartOffset[SAMPLEBANK_BUILDING_43] = SFX_CLUB_1_5;
	BankStartOffset[SAMPLEBANK_BUILDING_44] = SFX_CLUB_1_6;
	BankStartOffset[SAMPLEBANK_BUILDING_45] = SFX_CLUB_1_7;
	BankStartOffset[SAMPLEBANK_BUILDING_46] = SFX_CLUB_1_8;
	BankStartOffset[SAMPLEBANK_BUILDING_47] = SFX_CLUB_1_9;
	BankStartOffset[SAMPLEBANK_EXTRAS] = SFX_EXPLOSION_1;
#endif // GTA_PS2
	BankStartOffset[SFX_BANK_PED_COMMENTS] = SAMPLEBANK_PED_START;
}

void
AudioGameSetup_PostInitialise(cAudioManager *manager)
{
	// Preserve the complete original setup or shutdown operation and live write order
	manager->m_nFireAudioEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_FIRE, AudioGameSetupHost_FireManager());
	if (manager->m_nFireAudioEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nFireAudioEntity, 1);

	manager->m_nCollisionEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_COLLISION, (void *)1);
	if (manager->m_nCollisionEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nCollisionEntity, 1);

	manager->m_nFrontEndEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_FRONTEND, (void *)1);
	if (manager->m_nFrontEndEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nFrontEndEntity, 1);

	manager->m_nProjectileEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_PROJECTILE, (void *)1);
	if (manager->m_nProjectileEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nProjectileEntity, 1);

	manager->m_nWaterCannonEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_WATERCANNON, (void *)1);
	if (manager->m_nWaterCannonEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nWaterCannonEntity, 1);

	manager->m_nPoliceChannelEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_POLICERADIO, (void *)1);
	if (manager->m_nPoliceChannelEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nPoliceChannelEntity, 1);

	manager->m_nEscalatorEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_ESCALATOR, (void*)1);
	if (manager->m_nEscalatorEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nEscalatorEntity, 1);

	manager->m_nExtraSoundsEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_EXTRA_SOUNDS, (void*)1);
	if (manager->m_nExtraSoundsEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nExtraSoundsEntity, 1);

#ifdef GTA_BRIDGE
	manager->m_nBridgeEntity = AudioManager_CreateEntity(manager, AUDIOTYPE_BRIDGE, (void*)1);
	if (manager->m_nBridgeEntity >= 0)
		AudioEntities_SetStatus(manager->m_asAudioEntities, manager->m_bIsInitialised, manager->m_nBridgeEntity, 1);
#endif // GTA_BRIDGE

	for (int i = 0; i < MISSION_AUDIO_SLOTS; i++) {
		manager->m_nMissionAudioSampleIndex[i] = NO_SAMPLE;
		manager->m_nMissionAudioLoadingStatus[i] = LOADING_STATUS_NOT_LOADED;
		manager->m_nMissionAudioPlayStatus[i] = PLAY_STATUS_STOPPED;
		manager->m_bIsMissionAudioPlaying[i] = 0;
		manager->m_bIsMissionAudioAllowedToPlay[i] = 0;
		manager->m_bIsMissionAudio2D[i] = 1;
		manager->m_nMissionAudioFramesToPlay[i] = 0;
		manager->m_bIsMissionAudioPhoneCall[i] = 0;
		manager->m_nGlobalSfxVolumeMultiplier = 127;
	}

	AudioService_ResetLogicTimers(manager, AudioPlayerEngineHost_Time());
	manager->m_bIsPlayerShutUp = 0;
	manager->m_nPlayerMood = PLAYER_MOOD_CALM;
	manager->m_nPlayerMoodTimer = 0;
}

void
AudioGameSetup_PreTerminate(cAudioManager *manager)
{
	// Preserve the complete original setup or shutdown operation and live write order
#ifdef GTA_BRIDGE
	if (manager->m_nBridgeEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nBridgeEntity);
		manager->m_nBridgeEntity = AEHANDLE_NONE;
	}
#endif
	if (manager->m_nEscalatorEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nEscalatorEntity);
		manager->m_nEscalatorEntity = AEHANDLE_NONE;
	}
	if (manager->m_nExtraSoundsEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nExtraSoundsEntity);
		manager->m_nExtraSoundsEntity = AEHANDLE_NONE;
	}
	if (manager->m_nPoliceChannelEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nPoliceChannelEntity);
		manager->m_nPoliceChannelEntity = AEHANDLE_NONE;
	}
	if (manager->m_nWaterCannonEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nWaterCannonEntity);
		manager->m_nWaterCannonEntity = AEHANDLE_NONE;
	}
	if (manager->m_nFireAudioEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nFireAudioEntity);
		manager->m_nFireAudioEntity = AEHANDLE_NONE;
	}
	if (manager->m_nCollisionEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nCollisionEntity);
		manager->m_nCollisionEntity = AEHANDLE_NONE;
	}
	if (manager->m_nFrontEndEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nFrontEndEntity);
		manager->m_nFrontEndEntity = AEHANDLE_NONE;
	}
	if (manager->m_nProjectileEntity >= 0) {
		AudioEntities_Destroy(manager->m_asAudioEntities, manager->m_aAudioEntityOrderList, &manager->m_nAudioEntitiesCount, manager->m_bIsInitialised, manager->m_nProjectileEntity);
		manager->m_nProjectileEntity = AEHANDLE_NONE;
	}
}

void
AudioGameSetup_PostTerminate(cAudioManager *manager)
{
	// Preserve the complete original setup or shutdown operation and live write order
	(void)manager;
	;
}

//- rouz edit (ChatGPT)
