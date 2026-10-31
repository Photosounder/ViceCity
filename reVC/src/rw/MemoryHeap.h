//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "IntStack.h"
#ifdef MoveMemory
#undef MoveMemory
#endif
enum {
	MEMID_FREE,
	MEMID_GAME = 1,	// "Game"
	MEMID_WORLD = 2,	// "World"
	MEMID_ANIMATION = 3,	// "Animation"
	MEMID_POOLS = 4,	// "Pools"
	MEMID_DEF_MODELS = 5,	// "Default Models"
	MEMID_STREAM = 6,	// "Streaming"
	MEMID_STREAM_MODELS = 7,	// "Streamed Models"
	MEMID_STREAM_LODS = 8,	// "Streamed LODs"
	MEMID_STREAM_TEXUTRES = 9,	// "Streamed Textures"
	MEMID_STREAM_COLLISION = 10,	// "Streamed Collision"
	MEMID_STREAM_ANIMATION = 11,	// "Streamed Animation"
	MEMID_TEXTURES = 12,	// "Textures"
	MEMID_COLLISION = 13,	// "Collision"
	MEMID_PRE_ALLOC = 14,	// "PreAlloc"
	MEMID_GAME_PROCESS = 15,	// "Game Process"
	MEMID_SCRIPT = 16,	// "Script"
	MEMID_CARS = 17,	// "Cars"
	MEMID_RENDER = 18,	// "Render"
	MEMID_PED_ATTR = 19,	// "Ped Attr"
	NUM_MEMIDS,

	NUM_FIXED_MEMBLOCKS = 6
};
typedef struct HeapBlockDesc {
    uint32_t m_size;
    int16_t m_memId,m_ptrListIndex;
    struct HeapBlockDesc *m_next,*m_prev;
} HeapBlockDesc;
#ifdef USE_CUSTOM_ALLOCATOR
#ifdef __cplusplus
static_assert(sizeof(HeapBlockDesc)==0x10,"HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense");
#else
_Static_assert(sizeof(HeapBlockDesc)==0x10,"HeapBlockDesc must have 0x10 size otherwise most of assumptions don't make sense");
#endif
#endif
typedef struct HeapBlockList {HeapBlockDesc m_first,m_last;} HeapBlockList;
typedef struct CommonSize {
    HeapBlockList m_freeList;
    uint32_t m_size,m_failed,m_remaining;
} CommonSize;
typedef struct CMemoryHeap {
    HeapBlockDesc *m_start,*m_end;
    HeapBlockList m_freeList;
    CommonSize m_fixedSize[NUM_FIXED_MEMBLOCKS];
    uint32_t m_totalMemUsed;
    CIntStack16 m_idStack;
    uint32_t m_currentMemID;
    uint32_t *m_memUsed;
    uint32_t m_totalBlocksUsed;
    uint32_t *m_blocksUsed;
    uint32_t m_unkMemId;
} CMemoryHeap;
#ifdef USE_CUSTOM_ALLOCATOR
#define PUSH_MEMID(id) MemoryHeap_PushMemId(&gMainHeap,(id))
#define POP_MEMID() MemoryHeap_PopMemId(&gMainHeap)
#define REGISTER_MEMPTR(ptr) MemoryHeap_RegisterMemPointer(&gMainHeap,(ptr))
#else
#define PUSH_MEMID(id)
#define POP_MEMID()
#define REGISTER_MEMPTR(ptr)
#endif
#ifdef __cplusplus
extern "C" {
#endif
extern CMemoryHeap gMainHeap;
HeapBlockDesc * HeapBlock_GetNextConsecutive(HeapBlockDesc *self);
void * HeapBlock_GetDataPointer(HeapBlockDesc *self);
void HeapBlock_RemoveHeapFreeBlock(HeapBlockDesc *self);
void HeapBlock_InsertHeapFreeBlock(HeapBlockDesc *self,HeapBlockDesc *node);
HeapBlockDesc * HeapBlock_FindSmallestFreeBlock(HeapBlockDesc *self,uint32_t size);
void HeapList_Init(HeapBlockList *self);
void HeapList_Insert(HeapBlockList *self,HeapBlockDesc *node);
void HeapCommon_Free(CommonSize *self,HeapBlockDesc *node);
HeapBlockDesc * HeapCommon_Malloc(CommonSize *self);
void MemoryHeap_InitRecord(CMemoryHeap *self);
int32_t MemoryHeap_GetLargestFreeBlock(CMemoryHeap *self);
HeapBlockDesc * MemoryHeap_GetDescFromHeapPointer(CMemoryHeap *self,void *block);
uint32_t MemoryHeap_GetSizeBetweenBlocks(CMemoryHeap *self,HeapBlockDesc *first,HeapBlockDesc *second);
void MemoryHeap_FreeBlock(CMemoryHeap *self,HeapBlockDesc *block);
void MemoryHeap_Init(CMemoryHeap *self,uint32_t total);
void MemoryHeap_RegisterMalloc(CMemoryHeap *self,HeapBlockDesc *block);
void MemoryHeap_RegisterFree(CMemoryHeap *self,HeapBlockDesc *block);
void * MemoryHeap_Malloc(CMemoryHeap *self,uint32_t size);
void * MemoryHeap_Realloc(CMemoryHeap *self,void *ptr,uint32_t size);
void MemoryHeap_Free(CMemoryHeap *self,void *ptr);
void MemoryHeap_FillInBlockData(CMemoryHeap *self,HeapBlockDesc *block,HeapBlockDesc *end,uint32_t size);
uint32_t MemoryHeap_CombineFreeBlocks(CMemoryHeap *self,HeapBlockDesc *block);
void MemoryHeap_TidyHeap(CMemoryHeap *self);
void MemoryHeap_RegisterMemPointer(CMemoryHeap *self,void *ptr);
void * MemoryHeap_MoveMemory(CMemoryHeap *self,void *ptr);
HeapBlockDesc * MemoryHeap_WhereShouldMemoryMove(CMemoryHeap *self,void *ptr);
void * MemoryHeap_MoveHeapBlock(CMemoryHeap *self,HeapBlockDesc *dst,HeapBlockDesc *src);
uint32_t MemoryHeap_GetMemoryUsed(CMemoryHeap *self,int32_t id);
uint32_t MemoryHeap_GetBlocksUsed(CMemoryHeap *self,int32_t id);
void MemoryHeap_PopMemId(CMemoryHeap *self);
void MemoryHeap_PushMemId(CMemoryHeap *self,int32_t id);
void MemoryHeap_ParseHeap(CMemoryHeap *self);
void HeapCommon_Init(CommonSize *self,uint32_t size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
