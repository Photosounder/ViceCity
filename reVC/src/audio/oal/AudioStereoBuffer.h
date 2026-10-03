#pragma once

//+ rouz edit (ChatGPT)
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct CSortStereoBuffer {
	uint16_t *PcmBuf;
	size_t BufSize;
} CSortStereoBuffer;

static void CSortStereoBuffer_Close(CSortStereoBuffer *self)
{
	// Release shared stereo scratch storage and allow later initialization
	free(self->PcmBuf);
	self->PcmBuf = NULL;
	self->BufSize = 0;
}

static uint16_t * CSortStereoBuffer_GetBuffer(CSortStereoBuffer *self, size_t size)
{
	// Grow the owned scratch buffer only when the decoded PCM needs more space
	if (size == 0) return NULL;
	if (!self->PcmBuf)
	{
		self->BufSize = size;
		self->PcmBuf = (uint16_t*)malloc(self->BufSize);
	}
	else if (self->BufSize < size)
	{
		self->BufSize = size;
		self->PcmBuf = (uint16_t*)realloc(self->PcmBuf, size);
	}
	return self->PcmBuf;
}

static void CSortStereoBuffer_SortStereo(CSortStereoBuffer *self, void *buf, size_t size)
{
	// Split interleaved stereo PCM into contiguous left and right channel samples
	uint16_t* InBuf = (uint16_t*)buf;
	uint16_t* OutBuf = CSortStereoBuffer_GetBuffer(self, size);

	if (!OutBuf) return;

	size_t rightStart = size / 4;
	for (size_t i = 0; i < size / 4; i++)
	{
		OutBuf[i] = InBuf[i*2];
		OutBuf[i+rightStart] = InBuf[i*2+1];
	}

	memcpy(InBuf, OutBuf, size);
}

//- rouz edit (ChatGPT)
