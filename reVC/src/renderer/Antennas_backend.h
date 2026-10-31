//+ rouz edit (ChatGPT)
#pragma once
#include "Antennas.h"
#ifdef __cplusplus
extern "C" {
#endif
AntennasVector Antennas_BackendVector(const void *vector);
void Antennas_BackendPush(const char *name);
void Antennas_BackendPop(void);
void Antennas_BackendZWrite(void);
void Antennas_BackendVertexAlpha(bool enabled);
void Antennas_BackendSrcBlend(void);
void Antennas_BackendDestBlend(void);
void Antennas_BackendTextureRaster(void);
void Antennas_BackendSetRGBA(int index, int red, int green, int blue, int alpha);
void Antennas_BackendSetPos(int index, float x, float y, float z);
bool Antennas_BackendTransform(void);
void Antennas_BackendRenderLine(void);
void Antennas_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
