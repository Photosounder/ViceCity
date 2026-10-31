//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "MemoryMgr_native.h"
#include "MemoryMgr_backend.h"
#include "MemoryHeap.h"
#undef MemoryMgrMalloc
#undef MemoryMgrFree
#undef MemoryMgrRealloc
#undef MemoryMgrCalloc
RwMemoryFunctions memFuncs = {
	MemoryMgrMalloc,
	MemoryMgrFree,
	MemoryMgrRealloc,
	MemoryMgrCalloc
};
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
