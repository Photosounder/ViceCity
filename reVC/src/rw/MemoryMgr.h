#pragma once

extern RwMemoryFunctions memFuncs;
void InitMemoryMgr(void);

#if 0	// rouz edit
void *MemoryMgrMalloc(size_t size);
void *MemoryMgrRealloc(void *ptr, size_t size);
void *MemoryMgrCalloc(size_t num, size_t size);
void MemoryMgrFree(void *ptr);
//+ rouz edit
#else
#define MemoryMgrMalloc(s) cita_win_malloc((s), __FILE_NAME__, __func__, __LINE__)
#define MemoryMgrFree(p) cita_win_free((p), __FILE_NAME__, __func__, __LINE__)
#define MemoryMgrCalloc(n, s) cita_win_calloc((n), (s), __FILE_NAME__, __func__, __LINE__)
#define MemoryMgrRealloc(p, s) cita_win_realloc((p), (s), __FILE_NAME__, __func__, __LINE__)
#endif
//- rouz edit

void *RwMallocAlign(RwUInt32 size, RwUInt32 align);
void RwFreeAlign(void *mem);
