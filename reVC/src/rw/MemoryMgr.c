//+ rouz edit (ChatGPT)
#include "config.h"
#include "MemoryMgr.h"
#include "MemoryMgr_backend.h"
#include <stdlib.h>
#include <string.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#ifndef MASTER
#define ASSERT(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define ASSERT(expression) (expression)
#endif


uint8_t *pMemoryTop;

void
InitMemoryMgr(void)
{
    // Initialize the original custom heap size only under its existing platform guards
#ifdef USE_CUSTOM_ALLOCATOR
#ifdef GTA_PS2
#error "finish this"
#else
	// randomly allocate 128mb
	MemoryMgr_HostHeapInit(128*1024*1024);
#endif
#endif
}

#undef MemoryMgrMalloc // rouz edit
void*
MemoryMgrMalloc(size_t size)
{
    // Preserve allocator selection and the original live high-water pointer update
#ifdef USE_CUSTOM_ALLOCATOR
	void *mem = MemoryMgr_HostHeapMalloc(size);
#else
	void *mem = malloc(size);
#endif
	if((uint8_t*)mem + size > pMemoryTop)
		pMemoryTop = (uint8_t*)mem + size ;
	return mem;
}

#undef MemoryMgrRealloc // rouz edit
void*
MemoryMgrRealloc(void *ptr, size_t size)
{
    // Preserve the original reallocation argument and high-water update ordering
#ifdef USE_CUSTOM_ALLOCATOR
	void *mem = MemoryMgr_HostHeapRealloc(ptr, size);
#else
	void *mem = realloc(ptr, size);
#endif
	if((uint8_t*)mem + size  > pMemoryTop)
		pMemoryTop = (uint8_t*)mem + size ;
	return mem;
}

#undef MemoryMgrCalloc // rouz edit
void*
MemoryMgrCalloc(size_t num, size_t size)
{
    // Retain the original count multiplication, single-size high-water update and conditional clearing
#ifdef USE_CUSTOM_ALLOCATOR
	void *mem = MemoryMgr_HostHeapMalloc(num*size);
#else
	void *mem = calloc(num, size);
#endif
	if((uint8_t*)mem + size  > pMemoryTop)
		pMemoryTop = (uint8_t*)mem + size ;
#ifdef FIX_BUGS
	memset(mem, 0, num*size);
#endif
	return mem;
}

#undef MemoryMgrFree // rouz edit
void
MemoryMgrFree(void *ptr)
{
    // Preserve custom null-free guards and the actual normal allocator macro
#ifdef USE_CUSTOM_ALLOCATOR
#ifdef FIX_BUGS
	// i don't suppose this is handled by RW?
	if(ptr == NULL) return;
#endif
	MemoryMgr_HostHeapFree(ptr);
#else
	free(ptr);
#endif
}


void *
RwMallocAlign(uint32_t size, uint32_t align)
{
    // Retain both original size and alignment formulas and store the original allocation before the result
#if defined (FIX_BUGS) || defined(FIX_BUGS_64)
	uintptr_t ptralign = align-1;
	void *mem = (void *)MemoryMgrMalloc(size + sizeof(uintptr_t) + ptralign);

	ASSERT(mem != NULL);

	void *addr = (void *)((((uintptr_t)mem) + sizeof(uintptr_t) + ptralign) & ~ptralign);

	ASSERT(addr != NULL);
#else
	void *mem = (void *)MemoryMgrMalloc(size + align);

	ASSERT(mem != NULL);

	void *addr = (void *)((((uintptr_t)mem) + align) & ~(align - 1));

	ASSERT(addr != NULL);
#endif

	*(((void **)addr) - 1) = mem;

	return addr;
}

void
RwFreeAlign(void *mem)
{
    // Read and release the original allocation through the complete C memory owner
	ASSERT(mem != NULL);

	void *addr = *(((void **)mem) - 1);

	ASSERT(addr != NULL);

	MemoryMgrFree(addr);
}
//- rouz edit (ChatGPT)
