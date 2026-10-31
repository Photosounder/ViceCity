//+ rouz edit (ChatGPT)
#pragma once
#include "Wanted.h"
#include "Pool.h"
#ifdef __cplusplus
extern "C" {
#endif
bool Wanted_BackendFrenzy(void);
int32_t * Wanted_BackendAttained(void);
int32_t * Wanted_BackendEvaded(void);
CPool * Wanted_BackendPedPool(void);
CPool * Wanted_BackendVehiclePool(void);
int16_t Wanted_BackendPedModel(CPed *ped);
int16_t Wanted_BackendVehicleModel(CVehicle *vehicle);
WantedVector Wanted_BackendPedPosition(CPed *ped);
WantedVector Wanted_BackendVehiclePosition(CVehicle *vehicle);
bool Wanted_BackendLawEnforcer(CVehicle *vehicle);
uint8_t Wanted_BackendVehicleStatus(CVehicle *vehicle);
CVehicle * Wanted_BackendPlayerVehicle(void);
WantedVector Wanted_BackendPlayerCoors(void);
void Wanted_BackendClearCopState(CCopPed *cop);
bool Wanted_BackendCopDying(CCopPed *cop);
void Wanted_BackendCopWander(CCopPed *cop,float path);
WantedVector Wanted_BackendNativePosition(const CVector *value);
void Wanted_BackendError(const char *message);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
