//+ rouz edit (ChatGPT)
#include "common.h"
#include "RenderBuffer_backend.h"
VertexBufferUnion TempVertexBuffer;
void *RenderBuffer_BackendVertex(int32_t index)
{
    // Address the actual typed vertex array without guessing its stride
    return &TempBufferRenderVertices[index];
}
void RenderBuffer_BackendWriteVertex(void *output,int32_t index)
{
    // Publish the original typed pointer at the C algorithm's ordered write point
    *(RwIm3DVertex **)output=&TempBufferRenderVertices[index];
}
bool RenderBuffer_BackendTransform(int32_t vertices)
{
    // Preserve the original typed buffer, count, null matrix and UV flags
    return RwIm3DTransform(TempBufferRenderVertices,vertices,nil,rwIM3D_VERTEXUV)!=nil;
}
void RenderBuffer_BackendRenderIndexed(int32_t indices)
{
    // Render the original shared index array using the live C-selected count
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,TempBufferRenderIndexList,indices);
}
void RenderBuffer_BackendEnd(void)
{
    // Complete the original successful immediate draw
    RwIm3DEnd();
}
//- rouz edit (ChatGPT)
