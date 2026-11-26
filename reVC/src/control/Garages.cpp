//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"

#include "Garages.h"
#include "GarageLogic.h"
#include "StoredCar_backend.h"
#include "main.h"

#include "Bike.h"
#include "Boat.h"
#include "DMAudio.h"
#include "General.h"
#include "Font.h"
#include "HandlingMgr.h"
#include "Hud.h"
#include "Messages.h"
#include "ModelIndices.h"
#include "Pad.h"
#include "Particle.h"
#include "PlayerPed.h"
#include "Replay.h"
#include "Stats.h"
#include "Streaming.h"
#include "Text.h"
#include "Timer.h"
#include "Vehicle.h"
#include "Wanted.h"
#include "World.h"
#include "VarConsole.h"
#include "SaveBuf.h"
#include "Pools.h" // rouz edit (ChatGPT)
#include "VehicleLogic.h"
#include "WorldLogic.h"

#define ROTATED_DOOR_OPEN_SPEED (0.015f)
#define ROTATED_DOOR_CLOSE_SPEED (0.02f)
#define DEFAULT_DOOR_OPEN_SPEED (0.035f)
#define DEFAULT_DOOR_CLOSE_SPEED (0.04f)
#define CRUSHER_CRANE_SPEED (0.005f)

// Prices
#define BOMB_PRICE (500)
#define RESPRAY_PRICE (100)

// Distances
#define DISTANCE_TO_CALL_OFF_CHASE (10.0f)
#define DISTANCE_FOR_MRWHOOP_HACK (0.5f)
#define DISTANCE_TO_ACTIVATE_GARAGE (8.0f)
#define DISTANCE_TO_ACTIVATE_KEEPCAR_GARAGE (17.0f)
#define DISTANCE_TO_CLOSE_MISSION_GARAGE (30.0f)
#define DISTANCE_TO_CLOSE_COLLECTSPECIFICCARS_GARAGE (25.0f)
#define DISTANCE_TO_CLOSE_COLLECTCARS_GARAGE (40.0f)
#define DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_ON_FOOT (3.2f)
#define DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_IN_CAR (15.0f)
#define DISTANCE_TO_FORCE_CLOSE_HIDEOUT_GARAGE (70.0f)
#define DISTANCE_TO_OPEN_HIDEOUT_GARAGE_ON_FOOT (2.8f)
#define DISTANCE_TO_OPEN_HIDEOUT_GARAGE_IN_CAR (10.0f)
#define DISTANCE_TO_SHOW_HIDEOUT_MESSAGE (5.0f)

#define DISTANCE_TO_CONSIDER_DOOR_FOR_GARAGE (20.0f)

// Time
#define TIME_TO_RESPRAY (2000)
#define TIME_TO_SETUP_BOMB (2000)
#define TIME_TO_CRUSH_CAR (3000)
#define TIME_TO_PROCESS_KEEPCAR_GARAGE (2000)

// Respray stuff
#define FREE_RESPRAY_HEALTH_THRESHOLD (970.0f)
#define NUM_PARTICLES_IN_RESPRAY (200)
#define RESPRAY_CENTERING_COEFFICIENT (0.4f)

// Bomb stuff
#define KGS_OF_EXPLOSIVES_IN_BOMB (10)

// Collect specific cars stuff
#define REWARD_FOR_FIRST_POLICE_CAR (5000)
#define REWARD_FOR_FIRST_BANK_VAN (5000)
#define MAX_POLICE_CARS_TO_COLLECT (10)
#define MAX_BANK_VANS_TO_COLLECT (10)

// Collect cars stuff
#define MAX_SPEED_TO_SHOW_COLLECTED_MESSAGE (0.03f)
#define IMPORT_REWARD (500)
#define IMPORT_ALLCARS_REWARD (20500)

// Crusher stuff
#define CRUSHER_VEHICLE_TEST_SPAN (8)
#define CRUSHER_MIN_REWARD (25)
#define CRUSHER_MAX_REWARD (125)
#define CRUSHER_REWARD_COEFFICIENT (1.0f/500000)

// Hideout stuff
#define HIDEOUT_DOOR_SPEED_COEFFICIENT (1.7f)
#define TIME_BETWEEN_HIDEOUT_MESSAGES (18000)

// Camera stuff
#define MARGIN_FOR_CAMERA_COLLECTCARS (0.5f)
#define MARGIN_FOR_CAMERA_DEFAULT (0.5f)

const int32 gaCarsToCollectInCraigsGarages[TOTAL_COLLECTCARS_GARAGES][TOTAL_COLLECTCARS_CARS] =
{
	{ MI_LANDSTAL, MI_IDAHO,    MI_ESPERANT, MI_STALLION, MI_RANCHER, MI_BLISTAC  },
	{ MI_SABRE,    MI_VIRGO,    MI_SENTINEL, MI_STRETCH,  MI_WASHING, MI_ADMIRAL  },
	{ MI_CHEETAH,  MI_INFERNUS, MI_BANSHEE,  MI_PHEONIX,  MI_COMET,   MI_STINGER  },
	{ MI_VOODOO,   MI_CUBAN,    MI_CADDY,    MI_BAGGAGE,  MI_MRWHOOP, MI_PIZZABOY }
};

const int32 gaCarsToCollectIn60Seconds[] = { MI_CHEETAH, MI_TAXI, MI_ESPERANT, MI_SENTINEL, MI_IDAHO };

int32 Garages_BankVansCollected;
bool Garages_BombsAreFree;
bool Garages_RespraysAreFree;
int32 Garages_CarsCollected;
int32 Garages_CarTypesCollected[TOTAL_COLLECTCARS_GARAGES];
int32 Garages_CrushedCarId;
uint32 Garages_LastTimeHelpMessage;
int32 Garages_MessageNumberInString;
char Garages_MessageIDString[MESSAGE_LENGTH];
int32 Garages_MessageNumberInString2;
uint32 Garages_MessageStartTime;
uint32 Garages_MessageEndTime;
uint32 Garages_NumGarages;
bool Garages_PlayerInGarage;
int32 Garages_PoliceCarsCollected;
CStoredCar Garages_aCarsInSafeHouses[TOTAL_HIDEOUT_GARAGES][NUM_GARAGE_STORED_CARS];
int32 hGarages = AEHANDLE_NONE;
CGarage Garages_aGarages[NUM_GARAGES];
bool Garages_bCamShouldBeOutisde;

#ifndef MASTER
bool bPrintNearestObject;
#endif

void Garages_Init(void)
{
#ifndef MASTER
	VarConsole_AddBool(&VarConsole,"Print nearest object", &bPrintNearestObject, true); // rouz edit (ChatGPT)
#endif
	Garages_CrushedCarId = -1;
	Garages_NumGarages = 0;
	Garages_MessageEndTime = 0;
	Garages_MessageStartTime = 0;
	Garages_PlayerInGarage = false;
	Garages_BombsAreFree = false;
#ifdef FIX_BUGS
	Garages_RespraysAreFree = false;
#endif
	Garages_CarsCollected = 0;
	Garages_BankVansCollected = 0;
	Garages_PoliceCarsCollected = 0;
	for (int i = 0; i < TOTAL_COLLECTCARS_GARAGES; i++)
		Garages_CarTypesCollected[i] = 0;
	Garages_LastTimeHelpMessage = 0;
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		for (int j = 0; j < TOTAL_HIDEOUT_GARAGES; j++)
			StoredCar_Init(&Garages_aCarsInSafeHouses[j][i]);
	}
	hGarages = DMAudio_CreateEntity(AUDIOTYPE_GARAGE, (void*)1);
	if (hGarages >= 0)
		DMAudio_SetEntityStatus(hGarages, TRUE);
}

void Garages_Shutdown(void)
{
	Garages_NumGarages = 0;
	if (hGarages < 0)
		return;
	DMAudio_DestroyEntity(hGarages);
	hGarages = AEHANDLE_NONE;
}

void Garages_Update(void)
{
	static uint32 GarageToBeTidied = 0;
	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;
#ifdef SECUROM
	extern uint8 gameProcessPirateCheck;
	if (gameProcessPirateCheck == 2) return;
#endif
	Garages_bCamShouldBeOutisde = false;
	TheCamera.pToGarageWeAreIn = nil;
	TheCamera.pToGarageWeAreInForHackAvoidFirstPerson = nil;
#ifdef FIX_BUGS
	for (uint32 i = 0; i < Garages_NumGarages; i++) {
#else
	for (int i = 0; i < NUM_GARAGES; i++) {
#endif
		if (Garages_aGarages[i].IsUsed())
			Garages_aGarages[i].Update();
	}
	if (((*Timer_GetFrameCounter()) & 0xF) != 0xC)
		return;
#ifdef FIX_BUGS
	if (++GarageToBeTidied >= Garages_NumGarages)
#else
	if (++GarageToBeTidied >= NUM_GARAGES)
#endif
		GarageToBeTidied = 0;
	if (!Garages_aGarages[GarageToBeTidied].IsUsed())
		return;
	if (!Garages_aGarages[GarageToBeTidied].IsFar())
		Garages_aGarages[GarageToBeTidied].TidyUpGarageClose();
	else
		Garages_aGarages[GarageToBeTidied].TidyUpGarage();
}

int16 Garages_AddOne(float X1, float Y1, float Z1, float X2, float Y2, float X3, float Y3, float Z2, uint8 type, int32 targetId)
{
	if (Garages_NumGarages >= NUM_GARAGES) {
		assert(0);
		return Garages_NumGarages++;
	}
	CGarage* pGarage = &Garages_aGarages[Garages_NumGarages];
	// Delegate the bounds of the garage to the C implementation
	Garage_AxisBounds(X1, X2, X3, &pGarage->m_fInfX, &pGarage->m_fSupX);
	Garage_AxisBounds(Y1, Y2, Y3, &pGarage->m_fInfY, &pGarage->m_fSupY);
	pGarage->m_vecCorner1 = CVector(X1, Y1, Z1);
	pGarage->m_fInfZ = Z1;
	pGarage->m_vDir1 = CVector2D(X2 - X1, Y2 - Y1);
	pGarage->m_vDir2 = CVector2D(X3 - X1, Y3 - Y1);
	pGarage->m_fSupZ = Z2;
	pGarage->m_nMaxStoredCars = NUM_GARAGE_STORED_CARS;
	pGarage->m_fDir1Len = pGarage->m_vDir1.Magnitude();
	pGarage->m_fDir2Len = pGarage->m_vDir2.Magnitude();
	pGarage->m_vDir1 /= pGarage->m_fDir1Len;
	pGarage->m_vDir2 /= pGarage->m_fDir2Len;
	pGarage->m_pDoor1 = nil;
	pGarage->m_pDoor2 = nil;
	pGarage->m_fDoor1Z = Z1;
	pGarage->m_fDoor2Z = Z1;
	pGarage->m_eGarageType = type;
	pGarage->m_bRecreateDoorOnNextRefresh = false;
	pGarage->m_bRotatedDoor = false;
	pGarage->m_bCameraFollowsPlayer = false;
	pGarage->RefreshDoorPointers(true);
	if (pGarage->m_pDoor1) {
		pGarage->m_fDoor1Z = pGarage->m_pDoor1->GetPosition().z;
		pGarage->m_fDoor1X = pGarage->m_pDoor1->GetPosition().x;
		pGarage->m_fDoor1Y = pGarage->m_pDoor1->GetPosition().y;
	}
	if (pGarage->m_pDoor2) {
		pGarage->m_fDoor2Z = pGarage->m_pDoor2->GetPosition().z;
		pGarage->m_fDoor2X = pGarage->m_pDoor2->GetPosition().x;
		pGarage->m_fDoor2Y = pGarage->m_pDoor2->GetPosition().y;
	}
	pGarage->m_fDoorHeight = pGarage->m_pDoor1 ? Garages_FindDoorHeightForMI(pGarage->m_pDoor1->GetModelIndex()) : 4.0f;
	pGarage->m_fDoorPos = 0.0f;
	pGarage->m_eGarageState = GS_FULLYCLOSED;
	pGarage->m_nTimeToStartAction = 0;
	pGarage->field_2 = false;
	pGarage->m_nTargetModelIndex = targetId;
	pGarage->m_bCollectedCarsState = 0;
	pGarage->m_bDeactivated = false;
	pGarage->m_bResprayHappened = false;
	switch (type) {
	case GARAGE_MISSION:
	case GARAGE_COLLECTORSITEMS:
	case GARAGE_COLLECTSPECIFICCARS:
	case GARAGE_COLLECTCARS_1:
	case GARAGE_COLLECTCARS_2:
	case GARAGE_COLLECTCARS_3:
	case GARAGE_FORCARTOCOMEOUTOF:
	case GARAGE_60SECONDS:
	case GARAGE_MISSION_KEEPCAR:
	case GARAGE_FOR_SCRIPT_TO_OPEN:
	case GARAGE_HIDEOUT_ONE:
	case GARAGE_HIDEOUT_TWO:
	case GARAGE_HIDEOUT_THREE:
	case GARAGE_FOR_SCRIPT_TO_OPEN_AND_CLOSE:
	case GARAGE_KEEPS_OPENING_FOR_SPECIFIC_CAR:
	case GARAGE_MISSION_KEEPCAR_REMAINCLOSED:
	case GARAGE_COLLECTCARS_4:
	case GARAGE_FOR_SCRIPT_TO_OPEN_FOR_CAR:
	case GARAGE_HIDEOUT_FOUR:
	case GARAGE_HIDEOUT_FIVE:
	case GARAGE_HIDEOUT_SIX:
	case GARAGE_HIDEOUT_SEVEN:
	case GARAGE_HIDEOUT_EIGHT:
	case GARAGE_HIDEOUT_NINE:
	case GARAGE_HIDEOUT_TEN:
	case GARAGE_HIDEOUT_ELEVEN:
	case GARAGE_HIDEOUT_TWELVE:
		pGarage->m_eGarageState = GS_FULLYCLOSED;
		pGarage->m_fDoorPos = 0.0f;
		break;
	case GARAGE_BOMBSHOP1:
	case GARAGE_BOMBSHOP2:
	case GARAGE_BOMBSHOP3:
	case GARAGE_RESPRAY:
		pGarage->m_eGarageState = GS_OPENED;
		pGarage->m_fDoorPos = pGarage->m_fDoorHeight;
		break;
	case GARAGE_CRUSHER:
		pGarage->m_eGarageState = GS_OPENED;
		pGarage->m_fDoorPos = HALFPI;
		break;
	default:
		assert(false);
	}
	if (type == GARAGE_CRUSHER)
		pGarage->UpdateCrusherAngle();
	else
		pGarage->UpdateDoorsHeight();
	return Garages_NumGarages++;
}

void Garages_ChangeGarageType(int16 garage, uint8 type, int32 mi)
{
	CGarage* pGarage = &Garages_aGarages[garage];
	pGarage->m_eGarageType = type;
	pGarage->m_nTargetModelIndex = mi;
	pGarage->m_eGarageState = GS_FULLYCLOSED;
}

void CGarage::Update()
{
	if (m_eGarageType != GARAGE_CRUSHER) {
		switch (m_eGarageState) {
		case GS_FULLYCLOSED:
		case GS_OPENED:
		case GS_CLOSING:
		case GS_OPENING:
		case GS_OPENEDCONTAINSCAR:
		case GS_CLOSEDCONTAINSCAR:
			if (FindPlayerPed() && !m_bCameraFollowsPlayer) {
				CVehicle* pVehicle = FindPlayerVehicle();
				if (IsEntityEntirelyInside3D(FindPlayerPed(), 0.25f)) {
					TheCamera.pToGarageWeAreIn = this;
					Garages_bCamShouldBeOutisde = true;
				}
				if (pVehicle) {
					if (!IsEntityEntirelyOutside(pVehicle, 0.0f))
						TheCamera.pToGarageWeAreInForHackAvoidFirstPerson = this;
					if (pVehicle->GetModelIndex() == MI_MRWHOOP) {
						if (pVehicle->IsWithinArea(
							m_fInfX - DISTANCE_FOR_MRWHOOP_HACK,
							m_fInfY - DISTANCE_FOR_MRWHOOP_HACK,
							m_fSupX + DISTANCE_FOR_MRWHOOP_HACK,
							m_fSupY + DISTANCE_FOR_MRWHOOP_HACK)) {
							TheCamera.pToGarageWeAreIn = this;
							Garages_bCamShouldBeOutisde = true;
						}
					}
				}
			}
			break;
		default:
			break;
		}
	}
	if (m_bDeactivated && m_eGarageState == GS_FULLYCLOSED)
		return;
	if (m_bRotatedDoor) {
#ifdef GTA_PS2
		if (m_eGarageState == GS_OPENING) {
			if (m_pDoor1) {
				if (FindPlayerPed()->m_pCurrentPhysSurface == m_pDoor1)
					m_pDoor1->bUsesCollision = false;
			}
			if (m_pDoor2) {
				if (FindPlayerPed()->m_pCurrentPhysSurface == m_pDoor2)
					m_pDoor2->bUsesCollision = false;
			}
		}
		else if (m_eGarageState == GS_OPENED) {
			if (m_pDoor1)
				m_pDoor1->bUsesCollision = true;
			if (m_pDoor2)
				m_pDoor2->bUsesCollision = true;
		}
#else
		if (m_eGarageState == GS_OPENING || m_eGarageState == GS_OPENED) {
			if (m_pDoor1) {
				if (FindPlayerPed()->m_pCurrentPhysSurface == m_pDoor1 || FindPlayerPed()->GetPedState() == PED_JUMP || FindPlayerPed()->GetPedState() == PED_FALL || !FindPlayerPed()->bIsStanding)
					m_pDoor1->bUsesCollision = false;
			}
			if (m_pDoor2) {
				if (FindPlayerPed()->m_pCurrentPhysSurface == m_pDoor2 || FindPlayerPed()->GetPedState() == PED_JUMP || FindPlayerPed()->GetPedState() == PED_FALL || !FindPlayerPed()->bIsStanding)
					m_pDoor2->bUsesCollision = false;
			}
		}
		else {
			if (m_pDoor1)
				m_pDoor1->bUsesCollision = true;
			if (m_pDoor2)
				m_pDoor2->bUsesCollision = true;
		}
#endif
	}
	switch (m_eGarageType) {
	case GARAGE_RESPRAY:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (IsStaticPlayerCarEntirelyInside()) {
				if (Garages_IsCarSprayable(FindPlayerVehicle())) {
					if (World_Players[World_PlayerInFocus].m_nMoney >= RESPRAY_PRICE || Garages_RespraysAreFree) {
						m_eGarageState = GS_CLOSING;
						CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
						FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
					}
					else {
						Garages_TriggerMessage("GA_3", -1, 4000, -1); // No more freebies. $100 to respray!
						m_eGarageState = GS_OPENEDCONTAINSCAR;
						DMAudio_PlayFrontEndSound(SOUND_GARAGE_NO_MONEY, 1);
					}
				}
				else {
					Garages_TriggerMessage("GA_1", -1, 4000, -1); // Whoa! I don't touch nothing THAT hot!
					m_eGarageState = GS_OPENEDCONTAINSCAR;
					DMAudio_PlayFrontEndSound(SOUND_GARAGE_BAD_VEHICLE, 1);
				}
			}
			if (FindPlayerVehicle()) {
				if (CalcDistToGarageRectangleSquared(FindPlayerVehicle()->GetPosition().x, FindPlayerVehicle()->GetPosition().y) < SQR(DISTANCE_TO_ACTIVATE_GARAGE))
					World_CallOffChaseForArea(
						m_fInfX - DISTANCE_TO_CALL_OFF_CHASE,
						m_fInfY - DISTANCE_TO_CALL_OFF_CHASE,
						m_fSupX + DISTANCE_TO_CALL_OFF_CHASE,
						m_fSupY + DISTANCE_TO_CALL_OFF_CHASE);
			}
			break;
		case GS_CLOSING:
			if (FindPlayerVehicle())
				ThrowCarsNearDoorOutOfGarage(FindPlayerVehicle());
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				m_nTimeToStartAction = (*Timer_GetTimeInMilliseconds()) + TIME_TO_RESPRAY;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				Stats_CheckPointReachedSuccessfully();
			}
			UpdateDoorsHeight();
#ifdef FIX_BUGS
			if (FindPlayerVehicle() && FindPlayerVehicle()->IsCar())
#else
			if (FindPlayerVehicle())
#endif
				((CAutomobile*)(FindPlayerVehicle()))->m_fFireBlowUpTimer = 0.0f;
			World_CallOffChaseForArea(
				m_fInfX - DISTANCE_TO_CALL_OFF_CHASE,
				m_fInfY - DISTANCE_TO_CALL_OFF_CHASE,
				m_fSupX + DISTANCE_TO_CALL_OFF_CHASE,
				m_fSupY + DISTANCE_TO_CALL_OFF_CHASE);
			break;
		case GS_FULLYCLOSED:
			if ((*Timer_GetTimeInMilliseconds()) > m_nTimeToStartAction) {
				m_eGarageState = GS_OPENING;
				DMAudio_PlayFrontEndSound(SOUND_GARAGE_OPENING, 1);
				bool bTakeMoney = false;
				if (Wanted_GetWantedLevel(FindPlayerPed()->m_pWanted) != 0) {
					bTakeMoney = true;
					Wanted_Suspend(FindPlayerPed()->m_pWanted);
				}
				CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
#ifdef FIX_BUGS
				bool bChangedColour = false;
#else
				bool bChangedColour;
#endif
				if (FindPlayerVehicle() && (FindPlayerVehicle()->IsCar() || FindPlayerVehicle()->IsBike())) {
					if (FindPlayerVehicle()->m_fHealth < FREE_RESPRAY_HEALTH_THRESHOLD)
						bTakeMoney = true;
					FindPlayerVehicle()->m_fHealth = 1000.0f;
					if (FindPlayerVehicle()->IsCar()) {
						((CAutomobile*)(FindPlayerVehicle()))->m_fFireBlowUpTimer = 0.0f;
						((CAutomobile*)(FindPlayerVehicle()))->Fix();
					}
					else {
						((CBike*)(FindPlayerVehicle()))->m_fFireBlowUpTimer = 0.0f;
						((CBike*)(FindPlayerVehicle()))->Fix();
					}
					FindPlayerVehicle()->m_nDoorLock = CARLOCK_UNLOCKED;
					++Stats_Sprayings;
					if (FindPlayerVehicle()->GetUp().z < 0.0f) {
						FindPlayerVehicle()->GetUp() = -FindPlayerVehicle()->GetUp();
						FindPlayerVehicle()->GetRight() = -FindPlayerVehicle()->GetRight();
					}
					bChangedColour = false;
#ifdef FIX_BUGS
					if (!FindPlayerVehicle()->IsCar() || !((CAutomobile*)(FindPlayerVehicle()))->bFixedColour) {
#else
					if (!((CAutomobile*)(FindPlayerVehicle()))->bFixedColour) {
#endif
						uint8 colour1, colour2;
						uint16 attempt;
						FindPlayerVehicle()->GetModelInfo()->ChooseVehicleColour(colour1, colour2);
						for (attempt = 0; attempt < 10; attempt++) {
							if (colour1 != FindPlayerVehicle()->m_currentColour1 || colour2 != FindPlayerVehicle()->m_currentColour2)
								break;
							FindPlayerVehicle()->GetModelInfo()->ChooseVehicleColour(colour1, colour2);
						}
						bChangedColour = (attempt < 10);
						FindPlayerVehicle()->m_currentColour1 = colour1;
						FindPlayerVehicle()->m_currentColour2 = colour2;
						if (bChangedColour) {
							for (int i = 0; i < NUM_PARTICLES_IN_RESPRAY; i++) {
								CVector pos;
								pos.x = General_GetRandomNumberInRangeFloat(m_fInfX + 0.5f, m_fSupX - 0.5f);
								pos.y = General_GetRandomNumberInRangeFloat(m_fInfY + 0.5f, m_fSupY - 0.5f);
								pos.z = General_GetRandomNumberInRangeFloat(m_fDoor1Z - 3.0f, m_fDoor1Z + 1.0f);
								CParticle::AddParticle(PARTICLE_GARAGEPAINT_SPRAY, pos, CVector(0.0f, 0.0f, 0.0f), nil, 0.0f, CVehicleModelInfo::ms_vehicleColourTable[colour1]);
							}
						}
					}
					CenterCarInGarage(FindPlayerVehicle());
				}
				if (bTakeMoney) {
					if (!Garages_RespraysAreFree) {
						// Delegate the payment to the C implementation
						World_Players[World_PlayerInFocus].m_nMoney = Garage_MoneyAfterPayment(World_Players[World_PlayerInFocus].m_nMoney, RESPRAY_PRICE);
						Stats_AutoPaintingBudget += RESPRAY_PRICE;
					}
					Garages_TriggerMessage("GA_2", -1, 4000, -1); // New engine and paint job. The cops won't recognize you!
				}
				else if (bChangedColour) {
					if (General_GetRandomTrueFalse())
						Garages_TriggerMessage("GA_15", -1, 4000, -1); // Hope you like the new color.
					else
						Garages_TriggerMessage("GA_16", -1, 4000, -1); // Respray is complementary.
				}
				m_bResprayHappened = true;
			}
			World_CallOffChaseForArea(
				m_fInfX - DISTANCE_TO_CALL_OFF_CHASE,
				m_fInfY - DISTANCE_TO_CALL_OFF_CHASE,
				m_fSupX + DISTANCE_TO_CALL_OFF_CHASE,
				m_fSupY + DISTANCE_TO_CALL_OFF_CHASE);
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENEDCONTAINSCAR;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_OPENEDCONTAINSCAR:
			if (IsPlayerOutsideGarage())
				m_eGarageState = GS_OPENED;
			break;
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_BOMBSHOP1:
	case GARAGE_BOMBSHOP2:
	case GARAGE_BOMBSHOP3:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (IsStaticPlayerCarEntirelyInside()) {
				if (!FindPlayerVehicle() || FindPlayerVehicle()->m_bombType) {
					Garages_TriggerMessage("GA_5", -1, 4000, -1); //"Your car is already fitted with a bomb"
					m_eGarageState = GS_OPENEDCONTAINSCAR;
					DMAudio_PlayFrontEndSound(SOUND_GARAGE_BOMB_ALREADY_SET, 1);
					break;
				}
				if (!Garages_BombsAreFree && World_Players[World_PlayerInFocus].m_nMoney < BOMB_PRICE) {
					Garages_TriggerMessage("GA_4", -1, 4000, -1); // "Car bombs are $1000 each" - weird that the price is hardcoded in message
					m_eGarageState = GS_OPENEDCONTAINSCAR;
					DMAudio_PlayFrontEndSound(SOUND_GARAGE_NO_MONEY, 1);
					break;
				}
				m_eGarageState = GS_CLOSING;
				CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
				}
			break;
		case GS_CLOSING:
			if (FindPlayerVehicle())
				ThrowCarsNearDoorOutOfGarage(FindPlayerVehicle());
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				m_nTimeToStartAction = (*Timer_GetTimeInMilliseconds()) + TIME_TO_SETUP_BOMB;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			UpdateDoorsHeight();
			if (m_eGarageType == GARAGE_BOMBSHOP3)
				Streaming_RequestModel(MI_BOMB, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
			break;
		case GS_FULLYCLOSED:
			if ((*Timer_GetTimeInMilliseconds()) > m_nTimeToStartAction) {
				if (m_eGarageType != GARAGE_BOMBSHOP3 || Streaming_HasModelLoaded(MI_BOMB)) { // rouz edit (ChatGPT)
					switch (m_eGarageType) {
					case GARAGE_BOMBSHOP1: DMAudio_PlayFrontEndSound(SOUND_GARAGE_BOMB1_SET, 1); break;
					case GARAGE_BOMBSHOP2: DMAudio_PlayFrontEndSound(SOUND_GARAGE_BOMB2_SET, 1); break;
					case GARAGE_BOMBSHOP3: DMAudio_PlayFrontEndSound(SOUND_GARAGE_BOMB3_SET, 1); break;
					}
					m_eGarageState = GS_OPENING;
					if (!Garages_BombsAreFree)
						// Delegate the payment to the C implementation
						World_Players[World_PlayerInFocus].m_nMoney = Garage_MoneyAfterPayment(World_Players[World_PlayerInFocus].m_nMoney, BOMB_PRICE);
					if (FindPlayerVehicle() && (FindPlayerVehicle()->IsCar() || FindPlayerVehicle()->IsBike())) {
#if (!defined GTA_PS2 || defined FIX_BUGS)
						FindPlayerVehicle()->m_bombType = Garages_GetBombTypeForGarageType(m_eGarageType);
						FindPlayerVehicle()->m_pBombRigger = FindPlayerPed();
#else // PS2 version contained a bug: CBike was casted to CAutomobile, but due to coincidence it didn't corrupt memory
						((CAutomobile*)(FindPlayerVehicle()))->m_bombType = Garages_GetBombTypeForGarageType(m_eGarageType);
						((CAutomobile*)(FindPlayerVehicle()))->m_pBombRigger = FindPlayerPed();
#endif
						if (m_eGarageType == GARAGE_BOMBSHOP3)
							Garages_GivePlayerDetonator();
						Stats_KgsOfExplosivesUsed += KGS_OF_EXPLOSIVES_IN_BOMB;
					}
#ifdef DETECT_PAD_INPUT_SWITCH
					int16 Mode = Input_affectedByController ? CPad::GetPad(0)->Mode : 0; // rouz edit (ChatGPT)
#else
					int16 Mode = CPad::GetPad(0)->Mode;
#endif
					switch (m_eGarageType) {
					case GARAGE_BOMBSHOP1:
						switch (Mode) {
						case 0:
						case 1:
						case 2:
							Hud_SetHelpMessage(Text_Get(&TheText,"GA_6"), false); // Arm with ~h~~k~~PED_FIREWEAPON~ button~w~. Bomb will go off when engine is started.
							break;
						case 3:
							Hud_SetHelpMessage(Text_Get(&TheText,"GA_6B"), false); // Arm with ~h~~k~~PED_FIREWEAPON~ button~w~. Bomb will go off when engine is started.
							break;
						}
						break;
					case GARAGE_BOMBSHOP2:
						switch (Mode) {
						case 0:
						case 1:
						case 2:
							Hud_SetHelpMessage(Text_Get(&TheText,"GA_7"), false); // Park it, prime it by pressing the ~h~~k~~PED_FIREWEAPON~ button~w~ and LEG IT!
							break;
						case 3:
							Hud_SetHelpMessage(Text_Get(&TheText,"GA_7B"), false); // Park it, prime it by pressing the ~h~~k~~PED_FIREWEAPON~ button~w~ and LEG IT!
							break;
						}
						break;
					case GARAGE_BOMBSHOP3:
						Hud_SetHelpMessage(Text_Get(&TheText,"GA_8"), false); // Use the detonator to activate the bomb.
						break;
					}
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
				}
				else {
					Streaming_RequestModel(MI_BOMB, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
				}
			}
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENEDCONTAINSCAR;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_OPENEDCONTAINSCAR:
			if (IsPlayerOutsideGarage())
				m_eGarageState = GS_OPENED;
			break;
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
			}
		break;
	case GARAGE_MISSION:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (((CVector2D)FindPlayerCoors() - CVector2D(GetGarageCenterX(), GetGarageCenterY())).MagnitudeSqr() > SQR(DISTANCE_TO_CLOSE_MISSION_GARAGE)) {
				if (((*Timer_GetFrameCounter()) & 0x1F) == 0 
#ifndef GTA_PS2
					&& (!m_pTarget || IsEntityTouching3D(m_pTarget))
#endif
					) {
					m_eGarageState = GS_CLOSING;
					m_bClosingWithoutTargetCar = true;
				}
			}
			else if (!FindPlayerVehicle() && m_pTarget && IsEntityEntirelyInside3D(m_pTarget, 0.0f) &&
				IsEntityEntirelyOutside(FindPlayerVehicle() ? (CEntity*)FindPlayerVehicle() : (CEntity*)FindPlayerPed(), 2.0f)) {
				CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
				m_eGarageState = GS_CLOSING;
				m_bClosingWithoutTargetCar = false;
			}
			break;
		case GS_CLOSING:
			if (m_pTarget)
				ThrowCarsNearDoorOutOfGarage(m_pTarget);
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				if (m_bClosingWithoutTargetCar)
					m_eGarageState = GS_FULLYCLOSED;
				else {
					if (m_pTarget) {
						m_eGarageState = GS_CLOSEDCONTAINSCAR;
						DestroyVehicleAndDriverAndPassengers(m_pTarget);
						m_pTarget = nil;
					}
					else {
						m_eGarageState = GS_FULLYCLOSED;
					}
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() == m_pTarget && m_pTarget) {
				if (CalcDistToGarageRectangleSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y) < SQR(DISTANCE_TO_ACTIVATE_GARAGE))
					m_eGarageState = GS_OPENING;
			}
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_COLLECTSPECIFICCARS:
	case GARAGE_COLLECTCARS_1:
	case GARAGE_COLLECTCARS_2:
	case GARAGE_COLLECTCARS_3:
	case GARAGE_COLLECTCARS_4:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (FindPlayerVehicle() && DoesCraigNeedThisCar(FindPlayerVehicle()->GetModelIndex())) {
				m_pTarget = FindPlayerVehicle();
				m_pTarget->RegisterReference((CEntity**)&m_pTarget);
			}
			// Delegate the check that the player is far to the C implementation
			if (Garage_PlayerIsFar(FindPlayerCoors().x, FindPlayerCoors().y, GetGarageCenterX(), GetGarageCenterY(), DISTANCE_TO_CLOSE_COLLECTCARS_GARAGE)) {
				m_eGarageState = GS_CLOSING;
				m_pTarget = nil;
				break;
			}
			if (m_pTarget && !FindPlayerVehicle() && IsEntityEntirelyInside3D(m_pTarget, 0.0f) &&
				!IsAnyOtherCarTouchingGarage(m_pTarget) && IsEntityEntirelyOutside(FindPlayerPed(), 2.0f)) {
#ifdef FIX_BUGS
				if (!m_pTarget->IsCar() ||
					Damage_GetEngineStatus(&((CAutomobile*)(m_pTarget))->Damage) <= ENGINE_STATUS_ON_FIRE &&
					((CAutomobile*)(m_pTarget))->m_fFireBlowUpTimer == 0.0f) {
#else
				if (Damage_GetEngineStatus(&((CAutomobile*)(m_pTarget))->Damage) <= ENGINE_STATUS_ON_FIRE &&
					((CAutomobile*)(m_pTarget))->m_fFireBlowUpTimer == 0.0f) {
#endif
					if (m_pTarget->GetStatus() != STATUS_WRECKED) {
						CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
						FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
						m_eGarageState = GS_CLOSING;
						TheCamera.SetCameraDirectlyBehindForFollowPed_CamOnAString();
					}
				}
			}
			break;
		case GS_CLOSING:
			if (m_pTarget)
				ThrowCarsNearDoorOutOfGarage(m_pTarget);
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				if (m_pTarget) {
					MarkThisCarAsCollectedForCraig(m_pTarget->GetModelIndex());
					DestroyVehicleAndDriverAndPassengers(m_pTarget);
					m_pTarget = nil;
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() &&
				CalcSmallestDistToGarageDoorSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y
					) < SQR(DISTANCE_TO_ACTIVATE_GARAGE)) {
				if (DoesCraigNeedThisCar(FindPlayerVehicle()->GetModelIndex())) {
					if (FindPlayerVehicle()->VehicleCreatedBy == MISSION_VEHICLE)
						Garages_TriggerMessage("GA_1A", -1, 5000, -1); // Come back when you're not so busy...
					else
						m_eGarageState = GS_OPENING;
				}
				else {
					if (HasCraigCollectedThisCar(FindPlayerVehicle()->GetModelIndex()))
						Garages_TriggerMessage("GA_20", -1, 5000, -1); // We got more of these than we can shift. Sorry man, no deal.
					else if (FindPlayerSpeed().Magnitude() < MAX_SPEED_TO_SHOW_COLLECTED_MESSAGE)
						Garages_TriggerMessage("GA_19", -1, 5000, -1); // We're not interested in that model.
				}
			}
			m_pTarget = nil;
			break;
		case GS_OPENING:
			if (FindPlayerVehicle() && DoesCraigNeedThisCar(FindPlayerVehicle()->GetModelIndex())) {
				m_pTarget = FindPlayerVehicle();
				m_pTarget->RegisterReference((CEntity**)&m_pTarget);
			}
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_FORCARTOCOMEOUTOF:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (IsGarageEmpty())
				m_eGarageState = GS_CLOSING;
			break;
		case GS_CLOSING:
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			if (!IsGarageEmpty())
				m_eGarageState = GS_OPENING;
			break;
		case GS_FULLYCLOSED:
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_CRUSHER:
		break;
	case GARAGE_MISSION_KEEPCAR:
	case GARAGE_MISSION_KEEPCAR_REMAINCLOSED:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (((CVector2D)FindPlayerCoors() - CVector2D(GetGarageCenterX(), GetGarageCenterY())).MagnitudeSqr() > SQR(DISTANCE_TO_CLOSE_MISSION_GARAGE) &&
				!IsAnyOtherCarTouchingGarage(nil)) {
				m_eGarageState = GS_CLOSING;
				m_bClosingWithoutTargetCar = true;
			}
			else if (m_pTarget && m_pTarget == FindPlayerVehicle() && IsStaticPlayerCarEntirelyInside() && !IsAnyCarBlockingDoor()) {
				CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
				FindPlayerPed()->m_pWanted->m_bIgnoredByCops = true;
				m_eGarageState = GS_CLOSING;
				m_bClosingWithoutTargetCar = false;
			}
			break;
		case GS_CLOSING:
			if (m_pTarget)
				ThrowCarsNearDoorOutOfGarage(m_pTarget);
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				if (m_bClosingWithoutTargetCar)
					m_eGarageState = GS_FULLYCLOSED;
				else {
					if (m_pTarget) {
						m_eGarageState = GS_CLOSEDCONTAINSCAR;
						m_nTimeToStartAction = (*Timer_GetTimeInMilliseconds()) + TIME_TO_PROCESS_KEEPCAR_GARAGE;
						m_pTarget = nil;
					}
					else
						m_eGarageState = GS_FULLYCLOSED;
					CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
					FindPlayerPed()->m_pWanted->m_bIgnoredByCops = false;
				}
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() == m_pTarget && m_pTarget &&
				CalcDistToGarageRectangleSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y
					) < SQR(DISTANCE_TO_ACTIVATE_KEEPCAR_GARAGE))
				m_eGarageState = GS_OPENING;
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_CLOSEDCONTAINSCAR:
			if (m_eGarageType == GARAGE_MISSION_KEEPCAR && (*Timer_GetTimeInMilliseconds()) > m_nTimeToStartAction)
				m_eGarageState = GS_OPENING;
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_FOR_SCRIPT_TO_OPEN:
		switch (m_eGarageState) {
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENED:
		//case GS_CLOSING:
		//case GS_FULLYCLOSED:
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_FOR_SCRIPT_TO_OPEN_AND_CLOSE:
		switch (m_eGarageState) {
		case GS_CLOSING:
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENED:
		//case GS_FULLYCLOSED:
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_HIDEOUT_ONE:
	case GARAGE_HIDEOUT_TWO:
	case GARAGE_HIDEOUT_THREE:
	case GARAGE_HIDEOUT_FOUR:
	case GARAGE_HIDEOUT_FIVE:
	case GARAGE_HIDEOUT_SIX:
	case GARAGE_HIDEOUT_SEVEN:
	case GARAGE_HIDEOUT_EIGHT:
	case GARAGE_HIDEOUT_NINE:
	case GARAGE_HIDEOUT_TEN:
	case GARAGE_HIDEOUT_ELEVEN:
	case GARAGE_HIDEOUT_TWELVE:
		switch (m_eGarageState) {
		case GS_OPENED:
		{
			float distance = CalcDistToGarageRectangleSquared(FindPlayerCoors().x, FindPlayerCoors().y);
			// Close car doors either if player is far, or if he is in vehicle and garage is full,
			// or if player is very very far so that we can remove whatever is blocking garage door without him noticing
			if ((distance > SQR(DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_IN_CAR) ||
				!FindPlayerVehicle() && distance > SQR(DISTANCE_TO_CLOSE_HIDEOUT_GARAGE_ON_FOOT)) &&
				!IsAnyCarBlockingDoor())
				m_eGarageState = GS_CLOSING;
			else if (FindPlayerVehicle() &&
				CountCarsWithCenterPointWithinGarage(FindPlayerVehicle()) >=
				FindMaxNumStoredCarsForGarage()) {
				m_eGarageState = GS_CLOSING;
			}
			else if (distance > SQR(DISTANCE_TO_FORCE_CLOSE_HIDEOUT_GARAGE)) {
				m_eGarageState = GS_CLOSING;
				RemoveCarsBlockingDoorNotInside();
			}
			break;
		}
		case GS_CLOSING:
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, HIDEOUT_DOOR_SPEED_COEFFICIENT * (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (!IsPlayerOutsideGarage())
				m_eGarageState = GS_OPENING;
			else if (m_fDoorPos == 0.0f) {
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				m_eGarageState = GS_FULLYCLOSED;
				StoreAndRemoveCarsForThisHideout(Garages_aCarsInSafeHouses[Garages_FindSafeHouseIndexForGarageType(m_eGarageType)], NUM_GARAGE_STORED_CARS);
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
		{
			float distance = CalcDistToGarageRectangleSquared(FindPlayerCoors().x, FindPlayerCoors().y);
			if (distance < SQR(DISTANCE_TO_OPEN_HIDEOUT_GARAGE_ON_FOOT) ||
				distance < SQR(DISTANCE_TO_OPEN_HIDEOUT_GARAGE_IN_CAR) && FindPlayerVehicle()) {
				if (FindPlayerVehicle() && Garages_CountCarsInHideoutGarage(m_eGarageType) >= FindMaxNumStoredCarsForGarage()) {
					if (m_pDoor1) {
						if (((CVector2D)FindPlayerVehicle()->GetPosition() - (CVector2D)m_pDoor1->GetPosition()).MagnitudeSqr() < SQR(DISTANCE_TO_SHOW_HIDEOUT_MESSAGE) &&
							(*Timer_GetTimeInMilliseconds()) - Garages_LastTimeHelpMessage > TIME_BETWEEN_HIDEOUT_MESSAGES) {
							if (FindPlayerVehicle()->GetVehicleAppearance() != VEHICLE_APPEARANCE_HELI && FindPlayerVehicle()->GetVehicleAppearance() != VEHICLE_APPEARANCE_PLANE) {
								Hud_SetHelpMessage(Text_Get(&TheText,"GA_21"), false); // You cannot store any more cars in this garage.
								Garages_LastTimeHelpMessage = (*Timer_GetTimeInMilliseconds());
							}
						}
					}
				}
				else if (RestoreCarsForThisHideout(Garages_aCarsInSafeHouses[Garages_FindSafeHouseIndexForGarageType(m_eGarageType)]))
					m_eGarageState = GS_OPENING;
			}
			break;
		}
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, HIDEOUT_DOOR_SPEED_COEFFICIENT * (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	case GARAGE_KEEPS_OPENING_FOR_SPECIFIC_CAR:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (((CVector2D)FindPlayerCoors() - CVector2D(GetGarageCenterX(), GetGarageCenterY())).MagnitudeSqr() > SQR(DISTANCE_TO_CLOSE_MISSION_GARAGE)) {
				if (m_pTarget && IsEntityEntirelyOutside(m_pTarget, 0.0f) && !IsAnyOtherCarTouchingGarage(nil)) {
					m_eGarageState = GS_CLOSING;
					m_bClosingWithoutTargetCar = true;
				}
			}
			break;
		case GS_CLOSING:
			if (m_pTarget)
				ThrowCarsNearDoorOutOfGarage(m_pTarget);
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			if (FindPlayerVehicle() == m_pTarget && m_pTarget &&
				CalcDistToGarageRectangleSquared(
					FindPlayerVehicle()->GetPosition().x,
					FindPlayerVehicle()->GetPosition().y
					) < SQR(DISTANCE_TO_ACTIVATE_GARAGE))
				m_eGarageState = GS_OPENING;
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
		//case GS_OPENEDCONTAINSCAR:
		//case GS_CLOSEDCONTAINSCAR:
		//case GS_AFTERDROPOFF:
		default:
			break;
		}
		break;
	//case GARAGE_COLLECTORSITEMS:
	//case GARAGE_60SECONDS:
	case GARAGE_FOR_SCRIPT_TO_OPEN_FOR_CAR:
		switch (m_eGarageState) {
		case GS_OPENED:
			if (m_pTarget && IsEntityEntirelyInside3D(m_pTarget, 0.0f) && !IsAnyCarBlockingDoor() && IsPlayerOutsideGarage()) {
				CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_GARAGE);
				m_eGarageState = GS_CLOSING;
				m_bClosingWithoutTargetCar = false;
			}
			break;
		case GS_CLOSING:
			// Delegate the closing of the door to the C implementation
			m_fDoorPos = Garage_DoorClosedPos(m_fDoorPos, (m_bRotatedDoor ? ROTATED_DOOR_CLOSE_SPEED : DEFAULT_DOOR_CLOSE_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == 0.0f) {
				m_eGarageState = GS_FULLYCLOSED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_CLOSED, 1.0f);
				CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_GARAGE);
			}
			UpdateDoorsHeight();
			break;
		case GS_FULLYCLOSED:
			break;
		case GS_OPENING:
			// Delegate the opening of the door to the C implementation
			m_fDoorPos = Garage_DoorOpenedPos(m_fDoorPos, m_fDoorHeight, (m_bRotatedDoor ? ROTATED_DOOR_OPEN_SPEED : DEFAULT_DOOR_OPEN_SPEED), (*Timer_GetTimeStep()));
			if (m_fDoorPos == m_fDoorHeight) {
				m_eGarageState = GS_OPENED;
				DMAudio_PlayOneShot(hGarages, SOUND_GARAGE_DOOR_OPENED, 1.0f);
			}
			UpdateDoorsHeight();
			break;
			//case GS_OPENEDCONTAINSCAR:
			//case GS_CLOSEDCONTAINSCAR:
			//case GS_AFTERDROPOFF:
		default:
			break;
		}
	default:
		break;
	}
}

bool CGarage::IsStaticPlayerCarEntirelyInside()
{
	if (!FindPlayerVehicle())
		return false;
	if (!FindPlayerVehicle()->IsCar() && !FindPlayerVehicle()->IsBike())
		return false;
	if (FindPlayerPed()->GetPedState() != PED_DRIVING)
		return false;
	if (FindPlayerPed()->m_objective == OBJECTIVE_LEAVE_CAR)
		return false;
	CVehicle* pVehicle = FindPlayerVehicle();
	if (pVehicle->GetPosition().x < m_fInfX || pVehicle->GetPosition().x > m_fSupX ||
		pVehicle->GetPosition().y < m_fInfY || pVehicle->GetPosition().y > m_fSupY)
		return false;
	// Delegate the check that the vehicle moves to the C implementation
	if (VehicleLogic_AnySpeedAbove(pVehicle->GetSpeed().x, pVehicle->GetSpeed().y, pVehicle->GetSpeed().z, 0.01f))
		return false;
	if (pVehicle->GetSpeed().MagnitudeSqr() > SQR(0.01f))
		return false;
	return IsEntityEntirelyInside3D(pVehicle, 0.0f);
}

bool CGarage::IsPointInsideGarage(CVector pos)
{
	// Delegate to the C implementation
	return Garage_IsPointInside(this, pos.x, pos.y, pos.z);
}

bool CGarage::IsPointInsideGarage(CVector pos, float m_fMargin)
{
	// Delegate to the C implementation
	return Garage_IsPointInsideWithMargin(this, pos.x, pos.y, pos.z, m_fMargin);
}

bool CGarage::IsEntityEntirelyInside3D(CEntity* pEntity, float fMargin)
{
	if (pEntity->GetPosition().x < m_fInfX - fMargin || pEntity->GetPosition().x > m_fSupX + fMargin ||
		pEntity->GetPosition().y < m_fInfY - fMargin || pEntity->GetPosition().y > m_fSupY + fMargin ||
		pEntity->GetPosition().z < m_fInfZ - fMargin || pEntity->GetPosition().z > m_fSupZ + fMargin)
		return false;
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
		float radius = pColModel->spheres[i].bounds.radius;
		if (!IsPointInsideGarage(pos, fMargin - radius))
			return false;
	}
	return true;
}

bool CGarage::IsEntityEntirelyOutside(CEntity * pEntity, float fMargin)
{
	if (pEntity->GetPosition().x > m_fInfX - fMargin && pEntity->GetPosition().x < m_fSupX + fMargin &&
		pEntity->GetPosition().y > m_fInfY - fMargin && pEntity->GetPosition().y < m_fSupY + fMargin)
		return false;
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
		float radius = pColModel->spheres[i].bounds.radius;
		if (IsPointInsideGarage(pos, fMargin + radius))
			return false;
	}
	return true;
}

bool CGarage::IsGarageEmpty()
{
	int16 num;
	CEntity* pEntities[16];
	World_FindObjectsIntersectingCube(CVector(m_fInfX, m_fInfY, m_fInfZ), CVector(m_fSupX, m_fSupY, m_fSupZ), &num, 16, pEntities, false, true, true, false, false);
	if (num <= 0)
		return true;
	for (int i = 0; i < 16; i++) {
		if (IsEntityTouching3D(pEntities[i]))
			return false;
	}
	return true;
}

bool CGarage::IsPlayerOutsideGarage()
{
	if (FindPlayerVehicle())
		return IsEntityEntirelyOutside(FindPlayerVehicle(), 0.0f);
	return IsEntityEntirelyOutside(FindPlayerPed(), 0.0f);
}

bool CGarage::IsEntityTouching3D(CEntity* pEntity)
{
	float radius = pEntity->GetBoundRadius();
	if (m_fInfX - radius > pEntity->GetPosition().x || m_fSupX + radius < pEntity->GetPosition().x ||
		m_fInfY - radius > pEntity->GetPosition().y || m_fSupY + radius < pEntity->GetPosition().y ||
		m_fInfZ - radius > pEntity->GetPosition().z || m_fSupZ + radius < pEntity->GetPosition().z)
		return false;
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
		radius = pColModel->spheres[i].bounds.radius;
		if (IsPointInsideGarage(pos, radius))
			return true;
	}
	return false;
}

bool CGarage::EntityHasASphereWayOutsideGarage(CEntity * pEntity, float fMargin)
{
	CColModel* pColModel = pEntity->GetColModel();
	for (int i = 0; i < pColModel->numSpheres; i++) {
		CVector pos = pEntity->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
		float radius = pColModel->spheres[i].bounds.radius;
		if (!IsPointInsideGarage(pos, fMargin + radius))
			return true;
	}
	return false;
}

bool CGarage::IsAnyOtherCarTouchingGarage(CVehicle * pException)
{
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetVehiclePool());
		while (i--) {
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (!pVehicle || pVehicle == pException || pVehicle->GetStatus() == STATUS_WRECKED)
			continue;
		if (!IsEntityTouching3D(pVehicle))
			continue;
		CColModel* pColModel = pVehicle->GetColModel();
		for (int i = 0; i < pColModel->numSpheres; i++) {
			CVector pos = pVehicle->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
			float radius = pColModel->spheres[i].bounds.radius;
			if (IsPointInsideGarage(pos, radius))
				return true;
		}
	}
	return false;
}

void CGarage::ThrowCarsNearDoorOutOfGarage(CVehicle* pException)
{
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetVehiclePool());
		while (i--) {
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (!pVehicle || pVehicle == pException)
			continue;
		if (!IsEntityTouching3D(pVehicle))
			continue;
		CColModel* pColModel = pVehicle->GetColModel();
		for (int i = 0; i < pColModel->numSpheres; i++) {
			CVector pos = pVehicle->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
			float radius = pColModel->spheres[i].bounds.radius;
			if (!IsPointInsideGarage(pos, 0.0f)) {
				CVector vecDirectionAway(pVehicle->GetPosition().x - GetGarageCenterX(), pVehicle->GetPosition().y - GetGarageCenterY(), 0.0f);
				vecDirectionAway.Normalise();
				pVehicle->AddToMoveSpeed(vecDirectionAway * Timer_GetTimeStepInSeconds());
			}
		}
	}
}

bool CGarage::IsAnyOtherPedTouchingGarage(CPed * pException)
{
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetPedPool());
		while (i--) {
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetSlot(Pools_GetPedPool(), i));
				if (!pPed || pPed == pException)
			continue;
		if (!IsEntityTouching3D(pPed))
			continue;
		CColModel* pColModel = pException->GetColModel();
		for (int i = 0; i < pColModel->numSpheres; i++) {
			CVector pos = pPed->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
			float radius = pColModel->spheres[i].bounds.radius;
			if (IsPointInsideGarage(pos, radius))
				return true;
		}
	}
	return false;
}

bool CGarage::IsAnyCarBlockingDoor()
{
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetVehiclePool());
		while (i--) {
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (!pVehicle)
			continue;
		if (!IsEntityTouching3D(pVehicle))
			continue;
		CColModel* pColModel = pVehicle->GetColModel();
		for (int i = 0; i < pColModel->numSpheres; i++) {
			CVector pos = pVehicle->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
			float radius = pColModel->spheres[i].bounds.radius;
			if (!IsPointInsideGarage(pos, radius))
				return true;
		}
	}
	return false;
}

int32 CGarage::CountCarsWithCenterPointWithinGarage(CEntity * pException)
{
	int32 total = 0;
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetVehiclePool());
		while (i--) {
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (!pVehicle || pVehicle == pException)
			continue;
		if (IsPointInsideGarage(pVehicle->GetPosition()))
			total++;
	}
	return total;
}

void CGarage::RemoveCarsBlockingDoorNotInside()
{
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetVehiclePool());
		while (i--) {
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (!pVehicle)
			continue;
		if (!IsEntityTouching3D(pVehicle))
			continue;
		if (!IsPointInsideGarage(pVehicle->GetPosition())) {
			if (!pVehicle->bIsLocked && pVehicle->CanBeDeleted()) {
				World_Remove(pVehicle);
				// Destroy and release the vehicle without invoking C++ delete.
				pVehicle->~CVehicle();
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetVehiclePool(), pVehicle);
				#ifndef FIX_BUGS
				return;
#endif
			}
		}
	}
}

void Garages_PrintMessages()
{
	if ((*Timer_GetTimeInMilliseconds()) > Garages_MessageStartTime && (*Timer_GetTimeInMilliseconds()) < Garages_MessageEndTime) {
		Font_DrawFonts();
		Font_SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.5f));
		Font_SetPropOn();
		Font_SetJustifyOff();
		Font_SetBackgroundOff();
#ifdef FIX_BUGS
		Font_SetCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 50));
#else
		Font_SetCentreSize(SCREEN_WIDTH - 50);
#endif
		Font_SetCentreOn();
		Font_SetFontStyle(FONT_LOCALE(FONT_STANDARD));
		Font_SetColor(CRGBA(27, 89, 130, 255));
		Font_SetDropShadowPosition(2);
		Font_SetDropColor(CRGBA(0, 0, 0, 255));

		float y_offset = SCREEN_SCALE_Y(140.0f);

		if (Garages_MessageNumberInString2 >= 0) {
			Messages_InsertNumberInString(Text_Get(&TheText,Garages_MessageIDString), Garages_MessageNumberInString, Garages_MessageNumberInString2, -1, -1, -1, -1, gUString);
			Font_PrintString(SCREEN_WIDTH / 2, y_offset - SCREEN_SCALE_Y(30.0f), gUString);
		}
		else if (Garages_MessageNumberInString >= 0) {
			Messages_InsertNumberInString(Text_Get(&TheText,Garages_MessageIDString), Garages_MessageNumberInString, -1, -1, -1, -1, -1, gUString);
			Font_PrintString(SCREEN_WIDTH / 2, y_offset - SCREEN_SCALE_Y(30.0f), gUString);
		}
		else {
			Font_PrintString(SCREEN_WIDTH / 2, y_offset, Text_Get(&TheText,Garages_MessageIDString));
		}
	}
}

bool Garages_IsCarSprayable(CVehicle * pVehicle)
{
	switch (pVehicle->GetModelIndex()) {
	case MI_FIRETRUCK:
	case MI_AMBULAN:
	case MI_POLICE:
	case MI_ENFORCER:
	case MI_BUS:
	case MI_RHINO:
	case MI_BARRACKS:
	case MI_DODO:
	case MI_COACH:
#ifndef GTA_PS2
	case MI_FBIRANCH:
#endif
		return false;
	default:
		break;
	}
	return true;
}

void CGarage::UpdateDoorsHeight()
{
	RefreshDoorPointers(false);
	if (m_pDoor1) {
		m_pDoor1->GetMatrix().GetPosition().z = m_fDoorPos + m_fDoor1Z;
		if (m_bRotatedDoor) {
			CVector pos;
			pos.x = m_fDoor1X + m_fDoorPos * m_pDoor1->GetForward().y * 5.0f / 6.0f;
			pos.y = m_fDoor1Y - m_fDoorPos * m_pDoor1->GetForward().x * 5.0f / 6.0f;
			pos.z = m_pDoor1->GetPosition().z;
			m_pDoor1->SetPosition(pos);
			BuildRotatedDoorMatrix(m_pDoor1, m_fDoorPos / m_fDoorHeight);
		}
		m_pDoor1->GetMatrix().UpdateRW();
		m_pDoor1->UpdateRwFrame();
	}
	if (m_pDoor2) {
		m_pDoor2->GetMatrix().GetPosition().z = m_fDoorPos + m_fDoor2Z;
		if (m_bRotatedDoor) {
			CVector pos;
			pos.x = m_fDoor2X + m_fDoorPos * m_pDoor2->GetForward().y * 5.0f / 6.0f;
			pos.y = m_fDoor2Y - m_fDoorPos * m_pDoor2->GetForward().x * 5.0f / 6.0f;
			pos.z = m_pDoor2->GetPosition().z;
			m_pDoor2->SetPosition(pos);
			BuildRotatedDoorMatrix(m_pDoor2, m_fDoorPos / m_fDoorHeight);
		}
		m_pDoor2->GetMatrix().UpdateRW();
		m_pDoor2->UpdateRwFrame();
	}
}

void CGarage::BuildRotatedDoorMatrix(CEntity * pDoor, float fPosition)
{
	// Delegate the up vector to the C implementation
	float doorUp[3];
	Garage_RotatedDoorUp(fPosition, pDoor->GetForward().x, pDoor->GetForward().y, doorUp);
	CVector up(doorUp[0], doorUp[1], doorUp[2]);
	pDoor->GetRight() = CrossProduct(up, pDoor->GetForward()); // rouz edit (ChatGPT)
	pDoor->GetUp() = up; // rouz edit (ChatGPT)
}

void CGarage::UpdateCrusherAngle()
{
	RefreshDoorPointers(false);
	m_pDoor2->GetMatrix().SetRotateXOnly(TWOPI - m_fDoorPos);
	m_pDoor2->GetMatrix().UpdateRW();
	m_pDoor2->UpdateRwFrame();
}

void CGarage::UpdateCrusherShake(float X, float Y)
{
	RefreshDoorPointers(false);
	m_pDoor1->GetMatrix().GetPosition().x += X;
	m_pDoor1->GetMatrix().GetPosition().y += Y;
	m_pDoor1->GetMatrix().UpdateRW();
	m_pDoor1->UpdateRwFrame();
	m_pDoor1->GetMatrix().GetPosition().x -= X;
	m_pDoor1->GetMatrix().GetPosition().y -= Y;
	m_pDoor2->GetMatrix().GetPosition().x += X;
	m_pDoor2->GetMatrix().GetPosition().y += Y;
	m_pDoor2->GetMatrix().UpdateRW();
	m_pDoor2->UpdateRwFrame();
	m_pDoor2->GetMatrix().GetPosition().x -= X;
	m_pDoor2->GetMatrix().GetPosition().y -= Y;
}

void CGarage::RefreshDoorPointers(bool bCreate)
{
	bool bNeedToFindDoorEntities = bCreate || m_bRecreateDoorOnNextRefresh;
	m_bRecreateDoorOnNextRefresh = false;
	if (m_pDoor1) {
		if (m_bDoor1IsDummy) {
						// Access raw storage through the C store or pool API
			if (CPool_GetIsFree(Pools_GetDummyPool(), CPool_GetJustIndex_NoFreeAssert(Pools_GetDummyPool(), (CDummy*)m_pDoor1)))
							bNeedToFindDoorEntities = true;
			else {
								// Access raw storage through the C store or pool API
				if (m_bDoor1PoolIndex != (CPool_GetIndex(Pools_GetDummyPool(), (CDummy*)m_pDoor1) & 0x7F))
									bNeedToFindDoorEntities = true;
				if (!Garages_IsModelIndexADoor(m_pDoor1->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
		else {
						// Access raw storage through the C store or pool API
			if (CPool_GetIsFree(Pools_GetObjectPool(), CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), (CObject*)m_pDoor1)))
							bNeedToFindDoorEntities = true;
			else {
								// Access raw storage through the C store or pool API
				if (m_bDoor1PoolIndex != (CPool_GetIndex(Pools_GetObjectPool(), (CObject*)m_pDoor1) & 0x7F))
									bNeedToFindDoorEntities = true;
				if (!Garages_IsModelIndexADoor(m_pDoor1->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
	}
	if (m_pDoor2) {
		if (m_bDoor2IsDummy) {
						// Access raw storage through the C store or pool API
			if (CPool_GetIsFree(Pools_GetDummyPool(), CPool_GetJustIndex_NoFreeAssert(Pools_GetDummyPool(), (CDummy*)m_pDoor2)))
							bNeedToFindDoorEntities = true;
			else {
								// Access raw storage through the C store or pool API
				if (m_bDoor2PoolIndex != (CPool_GetIndex(Pools_GetDummyPool(), (CDummy*)m_pDoor2) & 0x7F))
									bNeedToFindDoorEntities = true;
				if (!Garages_IsModelIndexADoor(m_pDoor2->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
		else {
						// Access raw storage through the C store or pool API
			if (CPool_GetIsFree(Pools_GetObjectPool(), CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), (CObject*)m_pDoor2)))
							bNeedToFindDoorEntities = true;
			else {
								// Access raw storage through the C store or pool API
				if (m_bDoor2PoolIndex != (CPool_GetIndex(Pools_GetObjectPool(), (CObject*)m_pDoor2) & 0x7F))
									bNeedToFindDoorEntities = true;
				if (!Garages_IsModelIndexADoor(m_pDoor2->GetModelIndex()))
					bNeedToFindDoorEntities = true;
			}
		}
	}
	if (bNeedToFindDoorEntities)
		FindDoorsEntities();
}

void Garages_TriggerMessage(const char* text, int16 num1, uint16 time, int16 num2)
{
	if (strcmp(text, Garages_MessageIDString) == 0 &&
		(*Timer_GetTimeInMilliseconds()) >= Garages_MessageStartTime &&
		(*Timer_GetTimeInMilliseconds()) <= Garages_MessageEndTime) {
		if ((*Timer_GetTimeInMilliseconds()) - Garages_MessageStartTime <= 500)
			return;
		Garages_MessageStartTime = (*Timer_GetTimeInMilliseconds()) - 500;
		Garages_MessageEndTime = (*Timer_GetTimeInMilliseconds()) - 500 + time;
	}
	else {
		strcpy(Garages_MessageIDString, text);
		Garages_MessageStartTime = (*Timer_GetTimeInMilliseconds());
		Garages_MessageEndTime = (*Timer_GetTimeInMilliseconds()) + time;
	}
	Garages_MessageNumberInString = num1;
	Garages_MessageNumberInString2 = num2;
}

void Garages_SetTargetCarForMissonGarage(int16 garage, CVehicle* pVehicle)
{
	assert(garage >= 0 && garage < NUM_GARAGES);
	if (pVehicle) {
		Garages_aGarages[garage].m_pTarget = pVehicle;
		Garages_aGarages[garage].m_pTarget->RegisterReference((CEntity**)&Garages_aGarages[garage].m_pTarget);
		if (Garages_aGarages[garage].m_eGarageState == GS_CLOSEDCONTAINSCAR)
			Garages_aGarages[garage].m_eGarageState = GS_FULLYCLOSED;
	}
	else
		Garages_aGarages[garage].m_pTarget = nil;
}

bool Garages_HasCarBeenDroppedOffYet(int16 garage)
{
	return Garages_aGarages[garage].m_eGarageState == GS_CLOSEDCONTAINSCAR;
}

void Garages_DeActivateGarage(int16 garage)
{
	Garages_aGarages[garage].m_bDeactivated = true;
}

void Garages_ActivateGarage(int16 garage)
{
	Garages_aGarages[garage].m_bDeactivated = false;
	if (Garages_aGarages[garage].m_eGarageType == GARAGE_FORCARTOCOMEOUTOF && Garages_aGarages[garage].m_eGarageState == GS_FULLYCLOSED)
		Garages_aGarages[garage].m_eGarageState = GS_OPENING;
}

int32 Garages_QueryCarsCollected(int16 garage)
{
	return 0;
}

bool Garages_HasImportExportGarageCollectedThisCar(int16 garage, int8 car)
{
	return Garages_CarTypesCollected[Garages_GetCarsCollectedIndexForGarageType(Garages_aGarages[garage].m_eGarageType)] & (BIT(car));
}

bool Garages_IsGarageOpen(int16 garage)
{
	return Garages_aGarages[garage].IsOpen();
}

bool Garages_IsGarageClosed(int16 garage)
{
	return Garages_aGarages[garage].IsClosed();
}

bool Garages_HasThisCarBeenCollected(int16 garage, uint8 id)
{
	return Garages_aGarages[garage].m_bCollectedCarsState & BIT(id);
}

bool CGarage::DoesCraigNeedThisCar(int32 mi)
{
	int ct = Garages_GetCarsCollectedIndexForGarageType(m_eGarageType);
	for (int i = 0; i < TOTAL_COLLECTCARS_CARS; i++) {
		if (mi == gaCarsToCollectInCraigsGarages[ct][i] || (gaCarsToCollectInCraigsGarages[ct][i] == MI_CHEETAH && mi == MI_VICECHEE))
			return (Garages_CarTypesCollected[ct] & BIT(i)) == 0;
	}
	return false;
}

bool CGarage::HasCraigCollectedThisCar(int32 mi)
{
	int ct = Garages_GetCarsCollectedIndexForGarageType(m_eGarageType);
	for (int i = 0; i < TOTAL_COLLECTCARS_CARS; i++) {
		if (mi == gaCarsToCollectInCraigsGarages[ct][i] || (gaCarsToCollectInCraigsGarages[ct][i] == MI_CHEETAH && mi == MI_VICECHEE))
			return Garages_CarTypesCollected[ct] & BIT(i);
	}
	return false;
}

bool CGarage::MarkThisCarAsCollectedForCraig(int32 mi)
{
	int ct = Garages_GetCarsCollectedIndexForGarageType(m_eGarageType);
	int index;
	for (index = 0; index < TOTAL_COLLECTCARS_CARS; index++) {
		if (mi == gaCarsToCollectInCraigsGarages[ct][index] || (gaCarsToCollectInCraigsGarages[ct][index] == MI_CHEETAH && mi == MI_VICECHEE))
			break;
	}
	if (index >= TOTAL_COLLECTCARS_CARS)
		return false;
	Garages_CarTypesCollected[ct] |= BIT(index);
	World_Players[World_PlayerInFocus].m_nMoney += IMPORT_REWARD;
	for (int i = 0; i < TOTAL_COLLECTCARS_CARS; i++) {
		if ((Garages_CarTypesCollected[ct] & BIT(i)) == 0) {
			Garages_TriggerMessage("GA_13", -1, 5000, -1); // Delivered like a pro. Complete the list and there'll be a bonus for you.
			return false;
		}
	}
	World_Players[World_PlayerInFocus].m_nMoney += IMPORT_ALLCARS_REWARD;
	Garages_TriggerMessage("GA_14", -1, 5000, -1); // All the cars. NICE! Here's a little something.
	return true;
}

void CGarage::OpenThisGarage()
{
	// Delegate to the C implementation
	Garage_OpenThis(this);
}

void CGarage::CloseThisGarage()
{
	// Delegate to the C implementation
	Garage_CloseThis(this);
}

float CGarage::CalcDistToGarageRectangleSquared(float X, float Y)
{
	// Delegate to the C implementation
	return Garage_CalcDistToRectangleSquared(this, X, Y);
}

float CGarage::CalcSmallestDistToGarageDoorSquared(float X, float Y)
{
	// Delegate to the C implementation
	return Garage_CalcSmallestDistToDoorSquared(this, X, Y);
}

void CGarage::FindDoorsEntities()
{
	m_pDoor1 = nil;
	m_pDoor2 = nil;
	// Delegate the range of sectors to the C implementation
	int xstart, xend, ystart, yend;
	World_ClampSectorRange(World_GetSectorIndexX(GetGarageCenterX() - 100.0f), World_GetSectorIndexX(GetGarageCenterX() + 100.0f), NUMSECTORS_X, &xstart, &xend);
	World_ClampSectorRange(World_GetSectorIndexY(GetGarageCenterY() - 100.0f), World_GetSectorIndexY(GetGarageCenterY() + 100.0f), NUMSECTORS_Y, &ystart, &yend);
	assert(xstart <= xend);
	assert(ystart <= yend);

	World_AdvanceCurrentScanCode();

	for (int y = ystart; y <= yend; y++) {
		for (int x = xstart; x <= xend; x++) {
			CSector* s = World_GetSector(x, y);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_OBJECTS], false);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_OBJECTS_OVERLAP], false);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_DUMMIES], true);
			FindDoorsEntitiesSectorList(s->m_lists[ENTITYLIST_DUMMIES_OVERLAP], true);
		}
	}
	if (m_pDoor1 && m_pDoor2) {
		CVector2D vecDoor1ToGarage(m_pDoor1->GetPosition().x - GetGarageCenterX(), m_pDoor1->GetPosition().y - GetGarageCenterY());
		CVector2D vecDoor2ToGarage(m_pDoor2->GetPosition().x - GetGarageCenterX(), m_pDoor2->GetPosition().y - GetGarageCenterY());
		if (DotProduct2D(vecDoor1ToGarage, vecDoor2ToGarage) > 0.0f) {
			if (vecDoor1ToGarage.MagnitudeSqr() >= vecDoor2ToGarage.MagnitudeSqr()) {
				m_pDoor1 = m_pDoor2;
				m_bDoor1IsDummy = m_bDoor2IsDummy;
			}
			m_pDoor2 = nil;
			m_bDoor2IsDummy = false;
		}
	}
	if (m_pDoor1)
		m_pDoor1->bUsesCollision = true;
	if (m_pDoor2)
		m_pDoor2->bUsesCollision = true;
}

void CGarage::FindDoorsEntitiesSectorList(CPtrList& list, bool dummy)
{
	CPtrNode* node;
	for (node = list.first; node; node = node->next) {
		CEntity* pEntity = (CEntity*)node->item;
		if (pEntity->m_scanCode == World_GetCurrentScanCode())
			continue;
		pEntity->m_scanCode = World_GetCurrentScanCode();
		if (!pEntity || !Garages_IsModelIndexADoor(pEntity->GetModelIndex()))
			continue;
		if (!IsPointInsideGarage(pEntity->GetPosition(), 2.0f))
			continue;
		if (!m_pDoor1) {
			m_pDoor1 = pEntity;
			m_bDoor1IsDummy = dummy;
			if (dummy)
								// Access raw storage through the C store or pool API
				m_bDoor1PoolIndex = (CPool_GetIndex(Pools_GetDummyPool(), (CDummy*)pEntity)) & 0x7F;
							else
								// Access raw storage through the C store or pool API
				m_bDoor1PoolIndex = (CPool_GetIndex(Pools_GetObjectPool(), (CObject*)pEntity)) & 0x7F;
							continue;
		}
		else {
			m_pDoor2 = pEntity;
			m_bDoor2IsDummy = dummy;
			if (dummy)
								// Access raw storage through the C store or pool API
				m_bDoor2PoolIndex = (CPool_GetIndex(Pools_GetDummyPool(), (CDummy*)pEntity)) & 0x7F;
							else
								// Access raw storage through the C store or pool API
				m_bDoor2PoolIndex = (CPool_GetIndex(Pools_GetObjectPool(), (CObject*)pEntity)) & 0x7F;
						}
	}
}

bool Garages_HasResprayHappened(int16 garage)
{
	bool result = Garages_aGarages[garage].m_bResprayHappened;
	Garages_aGarages[garage].m_bResprayHappened = false;
	return result;
}

void Garages_SetGarageDoorToRotate(int16 garage)
{
	if (Garages_aGarages[garage].m_bRotatedDoor)
		return;
	Garages_aGarages[garage].m_bRotatedDoor = true;
	Garages_aGarages[garage].m_fDoorHeight /= 2.0f;
	Garages_aGarages[garage].m_fDoorHeight -= 0.1f;
	Garages_aGarages[garage].m_fDoorPos = Min(Garages_aGarages[garage].m_fDoorHeight, Garages_aGarages[garage].m_fDoorPos);
	Garages_aGarages[garage].UpdateDoorsHeight();
}

void Garages_SetLeaveCameraForThisGarage(int16 garage)
{
	Garages_aGarages[garage].m_bCameraFollowsPlayer = true;
}

bool Garages_IsThisCarWithinGarageArea(int16 garage, CEntity * pCar)
{
	return Garages_aGarages[garage].IsEntityEntirelyInside3D(pCar, 0.0f);
}

bool Garages_HasCarBeenCrushed(int32 handle)
{
	return Garages_CrushedCarId == handle;
}



void CGarage::StoreAndRemoveCarsForThisHideout(CStoredCar* aCars, int32 nMax)
{
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++)
		StoredCar_Clear(&aCars[i]);
		// Access raw storage through the C store or pool API
	int i = CPool_GetSize(Pools_GetVehiclePool());
		int index = 0;
	while (i--) {
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (!pVehicle)
			continue;
		if (IsPointInsideGarage(pVehicle->GetPosition())) {
			if (pVehicle->VehicleCreatedBy != MISSION_VEHICLE) {
				if (index < Max(NUM_GARAGE_STORED_CARS, nMax) && !EntityHasASphereWayOutsideGarage(pVehicle, 1.0f))
					StoredCar_StoreCar(&aCars[index++], pVehicle);
				World_Players[World_PlayerInFocus].CancelPlayerEnteringCars(pVehicle);
				World_Remove(pVehicle);
				// Destroy and release the stored vehicle without invoking C++ delete.
				pVehicle->~CVehicle();
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetVehiclePool(), pVehicle);
							}
		}
	}
	// why?
	for (i = index; i < NUM_GARAGE_STORED_CARS; i++)
		StoredCar_Clear(&aCars[i]);
}

bool CGarage::RestoreCarsForThisHideout(CStoredCar* aCars)
{
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		if (StoredCar_HasCar(&aCars[i])) {
			CVehicle* pVehicle = StoredCar_RestoreCar(&aCars[i]);
			if (pVehicle) {
				World_Add(pVehicle);
				StoredCar_Clear(&aCars[i]);
			}
		}
	}
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		if (StoredCar_HasCar(&aCars[i]))
			return false;
	}
	return true;
}

bool Garages_IsPointInAGarageCameraZone(CVector point)
{
#ifdef FIX_BUGS
	for (uint32 i = 0; i < Garages_NumGarages; i++) {
#else
	for (int i = 0; i < NUM_GARAGES; i++) {
#endif
		switch (Garages_aGarages[i].m_eGarageType) {
		case GARAGE_NONE:
			break;
		case GARAGE_COLLECTCARS_1:
		case GARAGE_COLLECTCARS_2:
		case GARAGE_COLLECTCARS_3:
		case GARAGE_COLLECTCARS_4:
			if (Garages_aGarages[i].IsPointInsideGarage(point, MARGIN_FOR_CAMERA_COLLECTCARS))
				return true;
			break;
		default:
			if (Garages_aGarages[i].IsPointInsideGarage(point, MARGIN_FOR_CAMERA_DEFAULT))
				return true;
			break;
		}
	}
	return false;
}

bool Garages_CameraShouldBeOutside()
{
	return Garages_bCamShouldBeOutisde;
}

void Garages_GivePlayerDetonator()
{
	CPlayerPed* pPed = FindPlayerPed();
	int slot = WeaponInfo_GetWeaponInfo(WEAPONTYPE_DETONATOR)->m_nWeaponSlot;
	pPed->GiveWeapon(WEAPONTYPE_DETONATOR, 1);
	pPed->GetWeapon(pPed->GetWeaponSlot(WEAPONTYPE_DETONATOR)).m_eWeaponState = WEAPONSTATE_READY;
	pPed->m_nSelectedWepSlot = slot;
	if (pPed->m_storedWeapon != WEAPONTYPE_UNIDENTIFIED)
		pPed->m_storedWeapon = WEAPONTYPE_DETONATOR;
}

float Garages_FindDoorHeightForMI(int32 mi)
{
	return ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(mi)))->boundingBox.max.z - ((CColModel*)ModelRegistry_Collision(ModelRegistry_Get(mi)))->boundingBox.min.z - 0.1f; // rouz edit (ChatGPT)
}

void CGarage::TidyUpGarage()
{
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetVehiclePool());
	#ifdef FIX_BUGS
	while (i--) {
#else
	while (--i) {
#endif
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (pVehicle && (pVehicle->IsCar() || pVehicle->IsBike())) {
			if (IsPointInsideGarage(pVehicle->GetPosition())) {
				if (pVehicle->GetStatus() == STATUS_WRECKED || pVehicle->GetUp().z < 0.5f) { // rouz edit (ChatGPT)
					World_Remove(pVehicle);
					// Destroy and release the garage vehicle without invoking C++ delete.
					pVehicle->~CVehicle();
										// Access raw storage through the C store or pool API
					CPool_Delete(Pools_GetVehiclePool(), pVehicle);
									}
			}
		}
	}
}

void CGarage::TidyUpGarageClose()
{
		// Access raw storage through the C store or pool API
	uint32 i = CPool_GetSize(Pools_GetVehiclePool());
	#ifdef FIX_BUGS
	while (i--) {
#else
	while (--i) {
#endif
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
				if (!pVehicle)
			continue;
		if ((!pVehicle->IsCar() && !pVehicle->IsBike()) || pVehicle->GetStatus() != STATUS_WRECKED || !IsEntityTouching3D(pVehicle))
			continue;
		bool bRemove = false;
		if (m_eGarageState != GS_FULLYCLOSED) {
			CColModel* pColModel = pVehicle->GetColModel();
			for (int i = 0; i < pColModel->numSpheres; i++) {
				CVector pos = pVehicle->GetMatrix() * CollisionVector_ToNative(pColModel->spheres[i].bounds.center);
				float radius = pColModel->spheres[i].bounds.radius;
				if (!IsPointInsideGarage(pos, radius))
					bRemove = true;
			}
		}
		else
			bRemove = true;
		if (bRemove) {
			// no MISSION_VEHICLE check???
			World_Remove(pVehicle);
			// Destroy and release the garage vehicle without invoking C++ delete.
			pVehicle->~CVehicle();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), pVehicle);
					}
	}
}

void Garages_PlayerArrestedOrDied()
{
	static int GarageToBeTidied = 0; // lol
#ifdef FIX_BUGS
	for (uint32 i = 0; i < Garages_NumGarages; i++) {
#else
	for (int i = 0; i < NUM_GARAGES; i++) {
#endif
		if (Garages_aGarages[i].m_eGarageType != GARAGE_NONE)
			Garages_aGarages[i].PlayerArrestedOrDied();
	}
	Garages_MessageEndTime = 0;
	Garages_MessageStartTime = 0;
}

void CGarage::PlayerArrestedOrDied()
{
	switch (m_eGarageType) {
	case GARAGE_MISSION:
	case GARAGE_COLLECTORSITEMS:
	case GARAGE_COLLECTSPECIFICCARS:
	case GARAGE_COLLECTCARS_1:
	case GARAGE_COLLECTCARS_2:
	case GARAGE_COLLECTCARS_3:
	case GARAGE_FORCARTOCOMEOUTOF:
	case GARAGE_60SECONDS:
	case GARAGE_MISSION_KEEPCAR:
	case GARAGE_FOR_SCRIPT_TO_OPEN:
	case GARAGE_HIDEOUT_ONE:
	case GARAGE_HIDEOUT_TWO:
	case GARAGE_HIDEOUT_THREE:
	case GARAGE_FOR_SCRIPT_TO_OPEN_AND_CLOSE:
	case GARAGE_KEEPS_OPENING_FOR_SPECIFIC_CAR:
	case GARAGE_MISSION_KEEPCAR_REMAINCLOSED:
	case GARAGE_COLLECTCARS_4:
	case GARAGE_FOR_SCRIPT_TO_OPEN_FOR_CAR:
	case GARAGE_HIDEOUT_FOUR:
	case GARAGE_HIDEOUT_FIVE:
	case GARAGE_HIDEOUT_SIX:
	case GARAGE_HIDEOUT_SEVEN:
	case GARAGE_HIDEOUT_EIGHT:
	case GARAGE_HIDEOUT_NINE:
	case GARAGE_HIDEOUT_TEN:
	case GARAGE_HIDEOUT_ELEVEN:
	case GARAGE_HIDEOUT_TWELVE:
		switch (m_eGarageState) {
		case GS_OPENED:
		case GS_CLOSING:
		case GS_OPENING:
			m_eGarageState = GS_CLOSING;
			break;
		default:
			break;
		}
		break;
	case GARAGE_BOMBSHOP1:
	case GARAGE_BOMBSHOP2:
	case GARAGE_BOMBSHOP3:
	case GARAGE_RESPRAY:
	case GARAGE_CRUSHER:
		switch (m_eGarageState) {
		case GS_FULLYCLOSED:
		case GS_CLOSING:
		case GS_OPENING:
			m_eGarageState = GS_OPENING;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void CGarage::CenterCarInGarage(CVehicle* pVehicle)
{
	if (IsAnyOtherCarTouchingGarage(FindPlayerVehicle()))
		return;
	if (IsAnyOtherPedTouchingGarage(FindPlayerPed()))
		return;
	CVector pos = pVehicle->GetPosition();
	float garageX = GetGarageCenterX();
	float garageY = GetGarageCenterY();
	float offsetX = garageX - pos.x;
	float offsetY = garageY - pos.y;
	float offsetZ = pos.z - pos.z;
	float distance = CVector(offsetX, offsetY, offsetZ).Magnitude();
	if (distance < RESPRAY_CENTERING_COEFFICIENT) {
		pVehicle->GetMatrix().GetPosition().x = GetGarageCenterX();
		pVehicle->GetMatrix().GetPosition().y = GetGarageCenterY();
	}
	else {
		pVehicle->GetMatrix().GetPosition().x += offsetX * RESPRAY_CENTERING_COEFFICIENT / distance;
		pVehicle->GetMatrix().GetPosition().y += offsetY * RESPRAY_CENTERING_COEFFICIENT / distance;
	}
	if (!IsEntityEntirelyInside3D(pVehicle, 0.3f))
		pVehicle->SetPosition(pos);
}

void Garages_CloseHideOutGaragesBeforeSave()
{
#ifdef FIX_BUGS
	for (uint32 i = 0; i < Garages_NumGarages; i++) {
#else
	for (int i = 0; i < NUM_GARAGES; i++) {
#endif
		if (!Garages_IsThisGarageTypeSafehouse(Garages_aGarages[i].m_eGarageType))
			continue;
		if (Garages_aGarages[i].m_eGarageState != GS_FULLYCLOSED) {
			Garages_aGarages[i].m_eGarageState = GS_FULLYCLOSED;
			Garages_aGarages[i].StoreAndRemoveCarsForThisHideout(Garages_aCarsInSafeHouses[Garages_FindSafeHouseIndexForGarageType(Garages_aGarages[i].m_eGarageType)], NUM_GARAGE_STORED_CARS);
			Garages_aGarages[i].RemoveCarsBlockingDoorNotInside();
			Garages_aGarages[i].m_fDoorPos = 0.0f;
			Garages_aGarages[i].UpdateDoorsHeight();
		}
	}
}

int32 Garages_CountCarsInHideoutGarage(uint8 type)
{
	int32 total = 0;
	for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		total += StoredCar_HasCar(&Garages_aCarsInSafeHouses[Garages_FindSafeHouseIndexForGarageType(type)][i]);
	}
	return total;
}

bool Garages_IsPointWithinHideOutGarage(Const CVector& point)
{
#ifdef FIX_BUGS
	for (uint32 i = 0; i < Garages_NumGarages; i++) {
#else
	for (int i = 0; i < NUM_GARAGES; i++) {
#endif
		switch (Garages_aGarages[i].m_eGarageType) {
		case GARAGE_HIDEOUT_ONE:
		case GARAGE_HIDEOUT_TWO:
		case GARAGE_HIDEOUT_THREE:
		case GARAGE_HIDEOUT_FOUR:
		case GARAGE_HIDEOUT_FIVE:
		case GARAGE_HIDEOUT_SIX:
		case GARAGE_HIDEOUT_SEVEN:
		case GARAGE_HIDEOUT_EIGHT:
		case GARAGE_HIDEOUT_NINE:
		case GARAGE_HIDEOUT_TEN:
		case GARAGE_HIDEOUT_ELEVEN:
		case GARAGE_HIDEOUT_TWELVE:
			if (Garages_aGarages[i].IsPointInsideGarage(point))
				return true;
		default: break;
		}
	}
	return false;
}

bool Garages_IsPointWithinAnyGarage(Const CVector& point)
{
#ifdef FIX_BUGS
	for (uint32 i = 0; i < Garages_NumGarages; i++) {
#else
	for (int i = 0; i < NUM_GARAGES; i++) {
#endif
		switch (Garages_aGarages[i].m_eGarageType) {
		case GARAGE_NONE:
			continue;
		default:
			if (Garages_aGarages[i].IsPointInsideGarage(point))
				return true;
		}
	}
	return false;
}

void Garages_SetAllDoorsBackToOriginalHeight()
{
#ifdef FIX_BUGS
	for (uint32 i = 0; i < Garages_NumGarages; i++) {
#else
	for (int i = 0; i < NUM_GARAGES; i++) {
#endif
		switch (Garages_aGarages[i].m_eGarageType) {
		case GARAGE_NONE:
			continue;
		default:
			Garages_aGarages[i].RefreshDoorPointers(true);
			if (Garages_aGarages[i].m_pDoor1) {
				Garages_aGarages[i].m_pDoor1->GetMatrix().GetPosition().x = Garages_aGarages[i].m_fDoor1X;
				Garages_aGarages[i].m_pDoor1->GetMatrix().GetPosition().y = Garages_aGarages[i].m_fDoor1Y;
				Garages_aGarages[i].m_pDoor1->GetMatrix().GetPosition().z = Garages_aGarages[i].m_fDoor1Z;
				if (Garages_aGarages[i].m_pDoor1->IsObject())
					((CObject*)Garages_aGarages[i].m_pDoor1)->m_objectMatrix.GetPosition().z = Garages_aGarages[i].m_fDoor1Z;
				if (Garages_aGarages[i].m_bRotatedDoor)
					Garages_aGarages[i].BuildRotatedDoorMatrix(Garages_aGarages[i].m_pDoor1, 0.0f);
				Garages_aGarages[i].m_pDoor1->GetMatrix().UpdateRW();
				Garages_aGarages[i].m_pDoor1->UpdateRwFrame();
			}
			if (Garages_aGarages[i].m_pDoor2) {
				Garages_aGarages[i].m_pDoor2->GetMatrix().GetPosition().x = Garages_aGarages[i].m_fDoor2X;
				Garages_aGarages[i].m_pDoor2->GetMatrix().GetPosition().y = Garages_aGarages[i].m_fDoor2Y;
				Garages_aGarages[i].m_pDoor2->GetMatrix().GetPosition().z = Garages_aGarages[i].m_fDoor2Z;
				if (Garages_aGarages[i].m_pDoor2->IsObject())
					((CObject*)Garages_aGarages[i].m_pDoor2)->m_objectMatrix.GetPosition().z = Garages_aGarages[i].m_fDoor2Z;
				if (Garages_aGarages[i].m_bRotatedDoor)
					Garages_aGarages[i].BuildRotatedDoorMatrix(Garages_aGarages[i].m_pDoor2, 0.0f);
				Garages_aGarages[i].m_pDoor2->GetMatrix().UpdateRW();
				Garages_aGarages[i].m_pDoor2->UpdateRwFrame();
			}
		}
	}
}

static void Garage_AssignForSave(CGarage *self, const CGarage *other)
{
    // Preserve enclosing member assignment and the stored record padding contract
    self->m_eGarageType = other->m_eGarageType;
    self->m_eGarageState = other->m_eGarageState;
    self->m_nMaxStoredCars = other->m_nMaxStoredCars;
    self->field_2 = other->field_2;
    self->m_bClosingWithoutTargetCar = other->m_bClosingWithoutTargetCar;
    self->m_bDeactivated = other->m_bDeactivated;
    self->m_bResprayHappened = other->m_bResprayHappened;
    self->m_nTargetModelIndex = other->m_nTargetModelIndex;
    self->m_pDoor1 = other->m_pDoor1;
    self->m_pDoor2 = other->m_pDoor2;
    self->m_bDoor1PoolIndex = other->m_bDoor1PoolIndex;
    self->m_bDoor2PoolIndex = other->m_bDoor2PoolIndex;
    self->m_bDoor1IsDummy = other->m_bDoor1IsDummy;
    self->m_bDoor2IsDummy = other->m_bDoor2IsDummy;
    self->m_bRecreateDoorOnNextRefresh = other->m_bRecreateDoorOnNextRefresh;
    self->m_bRotatedDoor = other->m_bRotatedDoor;
    self->m_bCameraFollowsPlayer = other->m_bCameraFollowsPlayer;
    self->m_vecCorner1 = other->m_vecCorner1;
    self->m_fInfZ = other->m_fInfZ;
    self->m_vDir1 = other->m_vDir1;
    self->m_vDir2 = other->m_vDir2;
    self->m_fSupZ = other->m_fSupZ;
    self->m_fDir1Len = other->m_fDir1Len;
    self->m_fDir2Len = other->m_fDir2Len;
    self->m_fInfX = other->m_fInfX;
    self->m_fSupX = other->m_fSupX;
    self->m_fInfY = other->m_fInfY;
    self->m_fSupY = other->m_fSupY;
    self->m_fDoorPos = other->m_fDoorPos;
    self->m_fDoorHeight = other->m_fDoorHeight;
    self->m_fDoor1X = other->m_fDoor1X;
    self->m_fDoor1Y = other->m_fDoor1Y;
    self->m_fDoor2X = other->m_fDoor2X;
    self->m_fDoor2Y = other->m_fDoor2Y;
    self->m_fDoor1Z = other->m_fDoor1Z;
    self->m_fDoor2Z = other->m_fDoor2Z;
    self->m_nTimeToStartAction = other->m_nTimeToStartAction;
    self->m_bCollectedCarsState = other->m_bCollectedCarsState;
    self->m_pTarget = other->m_pTarget;
    StoredCar_Assign(&self->m_sStoredCar, &other->m_sStoredCar);
}

void Garages_Save(uint8 * buf, uint32 * size)
{
//INITSAVEBUF
	*size = 7876; // for some reason it's not actual size again
	//*size = (6 * sizeof(uint32) + TOTAL_COLLECTCARS_GARAGES * sizeof(*CarTypesCollected) + sizeof(uint32) + TOTAL_HIDEOUT_GARAGES * NUM_GARAGE_STORED_CARS * sizeof(CStoredCar) + NUM_GARAGES * sizeof(CGarage));
#if !defined THIS_IS_STUPID && defined COMPATIBLE_SAVES
	memset(buf + 7340, 0, *size - 7340); // garbage data is written otherwise
#endif
	Garages_CloseHideOutGaragesBeforeSave();
		// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &Garages_NumGarages, sizeof(Garages_NumGarages));
	{
		// Materialize the saved value with its original serialized type
		uint32 saveValue = (uint32)Garages_BombsAreFree;
		WriteSaveBuf(&buf, &saveValue, sizeof(saveValue));
	}
	{
		// Materialize the saved value with its original serialized type
		uint32 saveValue = (uint32)Garages_RespraysAreFree;
		WriteSaveBuf(&buf, &saveValue, sizeof(saveValue));
	}
	WriteSaveBuf(&buf, &Garages_CarsCollected, sizeof(Garages_CarsCollected));
	WriteSaveBuf(&buf, &Garages_BankVansCollected, sizeof(Garages_BankVansCollected));
	WriteSaveBuf(&buf, &Garages_PoliceCarsCollected, sizeof(Garages_PoliceCarsCollected));
		for (int i = 0; i < TOTAL_COLLECTCARS_GARAGES; i++)
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &Garages_CarTypesCollected[i], sizeof(Garages_CarTypesCollected[i]));
				// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &Garages_LastTimeHelpMessage, sizeof(Garages_LastTimeHelpMessage));
		for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		for (int j = 0; j < TOTAL_HIDEOUT_GARAGES; j++) {
						// Transfer save data through the C buffer API with explicit sizes
			// Preserve record assignment semantics and compiler padding behavior
			StoredCar_Assign((CStoredCar*)buf, &Garages_aCarsInSafeHouses[j][i]);
			SkipSaveBuf(&buf, sizeof(Garages_aCarsInSafeHouses[j][i]));
					}
	}
	for (int i = 0; i < NUM_GARAGES; i++) {
#ifdef COMPATIBLE_SAVES
				// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_eGarageType, sizeof(Garages_aGarages[i].m_eGarageType));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_eGarageState, sizeof(Garages_aGarages[i].m_eGarageState));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_nMaxStoredCars, sizeof(Garages_aGarages[i].m_nMaxStoredCars));
		WriteSaveBuf(&buf, &Garages_aGarages[i].field_2, sizeof(Garages_aGarages[i].field_2));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bClosingWithoutTargetCar, sizeof(Garages_aGarages[i].m_bClosingWithoutTargetCar));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bDeactivated, sizeof(Garages_aGarages[i].m_bDeactivated));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bResprayHappened, sizeof(Garages_aGarages[i].m_bResprayHappened));
		ZeroSaveBuf(&buf, 1);
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_nTargetModelIndex, sizeof(Garages_aGarages[i].m_nTargetModelIndex));
		ZeroSaveBuf(&buf, 4 + 4);
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bDoor1PoolIndex, sizeof(Garages_aGarages[i].m_bDoor1PoolIndex));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bDoor2PoolIndex, sizeof(Garages_aGarages[i].m_bDoor2PoolIndex));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bDoor1IsDummy, sizeof(Garages_aGarages[i].m_bDoor1IsDummy));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bDoor2IsDummy, sizeof(Garages_aGarages[i].m_bDoor2IsDummy));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bRecreateDoorOnNextRefresh, sizeof(Garages_aGarages[i].m_bRecreateDoorOnNextRefresh));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bRotatedDoor, sizeof(Garages_aGarages[i].m_bRotatedDoor));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bCameraFollowsPlayer, sizeof(Garages_aGarages[i].m_bCameraFollowsPlayer));
		ZeroSaveBuf(&buf, 1);
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_vecCorner1, sizeof(Garages_aGarages[i].m_vecCorner1));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fInfZ, sizeof(Garages_aGarages[i].m_fInfZ));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_vDir1, sizeof(Garages_aGarages[i].m_vDir1));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_vDir2, sizeof(Garages_aGarages[i].m_vDir2));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fSupZ, sizeof(Garages_aGarages[i].m_fSupZ));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDir1Len, sizeof(Garages_aGarages[i].m_fDir1Len));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDir2Len, sizeof(Garages_aGarages[i].m_fDir2Len));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fInfX, sizeof(Garages_aGarages[i].m_fInfX));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fSupX, sizeof(Garages_aGarages[i].m_fSupX));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fInfY, sizeof(Garages_aGarages[i].m_fInfY));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fSupY, sizeof(Garages_aGarages[i].m_fSupY));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoorPos, sizeof(Garages_aGarages[i].m_fDoorPos));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoorHeight, sizeof(Garages_aGarages[i].m_fDoorHeight));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoor1X, sizeof(Garages_aGarages[i].m_fDoor1X));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoor1Y, sizeof(Garages_aGarages[i].m_fDoor1Y));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoor2X, sizeof(Garages_aGarages[i].m_fDoor2X));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoor2Y, sizeof(Garages_aGarages[i].m_fDoor2Y));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoor1Z, sizeof(Garages_aGarages[i].m_fDoor1Z));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_fDoor2Z, sizeof(Garages_aGarages[i].m_fDoor2Z));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_nTimeToStartAction, sizeof(Garages_aGarages[i].m_nTimeToStartAction));
		WriteSaveBuf(&buf, &Garages_aGarages[i].m_bCollectedCarsState, sizeof(Garages_aGarages[i].m_bCollectedCarsState));
		ZeroSaveBuf(&buf, 3 + 4);
		ZeroSaveBuf(&buf, sizeof(Garages_aGarages[i].m_sStoredCar));
		#else
				// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		Garage_AssignForSave((CGarage*)buf, &Garages_aGarages[i]);
		SkipSaveBuf(&buf, sizeof(Garages_aGarages[i]));
		#endif
	}
//VALIDATESAVEBUF(*size);
}


void Garages_Load(uint8* buf, uint32 size)
{
//INITSAVEBUF
	assert(size == 7876);
	//assert(size == (6 * sizeof(uint32) + TOTAL_COLLECTCARS_GARAGES * sizeof(*CarTypesCollected) + sizeof(uint32) + TOTAL_HIDEOUT_GARAGES * NUM_GARAGE_STORED_CARS * sizeof(CStoredCar) + NUM_GARAGES * sizeof(CGarage)));
	Garages_CloseHideOutGaragesBeforeSave();
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Garages_NumGarages, &buf, sizeof(Garages_NumGarages));
		int32 tempInt;
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tempInt, &buf, sizeof(tempInt));
		Garages_BombsAreFree = tempInt ? true : false;
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tempInt, &buf, sizeof(tempInt));
		Garages_RespraysAreFree = tempInt ? true : false;
		// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Garages_CarsCollected, &buf, sizeof(Garages_CarsCollected));
	ReadSaveBuf(&Garages_BankVansCollected, &buf, sizeof(Garages_BankVansCollected));
	ReadSaveBuf(&Garages_PoliceCarsCollected, &buf, sizeof(Garages_PoliceCarsCollected));
		for (int i = 0; i < TOTAL_COLLECTCARS_GARAGES; i++)
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Garages_CarTypesCollected[i], &buf, sizeof(Garages_CarTypesCollected[i]));
				// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Garages_LastTimeHelpMessage, &buf, sizeof(Garages_LastTimeHelpMessage));
		for (int i = 0; i < NUM_GARAGE_STORED_CARS; i++) {
		for (int j = 0; j < TOTAL_HIDEOUT_GARAGES; j++) {
						// Transfer save data through the C buffer API with explicit sizes
			// Preserve record assignment semantics and compiler padding behavior
			StoredCar_Assign(&Garages_aCarsInSafeHouses[j][i], (const CStoredCar*)buf);
			SkipSaveBuf(&buf, sizeof(Garages_aCarsInSafeHouses[j][i]));
					}
	}
	for (int i = 0; i < NUM_GARAGES; i++) {
#ifdef COMPATIBLE_SAVES
				// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Garages_aGarages[i].m_eGarageType, &buf, sizeof(Garages_aGarages[i].m_eGarageType));
		ReadSaveBuf(&Garages_aGarages[i].m_eGarageState, &buf, sizeof(Garages_aGarages[i].m_eGarageState));
		ReadSaveBuf(&Garages_aGarages[i].m_nMaxStoredCars, &buf, sizeof(Garages_aGarages[i].m_nMaxStoredCars));
		ReadSaveBuf(&Garages_aGarages[i].field_2, &buf, sizeof(Garages_aGarages[i].field_2));
		ReadSaveBuf(&Garages_aGarages[i].m_bClosingWithoutTargetCar, &buf, sizeof(Garages_aGarages[i].m_bClosingWithoutTargetCar));
		ReadSaveBuf(&Garages_aGarages[i].m_bDeactivated, &buf, sizeof(Garages_aGarages[i].m_bDeactivated));
		ReadSaveBuf(&Garages_aGarages[i].m_bResprayHappened, &buf, sizeof(Garages_aGarages[i].m_bResprayHappened));
		SkipSaveBuf(&buf, 1);
		ReadSaveBuf(&Garages_aGarages[i].m_nTargetModelIndex, &buf, sizeof(Garages_aGarages[i].m_nTargetModelIndex));
		SkipSaveBuf(&buf, 4 + 4);
		ReadSaveBuf(&Garages_aGarages[i].m_bDoor1PoolIndex, &buf, sizeof(Garages_aGarages[i].m_bDoor1PoolIndex));
		ReadSaveBuf(&Garages_aGarages[i].m_bDoor2PoolIndex, &buf, sizeof(Garages_aGarages[i].m_bDoor2PoolIndex));
		ReadSaveBuf(&Garages_aGarages[i].m_bDoor1IsDummy, &buf, sizeof(Garages_aGarages[i].m_bDoor1IsDummy));
		ReadSaveBuf(&Garages_aGarages[i].m_bDoor2IsDummy, &buf, sizeof(Garages_aGarages[i].m_bDoor2IsDummy));
		ReadSaveBuf(&Garages_aGarages[i].m_bRecreateDoorOnNextRefresh, &buf, sizeof(Garages_aGarages[i].m_bRecreateDoorOnNextRefresh));
		ReadSaveBuf(&Garages_aGarages[i].m_bRotatedDoor, &buf, sizeof(Garages_aGarages[i].m_bRotatedDoor));
		ReadSaveBuf(&Garages_aGarages[i].m_bCameraFollowsPlayer, &buf, sizeof(Garages_aGarages[i].m_bCameraFollowsPlayer));
		SkipSaveBuf(&buf, 1);
		ReadSaveBuf(&Garages_aGarages[i].m_vecCorner1, &buf, sizeof(Garages_aGarages[i].m_vecCorner1));
		ReadSaveBuf(&Garages_aGarages[i].m_fInfZ, &buf, sizeof(Garages_aGarages[i].m_fInfZ));
		ReadSaveBuf(&Garages_aGarages[i].m_vDir1, &buf, sizeof(Garages_aGarages[i].m_vDir1));
		ReadSaveBuf(&Garages_aGarages[i].m_vDir2, &buf, sizeof(Garages_aGarages[i].m_vDir2));
		ReadSaveBuf(&Garages_aGarages[i].m_fSupZ, &buf, sizeof(Garages_aGarages[i].m_fSupZ));
		ReadSaveBuf(&Garages_aGarages[i].m_fDir1Len, &buf, sizeof(Garages_aGarages[i].m_fDir1Len));
		ReadSaveBuf(&Garages_aGarages[i].m_fDir2Len, &buf, sizeof(Garages_aGarages[i].m_fDir2Len));
		ReadSaveBuf(&Garages_aGarages[i].m_fInfX, &buf, sizeof(Garages_aGarages[i].m_fInfX));
		ReadSaveBuf(&Garages_aGarages[i].m_fSupX, &buf, sizeof(Garages_aGarages[i].m_fSupX));
		ReadSaveBuf(&Garages_aGarages[i].m_fInfY, &buf, sizeof(Garages_aGarages[i].m_fInfY));
		ReadSaveBuf(&Garages_aGarages[i].m_fSupY, &buf, sizeof(Garages_aGarages[i].m_fSupY));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoorPos, &buf, sizeof(Garages_aGarages[i].m_fDoorPos));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoorHeight, &buf, sizeof(Garages_aGarages[i].m_fDoorHeight));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoor1X, &buf, sizeof(Garages_aGarages[i].m_fDoor1X));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoor1Y, &buf, sizeof(Garages_aGarages[i].m_fDoor1Y));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoor2X, &buf, sizeof(Garages_aGarages[i].m_fDoor2X));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoor2Y, &buf, sizeof(Garages_aGarages[i].m_fDoor2Y));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoor1Z, &buf, sizeof(Garages_aGarages[i].m_fDoor1Z));
		ReadSaveBuf(&Garages_aGarages[i].m_fDoor2Z, &buf, sizeof(Garages_aGarages[i].m_fDoor2Z));
		ReadSaveBuf(&Garages_aGarages[i].m_nTimeToStartAction, &buf, sizeof(Garages_aGarages[i].m_nTimeToStartAction));
		ReadSaveBuf(&Garages_aGarages[i].m_bCollectedCarsState, &buf, sizeof(Garages_aGarages[i].m_bCollectedCarsState));
		SkipSaveBuf(&buf, 3 + 4);
		SkipSaveBuf(&buf, sizeof(Garages_aGarages[i].m_sStoredCar));
		#else
				// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		Garage_AssignForSave(&Garages_aGarages[i], (const CGarage*)buf);
		SkipSaveBuf(&buf, sizeof(Garages_aGarages[i]));
		#endif
		Garages_aGarages[i].m_pDoor1 = nil;
		Garages_aGarages[i].m_pDoor2 = nil;
		Garages_aGarages[i].m_pTarget = nil;
		Garages_aGarages[i].m_bRecreateDoorOnNextRefresh = true;
		Garages_aGarages[i].RefreshDoorPointers(true);
		if (Garages_aGarages[i].m_eGarageType == GARAGE_CRUSHER)
			Garages_aGarages[i].UpdateCrusherAngle();
		else
			Garages_aGarages[i].UpdateDoorsHeight();
	}
//VALIDATESAVEBUF(size);

	Garages_MessageEndTime = 0;
	Garages_bCamShouldBeOutisde = false;
	Garages_MessageStartTime = 0;
}

bool
Garages_IsModelIndexADoor(uint32 id)
{
	return id == MI_GARAGEDOOR2 ||
		id == MI_GARAGEDOOR3 ||
		id == MI_GARAGEDOOR4 ||
		id == MI_GARAGEDOOR5 ||
		id == MI_GARAGEDOOR6 ||
		id == MI_GARAGEDOOR7 ||
		id == MI_GARAGEDOOR9 ||
		id == MI_GARAGEDOOR10 ||
		id == MI_GARAGEDOOR11 ||
		id == MI_GARAGEDOOR12 ||
		id == MI_GARAGEDOOR13 ||
		id == MI_GARAGEDOOR14 ||
		id == MI_GARAGEDOOR15 ||
		id == MI_GARAGEDOOR16 ||
		id == MI_GARAGEDOOR18 ||
		id == MI_GARAGEDOOR19 ||
		id == MI_GARAGEDOOR20 ||
		id == MI_GARAGEDOOR21 ||
		id == MI_GARAGEDOOR22 ||
		id == MI_GARAGEDOOR23 ||
		id == MI_GARAGEDOOR24 ||
		id == MI_GARAGEDOOR25 ||
		id == MI_GARAGEDOOR26;
}

void Garages_StopCarFromBlowingUp(CAutomobile* pCar)
{
	pCar->m_fFireBlowUpTimer = 0.0f;
	// Delegate the health and the engine of the saved car to the C implementation
	pCar->m_fHealth = Garage_SavedHealth(pCar->m_fHealth);
	Damage_SetEngineStatus(&pCar->Damage, Garage_SavedEngineStatus(Damage_GetEngineStatus(&pCar->Damage)));
}

bool CGarage::Does60SecondsNeedThisCarAtAll(int mi)
{
	for (int i = 0; i < ARRAY_SIZE(gaCarsToCollectIn60Seconds); i++) {
		if (gaCarsToCollectIn60Seconds[i] == mi)
			return true;
	}
	return false;
}

bool CGarage::Does60SecondsNeedThisCar(int mi)
{
	for (int i = 0; i < ARRAY_SIZE(gaCarsToCollectIn60Seconds); i++) {
		if (gaCarsToCollectIn60Seconds[i] == mi)
			return m_bCollectedCarsState & BIT(i);
	}
	return false;
}

void CGarage::MarkThisCarAsCollectedFor60Seconds(int mi)
{
	for (int i = 0; i < ARRAY_SIZE(gaCarsToCollectIn60Seconds); i++) {
		if (gaCarsToCollectIn60Seconds[i] == mi)
			m_bCollectedCarsState |= BIT(i);
	}
}

bool CGarage::IsPlayerEntirelyInsideGarage()
{
	return IsEntityEntirelyInside3D(FindPlayerVehicle() ? (CEntity*)FindPlayerVehicle() : (CEntity*)FindPlayerPed(), 0.0f);
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

int32_t StoredCar_BackendModelIndex(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->GetModelIndex();
}

StoredCarVector StoredCar_BackendPosition(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    const CVector &value = vehicle->GetPosition();
    const StoredCarVector result = { value.x, value.y, value.z };
    return result;
}

StoredCarVector StoredCar_BackendForward(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    const CVector &value = vehicle->GetForward();
    const StoredCarVector result = { value.x, value.y, value.z };
    return result;
}

uint8_t StoredCar_BackendPrimaryColor(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->m_currentColour1;
}

uint8_t StoredCar_BackendSecondaryColor(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->m_currentColour2;
}

uint8_t StoredCar_BackendRadioStation(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->m_nRadioStation;
}

int8_t StoredCar_BackendVariationA(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->m_aExtras[0];
}

int8_t StoredCar_BackendVariationB(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->m_aExtras[1];
}

bool StoredCar_BackendBulletProof(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->bBulletProof;
}

void StoredCar_BackendSetBulletProof(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    vehicle->bBulletProof = value;
}

bool StoredCar_BackendFireProof(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->bFireProof;
}

void StoredCar_BackendSetFireProof(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    vehicle->bFireProof = value;
}

bool StoredCar_BackendExplosionProof(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->bExplosionProof;
}

void StoredCar_BackendSetExplosionProof(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    vehicle->bExplosionProof = value;
}

bool StoredCar_BackendCollisionProof(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->bCollisionProof;
}

void StoredCar_BackendSetCollisionProof(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    vehicle->bCollisionProof = value;
}

bool StoredCar_BackendMeleeProof(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->bMeleeProof;
}

void StoredCar_BackendSetMeleeProof(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    vehicle->bMeleeProof = value;
}

bool StoredCar_BackendIsCar(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->IsCar();
}

bool StoredCar_BackendIsBike(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return vehicle->IsBike();
}

int8_t StoredCar_BackendBombType(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    return ((CAutomobile*)vehicle)->m_bombType;
}

void StoredCar_BackendRequestModel(int32_t model, int32_t flags)
{
    // Access the remaining native owner through its original operation
    Streaming_RequestModel(model, flags); // rouz edit (ChatGPT)
}

bool StoredCar_BackendHasModelLoaded(int32_t model)
{
    // Access the remaining native owner through its original operation
    return Streaming_HasModelLoaded(model); // rouz edit (ChatGPT)
}

CVehicleModelInfo * StoredCar_BackendModelInfo(int32_t model)
{
    // Access the remaining native owner through its original operation
    return (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(model)); // rouz edit (ChatGPT)
}

int8_t StoredCar_BackendNumComponents(CVehicleModelInfo *info)
{
    // Access the remaining native owner through its original operation
    return info->m_numComps;
}

void StoredCar_BackendSetComponents(int8_t first, int8_t second)
{
    // Access the remaining native owner through its original operation
    CVehicleModelInfo::SetComponentsToUse(first, second);
}

CVehicle * StoredCar_BackendAllocateVehicle(void)
{
    // Access the remaining native owner through its original operation
    return (CVehicle*)CPool_New(Pools_GetVehiclePool());
}

void StoredCar_BackendConstructBoat(CVehicle *vehicle, int32_t model)
{
    // Access the remaining native owner through its original operation
    std::allocator<CBoat>().construct((CBoat*)vehicle, model, RANDOM_VEHICLE);
}

void StoredCar_BackendConstructBike(CVehicle *vehicle, int32_t model)
{
    // Access the remaining native owner through its original operation
    std::allocator<CBike>().construct((CBike*)vehicle, model, RANDOM_VEHICLE);
}

void StoredCar_BackendConstructAutomobile(CVehicle *vehicle, int32_t model)
{
    // Access the remaining native owner through its original operation
    std::allocator<CAutomobile>().construct((CAutomobile*)vehicle, model, RANDOM_VEHICLE);
}

void StoredCar_BackendSetStanding(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    ((CBike*)vehicle)->bIsStanding = value;
}

void StoredCar_BackendSetPosition(CVehicle *vehicle, StoredCarVector value)
{
    // Access the remaining native owner through its original operation
    vehicle->SetPosition(CVector(value.x, value.y, value.z));
}

void StoredCar_BackendSetForward(CVehicle *vehicle, StoredCarVector value)
{
    // Access the remaining native owner through its original operation
    vehicle->GetForward() = CVector(value.x, value.y, value.z);
}

void StoredCar_BackendSetRight(CVehicle *vehicle, StoredCarVector value)
{
    // Access the remaining native owner through its original operation
    vehicle->GetRight() = CVector(value.x, value.y, value.z); // rouz edit (ChatGPT)
}

void StoredCar_BackendSetUp(CVehicle *vehicle, StoredCarVector value)
{
    // Access the remaining native owner through its original operation
    vehicle->GetUp() = CVector(value.x, value.y, value.z); // rouz edit (ChatGPT)
}

void StoredCar_BackendSetStatus(CVehicle *vehicle, int32_t status)
{
    // Access the remaining native owner through its original operation
    vehicle->SetStatus((eEntityStatus)status);
}

void StoredCar_BackendClearDriver(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    vehicle->pDriver = nil;
}

void StoredCar_BackendSetPrimaryColor(CVehicle *vehicle, int8_t value)
{
    // Access the remaining native owner through its original operation
    vehicle->m_currentColour1 = value;
}

void StoredCar_BackendSetSecondaryColor(CVehicle *vehicle, int8_t value)
{
    // Access the remaining native owner through its original operation
    vehicle->m_currentColour2 = value;
}

void StoredCar_BackendSetRadioStation(CVehicle *vehicle, int8_t value)
{
    // Access the remaining native owner through its original operation
    vehicle->m_nRadioStation = value;
}

void StoredCar_BackendSetFreebies(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    vehicle->bFreebies = value;
}

void StoredCar_BackendSetBombType(CVehicle *vehicle, int8_t value)
{
    // Access the remaining native owner through its original operation
    ((CAutomobile*)vehicle)->m_bombType = value;
}

void StoredCar_BackendSetOwnedByPlayer(CVehicle *vehicle, bool value)
{
    // Access the remaining native owner through its original operation
    vehicle->bHasBeenOwnedByPlayer = value;
}

CPed * StoredCar_BackendPlayerPed(void)
{
    // Access the remaining native owner through its original operation
    return FindPlayerPed();
}

void StoredCar_BackendSetBombRigger(CVehicle *vehicle, CPed *ped)
{
    // Access the remaining native owner through its original operation
    ((CAutomobile*)vehicle)->m_pBombRigger = ped;
}

void StoredCar_BackendUnlock(CVehicle *vehicle)
{
    // Access the remaining native owner through its original operation
    vehicle->m_nDoorLock = CARLOCK_UNLOCKED;
}

void StoredCar_BackendAssertFailed(const char *expression, const char *file, unsigned line, const char *function)
{
    // Access the remaining native owner through its original operation
    re3_assert(expression, file, line, function);
}
//- rouz edit (ChatGPT)
