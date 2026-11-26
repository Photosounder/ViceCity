#pragma once
#include "PopulationLimits.h" // rouz edit (ChatGPT)
#include "PopulationGroups.h" // rouz edit (ChatGPT)
#include "PopulationCounts.h" // rouz edit (ChatGPT)

#include "Game.h"
#include "PedType.h"

class CPed;
class CVehicle;
class CDummyObject;
class CObject;

//+ rouz edit (Claude)
// Formerly the static class CPopulation, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
void Population_DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool);
CPed *Population_AddPedInCar(CVehicle *car, bool isDriver);
void Population_RemovePed(CPed *ent);
void Population_AddToPopulation(float, float, float, float);
void Population_ManagePopulation(void);
void Population_MoveCarsAndPedsOutOfAbandonedZones(void);
void Population_ConvertToRealObject(CDummyObject*);
void Population_ConvertToDummyObject(CObject*);
void Population_ConvertAllObjectsToDummyObjects(void);
bool Population_TestRoomForDummyObject(CObject*);
bool Population_TestSafeForRealObject(CDummyObject*);
void Population_RemovePedsIfThePoolGetsFull(void);
void Population_PlaceCouple(ePedType, int32, ePedType, int32, CVector);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
CPed *Population_AddPed(ePedType pedType, uint32 mi, CVector const &coors, int32 modifier = 0);
bool Population_IsSkateable(CVector const&);
void Population_PlaceGangMembers(ePedType, int32, CVector const&);
void Population_PlaceGangMembersInFormation(ePedType, int32, CVector const&);
void Population_PlaceGangMembersInCircle(ePedType, int32, CVector const&);
void Population_PlaceMallPedsAsStationaryGroup(CVector const&, int32);
CPed* Population_AddDeadPedInFrontOfCar(const CVector& pos, CVehicle* pCulprit);
#endif
//- rouz edit (Claude)
