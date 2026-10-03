#pragma once

//+ rouz edit (ChatGPT)
#ifndef assert
#include <assert.h>
#endif
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef VALIDATE_SAVE_SIZE
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t _saveBufCount;
#ifdef __cplusplus
}
#endif
#define INITSAVEBUF _saveBufCount = 0;
#define VALIDATESAVEBUF(b) assert(_saveBufCount == b);
#else
#define INITSAVEBUF
#define VALIDATESAVEBUF(b)
#endif

static inline void
SkipSaveBuf(uint8_t **buf, int32_t skip)
{
	// Advance the caller's cursor and preserve the optional validation count
	*buf += skip;
#ifdef VALIDATE_SAVE_SIZE
	_saveBufCount += skip;
#endif
}

static inline void
SkipSaveBufWithLength(uint8_t **buf, uint32_t *length, int32_t skip)
{
	// Track the skipped bytes as well as advancing the caller's cursor
	SkipSaveBuf(buf, skip);
	*length += skip;
}

static inline void
ReadSaveBuf(void *out, uint8_t **buf, size_t size)
{
	// Copy a serialized value without requiring buffer alignment
	memcpy(out, *buf, size);
	SkipSaveBuf(buf, (int32_t)size);
}

static inline void
ReadSaveBufWithLength(void *out, uint8_t **buf, uint32_t *length, size_t size)
{
	// Read the value and accumulate its serialized size
	ReadSaveBuf(out, buf, size);
	*length += (uint32_t)size;
}

static inline void *
WriteSaveBuf(uint8_t **buf, const void *value, size_t size)
{
	// Return the saved value's address for callers that patch stored pointers
	void *saved = *buf;
	memcpy(saved, value, size);
	SkipSaveBuf(buf, (int32_t)size);
	return saved;
}

static inline void *
WriteSaveBufWithLength(uint8_t **buf, uint32_t *length, const void *value, size_t size)
{
	// Write the value and accumulate its serialized size
	void *saved = WriteSaveBuf(buf, value, size);
	*length += (uint32_t)size;
	return saved;
}

#ifdef COMPATIBLE_SAVES
static inline void
ZeroSaveBuf(uint8_t **buf, uint32_t length)
{
	// Clear compatibility padding and advance past it
	memset(*buf, 0, length);
	SkipSaveBuf(buf, (int32_t)length);
}
#endif

#define SAVE_HEADER_SIZE (4*sizeof(char)+sizeof(uint32_t))

static inline void
WriteSaveHeader(uint8_t **buf, char a, char b, char c, char d, uint32_t size)
{
	// Write four single-byte identifiers followed by the payload size
	const char magic[4] = { a, b, c, d };
	WriteSaveBuf(buf, magic, sizeof(magic));
	WriteSaveBuf(buf, &size, sizeof(size));
}

static inline void
WriteSaveHeaderWithLength(uint8_t **buf, uint32_t *length, char a, char b, char c, char d, uint32_t size)
{
	// Include the header bytes in the accumulated block length
	WriteSaveHeader(buf, a, b, c, d, size);
	*length += (uint32_t)SAVE_HEADER_SIZE;
}

static inline void
CheckSaveHeader(uint8_t **buf, char a, char b, char c, char d, uint32_t size)
{
#ifdef VALIDATE_SAVE_SIZE
	// Validate each identifier and the payload size in their saved order
	const char magic[4] = { a, b, c, d };
	char actual;
	uint32_t actualSize;
	for (size_t i = 0; i < sizeof(magic); i++) {
		// Read the next header byte before comparing its identifier
		ReadSaveBuf(&actual, buf, sizeof(actual));
		assert(actual == magic[i]);
	}
	// Check the serialized payload length after the identifiers
	ReadSaveBuf(&actualSize, buf, sizeof(actualSize));
	assert(actualSize == size);
#else
	// Preserve the unchecked header skip used by release saves
	(void)a;
	(void)b;
	(void)c;
	(void)d;
	(void)size;
	SkipSaveBuf(buf, (int32_t)SAVE_HEADER_SIZE);
#endif
}

static inline void
CheckSaveHeaderWithLength(uint8_t **buf, uint32_t *length, char a, char b, char c, char d, uint32_t size)
{
	// Check the header and preserve the original conditional length tracking
	CheckSaveHeader(buf, a, b, c, d, size);
#ifdef VALIDATE_SAVE_SIZE
	// Count checked header bytes just as the original read overload did
	*length += (uint32_t)SAVE_HEADER_SIZE;
#else
	// Leave the unchecked length unchanged as in the original release macro
	(void)length;
#endif
}
//- rouz edit (ChatGPT)
