//+ rouz edit (ChatGPT)
#pragma once
#include "PlayerSkin.h"
#include <stdbool.h>
typedef struct PlayerSkinVector {float x,y,z;} PlayerSkinVector;
typedef struct PlayerSkinColor {float red,green,blue,alpha;} PlayerSkinColor;
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*PlayerSkinMemoryCallback)(void *memory);
int PlayerSkin_BackendCompare(const char *a,const char *b);
int32_t PlayerSkin_BackendImageCount(void);
void PlayerSkin_BackendAddImage(const char *path);
void PlayerSkin_BackendReadSectors(uint8_t *buffer,uint32_t offset,uint32_t size);
void PlayerSkin_BackendSync(void);
void PlayerSkin_BackendRemoveImages(void);
uint8_t * PlayerSkin_BackendAllocate(uint32_t size,uint32_t alignment);
void PlayerSkin_BackendFree(uint8_t *buffer);
void * PlayerSkin_BackendOpenStream(void *memory);
bool PlayerSkin_BackendFindChunk(void *stream);
PlayerSkinClump * PlayerSkin_BackendReadClump(void *stream);
void PlayerSkin_BackendCloseStream(void *stream,void *memory);
int32_t PlayerSkin_BackendAddSlot(const char *name);
void PlayerSkin_BackendCreateDictionary(int32_t slot);
void PlayerSkin_BackendAddRef(int32_t slot);
void PlayerSkin_BackendRemoveSlot(int32_t slot);
void PlayerSkin_BackendPushDictionary(void);
void PlayerSkin_BackendSetDictionary(int32_t slot);
void PlayerSkin_BackendPopDictionary(void);
PlayerSkinTexture * PlayerSkin_BackendReadTexture(const char *name);
char * PlayerSkin_BackendString(void);
void * PlayerSkin_BackendReadImage(const char *path);
void PlayerSkin_BackendRasterFormat(void *image,int32_t *width,int32_t *height,int32_t *depth,int32_t *format);
void * PlayerSkin_BackendCreateRaster(int32_t width,int32_t height,int32_t depth,int32_t format);
void PlayerSkin_BackendRasterImage(void *raster,void *image);
PlayerSkinTexture * PlayerSkin_BackendCreateTexture(void *raster);
void PlayerSkin_BackendTextureName(PlayerSkinTexture *texture,const char *name);
void PlayerSkin_BackendTextureFilter(PlayerSkinTexture *texture);
void PlayerSkin_BackendAddTexture(int32_t slot,PlayerSkinTexture *texture);
void PlayerSkin_BackendDestroyImage(void *image);
void PlayerSkin_BackendSetRenderers(PlayerSkinClump *clump);
void PlayerSkin_BackendLoadPlayer(void);
void PlayerSkin_BackendDestroyClump(PlayerSkinClump *clump);
void * PlayerSkin_BackendFrame(PlayerSkinClump *clump);
void PlayerSkin_BackendTransform(void *frame);
void PlayerSkin_BackendTranslate(void *frame,PlayerSkinVector value);
void PlayerSkin_BackendRotate(void *frame,PlayerSkinVector value,float angle);
void PlayerSkin_BackendUpdateObjects(void *frame);
void PlayerSkin_BackendAmbient(PlayerSkinColor value);
void PlayerSkin_BackendRenderClump(PlayerSkinClump *clump);
void PlayerSkin_BackendWithMemory(uint8_t *buffer,uint32_t length,PlayerSkinMemoryCallback callback);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
