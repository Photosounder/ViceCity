//+ rouz edit (ChatGPT)
#include "common.h"
#include "ClumpRead_backend.h"
static rwFrameList nativeFrames;
bool ClumpRead_HostFindChunk(void *stream,uint32_t kind,uint32_t *size,uint32_t *version)
{
    // Translate the reader's chunk identity to the original SDK constant for one native lookup
    static const RwUInt32 ids[]={rwID_STRUCT,rwID_GEOMETRY,rwID_FRAMELIST,rwID_GEOMETRYLIST,rwID_ATOMIC};
    return RwStreamFindChunk((RwStream*)stream,ids[kind],size,version)!=0;
}
void * ClumpRead_HostAllocate(size_t size)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return RwMalloc(size);
}
void ClumpRead_HostFree(void *memory)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RwFree(memory);
}
uint32_t ClumpRead_HostRead(void *stream,void *output,uint32_t size)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return RwStreamRead((RwStream*)stream,output,size);
}
void * ClumpRead_HostReadGeometry(void *stream)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return RpGeometryStreamRead((RwStream*)stream);
}
void ClumpRead_HostDestroyGeometry(void *geometry)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpGeometryDestroy((RpGeometry*)geometry);
}
void * ClumpRead_HostCreateAtomic(void)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return RpAtomicCreate();
}
void ClumpRead_HostDestroyAtomic(void *atomic)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpAtomicDestroy((RpAtomic*)atomic);
}
void ClumpRead_HostAtomicFlags(void *atomic,int32_t flags)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpAtomicSetFlags((RpAtomic*)atomic,flags);
}
void ClumpRead_HostAtomicFrame(void *atomic,void *frame)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpAtomicSetFrame((RpAtomic*)atomic,(RwFrame*)frame);
}
void ClumpRead_HostAtomicGeometry(void *atomic,void *geometry,uint32_t flags)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpAtomicSetGeometry((RpAtomic*)atomic,(RpGeometry*)geometry,flags);
}
void * ClumpRead_HostFrames(void)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return &nativeFrames;
}
int32_t ClumpRead_HostFrameCount(void *frames)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return ((rwFrameList*)frames)->numFrames;
}
void * ClumpRead_HostFrameAt(void *frames,int32_t index)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return ((rwFrameList*)frames)->frames[index];
}
void ClumpRead_HostSetFrameCount(void *frames,int32_t count)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    ((rwFrameList*)frames)->numFrames=count;
}
void * ClumpRead_HostReadFrames(void *stream,void *frames)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return rwFrameListStreamRead((RwStream*)stream,(rwFrameList*)frames);
}
void ClumpRead_HostDeinitializeFrames(void *frames)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    rwFrameListDeinitialize((rwFrameList*)frames);
}
ClumpReadStreamPosition ClumpRead_HostPosition(void *stream)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return STREAMPOS((RwStream*)stream);
}
void ClumpRead_HostSkip(void *stream,uint32_t offset)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RwStreamSkip((RwStream*)stream,offset);
}
void * ClumpRead_HostCreateClump(void)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    return RpClumpCreate();
}
void ClumpRead_HostDestroyClump(void *clump)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpClumpDestroy((RpClump*)clump);
}
void ClumpRead_HostClumpFrame(void *clump,void *frame)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpClumpSetFrame((RpClump*)clump,(RwFrame*)frame);
}
void ClumpRead_HostAddAtomic(void *clump,void *atomic)
{
    // Preserve the original external SDK primitive and its live borrowed arguments
    RpClumpAddAtomic((RpClump*)clump,(RpAtomic*)atomic);
}
//- rouz edit (ChatGPT)
