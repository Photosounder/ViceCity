//+ rouz edit (ChatGPT)
#include "config.h"
#include "MemoryHeap.h"
#include "MemoryHeap_backend.h"
#include "FileMgr.h"
#include "Timer.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#define MEMORYHEAP_ASSERT(cond) assert(cond)
#define MEMORYHEAP_ASSERT_MESSAGE(cond,message) assert(cond)
#ifdef USE_CUSTOM_ALLOCATOR
void **gPtrList[4000];
int32_t numPtrs,gPosnInList;
CIntStack4000 m_ptrListIndexStack;
uint32_t memMoved;
CMemoryHeap gMainHeap;
#endif
HeapBlockDesc * HeapBlock_GetNextConsecutive(HeapBlockDesc *self)
{
    // Preserve the complete original block GetNextConsecutive operation and live mutation order

		return (HeapBlockDesc*)((uintptr_t)self + sizeof(HeapBlockDesc) + self->m_size);
	
}
void * HeapBlock_GetDataPointer(HeapBlockDesc *self)
{
    // Preserve the complete original block GetDataPointer operation and live mutation order

		return (void*)((uintptr_t)self + sizeof(HeapBlockDesc));
	
}
void HeapBlock_RemoveHeapFreeBlock(HeapBlockDesc *self)
{
    // Preserve the complete original block RemoveHeapFreeBlock operation and live mutation order

		self->m_next->m_prev = self->m_prev;
		self->m_prev->m_next = self->m_next;
	
}
void HeapBlock_InsertHeapFreeBlock(HeapBlockDesc *self,HeapBlockDesc *node)
{
    // Preserve the complete original block InsertHeapFreeBlock operation and live mutation order

		self->m_next = node->m_next;
		node->m_next->m_prev = self;
		self->m_prev = node;
		node->m_next = self;
	
}
HeapBlockDesc * HeapBlock_FindSmallestFreeBlock(HeapBlockDesc *self,uint32_t size)
{
    // Preserve the complete original block FindSmallestFreeBlock operation and live mutation order

		HeapBlockDesc *b;
		for(b = self->m_next; b->m_size < size; b = b->m_next);
		return b;
	
}
void HeapList_Init(HeapBlockList *self)
{
    // Preserve the complete original list Init operation and live mutation order

		self->m_first.m_next = &self->m_last;
		self->m_last.m_prev = &self->m_first;
	
}
void HeapList_Insert(HeapBlockList *self,HeapBlockDesc *node)
{
    // Preserve the complete original list Insert operation and live mutation order

		HeapBlock_InsertHeapFreeBlock(node,&self->m_first);
	
}
void HeapCommon_Free(CommonSize *self,HeapBlockDesc *node)
{
    // Preserve the complete original common Free operation and live mutation order

		HeapList_Insert(&self->m_freeList,node);
		self->m_remaining++;
	
}
HeapBlockDesc * HeapCommon_Malloc(CommonSize *self)
{
    // Preserve the complete original common Malloc operation and live mutation order

		if(self->m_freeList.m_first.m_next == &self->m_freeList.m_last){
			self->m_failed++;
			return NULL;
		}
		HeapBlockDesc *block = self->m_freeList.m_first.m_next;
		self->m_remaining--;
		HeapBlock_RemoveHeapFreeBlock(block);
		block->m_ptrListIndex = -1;
		return block;
	
}
void MemoryHeap_InitRecord(CMemoryHeap *self)
{
    // Preserve the complete original heap InitRecord operation and live mutation order

    self->m_start=NULL;

		// Initialize the embedded stack counter for heaps with automatic storage
		CIntStack16_Init(&self->m_idStack);
	
}
int32_t MemoryHeap_GetLargestFreeBlock(CMemoryHeap *self)
{
    // Preserve the complete original heap GetLargestFreeBlock operation and live mutation order
 return self->m_freeList.m_last.m_prev->m_size; 
}
HeapBlockDesc * MemoryHeap_GetDescFromHeapPointer(CMemoryHeap *self,void *block)
{
    // Preserve the complete original heap GetDescFromHeapPointer operation and live mutation order

		return (HeapBlockDesc*)((uintptr_t)block - sizeof(HeapBlockDesc));
	
}
uint32_t MemoryHeap_GetSizeBetweenBlocks(CMemoryHeap *self,HeapBlockDesc *first,HeapBlockDesc *second)
{
    // Preserve the complete original heap GetSizeBetweenBlocks operation and live mutation order

		return (uintptr_t)second - (uintptr_t)first - sizeof(HeapBlockDesc);
	
}
void MemoryHeap_FreeBlock(CMemoryHeap *self,HeapBlockDesc *block)
{
    // Preserve the complete original heap FreeBlock operation and live mutation order

		for(int i = 0; i < NUM_FIXED_MEMBLOCKS; i++){
			if(self->m_fixedSize[i].m_size == block->m_size){
				HeapCommon_Free(&self->m_fixedSize[i],block);
				return;
			}
		}
		HeapBlockDesc *b = HeapBlock_FindSmallestFreeBlock(&self->m_freeList.m_first,block->m_size);
		HeapBlock_InsertHeapFreeBlock(block,b->m_prev);
	
}
#ifdef USE_CUSTOM_ALLOCATOR
void MemoryHeap_Init(CMemoryHeap *self,uint32_t total)
{
    // Preserve the complete original heap Init operation and live mutation order

	MEMORYHEAP_ASSERT((total != 0xF) != 0);

	self->m_totalMemUsed = 0;
	self->m_memUsed = NULL;
	self->m_currentMemID = MEMID_FREE;
	self->m_blocksUsed = NULL;
	self->m_totalBlocksUsed = 0;
	self->m_unkMemId = -1;

	uint8_t *mem = (uint8_t*)malloc(total);
	assert(((uintptr_t)mem & 0xF) == 0);
	self->m_start = (HeapBlockDesc*)mem;
	self->m_end = (HeapBlockDesc*)(mem + total - sizeof(HeapBlockDesc));
	self->m_start->m_memId = MEMID_FREE;
	self->m_start->m_size = total - 2*sizeof(HeapBlockDesc);
	self->m_end->m_memId = MEMID_GAME;
	self->m_end->m_size = 0;

	self->m_freeList.m_last.m_size = INT_MAX;
	HeapList_Init(&self->m_freeList);
	HeapList_Insert(&self->m_freeList,self->m_start);

	// TODO: figure out what these are and use sizeof
	HeapCommon_Init(&self->m_fixedSize[0],0x10);
	HeapCommon_Init(&self->m_fixedSize[1],0x20);
	HeapCommon_Init(&self->m_fixedSize[2],0xE0);
	HeapCommon_Init(&self->m_fixedSize[3],0x60);
	HeapCommon_Init(&self->m_fixedSize[4],0x1C0);
	HeapCommon_Init(&self->m_fixedSize[5],0x50);

	self->m_currentMemID = MEMID_FREE;	// disable registration
	self->m_memUsed = (uint32_t*)MemoryHeap_Malloc(self,NUM_MEMIDS * sizeof(uint32_t));
	self->m_blocksUsed = (uint32_t*)MemoryHeap_Malloc(self,NUM_MEMIDS * sizeof(uint32_t));
	MemoryHeap_RegisterMalloc(self,MemoryHeap_GetDescFromHeapPointer(self,self->m_memUsed));
	MemoryHeap_RegisterMalloc(self,MemoryHeap_GetDescFromHeapPointer(self,self->m_blocksUsed));

	self->m_currentMemID = MEMID_GAME;
	for(int i = 0; i < NUM_MEMIDS; i++){
		self->m_memUsed[i] = 0;
		self->m_blocksUsed[i] = 0;
	}

}
void MemoryHeap_RegisterMalloc(CMemoryHeap *self,HeapBlockDesc *block)
{
    // Preserve the complete original heap RegisterMalloc operation and live mutation order

	block->m_memId = self->m_currentMemID;
	if(self->m_currentMemID == MEMID_FREE)
		return;
	self->m_totalMemUsed += block->m_size + sizeof(HeapBlockDesc);
	self->m_memUsed[self->m_currentMemID] += block->m_size + sizeof(HeapBlockDesc);
	self->m_blocksUsed[self->m_currentMemID]++;
	self->m_totalBlocksUsed++;

}
void MemoryHeap_RegisterFree(CMemoryHeap *self,HeapBlockDesc *block)
{
    // Preserve the complete original heap RegisterFree operation and live mutation order

	if(block->m_memId == MEMID_FREE)
		return;
	self->m_totalMemUsed -= block->m_size + sizeof(HeapBlockDesc);
	self->m_memUsed[block->m_memId] -= block->m_size + sizeof(HeapBlockDesc);
	self->m_blocksUsed[block->m_memId]--;
	self->m_totalBlocksUsed--;

}
void * MemoryHeap_Malloc(CMemoryHeap *self,uint32_t size)
{
    // Preserve the complete original heap Malloc operation and live mutation order

	static int recursion = 0;

	// weird way to round up
	if((size & 0xF) != 0)
		size = (size&~0xF) + 0x10;

	recursion++;

	// See if we can allocate from one of the fixed-size lists
	for(int i = 0; i < NUM_FIXED_MEMBLOCKS; i++){
		CommonSize *list = &self->m_fixedSize[i];
		if(self->m_fixedSize[i].m_size == size){
			HeapBlockDesc *block = HeapCommon_Malloc(list);
			if(block){
				MemoryHeap_RegisterMalloc(self,block);
				recursion--;
				return HeapBlock_GetDataPointer(block);
			}
			break;
		}
	}

	// now try the normal free list
	HeapBlockDesc *next;
	for(HeapBlockDesc *block = self->m_freeList.m_first.m_next;
	    block != &self->m_freeList.m_last;
	    block = next){
		MEMORYHEAP_ASSERT(block->m_memId == MEMID_FREE);
		MEMORYHEAP_ASSERT_MESSAGE(block >= self->m_start && block <= self->m_end, "Block outside of memory");

		// make sure block has maximum size
		uint32_t initialsize = block->m_size;
		uint32_t blocksize = MemoryHeap_CombineFreeBlocks(self,block);
#ifdef FIX_BUGS
		// has to be done here because block can be moved
		next = block->m_next;
#endif
		if(initialsize != blocksize){
			HeapBlock_RemoveHeapFreeBlock(block);
			HeapBlockDesc *pos = HeapBlock_FindSmallestFreeBlock(block->m_prev,block->m_size);
			HeapBlock_InsertHeapFreeBlock(block,pos->m_prev);
		}
		if(block->m_size >= size){
			// got space to allocate from!
			HeapBlock_RemoveHeapFreeBlock(block);
			MemoryHeap_FillInBlockData(self,block, HeapBlock_GetNextConsecutive(block), size);
			recursion--;
			return HeapBlock_GetDataPointer(block);
		}
#ifndef FIX_BUGS
		next = block->m_next;
#endif
	}

	// oh no, we're losing, try to free some stuff
	static bool removeCollision = false;
	static bool removeIslands = false;
	static bool removeBigBuildings = false;
	size_t initialMemoryUsed = MemoryHeap_HostMemoryUsed();
	MemoryHeap_HostMakeSpaceFor(0xCFE800 - MemoryHeap_HostMemoryUsed());
	if (recursion > 10)
		MemoryHeap_HostTidyGame(true, false);
	else if (recursion > 6)
		MemoryHeap_HostTidyGame(false, true);
	if (initialMemoryUsed == MemoryHeap_HostMemoryUsed() && recursion > 11) {
		if (!removeCollision && !MemoryHeap_HostPlayingIntro()) {
			MemoryHeap_HostRemoveOtherCollision();
			removeCollision = true;
		}
		else if (!removeIslands && !MemoryHeap_HostPlayingIntro()) {
			MemoryHeap_HostRemoveIslandsIndustrial();
			MemoryHeap_HostRemoveIslandsCommercial();
			MemoryHeap_HostRemoveIslandsSuburban();
			removeIslands = true;
		}
		else if (!removeBigBuildings) {
			MemoryHeap_HostRemoveBuildingsIndustrial();
			MemoryHeap_HostRemoveBuildingsCommercial();
			MemoryHeap_HostRemoveBuildingsSuburban();
		}
		else {
			MemoryHeap_HostLoadingScreen("NO MORE MEMORY", NULL, NULL);
			MemoryHeap_HostLoadingScreen("NO MORE MEMORY", NULL, NULL);
		}
		MemoryHeap_HostTidyGame(true, false);
	}
	void *mem = MemoryHeap_Malloc(self,size);
	if (removeCollision) {
		Timer_Stop();
		// TODO: different on PS2
		MemoryHeap_HostLoadCollision(MemoryHeap_HostCollisionLevel());
		removeCollision = false;
		Timer_Update();
	}
	if (removeBigBuildings || removeIslands) {
		Timer_Stop();
		if (!MemoryHeap_HostPlayingIntro())
			MemoryHeap_HostRequestBuildings(MemoryHeap_HostCurrentLevel());
		MemoryHeap_HostLoadRequested(true);
		removeBigBuildings = false;
		removeIslands = false;
		Timer_Update();
	}
	recursion--;
	return mem;

}
void * MemoryHeap_Realloc(CMemoryHeap *self,void *ptr,uint32_t size)
{
    // Preserve the complete original heap Realloc operation and live mutation order

	if(ptr == NULL)
		return MemoryHeap_Malloc(self,size);

	// weird way to round up
	if((size & 0xF) != 0)
		size = (size&~0xF) + 0x10;

	HeapBlockDesc *block = MemoryHeap_GetDescFromHeapPointer(self,ptr);

#ifdef FIX_BUGS
	// better handling of size < block->m_size
	if(size == 0){
		MemoryHeap_Free(self,ptr);
		return NULL;
	}
	if(block->m_size >= size){
		// shrink allocated block
		MemoryHeap_RegisterFree(self,block);
		MemoryHeap_PushMemId(self,block->m_memId);
		MemoryHeap_FillInBlockData(self,block, HeapBlock_GetNextConsecutive(block), size);
		MemoryHeap_PopMemId(self);
		return ptr;
	}
#else
	// not growing. just returning here is a bit cheap though
	if(block->m_size >= size)
		return ptr;
#endif

	// have to grow allocated block
	HeapBlockDesc *next = HeapBlock_GetNextConsecutive(block);
	MEMORYHEAP_ASSERT_MESSAGE(next >= self->m_start && next <= self->m_end, "Block outside of memory");
	if(next->m_memId == MEMID_FREE){
		// try to grow the current block
		// make sure the next free block has maximum size
		uint32_t freespace = MemoryHeap_CombineFreeBlocks(self,next);
		HeapBlockDesc *end = HeapBlock_GetNextConsecutive(next);
		MEMORYHEAP_ASSERT_MESSAGE(end >= self->m_start && end <= self->m_end, "Block outside of memory");
		// why the sizeof here?
		if(block->m_size + next->m_size + sizeof(HeapBlockDesc) >= size){
			// enough space to grow
			HeapBlock_RemoveHeapFreeBlock(next);
			MemoryHeap_RegisterFree(self,block);
			MemoryHeap_PushMemId(self,block->m_memId);
			MemoryHeap_FillInBlockData(self,block, HeapBlock_GetNextConsecutive(next), size);
			MemoryHeap_PopMemId(self);
			return ptr;
		}
	}

	// can't grow the existing block, have to get a new one and copy
	MemoryHeap_PushMemId(self,block->m_memId);
	void *dst = MemoryHeap_Malloc(self,size);
	MemoryHeap_PopMemId(self);
	memcpy(dst, ptr, block->m_size);
	MemoryHeap_Free(self,ptr);
	return dst;

}
void MemoryHeap_Free(CMemoryHeap *self,void *ptr)
{
    // Preserve the complete original heap Free operation and live mutation order

	HeapBlockDesc *block = MemoryHeap_GetDescFromHeapPointer(self,ptr);
	MEMORYHEAP_ASSERT_MESSAGE(block->m_memId != MEMID_FREE, "MemoryHeap corrupt");
	MEMORYHEAP_ASSERT(self->m_unkMemId == -1 || self->m_unkMemId == block->m_memId);

	MemoryHeap_RegisterFree(self,block);
	block->m_memId = MEMID_FREE;
	MemoryHeap_CombineFreeBlocks(self,block);
	MemoryHeap_FreeBlock(self,block);
	if(block->m_ptrListIndex != -1){
		int32_t idx = block->m_ptrListIndex;
		gPtrList[idx] = NULL;
		// Return the freed pointer-list index to the embedded C stack
		//+ rouz edit (ChatGPT)
		CIntStack4000_Push(&m_ptrListIndexStack, idx);
		//- rouz edit (ChatGPT)
	}
	block->m_ptrListIndex = -1;

}
void MemoryHeap_FillInBlockData(CMemoryHeap *self,HeapBlockDesc *block,HeapBlockDesc *end,uint32_t size)
{
    // Preserve the complete original heap FillInBlockData operation and live mutation order

	block->m_size = size;
	block->m_ptrListIndex = -1;
	HeapBlockDesc *remainder = HeapBlock_GetNextConsecutive(block);
	MEMORYHEAP_ASSERT(remainder <= end);

	if(remainder < end-1){
		MemoryHeap_RegisterMalloc(self,block);

		// can fit another block in the remaining space
		remainder->m_size = MemoryHeap_GetSizeBetweenBlocks(self,remainder, end);
		remainder->m_memId = MEMID_FREE;
		MEMORYHEAP_ASSERT(remainder->m_size != 0);
		MemoryHeap_FreeBlock(self,remainder);
	}else{
		// fully allocate self one
		if(remainder < end)
			// no gaps allowed
			block->m_size = MemoryHeap_GetSizeBetweenBlocks(self,block, end);
		MemoryHeap_RegisterMalloc(self,block);
	}

}
uint32_t MemoryHeap_CombineFreeBlocks(CMemoryHeap *self,HeapBlockDesc *block)
{
    // Preserve the complete original heap CombineFreeBlocks operation and live mutation order

	HeapBlockDesc *next = HeapBlock_GetNextConsecutive(block);
	if(next->m_memId != MEMID_FREE)
		return block->m_size;
	// get rid of free blocks after self one and adjust size
	for(; next->m_memId == MEMID_FREE; next = HeapBlock_GetNextConsecutive(next))
		HeapBlock_RemoveHeapFreeBlock(next);
	block->m_size = MemoryHeap_GetSizeBetweenBlocks(self,block, next);
	return block->m_size;

}
void MemoryHeap_TidyHeap(CMemoryHeap *self)
{
    // Preserve the complete original heap TidyHeap operation and live mutation order

	for(int i = 0; i < numPtrs; i++){
		if(gPtrList[i] == NULL || *gPtrList[i] == NULL)
			continue;
		HeapBlockDesc *newblock = MemoryHeap_WhereShouldMemoryMove(self,*gPtrList[i]);
		if(newblock)
			*gPtrList[i] = MemoryHeap_MoveHeapBlock(self,newblock, MemoryHeap_GetDescFromHeapPointer(self,*gPtrList[i]));
	}

}
void MemoryHeap_RegisterMemPointer(CMemoryHeap *self,void *ptr)
{
    // Preserve the complete original heap RegisterMemPointer operation and live mutation order

	HeapBlockDesc *block = MemoryHeap_GetDescFromHeapPointer(self,*(void**)ptr);

	if(block->m_ptrListIndex != -1)
		return;		// already registered

	int index;
	if(m_ptrListIndexStack.sp > 0){
		// re-use a previously free'd index
		// Reuse the most recently freed pointer-list index
		//+ rouz edit (ChatGPT)
		index = CIntStack4000_Pop(&m_ptrListIndexStack);
		//- rouz edit (ChatGPT)
	}else{
		// have to find a new index
		index = gPosnInList;

		void **pp = gPtrList[index];
		// we're replacing an old pointer here??
		if(pp && *pp && *pp != (void*)0xDDDDDDDD)
			MemoryHeap_GetDescFromHeapPointer(self,*pp)->m_ptrListIndex = -1;

		gPosnInList++;
		if(gPosnInList == 4000)
			gPosnInList = 0;
		if(numPtrs < 4000)
			numPtrs++;
	}
	gPtrList[index] = (void**)ptr;
	block->m_ptrListIndex = index;

}
void * MemoryHeap_MoveMemory(CMemoryHeap *self,void *ptr)
{
    // Preserve the complete original heap MoveMemory operation and live mutation order

	HeapBlockDesc *newblock = MemoryHeap_WhereShouldMemoryMove(self,ptr);
	if(newblock)
		return MemoryHeap_MoveHeapBlock(self,newblock, MemoryHeap_GetDescFromHeapPointer(self,ptr));
	else
		return ptr;

}
HeapBlockDesc * MemoryHeap_WhereShouldMemoryMove(CMemoryHeap *self,void *ptr)
{
    // Preserve the complete original heap WhereShouldMemoryMove operation and live mutation order

	HeapBlockDesc *block = MemoryHeap_GetDescFromHeapPointer(self,ptr);
	MEMORYHEAP_ASSERT(block->m_memId != MEMID_FREE);

	HeapBlockDesc *next = HeapBlock_GetNextConsecutive(block);
	if(next->m_memId != MEMID_FREE)
		return NULL;

	// we want to move the block into another block
	// such that the free space between self and the next block can be minimized
	HeapBlockDesc *newblock = HeapBlock_FindSmallestFreeBlock(&self->m_freeList.m_first,block->m_size);
	// size of free space wouldn't decrease, so return
	if(newblock->m_size >= block->m_size + next->m_size)
		return NULL;
	// size of free space wouldn't decrease enough
	if(newblock->m_size >= 16 + 1.125f*block->m_size)	// what are 16 and 1.125 here? sizeof(HeapBlockDesc)?
		return NULL;
	return newblock;

}
void * MemoryHeap_MoveHeapBlock(CMemoryHeap *self,HeapBlockDesc *dst,HeapBlockDesc *src)
{
    // Preserve the complete original heap MoveHeapBlock operation and live mutation order

	MemoryHeap_PushMemId(self,src->m_memId);
	HeapBlock_RemoveHeapFreeBlock(dst);
	MemoryHeap_FillInBlockData(self,dst, HeapBlock_GetNextConsecutive(dst), src->m_size);
	MemoryHeap_PopMemId(self);
	memcpy(HeapBlock_GetDataPointer(dst), HeapBlock_GetDataPointer(src), src->m_size);
	memMoved += src->m_size;
	dst->m_ptrListIndex = src->m_ptrListIndex;
	src->m_ptrListIndex = -1;
	MemoryHeap_Free(self,HeapBlock_GetDataPointer(src));
	return HeapBlock_GetDataPointer(dst);

}
uint32_t MemoryHeap_GetMemoryUsed(CMemoryHeap *self,int32_t id)
{
    // Preserve the complete original heap GetMemoryUsed operation and live mutation order

	return self->m_memUsed[id];

}
uint32_t MemoryHeap_GetBlocksUsed(CMemoryHeap *self,int32_t id)
{
    // Preserve the complete original heap GetBlocksUsed operation and live mutation order

	return self->m_blocksUsed[id];

}
void MemoryHeap_PopMemId(CMemoryHeap *self)
{
    // Preserve the complete original heap PopMemId operation and live mutation order

	assert(self->m_idStack.sp > 0);
	// Restore the previous memory identifier from the embedded C stack
	//+ rouz edit (ChatGPT)
	self->m_currentMemID = CIntStack16_Pop(&self->m_idStack);
	//- rouz edit (ChatGPT)
	assert(self->m_currentMemID != MEMID_FREE);

}
void MemoryHeap_PushMemId(CMemoryHeap *self,int32_t id)
{
    // Preserve the complete original heap PushMemId operation and live mutation order

	MEMORYHEAP_ASSERT(id != MEMID_FREE);
	assert(self->m_idStack.sp < 16);
	// Save the current memory identifier in the embedded C stack
	//+ rouz edit (ChatGPT)
	CIntStack16_Push(&self->m_idStack, self->m_currentMemID);
	//- rouz edit (ChatGPT)
	self->m_currentMemID = id;

}
void MemoryHeap_ParseHeap(CMemoryHeap *self)
{
    // Preserve the complete original heap ParseHeap operation and live mutation order

	char tmp[16];
	int fd = FileMgr_OpenFileForWriting("heap.txt");
	Timer_Stop();

	// CMemoryHeap::IntegrityCheck();

	uint32_t addrQW = 0;
	for(HeapBlockDesc *block = self->m_start; block < self->m_end; block = HeapBlock_GetNextConsecutive(block)){
		char chr = '*';	// free
		if(block->m_memId != MEMID_FREE)
			chr = block->m_memId-1 + 'A';
		int numQW = block->m_size>>4;

		if((addrQW & 0x3F) == 0){
			sprintf(tmp, "\n%5dK:", addrQW>>6);
			FileMgr_Write(fd, tmp, 8);
		}
		FileMgr_Write(fd, "#", 1);	// the descriptor, has to be 16 bytes!!!!
		addrQW++;

		while(numQW--){
			if((addrQW & 0x3F) == 0){
				sprintf(tmp, "\n%5dK:", addrQW>>6);
				FileMgr_Write(fd, tmp, 8);
			}
			FileMgr_Write(fd, &chr, 1);
			addrQW++;
		}
	}

	Timer_Update();
	FileMgr_CloseFile(fd);

}
#endif
#ifdef USE_CUSTOM_ALLOCATOR
void HeapCommon_Init(CommonSize *self,uint32_t size)
{
    // Preserve the complete original common Init operation and live mutation order

	HeapList_Init(&self->m_freeList);
	self->m_size = size;
	self->m_failed = 0;
	self->m_remaining = 0;

}
#endif
//- rouz edit (ChatGPT)
