//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct ClumpReadGeometryList {void **geometries;int32_t numGeoms;} ClumpReadGeometryList;
typedef struct ClumpReadAtomicBinary {int32_t frameIndex,geomIndex,flags,unused;} ClumpReadAtomicBinary;
typedef struct ClumpReadChunkInfo {int32_t numAtomics,numLights,numCameras;} ClumpReadChunkInfo;
#ifdef __cplusplus
extern "C" {
#endif
ClumpReadGeometryList *ClumpRead_GeometryListStreamRead1(void *stream,ClumpReadGeometryList *geomlist);
ClumpReadGeometryList *ClumpRead_GeometryListStreamRead2(void *stream,ClumpReadGeometryList *geomlist);
void ClumpRead_GeometryListDeinitialize(ClumpReadGeometryList *geomlist);
void *ClumpRead_AtomicStreamRead(void *stream,void *frames,ClumpReadGeometryList *geometries);
bool ClumpRead_Start(void *stream);
void *ClumpRead_Finish(void *stream);
void ClumpRead_Cancel(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
