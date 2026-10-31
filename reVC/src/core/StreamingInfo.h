//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
enum StreamFlags
{
	STREAMFLAGS_DONT_REMOVE = 0x01,
	STREAMFLAGS_SCRIPTOWNED = 0x02,
	STREAMFLAGS_DEPENDENCY  = 0x04,	// Is this right?
	STREAMFLAGS_PRIORITY    = 0x08,
	STREAMFLAGS_NOFADE      = 0x10,
	STREAMFLAGS_20          = 0x20,	// TODO(MIAMI): what's this

	STREAMFLAGS_CANT_REMOVE = STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_SCRIPTOWNED,
	STREAMFLAGS_KEEP_IN_MEMORY = STREAMFLAGS_DONT_REMOVE|STREAMFLAGS_SCRIPTOWNED|STREAMFLAGS_DEPENDENCY,
};

enum StreamLoadState
{
	STREAMSTATE_NOTLOADED = 0,
	STREAMSTATE_LOADED    = 1,
	STREAMSTATE_INQUEUE   = 2,
	STREAMSTATE_READING   = 3,	// channel is reading
	STREAMSTATE_STARTED   = 4,	// first part loaded
};

typedef struct CStreamingInfo {
	struct CStreamingInfo *m_next;
	struct CStreamingInfo *m_prev;
	uint8_t m_loadState;
	uint8_t m_flags;

	int16_t  m_nextID;
	uint32_t m_position;
	uint32_t m_size;

} CStreamingInfo;
#ifdef __cplusplus
extern "C" {
#endif
bool StreamingInfo_GetCdPosnAndSize(CStreamingInfo *self,uint32_t *posn,uint32_t *size);
void StreamingInfo_SetCdPosnAndSize(CStreamingInfo *self,uint32_t posn,uint32_t size);
void StreamingInfo_AddToList(CStreamingInfo *self,CStreamingInfo *link);
void StreamingInfo_RemoveFromList(CStreamingInfo *self);
uint32_t StreamingInfo_GetCdSize(CStreamingInfo *self);
bool StreamingInfo_IsPriority(CStreamingInfo *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
