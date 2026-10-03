//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "sampman.h"
#include <string.h>

#define AudioActive_Min(a,b) ((a) < (b) ? (a) : (b))
#define AudioActive_Max(a,b) ((a) > (b) ? (a) : (b))
#define AudioActive_Clamp(v,center,radius) ((v) > (center) ? AudioActive_Min(v, center + radius) : AudioActive_Max(v, center - radius))

void
AudioActive_Process(cAudioManager *manager)
{
	// Preserve channel matching, updates, startup and backend callback order
	uint8_t flag;
	float position2;
	float position1;

	uint32_t samplesPerFrame;
	uint32_t samplesToPlay;

#ifdef EXTERNAL_3D_SOUND
	float x;
	float usedX;
	float usedY;
	float usedZ;
#endif

	uint8_t vol;
	uint8_t emittingVol;
	AudioSoundPosition position;

	uint8_t isPhoneCall;
	uint8_t channelOffset = 0;

#ifdef EXTERNAL_3D_SOUND
	#define WORKING_VOLUME_FIELD m_nEmittingVolume
#else
	#define WORKING_VOLUME_FIELD m_nVolume
#endif 

#ifdef USE_TIME_SCALE_FOR_AUDIO
	float timeScale = manager->m_bIsPaused ? 1.0f : AudioPoliceHost_TimeScale();
#endif

	for (uint8_t i = 0; i < manager->m_nActiveSamples; i++) {
		manager->m_aRequestedQueue[manager->m_nActiveQueue][i].m_bIsBeingPlayed = 0;
		manager->m_asActiveSamples[i].m_bIsBeingPlayed = 0;
	}

	for (uint8_t i = 0; i < manager->m_nRequestedCount[manager->m_nActiveQueue]; i++) {
		tSound *sample = &manager->m_aRequestedQueue[manager->m_nActiveQueue][manager->m_aRequestedOrderList[manager->m_nActiveQueue][i]];
		if (sample->m_nSampleIndex != NO_SAMPLE) {
			for (uint8_t j = 0; j < manager->m_nActiveSamples; j++) {
				if (sample->m_nEntityIndex == manager->m_asActiveSamples[j].m_nEntityIndex && sample->m_nCounter == manager->m_asActiveSamples[j].m_nCounter &&
					sample->m_nSampleIndex == manager->m_asActiveSamples[j].m_nSampleIndex) {
					if (sample->m_nLoopCount > 0) {
						if (manager->m_FrameCounter & 1)
							flag = !!(j & 1);
						else
							flag = !(j & 1);

						if (flag && !SampleManager_GetChannelUsedFlag(&SampleManager, j)) {
							sample->m_bIsPlayingFinished = 1;
							manager->m_asActiveSamples[j].m_bIsPlayingFinished = 1;
							manager->m_asActiveSamples[j].m_nSampleIndex = NO_SAMPLE;
							manager->m_asActiveSamples[j].m_nEntityIndex = AEHANDLE_NONE;
							continue;
						}
						if (sample->m_nFramesToPlay == 0)
							sample->m_nFramesToPlay = 1;
					}
					sample->m_bIsBeingPlayed = 1;
					manager->m_asActiveSamples[j].m_bIsBeingPlayed = 1;
					sample->m_nVolumeChange = -1;
					if (!sample->m_bStatic) {
						if (sample->m_bIs2D) {
							emittingVol = manager->m_bDoubleVolume ? 2 * AudioActive_Min(63, sample->WORKING_VOLUME_FIELD) : sample->WORKING_VOLUME_FIELD;
#ifdef USE_TIME_SCALE_FOR_AUDIO
							SampleManager_SetChannelFrequency(&SampleManager, j, sample->m_nFrequency * timeScale);
#else
							SampleManager_SetChannelFrequency(&SampleManager, j, sample->m_nFrequency);
#endif
#ifdef EXTERNAL_3D_SOUND
							SampleManager_SetChannelEmittingVolume(&SampleManager, j, emittingVol);
#else
							SampleManager_SetChannelPan(&SampleManager, j, sample->m_nPan);
							SampleManager_SetChannelVolume(&SampleManager, j, emittingVol);
#endif
						} else {
							position2 = sample->m_fDistance;
							position1 = manager->m_asActiveSamples[j].m_fDistance;
							manager->m_asActiveSamples[j].m_fDistance = sample->m_fDistance;
							sample->m_nFrequency = AudioMath_ComputeDopplerFrequency(AudioActiveHost_CameraSwitched(), manager->m_nTimeSpent, manager->m_fSpeedOfSound, sample->m_nFrequency, position1, position2, sample->m_fSpeedMultiplier);
							if (sample->m_nFrequency != manager->m_asActiveSamples[j].m_nFrequency) {
								uint32_t freq = AudioActive_Clamp((int32_t)sample->m_nFrequency, (int32_t)manager->m_asActiveSamples[j].m_nFrequency, 6000);
								manager->m_asActiveSamples[j].m_nFrequency = freq;
#ifdef USE_TIME_SCALE_FOR_AUDIO
								SampleManager_SetChannelFrequency(&SampleManager, j, freq * timeScale);
#else
								SampleManager_SetChannelFrequency(&SampleManager, j, freq);
#endif
							}
#ifdef EXTERNAL_3D_SOUND
							if (sample->m_nEmittingVolume != manager->m_asActiveSamples[j].m_nEmittingVolume) {
								vol = AudioActive_Clamp((int8_t)sample->m_nEmittingVolume, (int8_t)manager->m_asActiveSamples[j].m_nEmittingVolume, 10);
#else
							if (sample->m_nVolume != manager->m_asActiveSamples[j].m_nVolume) {
								vol = AudioActive_Clamp((int8_t)sample->m_nVolume, (int8_t)manager->m_asActiveSamples[j].m_nVolume, 10);
#endif
								emittingVol = manager->m_bDoubleVolume ? 2 * AudioActive_Min(63, vol) : vol;

								isPhoneCall = 0;
								for (int32_t k = 0; k < MISSION_AUDIO_SLOTS; k++) {
									if (manager->m_bIsMissionAudioPhoneCall[k]) {
										isPhoneCall = 1;
										break;
									}
								}
								if (isPhoneCall) {
									emittingVol = (emittingVol * manager->m_nGlobalSfxVolumeMultiplier) / 127;
								} else {
									if (manager->m_nGlobalSfxVolumeMultiplier < 127)
										emittingVol = (emittingVol * manager->m_nGlobalSfxVolumeMultiplier) / 127;
								}

#ifdef EXTERNAL_3D_SOUND
								SampleManager_SetChannelEmittingVolume(&SampleManager, j, emittingVol);
								manager->m_asActiveSamples[j].m_nEmittingVolume = vol;
#else
								SampleManager_SetChannelVolume(&SampleManager, j, emittingVol);
								manager->m_asActiveSamples[j].m_nVolume = vol;
#endif
							}
							// Translate a copied C sound position through the existing camera matrix
							const AudioSoundPosition soundPosition = sample->m_vecPos;
							AudioGeometry_Translate(&soundPosition, &position);
#ifdef EXTERNAL_3D_SOUND
							SampleManager_SetChannel3DPosition(&SampleManager, j, position.x, position.y, position.z);
							SampleManager_SetChannel3DDistances(&SampleManager, j, sample->m_MaxDistance, 0.25f * sample->m_MaxDistance);
#else
							sample->m_nPan = AudioMath_ComputePan(sample->m_fDistance, position.x);
							SampleManager_SetChannelPan(&SampleManager, j, sample->m_nPan);
#endif
						}
#if !defined(GTA_PS2) || defined(AUDIO_REVERB)
						SampleManager_SetChannelReverbFlag(&SampleManager, j, sample->m_bReverb);
#endif
						break; //continue for i
					}
					sample->m_bIsBeingPlayed = 0;
					manager->m_asActiveSamples[j].m_bIsBeingPlayed = 0;
					//continue for j
				}
			}
		}
	}
	for (uint8_t i = 0; i < manager->m_nActiveSamples; i++) {
		if (manager->m_asActiveSamples[i].m_nSampleIndex != NO_SAMPLE && !manager->m_asActiveSamples[i].m_bIsBeingPlayed) {
			SampleManager_StopChannel(&SampleManager, i);
			manager->m_asActiveSamples[i].m_nSampleIndex = NO_SAMPLE;
			manager->m_asActiveSamples[i].m_nEntityIndex = AEHANDLE_NONE;
		}
	}
	for (uint8_t i = 0; i < manager->m_nRequestedCount[manager->m_nActiveQueue]; i++) {
		tSound *sample = &manager->m_aRequestedQueue[manager->m_nActiveQueue][manager->m_aRequestedOrderList[manager->m_nActiveQueue][i]];
		if (!sample->m_bIsBeingPlayed && !sample->m_bIsPlayingFinished && manager->m_asAudioEntities[sample->m_nEntityIndex].m_bIsUsed && sample->m_nSampleIndex < NO_SAMPLE) {
#ifdef AUDIO_REFLECTIONS
			if (sample->m_nCounter > 255 && sample->m_nLoopCount > 0 && sample->m_nReflectionDelay > 0) { // check if reflection
				sample->m_nReflectionDelay--;
				sample->m_nFramesToPlay = 1;
			} else
#endif
			{
				for (uint8_t j = 0; j < manager->m_nActiveSamples; j++) {
					uint8_t k = (j + manager->m_nChannelOffset) % manager->m_nActiveSamples;
					if (!manager->m_asActiveSamples[k].m_bIsBeingPlayed) {
						if (sample->m_nLoopCount > 0) {
							samplesPerFrame = sample->m_nFrequency / manager->m_nTimeSpent;
							samplesToPlay = sample->m_nLoopCount * SampleManager_GetSampleLength(&SampleManager, sample->m_nSampleIndex);
							if (samplesPerFrame == 0)
								continue;
							sample->m_nFramesToPlay = samplesToPlay / samplesPerFrame + 1;
						}
						memcpy(&manager->m_asActiveSamples[k], sample, sizeof(tSound));
						if (!manager->m_asActiveSamples[k].m_bIs2D) {
							// Translate a copied C sound position through the existing camera matrix
							const AudioSoundPosition soundPosition = manager->m_asActiveSamples[k].m_vecPos;
							AudioGeometry_Translate(&soundPosition, &position);
#ifndef EXTERNAL_3D_SOUND
							manager->m_asActiveSamples[j].m_nPan = AudioMath_ComputePan(manager->m_asActiveSamples[j].m_fDistance, position.x);
#endif
						}
						emittingVol = manager->m_bDoubleVolume ? 2 * AudioActive_Min(63, manager->m_asActiveSamples[j].WORKING_VOLUME_FIELD) : manager->m_asActiveSamples[j].WORKING_VOLUME_FIELD;
#ifdef GTA_PS2
						{
							SampleManager_InitialiseChannel(&SampleManager, k, manager->m_asActiveSamples[k].m_nSampleIndex, manager->m_asActiveSamples[k].m_nBankIndex);
#else
						if (SampleManager_InitialiseChannel(&SampleManager, k, manager->m_asActiveSamples[k].m_nSampleIndex, manager->m_asActiveSamples[k].m_nBankIndex)) {
#endif
#ifdef USE_TIME_SCALE_FOR_AUDIO
							SampleManager_SetChannelFrequency(&SampleManager, k, manager->m_asActiveSamples[k].m_nFrequency * timeScale);
#else
							SampleManager_SetChannelFrequency(&SampleManager, k, manager->m_asActiveSamples[k].m_nFrequency);
#endif
							isPhoneCall = 0;
							for (int32_t l = 0; l < MISSION_AUDIO_SLOTS; l++) {
								if (manager->m_bIsMissionAudioPhoneCall[l]) {
									isPhoneCall = 1;
									break;
								}
							}
							if (!isPhoneCall || manager->m_asActiveSamples[k].m_bIs2D) {
								if (manager->m_nGlobalSfxVolumeMultiplier < 127)
									emittingVol = (emittingVol * manager->m_nGlobalSfxVolumeMultiplier) / 127;
								vol = emittingVol;
							} else {
								vol = (emittingVol * manager->m_nGlobalSfxVolumeMultiplier) / 127;
							}
#ifdef EXTERNAL_3D_SOUND
							SampleManager_SetChannelEmittingVolume(&SampleManager, k, vol);
#else
							SampleManager_SetChannelVolume(&SampleManager, j, emittingVol);
							SampleManager_SetChannelPan(&SampleManager, j, manager->m_asActiveSamples[j].m_nPan);
#endif
#ifndef GTA_PS2
							SampleManager_SetChannelLoopPoints(&SampleManager, k, manager->m_asActiveSamples[k].m_nLoopStart, manager->m_asActiveSamples[k].m_nLoopEnd);
#endif
							SampleManager_SetChannelLoopCount(&SampleManager, k, manager->m_asActiveSamples[k].m_nLoopCount);
#if !defined(GTA_PS2) || defined(AUDIO_REVERB)
							SampleManager_SetChannelReverbFlag(&SampleManager, k, manager->m_asActiveSamples[k].m_bReverb);
#endif
#ifdef EXTERNAL_3D_SOUND
							if (manager->m_asActiveSamples[k].m_bIs2D) {
								uint8_t offset = manager->m_asActiveSamples[k].m_nPan;
								if (offset == 63)
									x = 0.0f;
								else if (offset >= 63)
									x = (offset - 63) * 1000.0f / 63;
								else
									x = -(63 - offset) * 1000.0f / 63; //same like line below
								usedX = x;
								usedY = 0.0f;
								usedZ = 0.0f;
								manager->m_asActiveSamples[k].m_MaxDistance = 100000.0f;
							} else {
								usedX = position.x;
								usedY = position.y;
								usedZ = position.z;
							}
							SampleManager_SetChannel3DPosition(&SampleManager, k, usedX, usedY, usedZ);
							SampleManager_SetChannel3DDistances(&SampleManager, k, manager->m_asActiveSamples[k].m_MaxDistance, 0.25f * manager->m_asActiveSamples[k].m_MaxDistance);
#endif
							SampleManager_StartChannel(&SampleManager, k);
						}
						manager->m_asActiveSamples[k].m_bIsBeingPlayed = 1;
						channelOffset++;
						sample->m_bIsBeingPlayed = 1;
						sample->m_nVolumeChange = -1;
						break;
					}
				}
			}
		}
	}

#ifdef GTA_PS2
	manager->m_nChannelOffset += channelOffset;
#endif
	manager->m_nChannelOffset %= manager->m_nActiveSamples;

#ifdef USE_TIME_SCALE_FOR_AUDIO
	for (uint8_t i = 0; i < manager->m_nActiveSamples; i++) {
		if (manager->m_asActiveSamples[i].m_nSampleIndex != NO_SAMPLE && manager->m_asActiveSamples[i].m_bIsBeingPlayed)
			SampleManager_SetChannelFrequency(&SampleManager, i, manager->m_asActiveSamples[i].m_nFrequency * timeScale);
	}
#endif

#undef WORKING_VOLUME_FIELD
}

#undef AudioActive_Clamp
#undef AudioActive_Max
#undef AudioActive_Min
//- rouz edit (ChatGPT)
