//+ rouz edit (ChatGPT)
#pragma once
#include "Fire.h"
#ifdef __cplusplus
extern "C" {
#endif
FireVector Fire_BackendReadVector(const void *position);
FireVector Fire_BackendPosition(CEntity *entity);
bool Fire_BackendIsPed(CEntity *entity);
bool Fire_BackendIsVehicle(CEntity *entity);
bool Fire_BackendIsCar(CEntity *entity);
CFire * Fire_BackendPedFire(CEntity *entity);
CFire * Fire_BackendVehicleFire(CEntity *entity);
void Fire_BackendSetPedFire(CEntity *entity,CFire *fire);
void Fire_BackendSetVehicleFire(CEntity *entity,CFire *fire);
int32_t Fire_BackendMoveState(CEntity *entity);
bool Fire_BackendInVehicle(CEntity *entity);
CEntity * Fire_BackendMyVehicle(CEntity *entity);
void Fire_BackendVehicleHealth(CEntity *entity,float health);
CEntity * Fire_BackendPlayerPed(void);
CEntity * Fire_BackendPlayerVehicle(void);
bool Fire_BackendPedDamage(CEntity *entity,CEntity *source,float damage);
void Fire_BackendVehicleDamage(CEntity *entity,CEntity *source,float damage);
void Fire_BackendScorched(CEntity *entity);
CEntity * Fire_BackendShortcutTaxi(void);
bool Fire_BackendShortcutTransition(void);
bool Fire_BackendFireProof(CEntity *entity);
void Fire_BackendPrepareOnFire(CEntity *entity);
int32_t Fire_BackendModel(CEntity *entity);
FireVector Fire_BackendHeadlights(int32_t model);
FireVector Fire_BackendTransform(CEntity *entity,FireVector position);
void Fire_BackendFlame(FireVector position,FireVector velocity,float size);
void Fire_BackendSmoke(FireVector position);
void Fire_BackendSteam(FireVector position,float upward,float size);
void Fire_BackendShadow(uintptr_t id,FireVector position,int16_t random);
void Fire_BackendReport(FireVector position);
bool Fire_BackendCanSetState(CEntity *entity);
int32_t Fire_BackendPedState(CEntity *entity);
bool Fire_BackendIsPlayer(CEntity *entity);
void Fire_BackendIdle(CEntity *entity);
void Fire_BackendLastStateNone(CEntity *entity);
void Fire_BackendWander(CEntity *entity);
void Fire_BackendWaitFinish(CEntity *entity);
bool Fire_BackendInControl(CEntity *entity);
uint8_t Fire_BackendEngineStatus(CEntity *entity);
void Fire_BackendSetEngineStatus(CEntity *entity,uint8_t status);
void Fire_BackendFleeEntity(CEntity *entity,CEntity *source);
void Fire_BackendFleePosition(CEntity *entity,FireVector position);
void Fire_BackendClearFleeFrom(CEntity *entity);
uint32_t * Fire_BackendFleeTimer(CEntity *entity);
void Fire_BackendDrawLastOff(CEntity *entity);
void Fire_BackendSprint(CEntity *entity);
void Fire_BackendMoveAnim(CEntity *entity);
void Fire_BackendOnFireState(CEntity *entity);
int32_t Fire_BackendPedType(CEntity *entity);
void Fire_BackendCopEvent(CEntity *entity,CEntity *source);
void Fire_BackendPedEvent(CEntity *entity,CEntity *source);
void Fire_BackendVehicleEvent(CEntity *entity,CEntity *source);
void Fire_BackendFleeOccupants(CEntity *entity);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
