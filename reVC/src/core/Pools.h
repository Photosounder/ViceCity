//+ rouz edit (ChatGPT)
#pragma once

//+ rouz edit (ChatGPT)
#include "Pool.h"
#ifdef __cplusplus
extern "C" {
#endif
CPool *CPools_GetAudioScriptObjectPool(void);
CPool *CPools_GetPtrNodePool(void);
CPool *CPools_GetEntryInfoNodePool(void);
CPool *CPools_GetPedPool(void); // rouz edit (ChatGPT)
CPool *CPools_GetVehiclePool(void); // rouz edit (ChatGPT)
CPool *CPools_GetObjectPool(void); // rouz edit (ChatGPT)
#ifdef __cplusplus
}
//- rouz edit (ChatGPT)

#include "templates.h"
#include "Lists.h"
#include "Treadable.h"
#include "Object.h"
#include "CutsceneObject.h"
#include "PlayerPed.h"
#include "Automobile.h"
#include "DummyPed.h"
#include "AudioScriptObject.h"

//+ rouz edit (ChatGPT)
typedef CPool CCPtrNodePool;
typedef CPool CEntryInfoNodePool;
typedef CPool CPedPool;
typedef CPool CVehiclePool;
typedef CPool CBuildingPool;
typedef CPool CTreadablePool;
typedef CPool CObjectPool;
typedef CPool CDummyPool;
typedef CPool CAudioScriptObjectPool;
typedef CPool CColModelPool;
//- rouz edit (ChatGPT)

//+ rouz edit (Claude)
// Formerly the static class CPools, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern CCPtrNodePool * Pools_ms_pPtrNodePool;
extern CEntryInfoNodePool * Pools_ms_pEntryInfoNodePool;
extern CPedPool * Pools_ms_pPedPool;
extern CVehiclePool * Pools_ms_pVehiclePool;
extern CBuildingPool * Pools_ms_pBuildingPool;
extern CTreadablePool * Pools_ms_pTreadablePool;
extern CObjectPool * Pools_ms_pObjectPool;
extern CDummyPool * Pools_ms_pDummyPool;
extern CAudioScriptObjectPool * Pools_ms_pAudioScriptObjectPool;
extern CColModelPool * Pools_ms_pColModelPool;
void Pools_Initialise(void);
void Pools_ShutDown(void);
int32 Pools_GetPedRef(CPed *ped);
CPed *Pools_GetPed(int32 handle);
int32 Pools_GetVehicleRef(CVehicle *vehicle);
CVehicle *Pools_GetVehicle(int32 handle);
int32 Pools_GetObjectRef(CObject *object);
CObject *Pools_GetObject(int32 handle);
void Pools_CheckPoolsEmpty();
void Pools_MakeSureSlotInObjectPoolIsEmpty(int32 slot);
void Pools_LoadObjectPool(uint8 *buf, uint32 size);
void Pools_LoadPedPool(uint8 *buf, uint32 size);
void Pools_LoadVehiclePool(uint8 *buf, uint32 size);
void Pools_SaveObjectPool(uint8 *buf, uint32 *size);
void Pools_SavePedPool(uint8 *buf, uint32 *size);
void Pools_SaveVehiclePool(uint8 *buf, uint32 *size);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
static inline CCPtrNodePool *Pools_GetPtrNodePool(void);
static inline CEntryInfoNodePool *Pools_GetEntryInfoNodePool(void);
static inline CPedPool *Pools_GetPedPool(void);
static inline CVehiclePool *Pools_GetVehiclePool(void);
static inline CBuildingPool *Pools_GetBuildingPool(void);
static inline CTreadablePool *Pools_GetTreadablePool(void);
static inline CObjectPool *Pools_GetObjectPool(void);
static inline CDummyPool *Pools_GetDummyPool(void);
static inline CAudioScriptObjectPool *Pools_GetAudioScriptObjectPool(void);
static inline CColModelPool *Pools_GetColModelPool(void);
static inline CCPtrNodePool *Pools_GetPtrNodePool(void) { return Pools_ms_pPtrNodePool; }
static inline CEntryInfoNodePool *Pools_GetEntryInfoNodePool(void) { return Pools_ms_pEntryInfoNodePool; }
static inline CPedPool *Pools_GetPedPool(void) { return Pools_ms_pPedPool; }
static inline CVehiclePool *Pools_GetVehiclePool(void) { return Pools_ms_pVehiclePool; }
static inline CBuildingPool *Pools_GetBuildingPool(void) { return Pools_ms_pBuildingPool; }
static inline CTreadablePool *Pools_GetTreadablePool(void) { return Pools_ms_pTreadablePool; }
static inline CObjectPool *Pools_GetObjectPool(void) { return Pools_ms_pObjectPool; }
static inline CDummyPool *Pools_GetDummyPool(void) { return Pools_ms_pDummyPool; }
static inline CAudioScriptObjectPool *Pools_GetAudioScriptObjectPool(void) { return Pools_ms_pAudioScriptObjectPool; }
static inline CColModelPool *Pools_GetColModelPool(void) { return Pools_ms_pColModelPool; }
#endif
//- rouz edit (Claude)
#endif // rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
