//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "sampman.h"
#include "soundlist.h"
#include "../weapons/WeaponType.h"
#include "../peds/PedStates.h"
#include "../entities/EntityTypes.h"
#include "../core/SurfaceTypes.h"
#include "../modelinfo/ModelIds.h"

enum {
	PED_ONE_SHOT_SHIRT_FLAP_MAX_DIST = 15,
	PED_ONE_SHOT_SHIRT_FLAP_VOLUME = 90,
	PED_ONE_SHOT_MINIGUN_MAX_DIST = 150,
	PED_ONE_SHOT_MINIGUN_VOLUME = MAX_VOLUME,
	PED_ONE_SHOT_SKATING_MAX_DIST = 20,
	PED_ONE_SHOT_SKATING_VOLUME = 70,
	PED_ONE_SHOT_STEP_MAX_DIST = 20,
	PED_ONE_SHOT_STEP_VOLUME = 45,
	PED_ONE_SHOT_FALL_MAX_DIST = 30,
	PED_ONE_SHOT_FALL_VOLUME = 80,
	PED_ONE_SHOT_PUNCH_MAX_DIST = 30,
	PED_ONE_SHOT_PUNCH_VOLUME = 100,
	PED_ONE_SHOT_WEAPON_COLT45_VOLUME = 90,
	PED_ONE_SHOT_WEAPON_UZI_VOLUME = 70,
	PED_ONE_SHOT_WEAPON_SHOTGUN_VOLUME = 100,
	PED_ONE_SHOT_WEAPON_M4_VOLUME = 90,
	PED_ONE_SHOT_WEAPON_M16_VOLUME = MAX_VOLUME,
	PED_ONE_SHOT_WEAPON_SNIPERRIFLE_VOLUME = 110,
	PED_ONE_SHOT_WEAPON_ROCKETLAUNCHER_VOLUME = 80,
	PED_ONE_SHOT_WEAPON_FLAMETHROWER_MAX_DIST = 60,
	PED_ONE_SHOT_WEAPON_FLAMETHROWER_VOLUME = 90,
	PED_ONE_SHOT_WEAPON_RELOAD_MAX_DIST = 30,
	PED_ONE_SHOT_WEAPON_RELOAD_VOLUME = 75,
	PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST = 120,
	PED_ONE_SHOT_WEAPON_BULLET_ECHO_VOLUME = 80,
	PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_MAX_DIST = 60,
	PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_VOLUME = 70,
	PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST = 60,
	PED_ONE_SHOT_WEAPON_CHAINSAW_IDLE_MAX_DIST = 50,
	PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME = 100,
	PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST = 30,
	PED_ONE_SHOT_WEAPON_HIT_PED_VOLUME = 90,
	PED_ONE_SHOT_SPLASH_MAX_DIST = 40,
	PED_ONE_SHOT_SPLASH_PED_VOLUME = 70,
};
#ifndef GTA_PS2
#define AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
#define AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_PED_ONE_SHOTS_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_PED_ONE_SHOTS_SET_SOUND_REVERB(b)
#endif


void
AudioPedOneShots_Process(cAudioManager *manager, cPedParams *params)
{
    // Retain captured objects and event-local storage throughout the original loop
	uint8_t Vol;
	uint32_t sampleIndex;

	CPed *ped = params->m_pPed;

	uint8_t narrowSoundRange;
	int16_t sound;
	uint8_t stereo;
	CWeapon *weapon;
#ifdef FIX_BUGS
	float maxDist = 0.0f; // uninitialized variable
#else
	float maxDist;
#endif

	// Preserve the shared sound-counter and splash-frame histories
	static uint8_t iSound = 21;
	static uint32_t iSplashFrame = 0;

	weapon = AudioPedOneShotsHost_Weapon(params->m_pPed);
	// Process each live event in its original order
	for (uint32_t i = 0; i < manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_AudioEvents; i++) {
		stereo = 0;
		narrowSoundRange = 0;
		AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(0);
		sound = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i];
		switch (sound) {
		case SOUND_SHIRT_WIND_FLAP:
			if (AudioPedOneShotsHost_IsPlayer(params->m_pPed) && AudioPedOneShotsHost_Vehicle(params->m_pPed)) {
				if (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i] > 0.0f) {
					if (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i] > 1.0f)
						manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i] = 1.0f;

					Vol = PED_ONE_SHOT_SHIRT_FLAP_VOLUME * manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];

					switch (AudioVehicleReverseHost_Model(AudioPedOneShotsHost_Vehicle(params->m_pPed)))
					{
					case MI_ANGEL:
					case MI_FREEWAY:
						manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_17;
						break;
					case MI_PCJ600:
						manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_20;
						break;
					case MI_SANCHEZ:
						manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_19;
						break;
					case MI_PIZZABOY:
					case MI_FAGGIO:
						manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_WIND_18;
						break;
					default:
						continue;
					};

					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nCounter = 71;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
					manager->m_sQueueSample.m_nPriority = 3;
					manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
					manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_SHIRT_FLAP_MAX_DIST;
					maxDist = (PED_ONE_SHOT_SHIRT_FLAP_MAX_DIST * PED_ONE_SHOT_SHIRT_FLAP_MAX_DIST);
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 3;
				}
			}
			continue;
		case SOUND_WEAPON_MINIGUN_ATTACK:
			manager->m_sQueueSample.m_nSampleIndex = SFX_MINIGUN_FIRE_LEFT;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 68;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_MINIGUN_FIRE_LEFT);
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_MINIGUN_MAX_DIST;
			Vol = PED_ONE_SHOT_MINIGUN_VOLUME;
			maxDist = (PED_ONE_SHOT_MINIGUN_MAX_DIST * PED_ONE_SHOT_MINIGUN_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(SFX_MINIGUN_FIRE_LEFT)
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_MINIGUN_VOLUME);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 3;
			break;
		case SOUND_WEAPON_MINIGUN_2:
			manager->m_sQueueSample.m_nSampleIndex = SFX_MINIGUN_FIRE_RIGHT;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 69;
			manager->m_sQueueSample.m_nFrequency = 18569;
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_MINIGUN_MAX_DIST;
			Vol = PED_ONE_SHOT_MINIGUN_VOLUME * manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			maxDist = (PED_ONE_SHOT_MINIGUN_MAX_DIST * PED_ONE_SHOT_MINIGUN_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 0;
			AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(SFX_MINIGUN_FIRE_RIGHT)
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 3;
			break;
		case SOUND_WEAPON_MINIGUN_3:
			manager->m_sQueueSample.m_nSampleIndex = SFX_MINIGUN_STOP;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 69;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_MINIGUN_STOP);
			manager->m_sQueueSample.m_nPriority = 4;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_MINIGUN_MAX_DIST;
			maxDist = (PED_ONE_SHOT_MINIGUN_MAX_DIST * PED_ONE_SHOT_MINIGUN_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = PED_ONE_SHOT_MINIGUN_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_MINIGUN_VOLUME);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_SKATING:
		{
			uint32_t soundParams = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			uint8_t param1 = soundParams & 0xFF;
			uint32_t param2 = soundParams >> 8;
			manager->m_sQueueSample.m_nSampleIndex = (manager->m_anRandomTable[3] & 1) + SFX_SKATE_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iSound;
			stereo = 1;
			iSound++;
			manager->m_sQueueSample.m_nFrequency = manager->m_anRandomTable[1] % 1000 + 17000;
			if (param2 == 0)
				manager->m_sQueueSample.m_nFrequency = (3 * manager->m_sQueueSample.m_nFrequency) >> 2;
			manager->m_sQueueSample.m_nPriority = 6;
			manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_SKATING_MAX_DIST;
			maxDist = (PED_ONE_SHOT_SKATING_MAX_DIST * PED_ONE_SHOT_SKATING_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = (manager->m_anRandomTable[2] % 20 + PED_ONE_SHOT_SKATING_VOLUME) * param1 / MAX_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		}
		case SOUND_FALL_LAND:
		case SOUND_FALL_COLLAPSE:
			if (AudioPedOneShotsHost_InAir(ped))
				continue;
			maxDist = (PED_ONE_SHOT_FALL_MAX_DIST * PED_ONE_SHOT_FALL_MAX_DIST);
			Vol = manager->m_anRandomTable[3] % 20 + PED_ONE_SHOT_FALL_VOLUME;
			if (AudioPedOneShotsHost_Surface(ped) == SURFACE_WATER)
				manager->m_sQueueSample.m_nSampleIndex = (manager->m_anRandomTable[3] % 4) + SFX_FOOTSTEP_WATER_1;
			else if (sound == SOUND_FALL_LAND)
				manager->m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND;
			else
				manager->m_sQueueSample.m_nSampleIndex = SFX_BODY_LAND_AND_FALL;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency / 17);
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_FALL_MAX_DIST;
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_STEP_START:
		case SOUND_STEP_END:
			if (AudioPedOneShotsHost_InAir(params->m_pPed))
				continue;
			Vol = manager->m_anRandomTable[3] % 15 + PED_ONE_SHOT_STEP_VOLUME;
			if (AudioPedOneShotsHost_Player() != manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_pEntity)
				Vol >>= 1;
			maxDist = (PED_ONE_SHOT_STEP_MAX_DIST * PED_ONE_SHOT_STEP_MAX_DIST);
			switch (AudioPedOneShotsHost_Surface(params->m_pPed)) {
			case SURFACE_GRASS:
				sampleIndex = manager->m_anRandomTable[1] % 5 + SFX_FOOTSTEP_GRASS_1;
				break;
			case SURFACE_GRAVEL:
			case SURFACE_MUD_DRY:
				sampleIndex = manager->m_anRandomTable[4] % 5 + SFX_FOOTSTEP_GRAVEL_1;
				break;
			case SURFACE_CAR:
			case SURFACE_GARAGE_DOOR:
			case SURFACE_CAR_PANEL:
			case SURFACE_THICK_METAL_PLATE:
			case SURFACE_SCAFFOLD_POLE:
			case SURFACE_LAMP_POST:
			case SURFACE_FIRE_HYDRANT:
			case SURFACE_GIRDER:
			case SURFACE_METAL_CHAIN_FENCE:
			case SURFACE_CONTAINER:
			case SURFACE_NEWS_VENDOR:
				sampleIndex = manager->m_anRandomTable[0] % 5 + SFX_FOOTSTEP_METAL_1;
				break;
			case SURFACE_SAND:
				sampleIndex = (manager->m_anRandomTable[4] & 3) + SFX_FOOTSTEP_SAND_1;
				break;
			case SURFACE_WATER:
				sampleIndex = (manager->m_anRandomTable[3] & 3) + SFX_FOOTSTEP_WATER_1;
				break;
			case SURFACE_WOOD_CRATES:
			case SURFACE_WOOD_BENCH:
			case SURFACE_WOOD_SOLID:
				sampleIndex = manager->m_anRandomTable[2] % 5 + SFX_FOOTSTEP_WOOD_1;
				break;
			case SURFACE_HEDGE:
				sampleIndex = manager->m_anRandomTable[2] % 5 + SFX_COL_VEG_1;
				break;
			default:
				sampleIndex = manager->m_anRandomTable[2] % 5 + SFX_FOOTSTEP_CONCRETE_1;
				break;
			}
			manager->m_sQueueSample.m_nSampleIndex = sampleIndex;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] - SOUND_STEP_START + 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency / 17);
			switch (AudioPedOneShotsHost_MoveState(params->m_pPed)) {
			case PEDMOVE_WALK:
				Vol >>= 2;
				manager->m_sQueueSample.m_nFrequency = 9 * manager->m_sQueueSample.m_nFrequency / 10;
				break;
			case PEDMOVE_RUN:
				Vol >>= 1;
				manager->m_sQueueSample.m_nFrequency = 11 * manager->m_sQueueSample.m_nFrequency / 10;
				break;
			case PEDMOVE_SPRINT:
				manager->m_sQueueSample.m_nFrequency = 12 * manager->m_sQueueSample.m_nFrequency / 10;
				break;
			default:
				break;
			}
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_STEP_MAX_DIST;
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_WEAPON_AK47_BULLET_ECHO:
		{
			uint32_t weaponType = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i];
			switch (weaponType) {
			case WEAPONTYPE_SPAS12_SHOTGUN:
				manager->m_sQueueSample.m_nSampleIndex = SFX_SPAS12_TAIL_LEFT;
				break;
			case WEAPONTYPE_M60:
			case WEAPONTYPE_HELICANNON:
				manager->m_sQueueSample.m_nSampleIndex = SFX_M60_TAIL_LEFT;
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_MP5:
				manager->m_sQueueSample.m_nSampleIndex = SFX_UZI_END_LEFT;
				break;
			case WEAPONTYPE_TEC9:
			case WEAPONTYPE_SILENCED_INGRAM:
				manager->m_sQueueSample.m_nSampleIndex = SFX_TEC_TAIL;
				break;
			case WEAPONTYPE_M4:
			case WEAPONTYPE_RUGER:
			case WEAPONTYPE_SNIPERRIFLE:
			case WEAPONTYPE_LASERSCOPE:
				manager->m_sQueueSample.m_nSampleIndex = SFX_RUGER_TAIL;
				break;
				break;
			default:
				continue;
			}
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = 1;
			switch (weaponType) {
			case WEAPONTYPE_SILENCED_INGRAM:
				manager->m_sQueueSample.m_nFrequency = 26000;
				break;
			case WEAPONTYPE_TEC9:
				manager->m_sQueueSample.m_nFrequency = 13000;
				break;
			case WEAPONTYPE_M4:
				manager->m_sQueueSample.m_nFrequency = 15600;
				break;
			case WEAPONTYPE_LASERSCOPE:
				manager->m_sQueueSample.m_nFrequency = 7904;
				break;
			case WEAPONTYPE_SNIPERRIFLE:
				manager->m_sQueueSample.m_nFrequency = 9959;
				break;
			default:
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
				break;
			}
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
			maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = manager->m_anRandomTable[4] % 10 + PED_ONE_SHOT_WEAPON_BULLET_ECHO_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		}
		case SOUND_WEAPON_FLAMETHROWER_FIRE:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_START_LEFT;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iSound++;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_FLAMETHROWER_START_LEFT);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_MAX_DIST;
			maxDist = (PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_MAX_DIST * PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_FLAMETHROWER_FIRE_VOLUME);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			break;
		case SOUND_WEAPON_SHOT_FIRED:
			weapon = AudioPedOneShotsHost_Weapon(ped);
			if (!weapon)
				continue;
			switch (AudioPedOneShotsHost_WeaponType(weapon)) {
			case WEAPONTYPE_PYTHON:
				manager->m_sQueueSample.m_nSampleIndex = SFX_PYTHON_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_PYTHON_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = MAX_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				stereo = 1;
				break;
			case WEAPONTYPE_COLT45:
				manager->m_sQueueSample.m_nSampleIndex = SFX_COLT45_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_COLT45_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[1] % 10 + PED_ONE_SHOT_WEAPON_COLT45_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				stereo = 1;
				break;
			case WEAPONTYPE_ROCKET:
			case WEAPONTYPE_ROCKETLAUNCHER:
				manager->m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_ROCKET_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 1;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[0] % 20 + PED_ONE_SHOT_WEAPON_ROCKETLAUNCHER_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				stereo = 1;
				break;
			case WEAPONTYPE_FLAMETHROWER:
				manager->m_sQueueSample.m_nSampleIndex = SFX_FLAMETHROWER_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = 9;
				Vol = PED_ONE_SHOT_WEAPON_FLAMETHROWER_VOLUME;
				manager->m_sQueueSample.m_nFrequency = (10 * manager->m_sQueueSample.m_nEntityIndex % 2048) + SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_FLAMETHROWER_LEFT);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_FLAMETHROWER_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_FLAMETHROWER_MAX_DIST * PED_ONE_SHOT_WEAPON_FLAMETHROWER_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_FLAMETHROWER_VOLUME);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 6;
				stereo = 1;
				break;
			case WEAPONTYPE_M60:
			case WEAPONTYPE_HELICANNON:
				manager->m_sQueueSample.m_nSampleIndex = SFX_M60_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_M60_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = PED_ONE_SHOT_WEAPON_M16_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_M16_VOLUME);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				stereo = 1;
				break;
			case WEAPONTYPE_MP5:
				manager->m_sQueueSample.m_nSampleIndex = SFX_MP5_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_MP5_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				stereo = 1;
				break;
			case WEAPONTYPE_TEC9:
				manager->m_sQueueSample.m_nSampleIndex = SFX_TEC_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 500) + 17000;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				stereo = 1;
				break;
			case WEAPONTYPE_SILENCED_INGRAM:
				manager->m_sQueueSample.m_nSampleIndex = SFX_TEC_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 34000;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				stereo = 1;
				break;
			case WEAPONTYPE_RUGER:
				manager->m_sQueueSample.m_nSampleIndex = SFX_RUGER_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_RUGER_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_M4_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				stereo = 1;
				break;
			case WEAPONTYPE_M4:
				manager->m_sQueueSample.m_nSampleIndex = SFX_RUGER_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 43150;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_M4_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				stereo = 1;
				break;
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_MINIGUN:
				manager->m_sQueueSample.m_nSampleIndex = SFX_UZI_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_UZI_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[3] % 15 + PED_ONE_SHOT_WEAPON_UZI_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				stereo = 1;
				break;
			case WEAPONTYPE_SNIPERRIFLE:
			case WEAPONTYPE_LASERSCOPE:
				manager->m_sQueueSample.m_nSampleIndex = SFX_SNIPER_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				if (AudioPedOneShotsHost_WeaponType(weapon) == WEAPONTYPE_SNIPERRIFLE)
					manager->m_sQueueSample.m_nFrequency = 25472;
				else
					manager->m_sQueueSample.m_nFrequency = 20182;
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[4] % 10 + PED_ONE_SHOT_WEAPON_SNIPERRIFLE_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				stereo = 1;
				break;
			case WEAPONTYPE_SPAS12_SHOTGUN:
				manager->m_sQueueSample.m_nSampleIndex = SFX_SPAS12_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_SPAS12_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[2] % 10 + PED_ONE_SHOT_WEAPON_SHOTGUN_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				stereo = 1;
				break;
			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_STUBBY_SHOTGUN:
				manager->m_sQueueSample.m_nSampleIndex = SFX_SHOTGUN_LEFT;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nCounter = iSound++;
				narrowSoundRange = 1;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_SHOTGUN_LEFT);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
				manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST;
				maxDist = (PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST * PED_ONE_SHOT_WEAPON_BULLET_ECHO_MAX_DIST);
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
				Vol = manager->m_anRandomTable[2] % 10 + PED_ONE_SHOT_WEAPON_SHOTGUN_VOLUME;
				AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
				stereo = 1;
				break;
			default:
				continue;
			}
			break;
		case SOUND_WEAPON_RELOAD:
			switch ((int32_t)manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i]) {
			case WEAPONTYPE_COLT45:
			case WEAPONTYPE_PYTHON:
				manager->m_sQueueSample.m_nSampleIndex = SFX_PISTOL_RELOAD;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_PISTOL_RELOAD) + AudioMath_RandomDisplacement(manager->m_anRandomTable, 300);
				break;
			case WEAPONTYPE_TEC9:
			case WEAPONTYPE_UZI:
			case WEAPONTYPE_SILENCED_INGRAM:
			case WEAPONTYPE_MP5:
			case WEAPONTYPE_M4:
			case WEAPONTYPE_M60:
			case WEAPONTYPE_HELICANNON:
				manager->m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				manager->m_sQueueSample.m_nFrequency = 39243;
				break;
			case WEAPONTYPE_SHOTGUN:
			case WEAPONTYPE_SPAS12_SHOTGUN:
			case WEAPONTYPE_STUBBY_SHOTGUN:
			case WEAPONTYPE_RUGER:
				manager->m_sQueueSample.m_nSampleIndex = SFX_AK47_RELOAD;
				manager->m_sQueueSample.m_nFrequency = 30290;
				break;
			case WEAPONTYPE_ROCKET:
			case WEAPONTYPE_ROCKETLAUNCHER:
				manager->m_sQueueSample.m_nSampleIndex = SFX_ROCKET_RELOAD;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_ROCKET_RELOAD);
				break;
			case WEAPONTYPE_SNIPERRIFLE:
			case WEAPONTYPE_LASERSCOPE:
				manager->m_sQueueSample.m_nSampleIndex = SFX_RIFLE_RELOAD;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_RIFLE_RELOAD);
				break;
			default:
				continue;
			}
			Vol = PED_ONE_SHOT_WEAPON_RELOAD_VOLUME;
			manager->m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = 1;
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, 300);
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nPriority = 5;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_RELOAD_MAX_DIST;
			maxDist = (PED_ONE_SHOT_WEAPON_RELOAD_MAX_DIST * PED_ONE_SHOT_WEAPON_RELOAD_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_RELOAD_VOLUME);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_WEAPON_HIT_PED:
			manager->m_sQueueSample.m_nSampleIndex = SFX_BULLET_PED;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_BULLET_PED);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 3);
			manager->m_sQueueSample.m_nPriority = 7;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST;
			maxDist = (PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST * PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = manager->m_anRandomTable[0] % 20 + PED_ONE_SHOT_WEAPON_HIT_PED_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			break;
		case SOUND_WEAPON_CHAINSAW_MADECONTACT:
			if (AudioGame_FindVehicle())
				continue;
			if ((int32_t)manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i] != ENTITY_TYPE_PED)
				// Report the ped impact through the explicit C audio owner
				AudioCollisionReport_Report(manager, AudioPedOneShotsHost_Entity(params->m_pPed), AudioPedOneShotsHost_Entity(params->m_pPed), SURFACE_CAR, SURFACE_TARMAC, 0.0f, 0.09f);
			manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_CHAINSAW_ATTACK;
#ifdef GTA_PS2
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_CAR_CHAINSAW;
#else
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif
			manager->m_sQueueSample.m_nCounter = 68;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 500) + 22000;
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST;
			maxDist = (PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST * PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 0;
			Vol = PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(SFX_CAR_CHAINSAW_ATTACK)
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 5;
			break;
		case SOUND_WEAPON_CHAINSAW_ATTACK:
			if (AudioGame_FindVehicle())
				continue;
			manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_CHAINSAW_ATTACK;
#ifdef GTA_PS2
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_CAR_CHAINSAW;
#else
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif
			manager->m_sQueueSample.m_nCounter = 68;
			manager->m_sQueueSample.m_nFrequency = 27000;
			manager->m_sQueueSample.m_nPriority = 2;
			manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST;
			maxDist = (PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST * PED_ONE_SHOT_WEAPON_CHAINSAW_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 0;
			Vol = PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(SFX_CAR_CHAINSAW_ATTACK)
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 5;
			break;
		case SOUND_WEAPON_CHAINSAW_IDLE:
			if (AudioGame_FindVehicle())
				continue;
			manager->m_sQueueSample.m_nSampleIndex = SFX_CAR_CHAINSAW_IDLE;
#ifdef GTA_PS2
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_CAR_CHAINSAW;
#else
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
#endif
			manager->m_sQueueSample.m_nCounter = 70;
			manager->m_sQueueSample.m_nFrequency = 27000;
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_CHAINSAW_IDLE_MAX_DIST;
			maxDist = (PED_ONE_SHOT_WEAPON_CHAINSAW_IDLE_MAX_DIST * PED_ONE_SHOT_WEAPON_CHAINSAW_IDLE_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 0;
			Vol = PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_LOOP_OFFSETS(SFX_CAR_CHAINSAW_IDLE)
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(PED_ONE_SHOT_WEAPON_CHAINSAW_VOLUME);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 0;
			manager->m_sQueueSample.m_nFramesToPlay = 5;
			break;
		case SOUND_WEAPON_BAT_ATTACK:
		case SOUND_WEAPON_KNIFE_ATTACK:
		{
			uint32_t soundParams = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i]; // wtf? storing int as float
			uint8_t damagerType = soundParams & 0xFF;
			uint32_t weaponType = soundParams >> 8;
			if (damagerType == ENTITY_TYPE_PED) {
				switch (weaponType) {
				case WEAPONTYPE_SCREWDRIVER:
				case WEAPONTYPE_KNIFE:
				case WEAPONTYPE_CLEAVER:
				case WEAPONTYPE_MACHETE:
				case WEAPONTYPE_KATANA:
					if (sound == SOUND_WEAPON_KNIFE_ATTACK)
						manager->m_sQueueSample.m_nSampleIndex = SFX_KNIFE_SLASH;
					else
						manager->m_sQueueSample.m_nSampleIndex = SFX_KNIFE_STAB;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
					manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
					break;
				case WEAPONTYPE_HAMMER:
					manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
					manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
					break;
				default:
					manager->m_sQueueSample.m_nSampleIndex = SFX_BAT_HIT_LEFT;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 2000) + 22000;
					stereo = 1;
					break;
				}
			}
			else {
				manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[4] % 6 + SFX_COL_CAR_PANEL_1;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
			}
			manager->m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = 1;
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_PUNCH_MAX_DIST;
			maxDist = (PED_ONE_SHOT_PUNCH_MAX_DIST * PED_ONE_SHOT_PUNCH_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = manager->m_anRandomTable[2] % 20 + PED_ONE_SHOT_PUNCH_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		}
		case SOUND_FIGHT_37:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			manager->m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_38:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			manager->m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_39:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_1;
			manager->m_sQueueSample.m_nFrequency = 20000;
			goto AddFightSound;
		case SOUND_FIGHT_40:
		case SOUND_186:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			manager->m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_41:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			manager->m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_42:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_2;
			manager->m_sQueueSample.m_nFrequency = 20000;
			goto AddFightSound;
		case SOUND_FIGHT_43:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			manager->m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_44:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			manager->m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_45:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_4;
			manager->m_sQueueSample.m_nFrequency = 20000;
			goto AddFightSound;
		case SOUND_FIGHT_46:
		case SOUND_187:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			manager->m_sQueueSample.m_nFrequency = 18000;
			goto AddFightSound;
		case SOUND_FIGHT_47:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			manager->m_sQueueSample.m_nFrequency = 16500;
			goto AddFightSound;
		case SOUND_FIGHT_48:
			manager->m_sQueueSample.m_nSampleIndex = SFX_FIGHT_5;
			manager->m_sQueueSample.m_nFrequency = 20000;
		AddFightSound:
			{
				uint32_t soundParams = manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i]; // wtf? storing int as float
				uint8_t damagerType = soundParams & 0xFF;
				uint32_t weaponType = soundParams >> 8;

				if (damagerType == ENTITY_TYPE_PED) {
					if (weaponType == WEAPONTYPE_BRASSKNUCKLE) {
						CPed* ped = params->m_pPed;
						uint32_t fightMove = AudioPedOneShotsHost_FightMove(ped);
						if (fightMove == FIGHTMOVE_BACKLEFT || fightMove == FIGHTMOVE_STDPUNCH || fightMove == FIGHTMOVE_PUNCH ||
							AudioPedOneShotsHost_State(ped) == PED_ATTACK) {
							void *damageEntity = AudioPedOneShotsHost_DamageEntity(ped);
							if (!damageEntity)
								manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
							else if (AudioPedOneShotsHost_EntityType(damageEntity) != ENTITY_TYPE_PED)
								manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
							else if (AudioPedOneShotsHost_FightMove((CPed *)damageEntity) != FIGHTMOVE_HITHEAD)
								manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[3] % 2 + SFX_HAMMER_HIT_1;
							else
								manager->m_sQueueSample.m_nSampleIndex = SFX_HAMMER_HIT_1;
						}
					}
				}
				else {
					manager->m_sQueueSample.m_nSampleIndex = manager->m_anRandomTable[4] % 6 + SFX_COL_CAR_PANEL_1;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
					manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency / 16);
				}
			}
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iSound;
			narrowSoundRange = 1;
			iSound++;
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_PUNCH_MAX_DIST;
			maxDist = (PED_ONE_SHOT_PUNCH_MAX_DIST * PED_ONE_SHOT_PUNCH_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = manager->m_anRandomTable[3] % 26 + PED_ONE_SHOT_PUNCH_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_SPLASH:
			if (manager->m_FrameCounter <= iSplashFrame)
				continue;
			iSplashFrame = manager->m_FrameCounter + 6;
			manager->m_sQueueSample.m_nSampleIndex = SFX_SPLASH_1;
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = 1;
			manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1400) + 20000;
			manager->m_sQueueSample.m_nPriority = 1;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_SPLASH_MAX_DIST;
			maxDist = (PED_ONE_SHOT_SPLASH_MAX_DIST * PED_ONE_SHOT_SPLASH_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			Vol = manager->m_anRandomTable[2] % 30 + PED_ONE_SHOT_SPLASH_PED_VOLUME;
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		case SOUND_MELEE_ATTACK_START:
		{
			uint32_t weaponType = ((uint32_t)manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_afVolume[i]) >> 8;
			switch (weaponType)
			{
			case WEAPONTYPE_SCREWDRIVER:
			case WEAPONTYPE_KNIFE:
			case WEAPONTYPE_CLEAVER:
			case WEAPONTYPE_MACHETE:
			case WEAPONTYPE_KATANA:
				manager->m_sQueueSample.m_nSampleIndex = SFX_KNIFE_SWING;
				break;
			default:
				manager->m_sQueueSample.m_nSampleIndex = SFX_GOLF_CLUB_SWING;
				break;
			}
			manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
			manager->m_sQueueSample.m_nCounter = iSound++;
			narrowSoundRange = 1;
			manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
			manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 4);
			manager->m_sQueueSample.m_nPriority = 3;
			manager->m_sQueueSample.m_fSpeedMultiplier = 0.0f;
			manager->m_sQueueSample.m_MaxDistance = PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST;
			if (weaponType == WEAPONTYPE_UNARMED || weaponType == WEAPONTYPE_BRASSKNUCKLE)
				Vol = manager->m_anRandomTable[1] % 10 + 35;
			else
				Vol = manager->m_anRandomTable[2] % 20 + 70;
			maxDist = (PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST * PED_ONE_SHOT_WEAPON_HIT_PED_MAX_DIST);
			manager->m_sQueueSample.m_nLoopCount = 1;
			AUDIO_PED_ONE_SHOTS_RESET_LOOP_OFFSETS
			AUDIO_PED_ONE_SHOTS_SET_EMITTING_VOLUME(Vol);
			manager->m_sQueueSample.m_bIs2D = 0;
			manager->m_sQueueSample.m_bStatic = 1;
			AUDIO_PED_ONE_SHOTS_SET_SOUND_REFLECTION(1);
			break;
		}
		default:
			AudioPedSpeech_Setup(manager, params, sound);
			continue;
		}

		if (narrowSoundRange && iSound > 60)
			iSound = 21;
		if (params->m_fDistance < maxDist) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				if (stereo) {
					if (manager->m_sQueueSample.m_fDistance < 0.2f * manager->m_sQueueSample.m_MaxDistance) {
						manager->m_sQueueSample.m_bIs2D = 1;
						manager->m_sQueueSample.m_nPan = 0;
					} else
						stereo = 0;
				}
				AUDIO_PED_ONE_SHOTS_SET_SOUND_REVERB(1);
				AudioRequests_Submit(manager);
				if (stereo) {
					manager->m_sQueueSample.m_nPan = 127;
					manager->m_sQueueSample.m_nSampleIndex++;
					if (manager->m_asAudioEntities[manager->m_sQueueSample.m_nEntityIndex].m_awAudioEvent[i] == SOUND_WEAPON_SHOT_FIRED &&
					    AudioPedOneShotsHost_WeaponType(weapon) == WEAPONTYPE_FLAMETHROWER)
						manager->m_sQueueSample.m_nCounter++;
					else {
						manager->m_sQueueSample.m_nCounter = iSound++;
						if (iSound > 60)
							iSound = 21;
					}
					AudioRequests_Submit(manager);
				}
			}
		}
	}
}
//- rouz edit (ChatGPT)
