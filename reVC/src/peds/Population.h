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

class CPopulation
{
public:


	static void DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool);
	static CPed *AddPedInCar(CVehicle *car, bool isDriver);
	static void RemovePed(CPed *ent);


	static CPed *AddPed(ePedType pedType, uint32 mi, CVector const &coors, int32 modifier = 0);
	static void AddToPopulation(float, float, float, float);
	static void ManagePopulation(void);
	static void MoveCarsAndPedsOutOfAbandonedZones(void);
	static void ConvertToRealObject(CDummyObject*);
	static void ConvertToDummyObject(CObject*);
	static void ConvertAllObjectsToDummyObjects(void);
	static bool TestRoomForDummyObject(CObject*);
	static bool TestSafeForRealObject(CDummyObject*);
	static bool IsSkateable(CVector const&);

	static void RemovePedsIfThePoolGetsFull(void);




	static void PlaceGangMembers(ePedType, int32, CVector const&);
	static void PlaceGangMembersInFormation(ePedType, int32, CVector const&);
	static void PlaceGangMembersInCircle(ePedType, int32, CVector const&);
	static void PlaceCouple(ePedType, int32, ePedType, int32, CVector);
	static void PlaceMallPedsAsStationaryGroup(CVector const&, int32);
	static CPed* AddDeadPedInFrontOfCar(const CVector& pos, CVehicle* pCulprit);
};
