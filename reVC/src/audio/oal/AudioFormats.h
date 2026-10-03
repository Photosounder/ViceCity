#pragma once

//+ rouz edit (ChatGPT)
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "AudioDecoder.h"
#include "AudioADPCM.h"
#include "AudioStereoBuffer.h"
#ifdef AUDIO_OAL_USE_SNDFILE
#include <sndfile.h>
#endif
#ifdef AUDIO_OAL_USE_MPG123
#include <mpg123.h>
#endif
#ifdef AUDIO_OAL_USE_OPUS
#include <opusfile.h>
#endif

#define AUDIO_FORMAT_MIN(a,b) ((a) < (b) ? (a) : (b))
#ifndef AUDIO_FORMAT_DEBUG
#define AUDIO_FORMAT_DEBUG(...) ((void)0)
#define AUDIO_FORMAT_DEFAULT_DEBUG
#endif

static CSortStereoBuffer SortStereoBuffer = {NULL, 0};

enum { WAVEFMT_PCM = 1, WAVEFMT_IMA_ADPCM = 0x11, WAVEFMT_XBOX_ADPCM = 0x69, SAMPLES_IN_LINE = 8 };
typedef struct CWavDataHeader { uint32_t ID; uint32_t Size; } CWavDataHeader;
typedef struct CWavFormatHeader {
    uint16_t AudioFormat, NumChannels;
    uint32_t SampleRate, ByteRate;
    uint16_t BlockAlign, BitsPerSample, extra[2];
} CWavFormatHeader;
typedef struct CWavFile {
    AudioDecoder decoder;
    FILE *m_pFile;
    bool m_bIsOpen;
    CWavFormatHeader m_FormatHeader;
    uint32_t m_DataStartOffset, m_nSampleCount, m_nSamplesPerBlock;
    uint8_t *m_pAdpcmBuffer;
    int16_t **m_ppPcmBuffers;
    CImaADPCMDecoder *m_pAdpcmDecoders;
} CWavFile;

static void CWavFile_CloseFile(void *state);
static uint32_t CWavFile_GetCurrentSample(void *state);
static void CWavFile_FileOpen(void *state);
static bool CWavFile_IsOpened(void *state);
static uint32_t CWavFile_GetSampleSize(void *state);
static uint32_t CWavFile_GetSampleCount(void *state);
static uint32_t CWavFile_GetSampleRate(void *state);
static uint32_t CWavFile_GetChannels(void *state);
static void CWavFile_Seek(void *state, uint32_t milliseconds);
static uint32_t CWavFile_Tell(void *state);
static uint32_t CWavFile_Decode(void *state, void *buffer);
static void CWavFile_Close(void *state);
static void CWavFile_Init(CWavFile *self, const char *path)
{
	// Initialize format fields before opening resources
	self->m_pFile = NULL;
	self->m_bIsOpen = false;
	memset(&self->m_FormatHeader, 0, sizeof(self->m_FormatHeader));
	self->m_DataStartOffset = self->m_nSampleCount = self->m_nSamplesPerBlock = 0;
	self->m_pAdpcmBuffer = NULL;
	self->m_ppPcmBuffers = NULL;
	self->m_pAdpcmDecoders = NULL;
	// Preserve the existing CWavFile Init behavior
	self->m_pFile = fopen(path, "rb");
	if (!self->m_pFile) return;

#define CLOSE_ON_ERROR(op)\
		if (op) { \
			CWavFile_CloseFile(self); \
			return; \
		}

	CWavDataHeader DataHeader;

	CLOSE_ON_ERROR(fread(&DataHeader, sizeof(DataHeader), 1, self->m_pFile) == 0);
	CLOSE_ON_ERROR(DataHeader.ID != 'FFIR');

	// TODO? validate filesizes

	int WAVE;
	CLOSE_ON_ERROR(fread(&WAVE, 4, 1, self->m_pFile) == 0);
	CLOSE_ON_ERROR(WAVE != 'EVAW')
	CLOSE_ON_ERROR(fread(&DataHeader, sizeof(DataHeader), 1, self->m_pFile) == 0);
	CLOSE_ON_ERROR(DataHeader.ID != ' tmf');

	CLOSE_ON_ERROR(fread(&self->m_FormatHeader, AUDIO_FORMAT_MIN(DataHeader.Size, sizeof(CWavFormatHeader)), 1, self->m_pFile) == 0);
	CLOSE_ON_ERROR(DataHeader.Size > sizeof(CWavFormatHeader));

	switch (self->m_FormatHeader.AudioFormat)
	{
	case WAVEFMT_XBOX_ADPCM:
		self->m_FormatHeader.AudioFormat = WAVEFMT_IMA_ADPCM;
		// Fall through
	case WAVEFMT_IMA_ADPCM:
		self->m_nSamplesPerBlock = (self->m_FormatHeader.BlockAlign / self->m_FormatHeader.NumChannels - 4) * 2 + 1;
		self->m_pAdpcmBuffer = (uint8_t*)malloc(self->m_FormatHeader.BlockAlign);
		self->m_ppPcmBuffers = (int16_t**)malloc(sizeof(int16_t*)*self->m_FormatHeader.NumChannels);
		// Allocate and initialize the IMA decoder state array
		self->m_pAdpcmDecoders = (CImaADPCMDecoder*)malloc(sizeof(CImaADPCMDecoder)*self->m_FormatHeader.NumChannels);
		for(uint32_t i = 0; i < self->m_FormatHeader.NumChannels; i++)
			CImaADPCMDecoder_Init(&self->m_pAdpcmDecoders[i], 0, 0);
		break;
	case WAVEFMT_PCM:
		self->m_nSamplesPerBlock = 1;
		if (self->m_FormatHeader.BitsPerSample != 16)
		{
			AUDIO_FORMAT_DEBUG("Unsupported PCM (%d bits), only signed 16-bit is supported (%s)\n", self->m_FormatHeader.BitsPerSample, path);
			CWavFile_CloseFile(self);
			return;
		}
		break;
	default:
		AUDIO_FORMAT_DEBUG("Unsupported wav format 0x%x (%s)\n", self->m_FormatHeader.AudioFormat, path);
		CWavFile_CloseFile(self);
		return;
	}

	while (true) {
		CLOSE_ON_ERROR(fread(&DataHeader, sizeof(DataHeader), 1, self->m_pFile) == 0);
		if (DataHeader.ID == 'atad')
			break;
		fseek(self->m_pFile, DataHeader.Size, SEEK_CUR);
		// TODO? validate data size
		// Maybe check if there no extreme custom headers that might break this
	}
	
	self->m_DataStartOffset = ftell(self->m_pFile);
	self->m_nSampleCount = DataHeader.Size / self->m_FormatHeader.BlockAlign * self->m_nSamplesPerBlock;

	self->m_bIsOpen = true;
#undef CLOSE_ON_ERROR
}

static void CWavFile_CloseFile(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile Close behavior
	if (self->m_pFile) {
		fclose(self->m_pFile);
		self->m_pFile = NULL;
	}
	free(self->m_pAdpcmBuffer);
	free(self->m_ppPcmBuffers);
	// Release the embedded IMA decoder state array
	if(self->m_pAdpcmDecoders){
		// Release plain IMA states without invoking destructors
		free(self->m_pAdpcmDecoders);
	}
}

static uint32_t CWavFile_GetCurrentSample(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile GetCurrentSample behavior
	// TODO: 64 bit?
	uint32_t FilePos = ftell(self->m_pFile);
	if (FilePos <= self->m_DataStartOffset)
		return 0;
	return (FilePos - self->m_DataStartOffset) / self->m_FormatHeader.BlockAlign * self->m_nSamplesPerBlock;
}

static void CWavFile_FileOpen(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile FileOpen behavior
	(void)self;
}

static bool CWavFile_IsOpened(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile IsOpened behavior
	return self->m_bIsOpen;
}

static uint32_t CWavFile_GetSampleSize(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile GetSampleSize behavior
	return sizeof(uint16_t);
}

static uint32_t CWavFile_GetSampleCount(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile GetSampleCount behavior
	return self->m_nSampleCount;
}

static uint32_t CWavFile_GetSampleRate(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile GetSampleRate behavior
	return self->m_FormatHeader.SampleRate;
}

static uint32_t CWavFile_GetChannels(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile GetChannels behavior
	return self->m_FormatHeader.NumChannels;
}

static void CWavFile_Seek(void *state, uint32_t milliseconds)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile Seek behavior
	if (!CWavFile_IsOpened(self)) return;
	fseek(self->m_pFile, self->m_DataStartOffset + AudioDecoder_MillisecondsToSamples(&self->decoder, milliseconds) / self->m_nSamplesPerBlock * self->m_FormatHeader.BlockAlign, SEEK_SET);
}

static uint32_t CWavFile_Tell(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile Tell behavior
	if (!CWavFile_IsOpened(self)) return 0;
	return AudioDecoder_SamplesToMilliseconds(&self->decoder, CWavFile_GetCurrentSample(self));
}

static uint32_t CWavFile_Decode(void *state, void *buffer)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile Decode behavior
	if (!CWavFile_IsOpened(self)) return 0;
	
	if (self->m_FormatHeader.AudioFormat == WAVEFMT_PCM)
	{
		// Just read the file and sort the samples
		uint32_t size = fread(buffer, 1, AudioDecoder_GetBufferSize(&self->decoder), self->m_pFile);
		if (self->m_FormatHeader.NumChannels == 2)
			CSortStereoBuffer_SortStereo(&SortStereoBuffer, buffer, size);
		return size;
	}
	else if (self->m_FormatHeader.AudioFormat == WAVEFMT_IMA_ADPCM)
	{
		// Trim the buffer size if we're at the end of our file
		uint32_t nMaxSamples = AudioDecoder_GetBufferSamples(&self->decoder) / self->m_FormatHeader.NumChannels;
		uint32_t nSamplesLeft = self->m_nSampleCount - CWavFile_GetCurrentSample(self);
		nMaxSamples = AUDIO_FORMAT_MIN(nMaxSamples, nSamplesLeft);

		// Align sample count to our block
		nMaxSamples = nMaxSamples / self->m_nSamplesPerBlock * self->m_nSamplesPerBlock;

		// Count the size of output buffer
		uint32_t OutBufSizePerChannel = nMaxSamples * CWavFile_GetSampleSize(self);
		uint32_t OutBufSize = OutBufSizePerChannel * self->m_FormatHeader.NumChannels;

		// Calculate the pointers to individual channel buffers
		for (uint32_t i = 0; i < self->m_FormatHeader.NumChannels; i++)
			self->m_ppPcmBuffers[i] = (int16_t*)((int8_t*)buffer + OutBufSizePerChannel * i);

		uint32_t samplesRead = 0;
		while (samplesRead < nMaxSamples)
		{
			// Read the file
			uint8_t *pAdpcmBuf = self->m_pAdpcmBuffer;
			if (fread(self->m_pAdpcmBuffer, 1, self->m_FormatHeader.BlockAlign, self->m_pFile) == 0)
				return 0;

			// Get the first sample in adpcm block and initialise the decoder(s)
			for (uint32_t i = 0; i < self->m_FormatHeader.NumChannels; i++)
			{
				int16_t Sample = *(int16_t*)pAdpcmBuf;
				pAdpcmBuf += sizeof(int16_t);
				int16_t Step = *(int16_t*)pAdpcmBuf;
				pAdpcmBuf += sizeof(int16_t);
				CImaADPCMDecoder_Init(&self->m_pAdpcmDecoders[i], Sample, Step);
				*(self->m_ppPcmBuffers[i]) = Sample;
				self->m_ppPcmBuffers[i]++;
			}
			samplesRead++;

			// Decode the rest of the block
			for (uint32_t s = 1; s < self->m_nSamplesPerBlock; s += SAMPLES_IN_LINE)
			{
				for (uint32_t i = 0; i < self->m_FormatHeader.NumChannels; i++)
				{
					CImaADPCMDecoder_Decode(&self->m_pAdpcmDecoders[i], pAdpcmBuf, self->m_ppPcmBuffers[i], SAMPLES_IN_LINE / 2);
					pAdpcmBuf += SAMPLES_IN_LINE / 2;
					self->m_ppPcmBuffers[i] += SAMPLES_IN_LINE;
				}
				samplesRead += SAMPLES_IN_LINE;
			}
		}
		return OutBufSize;
	}
	return 0;
}

static void CWavFile_Close(void *state)
{
	// Access the plain decoder state owned by the stream
	CWavFile *self = (CWavFile*)state;
	(void)self;
	// Preserve the existing CWavFile Close behavior
	CWavFile_CloseFile(self);
}

static const AudioDecoderOps CWavFile_Ops = {
	CWavFile_Close, CWavFile_IsOpened, CWavFile_FileOpen, CWavFile_GetSampleSize, CWavFile_GetSampleCount, CWavFile_GetSampleRate, CWavFile_GetChannels, CWavFile_Seek, CWavFile_Tell, CWavFile_Decode
};

#ifdef AUDIO_OAL_USE_SNDFILE
typedef struct CSndFile {
    AudioDecoder decoder;
    SNDFILE *m_pfSound;
    SF_INFO m_soundInfo;
} CSndFile;

static void CSndFile_FileOpen(void *state);
static bool CSndFile_IsOpened(void *state);
static uint32_t CSndFile_GetSampleSize(void *state);
static uint32_t CSndFile_GetSampleCount(void *state);
static uint32_t CSndFile_GetSampleRate(void *state);
static uint32_t CSndFile_GetChannels(void *state);
static void CSndFile_Seek(void *state, uint32_t milliseconds);
static uint32_t CSndFile_Tell(void *state);
static uint32_t CSndFile_Decode(void *state, void *buffer);
static void CSndFile_Close(void *state);
static void CSndFile_Init(CSndFile *self, const char *path)
{
	// Initialize format fields before opening resources
	self->m_pfSound = NULL;
	// Preserve the existing CSndFile Init behavior
	memset(&self->m_soundInfo, 0, sizeof(self->m_soundInfo));
	self->m_pfSound = sf_open(path, SFM_READ, &self->m_soundInfo);
}

static void CSndFile_FileOpen(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile FileOpen behavior
	(void)self;
}

static bool CSndFile_IsOpened(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile IsOpened behavior
	return self->m_pfSound != NULL;
}

static uint32_t CSndFile_GetSampleSize(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile GetSampleSize behavior
	return sizeof(uint16_t);
}

static uint32_t CSndFile_GetSampleCount(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile GetSampleCount behavior
	return self->m_soundInfo.frames;
}

static uint32_t CSndFile_GetSampleRate(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile GetSampleRate behavior
	return self->m_soundInfo.samplerate;
}

static uint32_t CSndFile_GetChannels(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile GetChannels behavior
	return self->m_soundInfo.channels;
}

static void CSndFile_Seek(void *state, uint32_t milliseconds)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile Seek behavior
	if ( !CSndFile_IsOpened(self) ) return;
	sf_seek(self->m_pfSound, AudioDecoder_MillisecondsToSamples(&self->decoder, milliseconds), SF_SEEK_SET);
}

static uint32_t CSndFile_Tell(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile Tell behavior
	if ( !CSndFile_IsOpened(self) ) return 0;
	return AudioDecoder_SamplesToMilliseconds(&self->decoder, sf_seek(self->m_pfSound, 0, SF_SEEK_CUR));
}

static uint32_t CSndFile_Decode(void *state, void *buffer)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile Decode behavior
	if ( !CSndFile_IsOpened(self) ) return 0;

	size_t size = sf_read_short(self->m_pfSound, (short*)buffer, AudioDecoder_GetBufferSamples(&self->decoder)) * CSndFile_GetSampleSize(self);
	if (CSndFile_GetChannels(self)==2)
		CSortStereoBuffer_SortStereo(&SortStereoBuffer, buffer, size);
	return size;
}

static void CSndFile_Close(void *state)
{
	// Access the plain decoder state owned by the stream
	CSndFile *self = (CSndFile*)state;
	(void)self;
	// Preserve the existing CSndFile Close behavior
	if ( self->m_pfSound )
	{
		sf_close(self->m_pfSound);
		self->m_pfSound = NULL;
	}
}

static const AudioDecoderOps CSndFile_Ops = {
	CSndFile_Close, CSndFile_IsOpened, CSndFile_FileOpen, CSndFile_GetSampleSize, CSndFile_GetSampleCount, CSndFile_GetSampleRate, CSndFile_GetChannels, CSndFile_Seek, CSndFile_Tell, CSndFile_Decode
};

#endif

#ifdef AUDIO_OAL_USE_MPG123
typedef struct CMP3File {
    AudioDecoder decoder;
    mpg123_handle *m_pMH;
    bool m_bOpened;
    uint32_t m_nRate, m_nChannels;
    const char *m_pPath;
    bool m_bFileNotOpenedYet;
} CMP3File;
typedef CMP3File CADFFile;

static void CMP3File_FileOpen(void *state);
static bool CMP3File_IsOpened(void *state);
static uint32_t CMP3File_GetSampleSize(void *state);
static uint32_t CMP3File_GetSampleCount(void *state);
static uint32_t CMP3File_GetSampleRate(void *state);
static uint32_t CMP3File_GetChannels(void *state);
static void CMP3File_Seek(void *state, uint32_t milliseconds);
static uint32_t CMP3File_Tell(void *state);
static uint32_t CMP3File_Decode(void *state, void *buffer);
static void CMP3File_Close(void *state);
static void CMP3File_Init(CMP3File *self, const char *path)
{
	// Initialize format fields before opening resources
	self->m_pMH = NULL;
	self->m_bOpened = false;
	self->m_nRate = self->m_nChannels = 0;
	self->m_pPath = path;
	self->m_bFileNotOpenedYet = false;
	// Preserve the existing CMP3File Init behavior
	self->m_pMH = mpg123_new(NULL, NULL);
	if ( self->m_pMH )
	{
		mpg123_param(self->m_pMH, MPG123_FLAGS, MPG123_SEEKBUFFER | MPG123_GAPLESS, 0.0);

		self->m_bOpened = true;
		self->m_bFileNotOpenedYet = true;
		// It's possible to move this to audioFileOpsThread(), but effect isn't noticable + probably not compatible with our current cutscene audio handling
#if 1
		CMP3File_FileOpen(self);
#endif
	}
}

static void CMP3File_FileOpen(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File FileOpen behavior
	if(!self->m_bFileNotOpenedYet) return;

	long rate = 0;
	int channels = 0;
	int encoding = 0;
	self->m_bOpened = mpg123_open(self->m_pMH, self->m_pPath) == MPG123_OK
	        && mpg123_getformat(self->m_pMH, &rate, &channels, &encoding) == MPG123_OK;

	self->m_nRate = rate;
	self->m_nChannels = channels;

	if(CMP3File_IsOpened(self)) {
		mpg123_format_none(self->m_pMH);
		mpg123_format(self->m_pMH, rate, channels, encoding);
	}
	self->m_bFileNotOpenedYet = false;
}

static bool CMP3File_IsOpened(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File IsOpened behavior
	return self->m_bOpened;
}

static uint32_t CMP3File_GetSampleSize(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File GetSampleSize behavior
	return sizeof(uint16_t);
}

static uint32_t CMP3File_GetSampleCount(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File GetSampleCount behavior
	if ( !CMP3File_IsOpened(self) || self->m_bFileNotOpenedYet ) return 0;
	return mpg123_length(self->m_pMH);
}

static uint32_t CMP3File_GetSampleRate(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File GetSampleRate behavior
	return self->m_nRate;
}

static uint32_t CMP3File_GetChannels(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File GetChannels behavior
	return self->m_nChannels;
}

static void CMP3File_Seek(void *state, uint32_t milliseconds)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File Seek behavior
	if ( !CMP3File_IsOpened(self) || self->m_bFileNotOpenedYet ) return;
	mpg123_seek(self->m_pMH, AudioDecoder_MillisecondsToSamples(&self->decoder, milliseconds), SEEK_SET);
}

static uint32_t CMP3File_Tell(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File Tell behavior
	if ( !CMP3File_IsOpened(self) || self->m_bFileNotOpenedYet ) return 0;
	return AudioDecoder_SamplesToMilliseconds(&self->decoder, mpg123_tell(self->m_pMH));
}

static uint32_t CMP3File_Decode(void *state, void *buffer)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File Decode behavior
	if ( !CMP3File_IsOpened(self) || self->m_bFileNotOpenedYet ) return 0;
	
	size_t size;
	int err = mpg123_read(self->m_pMH, (unsigned char *)buffer, AudioDecoder_GetBufferSize(&self->decoder), &size);
#if defined(__LP64__) || defined(_WIN64)
	assert("We can't handle audio files more then 2 GB yet :shrug:" && (size < UINT32_MAX));
#endif
	if (err != MPG123_OK && err != MPG123_DONE) return 0;
	if (CMP3File_GetChannels(self) == 2)
		CSortStereoBuffer_SortStereo(&SortStereoBuffer, buffer, size);
	return (uint32_t)size;
}

static void CMP3File_Close(void *state)
{
	// Access the plain decoder state owned by the stream
	CMP3File *self = (CMP3File*)state;
	(void)self;
	// Preserve the existing CMP3File Close behavior
	if ( self->m_pMH )
	{
		mpg123_close(self->m_pMH);
		mpg123_delete(self->m_pMH);
		self->m_pMH = NULL;
	}
}

static const AudioDecoderOps CMP3File_Ops = {
	CMP3File_Close, CMP3File_IsOpened, CMP3File_FileOpen, CMP3File_GetSampleSize, CMP3File_GetSampleCount, CMP3File_GetSampleRate, CMP3File_GetChannels, CMP3File_Seek, CMP3File_Tell, CMP3File_Decode
};

#endif

#ifdef AUDIO_OAL_USE_MPG123


static void CADFFile_FileOpen(void *state);
static ssize_t CADFFile_read(void *fh, void *buf, size_t size)
{
	// Preserve the existing CADFFile r_read behavior
	size_t bytesRead = fread(buf, 1, size, (FILE*)fh);
	uint8_t* _buf = (uint8_t*)buf;
	for (size_t i = 0; i < size; i++)
		_buf[i] ^= 0x22;
	return bytesRead;
}

static off_t CADFFile_seek(void *fh, off_t pos, int seekType)
{
	// Preserve the existing CADFFile r_seek behavior
	fseek((FILE*)fh, pos, seekType);
	return ftell((FILE*)fh);
}

static void CADFFile_close(void *fh)
{
	// Preserve the existing CADFFile r_close behavior
	fclose((FILE*)fh);
}

static void CADFFile_Init(CADFFile *self, const char *path)
{
	// Initialize format fields before opening resources
	self->m_pMH = NULL;
	self->m_bOpened = false;
	self->m_nRate = self->m_nChannels = 0;
	self->m_pPath = path;
	self->m_bFileNotOpenedYet = false;
	// Preserve the existing CADFFile Init behavior
	self->m_pMH = mpg123_new(NULL, NULL);
	if (self->m_pMH)
	{
		mpg123_param(self->m_pMH, MPG123_FLAGS, MPG123_SEEKBUFFER | MPG123_GAPLESS, 0.0);

		self->m_bOpened = true;
		self->m_bFileNotOpenedYet = true;
		self->m_pPath = path;
		// It's possible to move this to audioFileOpsThread(), but effect isn't noticable + probably not compatible with our current cutscene audio handling
#if 1
		CADFFile_FileOpen(self);
#endif

	}
}

static void CADFFile_FileOpen(void *state)
{
	// Access the plain decoder state owned by the stream
	CADFFile *self = (CADFFile*)state;
	(void)self;
	// Preserve the existing CADFFile FileOpen behavior
	if(!self->m_bFileNotOpenedYet) return;

	long rate = 0;
	int channels = 0;
	int encoding = 0;

	FILE *f = fopen(self->m_pPath, "rb");

	self->m_bOpened = f && mpg123_replace_reader_handle(self->m_pMH, CADFFile_read, CADFFile_seek, CADFFile_close) == MPG123_OK
		&& mpg123_open_handle(self->m_pMH, f) == MPG123_OK && mpg123_getformat(self->m_pMH, &rate, &channels, &encoding) == MPG123_OK;

	self->m_nRate = rate;
	self->m_nChannels = channels;

	if(CMP3File_IsOpened(self)) {
		mpg123_format_none(self->m_pMH);
		mpg123_format(self->m_pMH, rate, channels, encoding);
	}

	self->m_bFileNotOpenedYet = false;
}

static const AudioDecoderOps CADFFile_Ops = {
	CMP3File_Close, CMP3File_IsOpened, CADFFile_FileOpen, CMP3File_GetSampleSize, CMP3File_GetSampleCount, CMP3File_GetSampleRate, CMP3File_GetChannels, CMP3File_Seek, CMP3File_Tell, CMP3File_Decode
};

#endif

#define VAG_LINE_SIZE (0x10)
#define VAG_SAMPLES_IN_LINE (28)
#define VB_BLOCK_SIZE (0x2000)
#define NUM_VAG_LINES_IN_BLOCK (VB_BLOCK_SIZE / VAG_LINE_SIZE)
#define NUM_VAG_SAMPLES_IN_BLOCK (NUM_VAG_LINES_IN_BLOCK * VAG_SAMPLES_IN_LINE)
typedef struct CVbFile {
    AudioDecoder decoder;
    FILE *m_pFile;
    CVagDecoder *m_pVagDecoders;
    size_t m_FileSize, m_nNumberOfBlocks;
    uint32_t m_nSampleRate;
    uint8_t m_nChannels;
    bool m_bBlockRead;
    uint16_t m_LineInBlock;
    size_t m_CurrentBlock;
    uint8_t **m_ppVagBuffers;
    int16_t **m_ppPcmBuffers;
} CVbFile;

static void CVbFile_ReadBlock(void *state, int32_t block);
static void CVbFile_FileOpen(void *state);
static bool CVbFile_IsOpened(void *state);
static uint32_t CVbFile_GetSampleSize(void *state);
static uint32_t CVbFile_GetSampleCount(void *state);
static uint32_t CVbFile_GetSampleRate(void *state);
static uint32_t CVbFile_GetChannels(void *state);
static void CVbFile_Seek(void *state, uint32_t milliseconds);
static uint32_t CVbFile_Tell(void *state);
static uint32_t CVbFile_Decode(void *state, void *buffer);
static void CVbFile_Close(void *state);
static void CVbFile_Init(CVbFile *self, const char *path, uint32_t nSampleRate, uint8_t nChannels)
{
	// Initialize format fields before opening resources
	self->m_pFile = NULL;
	self->m_nSampleRate = nSampleRate;
	self->m_nChannels = nChannels;
	self->m_pVagDecoders = NULL;
	self->m_ppVagBuffers = NULL;
	self->m_ppPcmBuffers = NULL;
	self->m_FileSize = self->m_nNumberOfBlocks = self->m_CurrentBlock = 0;
	self->m_bBlockRead = false;
	self->m_LineInBlock = 0;
	// Preserve the existing CVbFile Init behavior
	self->m_pFile = fopen(path, "rb");
	if (!self->m_pFile) return;

	fseek(self->m_pFile, 0, SEEK_END);
	self->m_FileSize = ftell(self->m_pFile);
	fseek(self->m_pFile, 0, SEEK_SET);

	self->m_nNumberOfBlocks = self->m_FileSize / (nChannels * VB_BLOCK_SIZE);
	// Allocate and initialize the VAG decoder state array
	self->m_pVagDecoders = (CVagDecoder*)malloc(sizeof(CVagDecoder)*nChannels);
	for(uint8_t i = 0; i < nChannels; i++)
		CVagDecoder_ResetState(&self->m_pVagDecoders[i]);
	self->m_ppVagBuffers = (uint8_t**)malloc(sizeof(uint8_t*)*nChannels);
	self->m_ppPcmBuffers = (int16_t**)malloc(sizeof(int16_t*)*nChannels);
	for (uint8_t i = 0; i < nChannels; i++)
		self->m_ppVagBuffers[i] = (uint8_t*)malloc(VB_BLOCK_SIZE);
}

static void CVbFile_ReadBlock(void *state, int32_t block)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile ReadBlock behavior
	// Just read next block if -1
	if (block != -1)
		fseek(self->m_pFile, block * self->m_nChannels * VB_BLOCK_SIZE, SEEK_SET);

	for (int i = 0; i < self->m_nChannels; i++)
		fread(self->m_ppVagBuffers[i], VB_BLOCK_SIZE, 1, self->m_pFile);
	self->m_bBlockRead = true;
}

static void CVbFile_FileOpen(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile FileOpen behavior
	(void)self;
}

static bool CVbFile_IsOpened(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile IsOpened behavior
	return self->m_pFile != NULL;
}

static uint32_t CVbFile_GetSampleSize(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile GetSampleSize behavior
	return sizeof(uint16_t);
}

static uint32_t CVbFile_GetSampleCount(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile GetSampleCount behavior
	if (!CVbFile_IsOpened(self)) return 0;
	return self->m_nNumberOfBlocks * NUM_VAG_LINES_IN_BLOCK * VAG_SAMPLES_IN_LINE;
}

static uint32_t CVbFile_GetSampleRate(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile GetSampleRate behavior
	return self->m_nSampleRate;
}

static uint32_t CVbFile_GetChannels(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile GetChannels behavior
	return self->m_nChannels;
}

static void CVbFile_Seek(void *state, uint32_t milliseconds)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile Seek behavior
	if (!CVbFile_IsOpened(self)) return;
	uint32_t samples = AudioDecoder_MillisecondsToSamples(&self->decoder, milliseconds);

	// Find the block of our sample
	uint32_t block = samples / NUM_VAG_SAMPLES_IN_BLOCK;
	if (block > self->m_nNumberOfBlocks)
	{
		samples = 0;
		block = 0;
	}
	if (block != self->m_CurrentBlock)
		self->m_bBlockRead = false;

	// Find a line of our sample within our block
	uint32_t remainingSamples = samples - block * NUM_VAG_SAMPLES_IN_BLOCK;
	uint32_t newLine = remainingSamples / VAG_SAMPLES_IN_LINE / VAG_LINE_SIZE;

	if (self->m_CurrentBlock != block || self->m_LineInBlock != newLine)
	{
		self->m_CurrentBlock = block;
		self->m_LineInBlock = newLine;
		for (uint32_t i = 0; i < CVbFile_GetChannels(self); i++)
			CVagDecoder_ResetState(&self->m_pVagDecoders[i]);
	}
}

static uint32_t CVbFile_Tell(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile Tell behavior
	if (!CVbFile_IsOpened(self)) return 0;
	uint32_t pos = (self->m_CurrentBlock * NUM_VAG_LINES_IN_BLOCK + self->m_LineInBlock) * VAG_SAMPLES_IN_LINE;
	return AudioDecoder_SamplesToMilliseconds(&self->decoder, pos);
}

static uint32_t CVbFile_Decode(void *state, void *buffer)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile Decode behavior
	if (!CVbFile_IsOpened(self)) return 0;

	if (self->m_CurrentBlock >= self->m_nNumberOfBlocks) return 0;

	// Cache current ADPCM block
	if (!self->m_bBlockRead)
		CVbFile_ReadBlock(self, self->m_CurrentBlock);

	// Trim the buffer size if we're at the end of our file
	int numberOfRequiredLines = AudioDecoder_GetBufferSamples(&self->decoder) / self->m_nChannels / VAG_SAMPLES_IN_LINE;
	int numberOfRemainingLines = (self->m_nNumberOfBlocks - self->m_CurrentBlock) * NUM_VAG_LINES_IN_BLOCK - self->m_LineInBlock;
	int bufSizePerChannel = AUDIO_FORMAT_MIN(numberOfRequiredLines, numberOfRemainingLines) * VAG_SAMPLES_IN_LINE * CVbFile_GetSampleSize(self);

	// Calculate the pointers to individual channel buffers
	for (uint32_t i = 0; i < self->m_nChannels; i++)
		self->m_ppPcmBuffers[i] = (int16_t*)((int8_t*)buffer + bufSizePerChannel * i);

	int size = 0;
	while (size < bufSizePerChannel)
	{
		// Decode the VAG lines
		for (uint32_t i = 0; i < self->m_nChannels; i++)
		{
			CVagDecoder_Decode(&self->m_pVagDecoders[i], self->m_ppVagBuffers[i] + self->m_LineInBlock * VAG_LINE_SIZE, self->m_ppPcmBuffers[i], VAG_LINE_SIZE);
			self->m_ppPcmBuffers[i] += VAG_SAMPLES_IN_LINE;
		}
		size += VAG_SAMPLES_IN_LINE * CVbFile_GetSampleSize(self);
		self->m_LineInBlock++;

		// Block is over, read the next block
		if (self->m_LineInBlock >= NUM_VAG_LINES_IN_BLOCK)
		{
			self->m_CurrentBlock++;
			if (self->m_CurrentBlock >= self->m_nNumberOfBlocks) // end of file
				break;
			self->m_LineInBlock = 0;
			CVbFile_ReadBlock(self, -1);
		}
	}

	return bufSizePerChannel * self->m_nChannels;
}

static void CVbFile_Close(void *state)
{
	// Access the plain decoder state owned by the stream
	CVbFile *self = (CVbFile*)state;
	(void)self;
	// Preserve the existing CVbFile Close behavior
	if (self->m_pFile)
	{
		fclose(self->m_pFile);

		// Release the VAG decoder state array
		// Release plain VAG states without invoking destructors
		free(self->m_pVagDecoders);
		for (int i = 0; i < self->m_nChannels; i++)
			free(self->m_ppVagBuffers[i]);
		free(self->m_ppVagBuffers);
		free(self->m_ppPcmBuffers);
	}
}

static const AudioDecoderOps CVbFile_Ops = {
	CVbFile_Close, CVbFile_IsOpened, CVbFile_FileOpen, CVbFile_GetSampleSize, CVbFile_GetSampleCount, CVbFile_GetSampleRate, CVbFile_GetChannels, CVbFile_Seek, CVbFile_Tell, CVbFile_Decode
};

#ifdef AUDIO_OAL_USE_OPUS
typedef struct COpusFile {
    AudioDecoder decoder;
    OggOpusFile *m_FileH;
    bool m_bOpened;
    uint32_t m_nRate, m_nChannels;
} COpusFile;

static void COpusFile_FileOpen(void *state);
static bool COpusFile_IsOpened(void *state);
static uint32_t COpusFile_GetSampleSize(void *state);
static uint32_t COpusFile_GetSampleCount(void *state);
static uint32_t COpusFile_GetSampleRate(void *state);
static uint32_t COpusFile_GetChannels(void *state);
static void COpusFile_Seek(void *state, uint32_t milliseconds);
static uint32_t COpusFile_Tell(void *state);
static uint32_t COpusFile_Decode(void *state, void *buffer);
static void COpusFile_Close(void *state);
static void COpusFile_Init(COpusFile *self, const char *path)
{
	// Initialize format fields before opening resources
	self->m_FileH = NULL;
	self->m_bOpened = false;
	self->m_nRate = self->m_nChannels = 0;
	// Preserve the existing COpusFile Init behavior
	int ret;
	self->m_FileH = op_open_file(path, &ret);

	if (self->m_FileH) {
		self->m_nChannels = op_head(self->m_FileH, 0)->channel_count;
		self->m_nRate = 48000;
		const OpusTags *tags = op_tags(self->m_FileH, 0);
		for (int i = 0; i < tags->comments; i++) {
			if (strncmp(tags->user_comments[i], "SAMPLERATE", sizeof("SAMPLERATE")-1) == 0)
			{
				int parsedRate;
				if(sscanf(tags->user_comments[i], "SAMPLERATE=%i", &parsedRate) == 1)
					self->m_nRate = (uint32_t)parsedRate;
				break;
			}
		}
		
		self->m_bOpened = true;
	}
}

static void COpusFile_FileOpen(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile FileOpen behavior
	(void)self;
}

static bool COpusFile_IsOpened(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile IsOpened behavior
	return self->m_bOpened;
}

static uint32_t COpusFile_GetSampleSize(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile GetSampleSize behavior
	return sizeof(uint16_t);
}

static uint32_t COpusFile_GetSampleCount(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile GetSampleCount behavior
	if ( !COpusFile_IsOpened(self) ) return 0;
	return op_pcm_total(self->m_FileH, 0);
}

static uint32_t COpusFile_GetSampleRate(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile GetSampleRate behavior
	return self->m_nRate;
}

static uint32_t COpusFile_GetChannels(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile GetChannels behavior
	return self->m_nChannels;
}

static void COpusFile_Seek(void *state, uint32_t milliseconds)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile Seek behavior
	if ( !COpusFile_IsOpened(self) ) return;
	op_pcm_seek(self->m_FileH, AudioDecoder_MillisecondsToSamples(&self->decoder, milliseconds) / COpusFile_GetChannels(self));
}

static uint32_t COpusFile_Tell(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile Tell behavior
	if ( !COpusFile_IsOpened(self) ) return 0;
	return AudioDecoder_SamplesToMilliseconds(&self->decoder, op_pcm_tell(self->m_FileH) * COpusFile_GetChannels(self));
}

static uint32_t COpusFile_Decode(void *state, void *buffer)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile Decode behavior
	if ( !COpusFile_IsOpened(self) ) return 0;

	int size = op_read(self->m_FileH, (opus_int16 *)buffer, AudioDecoder_GetBufferSamples(&self->decoder), NULL);

	if (size < 0)
		return 0;

	if (COpusFile_GetChannels(self) == 2)
		CSortStereoBuffer_SortStereo(&SortStereoBuffer, buffer, size * self->m_nChannels * COpusFile_GetSampleSize(self));

	return size * self->m_nChannels * COpusFile_GetSampleSize(self);
}

static void COpusFile_Close(void *state)
{
	// Access the plain decoder state owned by the stream
	COpusFile *self = (COpusFile*)state;
	(void)self;
	// Preserve the existing COpusFile Close behavior
	if (self->m_FileH)
	{
		op_free(self->m_FileH);
		self->m_FileH = NULL;
	}
}

static const AudioDecoderOps COpusFile_Ops = {
	COpusFile_Close, COpusFile_IsOpened, COpusFile_FileOpen, COpusFile_GetSampleSize, COpusFile_GetSampleCount, COpusFile_GetSampleRate, COpusFile_GetChannels, COpusFile_Seek, COpusFile_Tell, COpusFile_Decode
};

#endif

#undef AUDIO_FORMAT_MIN
#ifdef AUDIO_FORMAT_DEFAULT_DEBUG
#undef AUDIO_FORMAT_DEBUG
#undef AUDIO_FORMAT_DEFAULT_DEBUG
#endif
//- rouz edit (ChatGPT)
