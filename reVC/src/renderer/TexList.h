//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
#ifdef LIBRW
namespace rw { struct Texture; }
typedef rw::Texture TexListTexture;
#else
typedef struct RwTexture TexListTexture;
#endif
extern "C" {
#else
typedef struct TexListTexture TexListTexture;
#endif
enum { TEXLIST_MAX_TEXUSED=400 };
extern bool TexList_ms_nTexUsed[TEXLIST_MAX_TEXUSED];
void TexList_Initialise(void);
void TexList_Shutdown(void);
TexListTexture *TexList_SetTexture(int32_t slot,char *name);
int32_t TexList_GetFirstFreeTexture(void);
TexListTexture *TexList_LoadFileNameTexture(char *name);
void TexList_LoadGlobalTextureList(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
