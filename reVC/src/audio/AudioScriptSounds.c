//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "MusicManager.h"
#include "AudioSamples.h"
#include "AudioScriptObject.h"
#include "soundlist.h"
#include "sampman.h"
#include "../core/SurfaceTypes.h"
#include <math.h>

enum {
	SCRIPT_OBJECT_SEAPLANE_LOW_FUEL_MAX_DIST = 1000,
	SCRIPT_OBJECT_SEAPLANE_LOW_FUEL_VOLUME = 100,

	SCRIPT_OBJECT_WILLIE_CARD_SWIPE_MAX_DIST = 40,
	SCRIPT_OBJECT_WILLIE_CARD_SWIPE_VOLUME = 70,

	SCRIPT_OBJECT_GATE_MAX_DIST = 40,

	SCRIPT_OBJECT_BULLET_HIT_GROUND_MAX_DIST = 50,
	SCRIPT_OBJECT_BULLET_HIT_GROUND_VOLUME = 90,

	SCRIPT_OBJECT_PAYPHONE_RINGING_MAX_DIST = 80,
	SCRIPT_OBJECT_PAYPHONE_RINGING_VOLUME = 80,

	SCRIPT_OBJECT_GLASS_BREAK_MAX_DIST = 60,
	SCRIPT_OBJECT_GLASS_BREAK_LONG_VOLUME = 70,
	SCRIPT_OBJECT_GLASS_BREAK_SHORT_VOLUME = 60,

	SCRIPT_OBJECT_GLASS_LIGHT_BREAK_MAX_DIST = 55,
	SCRIPT_OBJECT_GLASS_LIGHT_BREAK_VOLUME = 25,

	SCRIPT_OBJECT_BOX_DESTROYED_MAX_DIST = 60,
	SCRIPT_OBJECT_BOX_DESTROYED_VOLUME = 80,

	SCRIPT_OBJECT_METAL_COLLISION_VOLUME = 70,
	SCRIPT_OBJECT_TIRE_COLLISION_VOLUME = 60,
	SCRIPT_OBJECT_HIT_BALL_VOLUME = 60,

	SCRIPT_OBJECT_GUNSHELL_MAX_DIST = 20,
	SCRIPT_OBJECT_GUNSHELL_VOLUME = 30,

	SCRIPT_OBJECT_POLICE_CELL_DOOR_CLUNK_MAX_DIST = 40,
	SCRIPT_OBJECT_POLICE_CELL_DOOR_CLUNK_VOLUME = 60,

	SCRIPT_OBJECT_GARAGE_DOOR_CLUNK_MAX_DIST = 80,
	SCRIPT_OBJECT_GARAGE_DOOR_CLUNK_VOLUME = 60,

	//SCRIPT_OBJECT_SHORT_MAX_DIST = 30,
	SCRIPT_OBJECT_LONG_MAX_DIST = 80,
	SCRIPT_OBJECT_DEFAULT_VOLUME = MAX_VOLUME,
	SCRIPT_OBJECT_BANK_ALARM_VOLUME = 90,
	SCRIPT_OBJECT_SNORING_MAX_DIST = 6,
	SCRIPT_OBJECT_SNORING_VOLUME = 25,
	SCRIPT_OBJECT_GARAGE_DOOR_SLIDING_VOLUME = 90,
	SCRIPT_OBJECT_SHOOTING_RANGE_TARGET_MOVING_MAX_DIST = 40,
	SCRIPT_OBJECT_SHOOTING_RANGE_TARGET_MOVING_VOLUME = 60,
	SCRIPT_OBJECT_NEW_WATERFALL_VOLUME = 30,

};

#ifndef GTA_PS2
#define AUDIO_SCRIPT_SOUNDS_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_SCRIPT_SOUNDS_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_SCRIPT_SOUNDS_RESET_LOOP_OFFSETS
#define AUDIO_SCRIPT_SOUNDS_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_SCRIPT_SOUNDS_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_SCRIPT_SOUNDS_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_SCRIPT_SOUNDS_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_SCRIPT_SOUNDS_SET_SOUND_REVERB(b)
#endif

void
AudioScriptSounds_Process(cAudioManager *manager, int32_t id)
{
	// Preserve the complete original script sound decisions and live query order
	if (MusicState_GetMusicMode(&MusicManager) == MUSICMODE_GAME) {
		cAudioScriptObject *entity = (cAudioScriptObject*)manager->m_asAudioEntities[id].m_pEntity;
		if (entity != NULL) {
			// Copy the C record position into the game audio vector
			manager->m_sQueueSample.m_vecPos.x = entity->Posn.x;
			manager->m_sQueueSample.m_vecPos.y = entity->Posn.y;
			manager->m_sQueueSample.m_vecPos.z = entity->Posn.z;
			if (manager->m_asAudioEntities[id].m_AudioEvents == 1)
				AudioScriptSounds_OneShot(manager, manager->m_asAudioEntities[id].m_awAudioEvent[0]);
			else
				AudioScriptSounds_Looping(manager, entity->AudioId);
		}
	}
}

void
AudioScriptSounds_OneShot(cAudioManager *manager, uint8_t sound)
{
	// Preserve the complete original script sound decisions and live query order
	CPed *playerPed;
	uint8_t Vol;
	float distSquared;

	static uint8_t iSound = 0;

	switch (sound) {
	case SCRIPT_SOUND_SEAPLANE_LOW_FUEL:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_SEAPLANE_LOW_FUEL_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_SEAPLANE_LOW;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		Vol = SCRIPT_OBJECT_SEAPLANE_LOW_FUEL_VOLUME;
#ifdef FIX_BUGS
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_SEAPLANE_LOW);
#else
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_CAR_HORN_JEEP);
#endif
		manager->m_sQueueSample.m_nPriority = 1;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 1;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(0);
		break;
	case SCRIPT_SOUND_WILLIE_CARD_SWIPE:
		Vol = SCRIPT_OBJECT_WILLIE_CARD_SWIPE_VOLUME;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_WILLIE_CARD_SWIPE_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BOMB_BEEP;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = 20159;
		manager->m_sQueueSample.m_nPriority = 1;
		manager->m_sQueueSample.m_fSpeedMultiplier = 1.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(0);
		break;
	case SCRIPT_SOUND_BOX_DESTROYED_1:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_BOX_DESTROYED_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_WOODEN_BOX_SMASH;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1500) + 18600;
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		Vol = manager->m_anRandomTable[2] % 20 + SCRIPT_OBJECT_BOX_DESTROYED_VOLUME;
		break;
	case SCRIPT_SOUND_BOX_DESTROYED_2:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_BOX_DESTROYED_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_CARDBOARD_BOX_SMASH;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1500) + 18600;
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		Vol = manager->m_anRandomTable[2] % 20 + SCRIPT_OBJECT_BOX_DESTROYED_VOLUME;
		break;
	case SCRIPT_SOUND_METAL_COLLISION:
		manager->m_sQueueSample.m_MaxDistance = COLLISION_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[3] % 5 + SFX_COL_CAR_1;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		Vol = manager->m_anRandomTable[2] % 30 + SCRIPT_OBJECT_METAL_COLLISION_VOLUME;
		break;
	case SCRIPT_SOUND_TIRE_COLLISION:
		manager->m_sQueueSample.m_MaxDistance = COLLISION_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_TYRE_BUMP;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		Vol = manager->m_anRandomTable[2] % 30 + SCRIPT_OBJECT_TIRE_COLLISION_VOLUME;
		break;
	case SCRIPT_SOUND_HIT_BALL:
		manager->m_sQueueSample.m_MaxDistance = COLLISION_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_HIT_BALL;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
		manager->m_sQueueSample.m_nPriority = 5;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		Vol = manager->m_anRandomTable[2] % 30 + SCRIPT_OBJECT_HIT_BALL_VOLUME;
		break;
	case SCRIPT_SOUND_GUNSHELL_DROP:
		playerPed = AudioPedOneShotsHost_Player();
		if (playerPed) {
			switch (AudioPedOneShotsHost_Surface(playerPed)) {
			case SURFACE_GRASS:
			case SURFACE_GRAVEL:
			case SURFACE_MUD_DRY:
			case SURFACE_TRANSPARENT_CLOTH:
			case SURFACE_PED:
			case SURFACE_SAND:
			case SURFACE_RUBBER:
			case SURFACE_HEDGE:
			case SURFACE_SAND_BEACH:
				manager->m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_2;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 600) + 10600;
				manager->m_sQueueSample.m_nPriority = 18;
				break;
			case SURFACE_WATER:
				return;
			default:
				manager->m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_1;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1500) + 30000;
				manager->m_sQueueSample.m_nPriority = 15;
				break;
			}
		} else {
			manager->m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_1;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1500) + 30000;
			manager->m_sQueueSample.m_nPriority = 15;
		}
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_GUNSHELL_MAX_DIST;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		Vol = manager->m_anRandomTable[2] % 20 + SCRIPT_OBJECT_GUNSHELL_VOLUME;
		break;
	case SCRIPT_SOUND_GUNSHELL_DROP_SOFT:
		manager->m_sQueueSample.m_nSampleIndex = SFX_BULLET_SHELL_HIT_GROUND_2;
		manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 500) + 11000;
		manager->m_sQueueSample.m_nPriority = 18;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_GUNSHELL_MAX_DIST;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		Vol = manager->m_anRandomTable[2] % 20 + SCRIPT_OBJECT_GUNSHELL_VOLUME;
		break;
	case SCRIPT_SOUND_SHOOTING_RANGE_TARGET_HIT:
	case SCRIPT_SOUND_BULLET_HIT_GROUND_1:
	case SCRIPT_SOUND_BULLET_HIT_GROUND_2:
	case SCRIPT_SOUND_BULLET_HIT_GROUND_3:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_BULLET_HIT_GROUND_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[iSound % 5] % 3 + SFX_BULLET_WALL_1;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
		manager->m_sQueueSample.m_nPriority = 9;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		Vol = manager->m_anRandomTable[2] % 20 + SCRIPT_OBJECT_BULLET_HIT_GROUND_VOLUME;
		break;
	case SCRIPT_SOUND_PAYPHONE_RINGING:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_PAYPHONE_RINGING_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_PHONE_RING;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		Vol = SCRIPT_OBJECT_PAYPHONE_RINGING_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_PHONE_RING);
		manager->m_sQueueSample.m_nPriority = 1;
		manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(0);
		break;
	case SCRIPT_SOUND_GLASS_BREAK_L:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_GLASS_BREAK_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_GLASS_SMASH;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		Vol = SCRIPT_OBJECT_GLASS_BREAK_LONG_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_GLASS_SMASH);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_GLASS_BREAK_S:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_GLASS_BREAK_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_GLASS_SMASH;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		Vol = SCRIPT_OBJECT_GLASS_BREAK_SHORT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_GLASS_SMASH);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_GLASS_CRACK:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_GLASS_BREAK_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_GLASS_CRACK;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		Vol = SCRIPT_OBJECT_GLASS_BREAK_LONG_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_GLASS_CRACK);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		break;
	case SCRIPT_SOUND_GLASS_LIGHT_BREAK:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_GLASS_LIGHT_BREAK_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = (manager->m_anRandomTable[4] & 3) + SFX_GLASS_SHARD_1;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 2000) + 19000;
		manager->m_sQueueSample.m_nPriority = 9;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		Vol = AudioMath_RandomDisplacement(manager->m_anRandomTable, 11) + SCRIPT_OBJECT_GLASS_LIGHT_BREAK_VOLUME;
		break;
	case SCRIPT_SOUND_MALE_AMBULANCE_OUCH:
	{
		// Initialize the audio context explicitly before processing it
		cPedParams pedParams;
		PedParams_Init(&pedParams);
		pedParams.m_fDistance = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
		AudioPedSpeech_Setup(manager, &pedParams, SOUND_INJURED_PED_MALE_OUCH);
		return;
	}
	case SCRIPT_SOUND_FEMALE_AMBULANCE_OUCH:
	{
		// Initialize the audio context explicitly before processing it
		cPedParams pedParams;
		PedParams_Init(&pedParams);
		pedParams.m_fDistance = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
		AudioPedSpeech_Setup(manager, &pedParams, SOUND_INJURED_PED_FEMALE);
		return;
	}
	case SCRIPT_SOUND_POLICE_CELL_DOOR_CLUNK:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_POLICE_CELL_DOOR_CLUNK_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_COL_GATE;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = 10600;
		manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
		manager->m_sQueueSample.m_nPriority = 3;
		Vol = SCRIPT_OBJECT_POLICE_CELL_DOOR_CLUNK_VOLUME;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		break;
	case SCRIPT_SOUND_GARAGE_DOOR_CLUNK:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_GARAGE_DOOR_CLUNK_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_COL_CAR_PANEL_2; // huh?
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = 22000;
		manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
		manager->m_sQueueSample.m_nPriority = 4;
		Vol = SCRIPT_OBJECT_GARAGE_DOOR_CLUNK_VOLUME;
		manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(1);
		break;
	default:
		return;
	}

	distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
	if (distSquared < (manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)) {
		manager->m_sQueueSample.m_fDistance = (distSquared <= 0.0f ? 0.0f : sqrtf(distSquared));
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = iSound++;
			manager->m_sQueueSample.m_nLoopCount = 1;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_SCRIPT_SOUNDS_SET_EMITTING_VOLUME(Vol);
			AUDIO_SCRIPT_SOUNDS_RESET_LOOP_OFFSETS
			AUDIO_SCRIPT_SOUNDS_SET_SOUND_REVERB(1);
			AudioRequests_Submit(manager);
		}
	}
}

void
AudioScriptSounds_Looping(cAudioManager *manager, uint8_t sound)
{
	// Preserve the complete original script sound decisions and live query order
	uint8_t Vol;
	float distSquared;

	switch(sound) {
	case SCRIPT_SOUND_NEW_BUILDING_BAR_1:
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_BAR_1;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_BAR_1;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_BAR_2:
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_BAR_2;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_BAR_2;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_BAR_3:
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_BAR_3;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_BAR_3;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_BAR_4:
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_BAR_4;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_BAR_4;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_MALIBU_1:
		if(MusicState_GetCurrentTrack(&MusicManager) == STREAMED_SOUND_MALIBU_AMBIENT) return;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_MALIBU_1;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_MALIBU_1;
		MusicClubs_SetMalibuClubTrackPos(&MusicManager, SCRIPT_SOUND_NEW_BUILDING_MALIBU_1);
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_MALIBU_2:
		if(MusicState_GetCurrentTrack(&MusicManager) == STREAMED_SOUND_MALIBU_AMBIENT) return;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_MALIBU_2;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_MALIBU_2;
		MusicClubs_SetMalibuClubTrackPos(&MusicManager, SCRIPT_SOUND_NEW_BUILDING_MALIBU_2);
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_MALIBU_3:
		if(MusicState_GetCurrentTrack(&MusicManager) == STREAMED_SOUND_MALIBU_AMBIENT) return;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_MALIBU_3;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_MALIBU_3;
		MusicClubs_SetMalibuClubTrackPos(&MusicManager, SCRIPT_SOUND_NEW_BUILDING_MALIBU_3);
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_STRIP_1:
		if(MusicState_GetCurrentTrack(&MusicManager) == STREAMED_SOUND_STRIPCLUB_AMBIENT) return;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_STRIP_1;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_STRIP_1;
		MusicClubs_SetStripClubTrackPos(&MusicManager, SCRIPT_SOUND_NEW_BUILDING_STRIP_1);
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_STRIP_2:
		if(MusicState_GetCurrentTrack(&MusicManager) == STREAMED_SOUND_STRIPCLUB_AMBIENT) return;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_STRIP_2;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_STRIP_2;
		MusicClubs_SetStripClubTrackPos(&MusicManager, SCRIPT_SOUND_NEW_BUILDING_STRIP_2);
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_STRIP_3:
		if(MusicState_GetCurrentTrack(&MusicManager) == STREAMED_SOUND_STRIPCLUB_AMBIENT) return;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_STRIP_3;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_STRIP_3;
		MusicClubs_SetStripClubTrackPos(&MusicManager, SCRIPT_SOUND_NEW_BUILDING_STRIP_3);
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_BUILDING_CHURCH:
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_CHURCH;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_CHURCH;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		Vol = SCRIPT_OBJECT_DEFAULT_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 15;
		manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_SNORING_LOOP:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_SNORING_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDING_SNORE;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_SNORING;
		Vol = SCRIPT_OBJECT_SNORING_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_BUILDING_SNORE);
		manager->m_sQueueSample.m_nPriority = 6;
		manager->m_sQueueSample.m_nFramesToPlay = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_BANK_ALARM_LOOP:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BUILDINGS_BANK_ALARM;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_BUILDING_BANK_ALARM;
		Vol = SCRIPT_OBJECT_BANK_ALARM_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_BUILDINGS_BANK_ALARM);
		manager->m_sQueueSample.m_nPriority = 2;
		manager->m_sQueueSample.m_nFramesToPlay = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_POLICE_CELL_DOOR_SLIDING_LOOP:
	case SCRIPT_SOUND_GARAGE_DOOR_SLIDING_LOOP:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_GARAGE_DOOR_LOOP;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		Vol = SCRIPT_OBJECT_GARAGE_DOOR_SLIDING_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_GARAGE_DOOR_LOOP);
		manager->m_sQueueSample.m_nPriority = 3;
		manager->m_sQueueSample.m_nFramesToPlay = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_SHOOTING_RANGE_TARGET_MOVING_LOOP:
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_SHOOTING_RANGE_TARGET_MOVING_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_TANK_TURRET;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		Vol = SCRIPT_OBJECT_SHOOTING_RANGE_TARGET_MOVING_VOLUME;
		manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_TANK_TURRET);
		manager->m_sQueueSample.m_nPriority = 4;
		manager->m_sQueueSample.m_nFramesToPlay = 3;
		manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	case SCRIPT_SOUND_NEW_WATERFALL:
		Vol = SCRIPT_OBJECT_NEW_WATERFALL_VOLUME;
		manager->m_sQueueSample.m_MaxDistance = SCRIPT_OBJECT_LONG_MAX_DIST;
		manager->m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
		manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
		manager->m_sQueueSample.m_nFrequency = 20812;
		manager->m_sQueueSample.m_nPriority = 4;
		manager->m_sQueueSample.m_nFramesToPlay = 9;
		manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
		manager->m_sQueueSample.m_bIs2D = 0;
		break;
	default: return;
	}

	distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
	if(distSquared < (manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)) {
		manager->m_sQueueSample.m_fDistance = (distSquared <= 0.0f ? 0.0f : sqrtf(distSquared));
		manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
		if (manager->m_sQueueSample.m_nVolume > 0) {
			manager->m_sQueueSample.m_nCounter = 0;
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_nLoopCount = 0;
			manager->m_sQueueSample.m_bStatic = 0;
			AUDIO_SCRIPT_SOUNDS_SET_SOUND_REVERB(1);
			AUDIO_SCRIPT_SOUNDS_SET_EMITTING_VOLUME(Vol);
			AUDIO_SCRIPT_SOUNDS_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
			AUDIO_SCRIPT_SOUNDS_SET_SOUND_REFLECTION(0);
			AudioRequests_Submit(manager);
		}
	}
}

//- rouz edit (ChatGPT)
