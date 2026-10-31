//+ rouz edit (ChatGPT)
#pragma once
#include "Ropes.h"
#ifdef __cplusplus
class CVector;
class CCopPed;
RopesVector Ropes_BackendCopyVector(const CVector &vector);
extern "C" {
#else
typedef struct CCopPed CCopPed;
#endif
void Ropes_BackendStoreVector(void *destination, RopesVector vector);
bool Ropes_BackendEnvMap(void);
bool Ropes_BackendSphereVisible(RopesVector center, float radius);
void Ropes_BackendPush(const char *name);
void Ropes_BackendPop(void);
void Ropes_BackendSetRGBA(int index, int red, int green, int blue, int alpha);
void Ropes_BackendSetPos(int index, float x, float y, float z);
void Ropes_BackendVertexAlpha(void);
void Ropes_BackendSrcBlend(void);
void Ropes_BackendDestBlend(void);
void Ropes_BackendTextureRaster(void);
bool Ropes_BackendTransform(int count);
void Ropes_BackendRenderIndexed(bool lineList, uint16_t *indices, int count);
void Ropes_BackendEnd(void);
bool Ropes_BackendSwatLoaded(void);
CCopPed *Ropes_BackendAddSwat(RopesVector pos);
void Ropes_BackendDisableCollision(CCopPed *ped);
void Ropes_BackendRopeSentinel(CCopPed *ped);
void Ropes_BackendRopeID(CCopPed *ped, uint32_t id);
void Ropes_BackendBlendSwat(CCopPed *ped);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
