//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifndef LIBRW
#include <d3d8.h>
typedef struct CTexturePool {
    D3DFORMAT Format;
    int size;
    uint32_t levels;
    int32_t texturesMax,texturesUsed,texturesNum;
    IDirect3DTexture8 **pTextures;
} CTexturePool;
typedef struct CPaletteList {int Max,Num;int *Data;} CPaletteList;
#ifdef __cplusplus
extern "C" {
#endif
extern CTexturePool aTexturePools[12];
extern CPaletteList PaletteList;
extern int numTexturePools,MaxPaletteIndex;
extern bool bUsePaletteIndex;
void CTexturePool_Create(CTexturePool *self,D3DFORMAT _Format,int _size,uint32_t mipmapLevels,int32_t numTextures);
void CTexturePool_Release(CTexturePool *self);
IDirect3DTexture8 * CTexturePool_FindTexture(CTexturePool *self);
bool CTexturePool_AddTexture(CTexturePool *self,IDirect3DTexture8 *texture);
void CTexturePool_Resize(CTexturePool *self,int numTextures);
void CPaletteList_Alloc(CPaletteList *self,int max);
void CPaletteList_Free(CPaletteList *self);
int CPaletteList_Find(CPaletteList *self);
void CPaletteList_Add(CPaletteList *self,int item);
void CPaletteList_Resize(CPaletteList *self,int max);
#ifdef FIX_BUGS
int CTexturePool_GetSize(const CTexturePool *self);
#else
float CTexturePool_GetSize(const CTexturePool *self);
#endif
HRESULT TexturePools_CreateTexture(int width,int height,int levels,D3DFORMAT Format,IDirect3DTexture8 **texture);
void TexturePools_ReleaseTexture(IDirect3DTexture8 *texture);
int TexturePools_FindAvailablePaletteIndex(void);
void TexturePools_AddAvailablePaletteIndex(int index);
void TexturePools_Initialise(void);
void TexturePools_Shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
