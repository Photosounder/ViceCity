//+ rouz edit (ChatGPT)
#include "config.h"
#include "MemoryHeap.h"
#include "MemoryMgr_backend.h"
#include "ColModel_backend.h"
#include "AnimBlendSequence_backend.h"
#ifdef USE_CUSTOM_ALLOCATOR
void MemoryMgr_HostHeapInit(uint32_t size)
{
    // Initialize the actual native heap at the original external operation boundary
    MemoryHeap_Init(&gMainHeap,size);
}
#endif
#ifdef USE_CUSTOM_ALLOCATOR
void *MemoryMgr_HostHeapMalloc(uint32_t size)
{
    // Forward the original narrowed byte request to the actual native heap
    return MemoryHeap_Malloc(&gMainHeap,size);
}
#endif
#ifdef USE_CUSTOM_ALLOCATOR
void *MemoryMgr_HostHeapRealloc(void *ptr,uint32_t size)
{
    // Reallocate the exact caller pointer through the original native heap
    return MemoryHeap_Realloc(&gMainHeap,ptr,size);
}
#endif
#ifdef USE_CUSTOM_ALLOCATOR
void MemoryMgr_HostHeapFree(void *ptr)
{
    // Release the exact caller pointer through the original native heap
    MemoryHeap_Free(&gMainHeap,ptr);
}
#endif
void ColModel_BackendPush(int32_t id)
{
    // Preserve the original conditional custom-heap category operation
    PUSH_MEMID(id);
}
void ColModel_BackendPop(void)
{
    // Restore the original custom-heap category when that allocator is enabled
    POP_MEMID();
}
void ColModel_BackendRegister(void *location)
{
    // Register the actual live pointer slot with the original custom allocator
    REGISTER_MEMPTR(location);
}
void AnimSequence_BackendPushAnimation(void)
{
    // Preserve the original animation memory category boundary
    PUSH_MEMID(MEMID_ANIMATION);
}
void AnimSequence_BackendPopMemory(void)
{
    // Restore the original heap memory category after keyframe work
    POP_MEMID();
}
void AnimSequence_BackendRegister(void **memory)
{
    // Register the address of the owner's actual keyframe pointer
    REGISTER_MEMPTR(memory);
}
#ifdef USE_CUSTOM_ALLOCATOR
void *AnimSequence_BackendMoveMemory(void *memory)
{
    // Forward only the native heap relocation primitive
    return MemoryHeap_MoveMemory(&gMainHeap,memory);
}
#endif
//- rouz edit (ChatGPT)
