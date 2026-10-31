//+ rouz edit (ChatGPT)
#include "StreamingInfo.h"
#include <stddef.h>

bool
StreamingInfo_GetCdPosnAndSize(CStreamingInfo *self, uint32_t *posn, uint32_t *size)
{
	// Preserve the original record operation and its ordered writes
	if(self->m_size == 0)
		return false;
	*posn = self->m_position;
	*size = self->m_size;
	return true;
}

void
StreamingInfo_SetCdPosnAndSize(CStreamingInfo *self, uint32_t posn, uint32_t size)
{
	// Preserve the original record operation and its ordered writes
	self->m_position = posn;
	self->m_size = size;
}

void
StreamingInfo_AddToList(CStreamingInfo *self, CStreamingInfo *link)
{
	// Preserve the original record operation and its ordered writes
	// Insert self after link
	self->m_next = link->m_next;
	self->m_prev = link;
	link->m_next = self;
	self->m_next->m_prev = self;
}

void
StreamingInfo_RemoveFromList(CStreamingInfo *self)
{
	// Preserve the original record operation and its ordered writes
	self->m_next->m_prev = self->m_prev;
	self->m_prev->m_next = self->m_next;
	self->m_next = NULL;
	self->m_prev = NULL;
}

uint32_t
StreamingInfo_GetCdSize(CStreamingInfo *self)
{
    // Return the record's stored sector count without changing its state
    return self->m_size;
}

bool
StreamingInfo_IsPriority(CStreamingInfo *self)
{
    // Test only the original priority flag bit
    return !!(self->m_flags & STREAMFLAGS_PRIORITY);
}
//- rouz edit (ChatGPT)
