//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)

#include "Game.h"
#include "General.h"
#include "World.h"
#include "Population.h"
#include "CopPed.h"
#include "Wanted.h"
#include "FileMgr.h"
#include "Gangs.h"
#include "ModelIndices.h"
#include "Zones.h"
#include "CivilianPed.h"
#include "EmergencyPed.h"
#include "Replay.h"
#include "Camera.h"
#include "CutsceneMgr.h"
#include "CarCtrl.h"
#include "IniFile.h"
#include "VisibilityPlugins.h"
#include "PedPlacement.h"
#include "DummyObject.h"
#include "Script.h"
#include "Shadows.h"
#include "SurfaceTable.h"
#include "Weather.h"
#include "Darkel.h"
#include "Streaming.h"
#include "Clock.h"
#include "WaterLevel.h"
#include "Pools.h" // rouz edit (ChatGPT)

#define MIN_CREATION_DIST		40.0f // not for start of the game (look at the GeneratePedsAtStartOfGame)
#define CREATION_RANGE			10.0f // added over the MIN_CREATION_DIST.
#define OFFSCREEN_CREATION_MULT	0.5f
#define PED_REMOVE_DIST			(MIN_CREATION_DIST + CREATION_RANGE + 1.0f)
#define PED_REMOVE_DIST_SPECIAL	(MIN_CREATION_DIST + CREATION_RANGE + 15.0f) // for peds with bCullExtraFarAway flag



CEntity *gSunbatheObstacles[2];
CEntity *gCoupleObstacles[3];

//+ rouz edit (ChatGPT)
void PopulationCounts_InitialiseLog(bool starting)
{
    // Preserve the original platform-specific initialization messages
    if (starting)
        debug("Initialising CPopulation...\n");
    else
        debug("CPopulation ready\n");
}
//- rouz edit (ChatGPT)

void
CPopulation::RemovePed(CPed *ent)
{
	CWorld::Remove((CEntity*)ent);
//+ rouz edit (ChatGPT)
	// Destroy and release the ped without invoking C++ delete.
	ent->~CPed();
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPool_Delete(CPools::GetPedPool(), ent);
	//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
}

// returns eCopType


//+ rouz edit (ChatGPT)
void PopulationCounts_UnknownPedType(void)
{
    // Retain the original platform-specific error reporting
    Error("Unknown ped type, UpdatePedCount, Population.cpp");
}
bool PopulationGroups_IsCivilianMale(int32_t model)
{
    // Read the original native pedestrian type after male classification
    return ((CPedModelInfo *)((CBaseModelInfo *)ModelRegistry_Get(model)))->m_pedType == PEDTYPE_CIVMALE;
}
bool PopulationGroups_FemaleMatches(int32_t man, int32_t woman)
{
    // Preserve the original model lookup and field access order
    CPedModelInfo *womanInfo = (CPedModelInfo *)((CBaseModelInfo *)ModelRegistry_Get(woman));
    if (womanInfo->m_pedType == PEDTYPE_CIVFEMALE) {
        // Compare animation and skater status only after both lookups
        CPedModelInfo *manInfo = (CPedModelInfo *)((CBaseModelInfo *)ModelRegistry_Get(man));
        if (manInfo && womanInfo && manInfo->m_animGroup == womanInfo->m_animGroup) {
            // Keep the two original skater comparisons
            if (manInfo->m_pedStatType != PEDSTAT_SKATER && womanInfo->m_pedStatType != PEDSTAT_SKATER)
                return true;
            if (manInfo->m_pedStatType == PEDSTAT_SKATER && womanInfo->m_pedStatType == PEDSTAT_SKATER)
                return true;
        }
    }
    return false;
}
//- rouz edit (ChatGPT)



void
CPopulation::DealWithZoneChange(eLevelName oldLevel, eLevelName newLevel, bool forceIndustrialZone)
{
}

//+ rouz edit (ChatGPT)
void PopulationHost_Manage(void)
{
    // Run the original population maintenance sequence
    CPopulation::ManagePopulation();
    CPopulation::RemovePedsIfThePoolGetsFull();
    CPopulation::MoveCarsAndPedsOutOfAbandonedZones();
}
bool PopulationHost_PlayerVehicleSpeed(float *speed)
{
    // Read the original player vehicle and its two-dimensional speed
    CVehicle *vehicle = FindPlayerVehicle();
    if (!vehicle)
        return false;
    *speed = vehicle->m_vecMoveSpeed.Magnitude2D();
    return true;
}
float PopulationHost_CameraGenerationMultiplier(void)
{
    // Read the live camera generation distance multiplier
    return TheCamera.GenerationDistMultiplier;
}
void PopulationHost_AddToPopulation(float minDist, float maxDist, float minOffscreen, float maxOffscreen)
{
    // Forward the original four spawn distances
    CPopulation::AddToPopulation(minDist, maxDist, minOffscreen, maxOffscreen);
}
//- rouz edit (ChatGPT)

CPed*
CPopulation::AddPed(ePedType pedType, uint32 miOrCopType, CVector const &coors, int32 modifier)
{
	switch (pedType) {
		case PEDTYPE_CIVMALE:
		case PEDTYPE_CIVFEMALE:
		{
//+ rouz edit (ChatGPT)
			// Allocate the civilian ped without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CCivilianPed *ped = (CCivilianPed*)((CPed*)CPool_New(CPools::GetPedPool()));
			//- rouz edit (ChatGPT)
			assert(ped);
			std::allocator<CCivilianPed>().construct(ped, pedType, miOrCopType);
//- rouz edit (ChatGPT)
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			if (Population_givePedsWeapons) { // rouz edit (ChatGPT)
				eWeaponType weapon;

				switch (General_GetRandomNumber() & 3) {
					case 0:
						weapon = WEAPONTYPE_COLT45;
						break;
					case 1:
						weapon = WEAPONTYPE_NIGHTSTICK;
						break;
					case 2:
						weapon = WEAPONTYPE_GOLFCLUB;
						break;
					case 3:
						weapon = WEAPONTYPE_TEC9;
						break;
					default:
						break;
				}
				if (weapon != WEAPONTYPE_UNARMED) {
					ped->GiveDelayedWeapon(weapon, 25001);
					ped->SetCurrentWeapon(WeaponInfo_GetWeaponInfo(weapon)->m_nWeaponSlot);
				}
			}
			return ped;
		}
		case PEDTYPE_COP:
		{
//+ rouz edit (ChatGPT)
			// Allocate the cop ped without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CCopPed *ped = (CCopPed*)((CPed*)CPool_New(CPools::GetPedPool()));
			//- rouz edit (ChatGPT)
			assert(ped);
			std::allocator<CCopPed>().construct(ped, (eCopType)miOrCopType, modifier);
//- rouz edit (ChatGPT)
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		case PEDTYPE_GANG1:
		case PEDTYPE_GANG2:
		case PEDTYPE_GANG3:
		case PEDTYPE_GANG4:
		case PEDTYPE_GANG5:
		case PEDTYPE_GANG6:
		case PEDTYPE_GANG7:
		case PEDTYPE_GANG8:
		case PEDTYPE_GANG9:
		{
//+ rouz edit (ChatGPT)
			// Allocate the gang ped without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CCivilianPed *ped = (CCivilianPed*)((CPed*)CPool_New(CPools::GetPedPool()));
			//- rouz edit (ChatGPT)
			assert(ped);
			std::allocator<CCivilianPed>().construct(ped, pedType, miOrCopType);
//- rouz edit (ChatGPT)
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);

			eWeaponType weapon;
			if (General_GetRandomNumberInRangeInt(0, 100) >= 50)
				weapon = (eWeaponType)Gangs_GetGangInfo(pedType - PEDTYPE_GANG1)->m_Weapon2;
			else
				weapon = (eWeaponType)Gangs_GetGangInfo(pedType - PEDTYPE_GANG1)->m_Weapon1;

			ped->GiveDelayedWeapon(weapon, 25001);
			ped->SetCurrentWeapon(WeaponInfo_GetWeaponInfo(weapon)->m_nWeaponSlot);
			return ped;
		}
		case PEDTYPE_EMERGENCY:
		{
//+ rouz edit (ChatGPT)
			// Allocate the emergency ped without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CEmergencyPed *ped = (CEmergencyPed*)((CPed*)CPool_New(CPools::GetPedPool()));
			//- rouz edit (ChatGPT)
			assert(ped);
			std::allocator<CEmergencyPed>().construct(ped, PEDTYPE_EMERGENCY);
//- rouz edit (ChatGPT)
		    ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		case PEDTYPE_FIREMAN:
		{
//+ rouz edit (ChatGPT)
			// Allocate the fireman ped without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CEmergencyPed *ped = (CEmergencyPed*)((CPed*)CPool_New(CPools::GetPedPool()));
			//- rouz edit (ChatGPT)
			assert(ped);
			std::allocator<CEmergencyPed>().construct(ped, PEDTYPE_FIREMAN);
//- rouz edit (ChatGPT)
		    ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		case PEDTYPE_CRIMINAL:
		case PEDTYPE_PROSTITUTE:
		{
//+ rouz edit (ChatGPT)
			// Allocate the civilian ped without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CCivilianPed *ped = (CCivilianPed*)((CPed*)CPool_New(CPools::GetPedPool()));
			//- rouz edit (ChatGPT)
			assert(ped);
			std::allocator<CCivilianPed>().construct(ped, pedType, miOrCopType);
//- rouz edit (ChatGPT)
			ped->SetPosition(coors);
			ped->SetOrientation(0.0f, 0.0f, 0.0f);
			CWorld::Add(ped);
			return ped;
		}
		default:
			Error("Unknown ped type, AddPed, Population.cpp");
			return nil;
	}
}

void
CPopulation::AddToPopulation(float minDist, float maxDist, float minDistOffScreen, float maxDistOffScreen)
{
	uint32 pedTypeToAdd;
	int32 modelToAdd;
	int pedAmount;

	CZoneInfo zoneInfo;
	int32 man = -1, woman = -1;
	CPed *gangLeader = nil;
	bool addCop = false;
	bool isSecurityGuard = false;
	bool forceAddingCop = false;
	CPlayerInfo *playerInfo = &CWorld::Players[CWorld::PlayerInFocus];
	CVector playerCentreOfWorld = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);
	Zones_GetZoneInfoForTimeOfDay(ZonesPosition_FromVector(*(&playerCentreOfWorld)), &zoneInfo);
	CWanted *wantedInfo = playerInfo->m_pPed->m_pWanted;

	if (Wanted_GetWantedLevel(wantedInfo) > 2) {
		if (!CGame::IsInInterior() && (General_GetRandomNumber() % 32 == 0) && FindPlayerVehicle())
			forceAddingCop = true;

		uint32 maxCops = CGame::IsInInterior() ? wantedInfo->m_MaxCops * 1.6f : wantedInfo->m_MaxCops;
		if ((Population_nNumCop < maxCops || forceAddingCop) && // rouz edit (ChatGPT)
			(!playerInfo->m_pPed->bInVehicle &&
				(CCarCtrl::NumLawEnforcerCars >= wantedInfo->m_MaximumLawEnforcerVehicles
				|| CCarCtrl::NumRandomCars >= playerInfo->m_nTrafficMultiplier * CCarCtrl::CarDensityMultiplier
				|| CCarCtrl::NumFiretrucksOnDuty + CCarCtrl::NumAmbulancesOnDuty + CCarCtrl::NumParkedCars
				+ CCarCtrl::NumMissionCars + CCarCtrl::NumLawEnforcerCars + CCarCtrl::NumRandomCars >= PopulationLimits_MaxCars) || forceAddingCop)) { // rouz edit (ChatGPT)
			addCop = true;
			minDist = Population_PedCreationDistMultiplier() * MIN_CREATION_DIST; // rouz edit (ChatGPT)
			maxDist = Population_PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE); // rouz edit (ChatGPT)
		}
	}
	float missionAndWeatherMult = -0.8f * Sqrt(Weather_Rain) + 1.0f;

	// Taxi side mission
	if (CTheScripts::IsPlayerOnAMission()) {
		CPed *player = FindPlayerPed();
		if (player && player->InVehicle() && player->m_pMyVehicle->IsTaxi())
			missionAndWeatherMult = 1.0f;
	}
	if (Darkel_FrenzyOnGoing())
		missionAndWeatherMult = 1.0f;
	int selectedMaxPeds = CGame::IsInInterior() ? PopulationLimits_MaxInteriorPeds : PopulationLimits_MaxPeds; // rouz edit (ChatGPT)

	// Yeah, float
	float maxPossiblePedsForArea = (zoneInfo.pedDensity + zoneInfo.carDensity) * playerInfo->m_fRoadDensity * Population_pedDensityMultiplier // rouz edit (ChatGPT)
		* (Darkel_FrenzyOnGoing() ? 1.f : IniFile_PedNumberMultiplier) * missionAndWeatherMult;
	maxPossiblePedsForArea = Min(maxPossiblePedsForArea, selectedMaxPeds);

	if (Population_nTotalPeds - rouz.driver_ped_count < rouz.ped_mul * maxPossiblePedsForArea || addCop) { // rouz edit (ChatGPT)
		int decisionThreshold = General_GetRandomNumberInRangeInt(0, 1000);
		if (decisionThreshold < zoneInfo.copPedThreshold || addCop) {
			pedTypeToAdd = PEDTYPE_COP;
			modelToAdd = Population_ChoosePolicePedOccupation(); // rouz edit (ChatGPT)
		} else {
			int i = 0;
			for (i = 0; i < NUM_GANGS; i++) {
				if (decisionThreshold < zoneInfo.gangPedThreshold[i]) {
					break;
				}
			}

			if (i == NUM_GANGS) {
				if (General_GetRandomNumberInRangeFloat(0.0f, 1.0f) <= 0.95f) {
					modelToAdd = Population_ChooseCivilianOccupation(zoneInfo.pedGroup); // rouz edit (ChatGPT)

					if (modelToAdd == -1)
						return;
					pedTypeToAdd = ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(modelToAdd)))->m_pedType; // rouz edit (ChatGPT)

				} else {
					Population_ChooseCivilianCoupleOccupations(zoneInfo.pedGroup, &man, &woman); // rouz edit (ChatGPT)
					if (man == -1 || woman == -1)
						return;
					pedTypeToAdd = ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(woman)))->m_pedType; // rouz edit (ChatGPT)
				}
			} else {
				pedTypeToAdd = PEDTYPE_GANG1 + i;

				if (Population_IsSecurityGuard((ePedType)pedTypeToAdd)) { // rouz edit (ChatGPT)
					isSecurityGuard = true;
					modelToAdd = Population_ChooseGangOccupation(pedTypeToAdd - PEDTYPE_GANG1); // rouz edit (ChatGPT)

					if (modelToAdd == -1)
						return;
					pedTypeToAdd = ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(modelToAdd)))->m_pedType; // rouz edit (ChatGPT)

				}
			}
		}
		if (!addCop && Population_allRandomPedsType > PEDTYPE_PLAYER1) // rouz edit (ChatGPT)
			pedTypeToAdd = Population_allRandomPedsType; // rouz edit (ChatGPT)

		if (pedTypeToAdd >= PEDTYPE_GANG1 && pedTypeToAdd <= PEDTYPE_GANG9 && !isSecurityGuard) {
			minDist += 30.0f;
			maxDist += 30.0f;
			pedAmount = Population_ComputeRandomisedGangSize(); // rouz edit (ChatGPT)
		} else
			pedAmount = 1;

		CVector generatedCoors;
		int node1, node2;
		float randomPos;
		bool foundCoors = !!ThePaths.GeneratePedCreationCoors(playerCentreOfWorld.x, playerCentreOfWorld.y, minDist, maxDist, minDistOffScreen, maxDistOffScreen,
			&generatedCoors, &node1, &node2, &randomPos, nil);

		if (!foundCoors)
			return;

		uint8 nodeSpawnRate = Min(ThePaths.m_pathNodes[node1].spawnRate, ThePaths.m_pathNodes[node2].spawnRate);
		int randomRate = General_GetRandomNumber() & 0xF;
		if (randomRate > nodeSpawnRate)
			return;

		CPathFind::TakeWidthIntoAccountForCoors(&ThePaths.m_pathNodes[node1], &ThePaths.m_pathNodes[node2], General_GetRandomNumber(), &generatedCoors.x, &generatedCoors.y);
		if (CGame::currArea == AREA_MALL && (pedTypeToAdd == PEDTYPE_CIVMALE || pedTypeToAdd == PEDTYPE_CIVFEMALE || pedTypeToAdd == PEDTYPE_CRIMINAL) &&
			General_GetRandomNumberInRangeFloat(0.f, 1.f) > 0.5f) {

			PlaceMallPedsAsStationaryGroup(generatedCoors, zoneInfo.pedGroup);
			return;
		}

		if (pedTypeToAdd >= PEDTYPE_GANG1 && pedTypeToAdd <= PEDTYPE_GANG9 && !isSecurityGuard) {
			PlaceGangMembers((ePedType)pedTypeToAdd, pedAmount, generatedCoors);
			return;
		}

		if (man > -1 && woman > -1) {
			PlaceCouple(PEDTYPE_CIVMALE, man, PEDTYPE_CIVFEMALE, woman, generatedCoors);
			return;
		}

		for (int i = 0; i < pedAmount; ++i) {

			if (pedTypeToAdd == PEDTYPE_COP) {
				// Unused code, ChoosePolicePedOccupation returns COP_STREET. Spawning FBI/SWAT/Army done in somewhere else.
				if (modelToAdd == COP_STREET) {
					if (!Streaming_HasModelLoaded(MI_COP)) // rouz edit (ChatGPT)
						return;

				} else if (modelToAdd == COP_FBI) {
					if (!Streaming_HasModelLoaded(MI_COP) || !Streaming_HasModelLoaded(WeaponInfo_GetWeaponInfo(WEAPONTYPE_MP5)->m_nModelId)) // rouz edit (ChatGPT)
						return;

				} else if (modelToAdd == COP_SWAT) {
					if (!Streaming_HasModelLoaded(MI_SWAT) || !Streaming_HasModelLoaded(WeaponInfo_GetWeaponInfo(WEAPONTYPE_UZI)->m_nModelId)) // rouz edit (ChatGPT)
						return;

				} else if (modelToAdd == COP_ARMY) {
					if (!Streaming_HasModelLoaded(MI_ARMY) || // rouz edit (ChatGPT)
						!Streaming_HasModelLoaded(WeaponInfo_GetWeaponInfo(WEAPONTYPE_MP5)->m_nModelId) || !Streaming_HasModelLoaded(WeaponInfo_GetWeaponInfo(WEAPONTYPE_GRENADE)->m_nModelId)) // rouz edit (ChatGPT)
						return;
				}
			} else if (!Streaming_HasModelLoaded(modelToAdd)) { // rouz edit (ChatGPT)
				return;
			}
			generatedCoors.z += 0.7f;

			// What? How can this not be met?
			if (i < pedAmount) {
				// rand()
				// III leftover, unused
				if (gangLeader) {
					// Align gang members in formation. (btw i can't be 0 in here)
					float offsetMin = i * 0.75f;
					float offsetMax = (i + 1.0f) * 0.75f - offsetMin;
					float xOffset = General_GetRandomNumberInRangeFloat(offsetMin, offsetMin + offsetMax);
					float yOffset = General_GetRandomNumberInRangeFloat(offsetMin, offsetMin + offsetMax);
					if (General_GetRandomNumber() & 1)
						xOffset = -xOffset;
					if (General_GetRandomNumber() & 1)
						yOffset = -yOffset;
					generatedCoors.x = xOffset + gangLeader->GetPosition().x;
					generatedCoors.y = yOffset + gangLeader->GetPosition().y;
				}
			}
			if (!PedPlacement_CIsClear(generatedCoors.x, generatedCoors.y, generatedCoors.z, -1.0f, -1, nil)) // rouz edit (ChatGPT)
				break;

			// Why no love for last gang member?!
			if (i + 1 < pedAmount) {
				bool foundGround;
				float groundZ = CWorld::FindGroundZFor3DCoord(generatedCoors.x, generatedCoors.y, 2.0f + generatedCoors.z, &foundGround) + 0.7f;
				if (!foundGround)
					return;

				generatedCoors.z = Max(generatedCoors.z, groundZ);
			}
			bool surfaceAndDistIsOk = true;
			if (TheCamera.IsSphereVisible(generatedCoors, 2.0f)) {
				if (Population_PedCreationDistMultiplier() * MIN_CREATION_DIST > (generatedCoors - playerCentreOfWorld).Magnitude2D()) // rouz edit (ChatGPT)
					surfaceAndDistIsOk = false;
			}

			// Place skaters if only they're on tarmac.
			if (((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(modelToAdd)))->m_pedStatType == PEDSTAT_SKATER) { // rouz edit (ChatGPT)
				CEntity* foundEnt = nil;
				CColPoint foundCol;
				CWorld::ProcessVerticalLine(generatedCoors + CVector(0.f, 0.f, 2.f), generatedCoors.z - 2.0f, foundCol, foundEnt, true, false, false, false, false, false, nil);
				if (foundEnt) {
					if (foundCol.surfaceB == SURFACE_TARMAC || foundCol.surfaceB == SURFACE_PAVEMENT)
						surfaceAndDistIsOk = true;
					else
						surfaceAndDistIsOk = false;

				} else {
					surfaceAndDistIsOk = false;
				}
			}
			if (!surfaceAndDistIsOk)
				break;
			CPed *newPed = AddPed((ePedType)pedTypeToAdd, modelToAdd, generatedCoors);
			if (forceAddingCop && newPed->m_nPedType == PEDTYPE_COP)
				((CCopPed*)newPed)->m_bThrowsSpikeTrap = true;

			bool gonnaSunbathe = false;
			if (Population_IsSunbather(modelToAdd)) { // rouz edit (ChatGPT)
				CEntity* foundEnt = nil;
				CColPoint foundCol;
				CWorld::ProcessVerticalLine(generatedCoors + CVector(0.f, 0.f, 2.f), generatedCoors.z - 2.0f, foundCol, foundEnt, true, false, false, false, false, false, nil);
				if (foundEnt) {
					if ((foundCol.surfaceB == SURFACE_CONCRETE_BEACH || foundCol.surfaceB == SURFACE_SAND)
						&& Clock_GetHours() >= 10 && Clock_GetHours() <= 18 && 0.0f == Weather_Rain) {
						gonnaSunbathe = true;
						if (PedPlacement_CIsClear(generatedCoors.x, generatedCoors.y, generatedCoors.z, 3.0f, ARRAY_SIZE(gSunbatheObstacles), gSunbatheObstacles)) { // rouz edit (ChatGPT)
							for (int j = 0; j < ARRAY_SIZE(gSunbatheObstacles); j++) {
								if (gSunbatheObstacles[j] && gSunbatheObstacles[j] != newPed)
									gonnaSunbathe = false;
							}
						}
					}
				}
			}
			if (gonnaSunbathe) {
				float heading = General_GetRandomNumberInRangeFloat(0.f, 1.f) * TWOPI;
				newPed->m_fRotationDest = heading;
				newPed->m_fRotationCur = heading;
				// unused
				// v61 = CGeneral::GetRandomTrueFalse();
				newPed->SetWaitState(WAITSTATE_SUN_BATHE_IDLE, nil);
				CVector toyPos(newPed->GetPosition());
				float waterLevel;
				if (CWaterLevel::GetGroundLevel(toyPos, &waterLevel, nil, 30.0f)) {
					toyPos.z = 0.04f + waterLevel;
					CEntity *toy = CWaterLevel::CreateBeachToy(toyPos, BEACHTOY_ANY_TOWEL);
					if (toy)
						toy->SetHeading(heading);

					if (!(General_GetRandomNumber() & 3)) {
						CWaterLevel::CreateBeachToy(toyPos + CVector(General_GetRandomNumberInRangeFloat(-2.f, 2.f), General_GetRandomNumberInRangeFloat(-2.f, 2.f), 0.f), BEACHTOY_LOTION);
					}
				}
			} else {
				newPed->SetWanderPath(General_GetRandomNumberInRangeInt(0, 8));
			}
			
			if (i != 0) {
				// Gang member
				newPed->SetLeader(gangLeader);

				newPed->SetPedState(PED_UNKNOWN);
				gangLeader->SetPedState(PED_UNKNOWN);

			} else {
				gangLeader = newPed;
			}
			CVisibilityPlugins::SetClumpAlpha(newPed->GetClump(), 0);
			/*
			// Pointless, this is already a for loop
			if (i + 1 > pedAmount)
				break;
			if (pedAmount <= 1)
				break; */
		}
	}
}

CPed*
CPopulation::AddPedInCar(CVehicle* car, bool isDriver)
{
	const int defaultModel = MI_MALE01;
	int miamiViceIndex = 0;
	bool imSureThatModelIsLoaded = true;
	CVector coors = FindPlayerCoors();
	CZoneInfo zoneInfo;
	int pedType;

	// May be eCopType, model index or non-sense(for medic), AddPed knows that by looking to ped type.
	int preferredModel;

	Zones_GetZoneInfoForTimeOfDay(ZonesPosition_FromVector(*(&coors)), &zoneInfo);
	switch (car->GetModelIndex()) {
		case MI_FIRETRUCK:
			preferredModel = 0;
			pedType = PEDTYPE_FIREMAN;
			break;
		case MI_AMBULAN:
			preferredModel = 0;
			pedType = PEDTYPE_EMERGENCY;
			break;
		case MI_POLICE:
		case MI_PREDATOR:
			preferredModel = COP_STREET;
			pedType = PEDTYPE_COP;
			break;
		case MI_ENFORCER:
			preferredModel = COP_SWAT;
			pedType = PEDTYPE_COP;
			break;
		case MI_RHINO:
		case MI_BARRACKS:
			preferredModel = COP_ARMY;
			pedType = PEDTYPE_COP;
			break;
		case MI_FBIRANCH:
			preferredModel = COP_FBI;
			pedType = PEDTYPE_COP;
			break;
		default:
			if (car->IsTaxi()) {
				if (isDriver) {
					pedType = PEDTYPE_CIVMALE;
					preferredModel = MI_TAXI_D;
					break;
				}
				// fall through if not
			} else if (car->GetModelIndex() == MI_VICECHEE) {
				if (car->bIsLawEnforcer) {
					preferredModel = COP_MIAMIVICE;
					pedType = PEDTYPE_COP;
					miamiViceIndex = (isDriver ? 2 * CCarCtrl::MiamiViceCycle : 2 * CCarCtrl::MiamiViceCycle + 1);
					break;
				}
				// fall through if not
			}

			int gangOfPed = 0;
			imSureThatModelIsLoaded = false;

			while (gangOfPed < NUM_GANGS && Gangs_GetGangInfo(gangOfPed)->m_nVehicleMI != car->GetModelIndex())
				gangOfPed++;

			if (gangOfPed < NUM_GANGS) {
				pedType = gangOfPed + PEDTYPE_GANG1;
				preferredModel = Population_ChooseGangOccupation(gangOfPed); // rouz edit (ChatGPT)
			} else if (gangOfPed == NUM_GANGS) {
			    CVehicleModelInfo *carModel = ((CVehicleModelInfo *)((CBaseModelInfo*)ModelRegistry_Get(car->GetModelIndex()))); // rouz edit (ChatGPT)
				preferredModel = Population_ChooseCivilianOccupation(zoneInfo.pedGroup); // rouz edit (ChatGPT)
				int i = 15;
				for(; i >= 0; i--) {
					CPedModelInfo* pedModel = (CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(preferredModel)); // rouz edit (ChatGPT)

					if (pedModel->GetRwObject()) {
						if (!car->IsPassenger(preferredModel) && !car->IsDriver(preferredModel)) {
							if (((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(preferredModel)))->m_carsCanDrive & (1 << carModel->m_vehicleClass)) // rouz edit (ChatGPT)
								break;
						}
					}

					preferredModel = Population_ChooseNextCivilianOccupation(zoneInfo.pedGroup); // rouz edit (ChatGPT)
				}
				if (i == -1)
					preferredModel = defaultModel;

				pedType = ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(preferredModel)))->m_pedType; // rouz edit (ChatGPT)
			}
			break;
	}
	if (!imSureThatModelIsLoaded && !((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(preferredModel)))->GetRwObject()) { // rouz edit (ChatGPT)
		preferredModel = defaultModel;
		pedType = ((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(defaultModel)))->m_pedType; // rouz edit (ChatGPT)
	}

	CPed *newPed = CPopulation::AddPed((ePedType)pedType, preferredModel, car->GetPosition(), miamiViceIndex);
	newPed->bUsesCollision = false;

	if (newPed->GetWeapon()->m_eWeaponType != WEAPONTYPE_UNARMED) {
		newPed->RemoveWeaponModel(WeaponInfo_GetWeaponInfo(newPed->GetWeapon()->m_eWeaponType)->m_nModelId);
	}

	newPed->AddInCarAnims(car, isDriver);
	return newPed;
}

void
CPopulation::MoveCarsAndPedsOutOfAbandonedZones()
{
}

void
CPopulation::ConvertAllObjectsToDummyObjects()
{
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(CPools::GetObjectPool());
	//- rouz edit (ChatGPT)
	while(i--) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CObject *obj = ((CObject*)CPool_GetSlot(CPools::GetObjectPool(), i));
		//- rouz edit (ChatGPT)
		if (obj) {
			if (obj->CanBeDeleted())
				ConvertToDummyObject(obj);
		}
	}
}

void
CPopulation::ConvertToRealObject(CDummyObject *dummy)
{
	if (!TestSafeForRealObject(dummy))
		return;

//+ rouz edit (ChatGPT)
	// Allocate the real object without invoking C++ new.
	CObjectPool *objectPool = CPools::GetObjectPool();
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CObject *obj = ((CObject*)CPool_New(objectPool));
	//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
	if (!obj) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		for (int32 i = 0; i < CPool_GetSize(objectPool); i++) {
			// Access raw storage through the C store or pool API
			CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
		//- rouz edit (ChatGPT)
			if (existing && existing->ObjectCreatedBy == TEMP_OBJECT) {
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				int32 handle = CPool_GetIndex(objectPool, existing);
				//- rouz edit (ChatGPT)
				CWorld::Remove(existing);
				existing->~CObject();
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(objectPool, existing);
				obj = ((CObject*)CPool_NewAt(objectPool, handle));
				//- rouz edit (ChatGPT)
				break;
			}
		}
	}
	if (obj)
#endif
		std::allocator<CObject>().construct(obj, dummy);
//- rouz edit (ChatGPT)
	if (!obj)
		return;

	CWorld::Remove(dummy);
//+ rouz edit (ChatGPT)
	// Destroy and release the dummy without invoking C++ delete.
	dummy->~CDummyObject();
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPool_Delete(CPools::GetDummyPool(), dummy);
	//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
	CWorld::Add(obj);

	CSimpleModelInfo *mi = (CSimpleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(obj->GetModelIndex())); // rouz edit (ChatGPT)
	if (IsGlass(obj->GetModelIndex()) && !mi->m_isArtistGlass) {
		obj->bIsVisible = false;
	} else if (obj->GetModelIndex() == MI_BUOY) {
		obj->SetIsStatic(false);
		obj->m_vecMoveSpeed = CVector(0.0f, 0.0f, -0.001f);
		obj->bTouchingWater = true;
		obj->AddToMovingList();
	}
}

void
CPopulation::ConvertToDummyObject(CObject *obj)
{
//+ rouz edit (ChatGPT)
	// Allocate the dummy object without invoking C++ new.
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CDummyObject *dummy = (CDummyObject*)((CDummy*)CPool_New(CPools::GetDummyPool()));
	//- rouz edit (ChatGPT)
	assert(dummy);
	std::allocator<CDummyObject>().construct(dummy, obj);
//- rouz edit (ChatGPT)

	dummy->GetMatrix() = obj->m_objectMatrix;
	dummy->GetMatrix().UpdateRW();
	dummy->UpdateRwFrame();

	CSimpleModelInfo *mi = (CSimpleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(obj->GetModelIndex())); // rouz edit (ChatGPT)
	if (IsGlass(obj->GetModelIndex()) && !mi->m_isArtistGlass)
		dummy->bIsVisible = false;

	CWorld::Remove(obj);
//+ rouz edit (ChatGPT)
	// Destroy and release the real object without invoking C++ delete.
	obj->~CObject();
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPool_Delete(CPools::GetObjectPool(), obj);
	//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
	CWorld::Add(dummy);
}

bool
CPopulation::TestRoomForDummyObject(CObject *obj)
{
	int16 collidingObjs;
	CWorld::FindObjectsKindaColliding(obj->m_objectMatrix.GetPosition(), obj->GetBoundRadius(),
		false, &collidingObjs, 2, nil, false, true, true, false, false);

	return collidingObjs == 0;
}

bool
CPopulation::TestSafeForRealObject(CDummyObject *dummy)
{
	CPtrNode *ptrNode;
	CColModel *dummyCol = dummy->GetColModel();

	float radius = dummyCol->boundingSphere.radius;
	int minX = CWorld::GetSectorIndexX(dummy->GetPosition().x - radius);
	if (minX < 0) minX = 0;
	int minY = CWorld::GetSectorIndexY(dummy->GetPosition().y - radius);
	if (minY < 0) minY = 0;
	int maxX = CWorld::GetSectorIndexX(dummy->GetPosition().x + radius);
#ifdef FIX_BUGS
	if (maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X - 1;
#else
	if (maxX >= NUMSECTORS_X) maxX = NUMSECTORS_X;
#endif

	int maxY = CWorld::GetSectorIndexY(dummy->GetPosition().y + radius);
#ifdef FIX_BUGS
	if (maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y - 1;
#else
	if (maxY >= NUMSECTORS_Y) maxY = NUMSECTORS_Y;
#endif

	float colRadius = dummy->GetBoundRadius();
	CVUVECTOR colCentre;
	dummy->GetBoundCentre(colCentre);

	static CColPoint aTempColPoints[MAX_COLLISION_POINTS];

	for (int curY = minY; curY <= maxY; curY++) {
		for (int curX = minX; curX <= maxX; curX++) {
			CSector *sector = CWorld::GetSector(curX, curY);

			for (ptrNode = sector->m_lists[ENTITYLIST_VEHICLES].first; ptrNode; ptrNode = ptrNode->next) {
				CVehicle *veh = (CVehicle*)ptrNode->item;
				if (veh->m_scanCode != CWorld::GetCurrentScanCode()) {
					if (veh->GetIsTouching(colCentre, colRadius)) {
						veh->m_scanCode = CWorld::GetCurrentScanCode();
						if (CCollision::ProcessColModels(dummy->GetMatrix(), *dummyCol, veh->GetMatrix(), *veh->GetColModel(), aTempColPoints, nil, nil) > 0)
							return false;
					}
				}
			}

			for (ptrNode = sector->m_lists[ENTITYLIST_VEHICLES_OVERLAP].first; ptrNode; ptrNode = ptrNode->next) {
				CVehicle *veh = (CVehicle*)ptrNode->item;
				if (veh->m_scanCode != CWorld::GetCurrentScanCode()) {
					if (veh->GetIsTouching(colCentre, colRadius)) {
						veh->m_scanCode = CWorld::GetCurrentScanCode();
						if (CCollision::ProcessColModels(dummy->GetMatrix(), *dummyCol, veh->GetMatrix(), *veh->GetColModel(), aTempColPoints, nil, nil) > 0)
							return false;
					}
				}
			}
		}
	}
	return true;
}

void
CPopulation::ManagePopulation(void)
{
	if (FindPlayerPed()==NULL)	// rouz edit
		return;

	int frameMod32 = (*Timer_GetFrameCounter()) & 31;
	CVector playerPos = FindPlayerCentreOfWorld(CWorld::PlayerInFocus);

	// Why this code is here?! Delete temporary objects when they got too far, and convert others to "dummy" objects. (like lamp posts)
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int objectPoolSize = CPool_GetSize(CPools::GetObjectPool());
	//- rouz edit (ChatGPT)
	for (int i = objectPoolSize * frameMod32 / 32; i < objectPoolSize * (frameMod32 + 1) / 32; i++) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CObject *obj = ((CObject*)CPool_GetSlot(CPools::GetObjectPool(), i));
		//- rouz edit (ChatGPT)
		if (obj && obj->CanBeDeleted()) {
			float objPlayerDist = (obj->GetPosition() - playerPos).Magnitude();
			if (obj->ObjectCreatedBy == TEMP_OBJECT) {
				if (obj->GetModelIndex() != MI_ROADWORKBARRIER1 && obj->GetModelIndex() != MI_BEACHBALL) {
					if (objPlayerDist > 51.0f || objPlayerDist > 25.0f && !obj->GetIsOnScreen() || (*Timer_GetTimeInMilliseconds()) > obj->m_nEndOfLifeTime) {
						CWorld::Remove(obj);
//+ rouz edit (ChatGPT)
						// Destroy and release the temporary object without invoking C++ delete.
						obj->~CObject();
						//+ rouz edit (ChatGPT)
						// Access raw storage through the C store or pool API
						CPool_Delete(CPools::GetObjectPool(), obj);
						//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
					}
				} else if (objPlayerDist > 120.0f) {
					CWorld::Remove(obj);
//+ rouz edit (ChatGPT)
					// Destroy and release the temporary object without invoking C++ delete.
					obj->~CObject();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(CPools::GetObjectPool(), obj);
					//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
				}

			} else if (objPlayerDist > 80.0f && (obj->m_objectMatrix.GetPosition() - playerPos).Magnitude() > 80.0f) {
				if (obj->ObjectCreatedBy != CUTSCENE_OBJECT && TestRoomForDummyObject(obj)) {
					ConvertToDummyObject(obj);
				}
			}
		}
	}

	// Convert them back to real objects. Dummy objects don't have collisions, so they need to be converted.
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int dummyPoolSize = CPool_GetSize(CPools::GetDummyPool());
	//- rouz edit (ChatGPT)
	for (int i = dummyPoolSize * frameMod32 / 32; i < dummyPoolSize * (frameMod32 + 1) / 32; i++) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CDummy *dummy = ((CDummy*)CPool_GetSlot(CPools::GetDummyPool(), i));
		//- rouz edit (ChatGPT)
		if (dummy && (dummy->m_area == CGame::currArea || dummy->m_area == AREA_EVERYWHERE)) {
			if ((dummy->GetPosition() - playerPos).Magnitude() < 80.0f)
				ConvertToRealObject((CDummyObject*)dummy);
		}
	}

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int pedPoolSize = CPool_GetSize(CPools::GetPedPool());
	//- rouz edit (ChatGPT)
#ifndef SQUEEZE_PERFORMANCE
	for (int poolIndex = pedPoolSize-1; poolIndex >= 0; poolIndex--) {
#else
	for (int poolIndex = (pedPoolSize * (frameMod32 + 1) / 32) - 1; poolIndex >= pedPoolSize * frameMod32 / 32; poolIndex--) {
#endif
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPed *ped = ((CPed*)CPool_GetSlot(CPools::GetPedPool(), poolIndex));
		//- rouz edit (ChatGPT)

		if (ped && !ped->IsPlayer() && ped->CanBeDeleted() && !ped->bInVehicle) {
			uint32 timeSinceDeath = (*Timer_GetTimeInMilliseconds()) - ped->m_bloodyFootprintCountOrDeathTime;
			if (ped->m_nPedState == PED_DEAD && (timeSinceDeath > 30000 || Darkel_FrenzyOnGoing() && timeSinceDeath > 15000))
				ped->bFadeOut = true;

			if (ped->bFadeOut && CVisibilityPlugins::GetClumpAlpha(ped->GetClump()) == 0) {
				RemovePed(ped);
				continue;
			}

			float dist = (ped->GetPosition() - playerPos).Magnitude2D();

			bool pedIsFarAway = false;

			if (ped->IsGangMember())
				dist -= 30.0f;
			else if (ped->bDeadPedInFrontOfCar && ped->m_vehicleInAccident)
				dist = 0.0f;

			if (Population_PedCreationDistMultiplier() * (PED_REMOVE_DIST_SPECIAL * TheCamera.GenerationDistMultiplier) < dist || // rouz edit (ChatGPT)
				(!ped->bCullExtraFarAway && Population_PedCreationDistMultiplier() * PED_REMOVE_DIST * TheCamera.GenerationDistMultiplier < dist)) { // rouz edit (ChatGPT)
				pedIsFarAway = true;
			}
#ifndef EXTENDED_OFFSCREEN_DESPAWN_RANGE
			else if (Population_PedCreationDistMultiplier() * (MIN_CREATION_DIST + CREATION_RANGE) * OFFSCREEN_CREATION_MULT < dist) { // rouz edit (ChatGPT)
				if ((*Timer_GetTimeInMilliseconds()) > ped->m_nExtendedRangeTimer && !ped->GetIsOnScreen()) {
					if (TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_SNIPER
						&& TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_SNIPER_RUNABOUT
						&& TheCamera.Cams[TheCamera.ActiveCam].Mode != CCam::MODE_CAMERA
						&& !TheCamera.Cams[TheCamera.ActiveCam].LookingLeft
						&& !TheCamera.Cams[TheCamera.ActiveCam].LookingRight
						&& !TheCamera.Cams[TheCamera.ActiveCam].LookingBehind) {
						pedIsFarAway = true;
					}
				}
						
			}
#endif
			else {
				ped->m_nExtendedRangeTimer = ped->m_nPedType == PEDTYPE_COP ? (*Timer_GetTimeInMilliseconds()) + 10000 : (*Timer_GetTimeInMilliseconds()) + 4000;
			}

			if (!pedIsFarAway)
				continue;

			if (ped->m_nPedState == PED_DEAD && !ped->bFadeOut) {
				CVector pedPos = ped->GetPosition();

				float randAngle = (uint8) General_GetRandomNumber() * (3.14f / 128.0f); // Not PI, 3.14
				switch (General_GetRandomNumber() % 3) {
					case 0:
						CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpOutline1Tex, &pedPos,
							0.9f * Cos(randAngle), 0.9f * Sin(randAngle), 0.9f * Sin(randAngle), -0.9f * Cos(randAngle),
							255, 255, 255, 255, 4.0f, 40000, 1.0f);
						break;
					case 1:
						CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpOutline2Tex, &pedPos,
							0.9f * Cos(randAngle), 0.9f * Sin(randAngle), 0.9f * Sin(randAngle), -0.9f * Cos(randAngle),
							255, 255, 255, 255, 4.0f, 40000, 1.0f);
						break;
					case 2:
						CShadows::AddPermanentShadow(SHADOWTYPE_DARK, gpOutline3Tex, &pedPos,
							0.9f * Cos(randAngle), 0.9f * Sin(randAngle), 0.9f * Sin(randAngle), -0.9f * Cos(randAngle),
							255, 255, 255, 255, 4.0f, 40000, 1.0f);
						break;
					default:
						break;
				}
			}
			if (ped->GetIsOnScreen())
				ped->bFadeOut = true;
			else
				RemovePed(ped);
		}
	}
}

CPed* 
CPopulation::AddDeadPedInFrontOfCar(const CVector& pos, CVehicle* pCulprit)
{
	if (TheCamera.IsSphereVisible(pos, 2.0f) && MIN_CREATION_DIST * Population_PedCreationDistMultiplier() > (pos - FindPlayerPed()->GetPosition()).Magnitude2D()) { // rouz edit (ChatGPT)
			return nil;
	}

	bool found;
	float z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &found) + 1.0f;
	if (!found)
		return nil;
	z = Max(z, pos.z);
	if (!((CBaseModelInfo*)ModelRegistry_Get(MI_MALE01))->GetRwObject()) // rouz edit (ChatGPT)
		return nil;
	CPed* pPed = CPopulation::AddPed(PEDTYPE_CIVMALE, MI_MALE01, pos);
	pPed->SetDie();
	pPed->m_nPedMoney = 0;
	pPed->bDeadPedInFrontOfCar = true;
	pPed->m_vehicleInAccident = pCulprit;
	pCulprit->RegisterReference((CEntity**)&pPed->m_vehicleInAccident);
	CEntity* pEntities[3] = { 0 };
	if (!PedPlacement_CIsClear(pos.x, pos.y, pos.z, 2.0f, 3, pEntities)) { // rouz edit (ChatGPT)
		for (int i = 0; i < 3; i++) {
			if (pEntities[i] && pEntities[i] != pCulprit && pEntities[i] != pPed) {
				RemovePed(pPed);
				return nil;
			}
		}
	}
	CColPoint colpts[MAX_COLLISION_POINTS];
	if (CCollision::ProcessColModels(pCulprit->GetMatrix(), *pCulprit->GetColModel(), pPed->GetMatrix(), *pPed->GetColModel(), colpts, nil, nil)) {
		RemovePed(pPed);
		return nil;
	}
	CVisibilityPlugins::SetClumpAlpha(pPed->GetClump(), 0);
	return pPed;
}

bool
CPopulation::IsSkateable(CVector const& pos)
{
	CColPoint foundCol;
	CEntity* foundEnt = nil;
	CWorld::ProcessVerticalLine(pos + CVector(0.f, 0.f, 2.f), pos.z - 2.0f, foundCol, foundEnt, true, false, false, false, false, false, nil);
	if (!foundEnt)
		return false;

	return foundCol.surfaceB == SURFACE_TARMAC || foundCol.surfaceB == SURFACE_PAVEMENT;
}



void
CPopulation::RemovePedsIfThePoolGetsFull(void)
{
	if (((*Timer_GetFrameCounter()) & 7) == 5) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		if (CPool_GetNoOfFreeSpaces(CPools::GetPedPool()) < 8) {
		//- rouz edit (ChatGPT)
			CPed *closestPed = nil;
			float closestDist = 10000000.0;
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			int poolSize = CPool_GetSize(CPools::GetPedPool());
			//- rouz edit (ChatGPT)
			for (int i = poolSize - 1; i >= 0; i--) {
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPed* ped = ((CPed*)CPool_GetSlot(CPools::GetPedPool(), i));
				//- rouz edit (ChatGPT)
				if (ped && ped->CanBeDeleted()) {
					float dist = (TheCamera.GetPosition() - ped->GetPosition()).Magnitude();
					if (dist < closestDist) {
						closestDist = dist;
						closestPed = ped;
					}
				}
			}
			if (closestPed) {
				RemovePed(closestPed);
			}
		}
	}
}

















void
CPopulation::PlaceGangMembers(ePedType pedType, int pedAmount, CVector const& coors)
{
	if (General_GetRandomNumberInRangeFloat(0.f, 1.f) < 0.333f) {
		PlaceGangMembersInFormation(pedType, pedAmount, coors);
	} else {
		PlaceGangMembersInCircle(pedType, pedAmount, coors);
	}
}

void
CPopulation::PlaceGangMembersInFormation(ePedType pedType, int pedAmount, CVector const& coors)
{
	CPed *createdPeds[5];

	if (!TheCamera.IsSphereVisible(coors, 3.0f) || MIN_CREATION_DIST * Population_PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) { // rouz edit (ChatGPT)
		if (PedPlacement_CIsClear(coors.x, coors.y, coors.z, 3.0f, -1, nil)) { // rouz edit (ChatGPT)
			bool leaderFoundGround;
			float leaderGroundZ = CWorld::FindGroundZFor3DCoord(coors.x, coors.y, coors.z, &leaderFoundGround) + 1.0f;
			if (leaderFoundGround) {
				float finalZ = coors.z > leaderGroundZ ? coors.z : leaderGroundZ;
				int leaderModel = Population_ChooseGangOccupation(pedType - PEDTYPE_GANG1); // rouz edit (ChatGPT)
				if (((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(leaderModel)))->GetRwObject()) { // rouz edit (ChatGPT)
					CPed *leader = AddPed(pedType, leaderModel, CVector(coors.x, coors.y, finalZ));
					if (leader) {
						leader->SetObjective(OBJECTIVE_NONE);
						leader->SetWanderPath(General_GetRandomNumberInRangeInt(0, 8));
						leader->bIsLeader = true;
						if (Gangs_GetWillAttackPlayerWithCops(pedType))
							leader->bCanAttackPlayerWithCops = true;

						int pedIdx = 1;
						createdPeds[0] = leader;
						for (int i = 1; i < pedAmount; ++i) {
							int memberModel = Population_ChooseGangOccupation(pedType - PEDTYPE_GANG1); // rouz edit (ChatGPT)
							if (!((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(memberModel)))->GetRwObject()) // rouz edit (ChatGPT)
								continue;

							CPed* memberPed = AddPed(pedType, memberModel, CVector(coors.x, coors.y, finalZ));
							if (!memberPed)
								continue;

							memberPed->SetObjective(OBJECTIVE_FOLLOW_CHAR_IN_FORMATION, leader);
							memberPed->SetFormation((eFormation)i);
							CVector formationPos = memberPed->GetFormationPosition();
							CVector finalFormationPos = formationPos;
							bool formationFoundGround;
							float formationGroundZ = CWorld::FindGroundZFor3DCoord(formationPos.x, formationPos.y, 1.0f + formationPos.z, &formationFoundGround) + 1.0f;

							finalFormationPos.z = Max(finalFormationPos.z, formationGroundZ);
							if (formationFoundGround) {
								if (Abs(finalFormationPos.z - leader->GetPosition().z) <= 1.0f) {
									if (CWorld::GetIsLineOfSightClear(finalFormationPos, leader->GetPosition(), true, false, false, false, false, false, false)) {
										memberPed->SetPosition(finalFormationPos);
										createdPeds[pedIdx++] = memberPed;
										if (Gangs_GetWillAttackPlayerWithCops(pedType))
											leader->bCanAttackPlayerWithCops = true;

										CVisibilityPlugins::SetClumpAlpha(memberPed->GetClump(), 0);
										continue;
									}
								}
							}
							RemovePed(memberPed);
						}
						if (pedIdx >= 3) {
							for (int j = 1; j < pedIdx; ++j)
								createdPeds[j]->SetLeader(createdPeds[0]);

						} else {
							for (int k = 0; k < pedIdx; ++k) {
								RemovePed(createdPeds[k]);
							}
						}
					}
				}
			}
		}
	}
}

void
CPopulation::PlaceGangMembersInCircle(ePedType pedType, int pedAmount, CVector const& coors)
{
	CPed *createdPeds[5];

	if (pedAmount < 2)
		return;

	float circleSector = TWOPI / pedAmount;

	float circleR = Sqrt(0.5f / (1.0f - Cos(circleSector)));

	if (!TheCamera.IsSphereVisible(coors, circleR) ||
		MIN_CREATION_DIST * Population_PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) { // rouz edit (ChatGPT)

		if (PedPlacement_CIsClear(coors.x, coors.y, coors.z, circleR, -1, nil)) { // rouz edit (ChatGPT)
			int pedIdx = 0;
			CVector leaderPos;
#ifdef FIX_BUGS
			bool createLeader = true;
#endif

			for (int i = 0; i < pedAmount; i++) {
				float angleMult = i + General_GetRandomNumberInRangeFloat(-0.2f, 0.2f);
				float randomR = circleR + General_GetRandomNumberInRangeFloat(-0.2f, 0.2f) * circleR;
				float xOffset = randomR * Cos(angleMult * circleSector);
				float yOffset = randomR * Sin(angleMult * circleSector);
				bool foundGround;
				float groundZ = CWorld::FindGroundZFor3DCoord(xOffset + coors.x, yOffset + coors.y, coors.z + 1.0, &foundGround) + 1.0f;
				if (foundGround) {
					CVector finalPos(coors.x + xOffset, coors.y + yOffset, coors.z > groundZ ? coors.z : groundZ);
#ifndef FIX_BUGS
					const bool createLeader = i == 0;
#endif
					if (createLeader)
						leaderPos = finalPos;

					int gangModel = Population_ChooseGangOccupation(pedType - PEDTYPE_GANG1); // rouz edit (ChatGPT)
					if (((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(gangModel)))->GetRwObject()) { // rouz edit (ChatGPT)
						CEntity* obstacles[6] = { nil, nil, nil, nil, nil, nil };
						PedPlacement_CIsClear(finalPos.x, finalPos.y, finalPos.z, ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(gangModel)))->boundingSphere.radius, ARRAY_SIZE(obstacles), obstacles); // rouz edit (ChatGPT)
						bool foundObstacle = false;
						for (int m = 0; m < ARRAY_SIZE(obstacles); m++) {
							CEntity* obstacle = obstacles[m];
							if (obstacle) {
								int n = 0;
								bool obstacleIsHarmless = false;
								for (int n = 0; n < pedIdx; n++) {
									if (obstacle == createdPeds[n])
										obstacleIsHarmless = true;
								}
								if (!obstacleIsHarmless) {
									foundObstacle = true;
									break;
								}
							}
						}
						bool memberCanSeeLeader = createLeader ? true : CWorld::GetIsLineOfSightClear(finalPos, leaderPos, true, false, false, false, false, false, false);

						bool notTooHighFromLeader = createLeader ? true : !(Abs(finalPos.z - leaderPos.z) >= 1.0f);

						if (!foundObstacle && memberCanSeeLeader && notTooHighFromLeader) {
							CPed* newPed = AddPed(pedType, gangModel, finalPos);
							if (newPed) {
								createdPeds[pedIdx++] = newPed;
								float angle = General_GetRadianAngleBetweenPoints(
									coors.x, coors.y,
									finalPos.x, finalPos.y);
								newPed->m_fRotationDest = angle;
								newPed->m_fRotationCur = angle;
								if (Gangs_GetWillAttackPlayerWithCops(pedType))
									newPed->bCanAttackPlayerWithCops = true;

								CVisibilityPlugins::SetClumpAlpha(newPed->GetClump(), 0);
#ifdef FIX_BUGS
								createLeader = false;
#endif
							}
							// No.
#ifndef FIX_BUGS
							else
								CWorld::Remove(nil);
#endif
						}
					}
				}
			}
			if (pedIdx >= 3) {
				for (int j = 0; j < pedIdx / 2; ++j) {
					createdPeds[j]->SetChat(createdPeds[pedIdx - 1 - j], 100000);
					createdPeds[pedIdx - 1 - j]->SetChat(createdPeds[j], 100000);
				}

				// Make that extra guy in the middle stand there(PED_UNKNOWN locks him) and do nothing :lmao:
				if (pedIdx % 2 != 0) {
					CPed *tmim = createdPeds[(pedIdx - 1) / 2];
					float angle = General_GetRadianAngleBetweenPoints(
						tmim->GetPosition().x, tmim->GetPosition().y,
						createdPeds[0]->GetPosition().x, createdPeds[0]->GetPosition().y);
					tmim->SetHeading(angle);
					tmim->SetPedState(PED_UNKNOWN);
				}
				createdPeds[0]->bIsLeader = true;

				for (int l = 1; l < pedIdx; ++l)
					createdPeds[l]->SetLeader(createdPeds[0]);

			} else {
				for (int k = 0; k < pedIdx; ++k) {
					RemovePed(createdPeds[k]);
				}
			}
		}
	}
}

void
CPopulation::PlaceCouple(ePedType manType, int32 manModel, ePedType womanType, int32 womanModel, CVector coors)
{
	// Homosexuality filter!!!! Homophobic R* >>>:(
	if (manType != PEDTYPE_CIVMALE || womanType != PEDTYPE_CIVFEMALE)
		return;

	if (!TheCamera.IsSphereVisible(coors, 1.5f) || MIN_CREATION_DIST * Population_PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) { // rouz edit (ChatGPT)
		if (PedPlacement_CIsClear(coors.x, coors.y, coors.z, ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(manModel)))->boundingSphere.radius, -1, nil)) { // rouz edit (ChatGPT)
			bool manFoundGround;
			float manGroundZ = CWorld::FindGroundZFor3DCoord(coors.x, coors.y, coors.z, &manFoundGround) + 1.0f;
			if (manFoundGround) {
				CVector correctedManPos = coors;
				correctedManPos.z = Max(coors.z, manGroundZ);
				if (((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(manModel)))->GetRwObject()) { // rouz edit (ChatGPT)
					CPed *man = AddPed(PEDTYPE_CIVMALE, manModel, correctedManPos);
					if (man) {
						man->SetObjective(OBJECTIVE_NONE);
						man->SetWanderPath(General_GetRandomNumberInRangeInt(0, 8));
						man->bIsLeader = true;
						CVisibilityPlugins::SetClumpAlpha(man->GetClump(), 0);

						if (((CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(womanModel)))->GetRwObject()) { // rouz edit (ChatGPT)
							CPed* woman = AddPed(PEDTYPE_CIVFEMALE, womanModel, correctedManPos); // will set the correct position later
							if (woman) {
								woman->SetObjective(OBJECTIVE_FOLLOW_CHAR_IN_FORMATION, man);
								woman->SetFormation(FORMATION_RIGHT);

								CVector formationPos = woman->GetFormationPosition();
								CVector womanPos = formationPos;
								bool womanFoundGround;
								float formationGroundZ = CWorld::FindGroundZFor3DCoord(formationPos.x, formationPos.y, 1.0f + formationPos.z, &womanFoundGround) + 1.0f;

								if (womanFoundGround) {
									CVector correctedWomanPos = womanPos;
									correctedWomanPos.z = Max(womanPos.z, formationGroundZ);
									woman->SetPosition(correctedWomanPos);

									// What's the point of this??
									CEntity* obstacles[3];
									memcpy(obstacles, gCoupleObstacles, sizeof(gCoupleObstacles));

									PedPlacement_CIsClear(womanPos.x, womanPos.y, womanPos.z, ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(womanModel)))->boundingSphere.radius, ARRAY_SIZE(obstacles), obstacles); // rouz edit (ChatGPT)
									for (int i = 0; i < ARRAY_SIZE(obstacles); i++) {
										CEntity *obstacle = obstacles[i];
										if (obstacle) {

											// We found a real obstacle, so let's break and we can delete them...
											if (obstacle != man && obstacle != woman)
												break;
										}
										if (i == ARRAY_SIZE(obstacles) - 1) {
											CVisibilityPlugins::SetClumpAlpha(woman->GetClump(), 0);
											return;
										}
									}
								}
								RemovePed(woman);
								RemovePed(man);
							}
						}
					}
				}
			}
		}
	}
}

// Mostly copy paste of PlaceGangMembersInFormation.
void
CPopulation::PlaceMallPedsAsStationaryGroup(CVector const& coors, int32 group)
{
#ifdef FIX_BUGS
	CPed *createdPeds[6];
#else
	CPed *createdPeds[5];
#endif

	if (CGame::currArea != AREA_MALL)
		return;

	int pedAmount = General_GetRandomNumberInRangeInt(0, 4) + 3;

	float circleSector = TWOPI / pedAmount;

	float circleR = Sqrt(0.5f / (1.0f - Cos(circleSector)));

	if (!TheCamera.IsSphereVisible(coors, circleR) ||
		MIN_CREATION_DIST * Population_PedCreationDistMultiplier() <= (coors - FindPlayerPed()->GetPosition()).Magnitude2D()) { // rouz edit (ChatGPT)

		if (PedPlacement_CIsClear(coors.x, coors.y, coors.z, circleR, -1, nil)) { // rouz edit (ChatGPT)
			int pedIdx = 0;
			CVector leaderPos;
#ifdef FIX_BUGS
			bool createLeader = true;
#endif

			for (int i = 0; i < pedAmount; i++) {	
				float angleMult = i + General_GetRandomNumberInRangeFloat(-0.2f, 0.2f);
				float randomR = circleR + General_GetRandomNumberInRangeFloat(-0.2f, 0.2f) * circleR;
				float xOffset = randomR * Cos(angleMult * circleSector);
				float yOffset = randomR * Sin(angleMult * circleSector);
				bool foundGround;
				float groundZ = CWorld::FindGroundZFor3DCoord(xOffset + coors.x, yOffset + coors.y, coors.z + 1.0, &foundGround) + 1.0f;
				if (foundGround) {
					CVector finalPos(coors.x + xOffset, coors.y + yOffset, coors.z > groundZ ? coors.z : groundZ);

#ifndef FIX_BUGS
					const bool createLeader = i == 0;
#endif
					if (createLeader)
						leaderPos = finalPos;

					int pedModel = Population_ChooseCivilianOccupation(group); // rouz edit (ChatGPT)
					CPedModelInfo *pedModelInfo = (CPedModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(pedModel)); // rouz edit (ChatGPT)

					if (pedModelInfo->GetRwObject()) {
						CEntity* obstacles[6] = { nil, nil, nil, nil, nil, nil };
						PedPlacement_CIsClear(finalPos.x, finalPos.y, finalPos.z, ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(pedModel)))->boundingSphere.radius, ARRAY_SIZE(obstacles), obstacles); // rouz edit (ChatGPT)
						bool foundObstacle = false;
						for (int m = 0; m < ARRAY_SIZE(obstacles); m++) {
							CEntity* obstacle = obstacles[m];
							if (obstacle) {
								int n = 0;
								bool obstacleIsHarmless = false;
								for (int n = 0; n < pedIdx; n++) {
									if (obstacle == createdPeds[n])
										obstacleIsHarmless = true;
								}
								if (!obstacleIsHarmless) {
									foundObstacle = true;
									break;
								}
							}
						}
						bool memberCanSeeLeader = createLeader ? true : CWorld::GetIsLineOfSightClear(finalPos, leaderPos, true, false, false, false, false, false, false);

						bool notTooHighFromLeader = createLeader ? true : !(Abs(finalPos.z - leaderPos.z) >= 1.0f);

						if (!foundObstacle && memberCanSeeLeader && notTooHighFromLeader) {
							CPed *newPed = AddPed(pedModelInfo->m_pedType, pedModel, finalPos);
							if (newPed) {
								createdPeds[pedIdx++] = newPed;
								float angle = General_GetRadianAngleBetweenPoints(
									coors.x, coors.y,
									finalPos.x, finalPos.y);
								newPed->m_fRotationDest = angle;
								newPed->m_fRotationCur = angle;
								newPed->m_fearFlags = 0;
								CVisibilityPlugins::SetClumpAlpha(newPed->GetClump(), 0);
#ifdef FIX_BUGS
								createLeader = false;
#endif
							}
							// No.
#ifndef FIX_BUGS
							else
								CWorld::Remove(nil);
#endif
						}
					}
				}
			}
			if (pedIdx >= 3) {
				for (int j = 0; j < pedIdx / 2; ++j) {
					createdPeds[j]->SetChat(createdPeds[pedIdx - 1 - j], 100000);
					createdPeds[pedIdx - 1 - j]->SetChat(createdPeds[j], 100000);
				}

				// Make that extra guy in the middle stand there(PED_UNKNOWN locks him) and do nothing :lmao:
				if (pedIdx % 2 != 0) {
					CPed *tmim = createdPeds[(pedIdx - 1) / 2];
					float angle = General_GetRadianAngleBetweenPoints(
						tmim->GetPosition().x, tmim->GetPosition().y,
						createdPeds[0]->GetPosition().x, createdPeds[0]->GetPosition().y);
					tmim->SetHeading(angle);
					tmim->SetPedState(PED_UNKNOWN);
				}
				createdPeds[0]->bIsLeader = true;

				for (int l = 1; l < pedIdx; ++l)
					createdPeds[l]->SetLeader(createdPeds[0]);

			} else {
				for (int k = 0; k < pedIdx; ++k) {
					RemovePed(createdPeds[k]);
				}
			}
		}
	}
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
