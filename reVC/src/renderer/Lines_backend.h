//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void *Lines_BackendVertices(void);
void Lines_BackendSetRGBA(void *vertices, int index, int red, int green, int blue, int alpha);
void Lines_BackendSetPos(void *vertices, int index, float x, float y, float z);
void Lines_BackendLittleTest(void);
bool Lines_BackendTransform(void *vertices);
void Lines_BackendRenderLine(void);
void Lines_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
