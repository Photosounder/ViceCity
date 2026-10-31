//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include "References.h"
#include "Pool.h"
#ifdef __cplusplus
class CPlayerPed;
extern "C" {
#else
typedef struct CPlayerPed CPlayerPed;
#endif
bool References_BackendIsBuilding(CEntity *entity);
CReference **References_BackendFirst(CEntity *entity);
CEntity *References_BackendPlayerVehicle(void);
CPlayerPed *References_BackendPlayerPed(void);
CEntity *References_BackendPlayerEntity(CPlayerPed *ped);
void References_BackendRestorePlayer(CPlayerPed *ped);
CEntity **References_BackendPlayerSlot(void);
CPool *References_BackendPedPool(void);
CPool *References_BackendVehiclePool(void);
CPool *References_BackendObjectPool(void);
CEntity *References_BackendPed(void *slot);
CEntity *References_BackendVehicle(void *slot);
CEntity *References_BackendObject(void *slot);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
