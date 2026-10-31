//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
typedef struct GPUcaps {uint32_t version,platform,subplatform,dxtSupport;} GPUcaps;
#ifdef __cplusplus
extern "C" {
#endif
extern float texLoadTime;
extern int32_t texNumLoaded;
void *TexRead_ReadTexture(void *stream);
void *TexRead_DestroyTexture(void *texture,void *data);
void *TexRead_ReadDictionary(void *stream);
void *TexRead_ReadDictionary1(void *stream);
void *TexRead_ReadDictionary2(void *stream,void *dictionary);
#ifdef GTA_PC
#ifdef LIBRW
void TexRead_ReadVideoCardCapsFile(GPUcaps *caps);
#else
void TexRead_ReadVideoCardCapsFile(uint32_t *cap32,uint32_t *cap24,uint32_t *cap16,uint32_t *cap8);
#endif
bool TexRead_CheckVideoCardCaps(void);
void TexRead_WriteVideoCardCapsFile(void);
bool TexRead_CanVideoCardDoDXT(void);
void TexRead_ConvertingTexturesScreen(uint32_t num,uint32_t count,const char *text);
void TexRead_DealWithTxdWriteError(uint32_t num,uint32_t count,const char *text);
bool TexRead_CreateTxdImageForVideoCard(void);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
