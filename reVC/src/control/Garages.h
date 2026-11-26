#pragma once
#include "audio_enums.h"
#include "Camera.h"
#include "config.h"
#include "Lists.h"

class CVehicle;

//+ rouz edit (ChatGPT)
#include "GarageTypes.h"
//- rouz edit (ChatGPT)


enum
{
	TOTAL_COLLECTCARS_GARAGES = 4,
	TOTAL_HIDEOUT_GARAGES = 12,
	TOTAL_COLLECTCARS_CARS = 6
};

//+ rouz edit (ChatGPT)
#include "StoredCar.h"
//- rouz edit (ChatGPT)
#include "GarageData.h"
VALIDATE_SIZE(CStoredCar, 0x28);

#define SWITCH_GARAGE_DISTANCE_CLOSE 40.0f

class CGarage : public CGarageData
{
public:

	void OpenThisGarage();
	void CloseThisGarage();
	bool IsOpen() { return m_eGarageState == GS_OPENED || m_eGarageState == GS_OPENEDCONTAINSCAR; }
	bool IsClosed() { return m_eGarageState == GS_FULLYCLOSED; }
	bool IsUsed() { return m_eGarageType != GARAGE_NONE; }
	void Update();
	float GetGarageCenterX() { return (m_fInfX + m_fSupX) / 2; }
	float GetGarageCenterY() { return (m_fInfY + m_fSupY) / 2; }
	bool IsFar()
	{ 
#ifdef FIX_BUGS
		return Abs(TheCamera.GetPosition().x - GetGarageCenterX()) > SWITCH_GARAGE_DISTANCE_CLOSE ||
			Abs(TheCamera.GetPosition().y - GetGarageCenterY()) > SWITCH_GARAGE_DISTANCE_CLOSE;
#else
		return Abs(TheCamera.GetPosition().x - m_fInfX) > SWITCH_GARAGE_DISTANCE_CLOSE ||
			Abs(TheCamera.GetPosition().y - m_fInfY) > SWITCH_GARAGE_DISTANCE_CLOSE;
#endif
	}
	void TidyUpGarageClose();
	void TidyUpGarage();
	void RefreshDoorPointers(bool);
	void UpdateCrusherAngle();
	void UpdateDoorsHeight();
	bool IsEntityEntirelyInside3D(CEntity*, float);
	bool IsEntityEntirelyOutside(CEntity*, float);
	float CalcDistToGarageRectangleSquared(float, float);
	float CalcSmallestDistToGarageDoorSquared(float, float);
	bool IsAnyOtherCarTouchingGarage(CVehicle* pException);
	bool IsStaticPlayerCarEntirelyInside();
	bool IsPlayerOutsideGarage();
	bool IsAnyCarBlockingDoor();
	void CenterCarInGarage(CVehicle*);
	bool DoesCraigNeedThisCar(int32);
	bool MarkThisCarAsCollectedForCraig(int32);
	bool HasCraigCollectedThisCar(int32);
	bool IsGarageEmpty();
	void UpdateCrusherShake(float, float);
	int32 CountCarsWithCenterPointWithinGarage(CEntity* pException);
	void RemoveCarsBlockingDoorNotInside();
	void StoreAndRemoveCarsForThisHideout(CStoredCar*, int32);
	bool RestoreCarsForThisHideout(CStoredCar*);
	bool IsEntityTouching3D(CEntity*);
	bool EntityHasASphereWayOutsideGarage(CEntity*, float);
	bool IsAnyOtherPedTouchingGarage(CPed* pException);
	void BuildRotatedDoorMatrix(CEntity*, float);
	void FindDoorsEntities();
	void FindDoorsEntitiesSectorList(CPtrList&, bool);
	void PlayerArrestedOrDied();
	bool Does60SecondsNeedThisCarAtAll(int mi);
	bool Does60SecondsNeedThisCar(int mi);
	void MarkThisCarAsCollectedFor60Seconds(int mi);
	bool IsPlayerEntirelyInsideGarage();

	bool IsPointInsideGarage(CVector);
	bool IsPointInsideGarage(CVector, float);
	void ThrowCarsNearDoorOutOfGarage(CVehicle*);

	int32 FindMaxNumStoredCarsForGarage() { return Min(NUM_GARAGE_STORED_CARS, m_nMaxStoredCars); }

};

//+ rouz edit (Claude)
// Formerly the static class CGarages, now C globals and functions shared with C
enum {
		MESSAGE_LENGTH = 8,
	};
#ifdef __cplusplus
extern "C" {
#endif
extern int32 Garages_BankVansCollected;
extern bool Garages_BombsAreFree;
extern bool Garages_RespraysAreFree;
extern int32 Garages_CarsCollected;
extern int32 Garages_CarTypesCollected[TOTAL_COLLECTCARS_GARAGES];
extern int32 Garages_CrushedCarId;
extern uint32 Garages_LastTimeHelpMessage;
extern int32 Garages_MessageNumberInString;
extern char Garages_MessageIDString[MESSAGE_LENGTH];
extern int32 Garages_MessageNumberInString2;
extern uint32 Garages_MessageStartTime;
extern uint32 Garages_MessageEndTime;
extern uint32 Garages_NumGarages;
extern bool Garages_PlayerInGarage;
extern int32 Garages_PoliceCarsCollected;
extern bool Garages_bCamShouldBeOutisde;
void Garages_Init(void);
#ifndef PS2
void Garages_Shutdown(void);
#endif
void Garages_Update(void);
int16 Garages_AddOne(float X1, float Y1, float Z1, float X2, float Y2, float X3, float Y3, float Z2, uint8 type, int32 targetId);
void Garages_ChangeGarageType(int16, uint8, int32);
void Garages_PrintMessages(void);
void Garages_TriggerMessage(const char* text, int16, uint16 time, int16);
void Garages_SetTargetCarForMissonGarage(int16, CVehicle*);
bool Garages_HasCarBeenDroppedOffYet(int16);
void Garages_DeActivateGarage(int16);
void Garages_ActivateGarage(int16);
int32 Garages_QueryCarsCollected(int16);
bool Garages_HasImportExportGarageCollectedThisCar(int16, int8);
bool Garages_IsGarageOpen(int16);
bool Garages_IsGarageClosed(int16);
bool Garages_HasThisCarBeenCollected(int16, uint8);
bool Garages_HasResprayHappened(int16);
void Garages_SetGarageDoorToRotate(int16);
void Garages_SetLeaveCameraForThisGarage(int16);
bool Garages_IsThisCarWithinGarageArea(int16, CEntity*);
bool Garages_HasCarBeenCrushed(int32);
bool Garages_IsPointInAGarageCameraZone(CVector);
bool Garages_CameraShouldBeOutside(void);
void Garages_GivePlayerDetonator(void);
void Garages_PlayerArrestedOrDied(void);
void Garages_SetAllDoorsBackToOriginalHeight(void);
void Garages_Save(uint8* buf, uint32* size);
void Garages_Load(uint8* buf, uint32 size);
bool Garages_IsModelIndexADoor(uint32 id);
void Garages_StopCarFromBlowingUp(CAutomobile*);
bool Garages_IsCarSprayable(CVehicle*);
float Garages_FindDoorHeightForMI(int32);
void Garages_CloseHideOutGaragesBeforeSave(void);
int32 Garages_CountCarsInHideoutGarage(uint8);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CGarage Garages_aGarages[NUM_GARAGES];
extern CStoredCar Garages_aCarsInSafeHouses[TOTAL_HIDEOUT_GARAGES][NUM_GARAGE_STORED_CARS];
#ifndef PS2
#endif
static inline void Garages_OpenGarage(int16 garage);
static inline void Garages_CloseGarage(int16 garage);
bool Garages_IsPointWithinHideOutGarage(Const CVector&);
bool Garages_IsPointWithinAnyGarage(Const CVector&);
static inline void Garages_SetFreeBombs(bool bValue);
static inline void Garages_SetFreeResprays(bool bValue);
static inline void Garages_SetMaxNumStoredCarsForGarage(int16 garage, uint8 num);
static inline int32 Garages_GetBombTypeForGarageType(uint8 type);
static inline int32 Garages_GetCarsCollectedIndexForGarageType(uint8 type);
static inline int32 Garages_FindSafeHouseIndexForGarageType(uint8 type);
static inline bool Garages_IsThisGarageTypeSafehouse(uint8 type);
static inline void Garages_OpenGarage(int16 garage) { Garages_aGarages[garage].OpenThisGarage(); }
static inline void Garages_CloseGarage(int16 garage) { Garages_aGarages[garage].CloseThisGarage(); }
static inline void Garages_SetFreeBombs(bool bValue) { Garages_BombsAreFree = bValue; }
static inline void Garages_SetFreeResprays(bool bValue) { Garages_RespraysAreFree = bValue; }
static inline void Garages_SetMaxNumStoredCarsForGarage(int16 garage, uint8 num) { Garages_aGarages[garage].m_nMaxStoredCars = num; }
static inline int32 Garages_GetBombTypeForGarageType(uint8 type) { return type - GARAGE_BOMBSHOP1 + 1; }
static inline int32 Garages_GetCarsCollectedIndexForGarageType(uint8 type)
	{
		switch (type) {
		case GARAGE_COLLECTCARS_1: return 0;
		case GARAGE_COLLECTCARS_2: return 1;
		case GARAGE_COLLECTCARS_3: return 2;
		case GARAGE_COLLECTCARS_4: return 3;
		default: assert(0);
		}
		return 0;
	}
static inline int32 Garages_FindSafeHouseIndexForGarageType(uint8 type)
	{
		switch (type) {
		case GARAGE_HIDEOUT_ONE: return 0;
		case GARAGE_HIDEOUT_TWO: return 1;
		case GARAGE_HIDEOUT_THREE: return 2;
		case GARAGE_HIDEOUT_FOUR: return 3;
		case GARAGE_HIDEOUT_FIVE: return 4;
		case GARAGE_HIDEOUT_SIX: return 5;
		case GARAGE_HIDEOUT_SEVEN: return 6;
		case GARAGE_HIDEOUT_EIGHT: return 7;
		case GARAGE_HIDEOUT_NINE: return 8;
		case GARAGE_HIDEOUT_TEN: return 9;
		case GARAGE_HIDEOUT_ELEVEN: return 10;
		case GARAGE_HIDEOUT_TWELVE: return 11;
		}
		return -1;
	}
static inline bool Garages_IsThisGarageTypeSafehouse(uint8 type) { return Garages_FindSafeHouseIndexForGarageType(type) >= 0; }
#endif
//- rouz edit (Claude)
