//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "Rect.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef void *(*TexReadTextureVisitor)(void *texture,void *data);
#ifdef LIBRW
typedef int32_t TexReadStreamPosition;
#else
typedef uint32_t TexReadStreamPosition;
#endif
bool TexRead_HostReadNative(void *stream,void **texture,uint32_t size);
void TexRead_HostForAllTextures(void *dictionary,TexReadTextureVisitor visitor,void *data);
bool TexRead_HostFindChunk(void *stream,bool native,uint32_t *size,uint32_t *version);
bool TexRead_HostPlayingGame(void);
void TexRead_HostDestroyTexture(void *texture);
void * TexRead_HostCreateDictionary(void);
void TexRead_HostDestroyDictionary(void *dictionary);
void TexRead_HostAddTexture(void *dictionary,void *texture);
uint32_t TexRead_HostReadStream(void *stream,void *buffer,uint32_t size);
void TexRead_HostSkipStream(void *stream,uint32_t offset);
TexReadStreamPosition TexRead_HostPosition(void *stream);
int32_t TexRead_HostMaxAnisotropy(void);
void TexRead_HostSetAnisotropy(void *texture,int32_t level);
uint32_t TexRead_HostPlatform(void);
uint32_t TexRead_HostGLSubplatform(void);
uint32_t TexRead_HostGLDXT(void);
uint32_t TexRead_HostRaster8888(void);
uint32_t TexRead_HostRaster888(void);
uint32_t TexRead_HostRaster1555(void);
uint32_t TexRead_HostRasterPAL8(void);
bool TexRead_HostDXT1(void);
bool TexRead_HostDXT3(void);
void TexRead_HostHandleExit(void);
void * TexRead_HostLoadSplash(void);
bool TexRead_HostStartFrame(void);
void TexRead_HostRecipNearClip(void);
void TexRead_HostSpriteInit(void);
void TexRead_HostFontInit(void);
void TexRead_HostDefinedState(void);
void TexRead_HostAddressClamp(void);
void TexRead_HostSplashDraw(void *splash,CRect rect,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void TexRead_HostDrawRect(CRect rect,uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void TexRead_HostFontBackgroundOff(void);
void TexRead_HostFontPropOn(void);
void TexRead_HostFontScale(float x,float y);
void TexRead_HostFontCentreOff(void);
void TexRead_HostFontWrap(float x);
void TexRead_HostFontJustifyOff(void);
void TexRead_HostFontColor(uint8_t r,uint8_t g,uint8_t b,uint8_t a);
void TexRead_HostFontBackgroundTextOff(void);
void TexRead_HostFontStyle(int16_t style);
void TexRead_HostFontPrint(float x,float y,uint16_t *text);
void TexRead_HostDrawFonts(void);
void TexRead_HostEndFrame(void);
float TexRead_HostScreenWidth(void);
float TexRead_HostScreenHeight(void);
bool TexRead_HostQuit(void);
void TexRead_HostSetQuit(bool value);
void TexRead_HostUpdatePads(void);
bool TexRead_HostEscape(void);
void TexRead_HostLoadingScreen(void);
void TexRead_HostFlushRequests(void);
void * TexRead_HostOpenImage(void);
void TexRead_HostCloseStream(void *stream);
int64_t TexRead_HostOSVersion(void);
void TexRead_HostReadBack(bool value);
int8_t TexRead_HostVsync(void);
int8_t TexRead_HostFrameLimiter(void);
void TexRead_HostSetVsync(int8_t value);
void TexRead_HostSetFrameLimiter(int8_t value);
bool TexRead_HostInImage(int32_t id);
void TexRead_HostRequestTxd(int32_t slot,int32_t flags);
void TexRead_HostRequestModelStream(int32_t channel);
void TexRead_HostFlushChannels(void);
int32_t TexRead_HostTell(void *stream);
void * TexRead_HostFileInterface(void);
int32_t TexRead_HostLegacyTell(void *filesys,void *stream);
const void * TexRead_HostWriteDictionary(void *dictionary,void *stream);
void TexRead_HostWriteStream(void *stream,const void *buffer,uint32_t size);
void TexRead_HostRemoveTxd(int32_t slot);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
