//+ rouz edit (ChatGPT)
#include "Timer.h"
#include "AudioManager.h"
#include "AudioMath.h"
#include "AudioSamples.h"
#include "soundlist.h"
#include "sampman.h"

#define TRUE 1
#define FALSE 0
enum { PED_COMMENT_MAX_DIST = 40, PED_COMMENT_POLICE_HELI_MAX_DIST = 400 };

void
AudioPedPlayback_Debug(cAudioManager *manager, int32_t sound)
{
	// Prepare the original automatic comment record for the explicit owner
	tPedComment pedComment;

	pedComment.m_nSampleIndex = sound;
	pedComment.m_nLoadingTimeout = 10;
	pedComment.m_nEntityIndex = 0;
	pedComment.m_fDistance = 0.0f;
	pedComment.m_nVolume = 99;
#if defined(EXTERNAL_3D_SOUND) && defined(FIX_BUGS)
	pedComment.m_nEmittingVolume = 99;
#endif

	// Copy the game position into the C comment record
	AudioPedPlaybackHost_DebugPosition(&pedComment.m_vecPos);

	// Insert the debug comment with the original volume and timeout
	PedComments_Add(&manager->m_sPedComments, &pedComment);
}

void
PedComments_Process(cPedComments *comments)
{
	// Retain the original shared history across all comment queues
	uint32_t sampleIndex;
	uint8_t bIsPlayerComment;
	static uint8_t counter = 0;
	static uint32_t prevSamples[10] = { NO_SAMPLE, NO_SAMPLE, NO_SAMPLE, NO_SAMPLE, NO_SAMPLE, NO_SAMPLE, NO_SAMPLE, NO_SAMPLE, NO_SAMPLE, NO_SAMPLE };

	// Leave paused queues and shared playback history untouched
	if (AudioManager.m_bIsPaused) return;

	// Reject repeats before asking the original sample backend to load
	if (comments->m_nPedCommentCount[comments->m_nActiveQueue]) {
		for(int i = 0; i < (sizeof(prevSamples) / sizeof(prevSamples[0])); i++) {
			if(comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_nSampleIndex ==
			   prevSamples[(counter + 1 + i) % (sizeof(prevSamples) / sizeof(prevSamples[0]))]) {
				comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_nLoadingTimeout = -1;
				goto PedCommentAlreadyAdded;
			}
		}
#if defined(GTA_PS2) || defined(FIX_BUGS)
		// Capture the selected ID before backend queries can change live queue fields
		uint8_t IsLoadedResult;
		sampleIndex = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_nSampleIndex;
		if (sampleIndex >= PLAYER_COMMENTS_START && sampleIndex <= PLAYER_COMMENTS_END) {
			IsLoadedResult = SampleManager_IsMissionAudioLoaded(&SampleManager, MISSION_AUDIO_PLAYER_COMMENT, sampleIndex);
			bIsPlayerComment = TRUE;
		} else {
			IsLoadedResult = SampleManager_IsPedCommentLoaded(&SampleManager, sampleIndex);
			bIsPlayerComment = FALSE;
		}
		switch(IsLoadedResult) {
#else
		// Preserve the original optional PC branch with its unspecified sample ID
		switch(SampleManager_IsPedCommentLoaded(&SampleManager, sampleIndex)) {
#endif
		// Start loading through the same mission or pedestrian backend
		case LOADING_STATUS_NOT_LOADED:
#if defined(GTA_PC) && !defined(FIX_BUGS)
			if(!comments->m_bDelay)
#endif
#if defined(GTA_PS2) || defined(FIX_BUGS)
				if (bIsPlayerComment)
					SampleManager_LoadMissionAudio(&SampleManager, MISSION_AUDIO_PLAYER_COMMENT, sampleIndex);
				else
#endif
					SampleManager_LoadPedComment(&SampleManager, sampleIndex);
			break;
		// Populate the global scratch sound from the live highest-priority comment
		case LOADING_STATUS_LOADED:
			AudioManager.m_sQueueSample.m_nEntityIndex = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_nEntityIndex;
			AudioManager.m_sQueueSample.m_nCounter = 0;
			AudioManager.m_sQueueSample.m_nSampleIndex = sampleIndex;
			AudioManager.m_sQueueSample.m_nBankIndex = SFX_BANK_PED_COMMENTS;
			AudioManager.m_sQueueSample.m_nPriority = 3;
			AudioManager.m_sQueueSample.m_nVolume = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_nVolume;
			AudioManager.m_sQueueSample.m_fDistance = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_fDistance;
			AudioManager.m_sQueueSample.m_nLoopCount = 1;
#ifndef GTA_PS2
			AudioManager.m_sQueueSample.m_nLoopStart = 0;
			AudioManager.m_sQueueSample.m_nLoopEnd = -1;
#endif
#ifdef EXTERNAL_3D_SOUND
	#ifdef FIX_BUGS
			AudioManager.m_sQueueSample.m_nEmittingVolume = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_nEmittingVolume;
	#else
			AudioManager.m_sQueueSample.m_nEmittingVolume = MAX_VOLUME;
	#endif // FIX_BUGS
#endif // EXTERNAL_3D_SOUND
#ifdef ATTACH_RELEASING_SOUNDS_TO_ENTITIES
			// let's disable doppler because if sounds funny as the sound moves
			// originally position of ped comment doesn't change so this has no effect anyway
			AudioManager.m_sQueueSample.m_fSpeedMultiplier = 0.0f;
#else
			AudioManager.m_sQueueSample.m_fSpeedMultiplier = 3.0f;
#endif
			AudioManager.m_sQueueSample.m_MaxDistance = PED_COMMENT_MAX_DIST;
			AudioManager.m_sQueueSample.m_bStatic = TRUE;
			// Copy the C position components into the game sound vector
			AudioManager.m_sQueueSample.m_vecPos.x = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_vecPos.x;
			AudioManager.m_sQueueSample.m_vecPos.y = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_vecPos.y;
			AudioManager.m_sQueueSample.m_vecPos.z = comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_vecPos.z;
#ifdef AUDIO_REVERB
			AudioManager.m_sQueueSample.m_bReverb = TRUE;
#endif // AUDIO_REVERB
#ifdef AUDIO_REFLECTIONS
			AudioManager.m_sQueueSample.m_bReflections = TRUE;
#endif // AUDIO_REFLECTIONS
			AudioManager.m_sQueueSample.m_bIs2D = FALSE;
#ifdef FIX_BUGS
			if((sampleIndex >= SFX_POLICE_BOAT_1 && sampleIndex <= SFX_POLICE_BOAT_23) ||
				(sampleIndex >= SFX_POLICE_HELI_1 && sampleIndex <= SFX_POLICE_HELI_20))
				AudioManager.m_sQueueSample.m_MaxDistance = PED_COMMENT_POLICE_HELI_MAX_DIST;
	#ifndef ATTACH_RELEASING_SOUNDS_TO_ENTITIES
			else if (sampleIndex >= PLAYER_COMMENTS_START && sampleIndex <= PLAYER_COMMENTS_END) { // check if player sfx
				AudioManager.m_sQueueSample.m_bIs2D = TRUE;
				AudioManager.m_sQueueSample.m_nPan = 63;
			}
	#endif // ATTACH_RELEASING_SOUNDS_TO_ENTITIES
#endif // FIX_BUGS
			// Preserve base frequency and random-displacement evaluation at submission
			AudioManager.m_sQueueSample.m_nFrequency =
				SampleManager_GetSampleBaseFrequency(&SampleManager, AudioManager.m_sQueueSample.m_nSampleIndex) + AudioMath_RandomDisplacement(AudioManager.m_anRandomTable, 750);
#ifndef USE_TIME_SCALE_FOR_AUDIO
			if (Timer_GetIsSlowMotionActive())
				AudioManager.m_sQueueSample.m_nFrequency >>= 1;
#endif
			comments->m_aPedCommentQueue[comments->m_nActiveQueue][comments->m_aPedCommentOrderList[comments->m_nActiveQueue][0]].m_nLoadingTimeout = -1;
			// Record the captured sample before submitting through the global owner
			prevSamples[counter++] = sampleIndex;
			if(counter == 10) counter = 0;
			AudioRequests_Submit(&AudioManager);
#if defined(GTA_PC) && !defined(FIX_BUGS)
			comments->m_nDelayTimer = AudioPlayerEngineHost_Time();
			comments->m_bDelay = TRUE;
#endif
			break;
		case LOADING_STATUS_LOADING: break;
		default: break;
		}
	}

PedCommentAlreadyAdded:
	// Advance pending comments through the C queue implementation
	PedComments_Advance(comments);
#if defined(GTA_PC) && !defined(FIX_BUGS)
	if(comments->m_bDelay)
		if(AudioPlayerEngineHost_Time() - comments->m_nDelayTimer > 6000) comments->m_bDelay = FALSE;
#endif
}
//- rouz edit (ChatGPT)
