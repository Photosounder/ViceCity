//+ rouz edit (ChatGPT)
#pragma once
#include "Skidmarks.h"
#ifdef __cplusplus
class CVector;
const void *Skidmarks_BackendPosition(const CVector &position);
extern "C" {
#endif
SkidmarksVector Skidmarks_BackendReadPosition(const void *position);
void Skidmarks_BackendTxdPush(void);
int Skidmarks_BackendTxdFind(const char *name);
void Skidmarks_BackendTxdSet(int slot);
void Skidmarks_BackendTxdPop(void);
void Skidmarks_BackendZWrite(bool enabled);
void Skidmarks_BackendZTest(void);
void Skidmarks_BackendVertexAlpha(bool enabled);
void Skidmarks_BackendSrcBlend(void);
void Skidmarks_BackendDestBlend(void);
void Skidmarks_BackendSetRGBA(int index, uint8_t red, uint8_t green, uint8_t blue, uint32_t alpha);
void Skidmarks_BackendSetPos(int index, float x, float y, float z);
void Skidmarks_BackendSetU(int index, float value);
void Skidmarks_BackendSetV(int index, float value);
void Skidmarks_BackendLittleTest(void);
bool Skidmarks_BackendTransform(int count);
void Skidmarks_BackendRenderIndexed(uint16_t *indices, int count);
void Skidmarks_BackendEnd(void);
void Skidmarks_BackendPush(const char *name);
void Skidmarks_BackendPop(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
