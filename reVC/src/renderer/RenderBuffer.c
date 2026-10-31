//+ rouz edit (ChatGPT)
#include "RenderBuffer_backend.h"

typedef void (*RenderBufferWriter)(void *,int32_t);
static void RenderBuffer_WriteC(void *output,int32_t index)
{
    // Publish an opaque C vertex handle before updating pending reservation counts
    *(void **)output=RenderBuffer_BackendVertex(index);
}
int32_t TempBufferVerticesStored;
int32_t TempBufferIndicesStored;

uint16_t TempBufferRenderIndexList[TEMPBUFFERINDEXSIZE];

int32_t RenderBuffer_VerticesToBeStored;
int32_t RenderBuffer_IndicesToBeStored;

void
RenderBuffer_ClearRenderBuffer(void)
{
	// Preserve original live counters, short circuits and write order
	TempBufferVerticesStored = 0;
	TempBufferIndicesStored = 0;
}

static void
RenderBuffer_Reserve(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void *vertexStart,RenderBufferWriter write)
{
	// Preserve original live counters, short circuits and write order
	if(TempBufferIndicesStored + numIndices >= TEMPBUFFERINDEXSIZE)
		RenderBuffer_RenderStuffInBuffer();
	if(TempBufferVerticesStored + numVertices >= TEMPBUFFERVERTSIZE)
		RenderBuffer_RenderStuffInBuffer();
        *indexStart = &TempBufferRenderIndexList[TempBufferIndicesStored];
        write(vertexStart,TempBufferVerticesStored);
        RenderBuffer_IndicesToBeStored = numIndices;
        RenderBuffer_VerticesToBeStored = numVertices;
}

void
RenderBuffer_StopStoring(void)
{
	// Preserve original live counters, short circuits and write order
	int i;
	for(i = TempBufferIndicesStored; i < TempBufferIndicesStored+RenderBuffer_IndicesToBeStored; i++)
		TempBufferRenderIndexList[i] += TempBufferVerticesStored;
	TempBufferIndicesStored += RenderBuffer_IndicesToBeStored;
	TempBufferVerticesStored += RenderBuffer_VerticesToBeStored;
}

void
RenderBuffer_RenderStuffInBuffer(void)
{
	// Preserve original live counters, short circuits and write order
	if(TempBufferVerticesStored && RenderBuffer_BackendTransform(TempBufferVerticesStored)){
		// Render and end only after the original transform succeeds
		RenderBuffer_BackendRenderIndexed(TempBufferIndicesStored);
		RenderBuffer_BackendEnd();
	}
	RenderBuffer_ClearRenderBuffer();
}

void RenderBuffer_StartStoring(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void **vertexStart)
{
    // Enter the complete reservation algorithm with a legal C pointer output
    RenderBuffer_Reserve(numIndices,numVertices,indexStart,vertexStart,RenderBuffer_WriteC);
}
void RenderBuffer_StartStoringLegacy(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void *vertexStart)
{
    // Write actual legacy pointer storage through its typed owner operation
    RenderBuffer_Reserve(numIndices,numVertices,indexStart,vertexStart,RenderBuffer_BackendWriteVertex);
}
//- rouz edit (ChatGPT)
