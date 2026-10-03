//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "../../core/config.h"
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif
#include "AudioStreamHost.h"

typedef uint32_t uint32;
typedef uint8_t uint8;
typedef int32_t int32;
typedef int8_t int8;
#define nil NULL
#define ASSERT assert
#define DEV(...) ((void)0)
#define MAX_VOLUME 127
#define Sqrt sqrtf
#define SQR(value) ((value) * (value))
#define Clamp(value, low, high) ((value) < (low) ? (low) : (value) > (high) ? (high) : (value))

#ifdef AUDIO_OAL

#if defined _MSC_VER && !defined CMAKE_NO_AUTOLINK
#ifdef AUDIO_OAL_USE_SNDFILE
#pragma comment( lib, "libsndfile-1.lib" )
#endif
#ifdef AUDIO_OAL_USE_MPG123
#pragma comment( lib, "libmpg123-0.lib" )
#endif
#endif
#ifdef AUDIO_OAL_USE_SNDFILE
#include <sndfile.h>
#endif
#ifdef AUDIO_OAL_USE_MPG123
#include <mpg123.h>
#endif
#ifdef AUDIO_OAL_USE_OPUS
#include <opusfile.h>
#endif

// Removed the global STL queues and pair dependency // rouz edit (ChatGPT)

#ifdef MULTITHREADED_AUDIO
#include "AudioThread.h"
#include "stream.h"

AudioThread gAudioThread;
AudioMutex gAudioThreadQueueMutex;
AudioWakeSignal gAudioThreadCv;
bool gAudioThreadTerm = false;
#include "AudioJobQueue.h"
AudioProcessQueue gStreamsToProcess = {nil, nil};
AudioCloseQueue gStreamsToClose = {nil, nil};
#else
#include "stream.h"
#endif



/*
As we ran onto an issue of having different volume levels for mono streams
and stereo streams we are now handling all the stereo panning ourselves.
Each stream now has two sources - one panned to the left and one to the right,
and uses two separate buffers to store data for each individual channel.
For that we also have to reshuffle all decoded PCM stereo data from LRLRLRLR to
LLLLRRRR (handled by CSortStereoBuffer).
*/

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
#define AUDIO_FORMAT_DEBUG(format, ...) re3_debug("[DBG]: " format, __VA_ARGS__)
#include "AudioFormats.h"
#undef AUDIO_FORMAT_DEBUG
// For multi-thread: Someone always acquire stream's mutex before entering here
void
CStream_BuffersShouldBeFilled(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

#ifdef MULTITHREADED_AUDIO
	if (!AudioStream_IsCutscene()) {
		// Discard earlier fill requests before reseeding the stream-owned pairs
		AudioBufferQueue_Clear(&stream->m_fillBuffers);
		for(int i = 0; i < NUM_STREAMBUFFERS / 2; i++) {
			// Enqueue each stream-owned left and right pair exactly once
			AudioBufferPair pair = {stream->m_alBuffers[i * 2], stream->m_alBuffers[i * 2 + 1]};
			bool pushed = AudioBufferQueue_Push(&stream->m_fillBuffers, pair);
			ASSERT(pushed);
		}

		CStream_FlagAsToBeProcessed(stream, false);

		stream->m_bActive = true; // to allow CStream_Update(stream) to queue the filled buffers & play
		return;
	}
	// Discard asynchronous fill requests before synchronous cutscene playback
	AudioBufferQueue_Clear(&stream->m_fillBuffers);
#endif
	if ( CStream_FillBuffers(stream) != 0 )
	{
		CStream_SetPlay(stream, true);
	}
}

// returns whether it's queued (not on multi-thread)
bool
CStream_BufferShouldBeFilledAndQueued(CStream *stream, AudioBufferPair* bufs)
{
    // Access the caller-owned stream through explicit C state

#ifdef MULTITHREADED_AUDIO
	// Return processed pairs to the fill queue under the stream lock
	if (!AudioStream_IsCutscene()) {
		// Append the processed pair before waking the decoder worker
		bool pushed = AudioBufferQueue_Push(&stream->m_fillBuffers, *bufs);
		ASSERT(pushed);
	}
	else
#endif
	{
		// Convert the pair fields to the OpenAL channel buffer array
		ALuint alBuffers[2] = {bufs->left, bufs->right}; // left - right
		if (CStream_FillBuffer(stream, alBuffers)) {
			alSourceQueueBuffers(stream->m_pAlSources[0], 1, &alBuffers[0]);
			alSourceQueueBuffers(stream->m_pAlSources[1], 1, &alBuffers[1]);
			return true;
		}
	}
	return false;
}

#ifdef MULTITHREADED_AUDIO
void
CStream_FlagAsToBeProcessed(CStream *stream, bool close)
{
    // Access the caller-owned stream through explicit C state

    // Keep cutscene decoding synchronous while allowing deferred cleanup
    if(!close && AudioStream_IsCutscene())
        return;

    // Allocate each unbounded request at its owning scheduling call site
    AudioCloseJob *closeJob = nil;
    AudioProcessJob *processJob = nil;
    if(close) {
        closeJob = (AudioCloseJob*)malloc(sizeof(*closeJob));
        if(!closeJob) abort();
    } else {
        processJob = (AudioProcessJob*)malloc(sizeof(*processJob));
        if(!processJob) abort();
    }

    // Capture and publish the same values under the existing scheduler lock
    // Manage native synchronization with an explicit lifetime
    AudioMutex_Lock(&gAudioThreadQueueMutex);
    if(close) {
        closeJob->decoder = stream->m_pSoundFile;
        closeJob->buffer = stream->m_pBuffer;
        AudioCloseQueue_Push(&gStreamsToClose, closeJob);
    } else {
        processJob->stream = stream;
        AudioProcessQueue_Push(&gStreamsToProcess, processJob);
    }
    // Manage native synchronization with an explicit lifetime
    AudioMutex_Unlock(&gAudioThreadQueueMutex);

    // Wake the decoder after the published request becomes visible
    // Manage native synchronization with an explicit lifetime
    AudioWakeSignal_Notify(&gAudioThreadCv);
}

static void AudioReleaseCloseJob(AudioCloseJob *job)
{
    // Release captured format resources before their externally owned state
    if(job->decoder) {
        void *decoderState = job->decoder->state;
        AudioDecoder_Close(job->decoder);
        free(decoderState);
    }
    // Release the captured PCM buffer and scheduling node exactly once
    free(job->buffer);
    free(job);
}
void audioFileOpsThread(void *context)
{
	// The worker uses global scheduler state rather than a heap-allocated context
	(void)context;
	do
	{
		CStream *stream;
		{
			// Just a semaphore
			// Manage native synchronization with an explicit lifetime
			AudioMutex_Lock(&gAudioThreadQueueMutex);
            // Wait for either FIFO or shutdown while retaining the original predicate
            while(AudioProcessQueue_IsEmpty(&gStreamsToProcess)
                && AudioCloseQueue_IsEmpty(&gStreamsToClose) && !gAudioThreadTerm)
                AudioWakeSignal_Wait(&gAudioThreadCv, &gAudioThreadQueueMutex);
            if(gAudioThreadTerm) {
                // Release the scheduler lock before exiting the worker
                AudioMutex_Unlock(&gAudioThreadQueueMutex);
                return;
            }

            // Preserve one-close-then-one-process scheduling and per-queue FIFO ordering
            AudioCloseJob *closeJob = AudioCloseQueue_Pop(&gStreamsToClose);
            if(closeJob) AudioReleaseCloseJob(closeJob);
            AudioProcessJob *processJob = AudioProcessQueue_Pop(&gStreamsToProcess);
            if(processJob) {
                stream = (CStream*)processJob->stream;
                free(processJob);
            } else {
                // Release the scheduler lock before waiting for another request
                AudioMutex_Unlock(&gAudioThreadQueueMutex);
                continue;
            }
            AudioMutex_Unlock(&gAudioThreadQueueMutex);
        }

		// Manage native synchronization with an explicit lifetime
		AudioMutex_Lock(&stream->m_mutex);
		// Track queue revisions across unlocked decoding and deferred publication
		AudioBufferPair buffers;
		uint64_t fillRevision = 0;
		uint64_t pendingRevision = 0;
		bool insertBufsAfterCheck = false;

		do {
			if (!CStream_IsOpened(stream)) {
				break;
			}

			if (stream->m_bReset)
				break;

			// We gave up this idea for now
			/*
			// Use C decoder dispatch while preserving the existing format behavior
			AudioDecoder_FileOpen(stream->m_pSoundFile);
			// Deffered allocation, do it now
			if (stream->m_pBuffer == nil) {
				// Use C decoder dispatch while preserving the existing format behavior
				stream->m_pBuffer = malloc(AudioDecoder_GetBufferSize(stream->m_pSoundFile));
				ASSERT(stream->m_pBuffer != nil);
			}
			*/

			if (stream->m_bDoSeek) {
				stream->m_bDoSeek = false;
				int pos = stream->m_SeekPos;
				// Manage native synchronization with an explicit lifetime
				AudioMutex_Unlock(&stream->m_mutex);
				// Use C decoder dispatch while preserving the existing format behavior
				AudioDecoder_Seek(stream->m_pSoundFile, pos);
				// Manage native synchronization with an explicit lifetime
				AudioMutex_Lock(&stream->m_mutex);
				continue; // let's do the checks again, make sure we didn't miss anything while Seeking
			}

			if (insertBufsAfterCheck) {
				// Keep the original ready-queue lock while rejecting results invalidated by a reset
				if(stream->m_fillBuffers.revision == pendingRevision) {
					// Publish the decoded pair without allocating queue storage
					// Manage native synchronization with an explicit lifetime
					AudioMutex_Lock(&stream->m_queueBuffersMutex);
					bool pushed = AudioBufferQueue_Push(&stream->m_queueBuffers, buffers);
					ASSERT(pushed);
					// Manage native synchronization with an explicit lifetime
					AudioMutex_Unlock(&stream->m_queueBuffersMutex);
				}
				insertBufsAfterCheck = false;
			}

			// Read the next fill request without removing its borrowed front
			if (AudioBufferQueue_Peek(&stream->m_fillBuffers, &buffers)) {
				// Snapshot the front revision before allowing resets during decoding
				fillRevision = stream->m_fillBuffers.revision;
				// Manage native synchronization with an explicit lifetime
				AudioMutex_Unlock(&stream->m_mutex);
				// Upload the borrowed pair through the existing decoder path
				ALuint alBuffers[2] = {buffers.left, buffers.right}; // left - right
				bool filled = CStream_FillBuffer(stream, alBuffers);
				
				// Manage native synchronization with an explicit lifetime
				AudioMutex_Lock(&stream->m_mutex);
				// Reject decoded results if the front changed while the stream lock was released
				if (AudioBufferQueue_IsCurrent(&stream->m_fillBuffers, fillRevision)) {
					// Consume the validated pair and remember the queue state for publication
					AudioBufferQueue_Pop(&stream->m_fillBuffers, &buffers);
					pendingRevision = stream->m_fillBuffers.revision;
					if (filled)
						insertBufsAfterCheck = true; // Also make sure stream's properties aren't changed. So make one more pass, and push it to m_queueBuffers only if it pass checks again.
				}
			} else
				break;

		} while (true);

		// Release the stream after every open, reset, empty or seek exit
		AudioMutex_Unlock(&stream->m_mutex);

	} while(true);
}
#endif

void CStream_Initialise(void)
{
    // Initialize the decoder library before starting any format work
#ifdef AUDIO_OAL_USE_MPG123
    mpg123_init();
#endif
#ifdef MULTITHREADED_AUDIO
    // Initialize native synchronization before publishing the worker
    if(!AudioMutex_Init(&gAudioThreadQueueMutex) || !AudioWakeSignal_Init(&gAudioThreadCv)) abort();
    // Reset shutdown state so a completed scheduler can be initialized again
    // Manage native synchronization with an explicit lifetime
    AudioMutex_Lock(&gAudioThreadQueueMutex);
    gAudioThreadTerm = false;
    // Manage native synchronization with an explicit lifetime
    AudioMutex_Unlock(&gAudioThreadQueueMutex);
    if(!AudioThread_Start(&gAudioThread, audioFileOpsThread, nil)) abort();
#endif
}

void CStream_Terminate(void)
{
#ifdef MULTITHREADED_AUDIO
    // Stop accepting decode work and wait for any unlocked decoding to finish
    // Manage native synchronization with an explicit lifetime
    AudioMutex_Lock(&gAudioThreadQueueMutex);
    gAudioThreadTerm = true;
    // Manage native synchronization with an explicit lifetime
    AudioMutex_Unlock(&gAudioThreadQueueMutex);
    AudioWakeSignal_Notify(&gAudioThreadCv);
    AudioThread_Join(&gAudioThread);

    // Discard pending stream requests without dereferencing their stream objects
    AudioProcessJob *processJob;
    while((processJob = AudioProcessQueue_Pop(&gStreamsToProcess)) != nil)
        free(processJob);

    // Release queued decoder resources while their libraries remain initialized
    AudioCloseJob *closeJob;
    while((closeJob = AudioCloseQueue_Pop(&gStreamsToClose)) != nil)
        AudioReleaseCloseJob(closeJob);
    // Release scheduler synchronization after all native worker use has ended
    AudioWakeSignal_Destroy(&gAudioThreadCv);
    AudioMutex_Destroy(&gAudioThreadQueueMutex);
#endif
#ifdef AUDIO_OAL_USE_MPG123
    // Shut down MPG123 only after decoding and captured cleanup have finished
    mpg123_exit();
#endif
    // Release shared stereo scratch storage after the decoder worker stops
    CSortStereoBuffer_Close(&SortStereoBuffer);
}
void CStream_Init(CStream *stream, ALuint *sources, ALuint *buffers)
{
    // Initialize caller-owned stream storage and retain the caller-owned source and buffer arrays
    stream->m_pAlSources = sources;
    stream->m_alBuffers = buffers;
    stream->m_pBuffer = nil;
    stream->m_bPaused = false;
    stream->m_bActive = false;
#ifdef MULTITHREADED_AUDIO
    stream->m_bIExist = false;
    stream->m_bDoSeek = false;
    stream->m_SeekPos = 0;
#endif
    stream->m_pSoundFile = nil;
    stream->m_bReset = false;
    stream->m_nVolume = 0;
    stream->m_nPan = 0;
    stream->m_nPosBeforeReset = 0;
    stream->m_nLoopCount = 1;

	// Initialize the embedded queues before the stream can be processed
#ifdef MULTITHREADED_AUDIO
	// Initialize native stream locks before another thread can access this stream
	if(!AudioMutex_Init(&stream->m_mutex) || !AudioMutex_Init(&stream->m_queueBuffersMutex)) abort();
	AudioBufferQueue_Init(&stream->m_fillBuffers);
	AudioBufferQueue_Init(&stream->m_queueBuffers);
#endif
}

bool CStream_Open(CStream *stream, const char* filename, uint32 overrideSampleRate)
{
    // Access the caller-owned stream through explicit C state

	if (CStream_IsOpened(stream)) return false;

#ifdef MULTITHREADED_AUDIO
	// Manage native synchronization with an explicit lifetime
	AudioMutex_Lock(&stream->m_mutex);
	stream->m_bDoSeek = false;
	stream->m_SeekPos = 0;
#endif

	stream->m_bPaused = false;
	stream->m_bActive = false;
	stream->m_bReset = false;
	stream->m_nVolume = 0;
	stream->m_nPan = 0;
	stream->m_nPosBeforeReset = 0;
	stream->m_nLoopCount = 1;

// Be case-insensitive on linux (from https://github.com/OneSadCookie/fcaseopen/)
#if !defined(_WIN32)
	char *real = AudioStream_CasePath(filename);
	if (real) {
		strcpy(stream->m_aFilename, real);
		free(real);
	} else {
#else
	{
#endif
		strcpy(stream->m_aFilename, filename);
	}
		
	DEV("Stream %s\n", stream->m_aFilename);

	if (!strcasecmp(&stream->m_aFilename[strlen(stream->m_aFilename) - strlen(".wav")], ".wav"))
#ifdef AUDIO_OAL_USE_SNDFILE
		// Construct the sound-file decoder without invoking C++ new.
	{
		CSndFile *soundFile = (CSndFile*)malloc(sizeof(CSndFile));
		CSndFile_Init(soundFile, stream->m_aFilename); // rouz edit (ChatGPT)
		// Bind the constructed state to its concrete C dispatch table
		AudioDecoder_Init(&soundFile->decoder, &CSndFile_Ops, soundFile);
		stream->m_pSoundFile = &soundFile->decoder;
	}
#else
		// Construct the WAV decoder without invoking C++ new.
	{
		CWavFile *soundFile = (CWavFile*)malloc(sizeof(CWavFile));
		CWavFile_Init(soundFile, stream->m_aFilename); // rouz edit (ChatGPT)
		// Bind the constructed state to its concrete C dispatch table
		AudioDecoder_Init(&soundFile->decoder, &CWavFile_Ops, soundFile);
		stream->m_pSoundFile = &soundFile->decoder;
	}
#endif
#ifdef AUDIO_OAL_USE_MPG123
	else if (!strcasecmp(&stream->m_aFilename[strlen(stream->m_aFilename) - strlen(".mp3")], ".mp3"))
		// Construct the MP3 decoder without invoking C++ new.
	{
		CMP3File *soundFile = (CMP3File*)malloc(sizeof(CMP3File));
		CMP3File_Init(soundFile, stream->m_aFilename); // rouz edit (ChatGPT)
		// Bind the constructed state to its concrete C dispatch table
		AudioDecoder_Init(&soundFile->decoder, &CMP3File_Ops, soundFile);
		stream->m_pSoundFile = &soundFile->decoder;
	}
	else if (!strcasecmp(&stream->m_aFilename[strlen(stream->m_aFilename) - strlen(".adf")], ".adf"))
		// Construct the ADF decoder without invoking C++ new.
	{
		CADFFile *soundFile = (CADFFile*)malloc(sizeof(CADFFile));
		CADFFile_Init(soundFile, stream->m_aFilename); // rouz edit (ChatGPT)
		// Bind the constructed state to its concrete C dispatch table
		AudioDecoder_Init(&soundFile->decoder, &CADFFile_Ops, soundFile);
		stream->m_pSoundFile = &soundFile->decoder;
	}
#endif
	else if (!strcasecmp(&stream->m_aFilename[strlen(stream->m_aFilename) - strlen(".vb")], ".VB"))
		// Construct the VB decoder without invoking C++ new.
	{
		CVbFile *soundFile = (CVbFile*)malloc(sizeof(CVbFile));
		CVbFile_Init(soundFile, stream->m_aFilename, overrideSampleRate, 2); // rouz edit (ChatGPT)
		// Bind the constructed state to its concrete C dispatch table
		AudioDecoder_Init(&soundFile->decoder, &CVbFile_Ops, soundFile);
		stream->m_pSoundFile = &soundFile->decoder;
	}
#ifdef AUDIO_OAL_USE_OPUS
	else if (!strcasecmp(&stream->m_aFilename[strlen(stream->m_aFilename) - strlen(".opus")], ".opus"))
		// Construct the Opus decoder without invoking C++ new.
	{
		COpusFile *soundFile = (COpusFile*)malloc(sizeof(COpusFile));
		COpusFile_Init(soundFile, stream->m_aFilename); // rouz edit (ChatGPT)
		// Bind the constructed state to its concrete C dispatch table
		AudioDecoder_Init(&soundFile->decoder, &COpusFile_Ops, soundFile);
		stream->m_pSoundFile = &soundFile->decoder;
	}
#endif
	else 
		stream->m_pSoundFile = nil;

	// Use C decoder dispatch while preserving the existing format behavior
	if ( stream->m_pSoundFile && AudioDecoder_IsOpened(stream->m_pSoundFile) )
	{
		// Use C decoder dispatch while preserving the existing format behavior
		uint32 bufSize = AudioDecoder_GetBufferSize(stream->m_pSoundFile);
		if(bufSize != 0) { // Otherwise it's deferred
			stream->m_pBuffer = malloc(bufSize);
			ASSERT(stream->m_pBuffer != nil);

			// Use C decoder dispatch while preserving the existing format behavior
			DEV("AvgSamplesPerSec: %d\n", AudioDecoder_GetAvgSamplesPerSec(stream->m_pSoundFile));
			DEV("SampleCount: %d\n",      AudioDecoder_GetSampleCount(stream->m_pSoundFile));
			DEV("SampleRate: %d\n",       AudioDecoder_GetSampleRate(stream->m_pSoundFile));
			DEV("Channels: %d\n",         AudioDecoder_GetChannels(stream->m_pSoundFile));
			DEV("Buffer Samples: %d\n",   AudioDecoder_GetBufferSamples(stream->m_pSoundFile));
			DEV("Buffer sec: %f\n",       ((float)AudioDecoder_GetBufferSamples(stream->m_pSoundFile) / (float)AudioDecoder_GetChannels(stream->m_pSoundFile)/ (float)AudioDecoder_GetSampleRate(stream->m_pSoundFile)));
			DEV("Length MS: %02d:%02d\n", (AudioDecoder_GetLength(stream->m_pSoundFile) / 1000) / 60, (AudioDecoder_GetLength(stream->m_pSoundFile) / 1000) % 60);
		}
#ifdef MULTITHREADED_AUDIO
		stream->m_bIExist = true;
#endif
		// Release the stream before returning successful initialization
#ifdef MULTITHREADED_AUDIO
		AudioMutex_Unlock(&stream->m_mutex);
#endif
		return true;
	}
	// Release the stream before returning a failed format opening
#ifdef MULTITHREADED_AUDIO
	AudioMutex_Unlock(&stream->m_mutex);
#endif
	return false;
}

void CStream_Destroy(CStream *stream)
{
    // Release stream synchronization after closing and joining the scheduler

	assert(!CStream_IsOpened(stream));
#ifdef MULTITHREADED_AUDIO
	// Release stream mutexes after the stream can no longer be processed
	AudioMutex_Destroy(&stream->m_queueBuffersMutex);
	AudioMutex_Destroy(&stream->m_mutex);
#endif
}

void CStream_Close(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if(!CStream_IsOpened(stream)) return;

#ifdef MULTITHREADED_AUDIO
	{
		// Manage native synchronization with an explicit lifetime
		AudioMutex_Lock(&stream->m_mutex);
		CStream_Stop(stream);
		CStream_ClearBuffers(stream);
		stream->m_bIExist = false;
		// Discard pending pairs when closing the stream
		AudioBufferQueue_Clear(&stream->m_fillBuffers);
		AudioBufferQueue_Clear(&stream->m_queueBuffers);
		// Publish the closed state before capturing cleanup in the scheduler
		AudioMutex_Unlock(&stream->m_mutex);
	}

	CStream_FlagAsToBeProcessed(stream, true);
#else

	CStream_Stop(stream);
	CStream_ClearBuffers(stream);

	if ( stream->m_pSoundFile )
	{
		// Destroy and release the decoder without invoking C++ delete.
		// Use C decoder dispatch while preserving the existing format behavior
		void *decoderState = stream->m_pSoundFile->state;
		AudioDecoder_Close(stream->m_pSoundFile);
		free(decoderState);
		stream->m_pSoundFile = nil;
	}

	if ( stream->m_pBuffer )
	{
		free(stream->m_pBuffer);
		stream->m_pBuffer = nil;
	}
#endif
}

bool CStream_HasSource(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	return (stream->m_pAlSources[0] != AL_NONE) && (stream->m_pAlSources[1] != AL_NONE);
}

// m_bIExist only written in main thread, thus mutex is not needed on main thread
bool CStream_IsOpened(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

#ifdef MULTITHREADED_AUDIO
	return stream->m_bIExist;
#else
	// Use C decoder dispatch while preserving the existing format behavior
	return stream->m_pSoundFile && AudioDecoder_IsOpened(stream->m_pSoundFile);
#endif
}

bool CStream_IsPlaying(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) || !CStream_IsOpened(stream) ) return false;
	
	if ( !stream->m_bPaused )
	{
		ALint sourceState[2];
		alGetSourcei(stream->m_pAlSources[0], AL_SOURCE_STATE, &sourceState[0]);
		alGetSourcei(stream->m_pAlSources[1], AL_SOURCE_STATE, &sourceState[1]);
		if (sourceState[0] == AL_PLAYING || sourceState[1] == AL_PLAYING)
			return true;

#ifdef MULTITHREADED_AUDIO
		// Inspect pending work under the stream lock and release it before returning
		AudioMutex_Lock(&stream->m_mutex);
		bool pending = !AudioBufferQueue_IsEmpty(&stream->m_fillBuffers) || !AudioBufferQueue_IsEmpty(&stream->m_queueBuffers);
		AudioMutex_Unlock(&stream->m_mutex);
		if(pending) return true;
#endif
	}
	
	return false;
}

void CStream_Pause(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	ALint sourceState = AL_PAUSED;
	alGetSourcei(stream->m_pAlSources[0], AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PAUSED)
		alSourcePause(stream->m_pAlSources[0]);
	alGetSourcei(stream->m_pAlSources[1], AL_SOURCE_STATE, &sourceState);
	if (sourceState != AL_PAUSED)
		alSourcePause(stream->m_pAlSources[1]);
}

void CStream_SetPause(CStream *stream, bool bPause)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	if ( bPause )
	{
		CStream_Pause(stream);
		stream->m_bPaused = true;
	}
	else
	{
		if (stream->m_bPaused)
			CStream_SetPlay(stream, true);
		stream->m_bPaused = false;
	}
}

void CStream_SetPitch(CStream *stream, float pitch)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	alSourcef(stream->m_pAlSources[0], AL_PITCH, pitch);
	alSourcef(stream->m_pAlSources[1], AL_PITCH, pitch);
}

void CStream_SetGain(CStream *stream, float gain)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	alSourcef(stream->m_pAlSources[0], AL_GAIN, gain);
	alSourcef(stream->m_pAlSources[1], AL_GAIN, gain);
}

void CStream_SetPosition(CStream *stream, int i, float x, float y, float z)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	alSource3f(stream->m_pAlSources[i], AL_POSITION, x, y, z);
}

void CStream_SetVolume(CStream *stream, uint32 nVol)
{
    // Access the caller-owned stream through explicit C state

	stream->m_nVolume = nVol;
	CStream_SetGain(stream, (ALfloat)nVol / MAX_VOLUME);
}

void CStream_SetPan(CStream *stream, uint8 nPan)
{
    // Access the caller-owned stream through explicit C state

	stream->m_nPan = Clamp((int8)nPan - 63, 0, 63);
	CStream_SetPosition(stream, 0, (stream->m_nPan - 63) / 64.0f, 0.0f, Sqrt(1.0f - SQR((stream->m_nPan - 63) / 64.0f)));

	stream->m_nPan = Clamp((int8)nPan + 64, 64, 127);
	CStream_SetPosition(stream, 1, (stream->m_nPan - 63) / 64.0f, 0.0f, Sqrt(1.0f - SQR((stream->m_nPan - 63) / 64.0f)));

	stream->m_nPan = nPan;
}

// Should only be called if source is stopped
void CStream_SetPosMS(CStream *stream, uint32 nPos)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_IsOpened(stream) ) return;
	
#ifdef MULTITHREADED_AUDIO
	// Manage native synchronization with an explicit lifetime
	AudioMutex_Lock(&stream->m_mutex);
	// Invalidate pending pairs before seeking the decoder
	AudioBufferQueue_Clear(&stream->m_fillBuffers);
	AudioBufferQueue_Clear(&stream->m_queueBuffers);
	if (!AudioStream_IsCutscene()) {
		stream->m_bDoSeek = true;
		stream->m_SeekPos = nPos;
	} else
#endif	
	{
		// Use C decoder dispatch while preserving the existing format behavior
		AudioDecoder_Seek(stream->m_pSoundFile, nPos);
	}	
	CStream_ClearBuffers(stream);
	
	// adding to gStreamsToProcess not needed, someone always calls CStream_Start(stream) / CStream_BuffersShouldBeFilled(stream) after SetPosMS
	// Release the explicit stream lock on normal function completion
#ifdef MULTITHREADED_AUDIO
	AudioMutex_Unlock(&stream->m_mutex);
#endif
}

uint32 CStream_GetPosMS(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return 0;
	if ( !CStream_IsOpened(stream) ) return 0;
	
	// Deferred init causes division by zero
	// Use C decoder dispatch while preserving the existing format behavior
	if (AudioDecoder_GetChannels(stream->m_pSoundFile) == 0)
		return 0;

	ALint offset;
	//alGetSourcei(stream->m_alSource, AL_SAMPLE_OFFSET, &offset);
	alGetSourcei(stream->m_pAlSources[0], AL_BYTE_OFFSET, &offset);

	// Position reads retain their existing caller synchronization
	// Use C decoder dispatch while preserving the existing format behavior
	return AudioDecoder_Tell(stream->m_pSoundFile)
		- AudioDecoder_SamplesToMilliseconds(stream->m_pSoundFile, AudioDecoder_GetBufferSamples(stream->m_pSoundFile) * (NUM_STREAMBUFFERS/2-1)) / AudioDecoder_GetChannels(stream->m_pSoundFile)
		+ AudioDecoder_SamplesToMilliseconds(stream->m_pSoundFile, offset/AudioDecoder_GetSampleSize(stream->m_pSoundFile)) / AudioDecoder_GetChannels(stream->m_pSoundFile);
}

uint32 CStream_GetLengthMS(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_IsOpened(stream) ) return 0;
	// Use C decoder dispatch while preserving the existing format behavior
	return AudioDecoder_GetLength(stream->m_pSoundFile);
}

bool CStream_FillBuffer(CStream *stream, ALuint *alBuffer)
{
    // Access the caller-owned stream through explicit C state

#ifndef MULTITHREADED_AUDIO
	if ( !CStream_HasSource(stream) )
		return false;
	if ( !CStream_IsOpened(stream) )
		return false;
	if ( !(alBuffer[0] != AL_NONE && alIsBuffer(alBuffer[0])) )
		return false;
	if ( !(alBuffer[1] != AL_NONE && alIsBuffer(alBuffer[1])) )
		return false;
#endif

	// Use C decoder dispatch while preserving the existing format behavior
	uint32 size = AudioDecoder_Decode(stream->m_pSoundFile, stream->m_pBuffer);
	if( size == 0 )
		return false;

	// Use C decoder dispatch while preserving the existing format behavior
	uint32 channelSize = size / AudioDecoder_GetChannels(stream->m_pSoundFile);
	// Use C decoder dispatch while preserving the existing format behavior
	alBufferData(alBuffer[0], AL_FORMAT_MONO16, stream->m_pBuffer, channelSize, AudioDecoder_GetSampleRate(stream->m_pSoundFile));
	// TODO: use just one buffer if we play mono
	// Use C decoder dispatch while preserving the existing format behavior
	if (AudioDecoder_GetChannels(stream->m_pSoundFile) == 1)
		alBufferData(alBuffer[1], AL_FORMAT_MONO16, stream->m_pBuffer, channelSize, AudioDecoder_GetSampleRate(stream->m_pSoundFile));
	else
		// Use C decoder dispatch while preserving the existing format behavior
		alBufferData(alBuffer[1], AL_FORMAT_MONO16, (uint8*)stream->m_pBuffer + channelSize, channelSize, AudioDecoder_GetSampleRate(stream->m_pSoundFile));
	return true;
}

#ifdef MULTITHREADED_AUDIO
bool CStream_QueueBuffers(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	bool buffersQueued = false;
	// Drain ready pairs through the dedicated producer and consumer lock
	AudioBufferPair buffers;
	while (true)
	{
		// Pop a ready pair under the same dedicated lock used by the decoder worker
		{
			// Manage native synchronization with an explicit lifetime
			AudioMutex_Lock(&stream->m_queueBuffersMutex);
			bool popped = AudioBufferQueue_Pop(&stream->m_queueBuffers, &buffers);
			AudioMutex_Unlock(&stream->m_queueBuffersMutex);
			if(!popped) break;
		}
		ALuint leftBuf = buffers.left;
		ALuint rightBuf = buffers.right;
		alSourceQueueBuffers(stream->m_pAlSources[0], 1, &leftBuf);
		alSourceQueueBuffers(stream->m_pAlSources[1], 1, &rightBuf);

		buffersQueued = true;
	}
	return buffersQueued;	
}
#endif

// Only used in single-threaded audio or cutscene audio
int32 CStream_FillBuffers(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	int32 i = 0;
	for ( i = 0; i < NUM_STREAMBUFFERS/2; i++ )
	{
		if ( !CStream_FillBuffer(stream, &stream->m_alBuffers[i*2]) )
			break;
		alSourceQueueBuffers(stream->m_pAlSources[0], 1, &stream->m_alBuffers[i*2]);
		alSourceQueueBuffers(stream->m_pAlSources[1], 1, &stream->m_alBuffers[i*2+1]);
	}
	
	return i;
}

void CStream_ClearBuffers(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	
	ALint buffersQueued[2];
	alGetSourcei(stream->m_pAlSources[0], AL_BUFFERS_QUEUED, &buffersQueued[0]);
	alGetSourcei(stream->m_pAlSources[1], AL_BUFFERS_QUEUED, &buffersQueued[1]);

	ALuint value;
	while (buffersQueued[0]--)
		alSourceUnqueueBuffers(stream->m_pAlSources[0], 1, &value);
	while (buffersQueued[1]--)
		alSourceUnqueueBuffers(stream->m_pAlSources[1], 1, &value);
}

bool CStream_Setup(CStream *stream, bool imSureQueueIsEmpty, bool lock)
{
    // Access the caller-owned stream through explicit C state

	if ( CStream_IsOpened(stream) )
	{
#ifdef MULTITHREADED_AUDIO
		if (lock)
			// Manage native synchronization with an explicit lifetime
			AudioMutex_Lock(&stream->m_mutex);
#endif

		if (!imSureQueueIsEmpty) {
			CStream_Stop(stream);
			CStream_ClearBuffers(stream);
		}
#ifdef MULTITHREADED_AUDIO
		if (AudioStream_IsCutscene()) {
			// Use C decoder dispatch while preserving the existing format behavior
			AudioDecoder_Seek(stream->m_pSoundFile, 0);
		} else {
			stream->m_bDoSeek = true;
			stream->m_SeekPos = 0;
		}

		if (lock)
			// Manage native synchronization with an explicit lifetime
			AudioMutex_Unlock(&stream->m_mutex);
#else
		// Use C decoder dispatch while preserving the existing format behavior
		AudioDecoder_Seek(stream->m_pSoundFile, 0);
#endif

		//CStream_SetPosition(stream, 0.0f, 0.0f, 0.0f);
		CStream_SetPitch(stream, 1.0f);
		//CStream_SetPan(stream, stream->m_nPan);
		//CStream_SetVolume(stream, 100);
	}
	
	return CStream_IsOpened(stream);
}

void CStream_SetLoopCount(CStream *stream, int32 count)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;

	stream->m_nLoopCount = count;
}

void CStream_SetPlay(CStream *stream, bool state)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	if ( state )
	{
		ALint sourceState = AL_PLAYING;
		alGetSourcei(stream->m_pAlSources[0], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_PLAYING )
			alSourcePlay(stream->m_pAlSources[0]);

		sourceState = AL_PLAYING;
		alGetSourcei(stream->m_pAlSources[1], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_PLAYING)
			alSourcePlay(stream->m_pAlSources[1]);

		stream->m_bActive = true;
	}
	else
	{
		ALint sourceState = AL_STOPPED;
		alGetSourcei(stream->m_pAlSources[0], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_STOPPED)
			alSourceStop(stream->m_pAlSources[0]);

		sourceState = AL_STOPPED;
		alGetSourcei(stream->m_pAlSources[1], AL_SOURCE_STATE, &sourceState);
		if (sourceState != AL_STOPPED)
			alSourceStop(stream->m_pAlSources[1]);

		stream->m_bActive = false;
	}
}

void CStream_Start(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	
#ifdef MULTITHREADED_AUDIO
	// Manage native synchronization with an explicit lifetime
	AudioMutex_Lock(&stream->m_mutex);
	// Discard earlier ready pairs before restarting the stream
	AudioBufferQueue_Clear(&stream->m_queueBuffers);
#endif
	CStream_BuffersShouldBeFilled(stream);
	// Release the explicit stream lock on normal function completion
#ifdef MULTITHREADED_AUDIO
	AudioMutex_Unlock(&stream->m_mutex);
#endif
}

void CStream_Stop(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_HasSource(stream) ) return;
	CStream_SetPlay(stream, false);
}

void CStream_Update(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( !CStream_IsOpened(stream) )
		return;
	
	if ( !CStream_HasSource(stream) )
		return;
	
	if ( stream->m_bReset )
		return;
	
	if ( !stream->m_bPaused )
	{
		
		bool buffersQueuedAndStarted = false;
		bool buffersQueuedButNotStarted = false;
#ifdef MULTITHREADED_AUDIO
		// Put it in here because we need totalBuffers after queueing to decide when to loop audio
		if (stream->m_bActive)
		{
			buffersQueuedAndStarted = CStream_QueueBuffers(stream);
			if(buffersQueuedAndStarted) {
				CStream_SetPlay(stream, true);
			}
		}
#endif

		ALint totalBuffers[2] = {0, 0};
		ALint buffersProcessed[2] = {0, 0};

		// Relying a lot on left buffer states in here

		do
		{
			//alSourcef(stream->m_pAlSources[0], AL_ROLLOFF_FACTOR, 0.0f);
			alGetSourcei(stream->m_pAlSources[0], AL_BUFFERS_QUEUED, &totalBuffers[0]);
			alGetSourcei(stream->m_pAlSources[0], AL_BUFFERS_PROCESSED, &buffersProcessed[0]);
			//alSourcef(stream->m_pAlSources[1], AL_ROLLOFF_FACTOR, 0.0f);
			alGetSourcei(stream->m_pAlSources[1], AL_BUFFERS_QUEUED, &totalBuffers[1]);
			alGetSourcei(stream->m_pAlSources[1], AL_BUFFERS_PROCESSED, &buffersProcessed[1]);
		} while (buffersProcessed[0] != buffersProcessed[1]);

		assert(buffersProcessed[0] == buffersProcessed[1]);

		// Correcting OpenAL concepts here:
		// AL_BUFFERS_QUEUED = Number of *all* buffers in queue, including processed, processing and pending
		// AL_BUFFERS_PROCESSED = Index of the buffer being processing right now. Buffers coming after that(have greater index) are pending buffers.
		// which means: totalBuffers[0] - buffersProcessed[0] = pending buffers
		
		// We should wait queue to be cleared to loop track, because position calculation relies on queue.
		if (stream->m_nLoopCount != 1 && stream->m_bActive && totalBuffers[0] == 0)
		{
#ifdef MULTITHREADED_AUDIO
			// Manage native synchronization with an explicit lifetime
			AudioMutex_Lock(&stream->m_mutex);
			// Restart looping playback only when both pending queues are empty
			if (AudioBufferQueue_IsEmpty(&stream->m_fillBuffers) && AudioBufferQueue_IsEmpty(&stream->m_queueBuffers)) // The stream lock excludes ready-queue publication
#endif
			{
				CStream_Setup(stream, true, false);
				CStream_BuffersShouldBeFilled(stream); // will also call CStream_SetPlay(stream, true)
				if (stream->m_nLoopCount != 0)
					stream->m_nLoopCount--;
			}
#ifdef MULTITHREADED_AUDIO
			// Release the loop-restart lock before continuing playback polling
			AudioMutex_Unlock(&stream->m_mutex);
#endif
		}
		else
		{
			// Collect processed pairs in a local embedded queue
			AudioBufferQueue tempFillBuffer;
			AudioBufferQueue_Init(&tempFillBuffer);
			while ( buffersProcessed[0]-- )
			{
				ALuint buffer[2];
				
				alSourceUnqueueBuffers(stream->m_pAlSources[0], 1, &buffer[0]);
				alSourceUnqueueBuffers(stream->m_pAlSources[1], 1, &buffer[1]);

				if (stream->m_bActive)
				{
					// Preserve the processed left and right pairing for recycling
					AudioBufferPair pair = {buffer[0], buffer[1]};
					bool pushed = AudioBufferQueue_Push(&tempFillBuffer, pair);
					ASSERT(pushed);
				}
			}

			if (stream->m_bActive && buffersProcessed[1])
			{
#ifdef MULTITHREADED_AUDIO
				// Manage native synchronization with an explicit lifetime
				AudioMutex_Lock(&stream->m_mutex);
#endif
				// Return each collected pair to the existing refill path
				AudioBufferPair elem;
				while (AudioBufferQueue_Pop(&tempFillBuffer, &elem)) {
					buffersQueuedButNotStarted = CStream_BufferShouldBeFilledAndQueued(stream, &elem);
				}
#ifdef MULTITHREADED_AUDIO
				// Manage native synchronization with an explicit lifetime
				AudioMutex_Unlock(&stream->m_mutex);
				CStream_FlagAsToBeProcessed(stream, false);
#endif

			}
		}

		// Source may be starved to audio and stopped itself
		if (stream->m_bActive && !buffersQueuedAndStarted && (buffersQueuedButNotStarted || (totalBuffers[1] - buffersProcessed[1] != 0)))
			CStream_SetPlay(stream, true);
	}
}

void CStream_ProviderInit(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

	if ( stream->m_bReset )
	{
		if ( CStream_Setup(stream, true, false) ) // lock not needed, thread can't process streams with stream->m_bReset set
		{
			CStream_SetPan(stream, stream->m_nPan);
			CStream_SetVolume(stream, stream->m_nVolume);
			CStream_SetLoopCount(stream, stream->m_nLoopCount);
			CStream_SetPosMS(stream, stream->m_nPosBeforeReset);
#ifdef MULTITHREADED_AUDIO
			// Manage native synchronization with an explicit lifetime
			AudioMutex_Lock(&stream->m_mutex);
#endif
			if(stream->m_bActive)
				CStream_BuffersShouldBeFilled(stream);

			if (stream->m_bPaused)
				CStream_Pause(stream);

			stream->m_bReset = false;
#ifdef MULTITHREADED_AUDIO
			// Publish provider reset completion before releasing the stream
			AudioMutex_Unlock(&stream->m_mutex);
#endif
		} else {
#ifdef MULTITHREADED_AUDIO
			// Manage native synchronization with an explicit lifetime
			AudioMutex_Lock(&stream->m_mutex);
#endif
			stream->m_bReset = false;
#ifdef MULTITHREADED_AUDIO
			// Publish provider reset completion before releasing the stream
			AudioMutex_Unlock(&stream->m_mutex);
#endif
		}
	}
}

void CStream_ProviderTerm(CStream *stream)
{
    // Access the caller-owned stream through explicit C state

#ifdef MULTITHREADED_AUDIO
	// Manage native synchronization with an explicit lifetime
	AudioMutex_Lock(&stream->m_mutex);
	// unlike CStream_Close(stream) we will reuse stream stream, so clearing queues are important.
	// Discard pending pairs before resetting the audio provider
	AudioBufferQueue_Clear(&stream->m_fillBuffers);
	AudioBufferQueue_Clear(&stream->m_queueBuffers);
#endif
	stream->m_bReset = true;
	stream->m_nPosBeforeReset = CStream_GetPosMS(stream);

	CStream_Stop(stream);
	CStream_ClearBuffers(stream);
	// Release the explicit stream lock on normal function completion
#ifdef MULTITHREADED_AUDIO
	AudioMutex_Unlock(&stream->m_mutex);
#endif
}
	
#endif
//- rouz edit (ChatGPT)
