#pragma once

//+ rouz edit (ChatGPT)
#ifndef assert
#include <assert.h>
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define POOLFLAG_ID     0x7f
#define POOLFLAG_ISFREE 0x80

typedef struct CPool {
	uint8_t *m_entries;
	uint8_t *m_flags;
	int32_t m_size;
	int32_t m_allocPtr;
	size_t m_entrySize;
} CPool;

typedef struct CPoolSnapshot {
	uint8_t *flags;
	void *entries;
} CPoolSnapshot;

static inline int
CPool_Init(CPool *pool, int32_t size, size_t entrySize, const char *name)
{
	// Allocate raw slots using the physical storage type's size
	(void)name;
	assert(size >= 0);
	assert(entrySize > 0);
	pool->m_entries = size > 0 ? (uint8_t*)malloc(entrySize * size) : NULL;
	pool->m_flags = size > 0 ? (uint8_t*)malloc((size_t)size) : NULL;
	pool->m_size = size;
	pool->m_allocPtr = -1;
	pool->m_entrySize = entrySize;
	if (size > 0 && (!pool->m_entries || !pool->m_flags)) {
		// Release a partial allocation before reporting initialization failure
		free(pool->m_entries);
		free(pool->m_flags);
		pool->m_entries = NULL;
		pool->m_flags = NULL;
		pool->m_size = 0;
		return 0;
	}
	if (size > 0) {
		// Start with generation zero and every slot marked free
		memset(pool->m_flags, POOLFLAG_ISFREE, (size_t)size);
	}
	// Report successful initialization without constructing any entry objects
	return 1;
}

static inline CPool *
CPool_Create(int32_t size, size_t entrySize, const char *name)
{
	// Allocate the pool manager separately from its raw entry storage
	CPool *pool = (CPool*)malloc(sizeof(CPool));
	if (pool && !CPool_Init(pool, size, entrySize, name)) {
		// Release the manager when its storage could not be initialized
		free(pool);
		return NULL;
	}
	// Return the initialized manager or the allocation failure
	return pool;
}

static inline void
CPool_Flush(CPool *pool)
{
	// Release storage without destroying the objects held in its slots
	free(pool->m_entries);
	free(pool->m_flags);
	pool->m_entries = NULL;
	pool->m_flags = NULL;
	pool->m_size = 0;
	pool->m_allocPtr = 0;
}

static inline void
CPool_Destroy(CPool *pool)
{
	// Release both the pool's raw storage and its manager
	if (pool) {
		// Keep entry destruction in the existing object owners
		CPool_Flush(pool);
		free(pool);
	}
}

static inline int
CPool_GetId(const CPool *pool, int32_t index)
{
	// Extract the slot's seven-bit generation counter
	return pool->m_flags[index] & POOLFLAG_ID;
}

static inline int
CPool_GetIsFree(const CPool *pool, int32_t index)
{
	// Read the free bit without changing the stored generation
	return !!(pool->m_flags[index] & POOLFLAG_ISFREE);
}

static inline void
CPool_SetId(CPool *pool, int32_t index, int id)
{
	// Replace the generation while preserving the slot's free bit
	pool->m_flags[index] = (uint8_t)((pool->m_flags[index] & POOLFLAG_ISFREE) | (id & POOLFLAG_ID));
}

static inline void
CPool_SetIsFree(CPool *pool, int32_t index, int isFree)
{
	// Replace the free bit while preserving the slot's generation
	if (isFree) {
		// Mark the slot available for allocation
		pool->m_flags[index] |= POOLFLAG_ISFREE;
	} else {
		// Mark the slot occupied
		pool->m_flags[index] &= (uint8_t)~POOLFLAG_ISFREE;
	}
}

static inline int32_t
CPool_GetSize(const CPool *pool)
{
	// Return the fixed number of physical slots
	return pool->m_size;
}

static inline uint32_t
CPool_GetMaxEntrySize(const CPool *pool)
{
	// Expose the physical stride used by snapshot and relocation callers
	return (uint32_t)pool->m_entrySize;
}

static inline void *
CPool_New(CPool *pool)
{
	// Search from the allocation cursor without constructing an entry object
	int wrapped = 0;
	if (pool->m_size <= 0) {
		// Treat an empty or flushed pool as exhausted
		return NULL;
	}
	do {
#ifdef FIX_BUGS
		// Preserve the corrected wrap condition after handle-based allocation
		if (++pool->m_allocPtr >= pool->m_size) {
			// Wrap the cursor and stop after searching a full cycle
			pool->m_allocPtr = 0;
			if (wrapped) {
				// Report exhaustion after the second wrap
				return NULL;
			}
			// Record the first wrap through the pool
			wrapped = 1;
		}
#else
		// Preserve the original wrap condition in builds without the fix
		if (++pool->m_allocPtr == pool->m_size) {
			// Report exhaustion after searching a full cycle
			if (wrapped) {
				// Leave the original exhausted cursor unchanged
				return NULL;
			}
			// Start the second pass from the first physical slot
			wrapped = 1;
			pool->m_allocPtr = 0;
		}
#endif
	} while (!CPool_GetIsFree(pool, pool->m_allocPtr));
	// Occupy the slot and advance its wrapping generation counter
	CPool_SetIsFree(pool, pool->m_allocPtr, 0);
	CPool_SetId(pool, pool->m_allocPtr, CPool_GetId(pool, pool->m_allocPtr) + 1);
	return pool->m_entries + pool->m_entrySize * pool->m_allocPtr;
}

static inline void
CPool_SetNotFreeAt(CPool *pool, int32_t handle)
{
	// Restore the slot and generation encoded in a saved handle
	int32_t index = handle >> 8;
	CPool_SetIsFree(pool, index, 0);
	CPool_SetId(pool, index, handle & POOLFLAG_ID);
	for (pool->m_allocPtr = 0; pool->m_allocPtr < pool->m_size; pool->m_allocPtr++) {
		// Leave the cursor at the first free slot or just past a full pool
		if (CPool_GetIsFree(pool, pool->m_allocPtr)) {
			// Stop searching once an available slot is found
			return;
		}
	}
}

static inline void *
CPool_NewAt(CPool *pool, int32_t handle)
{
	// Reserve a specific saved handle without incrementing its generation
	void *entry = pool->m_entries + pool->m_entrySize * (handle >> 8);
	CPool_SetNotFreeAt(pool, handle);
	return entry;
}

static inline int32_t
CPool_GetJustIndex_NoFreeAssert(const CPool *pool, const void *entry)
{
	// Preserve signed index probes for pointers that may belong to another pool
	intptr_t offset = (intptr_t)((uintptr_t)entry - (uintptr_t)pool->m_entries);
	intptr_t stride = (intptr_t)pool->m_entrySize;
	intptr_t index = offset / stride;
	if (offset < 0 && offset % stride != 0) {
		// Keep a foreign pointer below the allocation from aliasing slot zero
		index--;
	}
	// Return the physical slot index without dereferencing the supplied pointer
	return (int32_t)index;
}

static inline int32_t
CPool_GetJustIndex(const CPool *pool, const void *entry)
{
	// Validate occupied entries after resolving their physical slot index
	int32_t index = CPool_GetJustIndex_NoFreeAssert(pool, entry);
	assert((const uint8_t*)entry == pool->m_entries + pool->m_entrySize * index);
	assert(!CPool_GetIsFree(pool, index));
	return index;
}

static inline int32_t
CPool_GetIndex(const CPool *pool, const void *entry)
{
	// Combine the physical slot index with its full flag byte
	int32_t index = CPool_GetJustIndex_NoFreeAssert(pool, entry);
	return pool->m_flags[index] + (index << 8);
}

static inline void
CPool_Delete(CPool *pool, void *entry)
{
	// Release a slot without destroying its object or changing its generation
	int32_t index = CPool_GetJustIndex(pool, entry);
	CPool_SetIsFree(pool, index, 1);
	if (index < pool->m_allocPtr) {
		// Preserve the original cursor adjustment when an earlier slot is freed
		pool->m_allocPtr = index;
	}
}

static inline void *
CPool_GetSlot(const CPool *pool, int32_t index)
{
	// Resolve an occupied slot using the physical storage stride
	return CPool_GetIsFree(pool, index) ? NULL : pool->m_entries + pool->m_entrySize * index;
}

static inline void *
CPool_GetAt(const CPool *pool, int32_t handle)
{
#ifdef FIX_BUGS
	// Preserve the invalid-handle sentinel used by corrected builds
	if (handle == -1) {
		// Reject the sentinel before accessing the flag array
		return NULL;
	}
#endif
	// Match the complete saved flag byte before returning the slot
	return pool->m_flags[handle >> 8] == (handle & 0xff) ?
	       pool->m_entries + pool->m_entrySize * (handle >> 8) : NULL;
}

static inline int32_t
CPool_GetNoOfUsedSpaces(const CPool *pool)
{
	// Count occupied slots independently of the allocation cursor
	int32_t used = 0;
	for (int32_t i = 0; i < pool->m_size; i++) {
		// Include each slot whose free bit is clear
		if (!CPool_GetIsFree(pool, i)) {
			// Accumulate the occupied slot count
			used++;
		}
	}
	// Return the total number of occupied physical slots
	return used;
}

static inline int32_t
CPool_GetNoOfFreeSpaces(const CPool *pool)
{
	// Derive the available capacity from the occupied slot count
	return pool->m_size - CPool_GetNoOfUsedSpaces(pool);
}

static inline void
CPool_InitSnapshot(CPoolSnapshot *snapshot)
{
	// Initialize the two raw buffers owned by a replay snapshot
	snapshot->flags = NULL;
	snapshot->entries = NULL;
}

static inline void
CPool_ClearStorage(CPoolSnapshot *snapshot)
{
	// Release snapshot bytes without destroying any copied entry objects
	free(snapshot->flags);
	free(snapshot->entries);
	CPool_InitSnapshot(snapshot);
}

static inline void
CPool_Store(const CPool *pool, CPoolSnapshot *snapshot)
{
	// Copy every physical entry and flag into independent replay storage
	size_t bytes = pool->m_entrySize * pool->m_size;
	snapshot->flags = pool->m_size > 0 ? (uint8_t*)malloc((size_t)pool->m_size) : NULL;
	snapshot->entries = bytes > 0 ? malloc(bytes) : NULL;
	if (pool->m_size > 0) {
		// Copy nonempty buffers without passing null pointers to memcpy
		memcpy(snapshot->flags, pool->m_flags, (size_t)pool->m_size);
		memcpy(snapshot->entries, pool->m_entries, bytes);
	}
#ifdef debug
	// Preserve the existing replay storage diagnostic when logging is available
	debug("Stored:%d (/%d)\n", CPool_GetNoOfUsedSpaces(pool), pool->m_size);
#endif
}

static inline void
CPool_CopyBack(CPool *pool, CPoolSnapshot *snapshot)
{
	// Restore entries and generations without replacing the live pool allocation
	size_t bytes = pool->m_entrySize * pool->m_size;
	if (pool->m_size > 0) {
		// Restore nonempty buffers without passing null pointers to memcpy
		memcpy(pool->m_flags, snapshot->flags, (size_t)pool->m_size);
		memcpy(pool->m_entries, snapshot->entries, bytes);
	}
#ifdef debug
	// Preserve the existing snapshot byte-count diagnostic
	debug("Size copied:%zu (%d)\n", bytes, pool->m_size);
#endif
	// Reset the allocation cursor and release the consumed snapshot
	pool->m_allocPtr = 0;
	CPool_ClearStorage(snapshot);
#ifdef debug
	// Preserve the occupied-slot diagnostic after replay restoration
	debug("CopyBack:%d (/%d)\n", CPool_GetNoOfUsedSpaces(pool), pool->m_size);
#endif
}
//- rouz edit (ChatGPT)
