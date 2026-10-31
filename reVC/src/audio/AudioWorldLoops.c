//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "sampman.h"
#include "../weapons/WeaponType.h"
#include <math.h>

enum {
	PROJECTILE_ROCKET_MAX_DIST = 90,
	PROJECTILE_ROCKET_VOLUME = MAX_VOLUME,

	PROJECTILE_MOLOTOV_MAX_DIST = 30,
	PROJECTILE_MOLOTOV_VOLUME = 50,

	PROJECTILE_TEARGAS_MAX_DIST = 40,
	PROJECTILE_TEARGAS_VOLUME = 80,

	ESCALATOR_MAX_DIST = 30,
	ESCALATOR_VOLUME = 26,

	ARCADE_MAX_DIST = 18,
	ARCADE_VOLUME = 50,

};

AudioSoundPosition aVecExtraSoundPosition[2] = { {-1042.546f, 88.794f, 11.324f}, {-1004.476f, 181.697f, 11.324f} };

#ifndef GTA_PS2
#define AUDIO_WORLD_LOOPS_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_WORLD_LOOPS_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_WORLD_LOOPS_RESET_LOOP_OFFSETS
#define AUDIO_WORLD_LOOPS_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_WORLD_LOOPS_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_WORLD_LOOPS_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_WORLD_LOOPS_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_WORLD_LOOPS_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_WORLD_LOOPS_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_WORLD_LOOPS_SET_SOUND_REVERB(b)
#endif

void
AudioWorldLoops_Projectiles(cAudioManager *manager)
{
	// Preserve the complete original generator and its live query and submission order
	uint8_t Vol;

	for (uint8_t i = 0; i < NUM_PROJECTILES; i++) {
		if (AudioWorldLoopsHost_ProjectileInUse(i)) {
			switch (AudioWorldLoopsHost_ProjectileWeapon(i)) {
			case WEAPONTYPE_TEARGAS:
				Vol = PROJECTILE_TEARGAS_VOLUME;
				manager->m_sQueueSample.m_MaxDistance = PROJECTILE_TEARGAS_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_PALM_TREE_LO;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nFrequency = 13879;
				manager->m_sQueueSample.m_nPriority = 7;
				break;
			case WEAPONTYPE_ROCKET:
				Vol = PROJECTILE_ROCKET_VOLUME;
				manager->m_sQueueSample.m_MaxDistance = PROJECTILE_ROCKET_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_ROCKET_FLY;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_ROCKET_FLY);
				manager->m_sQueueSample.m_nPriority = 3;
				break;
			case WEAPONTYPE_MOLOTOV:
				Vol = PROJECTILE_MOLOTOV_VOLUME;
				manager->m_sQueueSample.m_MaxDistance = PROJECTILE_MOLOTOV_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_PED_ON_FIRE;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nFrequency = 32 * SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_PED_ON_FIRE) / 25;
				manager->m_sQueueSample.m_nPriority = 7;
				break;
			default:
				continue;
			}
			manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
			manager->m_sQueueSample.m_nFramesToPlay = 3;
			// Copy the game position into the C sound record
			AudioWorldLoopsHost_ProjectilePosition(i, &manager->m_sQueueSample.m_vecPos);
			float distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
			if (distSquared < (manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)) {
				manager->m_sQueueSample.m_fDistance = (distSquared <= 0.0f ? 0.0f : sqrtf(distSquared));
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = i;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_WORLD_LOOPS_SET_EMITTING_VOLUME(Vol);
					AUDIO_WORLD_LOOPS_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_bStatic = 0;
					AUDIO_WORLD_LOOPS_SET_SOUND_REVERB(1);
					AUDIO_WORLD_LOOPS_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
	}
}

void
AudioWorldLoops_Escalators(cAudioManager *manager)
{
	// Preserve the complete original generator and its live query and submission order
	float distance;

	for (int i = 0; i < AudioWorldLoopsHost_EscalatorCount(); i++) {
		if (!AudioWorldLoopsHost_EscalatorActive(i))
			continue;
		// Copy the game position into the C sound record
		AudioWorldLoopsHost_EscalatorPosition(i, &manager->m_sQueueSample.m_vecPos);
		distance = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
		if (distance < (ESCALATOR_MAX_DIST * ESCALATOR_MAX_DIST)) {
			manager->m_sQueueSample.m_fDistance = (distance <= 0.0f ? 0.0f : sqrtf(distance));
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(ESCALATOR_VOLUME, ESCALATOR_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nSampleIndex = SFX_BOAT_V12_LOOP;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nFrequency = i * 50 % 250 + 3973;
				manager->m_sQueueSample.m_nPriority = 3;
				manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
				manager->m_sQueueSample.m_nFramesToPlay = 5;
				manager->m_sQueueSample.m_MaxDistance = ESCALATOR_MAX_DIST;
				manager->m_sQueueSample.m_nCounter = i;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_WORLD_LOOPS_SET_EMITTING_VOLUME(ESCALATOR_VOLUME);
				AUDIO_WORLD_LOOPS_SET_LOOP_OFFSETS(SFX_BOAT_V12_LOOP)
				AUDIO_WORLD_LOOPS_SET_SOUND_REVERB(1);
				manager->m_sQueueSample.m_bStatic = 0;
				AUDIO_WORLD_LOOPS_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
	}
}

void
AudioWorldLoops_ExtraSounds(cAudioManager *manager)
{
	// Preserve the complete original generator and its live query and submission order
	float distance;

	for (int i = 0; i < (sizeof(aVecExtraSoundPosition) / sizeof(aVecExtraSoundPosition[0])); i++) {
		// Copy the game position into the C sound record
		manager->m_sQueueSample.m_vecPos = aVecExtraSoundPosition[i];
		distance = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
		if (distance < (ARCADE_MAX_DIST * ARCADE_MAX_DIST)) {
			manager->m_sQueueSample.m_fDistance = (distance <= 0.0f ? 0.0f : sqrtf(distance));
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(ARCADE_VOLUME, ARCADE_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if (manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = i;
				manager->m_sQueueSample.m_nSampleIndex = SFX_ARCADE;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_ARCADE);
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nLoopCount = 0;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nPriority = 4;
				manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
				AUDIO_WORLD_LOOPS_SET_EMITTING_VOLUME(ARCADE_VOLUME);
				AUDIO_WORLD_LOOPS_SET_LOOP_OFFSETS(SFX_ARCADE)
				AUDIO_WORLD_LOOPS_SET_SOUND_REVERB(1);
				manager->m_sQueueSample.m_MaxDistance = ARCADE_MAX_DIST;
				AUDIO_WORLD_LOOPS_SET_SOUND_REFLECTION(0);
				manager->m_sQueueSample.m_nFramesToPlay = 3;
				AudioRequests_Submit(manager);
			}
		}
	}
}

//- rouz edit (ChatGPT)
