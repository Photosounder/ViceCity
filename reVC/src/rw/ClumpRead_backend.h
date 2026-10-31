//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
enum {CLUMPREAD_STRUCT,CLUMPREAD_GEOMETRY,CLUMPREAD_FRAMELIST,CLUMPREAD_GEOMETRYLIST,CLUMPREAD_ATOMIC};
#ifdef LIBRW
typedef int32_t ClumpReadStreamPosition;
#else
typedef uint32_t ClumpReadStreamPosition;
#endif
bool ClumpRead_HostFindChunk(void *stream,uint32_t kind,uint32_t *size,uint32_t *version);
void * ClumpRead_HostAllocate(size_t size);
void ClumpRead_HostFree(void *memory);
uint32_t ClumpRead_HostRead(void *stream,void *output,uint32_t size);
void * ClumpRead_HostReadGeometry(void *stream);
void ClumpRead_HostDestroyGeometry(void *geometry);
void * ClumpRead_HostCreateAtomic(void);
void ClumpRead_HostDestroyAtomic(void *atomic);
void ClumpRead_HostAtomicFlags(void *atomic,int32_t flags);
void ClumpRead_HostAtomicFrame(void *atomic,void *frame);
void ClumpRead_HostAtomicGeometry(void *atomic,void *geometry,uint32_t flags);
void * ClumpRead_HostFrames(void);
int32_t ClumpRead_HostFrameCount(void *frames);
void * ClumpRead_HostFrameAt(void *frames,int32_t index);
void ClumpRead_HostSetFrameCount(void *frames,int32_t count);
void * ClumpRead_HostReadFrames(void *stream,void *frames);
void ClumpRead_HostDeinitializeFrames(void *frames);
ClumpReadStreamPosition ClumpRead_HostPosition(void *stream);
void ClumpRead_HostSkip(void *stream,uint32_t offset);
void * ClumpRead_HostCreateClump(void);
void ClumpRead_HostDestroyClump(void *clump);
void ClumpRead_HostClumpFrame(void *clump,void *frame);
void ClumpRead_HostAddAtomic(void *clump,void *atomic);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
