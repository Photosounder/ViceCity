//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "AudioGeometry.h"
#include "sampman.h"
#include "../weapons/Explosion_access.h"
#include <math.h>

enum {
	EXPLOSION_DEFAULT_MAX_DIST = 200,
	EXPLOSION_MOLOTOV_MAX_DIST = 150,
	EXPLOSION_MINE_MAX_DIST = 200,
};

#ifndef GTA_PS2
#define AUDIO_EXPLOSIONS_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_EXPLOSIONS_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_EXPLOSIONS_RESET_LOOP_OFFSETS
#define AUDIO_EXPLOSIONS_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_EXPLOSIONS_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_EXPLOSIONS_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_EXPLOSIONS_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_EXPLOSIONS_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_EXPLOSIONS_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_EXPLOSIONS_SET_SOUND_REVERB(b)
#endif

void
AudioExplosions_Process(cAudioManager *manager, int32_t id)
{
	// Retain the original scan and consume each eligible explosion before distance checks
	uint8_t type;
	float distSquared;

	for (uint8_t i = 0; i < NUM_EXPLOSIONS; i++) {
		if (Explosion_DoesExplosionMakeSound(i) && Explosion_GetExplosionActiveCounter(i) == 1) {
			Explosion_ResetExplosionActiveCounter(i);
			// Select the original sample, frequency and reflection settings for this type
			type = Explosion_GetExplosionType(i);
			switch (type) {
			case EXPLOSION_GRENADE:
			case EXPLOSION_ROCKET:
			case EXPLOSION_BARREL:
			case EXPLOSION_TANK_GRENADE:
				manager->m_sQueueSample.m_MaxDistance = EXPLOSION_DEFAULT_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_2;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 19000;
				manager->m_sQueueSample.m_nPriority = 0;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				AUDIO_EXPLOSIONS_SET_SOUND_REFLECTION(1);
				break;
			case EXPLOSION_MINE:
			case EXPLOSION_HELI_BOMB:
				manager->m_sQueueSample.m_MaxDistance = EXPLOSION_MINE_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_ROCKET_LEFT;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 12347;
				manager->m_sQueueSample.m_nPriority = 0;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				AUDIO_EXPLOSIONS_SET_SOUND_REFLECTION(1);
				break;
			case EXPLOSION_MOLOTOV:
				manager->m_sQueueSample.m_MaxDistance = EXPLOSION_MOLOTOV_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_3;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 19000;
				manager->m_sQueueSample.m_nPriority = 0;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				AUDIO_EXPLOSIONS_SET_SOUND_REFLECTION(0);
				break;
			default:
				manager->m_sQueueSample.m_MaxDistance = EXPLOSION_DEFAULT_MAX_DIST;
				manager->m_sQueueSample.m_nSampleIndex = SFX_EXPLOSION_1;
				manager->m_sQueueSample.m_nFrequency = AudioMath_RandomDisplacement(manager->m_anRandomTable, 1000) + 19500;
				if (type == EXPLOSION_HELI)
					manager->m_sQueueSample.m_nFrequency = 8 * manager->m_sQueueSample.m_nFrequency / 10; //same *= 8 / 10;
				manager->m_sQueueSample.m_nPriority = 0;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_GENERIC_EXTRA;
#ifdef FIX_BUGS
				AUDIO_EXPLOSIONS_SET_SOUND_REFLECTION(1);
#endif
				break;
			}
			// Copy the live explosion position into the C sound record
			ExplosionPosition position=Explosion_GetExplosionPosition(i);
			manager->m_sQueueSample.m_vecPos.x=position.x;
			manager->m_sQueueSample.m_vecPos.y=position.y;
			manager->m_sQueueSample.m_vecPos.z=position.z;
			distSquared = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
			// Submit only an audible sound inside the original strict distance boundary
			if (distSquared < (manager->m_sQueueSample.m_MaxDistance * manager->m_sQueueSample.m_MaxDistance)) {
				manager->m_sQueueSample.m_fDistance = (distSquared <= 0.0f ? 0.0f : sqrtf(distSquared));
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(MAX_VOLUME, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = i;
					manager->m_sQueueSample.m_fSpeedMultiplier = 2.0f;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nLoopCount = 1;
					manager->m_sQueueSample.m_bStatic = 1;
					AUDIO_EXPLOSIONS_SET_EMITTING_VOLUME(MAX_VOLUME);
					AUDIO_EXPLOSIONS_RESET_LOOP_OFFSETS
					AUDIO_EXPLOSIONS_SET_SOUND_REVERB(1);
					AudioRequests_Submit(manager);
				}
			}
		}
	}
}
//- rouz edit (ChatGPT)
