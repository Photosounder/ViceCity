//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void BulletTraces_BackendPushTxd(void);
int32_t BulletTraces_BackendFindTxd(const char *name);
void BulletTraces_BackendSetTxd(int32_t slot);
void BulletTraces_BackendPopTxd(void);
void Clouds_BackendPushTxd(void);
int32_t Clouds_BackendFindTxd(const char *name);
void Clouds_BackendSetTxd(int32_t slot);
void Clouds_BackendPopTxd(void);
int32_t Markers3D_BackendFindTxd(const char *name);
void Markers3D_BackendPushTxd(void);
void Markers3D_BackendSetTxd(int32_t slot);
void Markers3D_BackendPopTxd(void);
int32_t PlayerSkin_BackendAddSlot(const char *name);
void PlayerSkin_BackendCreateDictionary(int32_t slot);
void PlayerSkin_BackendAddRef(int32_t slot);
void PlayerSkin_BackendRemoveSlot(int32_t slot);
void PlayerSkin_BackendPushDictionary(void);
void PlayerSkin_BackendSetDictionary(int32_t slot);
void PlayerSkin_BackendPopDictionary(void);
void Rubbish_BackendTxdPush(void);
int Rubbish_BackendTxdFind(const char *name);
void Rubbish_BackendTxdSet(int slot);
void Rubbish_BackendTxdPop(void);
void Skidmarks_BackendTxdPush(void);
int Skidmarks_BackendTxdFind(const char *name);
void Skidmarks_BackendTxdSet(int slot);
void Skidmarks_BackendTxdPop(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
