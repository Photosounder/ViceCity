//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioSamples.h"
#include "sampman.h"

#ifdef GTA_TRAIN
enum {
	TRAIN_NOISE_FAR_MAX_DIST = 300,
	TRAIN_NOISE_NEAR_MAX_DIST = 70,
	TRAIN_NOISE_VOLUME = 75,
};
#ifndef GTA_PS2
#define AUDIO_TRAIN_RESET_LOOP_OFFSETS \
	manager->m_sQueueSample.m_nLoopStart = 0; \
	manager->m_sQueueSample.m_nLoopEnd = -1;
#define AUDIO_TRAIN_SET_LOOP_OFFSETS(sample) \
	manager->m_sQueueSample.m_nLoopStart = SampleManager_GetSampleLoopStartOffset(&SampleManager, sample); \
	manager->m_sQueueSample.m_nLoopEnd = SampleManager_GetSampleLoopEndOffset(&SampleManager, sample);
#else
#define AUDIO_TRAIN_RESET_LOOP_OFFSETS
#define AUDIO_TRAIN_SET_LOOP_OFFSETS(sample)
#endif
#ifdef EXTERNAL_3D_SOUND
#define AUDIO_TRAIN_SET_EMITTING_VOLUME(vol) manager->m_sQueueSample.m_nEmittingVolume = vol
#else
#define AUDIO_TRAIN_SET_EMITTING_VOLUME(vol)
#endif
#ifdef AUDIO_REFLECTIONS
#define AUDIO_TRAIN_SET_SOUND_REFLECTION(b) manager->m_sQueueSample.m_bReflections = b
#else
#define AUDIO_TRAIN_SET_SOUND_REFLECTION(b)
#endif
#ifdef AUDIO_REVERB
#define AUDIO_TRAIN_SET_SOUND_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_TRAIN_SET_SOUND_REVERB(b)
#endif


static uint8_t AudioTrain_Less(float a, float b)
{
    // Retain the selected repeated speed query after the original comparison
    volatile uint8_t result = a < b;
    return result;
}
#define AUDIO_TRAIN_MIN(a,b) (AudioTrain_Less((a),(b)) ? (a) : (b))

uint8_t
AudioTrain_Process(cAudioManager *manager, cVehicleParams *params)
{
	// Retain event-local storage without changing any owner field
	CVehicle *train;
	uint8_t Vol;
	float speedMultipler;

	// Preserve the far range return gate independently of motion and attenuation
	if (params->m_fDistance < (TRAIN_NOISE_FAR_MAX_DIST * TRAIN_NOISE_FAR_MAX_DIST)){
		// Capture and query the train only after the original motion gate
		if (params->m_fVelocityChange > 0.0f) {
			AudioMath_CalculateDistance(&params->m_bDistanceCalculated, &manager->m_sQueueSample.m_fDistance, params->m_fDistance);
			train = params->m_pVehicle;
			speedMultipler = AUDIO_TRAIN_MIN(1.0f, AudioTrainHost_Speed(train) * 250.0f / 51.0f);
			Vol = (TRAIN_NOISE_VOLUME * speedMultipler);
			// Generate the far layer only for a zero-position wagon
			if (AudioTrainHost_WagonPosition(train) == 0.0f) {
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, TRAIN_NOISE_FAR_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = 32;
					manager->m_sQueueSample.m_nSampleIndex = SFX_TRAIN_FAR;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 2;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_TRAIN_FAR);
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_TRAIN_SET_EMITTING_VOLUME(Vol);
					AUDIO_TRAIN_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 3.0f;
					manager->m_sQueueSample.m_MaxDistance = TRAIN_NOISE_FAR_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 3;
					AUDIO_TRAIN_SET_SOUND_REVERB(1);
					AUDIO_TRAIN_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
			// Recheck the live near distance after the far request and its callbacks
			if (params->m_fDistance < (TRAIN_NOISE_NEAR_MAX_DIST * TRAIN_NOISE_NEAR_MAX_DIST)) {
				manager->m_sQueueSample.m_nVolume = AudioMath_ComputeVolume(Vol, TRAIN_NOISE_NEAR_MAX_DIST, manager->m_sQueueSample.m_fDistance);
				if (manager->m_sQueueSample.m_nVolume > 0) {
					manager->m_sQueueSample.m_nCounter = 33;
					manager->m_sQueueSample.m_nSampleIndex = SFX_TRAIN_NEAR;
					manager->m_sQueueSample.m_nBankIndex = SFX_BANK_0;
					manager->m_sQueueSample.m_bIs2D = 0;
					manager->m_sQueueSample.m_nPriority = 5;
					manager->m_sQueueSample.m_nFrequency = SampleManager_GetSampleBaseFrequency(&SampleManager, SFX_TRAIN_NEAR) + 100 * manager->m_sQueueSample.m_nEntityIndex % 987;
					manager->m_sQueueSample.m_nLoopCount = 0;
					AUDIO_TRAIN_SET_EMITTING_VOLUME(Vol);
					AUDIO_TRAIN_SET_LOOP_OFFSETS(manager->m_sQueueSample.m_nSampleIndex)
					manager->m_sQueueSample.m_fSpeedMultiplier = 6.0f;
					manager->m_sQueueSample.m_MaxDistance = TRAIN_NOISE_NEAR_MAX_DIST;
					manager->m_sQueueSample.m_bStatic = 0;
					manager->m_sQueueSample.m_nFramesToPlay = 3;
					AUDIO_TRAIN_SET_SOUND_REVERB(1);
					AUDIO_TRAIN_SET_SOUND_REFLECTION(0);
					AudioRequests_Submit(manager);
				}
			}
		}
		return 1;
	}
	return 0;
}
#endif
//- rouz edit (ChatGPT)
