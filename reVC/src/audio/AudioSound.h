#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "../core/config.h"

typedef struct AudioSoundPosition {
    float x, y, z;
} AudioSoundPosition;

typedef struct tSound {
	int32_t m_nEntityIndex;		// audio entity index
	uint32_t m_nCounter;			// I'm not sure what this is but it looks like a virtual counter to determine the same sound in queue
								// Values higher than 255 are used by reflections
	uint32_t m_nSampleIndex;		// An index of sample from AudioSamples.h
	uint8_t m_nBankIndex;			// A sound bank index. IDK what's the point of it here since samples are hardcoded anyway
	uint8_t m_bIs2D;				// If TRUE then sound is played in 2D space (such as frontend or police radio)
	uint32_t m_nPriority;			// The multiplier for the sound priority (see m_nFinalPriority below). Lesser value means higher priority
	uint32_t m_nFrequency;		// Sound frequency, plain and simple
	uint8_t m_nVolume;			// Sound volume (0..127), only used as an actual volume without EXTERNAL_3D_SOUND (see m_nEmittingVolume)
	float m_fDistance;			// Distance to camera (useless if m_bIs2D == TRUE)
	uint32_t m_nLoopCount;		// 0 - always loop, 1 - don't loop, other values never seen
#ifndef GTA_PS2
	// Loop offsets
	uint32_t m_nLoopStart;
	int32_t m_nLoopEnd;
#endif
#ifdef EXTERNAL_3D_SOUND
	uint8_t m_nEmittingVolume;	// The volume in 3D space, provided to 3D audio engine
#endif
	float m_fSpeedMultiplier;	// Used for doppler effect. 0.0f - unaffected by doppler
	float m_MaxDistance;		// The maximum distance at which sound could be heard. Minimum distance = MaxDistance / 5 or MaxDistance / 4 in case of emitting volume (useless if m_bIs2D == TRUE)
	uint8_t m_bStatic;			// If TRUE then sound parameters cannot be changed during playback (frequency, position, etc.)
	AudioSoundPosition m_vecPos;			// Position of sound in 3D space. Unused if m_bIs2D == TRUE
#if !defined(GTA_PS2) || defined(AUDIO_REVERB) // GTA_PS2 because this field exists on mobile but not on PS2
	uint8_t m_bReverb;			// Toggles reverb effect
#endif
#ifdef AUDIO_REFLECTIONS
	uint8_t m_nReflectionDelay;	// Number of frames before reflection could be played. This is calculated internally by AudioManager and shouldn't be set by queued sample
	uint8_t m_bReflections;		// Add sound reflections
#endif
	uint8_t m_nPan;				// Sound panning (0-127). Controls the volume of the playback coming from left and right speaker. Calculated internally unless m_bIs2D==TRUE.
								// 0 =   L 100%  R 0%
								// 63 =  L 100%  R 100%
								// 127 = L 0%    R 100%
	uint8_t m_nFrontRearPan;		// Used on PS2 for surround panning
#ifndef FIX_BUGS
	uint32_t m_nFramesToPlay;		// Number of frames the sound would be played (if it stops being queued).
								// This one is being set by queued sample for looping sounds, otherwise calculated inside AudioManager
#else
	float m_nFramesToPlay;		// Made into float for high fps fix
#endif

	// all fields below are internal to AudioManager calculations and aren't set by queued sample
	uint8_t m_bIsBeingPlayed;		// Set to TRUE when the sound was added or changed on current frame to avoid it being overwritten
	uint8_t m_bIsPlayingFinished;	// Not sure about the name. Set to TRUE when sampman channel becomes free
	uint32_t m_nFinalPriority;	// Actual value used to compare priority, calculated using volume and m_nPriority. Lesser value means higher priority
	int8_t m_nVolumeChange;		// How much m_nVolume should reduce per each frame.
#if defined(FIX_BUGS) && defined(EXTERNAL_3D_SOUND)
	int8_t m_nEmittingVolumeChange; // same as above but for m_nEmittingVolume
#endif
} tSound;


#ifdef __cplusplus
extern "C" {
#endif
void AudioSoundQueue_ClearRequested(uint8_t *order, uint8_t *count, uint8_t activeSamples);
void AudioSoundQueue_ClearActive(tSound *active, uint8_t activeSamples);
#ifdef EXTERNAL_3D_SOUND
void AudioSoundQueue_AdjustVolume(tSound *requested, const uint8_t *order, uint8_t count);
#endif
void AudioSoundQueue_InsertOrder(tSound *requested, uint8_t *order, uint8_t activeSamples, uint8_t sample);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
