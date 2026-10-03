#pragma once

//+ rouz edit (ChatGPT)
#ifdef AUDIO_OAL
#include <stdbool.h>
#include <stdint.h>
#include <AL/al.h>
#include "AudioBufferQueue.h"
#include "AudioDecoder.h"
#ifdef MULTITHREADED_AUDIO
#include "AudioThread.h"
#endif
#define NUM_STREAMBUFFERS (AUDIO_BUFFER_PAIR_CAPACITY * 2)
#ifdef __cplusplus
static_assert(sizeof(ALuint) == sizeof(uint32_t), "Audio buffer IDs must be 32 bits");
extern "C" {
#else
_Static_assert(sizeof(ALuint) == sizeof(uint32_t), "Audio buffer IDs must be 32 bits");
#endif

typedef struct CStream {
	char     m_aFilename[128];
	ALuint  *m_pAlSources;
	ALuint *m_alBuffers;
	
	bool     m_bPaused;
	bool     m_bActive;
	

#ifdef MULTITHREADED_AUDIO
	AudioMutex m_mutex; // rouz edit (ChatGPT)
	AudioBufferQueue m_fillBuffers;
	AudioBufferQueue m_queueBuffers;
	AudioMutex m_queueBuffersMutex;
// Native worker notifications are owned by the scheduler // rouz edit (ChatGPT)
	bool     m_bDoSeek;
	uint32_t   m_SeekPos;
	bool	 m_bIExist;
#endif

	void    *m_pBuffer;
	
	bool     m_bReset;
	uint32_t   m_nVolume;
	uint8_t    m_nPan;
	uint32_t   m_nPosBeforeReset;
	int32_t   m_nLoopCount;
	
	AudioDecoder *m_pSoundFile; // rouz edit (ChatGPT)

} CStream;

void CStream_Init(CStream *stream, ALuint *sources, ALuint *buffers);
void CStream_Destroy(CStream *stream);
void CStream_BuffersShouldBeFilled(CStream *stream);
bool CStream_BufferShouldBeFilledAndQueued(CStream *stream, AudioBufferPair*);
#ifdef MULTITHREADED_AUDIO
void CStream_FlagAsToBeProcessed(CStream *stream, bool close);
#endif
#ifdef MULTITHREADED_AUDIO
bool CStream_QueueBuffers(CStream *stream);
#endif
bool CStream_HasSource(CStream *stream);
void CStream_SetPosition(CStream *stream, int i, float x, float y, float z);
void CStream_SetPitch(CStream *stream, float pitch);
void CStream_SetGain(CStream *stream, float gain);
void CStream_Pause(CStream *stream);
void CStream_SetPlay(CStream *stream, bool state);
bool CStream_FillBuffer(CStream *stream, ALuint *alBuffer);
int32_t CStream_FillBuffers(CStream *stream);
void CStream_ClearBuffers(CStream *stream);
void CStream_Initialise(void);
void CStream_Terminate(void);
bool CStream_Open(CStream *stream, const char *filename, uint32_t overrideSampleRate);
void CStream_Close(CStream *stream);
bool CStream_IsOpened(CStream *stream);
bool CStream_IsPlaying(CStream *stream);
void CStream_SetPause(CStream *stream, bool bPause);
void CStream_SetVolume(CStream *stream, uint32_t nVol);
void CStream_SetPan(CStream *stream, uint8_t nPan);
void CStream_SetPosMS(CStream *stream, uint32_t nPos);
uint32_t CStream_GetPosMS(CStream *stream);
uint32_t CStream_GetLengthMS(CStream *stream);
bool CStream_Setup(CStream *stream, bool imSureQueueIsEmpty, bool lock);
void CStream_Start(CStream *stream);
void CStream_Stop(CStream *stream);
void CStream_Update(CStream *stream);
void CStream_SetLoopCount(CStream *stream, int32_t);
void CStream_ProviderInit(CStream *stream);
void CStream_ProviderTerm(CStream *stream);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
