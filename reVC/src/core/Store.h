#pragma once

//+ rouz edit (ChatGPT)
#ifndef assert
#include <assert.h>
#endif
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef struct CStore {
	int32_t allocPtr;
	void *entries;
	size_t entrySize;
	int32_t capacity;
} CStore;

static inline void *
CStore_Alloc(CStore *store)
{
	// Reserve the next entry without constructing or resetting its contents
	if (store->allocPtr >= store->capacity) {
		// Preserve the fixed-store capacity diagnostic
		printf("Size of this thing:%d needs increasing\n", store->capacity);
		assert(0);
	}
	// Advance the allocation cursor by one physical entry
	return (uint8_t*)store->entries + store->entrySize * store->allocPtr++;
}

static inline void
CStore_Clear(CStore *store)
{
	// Reuse the existing entries without changing their object lifetimes
	store->allocPtr = 0;
}

static inline int32_t
CStore_GetIndex(const CStore *store, const void *item)
{
	// Validate the entry's range and convert its byte offset to a store index
	uintptr_t start = (uintptr_t)store->entries;
	uintptr_t address = (uintptr_t)item;
	assert(address >= start);
	assert(address < start + store->entrySize * store->capacity);
	return (int32_t)((address - start) / store->entrySize);
}

static inline void *
CStore_GetItem(const CStore *store, int32_t index)
{
	// Resolve an index within the fixed storage capacity
	assert(index >= 0);
	assert(index < store->capacity);
	return (uint8_t*)store->entries + store->entrySize * index;
}
//- rouz edit (ChatGPT)
