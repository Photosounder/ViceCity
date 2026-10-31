//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "soundlist.h"
#include "AudioVehicleDoorTypes.h"
#include "../modelinfo/VehicleTypes.h"
#include "../modelinfo/VehicleAppearance.h"
#include "../modelinfo/ModelIds.h"
#include "../weapons/WeaponType.h"
#include "sampman.h"

enum {
	CAR_BOMB_TICK_VOLUME = 60,
	VEHICLE_ONE_SHOT_HELI_BLADE_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_HELI_BLADE_VOLUME = 70,
	VEHICLE_ONE_SHOT_CAR_TYRE_POP_MAX_DIST = 60,
	VEHICLE_ONE_SHOT_CAR_TYRE_POP_VOLUME = 117,
	VEHICLE_ONE_SHOT_DOOR_MAX_DIST = 50,
	VEHICLE_ONE_SHOT_DOOR_OPEN_VOLUME = 122,
	VEHICLE_ONE_SHOT_DOOR_CLOSE_VOLUME = 117,
	VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_VOLUME = 80,
	VEHICLE_ONE_SHOT_CAR_JUMP_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_CAR_JUMP_VOLUME = 80,
	VEHICLE_ONE_SHOT_CAR_ENGINE_START_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_CAR_ENGINE_START_VOLUME = 60,
	VEHICLE_ONE_SHOT_CAR_LIGHT_BREAK_VOLUME = 30,
	VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_CAR_HYDRAULIC_VOLUME = 55,
	VEHICLE_ONE_SHOT_CAR_SPLASH_MAX_DIST = 60,
	VEHICLE_ONE_SHOT_CAR_SPLASH_VOLUME = 35,
	VEHICLE_ONE_SHOT_BOAT_SLOWDOWN_MAX_DIST = 50,
	VEHICLE_ONE_SHOT_TRAIN_DOOR_MAX_DIST = 35,
	VEHICLE_ONE_SHOT_TRAIN_DOOR_VOLUME = 70,
	VEHICLE_ONE_SHOT_CAR_TANK_TURRET_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_CAR_TANK_TURRET_VOLUME = 90,
	VEHICLE_ONE_SHOT_CAR_BOMB_TICK_MAX_DIST = 30,
	VEHICLE_ONE_SHOT_CAR_BOMB_TICK_VOLUME = CAR_BOMB_TICK_VOLUME,
	VEHICLE_ONE_SHOT_PLANE_ON_GROUND_MAX_DIST = 180,
	VEHICLE_ONE_SHOT_PLANE_ON_GROUND_VOLUME = 75,
	VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST = 120,
	VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_VOLUME = 65,
	VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_VOLUME = 90,
	VEHICLE_ONE_SHOT_BOMB_ARMED_MAX_DIST = 50,
	VEHICLE_ONE_SHOT_BOMB_ARMED_VOLUME = 50,
	VEHICLE_ONE_SHOT_WATER_FALL_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_WATER_FALL_VOLUME = 90,
	VEHICLE_ONE_SHOT_SPLATTER_MAX_DIST = 40,
	VEHICLE_ONE_SHOT_SPLATTER_VOLUME = 55,
	VEHICLE_ONE_SHOT_CAR_PED_COLLISION_MAX_DIST = 40,
};
#define AUDIO_VEHICLE_ONE_SHOTS_MIN(a,b) ((a) < (b) ? (a) : (b))
#define AUDIO_VEHICLE_ONE_SHOTS_MAX(a,b) ((a) > (b) ? (a) : (b))
#define AUDIO_VEHICLE_ONE_SHOTS_COUNT(a) (sizeof(a) / sizeof((a)[0]))
#ifndef GTA_PS2
#define AUDIO_VEHICLE_ONE_SHOTS_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_VEHICLE_ONE_SHOTS_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_VEHICLE_ONE_SHOTS_RESET_LOOP_OFFSETS
#define AUDIO_VEHICLE_ONE_SHOTS_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_VEHICLE_ONE_SHOTS_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_VEHICLE_ONE_SHOTS_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REVERB(b)
#endif


void
AudioVehicleOneShots_Process(cAudioManager *manager, cVehicleParams *params)
{
	// Retain event-local setup and all original shared sample histories
	int16_t event;
	uint8_t Vol;
	float eventRelVol;
	float eventVol;
	uint8_t bLoop;
	uint8_t stereo;
	float maxDist;
	static uint8_t WaveIndex = 41;
	static uint8_t GunIndex = 53;
	// Initialize the audio context explicitly before processing it
	cPedParams pedParams;
	PedParams_Init(&pedParams);
	static uint32_t WaterFallFrame = 0;

	// Reload the live entity event count and sound record for each event
	for (uint16_t i = 0; i < manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++) {
		bLoop = 0;
		stereo = 0;
		AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(0);
		event = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		// Select each event using the original captured event value
		switch (event) {
		case SOUND_HELI_BLADE:
		{
			// Rotate the shared helicopter blade counter after the original rotor gates
			static uint8_t HeliIndex = 89;
			eventRelVol = AudioAircraftHost_WheelSpeed(params->m_pVehicle) * 50.0f / 11.0f;
			if (eventRelVol < 0.2f || eventRelVol == 1.0f)
				continue;
			Vol = (1.0f - eventRelVol) * VEHICLE_ONE_SHOT_HELI_BLADE_VOLUME;
			maxDist = (VEHICLE_ONE_SHOT_HELI_BLADE_MAX_DIST * VEHICLE_ONE_SHOT_HELI_BLADE_MAX_DIST);
			manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_HELI_ROT;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = HeliIndex++;
			if (HeliIndex > 90)
				HeliIndex = 89;
			manager->m_sQueueSample.m_nFrequency = (8000 * eventRelVol) + 16000;
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_HELI_BLADE_MAX_DIST;
			break;
		}
		case SOUND_CAR_WINDSHIELD_CRACK:
			maxDist = (VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST * VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST);
			manager->m_sQueueSample.m_nSampleIndex = SFX_GLASS_CRACK;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 68;
			Vol = manager->m_anRandomTable[1] % 30 + VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_VOLUME;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_GLASS_CRACK);
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST;
			break;
		case SOUND_CAR_DOOR_OPEN_BONNET:
		case SOUND_CAR_DOOR_OPEN_BUMPER:
		case SOUND_CAR_DOOR_OPEN_FRONT_LEFT:
		case SOUND_CAR_DOOR_OPEN_FRONT_RIGHT:
		case SOUND_CAR_DOOR_OPEN_BACK_LEFT:
		case SOUND_CAR_DOOR_OPEN_BACK_RIGHT:
			maxDist = (VEHICLE_ONE_SHOT_DOOR_MAX_DIST * VEHICLE_ONE_SHOT_DOOR_MAX_DIST);
			Vol = manager->m_anRandomTable[1] % (MAX_VOLUME - VEHICLE_ONE_SHOT_DOOR_CLOSE_VOLUME) + VEHICLE_ONE_SHOT_DOOR_CLOSE_VOLUME;
			switch (AudioVehicleOneShotsHost_DoorType(params->m_nIndex)) {
			case OLD_DOOR:
				manager->m_sQueueSample.m_nSampleIndex = SFX_OLD_CAR_DOOR_OPEN;
				break;
			case NEW_DOOR:
			default:
				manager->m_sQueueSample.m_nSampleIndex = SFX_NEW_CAR_DOOR_OPEN;
				break;
			case TRUCK_DOOR:
				manager->m_sQueueSample.m_nSampleIndex = SFX_TRUCK_DOOR_OPEN;
				break;
			case BUS_DOOR:
				manager->m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
				break;
			}
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#ifdef THIS_IS_STUPID
			manager->m_sQueueSample.m_nCounter = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] + 10;
#else
			manager->m_sQueueSample.m_nCounter = event + 10;
#endif
			if (AudioVehicleHost_Appearance(params->m_pVehicle) == VEHICLE_APPEARANCE_HELI)
				manager->m_sQueueSample.m_nFrequency = 23459;
			else
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_DOOR_MAX_DIST;
			AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_CAR_DOOR_CLOSE_BONNET:
		case SOUND_CAR_DOOR_CLOSE_BUMPER:
		case SOUND_CAR_DOOR_CLOSE_FRONT_LEFT:
		case SOUND_CAR_DOOR_CLOSE_FRONT_RIGHT:
		case SOUND_CAR_DOOR_CLOSE_BACK_LEFT:
		case SOUND_CAR_DOOR_CLOSE_BACK_RIGHT:
			maxDist = (VEHICLE_ONE_SHOT_DOOR_MAX_DIST * VEHICLE_ONE_SHOT_DOOR_MAX_DIST);
			Vol = manager->m_anRandomTable[2] % (MAX_VOLUME - VEHICLE_ONE_SHOT_DOOR_OPEN_VOLUME) + VEHICLE_ONE_SHOT_DOOR_OPEN_VOLUME;
			switch (AudioVehicleOneShotsHost_DoorType(params->m_nIndex)) {
			case OLD_DOOR:
				manager->m_sQueueSample.m_nSampleIndex = SFX_OLD_CAR_DOOR_CLOSE;
				break;
			case NEW_DOOR:
			default:
				manager->m_sQueueSample.m_nSampleIndex = SFX_NEW_CAR_DOOR_CLOSE;
				break;
			case TRUCK_DOOR:
				manager->m_sQueueSample.m_nSampleIndex = SFX_TRUCK_DOOR_CLOSE;
				break;
			case BUS_DOOR:
				manager->m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
				break;
			}
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#ifdef THIS_IS_STUPID
			manager->m_sQueueSample.m_nCounter = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] + 22;
#else
			manager->m_sQueueSample.m_nCounter = event + 22;
#endif
			if (AudioVehicleHost_Appearance(params->m_pVehicle) == VEHICLE_APPEARANCE_HELI)
				manager->m_sQueueSample.m_nFrequency = 28062;
			else
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency / 32);
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_DOOR_MAX_DIST;
			AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_CAR_ENGINE_START:
			if (AudioVehicleHost_Appearance(params->m_pVehicle) != VEHICLE_APPEARANCE_CAR
				|| AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_CADDY)
				continue;
			Vol = VEHICLE_ONE_SHOT_CAR_ENGINE_START_VOLUME;
			maxDist = (VEHICLE_ONE_SHOT_CAR_ENGINE_START_MAX_DIST * VEHICLE_ONE_SHOT_CAR_ENGINE_START_MAX_DIST);
			manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_STARTER;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 33;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_CAR_STARTER);
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_ENGINE_START_MAX_DIST;
			AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_WEAPON_HIT_VEHICLE:
			manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[manager->m_sQueueSample.m_nEntityIndex % AUDIO_VEHICLE_ONE_SHOTS_COUNT(manager->m_anRandomTable)] % 6 + SFX_BULLET_CAR_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 34;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
			manager->m_sQueueSample.m_nPriority = 7;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_MAX_DIST * VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_MAX_DIST);
			Vol = manager->m_anRandomTable[3] % 20 + VEHICLE_ONE_SHOT_WEAPON_HIT_VEHICLE_VOLUME;
			break;
		case SOUND_BOMB_TIMED_ACTIVATED:
		case SOUND_91:
		case SOUND_BOMB_ONIGNITION_ACTIVATED:
		case SOUND_BOMB_TICK:
			manager->m_sQueueSample.m_nSampleIndex = SFX_ARM_BOMB;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 36;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_ARM_BOMB);
			manager->m_sQueueSample.m_nPriority = 0;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_BOMB_ARMED_MAX_DIST;
			AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			Vol = VEHICLE_ONE_SHOT_BOMB_ARMED_VOLUME;
			maxDist = (VEHICLE_ONE_SHOT_BOMB_ARMED_MAX_DIST * VEHICLE_ONE_SHOT_BOMB_ARMED_MAX_DIST);
			break;
		case SOUND_CAR_LIGHT_BREAK:
			manager->m_sQueueSample.m_nSampleIndex = SFX_GLASS_SHARD_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 37;
			manager->m_sQueueSample.m_nFrequency = 9 * SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_GLASS_SHARD_1) / 10;
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 3);
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST * VEHICLE_ONE_SHOT_WINDSHIELD_CRACK_MAX_DIST);
			Vol = manager->m_anRandomTable[4] % 10 + VEHICLE_ONE_SHOT_CAR_LIGHT_BREAK_VOLUME;
			break;
		case SOUND_PLANE_ON_GROUND:
			manager->m_sQueueSample.m_nSampleIndex = SFX_JUMBO_LAND_WHEELS;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 81;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_JUMBO_LAND_WHEELS);
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_PLANE_ON_GROUND_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_PLANE_ON_GROUND_MAX_DIST * VEHICLE_ONE_SHOT_PLANE_ON_GROUND_MAX_DIST);
			Vol = manager->m_anRandomTable[4] % 25 + VEHICLE_ONE_SHOT_PLANE_ON_GROUND_VOLUME;
			break;
		case SOUND_CAR_JERK:
			manager->m_sQueueSample.m_nSampleIndex = SFX_SHAG_SUSPENSION;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 87;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_SHAG_SUSPENSION);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 3);
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST * VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST);
			Vol = manager->m_anRandomTable[1] % 15 + VEHICLE_ONE_SHOT_CAR_HYDRAULIC_VOLUME;
			break;
		case SOUND_CAR_HYDRAULIC_1:
		case SOUND_CAR_HYDRAULIC_2:
			if (event == SOUND_CAR_HYDRAULIC_1)
				manager->m_sQueueSample.m_nFrequency = 15600;
			else
				manager->m_sQueueSample.m_nFrequency = 13118;
			manager->m_sQueueSample.m_nSampleIndex = SFX_SUSPENSION_FAST_MOVE;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 51;
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 3);
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST * VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST);
			Vol = manager->m_anRandomTable[0] % 15 + VEHICLE_ONE_SHOT_CAR_HYDRAULIC_VOLUME;
			break;
		case SOUND_CAR_HYDRAULIC_3:
			manager->m_sQueueSample.m_nSampleIndex = SFX_SUSPENSION_SLOW_MOVE_LOOP;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 86;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_SUSPENSION_SLOW_MOVE_LOOP);
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST;
			manager->m_sQueueSample.m_nFramesToPlay = 7;
			bLoop = 1;
			maxDist = (VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST * VEHICLE_ONE_SHOT_CAR_HYDRAULIC_MAX_DIST);
			Vol = manager->m_anRandomTable[0] % 15 + VEHICLE_ONE_SHOT_CAR_HYDRAULIC_VOLUME;
			break;
		case SOUND_WATER_FALL:
			// Apply the shared waterfall deadline before updating sound fields
			if (manager->m_FrameCounter <= WaterFallFrame)
				continue;
			WaterFallFrame = manager->m_FrameCounter + 6;
			manager->m_sQueueSample.m_nSampleIndex = SFX_SPLASH_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 15;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 16000;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_WATER_FALL_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_WATER_FALL_MAX_DIST * VEHICLE_ONE_SHOT_WATER_FALL_MAX_DIST);
			AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			Vol = manager->m_anRandomTable[4] % 20 + VEHICLE_ONE_SHOT_WATER_FALL_VOLUME;
			break;
		case SOUND_CAR_BOMB_TICK:
			manager->m_sQueueSample.m_nSampleIndex = SFX_BOMB_BEEP;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 80;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_BOMB_BEEP);
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_BOMB_TICK_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_CAR_BOMB_TICK_MAX_DIST * VEHICLE_ONE_SHOT_CAR_BOMB_TICK_MAX_DIST);
			AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			Vol = VEHICLE_ONE_SHOT_CAR_BOMB_TICK_VOLUME;
			break;
		case SOUND_CAR_SPLASH:
			// Clamp the live splash event volume only after the original minimum gate
			eventVol = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			if (eventVol <= 150)
				continue;
			if (eventVol > 800)
				manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i] = 800;
			eventRelVol = (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i] - 150) / (800 - 150);
			manager->m_sQueueSample.m_nSampleIndex = (manager->m_anRandomTable[0] & 1) + SFX_BOAT_SPLASH_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = WaveIndex++;
			if (WaveIndex > 46)
				WaveIndex = 41;
			manager->m_sQueueSample.m_nFrequency = (7000 * eventRelVol) + 6000;
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_SPLASH_MAX_DIST;
			Vol = (VEHICLE_ONE_SHOT_CAR_SPLASH_VOLUME * eventRelVol);
			maxDist = (VEHICLE_ONE_SHOT_CAR_SPLASH_MAX_DIST * VEHICLE_ONE_SHOT_CAR_SPLASH_MAX_DIST);
			break;
		case SOUND_CAR_JUMP: 
		case SOUND_CAR_JUMP_2:
		{
			// Rotate the shared jump counter and retain the alternate event-table reload
			static uint8_t iWheelIndex = 82;
			maxDist = (VEHICLE_ONE_SHOT_CAR_JUMP_MAX_DIST * VEHICLE_ONE_SHOT_CAR_JUMP_MAX_DIST);
#ifdef THIS_IS_STUPID
			if (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] == SOUND_CAR_JUMP_2) {
#else
			if (event == SOUND_CAR_JUMP_2) {
#endif
				manager->m_sQueueSample.m_nSampleIndex = SFX_TYRE_BURST_B;
				Vol = AUDIO_VEHICLE_ONE_SHOTS_MAX(50, 2 * (60 * manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i]));
			} else {
				manager->m_sQueueSample.m_nSampleIndex = SFX_TYRE_BUMP;
				Vol = AUDIO_VEHICLE_ONE_SHOTS_MAX(VEHICLE_ONE_SHOT_CAR_JUMP_VOLUME, 2 * (100 * manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i]));
			}
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iWheelIndex++;
			if (iWheelIndex > 85)
				iWheelIndex = 82;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_TYRE_BUMP);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
			if (params->m_VehicleType == VEHICLE_TYPE_BIKE)
				manager->m_sQueueSample.m_nFrequency <<= 1;
			manager->m_sQueueSample.m_nPriority = 6;
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_JUMP_MAX_DIST;
			break;
		}
		case SOUND_CAR_TYRE_POP:
		{
			// Rotate the shared tyre counter before backend sample queries
			static uint8_t WheelIndex = 91;
			manager->m_sQueueSample.m_nSampleIndex = SFX_TYRE_BURST;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = WheelIndex++;
			if (WheelIndex > 94)
				WheelIndex = 91;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_TYRE_BURST);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, 2000);
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_TYRE_POP_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_CAR_TYRE_POP_MAX_DIST * VEHICLE_ONE_SHOT_CAR_TYRE_POP_MAX_DIST);
			Vol = manager->m_anRandomTable[4] % (MAX_VOLUME-VEHICLE_ONE_SHOT_CAR_TYRE_POP_VOLUME) + VEHICLE_ONE_SHOT_CAR_TYRE_POP_VOLUME;
			break;
		}
		case SOUND_WEAPON_SHOT_FIRED:
			switch (AudioVehicleReverseHost_Model(params->m_pVehicle)) {
			case MI_HUNTER:
			case MI_CHOPPER:
			case MI_SEASPAR:
			case MI_SPARROW:
			case MI_MAVERICK:
			case MI_VCNMAV:
				if (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_HUNTER) {
					if (AudioVehicleOneShotsHost_HandBrake() == 0) {
						if (AudioGame_FindVehicle() != params->m_pVehicle) {
							manager->m_sQueueSample.m_nSampleIndex = SFX_M60_LEFT;
							manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						} else {
							manager->m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
							manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
						}
					} else {
						manager->m_sQueueSample.m_nSampleIndex = SFX_M60_LEFT;
						manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					}
				} else {
					manager->m_sQueueSample.m_nSampleIndex = SFX_M60_LEFT;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				}
				maxDist = (VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST * VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST);
				manager->m_sQueueSample.m_nCounter = GunIndex++;
				Vol = MAX_VOLUME;
				if (GunIndex > 58)
					GunIndex = 53;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_M60_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
				manager->m_sQueueSample.m_nPriority = 2;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST;
				AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				stereo = 1;
				break;
			default:
			{
				maxDist = (VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST * VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST);
#ifdef FIX_BUGS
				uint32_t sampleIndex;
				uint32_t frequency;
				// Capture the driver once while keeping subsequent vehicle reads live
				CPed *pPed = AudioVehicleOneShotsHost_Driver(params->m_pVehicle);
				if(!pPed)
					continue;
				if(!AudioVehicleOneShotsHost_HasWeapon(pPed, WEAPONSLOT_SUBMACHINEGUN) || (AudioVehicleReverseHost_Model(params->m_pVehicle) == MI_PREDATOR && !AudioVehicleOneShotsHost_DriveBy(pPed))) {
					sampleIndex = SFX_UZI_LEFT;
					frequency = SampleManager_GetSampleBaseFrequency(&SampleManager, sampleIndex);
					frequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, frequency >> 5);
				} else
					switch(AudioVehicleOneShotsHost_Weapon(pPed, WEAPONSLOT_SUBMACHINEGUN)) {
					case WEAPONTYPE_TEC9:
						sampleIndex = SFX_TEC_LEFT;
						frequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 500) + 17000;
						break;
					case WEAPONTYPE_SILENCED_INGRAM:
						sampleIndex = SFX_TEC_LEFT;
						frequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 34000;
						break;
					case WEAPONTYPE_MP5:
						sampleIndex = SFX_MP5_LEFT;
						frequency = SampleManager_GetSampleBaseFrequency(&SampleManager, sampleIndex);
						frequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, frequency >> 5);
						break;
					default:
						sampleIndex = SFX_UZI_LEFT;
						frequency = SampleManager_GetSampleBaseFrequency(&SampleManager, sampleIndex);
						frequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, frequency >> 5);
						break;
					}
				manager->m_sQueueSample.m_nSampleIndex = sampleIndex;
#else
				manager->m_sQueueSample.m_nSampleIndex = SFX_UZI_LEFT;
#endif
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = GunIndex++;
				Vol = manager->m_anRandomTable[2] % 15 + VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_VOLUME;
				if(GunIndex > 58) GunIndex = 53;
#ifdef FIX_BUGS
				manager->m_sQueueSample.m_nFrequency = frequency;
#else
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_UZI_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
#endif
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_WEAPON_SHOT_FIRED_MAX_DIST;
				AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				break;
			}
			}
			break;
#ifdef GTA_TRAIN
		case SOUND_TRAIN_DOOR_CLOSE:
		case SOUND_TRAIN_DOOR_OPEN:
			manager->m_sQueueSample.m_nSampleIndex = SFX_AIR_BRAKES;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 59;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 11025;
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 5.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_TRAIN_DOOR_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_TRAIN_DOOR_MAX_DIST * VEHICLE_ONE_SHOT_TRAIN_DOOR_MAX_DIST);
			Vol = manager->m_anRandomTable[1] % 20 + VEHICLE_ONE_SHOT_TRAIN_DOOR_VOLUME;
			break;
#endif
		case SOUND_SPLATTER:
		{
			// Alternate the shared pedestrian crunch sample offset
			static uint8_t CrunchOffset = 0;
			manager->m_sQueueSample.m_nSampleIndex = CrunchOffset + SFX_PED_CRUNCH_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 48;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 6000) + 16000;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_SPLATTER_MAX_DIST;
			CrunchOffset++;
			maxDist = (VEHICLE_ONE_SHOT_SPLATTER_MAX_DIST * VEHICLE_ONE_SHOT_SPLATTER_MAX_DIST);
			Vol = manager->m_anRandomTable[4] % 20 + VEHICLE_ONE_SHOT_SPLATTER_VOLUME;
			CrunchOffset %= 2;
			AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		}
		case SOUND_CAR_PED_COLLISION:
			eventVol = AUDIO_VEHICLE_ONE_SHOTS_MIN(20.0f, manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i]);
			Vol = AUDIO_VEHICLE_ONE_SHOTS_MIN(MAX_VOLUME, (3 * (eventVol / 20 * MAX_VOLUME)) / 2);
			if (Vol == 0)
				continue;

			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 50;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_PED_COLLISION_MAX_DIST;
			maxDist = (VEHICLE_ONE_SHOT_CAR_PED_COLLISION_MAX_DIST * VEHICLE_ONE_SHOT_CAR_PED_COLLISION_MAX_DIST);
			break;
		case SOUND_PED_HELI_PLAYER_FOUND:
			pedParams.m_bDistanceCalculated = params->m_bDistanceCalculated;
			pedParams.m_fDistance = params->m_fDistance;
			AudioPedSpeech_Setup(manager, &pedParams, SOUND_PED_HELI_PLAYER_FOUND);
			continue;
		case SOUND_PED_VCPA_PLAYER_FOUND:
			pedParams.m_bDistanceCalculated = params->m_bDistanceCalculated;
			pedParams.m_fDistance = params->m_fDistance;
			AudioPedSpeech_Setup(manager, &pedParams, SOUND_PED_VCPA_PLAYER_FOUND);
			continue;
		case SOUND_CAR_TANK_TURRET_ROTATE:
			eventVol = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			if (eventVol > 96.0f / 2500.0f)
				eventVol = 96.0f / 2500.0f;
			manager->m_sQueueSample.m_nSampleIndex = SFX_TANK_TURRET;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 79;
			manager->m_sQueueSample.m_nFrequency = (3000 * eventVol / (96.0f / 2500.0f)) + 9000;
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
			manager->m_sQueueSample.m_nFramesToPlay = 3;
			manager->m_sQueueSample.m_MaxDistance = VEHICLE_ONE_SHOT_CAR_TANK_TURRET_MAX_DIST;
			Vol = (37 * eventVol / (96.0f / 2500.0f)) + VEHICLE_ONE_SHOT_CAR_TANK_TURRET_VOLUME;
			maxDist = (VEHICLE_ONE_SHOT_CAR_TANK_TURRET_MAX_DIST * VEHICLE_ONE_SHOT_CAR_TANK_TURRET_MAX_DIST);
			bLoop = 1;
			break;
		default:
			continue;
		}
		// Apply the original distance gate and assemble one-shot or looping request fields
		if (params->m_fDistance < maxDist) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
			// Complete request fields only when the original attenuation produces nonzero volume
			if (manager->m_sQueueSample.m_nVolume > 0) {
				// Preserve the distinction between looping and static one-shot requests
				if (bLoop) {
					manager->m_sQueueSample.m_nLoopCount = 0;
					manager->m_sQueueSample.m_bStatic = 0;
				} else {
					manager->m_sQueueSample.m_nLoopCount = 1;
					manager->m_sQueueSample.m_bStatic = 1;
				}
				// Query both loop offsets using their original live sample arguments
				AUDIO_VEHICLE_ONE_SHOTS_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				AUDIO_VEHICLE_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REVERB(1);
				if (stereo) {
					if(manager->m_sQueueSample.m_fDistance < 0.2f * manager->m_sQueueSample.m_MaxDistance) {
						manager->m_sQueueSample.m_bIs2D = 1;
						manager->m_sQueueSample.m_nPan = 0;
					} else {
						stereo = 0;
						manager->m_sQueueSample.m_bIs2D = 0;
					}
				} else manager->m_sQueueSample.m_bIs2D = 0;
				// Submit the first channel before changing the second channel sample and shared gun counter
				AudioRequests_Submit(manager);
				if (stereo) {
					manager->m_sQueueSample.m_nPan = 127;
					manager->m_sQueueSample.m_nSampleIndex++;
					manager->m_sQueueSample.m_nCounter = GunIndex++;
					if (GunIndex > 58)
						GunIndex = 53;
					AUDIO_VEHICLE_ONE_SHOTS_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
				continue;

			}
		}
	}
}
//- rouz edit (ChatGPT)
