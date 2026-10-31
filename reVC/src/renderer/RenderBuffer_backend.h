//+ rouz edit (ChatGPT)
#pragma once
#include "RenderBuffer.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void *RenderBuffer_BackendVertex(int32_t index);
void RenderBuffer_BackendWriteVertex(void *output,int32_t index);
bool RenderBuffer_BackendTransform(int32_t vertices);
void RenderBuffer_BackendRenderIndexed(int32_t indices);
void RenderBuffer_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
