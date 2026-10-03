//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "sampman.h"
#include <math.h>
#include <string.h>

static float AudioRelease_Square(float value)
{
    // Preserve the original float square helper and its argument conversion
    return value * value;
}

static float AudioRelease_Sqrt(float value)
{
    // Retain the owner helper's nonpositive clamp before the square root
    return value <= 0.0f ? 0.0f : sqrtf(value);
}

void
AudioRelease_Add(cAudioManager *manager)
{
	// Carry over unmatched sounds with the original attachment and fading rules
	// in case someone would want to increase it
#ifdef FIX_BUGS
	uint8_t toProcess[NUM_CHANNELS_GENERIC];
#else
	uint8_t toProcess[44];
#endif

	uint8_t queue = manager->m_nActiveQueue == 0 ? 1 : 0;

	for (uint8_t i = 0; i < manager->m_nRequestedCount[queue]; i++) {
		tSound *sample = &manager->m_aRequestedQueue[queue][manager->m_aRequestedOrderList[queue][i]];
		if (sample->m_bIsPlayingFinished)
			continue;

		toProcess[i] = 0;
		for (uint8_t j = 0; j < manager->m_nRequestedCount[manager->m_nActiveQueue]; j++) {
			if (sample->m_nEntityIndex == manager->m_aRequestedQueue[manager->m_nActiveQueue][manager->m_aRequestedOrderList[manager->m_nActiveQueue][j]].m_nEntityIndex &&
			    sample->m_nCounter == manager->m_aRequestedQueue[manager->m_nActiveQueue][manager->m_aRequestedOrderList[manager->m_nActiveQueue][j]].m_nCounter) {
				toProcess[i] = 1;
				break;
			}
		}
		if(!toProcess[i]) {
#ifdef AUDIO_REFLECTIONS
			if(sample->m_nCounter <= 255 || sample->m_nReflectionDelay == 0) // check if not delayed reflection
#endif
			{
#ifdef ATTACH_RELEASING_SOUNDS_TO_ENTITIES
				if (sample->m_nCounter <= 255 && !sample->m_bIs2D) { // check if not reflection and is a 3D sound
					void *entity = AudioEntities_GetPointer(manager->m_asAudioEntities, manager->m_bIsInitialised, sample->m_nEntityIndex);
					if (entity && manager->m_asAudioEntities[sample->m_nEntityIndex].m_nType == AUDIOTYPE_PHYSICAL) {
						// Copy the game position into the C sound record
						AudioReleaseHost_Position(entity, &sample->m_vecPos);
						float oldDistance = sample->m_fDistance;
						sample->m_fDistance = AudioRelease_Sqrt(AudioGeometry_DistanceSquared(&sample->m_vecPos));
						if (sample->m_nSampleIndex >= SAMPLEBANK_PED_START && sample->m_nSampleIndex <= SAMPLEBANK_PED_END) { // check if it's ped comment
							uint8_t vol;
							if (AudioReleaseHost_LineClear(&sample->m_vecPos))
								vol = PED_COMMENT_VOLUME;
							else
								vol = PED_COMMENT_VOLUME_BEHIND_WALL;
#ifdef EXTERNAL_3D_SOUND
							sample->m_nEmittingVolume = vol;
#endif
							sample->m_nVolume = AudioMath_ComputeVolume(vol, sample->m_MaxDistance, sample->m_fDistance);
						} else {
							// calculate new volume with changed distance
							float volumeDiff = AudioRelease_Square((sample->m_MaxDistance - sample->m_fDistance) / (sample->m_MaxDistance - oldDistance));
							if (volumeDiff > 0.0f) {
								uint8_t newVolume = volumeDiff * sample->m_nVolume;
								if (sample->m_nVolumeChange > 0)
									sample->m_nVolumeChange = volumeDiff * sample->m_nVolumeChange;
#if defined(FIX_BUGS) && defined(EXTERNAL_3D_SOUND)
								if (sample->m_nEmittingVolumeChange > 0)
									sample->m_nEmittingVolumeChange = volumeDiff * sample->m_nEmittingVolumeChange;
#endif
								sample->m_nVolume = (MAX_VOLUME < newVolume ? MAX_VOLUME : newVolume);
							}
						}
						if (sample->m_nVolume == 0)
							sample->m_nFramesToPlay = 0;
					}
				}
#endif
#ifdef FIX_BUGS
				if (sample->m_nFramesToPlay <= 0)
					continue;
				if (sample->m_nLoopCount == 0) {
					if (sample->m_nVolumeChange == -1) {
						sample->m_nVolumeChange = sample->m_nVolume / sample->m_nFramesToPlay;
						if (sample->m_nVolumeChange <= 0)
							sample->m_nVolumeChange = 1;
#ifdef EXTERNAL_3D_SOUND
						sample->m_nEmittingVolumeChange = sample->m_nEmittingVolume / sample->m_nFramesToPlay;
						if (sample->m_nEmittingVolumeChange <= 0)
							sample->m_nEmittingVolumeChange = 1;
#endif
					}
					if (sample->m_nVolume <= sample->m_nVolumeChange * AudioReleaseHost_TimeStep()) {
						sample->m_nFramesToPlay = 0;
						continue;
					}
					sample->m_nVolume -= sample->m_nVolumeChange * AudioReleaseHost_TimeStep();
#ifdef EXTERNAL_3D_SOUND
					if (sample->m_nEmittingVolume <= sample->m_nEmittingVolumeChange * AudioReleaseHost_TimeStep()) {
						sample->m_nFramesToPlay = 0;
						continue;
					}
					sample->m_nEmittingVolume -= sample->m_nEmittingVolumeChange * AudioReleaseHost_TimeStep();
#endif
				}
				sample->m_nFramesToPlay -= AudioReleaseHost_TimeStep();
				if (sample->m_nFramesToPlay < 0)
					sample->m_nFramesToPlay = 0;
#else
				if (sample->m_nFramesToPlay == 0)
					continue;
				if (sample->m_nLoopCount == 0) {
					if (sample->m_nVolumeChange == -1) {
						sample->m_nVolumeChange = sample->m_nVolume / sample->m_nFramesToPlay;
						if (sample->m_nVolumeChange <= 0)
							sample->m_nVolumeChange = 1;
					}
					if (sample->m_nVolume <= sample->m_nVolumeChange) {
						sample->m_nFramesToPlay = 0;
						continue;
					}
					sample->m_nVolume -= sample->m_nVolumeChange;
				}
				sample->m_nFramesToPlay--;
#endif
				if (manager->m_bReduceReleasingPriority) {
					if (sample->m_nPriority < 20)
						sample->m_nPriority++;
				}
				sample->m_bStatic = 0;
			}
			memcpy(&manager->m_sQueueSample, sample, sizeof(tSound));
			AudioRequests_Submit(manager);
		}
	}
}
//- rouz edit (ChatGPT)
