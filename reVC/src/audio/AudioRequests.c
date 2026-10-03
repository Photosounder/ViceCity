//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "sampman.h"

void
AudioRequests_Submit(cAudioManager *manager)
{
	// Preserve the original queue gates, write order and reflection state changes
	uint32_t finalPriority;
	uint8_t sampleIndex;
#ifdef AUDIO_REFLECTIONS
	uint8_t bReflections;
#endif

	if (manager->m_sQueueSample.m_nSampleIndex < TOTAL_AUDIO_SAMPLES) {
		finalPriority = manager->m_sQueueSample.m_nPriority * (MAX_VOLUME - manager->m_sQueueSample.m_nVolume);
		sampleIndex = manager->m_nRequestedCount[manager->m_nActiveQueue];
		if (sampleIndex >= manager->m_nActiveSamples) {
			sampleIndex = manager->m_aRequestedOrderList[manager->m_nActiveQueue][manager->m_nActiveSamples - 1];
			if (manager->m_aRequestedQueue[manager->m_nActiveQueue][sampleIndex].m_nFinalPriority <= finalPriority)
				return;
		} else
			manager->m_nRequestedCount[manager->m_nActiveQueue]++;
		manager->m_sQueueSample.m_nFinalPriority = finalPriority;
		manager->m_sQueueSample.m_bIsPlayingFinished = 0;
#ifdef AUDIO_REFLECTIONS
		if (manager->m_sQueueSample.m_bIs2D || AudioRequestsHost_InRoom()) {
			manager->m_sQueueSample.m_bReflections = 0;
			manager->m_sQueueSample.m_nReflectionDelay = 0;
		}
		if (manager->m_bDynamicAcousticModelingStatus && manager->m_sQueueSample.m_nLoopCount > 0) {
			bReflections = manager->m_sQueueSample.m_bReflections;
		} else {
			bReflections = 0;
			manager->m_sQueueSample.m_nReflectionDelay = 0;
		}
		manager->m_sQueueSample.m_bReflections = 0;

		if ( manager->m_bIsSurround && manager->m_sQueueSample.m_bIs2D )
			manager->m_sQueueSample.m_nFrontRearPan = 30;
#ifdef AUDIO_REVERB
		if (!manager->m_bDynamicAcousticModelingStatus)
			manager->m_sQueueSample.m_bReverb = 0;
#endif
#endif

		manager->m_aRequestedQueue[manager->m_nActiveQueue][sampleIndex] = manager->m_sQueueSample;

		AudioSoundQueue_InsertOrder(manager->m_aRequestedQueue[manager->m_nActiveQueue], manager->m_aRequestedOrderList[manager->m_nActiveQueue], manager->m_nActiveSamples, sampleIndex);
#ifdef AUDIO_REFLECTIONS
		if (bReflections)
			AudioRequests_AddReflections(manager);
#endif
	}
}

#ifdef AUDIO_REFLECTIONS
void
AudioRequests_AddReflections(cAudioManager *manager)
{
	// Preserve the original queue gates, write order and reflection state changes
#ifdef FIX_BUGS
  	uint32_t oldFreq = 0;
#else
  	uint32_t oldFreq;
#endif
	float reflectionDistance;
	int32_t noise;
	uint8_t emittingVolume;

	uint32_t oldCounter = manager->m_sQueueSample.m_nCounter;
	float oldDist = manager->m_sQueueSample.m_fDistance;
	// Copy the sound coordinates for the game vector operation
	AudioSoundPosition oldPos = manager->m_sQueueSample.m_vecPos;
#ifndef USE_TIME_SCALE_FOR_AUDIO
	if ( AudioRequestsHost_SlowMotion() ) {
		emittingVolume = manager->m_sQueueSample.m_nVolume;
		oldFreq = manager->m_sQueueSample.m_nFrequency;
	} else
#endif
		emittingVolume = (9 * manager->m_sQueueSample.m_nVolume) >> 4;
	manager->m_sQueueSample.m_MaxDistance /= 2.0f;

	uint32_t halfOldFreq = oldFreq >> 1;

	for (uint32_t i = 0; i < (sizeof(manager->m_afReflectionsDistances) / sizeof(manager->m_afReflectionsDistances[0])); i++) {
#ifndef USE_TIME_SCALE_FOR_AUDIO
		if ( AudioRequestsHost_SlowMotion() )
			manager->m_afReflectionsDistances[i] = (manager->m_anRandomTable[i % 4] % 3) * 100.0f / 8.0f;
#endif

		reflectionDistance = manager->m_afReflectionsDistances[i];
		if (reflectionDistance > 0.0f && reflectionDistance < 100.0f && reflectionDistance < manager->m_sQueueSample.m_MaxDistance) {
#ifndef USE_TIME_SCALE_FOR_AUDIO
			manager->m_sQueueSample.m_nReflectionDelay = AudioRequestsHost_SlowMotion() ? (reflectionDistance * 800.0f / 1029.0f) : (reflectionDistance * 500.0f / 1029.0f);
#else
			manager->m_sQueueSample.m_nReflectionDelay = reflectionDistance * 500.0f / 1029.0f;
#endif
			if (manager->m_sQueueSample.m_nReflectionDelay > 3) {
				manager->m_sQueueSample.m_fDistance = manager->m_afReflectionsDistances[i];
				#ifdef EXTERNAL_3D_SOUND
				manager->m_sQueueSample.m_nEmittingVolume = emittingVolume;
#endif
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(emittingVolume, manager->m_sQueueSample.m_MaxDistance, manager->m_sQueueSample.m_fDistance);

				if (manager->m_sQueueSample.m_nVolume > emittingVolume >> 4) {
					manager->m_sQueueSample.m_nCounter = oldCounter + ((i + 1) << 8);
					if (manager->m_sQueueSample.m_nLoopCount > 0) {
#ifndef USE_TIME_SCALE_FOR_AUDIO
						if ( AudioRequestsHost_SlowMotion() ) {
							manager->m_sQueueSample.m_nFrequency = halfOldFreq + ((halfOldFreq * i) / (sizeof(manager->m_afReflectionsDistances) / sizeof(manager->m_afReflectionsDistances[0])));
						} else
#endif
						{
							noise = AudioMath_RandomDisplacement(manager->m_anRandomTable, manager->m_sQueueSample.m_nFrequency >> 5);
							if (noise > 0)
								manager->m_sQueueSample.m_nFrequency -= noise;
							else
								manager->m_sQueueSample.m_nFrequency += noise;
						}
					}
					manager->m_sQueueSample.m_nPriority += 20;
					// Copy the plain reflection coordinates into the sound record
					manager->m_sQueueSample.m_vecPos = manager->m_avecReflectionsPos[i];
					AudioRequests_Submit(manager);
				}
			}
		}
	}
	// Copy the game position into the C sound record
	manager->m_sQueueSample.m_vecPos = oldPos;
	manager->m_sQueueSample.m_fDistance = oldDist;
}

#endif
//- rouz edit (ChatGPT)
