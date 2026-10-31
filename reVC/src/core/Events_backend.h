//+ rouz edit (ChatGPT)
#pragma once
#include "EventList.h"
#include "Accident.h"
#include "CrimeTypes.h"
enum {EVENTS_MISSION_CHAR=2};
#ifdef __cplusplus
extern "C" {
#endif
void Events_BackendDebug(const char *message);
void Events_BackendError(const char *message);
int32_t Events_BackendPedRef(CEntity *entity);
int32_t Events_BackendVehicleRef(CEntity *entity);
int32_t Events_BackendObjectRef(CEntity *entity);
CPed * Events_BackendPedFromEntity(CEntity *entity);
EventsVector Events_BackendEntityPosition(CEntity *entity);
void Events_BackendRegisterReference(CPed *ped,CPed **slot);
CPed * Events_BackendPlayer(void);
bool Events_BackendPedValid(CPed *ped);
int32_t Events_BackendWantedLevel(void);
bool Events_BackendChased(CPed *ped);
bool Events_BackendDyingOrDead(CPed *ped);
int32_t * Events_BackendMoney(void);
EventsVector Events_BackendPlayerCoors(void);
EventsVector Events_BackendPlayerPedPosition(void);
int32_t Events_BackendPolicePresence(EventsVector position,float radius);
bool Events_BackendGermanGame(void);
void Events_BackendCrime(enum eCrimeType type,EventsVector position,uint32_t id,bool copsDontCare);
void Events_BackendImmediateCrime(enum eCrimeType type,EventsVector position,uint32_t id,bool copsDontCare);
void Events_BackendWantedNoDrop(int32_t level);
void Events_BackendPedWantedNoDrop(int32_t level);
bool Events_BackendIsPlayer(CPed *ped);
uint8_t Events_BackendCreatedBy(CPed *ped);
bool Events_BackendScorched(CPed *ped);
bool Events_BackendBodyPart(CPed *ped);
bool Events_BackendAllowRevive(CPed *ped);
bool Events_BackendInWater(CPed *ped);
bool Events_BackendHasSurface(CPed *ped);
EventsVector Events_BackendPedPosition(CPed *ped);
float Events_BackendPedZ(CPed *ped);
bool Events_BackendVerticalLine(EventsVector position,float endZ);
void Events_BackendSetLastAccident(CPed *ped,CAccident *accident);
CPed * Events_BackendPed(int32_t reference);
float Events_BackendHealth(CPed *ped);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
