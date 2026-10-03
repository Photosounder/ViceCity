//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "../core/SurfaceTypes.h"
#include "sampman.h"
#include <math.h>

#define AudioCollisionSounds_Min(a,b) ((a) < (b) ? (a) : (b))
#ifndef GTA_PS2
#define AUDIO_COLLISION_SOUNDS_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_COLLISION_SOUNDS_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_COLLISION_SOUNDS_RESET_LOOP_OFFSETS
#define AUDIO_COLLISION_SOUNDS_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_COLLISION_SOUNDS_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_COLLISION_SOUNDS_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_COLLISION_SOUNDS_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_COLLISION_SOUNDS_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_COLLISION_SOUNDS_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_COLLISION_SOUNDS_SET_SOUND_REVERB(b)
#endif


static const uint32_t gOneShotCol[] = {SFX_COL_TARMAC_1,
                                    SFX_COL_TARMAC_1,
                                    SFX_COL_GRASS_1,
                                    SFX_COL_GRAVEL_1,
                                    SFX_COL_MUD_1,
                                    SFX_COL_TARMAC_1,
                                    SFX_COL_CAR_1,
                                    SFX_COL_GRASS_1,
                                    SFX_COL_SCAFFOLD_POLE_1,
                                    SFX_COL_GARAGE_DOOR_1,
                                    SFX_COL_CAR_PANEL_1,
                                    SFX_COL_THICK_METAL_PLATE_1,
                                    SFX_COL_SCAFFOLD_POLE_1,
                                    SFX_COL_LAMP_POST_1,
                                    SFX_COL_HYDRANT_1,
                                    SFX_COL_HYDRANT_1,
                                    SFX_COL_METAL_CHAIN_FENCE_1,
                                    SFX_COL_PED_1,
                                    SFX_COL_SAND_1,
                                    SFX_SPLASH_1,
                                    SFX_COL_WOOD_CRATES_1,
                                    SFX_COL_WOOD_BENCH_1,
                                    SFX_COL_WOOD_SOLID_1,
                                    SFX_COL_GRASS_1,
                                    SFX_COL_GRASS_1,
                                    SFX_COL_VEG_1,
                                    SFX_COL_TARMAC_1,
                                    SFX_COL_CONTAINER_1,
                                    SFX_COL_NEWS_VENDOR_1,
                                    SFX_TYRE_BUMP,
                                    SFX_COL_CARDBOARD_1,
                                    SFX_COL_TARMAC_1,
                                    SFX_COL_GATE,
                                    SFX_COL_SAND_1,
                                    SFX_COL_TARMAC_1 };

static float AudioCollisionSounds_Sqrt(float value)
{
    // Retain the original owner square root helper's nonpositive clamp
    return value <= 0.0f ? 0.0f : sqrtf(value);
}

void
AudioCollisionSounds_OneShot(cAudioManager *manager, const cAudioCollision *col)
{
    // Retain surface selection, numeric conversions and sound request ordering
	uint16_t s1;
	uint16_t s2;

	uint32_t emittingVol;
	float ratio;

	static uint16_t counter = 28;

	for(int32_t i = 0; i < 2; i++) {
		if(i) {
			s1 = col->m_bSurface2;
			s2 = col->m_bSurface1;
		} else {
			s1 = col->m_bSurface1;
			s2 = col->m_bSurface2;
		}
		ratio = AudioCollision_GetOneShotRatio(s1, col->m_fIntensity1);
		if(s1 == SURFACE_CAR && s2 == SURFACE_PED) ratio /= 4.0f;
		if(s1 == SURFACE_CAR && ratio < 0.6f) {
			s1 = SURFACE_CAR_PANEL;
			ratio = AudioCollisionSounds_Min(1.f, 2.f * ratio);
		}
		emittingVol = 40 * ratio;
		if(emittingVol) {
			manager->m_sQueueSample.m_fDistance = AudioCollisionSounds_Sqrt(col->m_fDistance);
			manager->m_sQueueSample.m_nVolume =
			    AudioMath_ComputeVolume(emittingVol, COLLISION_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if(manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nSampleIndex = gOneShotCol[s1];
				switch(manager->m_sQueueSample.m_nSampleIndex) {
				case SFX_COL_TARMAC_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[3] % 5;
					break;
				case SFX_COL_CAR_PANEL_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[0] % 6;
					break;
				case SFX_COL_LAMP_POST_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[1] % 2;
					break;
				case SFX_COL_METAL_CHAIN_FENCE_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[3] % 4;
					break;
				case SFX_COL_PED_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[4] % 2;
					break;
				case SFX_COL_WOOD_CRATES_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[4] % 4;
					break;
				case SFX_COL_WOOD_BENCH_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[1] % 4;
					break;
				case SFX_COL_VEG_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[2] % 5;
					break;
				case SFX_COL_NEWS_VENDOR_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[2] % 3;
					break;
				case SFX_COL_CAR_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[1] % 5;
					break;
				case SFX_COL_CARDBOARD_1:
					manager->m_sQueueSample.m_nSampleIndex += manager->m_anRandomTable[3] % 2;
					break;
				default: break;
				}
				switch(s1) {
				case SURFACE_GLASS: manager->m_sQueueSample.m_nFrequency = 13500; break;
				case SURFACE_GIRDER: manager->m_sQueueSample.m_nFrequency = 8819; break;
				case SURFACE_WATER:
					manager->m_sQueueSample.m_nFrequency =
					    2 * SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
					break;
				case SURFACE_RUBBER: manager->m_sQueueSample.m_nFrequency = 6000; break;
				case SURFACE_PLASTIC: manager->m_sQueueSample.m_nFrequency = 8000; break;
				default:
					manager->m_sQueueSample.m_nFrequency =
					    SampleManager_GetSampleBaseFrequency(&SampleManager, manager->m_sQueueSample.m_nSampleIndex);
					break;
				}
				manager->m_sQueueSample.m_nFrequency += AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency / 16);
				manager->m_sQueueSample.m_nCounter = counter++;
				if(counter >= 255) counter = 28;
				// Copy the C collision position into the game sample vector
				manager->m_sQueueSample.m_vecPos.x = col->m_vecPosition.x;
				manager->m_sQueueSample.m_vecPos.y = col->m_vecPosition.y;
				manager->m_sQueueSample.m_vecPos.z = col->m_vecPosition.z;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 11;
				manager->m_sQueueSample.m_nLoopCount = 1;
				AUDIO_COLLISION_SOUNDS_SET_EMITTING_VOLUME(emittingVol);
				AUDIO_COLLISION_SOUNDS_RESET_LOOP_OFFSETS
				manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
				manager->m_sQueueSample.m_MaxDistance = COLLISION_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 1;
				AUDIO_COLLISION_SOUNDS_SET_SOUND_REVERB(1);
				AUDIO_COLLISION_SOUNDS_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
	}
}

void
AudioCollisionSounds_Looping(cAudioManager *manager, const cAudioCollision *col, uint8_t counter)
{
    // Retain surface selection, numeric conversions and sound request ordering
    uint8_t distCalculated = 0;
	if(col->m_fIntensity2 > 0.0016f) {
		uint8_t emittingVol = AudioCollisionSounds_LoopingVolume(manager, col);
		if(emittingVol) {
			AudioMath_CalculateDistance(&distCalculated, &manager->m_sQueueSample.m_fDistance, manager->m_sQueueSample.m_fDistance);
			manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(emittingVol, COLLISION_MAX_DIST, manager->m_sQueueSample.m_fDistance);
			if(manager->m_sQueueSample.m_nVolume > 0) {
				manager->m_sQueueSample.m_nCounter = counter;
				// Copy the C collision position into the game sample vector
				manager->m_sQueueSample.m_vecPos.x = col->m_vecPosition.x;
				manager->m_sQueueSample.m_vecPos.y = col->m_vecPosition.y;
				manager->m_sQueueSample.m_vecPos.z = col->m_vecPosition.z;
				manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
				manager->m_sQueueSample.m_bIs2D = 0;
				manager->m_sQueueSample.m_nPriority = 7;
				manager->m_sQueueSample.m_nLoopCount = 0;
				AUDIO_COLLISION_SOUNDS_SET_EMITTING_VOLUME(emittingVol);
				AUDIO_COLLISION_SOUNDS_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
				manager->m_sQueueSample.m_fSpeedMultiplier = 4.0f;
				manager->m_sQueueSample.m_MaxDistance = COLLISION_MAX_DIST;
				manager->m_sQueueSample.m_bStatic = 0;
				manager->m_sQueueSample.m_nFramesToPlay = 5;
				AUDIO_COLLISION_SOUNDS_SET_SOUND_REVERB(1);
				AUDIO_COLLISION_SOUNDS_SET_SOUND_REFLECTION(0);
				AudioRequests_Submit(manager);
			}
		}
	}
}

uint32_t
AudioCollisionSounds_LoopingVolume(cAudioManager *manager, const cAudioCollision *audioCollision)
{
    // Retain surface selection, numeric conversions and sound request ordering
	uint8_t surface1 = audioCollision->m_bSurface1;
	uint8_t surface2 = audioCollision->m_bSurface2;
	int32_t vol;
	float ratio;

	if(surface1 == SURFACE_GRASS || surface2 == SURFACE_GRASS || surface1 == SURFACE_HEDGE ||
	   surface2 == SURFACE_HEDGE) {
		ratio = AudioCollision_GetRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
		manager->m_sQueueSample.m_nSampleIndex = SFX_RAIN;
		manager->m_sQueueSample.m_nFrequency = 13000.f * ratio + 35000;
		vol = 50.f * ratio;
	} else if(surface1 == SURFACE_WATER || surface2 == SURFACE_WATER) {
		ratio = AudioCollision_GetRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
		manager->m_sQueueSample.m_nSampleIndex = SFX_BOAT_WATER_LOOP;
		manager->m_sQueueSample.m_nFrequency = 6050.f * ratio + 16000;
		vol = 30.f * ratio;
	} else if(surface1 == SURFACE_GRAVEL || surface2 == SURFACE_GRAVEL || surface1 == SURFACE_MUD_DRY || surface2 == SURFACE_MUD_DRY ||
	          surface1 == SURFACE_SAND || surface2 == SURFACE_SAND || surface1 == SURFACE_SAND_BEACH || surface2 == SURFACE_SAND_BEACH) {
		ratio = AudioCollision_GetRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
		manager->m_sQueueSample.m_nSampleIndex = SFX_GRAVEL_SKID;
		manager->m_sQueueSample.m_nFrequency = 6000.f * ratio + 10000;
		vol = 50.f * ratio;
	} else if(surface1 == SURFACE_PED || surface2 == SURFACE_PED) {
		return 0;
	} else {
		ratio = AudioCollision_GetRatio(audioCollision->m_fIntensity2, 0.0001f, 0.09f, 0.0899f);
		manager->m_sQueueSample.m_nSampleIndex = SFX_SCRAPE_CAR_1;
		manager->m_sQueueSample.m_nFrequency = 10000.f * ratio + 10000;
		vol = 40.f * ratio;
	}
	if(audioCollision->m_nBaseVolume < 2) vol = audioCollision->m_nBaseVolume * vol / 2;
	return vol;
}

//- rouz edit (ChatGPT)
