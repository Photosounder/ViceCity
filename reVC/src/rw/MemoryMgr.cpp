#include "common.h"
#include "MemoryHeap.h"
#include "MemoryMgr.h"


uint8 *pMemoryTop;

void
InitMemoryMgr(void)
{
#ifdef USE_CUSTOM_ALLOCATOR
#ifdef GTA_PS2
#error "finish this"
#else
	// randomly allocate 128mb
	gMainHeap.Init(128*1024*1024);
#endif
#endif
}

#undef MemoryMgrMalloc // rouz edit
void*
MemoryMgrMalloc(size_t size)
{
#ifdef USE_CUSTOM_ALLOCATOR
	void *mem = gMainHeap.Malloc(size);
#else
	void *mem = malloc(size);
#endif
	if((uint8*)mem + size > pMemoryTop)
		pMemoryTop = (uint8*)mem + size ;
	return mem;
}

#undef MemoryMgrRealloc // rouz edit
void*
MemoryMgrRealloc(void *ptr, size_t size)
{
#ifdef USE_CUSTOM_ALLOCATOR
	void *mem = gMainHeap.Realloc(ptr, size);
#else
	void *mem = realloc(ptr, size);
#endif
	if((uint8*)mem + size  > pMemoryTop)
		pMemoryTop = (uint8*)mem + size ;
	return mem;
}

#undef MemoryMgrCalloc // rouz edit
void*
MemoryMgrCalloc(size_t num, size_t size)
{
#ifdef USE_CUSTOM_ALLOCATOR
	void *mem = gMainHeap.Malloc(num*size);
#else
	void *mem = calloc(num, size);
#endif
	if((uint8*)mem + size  > pMemoryTop)
		pMemoryTop = (uint8*)mem + size ;
#ifdef FIX_BUGS
	memset(mem, 0, num*size);
#endif
	return mem;
}

#undef MemoryMgrFree // rouz edit
void
MemoryMgrFree(void *ptr)
{
#ifdef USE_CUSTOM_ALLOCATOR
#ifdef FIX_BUGS
	// i don't suppose this is handled by RW?
	if(ptr == nil) return;
#endif
	gMainHeap.Free(ptr);
#else
	free(ptr);
#endif
}

// rouz edit
RwMemoryFunctions memFuncs = {
	MemoryMgrMalloc,
	MemoryMgrFree,
	MemoryMgrRealloc,
	MemoryMgrCalloc
};

void *
RwMallocAlign(RwUInt32 size, RwUInt32 align)
{
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
	uintptr ptralign = align-1;
	void *mem = (void *)MemoryMgrMalloc(size + sizeof(uintptr) + ptralign);

	ASSERT(mem != nil);

	void *addr = (void *)((((uintptr)mem) + sizeof(uintptr) + ptralign) & ~ptralign);

	ASSERT(addr != nil);
#else
	void *mem = (void *)MemoryMgrMalloc(size + align);

	ASSERT(mem != nil);

	void *addr = (void *)((((uintptr)mem) + align) & ~(align - 1));

	ASSERT(addr != nil);
#endif

	*(((void **)addr) - 1) = mem;

	return addr;
}

void
RwFreeAlign(void *mem)
{
	ASSERT(mem != nil);

	void *addr = *(((void **)mem) - 1);

	ASSERT(addr != nil);

	MemoryMgrFree(addr);
}
