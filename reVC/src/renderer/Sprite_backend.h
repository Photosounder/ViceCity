//+ rouz edit (ChatGPT)
#pragma once
#include "Sprite.h"
#include "RwHelper_backend.h"
#ifdef __cplusplus
extern "C" {
#endif
int32_t Sprite_HostScreenWidth(void);
int32_t Sprite_HostScreenHeight(void);
float Sprite_HostFrontX(void);
float Sprite_HostFrontY(void);
float Sprite_HostDefaultFOV(void);
void * Sprite_HostSceneCamera(void);
float Sprite_HostNearClip(void *camera);
float Sprite_HostNearScreenZ(void);
float Sprite_HostFarScreenZ(void);
void * Sprite_HostBufferedVertices(void);
void * Sprite_HostImmediateVertices(void);
void Sprite_HostDraw(int32_t kind,void *vertices,int32_t count);
SpriteVectorRef Sprite_HostCameraPosition(void);
SpriteVector Sprite_HostViewTransform(const SpriteVector *source);
#ifdef REVC_SOFTWARE_POLYGONS
bool Sprite_HostCapturingWorldEffects(void);
#endif
enum {SPRITE_CONST_rwPRIMTYPETRIFAN,SPRITE_CONST_rwPRIMTYPETRILIST,SPRITE_CONST_rwRENDERSTATEZTESTENABLE};
int32_t Sprite_HostConstant(int32_t identity);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
