//+ rouz edit (ChatGPT)
#include "ModelRegistry.h"
#include "Fire_backend.h"
#include "Timer.h"
#include "General.h"
#include "PedType.h"
#include "PedStates.h"
#include "References.h"
#include "PointLights.h"
#include <stddef.h>
#include <math.h>
_Static_assert(NUM_FIRES==40,"Original fire count for constructor-equivalent static initialization");
CFireManager gFireManager={0,{
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false},
    {false,false,true,true,{0.0f,0.0f,0.0f},NULL,NULL,0,0,0,0.8f,1.0f,false}
}};
static FireVector Fire_Make(float x,float y,float z)
{
    // Materialize the original three-component vector value
    FireVector v={x,y,z};return v;
}
static FireVector Fire_Add(FireVector a,FireVector b)
{
    // Preserve the original component-wise additions
    return Fire_Make(a.x+b.x,a.y+b.y,a.z+b.z);
}
static FireVector Fire_Sub(FireVector a,FireVector b)
{
    // Preserve the original component-wise subtractions
    return Fire_Make(a.x-b.x,a.y-b.y,a.z-b.z);
}
static float Fire_MagnitudeSqr(FireVector v)
{
    // Keep the original three-term squared distance evaluation
    return v.x*v.x+v.y*v.y+v.z*v.z;
}
static float Fire_Magnitude2D(FireVector v)
{
    // Keep the original two-term squared distance and float square root
    return sqrtf(v.x*v.x+v.y*v.y);
}
static float Fire_Square(float value)
{
    // Evaluate the original range argument once before multiplying it by itself
    return value*value;
}
static float Fire_Min(float a,float b)
{
    // Preserve the original strict comparison including ties and unordered values
    return a<b?a:b;
}
static PointLightsVector Fire_LightVector(FireVector v)
{
    // Transfer the three original coordinates into the existing C point-light API
    PointLightsVector result={v.x,v.y,v.z};return result;
}
void
Fire_ProcessFire(CFire *fire)
{
    // Preserve the complete original record ProcessFire operation and ordered live accesses
	float fDamagePlayer;
	float fDamagePeds;
	float fDamageVehicle;
	int16_t nRandNumber;
	float fGreen;
	float fRed;
	FireVector lightpos;
	FireVector firePos;
	CEntity *ped = fire->m_pEntity;
	CEntity *veh = fire->m_pEntity;

	fire->m_fWaterExtinguishCountdown = Fire_Min(1.0f, 0.002f * (*Timer_GetTimeStep()) + fire->m_fWaterExtinguishCountdown);

	if (fire->m_pEntity) {
		fire->m_vecPos = Fire_BackendPosition(fire->m_pEntity);

		if (Fire_BackendIsPed(fire->m_pEntity)) {
			if (Fire_BackendPedFire(ped) != fire) {
				Fire_Extinguish(fire);
				return;
			}
#if defined GTAVC_JP_PATCH && !defined FIX_BUGS
			if (fire->m_pEntity == Fire_BackendShortcutTaxi() && Fire_BackendShortcutTransition()) {
				Fire_Extinguish(fire);
				return;
			}
#endif
			if (Fire_BackendMoveState(ped) != PEDMOVE_RUN)
				fire->m_vecPos.z -= 1.0f;
			if (Fire_BackendInVehicle(ped) && Fire_BackendMyVehicle(ped)) {
				if (Fire_BackendIsCar(Fire_BackendMyVehicle(ped)))
					Fire_BackendVehicleHealth(Fire_BackendMyVehicle(ped),75.0f);
			} else if (fire->m_pEntity == Fire_BackendPlayerPed()) {
				fDamagePlayer = 1.2f * (*Timer_GetTimeStep());

				Fire_BackendPedDamage(fire->m_pEntity,fire->m_pSource,fDamagePlayer);
			} else {
				fDamagePeds = 1.2f * (*Timer_GetTimeStep());

				if (Fire_BackendPedDamage(fire->m_pEntity,fire->m_pSource,fDamagePeds)) {
					Fire_BackendScorched(fire->m_pEntity);
				}
			}
		} else if (Fire_BackendIsVehicle(fire->m_pEntity)) {
			if (Fire_BackendVehicleFire(veh) != fire) {
				Fire_Extinguish(fire);
				return;
			}
#ifdef FIX_BUGS
			if (fire->m_pEntity == Fire_BackendShortcutTaxi() && Fire_BackendShortcutTransition()) {
				Fire_Extinguish(fire);
				return;
			}
#endif
			if (!fire->m_bIsScriptFire) {
				fDamageVehicle = 1.2f * (*Timer_GetTimeStep());
				Fire_BackendVehicleDamage(veh,fire->m_pSource,fDamageVehicle);
			}
		}
	}
	if (!Fire_BackendPlayerVehicle() &&
#ifdef FIX_BUGS
		Fire_BackendPlayerPed() &&
#endif
		!Fire_BackendPedFire(Fire_BackendPlayerPed()) && !(Fire_BackendFireProof(Fire_BackendPlayerPed()))
		&& (Fire_MagnitudeSqr(Fire_Sub(Fire_BackendPosition(Fire_BackendPlayerPed()),fire->m_vecPos)) < 2.0f)) {
		Fire_BackendPrepareOnFire(Fire_BackendPlayerPed());
		FireManager_StartFireOn(&gFireManager,Fire_BackendPlayerPed(),fire->m_pSource,0.8f,1);
	}
	if ((*Timer_GetTimeInMilliseconds()) > fire->m_nNextTimeToAddFlames) {
		fire->m_nNextTimeToAddFlames = (*Timer_GetTimeInMilliseconds()) + (fire->m_fWaterExtinguishCountdown < 0.3f ? 400 : (fire->m_fWaterExtinguishCountdown < 0.7f ? 200 : 80));
		firePos = fire->m_vecPos;

		if (veh && Fire_BackendIsVehicle(veh) && Fire_BackendIsCar(veh)) {
			FireVector ModelInfo = Fire_BackendHeadlights(Fire_BackendModel(veh));
			ModelInfo = Fire_BackendTransform(fire->m_pEntity,ModelInfo);

			firePos.x = ModelInfo.x;
			firePos.y = ModelInfo.y;
			firePos.z = ModelInfo.z + 0.15f;
		}

		Fire_BackendFlame(firePos,Fire_Make(0.0f, 0.0f, General_GetRandomNumberInRangeFloat(0.0125f, 0.1f) * fire->m_fStrength),fire->m_fStrength);

		General_GetRandomNumber(); General_GetRandomNumber(); General_GetRandomNumber(); /* unsure why these three rands are called */

		Fire_BackendSmoke(firePos);
	}
	if ((*Timer_GetTimeInMilliseconds()) < fire->m_nExtinguishTime || fire->m_bIsScriptFire) {
		if ((*Timer_GetTimeInMilliseconds()) > fire->m_nStartTime)
			fire->m_nStartTime = (*Timer_GetTimeInMilliseconds()) + 400;

		nRandNumber = General_GetRandomNumber() & 127;
		lightpos.x = fire->m_vecPos.x;
		lightpos.y = fire->m_vecPos.y;
		lightpos.z = fire->m_vecPos.z + 5.0f;

		if (!fire->m_pEntity) {
			Fire_BackendShadow((uintptr_t)fire,lightpos,nRandNumber);
		}
		fGreen = nRandNumber / 128.f;
		fRed = nRandNumber / 128.f;

		// Enter the C point-light algorithm with the original owner expressions
		PointLights_AddLight(POINTLIGHTS_LIGHT_POINT,Fire_LightVector(fire->m_vecPos),(PointLightsVector){0.0f,0.0f,0.0f},12.0f,fRed,fGreen,0.0f,0,0);
	} else {
		Fire_Extinguish(fire);
	}
}

void
Fire_ReportThisFire(CFire *fire)
{
    // Preserve the complete original record ReportThisFire operation and ordered live accesses
	gFireManager.m_nTotalFires++;
	Fire_BackendReport(fire->m_vecPos);
}

void
Fire_Extinguish(CFire *fire)
{
    // Preserve the complete original record Extinguish operation and ordered live accesses
	if (fire->m_bIsOngoing) {
		if (!fire->m_bIsScriptFire)
			gFireManager.m_nTotalFires--;

		fire->m_nExtinguishTime = 0;
		fire->m_bIsOngoing = false;
		fire->m_bExtinguishedWithWater = false;

		if (fire->m_pEntity) {
			if (Fire_BackendIsPed(fire->m_pEntity)) {
				CEntity *ped = fire->m_pEntity;
				if (Fire_BackendCanSetState(ped)) {
					if (Fire_BackendPedState(ped) != PED_DRIVING && Fire_BackendPedState(ped) != PED_FALL) {
						if (Fire_BackendIsPlayer(ped)) {
							Fire_BackendIdle(ped);
						} else {
							Fire_BackendLastStateNone(ped);
							Fire_BackendWander(ped);
							Fire_BackendWaitFinish(ped);
						}
					}
				}
				Fire_BackendSetPedFire(ped,NULL);
			} else if (Fire_BackendIsVehicle(fire->m_pEntity)) {
				Fire_BackendSetVehicleFire(fire->m_pEntity,NULL);
			}
			fire->m_pEntity = NULL;
		}
	}
}

void
FireManager_StartFireAt(CFireManager *manager,FireVector pos, float size, uint8_t propagation)
{
    // Preserve the complete original manager StartFireAt operation and ordered live accesses
	CFire *fire = FireManager_GetNextFreeFire(manager);

	if (fire) {
		fire->m_bIsOngoing = true;
		fire->m_bIsScriptFire = false;
		fire->m_bPropagationFlag = propagation;
		fire->m_bAudioSet = true;
		fire->m_vecPos = pos;
		fire->m_nExtinguishTime = (*Timer_GetTimeInMilliseconds()) + 10000;
		fire->m_nStartTime = (*Timer_GetTimeInMilliseconds()) + 400;
		fire->m_pEntity = NULL;
		fire->m_pSource = NULL;
		fire->m_nNextTimeToAddFlames = 0;
		Fire_ReportThisFire(fire);
		fire->m_fStrength = size;
		fire->m_bExtinguishedWithWater = false;
	}
}

CFire *
FireManager_StartFireOn(CFireManager *manager,CEntity *entityOnFire, CEntity *fleeFrom, float strength, uint8_t propagation)
{
    // Preserve the complete original manager StartFireOn operation and ordered live accesses
	CEntity *ped = entityOnFire;
	CEntity *veh = entityOnFire;

	if (Fire_BackendIsPed(entityOnFire)) {
		if (Fire_BackendPedFire(ped))
			return NULL;
		if (!Fire_BackendInControl(ped))
			return NULL;
	} else if (Fire_BackendIsVehicle(entityOnFire)) {
		if (Fire_BackendVehicleFire(veh))
			return NULL;
		if (Fire_BackendIsCar(veh) &&  Fire_BackendEngineStatus(veh) >= 225)
			return NULL;
	}
	CFire *fire = FireManager_GetNextFreeFire(manager);

	if (fire) {
		if (Fire_BackendIsPed(entityOnFire)) {
			Fire_BackendSetPedFire(ped,fire);
			if (ped != Fire_BackendPlayerPed()) {
				if (fleeFrom) {
					Fire_BackendFleeEntity(ped,fleeFrom);
				} else {
					FireVector fleePosition = Fire_BackendPosition(entityOnFire);
					Fire_BackendFleePosition(ped,fleePosition);
					Fire_BackendClearFleeFrom(ped);
				}
				(*Fire_BackendFleeTimer(ped)) = (*Timer_GetTimeInMilliseconds()) + 10000;
				Fire_BackendDrawLastOff(ped);
				Fire_BackendSprint(ped);
				Fire_BackendMoveAnim(ped);
				Fire_BackendOnFireState(ped);
			}
			if (fleeFrom) {
				if (Fire_BackendPedType(ped) == PEDTYPE_COP) {
					Fire_BackendCopEvent(entityOnFire,fleeFrom);
				} else {
					Fire_BackendPedEvent(entityOnFire,fleeFrom);
				}
			}
		} else {
			if (Fire_BackendIsVehicle(entityOnFire)) {
				Fire_BackendSetVehicleFire(veh,fire);
				if (ModelRegistry_IsBikeModel(Fire_BackendModel(veh)) || ModelRegistry_IsCarModel(Fire_BackendModel(veh)))
        				Fire_BackendFleeOccupants(veh);

				if (fleeFrom) {
					Fire_BackendVehicleEvent(entityOnFire,fleeFrom);
				}
			}
		}

		fire->m_bIsOngoing = true;
		fire->m_bExtinguishedWithWater = false;
		fire->m_bIsScriptFire = false;
		fire->m_vecPos = Fire_BackendPosition(entityOnFire);

		if (entityOnFire && Fire_BackendIsPed(entityOnFire) && Fire_BackendIsPlayer(ped)) {
			fire->m_nExtinguishTime = (*Timer_GetTimeInMilliseconds()) + 3333;
		} else if (Fire_BackendIsVehicle(entityOnFire)) {
			fire->m_nExtinguishTime = (*Timer_GetTimeInMilliseconds()) + General_GetRandomNumberInRangeInt(4000, 5000);
		} else {
			fire->m_nExtinguishTime = (*Timer_GetTimeInMilliseconds()) + General_GetRandomNumberInRangeInt(10000, 11000);
		}
		fire->m_nStartTime = (*Timer_GetTimeInMilliseconds()) + 400;
		fire->m_pEntity = entityOnFire;

		References_RegisterReference(entityOnFire,&fire->m_pEntity);
		fire->m_pSource = fleeFrom;

		if (fleeFrom)
			References_RegisterReference(fleeFrom,&fire->m_pSource);
		Fire_ReportThisFire(fire);
		fire->m_nNextTimeToAddFlames = 0;
		fire->m_fStrength = strength;
		fire->m_bPropagationFlag = propagation;
		fire->m_bAudioSet = true;
	}
	return fire;
}

void
FireManager_Update(CFireManager *manager)
{
    // Preserve the complete original manager Update operation and ordered live accesses
	for (int i = 0; i < NUM_FIRES; i++) {
		if (manager->m_aFires[i].m_bIsOngoing)
			Fire_ProcessFire(&manager->m_aFires[i]);
	}
}

CFire* FireManager_FindNearestFire(CFireManager *manager,FireVector vecPos, float *pDistance)
{
    // Preserve the complete original manager FindNearestFire operation and ordered live accesses
	int fireId = -1;
	float minDistance = 999999;
	for (int j = 0; j < NUM_FIRES; j++) {
		if (!manager->m_aFires[j].m_bIsOngoing)
			continue;
		if (manager->m_aFires[j].m_bIsScriptFire)
			continue;
		float distance = Fire_Magnitude2D(Fire_Sub(manager->m_aFires[j].m_vecPos,vecPos));
		if (distance < minDistance) {
			minDistance = distance;
			fireId = j;
		}
	}
	*pDistance = minDistance;
	if (fireId != -1)
		return &manager->m_aFires[fireId];

	return NULL;
}

CFire *
FireManager_FindFurthestFire_NeverMindFireMen(CFireManager *manager,FireVector coords, float minRange, float maxRange)
{
    // Preserve the complete original manager FindFurthestFire_NeverMindFireMen operation and ordered live accesses
	int furthestFire = -1;
	float lastFireDist = 0.0f;
	float fireDist;

	for (int i = 0; i < NUM_FIRES; i++) {
		if (manager->m_aFires[i].m_bIsOngoing && !manager->m_aFires[i].m_bIsScriptFire) {
			fireDist = Fire_Magnitude2D(Fire_Sub(manager->m_aFires[i].m_vecPos,coords));
			if (fireDist > minRange && fireDist < maxRange && fireDist > lastFireDist) {
				lastFireDist = fireDist;
				furthestFire = i;
			}
		}
	}
	if (furthestFire == -1)
		return NULL;
	else
		return &manager->m_aFires[furthestFire];
}

CFire *
FireManager_GetNextFreeFire(CFireManager *manager)
{
    // Preserve the complete original manager GetNextFreeFire operation and ordered live accesses
	for (int i = 0; i < NUM_FIRES; i++) {
		if (!manager->m_aFires[i].m_bIsOngoing && !manager->m_aFires[i].m_bIsScriptFire)
			return &manager->m_aFires[i];
	}
	return NULL;
}

uint32_t
FireManager_GetTotalActiveFires(const CFireManager *manager)
{
    // Preserve the complete original manager GetTotalActiveFires operation and ordered live accesses
	return manager->m_nTotalFires;
}

void
FireManager_ExtinguishPoint(CFireManager *manager,FireVector point, float range)
{
    // Preserve the complete original manager ExtinguishPoint operation and ordered live accesses
	for (int i = 0; i < NUM_FIRES; i++) {
		if (manager->m_aFires[i].m_bIsOngoing) {
			if (Fire_MagnitudeSqr(Fire_Sub(point,manager->m_aFires[i].m_vecPos)) < Fire_Square(range))
				Fire_Extinguish(&manager->m_aFires[i]);
		}
	}
}

bool
FireManager_ExtinguishPointWithWater(CFireManager *manager,FireVector point, float range)
{
    // Preserve the complete original manager ExtinguishPointWithWater operation and ordered live accesses
    int i;
    for (i = 0; i < NUM_FIRES;) {
        if (manager->m_aFires[i].m_bIsOngoing && Fire_MagnitudeSqr(Fire_Sub(point,manager->m_aFires[i].m_vecPos)) < Fire_Square(range)) {
                break;
        }
        if (++i >= NUM_FIRES)
          return false;
    }
    
	CFire *fireToExtinguish = &manager->m_aFires[i];
	fireToExtinguish->m_fWaterExtinguishCountdown -= 0.012f * (*Timer_GetTimeStep());
	FireVector steamPos = Fire_Add(fireToExtinguish->m_vecPos,Fire_Make((General_GetRandomNumber() - 128) * 3.1f / 200.f,
			(General_GetRandomNumber() - 128) * 3.1f / 200.f,
			General_GetRandomNumber() / 200.f));

	Fire_BackendSteam(steamPos,0.2f,0.5f);
	Fire_BackendSteam(steamPos,0.1f,0.8f);
	fireToExtinguish->m_bExtinguishedWithWater = true;
	if (fireToExtinguish->m_fWaterExtinguishCountdown < 0.0f )
	  Fire_Extinguish(fireToExtinguish);

	return true;
}

int32_t
FireManager_StartScriptFire(CFireManager *manager,const FirePosition *position, CEntity *target, float strength, uint8_t propagation)
{
    // Preserve the complete original manager StartScriptFire operation and ordered live accesses
	CFire *fire;
	CEntity *ped = target;
	CEntity *veh = target;

	if (target) {
		if (Fire_BackendIsPed(target)) {
			if (Fire_BackendPedFire(ped))
				Fire_Extinguish(Fire_BackendPedFire(ped));
		} else if (Fire_BackendIsVehicle(target)) {
			if (Fire_BackendVehicleFire(veh))
				Fire_Extinguish(Fire_BackendVehicleFire(veh));
			if (Fire_BackendIsCar(veh) && Fire_BackendEngineStatus(veh) >= 225) {
				Fire_BackendSetEngineStatus(veh,215);
			}
		}
	}

	fire = FireManager_GetNextFreeFire(manager);
	fire->m_bIsOngoing = true;
	fire->m_bIsScriptFire = true;
	fire->m_bPropagationFlag = propagation;
	fire->m_bAudioSet = true;
	fire->m_vecPos = Fire_BackendReadVector(position);
	fire->m_nStartTime = (*Timer_GetTimeInMilliseconds()) + 400;
	fire->m_pEntity = target;
	fire->m_bExtinguishedWithWater = false;

	if (target)
		References_RegisterReference(target,&fire->m_pEntity);
	fire->m_pSource = NULL;
	fire->m_nNextTimeToAddFlames = 0;
	fire->m_fStrength = strength;
	fire->m_fWaterExtinguishCountdown = 1.0f;

	if (target) {
		if (Fire_BackendIsPed(target)) {
			Fire_BackendSetPedFire(ped,fire);
			if (target != Fire_BackendPlayerPed()) {
				FireVector fleePosition = Fire_BackendPosition(target);
				Fire_BackendFleePosition(ped,fleePosition);
				Fire_BackendMoveAnim(ped);
				Fire_BackendOnFireState(ped);
			}
		} else if (Fire_BackendIsVehicle(target)) {
			Fire_BackendSetVehicleFire(veh,fire);
		}
	}
	return fire - manager->m_aFires;
}

bool
FireManager_IsScriptFireExtinguish(CFireManager *manager,int16_t index)
{
    // Preserve the complete original manager IsScriptFireExtinguish operation and ordered live accesses
	return !manager->m_aFires[index].m_bIsOngoing;
}

void
FireManager_RemoveAllScriptFires(CFireManager *manager)
{
    // Preserve the complete original manager RemoveAllScriptFires operation and ordered live accesses
	for (int i = 0; i < NUM_FIRES; i++) {
		if (manager->m_aFires[i].m_bIsScriptFire) {
			FireManager_RemoveScriptFire(manager,i);
		}
	}
}

void
FireManager_RemoveScriptFire(CFireManager *manager,int16_t index)
{
    // Preserve the complete original manager RemoveScriptFire operation and ordered live accesses
	Fire_Extinguish(&manager->m_aFires[index]);
	manager->m_aFires[index].m_bIsScriptFire = false;
}

void
FireManager_SetScriptFireAudio(CFireManager *manager,int16_t index, bool state)
{
    // Preserve the complete original manager SetScriptFireAudio operation and ordered live accesses
	manager->m_aFires[index].m_bAudioSet = state;
}
//- rouz edit (ChatGPT)
