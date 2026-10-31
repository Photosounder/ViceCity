//+ rouz edit (ChatGPT)
#pragma once
#include "EventsShared.h"
typedef enum eEventType
{
	EVENT_NULL,
	EVENT_ASSAULT,
	EVENT_RUN_REDLIGHT,
	EVENT_ASSAULT_POLICE,
	EVENT_GUNSHOT,
	EVENT_INJURED_PED,
	EVENT_DEAD_PED,
	EVENT_FIRE,
	EVENT_STEAL_CAR,
	EVENT_HIT_AND_RUN,
	EVENT_HIT_AND_RUN_COP,
	EVENT_SHOOT_PED,
	EVENT_SHOOT_COP,
	EVENT_EXPLOSION,
	EVENT_PED_SET_ON_FIRE,
	EVENT_COP_SET_ON_FIRE,
	EVENT_CAR_SET_ON_FIRE,
	EVENT_ASSAULT_NASTYWEAPON,
	EVENT_ASSAULT_NASTYWEAPON_POLICE,
	EVENT_UNK, // Not on SA it seems
	EVENT_ICECREAM,
	EVENT_ATM,
	EVENT_SHOPSTALL,
	EVENT_SHOPWINDOW,
	EVENT_LAST_EVENT
} eEventType;
typedef enum eEventEntity
{
	EVENT_ENTITY_NONE,
	EVENT_ENTITY_PED,
	EVENT_ENTITY_VEHICLE,
	EVENT_ENTITY_OBJECT
} eEventEntity;
typedef struct CEvent {eEventType type;eEventEntity entityType;int32_t entityRef;CPed *criminal;EventsVector posn;uint32_t timeout;int32_t state;} CEvent;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Events_FirstFreeSlotIndex;
extern CEvent gaEvent[NUMEVENTS];
void Events_Initialise(void);
void Events_Update(void);
void Events_RegisterEntity(eEventType type,eEventEntity entityType,CEntity *entity,CPed *criminal,int32_t timeout);
void Events_RegisterPosition(eEventType type,EventsVector position,int32_t timeout);
bool Events_GetEvent(eEventType type,int32_t *event);
void Events_ClearEvent(int32_t event);
bool Events_FindClosestEvent(eEventType type,EventsVector position,int32_t *event);
void Events_ReportCrimeForEvent(eEventType type,intptr_t crimeId,bool copsDontCare);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
