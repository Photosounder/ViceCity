//+ rouz edit (ChatGPT)
#include "Events_backend.h"
#include "Timer.h"
#include "Messages.h"
#include "Text.h"
#include <math.h>
#include <stddef.h>
int32_t Events_FirstFreeSlotIndex;
CEvent gaEvent[NUMEVENTS];
CAccidentManager gAccidentManager;
enum {EVENT_STATE_0,EVENT_STATE_CANDELETE,EVENT_STATE_CLEAR};
static EventsVector Events_Sub(EventsVector a,EventsVector b)
{
    // Preserve all three original vector subtractions before selecting a distance norm
    EventsVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float Events_Magnitude(EventsVector value)
{
    // Preserve the original ordered three-component squared length and float square root
    return sqrtf(value.x*value.x+value.y*value.y+value.z*value.z);
}
static float Events_Magnitude2D(EventsVector value)
{
    // Preserve the original two-component squared length without changing the vertical eligibility test
    return sqrtf(value.x*value.x+value.y*value.y);
}
void Accident_Init(CAccident *self)
{
    // Preserve the original constructor's named-field initialization in declaration order
    self->m_pVictim=NULL;self->m_nMedicsAttending=0;self->m_nMedicsPerformingCPR=0;
}
void AccidentManager_Init(CAccidentManager *self)
{
    // Construct all twenty original accident subrecords without clearing unrelated bytes
    for(int i=0;i<NUM_ACCIDENTS;i++)Accident_Init(&self->m_aAccidents[i]);
}


void
Events_Initialise(void)
{
    // Preserve the complete original Initialise policy and all ordered native accesses
	int i;

	Events_BackendDebug("Initialising CEventList...");
	for(i = 0; i < NUMEVENTS; i++){
		gaEvent[i].type = EVENT_NULL;
		gaEvent[i].entityType = EVENT_ENTITY_NONE;
		gaEvent[i].entityRef = 0;
		gaEvent[i].posn.x = 0.0f;
		gaEvent[i].posn.y = 0.0f;
		gaEvent[i].posn.z = 0.0f;
		gaEvent[i].timeout = 0;
		gaEvent[i].state = EVENT_STATE_0;
	}
	Events_FirstFreeSlotIndex = 0;
}

void
Events_Update(void)
{
    // Preserve the complete original Update policy and all ordered native accesses
	int i;

	Events_FirstFreeSlotIndex = 0;
	for(i = 0; i < NUMEVENTS; i++){
		if(gaEvent[i].type == EVENT_NULL)
			continue;
		if((*Timer_GetTimeInMilliseconds()) > gaEvent[i].timeout || gaEvent[i].state == EVENT_STATE_CANDELETE){
			gaEvent[i].type = EVENT_NULL;
			gaEvent[i].state = EVENT_STATE_0;
		}
		if(gaEvent[i].state == EVENT_STATE_CLEAR)
			gaEvent[i].state = EVENT_STATE_CANDELETE;
	}
}

void
Events_RegisterEntity(eEventType type, eEventEntity entityType, CEntity *ent, CPed *criminal, int32_t timeout)
{
    // Preserve the complete original RegisterEntity policy and all ordered native accesses
	int i;
	int ref;
	bool copsDontCare;

#ifdef SQUEEZE_PERFORMANCE
	if (type == EVENT_INJURED_PED) {
		AccidentManager_ReportAccident(&gAccidentManager,Events_BackendPedFromEntity(ent));
		return;
	}
#endif

	copsDontCare = false;
	switch(entityType){
	case EVENT_ENTITY_PED:
		ref = Events_BackendPedRef(ent);
		break;
	case EVENT_ENTITY_VEHICLE:
		ref = Events_BackendVehicleRef(ent);
		break;
	case EVENT_ENTITY_OBJECT:
		ref = Events_BackendObjectRef(ent);
		break;
	default:
		Events_BackendError("Undefined entity type, RegisterEvent, EventList.cpp");
		ref = 0;
		break;
	}

	// only update time if event exists already
	for(i = 0; i < NUMEVENTS; i++)
		if(gaEvent[i].type == type &&
		   gaEvent[i].entityType == entityType &&
		   gaEvent[i].entityRef == ref){
			gaEvent[i].timeout = (*Timer_GetTimeInMilliseconds()) + timeout;
			return;
		}

	for(i = Events_FirstFreeSlotIndex; i < NUMEVENTS; i++)
		if(gaEvent[i].type == EVENT_NULL){
			Events_FirstFreeSlotIndex = i;
			break;
		}
	if(i < NUMEVENTS){
		gaEvent[i].type = type;
		gaEvent[i].entityType = entityType;
		gaEvent[i].timeout = (*Timer_GetTimeInMilliseconds()) + timeout;
		gaEvent[i].entityRef = ref;
		gaEvent[i].posn = Events_BackendEntityPosition(ent);
		gaEvent[i].criminal = criminal;
		if(gaEvent[i].criminal)
			Events_BackendRegisterReference(gaEvent[i].criminal,&gaEvent[i].criminal);
		if(type == EVENT_GUNSHOT)
			gaEvent[i].state = EVENT_STATE_CLEAR;
		else
			gaEvent[i].state = EVENT_STATE_0;
	}

	if(criminal == Events_BackendPlayer())
		Events_ReportCrimeForEvent(type, (intptr_t)ent, copsDontCare);
}

void
Events_RegisterPosition(eEventType type, EventsVector posn, int32_t timeout)
{
    // Preserve the complete original RegisterPosition policy and all ordered native accesses
	int i;

	// only update time if event exists already
	for(i = 0; i < NUMEVENTS; i++)
		if(gaEvent[i].type == type &&
		   gaEvent[i].posn.x == posn.x &&
		   gaEvent[i].posn.y == posn.y &&
		   gaEvent[i].posn.z == posn.z &&
		   gaEvent[i].entityType == EVENT_ENTITY_NONE){
			gaEvent[i].timeout = (*Timer_GetTimeInMilliseconds()) + timeout;
			return;
		}

	for(i = Events_FirstFreeSlotIndex; i < NUMEVENTS; i++)
		if(gaEvent[i].type == EVENT_NULL){
			Events_FirstFreeSlotIndex = i;
			break;
		}
	if(i < NUMEVENTS){
		gaEvent[i].type = type;
		gaEvent[i].entityType = EVENT_ENTITY_NONE;
		gaEvent[i].timeout = (*Timer_GetTimeInMilliseconds()) + timeout;
		gaEvent[i].posn = posn;
		gaEvent[i].entityRef = 0;
		if(type == EVENT_GUNSHOT)
			gaEvent[i].state = EVENT_STATE_CLEAR;
		else
			gaEvent[i].state = EVENT_STATE_0;
	}
}

bool
Events_GetEvent(eEventType type, int32_t *event)
{
    // Preserve the complete original GetEvent policy and all ordered native accesses
	int i;
	for(i = 0; i < NUMEVENTS; i++)
		if(gaEvent[i].type == type){
			*event = i;
			return true;
		}
	return false;
}

void
Events_ClearEvent(int32_t event)
{
    // Preserve the complete original ClearEvent policy and all ordered native accesses
	if(gaEvent[event].state != EVENT_STATE_CANDELETE)
		gaEvent[event].state = EVENT_STATE_CLEAR;
}

bool
Events_FindClosestEvent(eEventType type, EventsVector posn, int32_t *event)
{
    // Preserve the complete original FindClosestEvent policy and all ordered native accesses
	int i;
	float dist;
	bool found = false;
	float mindist = 60.0f;

	for(i = 0; i < NUMEVENTS; i++){
		if(gaEvent[i].type != type)
			continue;
		dist = Events_Magnitude(Events_Sub(posn,gaEvent[i].posn));
		if(dist < mindist){
			mindist = dist;
			found = true;
			*event = i;
		}
	}
	return found;
}

void
Events_ReportCrimeForEvent(eEventType type, intptr_t crimeId, bool copsDontCare)
{
    // Preserve the complete original ReportCrimeForEvent policy and all ordered native accesses
	enum eCrimeType crime;
	switch(type){
	case EVENT_ASSAULT: crime = CRIME_HIT_PED; break;
	case EVENT_RUN_REDLIGHT: crime = CRIME_RUN_REDLIGHT; break;
	case EVENT_ASSAULT_POLICE: crime = CRIME_HIT_COP; break;
	case EVENT_GUNSHOT: crime = CRIME_POSSESSION_GUN; break;
	case EVENT_STEAL_CAR: crime = CRIME_STEAL_CAR; break;
	case EVENT_HIT_AND_RUN: crime = CRIME_RUNOVER_PED; break;
	case EVENT_HIT_AND_RUN_COP: crime = CRIME_RUNOVER_COP; break;
	case EVENT_SHOOT_PED: crime = CRIME_SHOOT_PED; break;
	case EVENT_SHOOT_COP: crime = CRIME_SHOOT_COP; break;
	case EVENT_EXPLOSION: crime = CRIME_EXPLOSION; break;
	case EVENT_PED_SET_ON_FIRE: crime = CRIME_PED_BURNED; break;
	case EVENT_COP_SET_ON_FIRE: crime = CRIME_COP_BURNED; break;
	case EVENT_CAR_SET_ON_FIRE: crime = CRIME_VEHICLE_BURNED; break;
	case EVENT_ASSAULT_NASTYWEAPON: crime = CRIME_HIT_PED_NASTYWEAPON; break;
	case EVENT_ASSAULT_NASTYWEAPON_POLICE: crime = CRIME_HIT_COP_NASTYWEAPON; break;
	default: crime = CRIME_NONE; break;
	}
	
	if (crime == CRIME_HIT_PED && Events_BackendPedValid((CPed*)crimeId) && Events_BackendWantedLevel() == 0 && Events_BackendChased((CPed*)crimeId)) {
		if (!Events_BackendDyingOrDead((CPed*)crimeId)) {
			Messages_AddBigMessage(Text_Get(&TheText,"GOODBOY"), 5000, 0);
			(*Events_BackendMoney()) += 50;
		}
		return;
	}

	if(crime == CRIME_NONE)
		return;

#ifdef FIX_BUGS
	EventsVector playerPedCoors = Events_BackendPlayerCoors();
#else
	EventsVector playerPedCoors = Events_BackendPlayerPedPosition();
#endif
	EventsVector playerCoors = Events_BackendPlayerCoors();

	if(Events_BackendPolicePresence(playerCoors, 14.0f) != 0 ||
		Events_BackendGermanGame() && (crime == CRIME_SHOOT_PED || crime == CRIME_SHOOT_COP || crime == CRIME_COP_BURNED || crime == CRIME_VEHICLE_BURNED)){
		Events_BackendImmediateCrime(crime, playerPedCoors, (uint32_t)crimeId, copsDontCare);
		Events_BackendWantedNoDrop(1);
	}else
		Events_BackendCrime(crime, playerPedCoors, (uint32_t)crimeId, copsDontCare);

	if(type == EVENT_ASSAULT_POLICE)
		Events_BackendPedWantedNoDrop(1);
	if(type == EVENT_SHOOT_COP || type == EVENT_ASSAULT_NASTYWEAPON_POLICE)
		Events_BackendPedWantedNoDrop(2);

}

CAccident*
AccidentManager_GetNextFreeAccident(CAccidentManager *self)
{
    // Preserve the complete original GetNextFreeAccident policy and all ordered native accesses
	for (int i = 0; i < NUM_ACCIDENTS; i++) {
		if (self->m_aAccidents[i].m_pVictim == NULL)
			return &self->m_aAccidents[i];
	}

	return NULL;
}

void
AccidentManager_ReportAccident(CAccidentManager *self,CPed *ped)
{
    // Preserve the complete original ReportAccident policy and all ordered native accesses
	if (!Events_BackendIsPlayer(ped) && Events_BackendCreatedBy(ped) != EVENTS_MISSION_CHAR && !Events_BackendScorched(ped) && !Events_BackendBodyPart(ped) && Events_BackendAllowRevive(ped) && !Events_BackendInWater(ped)) {
		for (int i = 0; i < NUM_ACCIDENTS; i++) {
			if (self->m_aAccidents[i].m_pVictim != NULL && self->m_aAccidents[i].m_pVictim == ped)
				return;
		}

		if (!Events_BackendHasSurface(ped)) {
			EventsVector point = Events_BackendPedPosition(ped);
			point.z -= 2.0f;


			if (!Events_BackendVerticalLine(point,-100.0f)) {
				CAccident *accident = AccidentManager_GetNextFreeAccident(self);
				if (accident != NULL) {
					accident->m_pVictim = ped;
					Events_BackendRegisterReference(ped,&accident->m_pVictim);
					accident->m_nMedicsPerformingCPR = 0;
					accident->m_nMedicsAttending = 0;
					Events_BackendSetLastAccident(ped,accident);
					AccidentManager_WorkToDoForMedics(self);
				}
			}
		}
	}
}

void
AccidentManager_Update(CAccidentManager *self)
{
    // Preserve the complete original Update policy and all ordered native accesses
#ifdef SQUEEZE_PERFORMANCE
	// Handled after injury registered.
	return;
#endif
	int32_t e;
	if (Events_GetEvent(EVENT_INJURED_PED, &e)) {
		CPed *ped = Events_BackendPed(gaEvent[e].entityRef);
		if (ped) {
			AccidentManager_ReportAccident(self,ped);
			Events_ClearEvent(e);
		}
	}
}

CAccident*
AccidentManager_FindNearestAccident(CAccidentManager *self,EventsVector vecPos, float *pDistance)
{
    // Preserve the complete original FindNearestAccident policy and all ordered native accesses
	for (int i = 0; i < ACCIDENT_MAX_MEDICS_TO_ATTEND; i++){
		int accidentId = -1;
		float minDistance = 999999;
		for (int j = 0; j < NUM_ACCIDENTS; j++){
			CPed* pVictim = self->m_aAccidents[j].m_pVictim;
			if (!pVictim)
				continue;
			if (Events_BackendCreatedBy(pVictim) == EVENTS_MISSION_CHAR)
				continue;
			if (Events_BackendHealth(pVictim) != 0.0f)
				continue;
			if (self->m_aAccidents[j].m_nMedicsPerformingCPR != i)
				continue;
			float distance = Events_Magnitude2D(Events_Sub(Events_BackendPedPosition(pVictim),vecPos));
			if (distance / 2 > Events_BackendPedZ(pVictim) - vecPos.z && distance < minDistance){
				minDistance = distance;
				accidentId = j;
			}
		}
		*pDistance = minDistance;
		if (accidentId != -1)
			return &self->m_aAccidents[accidentId];
	}
	return NULL;
}

uint16_t
AccidentManager_CountActiveAccidents(CAccidentManager *self)
{
    // Preserve the complete original CountActiveAccidents policy and all ordered native accesses
	uint16_t accidents = 0;
	for (int i = 0; i < NUM_ACCIDENTS; i++) {
		if (self->m_aAccidents[i].m_pVictim)
			accidents++;
	}
	return accidents;
}

bool
AccidentManager_WorkToDoForMedics(CAccidentManager *self)
{
    // Preserve the complete original WorkToDoForMedics policy and all ordered native accesses
	for (int i = 0; i < NUM_ACCIDENTS; i++) {
		if (self->m_aAccidents[i].m_pVictim != NULL && self->m_aAccidents[i].m_nMedicsAttending < ACCIDENT_MAX_MEDICS_TO_ATTEND)
			return true;
	}
	return false;
}

bool
AccidentManager_UnattendedAccidents(CAccidentManager *self)
{
    // Preserve the complete original UnattendedAccidents policy and all ordered native accesses
	for (int i = 0; i < NUM_ACCIDENTS; i++) {
		if (self->m_aAccidents[i].m_pVictim != NULL && self->m_aAccidents[i].m_nMedicsAttending == 0)
			return true;
	}
	return false;
}
//- rouz edit (ChatGPT)
