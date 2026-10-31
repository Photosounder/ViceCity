//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#define TEMPBUFFERVERTSIZE 512
#define TEMPBUFFERINDEXSIZE 1024
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t TempBufferVerticesStored;
extern int32_t TempBufferIndicesStored;
extern int32_t RenderBuffer_VerticesToBeStored;
extern int32_t RenderBuffer_IndicesToBeStored;
extern uint16_t TempBufferRenderIndexList[TEMPBUFFERINDEXSIZE];
void RenderBuffer_ClearRenderBuffer(void);
void RenderBuffer_StartStoring(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void **vertexStart);
void RenderBuffer_StartStoringLegacy(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void *vertexStart);
void RenderBuffer_StopStoring(void);
void RenderBuffer_RenderStuffInBuffer(void);
#ifdef __cplusplus
}
struct VertexBufferUnion
{
	RwIm2DVertex im2d[TEMPBUFFERVERTSIZE];
	RwIm3DVertex im3d[TEMPBUFFERVERTSIZE];
};

extern VertexBufferUnion TempVertexBuffer;
#define TempBufferRenderVertices (TempVertexBuffer.im3d)
#endif
//- rouz edit (ChatGPT)
