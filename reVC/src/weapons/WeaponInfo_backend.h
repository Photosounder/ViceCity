//+ rouz edit (ChatGPT)
#pragma once
#include "WeaponInfo.h"
typedef uint8_t WeaponInfoWorkBuffer[55000];
#ifdef __cplusplus
extern "C" {
#endif
WeaponInfoWorkBuffer * WeaponInfo_BackendBuffer(void);
bool WeaponInfo_BackendIsShotgun(int32_t weapon);
void WeaponInfo_BackendSetModelWeapon(int32_t model,int32_t weapon);
const char * WeaponInfo_BackendAnimGroupName(enum AssocGroupId group);
void WeaponInfo_BackendInitialising(void);
void WeaponInfo_BackendLoading(void);
void WeaponInfo_BackendReady(void);
void WeaponInfo_BackendShuttingDown(void);
void WeaponInfo_BackendShutDown(void);
void WeaponInfo_BackendUnknownFireType(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
