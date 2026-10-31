//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#define DEFAULT_SKIN_NAME "$$\"\""
#ifdef __cplusplus
#ifdef LIBRW
namespace rw { struct Texture;struct Clump; }
typedef rw::Texture PlayerSkinTexture;
typedef rw::Clump PlayerSkinClump;
#else
typedef struct RwTexture PlayerSkinTexture;
typedef struct RpClump PlayerSkinClump;
#endif
extern "C" {
#else
typedef struct PlayerSkinTexture PlayerSkinTexture;
typedef struct PlayerSkinClump PlayerSkinClump;
#endif
extern PlayerSkinClump *PlayerSkin_Clump;
extern float PlayerSkin_OldFov;
extern int32_t PlayerSkin_txdSlot;
void PlayerSkin_FindPlayerDff(uint32_t *offset,uint32_t *size);
void PlayerSkin_LoadPlayerDff(void);
void PlayerSkin_Initialise(void);
void PlayerSkin_Shutdown(void);
PlayerSkinTexture *PlayerSkin_GetSkinTexture(const char *name);
void PlayerSkin_BeginFrontendSkinEdit(void);
void PlayerSkin_EndFrontendSkinEdit(void);
void PlayerSkin_RenderFrontendSkinEdit(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
