//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)

#include "CarGen.h"
#include "CarGenLogic.h"

#include "Automobile.h"
#include "Bike.h"
#include "Boat.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CutsceneMgr.h"
#include "General.h"
#include "Pools.h"
#include "Streaming.h"
#include "Timer.h"
#include "Vehicle.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "Zones.h"
#include "Occlusion.h"
#include "SaveBuf.h"

uint8 TheCarGenerators_ProcessCounter;
uint32 TheCarGenerators_NumOfCarGenerators;
CCarGenerator TheCarGenerators_CarGeneratorArray[NUM_CARGENS];
uint8 TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter;
uint32 TheCarGenerators_CurrentActiveCount;

void CCarGenerator::SwitchOff()
{
	// Delegate to the C implementation
	CarGen_SwitchOff(this, &TheCarGenerators_CurrentActiveCount);
}

void CCarGenerator::SwitchOn()
{
	// Delegate to the C implementation
	CarGen_SwitchOn(this, &TheCarGenerators_CurrentActiveCount, (*Timer_GetTimeInMilliseconds()));
}

uint32 CCarGenerator::CalcNextGen()
{
	// Delegate to the C implementation
	return CarGen_CalcNextGen((*Timer_GetTimeInMilliseconds()));
}

void CCarGenerator::DoInternalProcessing()
{
	int mi;
	if (CarCtrl_NumParkedCars >= 10)
		return;
	if (m_nModelIndex >= 0) {
		if (CheckForBlockage(m_nModelIndex)) {
			m_nTimer += 4;
			return;
		}
		Streaming_RequestModel(m_nModelIndex, STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
		mi = m_nModelIndex;
	}
	else {
		mi = -m_nModelIndex;
		if (m_nModelIndex == -1 || !Streaming_HasModelLoaded(mi)) { // rouz edit (ChatGPT)
			CZoneInfo pZone;
			CVector pos = FindPlayerCoors();
			Zones_GetZoneInfoForTimeOfDay(ZonesPosition_FromVector(*(&pos)), &pZone);
			mi = CarCtrl_ChooseCarModel(CarCtrl_ChooseCarRating(&pZone));
			if (mi < 0)
				return;
			m_nModelIndex = -mi;
			m_nColor1 = -1;
			m_nColor2 = -1;
		}
		if (CheckForBlockage(mi)) {
			m_nTimer += 4;
			return;
		}
	}
	if (!Streaming_HasModelLoaded(mi)) // rouz edit (ChatGPT)
		return;
	CVehicle* pVehicle;

	CVector pos;
	if (ModelRegistry_IsBoatModel(mi)){ // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
		// Allocate the parked boat from the vehicle pool without invoking C++ new.
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CBoat* pBoat = (CBoat*)((CVehicle*)CPool_New(Pools_GetVehiclePool()));
		//- rouz edit (ChatGPT)
		assert(pBoat);
		std::allocator<CBoat>().construct(pBoat, mi, PARKED_VEHICLE);
//- rouz edit (ChatGPT)
		pos = m_vecPos;
		pVehicle = pBoat;
		if (pos.z <= -100.0f)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		pBoat->bExtendedRange = true;
	}else{
		bool groundFound;
		pos = m_vecPos;
		if (pos.z > -100.0f){
			pos.z = World_FindGroundZFor3DCoord(pos.x, pos.y, pos.z, &groundFound);
		}else{
			groundFound = false;
			CColPoint cp;
			CEntity* pEntity;
			groundFound = World_ProcessVerticalLine(CVector(pos.x, pos.y, 1000.0f), -1000.0f,
				cp, pEntity, true, false, false, false, false, false, nil);
			if (groundFound)
				pos.z = cp.point.z;
		}
		if (!groundFound) {
			debug("CCarGenerator::DoInternalProcessing - can't find ground z for new car x = %f y = %f \n", m_vecPos.x, m_vecPos.y);
			return;
		}
		if (((CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(mi)))->m_vehicleType == VEHICLE_TYPE_BIKE) { // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
			// Allocate the parked bike from the vehicle pool without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CBike* pBike = (CBike*)((CVehicle*)CPool_New(Pools_GetVehiclePool()));
			//- rouz edit (ChatGPT)
			assert(pBike);
			std::allocator<CBike>().construct(pBike, mi, PARKED_VEHICLE);
//- rouz edit (ChatGPT)
			pBike->bIsStanding = true;
			pVehicle = pBike;
		}
		else {
//+ rouz edit (ChatGPT)
			// Allocate the parked automobile from the vehicle pool without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CAutomobile* pCar = (CAutomobile*)((CVehicle*)CPool_New(Pools_GetVehiclePool()));
			//- rouz edit (ChatGPT)
			assert(pCar);
			std::allocator<CAutomobile>().construct(pCar, mi, PARKED_VEHICLE);
//- rouz edit (ChatGPT)
			pVehicle = pCar;
		}
		// pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
		pVehicle->bLightsOn = false;
	}
	pVehicle->bIsStatic = false;
	pVehicle->bEngineOn = false;
	pos.z += pVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
	pVehicle->SetPosition(pos);
	pVehicle->SetOrientation(0.0f, 0.0f, DEGTORAD(m_fAngle));
	pVehicle->SetStatus(STATUS_ABANDONED);
	pVehicle->m_nDoorLock = CARLOCK_UNLOCKED;
	World_Add(pVehicle);
	if (General_GetRandomNumberInRangeInt(0, 100) < m_nAlarm)
		pVehicle->m_nAlarmState = -1;
	if (General_GetRandomNumberInRangeInt(0, 100) < m_nDoorlock)
		pVehicle->m_nDoorLock = CARLOCK_LOCKED;
	if (m_nColor1 != -1 && m_nColor2 != -1) {
		pVehicle->m_currentColour1 = m_nColor1;
		pVehicle->m_currentColour2 = m_nColor2;
	}
	else if (m_nModelIndex < -1) {
		m_nColor1 = pVehicle->m_currentColour1;
		m_nColor2 = pVehicle->m_currentColour2;
	}
	CVisibilityPlugins::SetClumpAlpha(pVehicle->GetClump(), 0);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	m_nVehicleHandle = CPool_GetIndex(Pools_GetVehiclePool(), pVehicle);
	//- rouz edit (ChatGPT)
	// Use up a spawn and arm the timer in C
	CarGen_AfterSpawn(this, &TheCarGenerators_CurrentActiveCount, (*Timer_GetTimeInMilliseconds()));
}

void CCarGenerator::Process()
{
	if (CarGen_IsDueToSpawn(this, TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter != 0, (*Timer_GetTimeInMilliseconds())) && CheckIfWithinRangeOfAnyPlayers())
		DoInternalProcessing();
	if (m_nVehicleHandle == -1)
		return;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), m_nVehicleHandle));
	//- rouz edit (ChatGPT)
	if (!pVehicle){
		m_nVehicleHandle = -1;
		return;
	}
	if (pVehicle->GetStatus() != STATUS_PLAYER)
		return;
	CarGen_ReleaseAfterPlayerTookOver(this);
	pVehicle->bExtendedRange = false;
}

void CCarGenerator::Setup(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay)
{
	// Delegate to the C implementation
	CarGen_Setup(this, x, y, z, angle, mi, color1, color2, force, alarm, lock, min_delay, max_delay, (*Timer_GetTimeInMilliseconds()));
}

bool CCarGenerator::CheckForBlockage(int32 mi)
{
	int16 entities;
	CEntity* pEntities[8];
	CColModel* pColModel = ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(mi))); // rouz edit (ChatGPT)
	World_FindObjectsKindaColliding(CVector(m_vecPos), pColModel->boundingSphere.radius, 1, &entities, 8, pEntities, false, true, true, false, false);
	for (int i = 0; i < entities; i++) {
		if (m_vecPos.z + pColModel->boundingBox.min.z < pEntities[i]->GetPosition().z + pEntities[i]->GetColModel()->boundingBox.max.z + 1.0f &&
			m_vecPos.z + pColModel->boundingBox.max.z > pEntities[i]->GetPosition().z + pEntities[i]->GetColModel()->boundingBox.min.z - 1.0f) {
			m_bIsBlocking = true;
			return true;
		}
	}
	return false;
}

static bool CarGenIsVisible(void *context)
{
	// The generator position is on screen and not occluded
	const CVector *pos = (const CVector*)context;
	return TheCamera.IsSphereVisible(*pos, 0.0f) && !Occlusion_IsPositionOccluded(*pos, 0.0f);
}

bool CCarGenerator::CheckIfWithinRangeOfAnyPlayers()
{
	// Delegate the decision to the C implementation
	CVector2D direction = FindPlayerCentreOfWorld(World_PlayerInFocus) - m_vecPos;
	CVector2D speed = CVector2D(FindPlayerSpeed());
	return CarGen_CheckRange(this, direction.x, direction.y, speed.x, speed.y, TheCamera.GenerationDistMultiplier,
		m_nModelIndex > 0 && ModelRegistry_IsBoatModel(m_nModelIndex), CarGenIsVisible, &m_vecPos, TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter != 0);
}

void TheCarGenerators_Process()
{
	if (FindPlayerTrain() || Cutscene_IsProcessing()) // rouz edit (ChatGPT)
		return;
	if (++TheCarGenerators_ProcessCounter == 4)
		TheCarGenerators_ProcessCounter = 0;
	for (uint32 i = TheCarGenerators_ProcessCounter; i < TheCarGenerators_NumOfCarGenerators; i += 4)
		TheCarGenerators_CarGeneratorArray[i].Process();
	if (TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter)
		TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter--;
}

int32 TheCarGenerators_CreateCarGenerator(float x, float y, float z, float angle, int32 mi, int16 color1, int16 color2, uint8 force, uint8 alarm, uint8 lock, uint16 min_delay, uint16 max_delay)
{
	if (TheCarGenerators_NumOfCarGenerators < NUM_CARGENS)
		TheCarGenerators_CarGeneratorArray[TheCarGenerators_NumOfCarGenerators++].Setup(x, y, z, angle, mi, color1, color2, force, alarm, lock, min_delay, max_delay);
	return TheCarGenerators_NumOfCarGenerators - 1;
}

void TheCarGenerators_Init()
{
	TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter = 0;
	TheCarGenerators_NumOfCarGenerators = 0;
	TheCarGenerators_ProcessCounter = 0;
	TheCarGenerators_CurrentActiveCount = 0;
}

void TheCarGenerators_SaveAllCarGenerators(uint8 *buffer, uint32 *size)
{
	const uint32 nGeneralDataSize = sizeof(TheCarGenerators_NumOfCarGenerators) + sizeof(TheCarGenerators_CurrentActiveCount) + sizeof(TheCarGenerators_ProcessCounter) + sizeof(TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter) + sizeof(int16);
	*size = sizeof(int) + nGeneralDataSize + sizeof(uint32) + sizeof(TheCarGenerators_CarGeneratorArray) + SAVE_HEADER_SIZE;
INITSAVEBUF
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveHeader(&buffer, 'C', 'G', 'N', '\0', *size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buffer, &nGeneralDataSize, sizeof(nGeneralDataSize));
	WriteSaveBuf(&buffer, &TheCarGenerators_NumOfCarGenerators, sizeof(TheCarGenerators_NumOfCarGenerators));
	WriteSaveBuf(&buffer, &TheCarGenerators_CurrentActiveCount, sizeof(TheCarGenerators_CurrentActiveCount));
	WriteSaveBuf(&buffer, &TheCarGenerators_ProcessCounter, sizeof(TheCarGenerators_ProcessCounter));
	WriteSaveBuf(&buffer, &TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter, sizeof(TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter));
	{
		// Materialize the saved value with its original serialized type
		int16 saveValue = (int16)0;
		WriteSaveBuf(&buffer, &saveValue, sizeof(saveValue)); // alignment
	}
	{
		// Materialize the saved value with its original serialized type
		uint32 saveValue = (uint32)sizeof(TheCarGenerators_CarGeneratorArray);
		WriteSaveBuf(&buffer, &saveValue, sizeof(saveValue));
	}
	//- rouz edit (ChatGPT)
	for (int i = 0; i < NUM_CARGENS; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		{
			// Preserve record assignment semantics and compiler padding behavior
			*(CCarGenerator*)buffer = TheCarGenerators_CarGeneratorArray[i];
			SkipSaveBuf(&buffer, sizeof(TheCarGenerators_CarGeneratorArray[i]));
		}
		//- rouz edit (ChatGPT)
VALIDATESAVEBUF(*size)
}

void TheCarGenerators_LoadAllCarGenerators(uint8* buffer, uint32 size)
{
	TheCarGenerators_NumOfCarGenerators = 0;
	TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter = 0;
	TheCarGenerators_CurrentActiveCount = 0;
	TheCarGenerators_ProcessCounter = 0;

	const int32 nGeneralDataSize = sizeof(TheCarGenerators_NumOfCarGenerators) + sizeof(TheCarGenerators_CurrentActiveCount) + sizeof(TheCarGenerators_ProcessCounter) + sizeof(TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter) + sizeof(int16);
	TheCarGenerators_Init();
INITSAVEBUF
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	CheckSaveHeader(&buffer, 'C', 'G', 'N', '\0', size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)
	uint32 tmp;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tmp, &buffer, sizeof(tmp));
	//- rouz edit (ChatGPT)
	assert(tmp == nGeneralDataSize);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&TheCarGenerators_NumOfCarGenerators, &buffer, sizeof(TheCarGenerators_NumOfCarGenerators));
	ReadSaveBuf(&TheCarGenerators_CurrentActiveCount, &buffer, sizeof(TheCarGenerators_CurrentActiveCount));
	ReadSaveBuf(&TheCarGenerators_ProcessCounter, &buffer, sizeof(TheCarGenerators_ProcessCounter));
	ReadSaveBuf(&TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter, &buffer, sizeof(TheCarGenerators_GenerateEvenIfPlayerIsCloseCounter));
	SkipSaveBuf(&buffer, 2);
	ReadSaveBuf(&tmp, &buffer, sizeof(tmp));
	//- rouz edit (ChatGPT)
	assert(tmp == sizeof(TheCarGenerators_CarGeneratorArray));
	for (int i = 0; i < NUM_CARGENS; i++) 
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		{
			// Preserve record assignment semantics and compiler padding behavior
			TheCarGenerators_CarGeneratorArray[i] = *(CCarGenerator*)buffer;
			SkipSaveBuf(&buffer, sizeof(TheCarGenerators_CarGeneratorArray[i]));
		}
		//- rouz edit (ChatGPT)
VALIDATESAVEBUF(size)
}

//- rouz edit (ChatGPT)
