//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelRegistry.h" // rouz edit (ChatGPT)

#include "Script.h"
#include "ScriptBytecode.h"
#include "DoubleList.h"
#include "ScriptCommands.h"

#include "AnimBlendAssociation.h"
#include "AudioManager.h"
#include "Boat.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CivilianPed.h"
#include "Clock.h"
#include "CopPed.h"
#include "Debug.h"
#include "DMAudio.h"
#include "EmergencyPed.h"
#include "FileMgr.h"
#include "Frontend.h"
#include "General.h"
#ifdef MISSION_REPLAY
#include "GenericGameStorage.h"
#endif
#include "HandlingMgr.h"
#include "Heli.h"
#include "Hud.h"
#include "Lines.h"
#include "Messages.h"
#include "Pad.h"
#include "Pickups.h"
#include "Pools.h"
#include "Population.h"
#include "Remote.h"
#include "Replay.h"
#include "Stats.h"
#include "Streaming.h"
#include "User.h"
#include "Wanted.h"
#include "Weather.h"
#include "Zones.h"
#include "main.h"
#include "Ropes.h"
#include "ColStore.h"
#include "Fluff.h"
#include "GameLogic.h"
#include "MBlur.h"
#include "PedRoutes.h"
#include "RoadBlocks.h"
#include "SpecialFX.h"
#include "Timecycle.h"
#include "TxdStore.h"
#include "Bike.h"
#include "ScriptArea.h"
#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
#include <stdarg.h>
#include "ScriptArea.h"
#endif

uint8 TheScripts_ScriptSpace[SIZE_SCRIPT_SPACE];
CRunningScript TheScripts_ScriptsArray[MAX_NUM_SCRIPTS];
intro_text_line TheScripts_IntroTextLines[MAX_NUM_INTRO_TEXT_LINES];
intro_script_rectangle TheScripts_IntroRectangles[MAX_NUM_INTRO_RECTANGLES];
Sprite2d TheScripts_ScriptSprites[MAX_NUM_SCRIPT_SRPITES];
static void Sprite2d_Exit_CTheScripts_ScriptSprites(void)
{
    // Destroy this original object group in its former reverse array order
    Sprite2d_DeinitArray(TheScripts_ScriptSprites,(size_t)(MAX_NUM_SCRIPT_SRPITES));
}
static int Sprite2d_Start_CTheScripts_ScriptSprites(void)
{
    // Initialize at the original definition boundary and retain its exit registration position
    Sprite2d_InitArray(TheScripts_ScriptSprites,(size_t)(MAX_NUM_SCRIPT_SRPITES));atexit(Sprite2d_Exit_CTheScripts_ScriptSprites);return 0;
}
static const int Sprite2d_Started_CTheScripts_ScriptSprites=Sprite2d_Start_CTheScripts_ScriptSprites();
script_sphere_struct TheScripts_ScriptSphereArray[MAX_NUM_SCRIPT_SPHERES];
tUsedObject TheScripts_UsedObjectArray[MAX_NUM_USED_OBJECTS];
int32 TheScripts_MultiScriptArray[MAX_NUM_MISSION_SCRIPTS];
tBuildingSwap TheScripts_BuildingSwapArray[MAX_NUM_BUILDING_SWAPS];
CEntity* TheScripts_InvisibilitySettingArray[MAX_NUM_INVISIBILITY_SETTINGS];
CStoredLine TheScripts_aStoredLines[MAX_NUM_STORED_LINES];
bool TheScripts_DbgFlag;
uint32 TheScripts_OnAMissionFlag;
int32 TheScripts_StoreVehicleIndex;
bool TheScripts_StoreVehicleWasRandom;
CRunningScript *TheScripts_pIdleScripts;
CRunningScript *TheScripts_pActiveScripts;
int32 TheScripts_NextFreeCollectiveIndex;
int32 TheScripts_LastRandomPedId;
uint16 TheScripts_NumberOfUsedObjects;
bool TheScripts_bAlreadyRunningAMissionScript;
bool TheScripts_bUsingAMultiScriptFile;
uint16 TheScripts_NumberOfMissionScripts;
uint32 TheScripts_LargestMissionScriptSize;
uint32 TheScripts_MainScriptSize;
uint8 TheScripts_FailCurrentMission;
uint16 TheScripts_NumScriptDebugLines;
uint16 TheScripts_NumberOfIntroRectanglesThisFrame;
uint16 TheScripts_NumberOfIntroTextLinesThisFrame;
uint8 TheScripts_UseTextCommands;
CMissionCleanup TheScripts_MissionCleanUp;
CUpsideDownCarCheck TheScripts_UpsideDownCars;
CStuckCarCheck TheScripts_StuckCars;
uint16 TheScripts_CommandsExecuted;
uint16 TheScripts_ScriptsUpdated;
int32 ScriptParams[32];
uint8 TheScripts_RiotIntensity;
uint32 TheScripts_LastMissionPassedTime;
uint16 TheScripts_NumberOfExclusiveMissionScripts;
bool TheScripts_bPlayerHasMetDebbieHarry;
bool TheScripts_bPlayerIsInTheStatium;
#if (defined GTA_PC && !defined GTAVC_JP_PATCH || defined GTA_XBOX || defined SUPPORT_XBOX_SCRIPT || defined GTA_MOBILE || defined SUPPORT_MOBILE_SCRIPT)
int16 TheScripts_CardStack[CARDS_IN_DECK * MAX_DECKS];
int16 TheScripts_CardStackPosition;
#endif

#ifdef MISSION_REPLAY

static const char* nonMissionScripts[] = {
	"copcar",
	"ambulan",
	"taxi",
	"firetru",
	"rampage",
	"t4x4_1",
	"t4x4_2",
	"t4x4_3",
	"rc1",
	"rc2",
	"rc3",
	"rc4",
	"hj",
	"usj",
	"mayhem",
	"range",
	"race",
	"pizza",
	"rcheli",
	"rcplne1",
	"rcrace1",
	"cokerun",
	"buypro1",
	"carbuy1",
	"buypro2",
	"icecut",
	"icecre1",
	"buypro3",
	"buypro4",
	"buypro5",
	"buypro6",
	"buypro7",
	"buypro8",
	"buypro9",
	"buypro10",
	"buypro11",
	"ovalrng",
	"mm",
	"kickst",
	"heli1sc",
	"heli2sc",
	"heli3sc",
	"heli4sc",
	"carpark_1",
	"bmx_1",
	"bmx_2"
};

static const char* MissionScripts[] = {
	"LAWYER1",
	"LAWYER2",
	"LAWYER3",
	"LAWYER4",
	"GENERL1",
	"COL2",
	"GENERL3",
	"COL_4",
	"COL_5",
	"baron1",
	"baron2",
	"baron3",
	"baron4",
	"kent1",
	"baron5",
	"serg1",
	"serg2",
	"serg3",
	"bankjo1",
	"bankjo2",
	"bankjo3",
	"bankjo4",
	"phil1",
	"phil2",
	"porno1",
	"porno2",
	"porno3",
	"porno4",
	"protec1",
	"protec2",
	"protec3",
	"count1",
	"count2",
	"CAP_1",
	"FIN_1",
	"bike1",
	"bike2",
	"bike3",
	"rockb1",
	"rockb2",
	"rockb3",
	"cuban1",
	"cuban2",
	"cuban3",
	"cuban4",
	"hait1",
	"hait2",
	"hait3",
	"assin1",
	"assin2",
	"assin3",
	"assin4",
	"assin5",
	"taxwar1",
	"taxwar2",
	"taxwar3"
};

int AllowMissionReplay;
uint32 NextMissionDelay;
uint32 MissionStartTime;
uint32 WaitForMissionActivate;
uint32 WaitForSave;
float oldTargetX;
float oldTargetY;
int missionRetryScriptIndex;
bool doingMissionRetry;
bool gbTryingPorn4Again;
int IsInAmmunation;
int MissionSkipLevel;

#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
bool UsingMobileScript;
bool AlreadySavedGame;
#endif

#endif

const uint32 CRunningScript::nSaveStructSize =
#ifdef COMPATIBLE_SAVES
	136;
#else
	sizeof(CRunningScript);
#endif

CMissionCleanup::CMissionCleanup()
{
	Init();
}

void CMissionCleanup::Init()
{
	m_nCount = 0;
	for (int i = 0; i < MAX_CLEANUP; i++){
		m_sEntities[i].type = CLEANUP_UNUSED;
		m_sEntities[i].id = 0;
	}
}

cleanup_entity_struct* CMissionCleanup::FindFree()
{
	for (int i = 0; i < MAX_CLEANUP; i++){
		if (m_sEntities[i].type == CLEANUP_UNUSED)
			return &m_sEntities[i];
	}
	script_assert(0);
	return nil;
}

void CMissionCleanup::AddEntityToList(int32 id, uint8 type)
{
	cleanup_entity_struct* pNew = FindFree();
	if (!pNew)
		return;
	pNew->id = id;
	pNew->type = type;
	m_nCount++;
}

void CMissionCleanup::RemoveEntityFromList(int32 id, uint8 type)
{
	for (int i = 0; i < MAX_CLEANUP; i++){
		if (m_sEntities[i].type == type && m_sEntities[i].id == id){
			switch (m_sEntities[i].type) {
			case CLEANUP_CAR:
			{
								// Access raw storage through the C store or pool API
				CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), m_sEntities[i].id));
								if (pVehicle) {
					if (pVehicle->bIsStaticWaitingForCollision) {
						pVehicle->bIsStaticWaitingForCollision = false;
						if (!pVehicle->GetIsStatic())
							pVehicle->AddToMovingList();
					}
				}
				break;
			}
			case CLEANUP_CHAR:
			{
								// Access raw storage through the C store or pool API
				CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), m_sEntities[i].id));
								if (pPed) {
					if (pPed->bIsStaticWaitingForCollision) {
						pPed->bIsStaticWaitingForCollision = false;
						if (!pPed->GetIsStatic())
							pPed->AddToMovingList();
					}
				}
				break;
			}
			case CLEANUP_OBJECT:
			{
								// Access raw storage through the C store or pool API
				CObject* pObject = ((CObject*)CPool_GetAt(Pools_GetObjectPool(), m_sEntities[i].id));
								if (pObject) {
					if (pObject->bIsStaticWaitingForCollision) {
						pObject->bIsStaticWaitingForCollision = false;
						if (!pObject->GetIsStatic())
							pObject->AddToMovingList();
					}
				}
				break;
			}
			default:
				break;
			}
			m_sEntities[i].id = 0;
			m_sEntities[i].type = CLEANUP_UNUSED;
			m_nCount--;
		}
	}
}

void CMissionCleanup::CheckIfCollisionHasLoadedForMissionObjects()
{
	for (int i = 0; i < MAX_CLEANUP; i++) {
		switch (m_sEntities[i].type) {
		case CLEANUP_CAR:
		{
						// Access raw storage through the C store or pool API
			CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), m_sEntities[i].id));
						if (pVehicle) {
				if (pVehicle->bIsStaticWaitingForCollision) {
					if (ColStore_HasCollisionLoaded(pVehicle->GetPosition())) {
						pVehicle->bIsStaticWaitingForCollision = false;
							if (!pVehicle->GetIsStatic())
								pVehicle->AddToMovingList();
					}
				}
			}
			break;
		}
		case CLEANUP_CHAR:
		{
						// Access raw storage through the C store or pool API
			CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), m_sEntities[i].id));
						if (pPed) {
				if (pPed->bIsStaticWaitingForCollision) {
					if (ColStore_HasCollisionLoaded(pPed->GetPosition())) {
						pPed->bIsStaticWaitingForCollision = false;
						if (!pPed->GetIsStatic())
							pPed->AddToMovingList();
					}
				}
			}
			break;
		}
		case CLEANUP_OBJECT:
		{
						// Access raw storage through the C store or pool API
			CObject* pObject = ((CObject*)CPool_GetAt(Pools_GetObjectPool(), m_sEntities[i].id));
						if (pObject) {
				if (pObject->bIsStaticWaitingForCollision) {
					if (ColStore_HasCollisionLoaded(pObject->GetPosition())) {
						pObject->bIsStaticWaitingForCollision = false;
						if (!pObject->GetIsStatic())
							pObject->AddToMovingList();
					}
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

void CMissionCleanup::Process()
{
	Population_allRandomPedsType = -1; // rouz edit (ChatGPT)
	Population_pedDensityMultiplier = 1.0f; // rouz edit (ChatGPT)
	CarCtrl_CarDensityMultiplier = 1.0f;
	CPed::nThreatReactionRangeMultiplier = 1;
	CPed::nEnterCarRangeMultiplier = 1;
	FindPlayerPed()->m_pWanted->m_fCrimeSensitivity = 1.0f;
	RoadBlocks_ClearScriptRoadBlocks();
	PedRoutes_Initialise();
	if (!World_Players[World_PlayerInFocus].m_pRemoteVehicle)
		TheCamera.Restore();
	TheCamera.SetWideScreenOff();
	SpecialFX_bLiftCam = false;
	SpecialFX_bVideoCam = false;
	TimeCycle_StopExtraColour(0);
	for (int i = 0; i < MISSION_AUDIO_SLOTS; i++)
		DMAudio_ClearMissionAudio(i);
	Weather_ReleaseWeather();
	for (int i = 0; i < NUM_OF_SPECIAL_CHARS; i++)
		Streaming_SetMissionDoesntRequireSpecialChar(i); // rouz edit (ChatGPT)
	for (int i = 0; i < NUM_OF_CUTSCENE_OBJECTS; i++)
		Streaming_SetMissionDoesntRequireModel(MI_CUTOBJ01 + i); // rouz edit (ChatGPT)
	Streaming_ms_disableStreaming = false;
	Hud_m_ItemToFlash = -1;
	Hud_SetHelpMessage(nil, false);
	OnscreenTimer_state.m_bDisabled = false;
	TheScripts_RemoveScriptTextureDictionary();
	World_Players[0].m_pPed->m_pWanted->m_bIgnoredByCops = false;
	World_Players[0].m_pPed->m_pWanted->m_bIgnoredByEveryone = false;
	World_Players[0].MakePlayerSafe(false);
	World_Players[0].m_pPed->m_nFadeDrunkenness = 1;
	World_Players[0].m_pPed->m_nDrunkCountdown = 0;
	CPad::GetPad(0)->SetDrunkInputDelay(0);
	World_Players[0].m_bDriveByAllowed = true;
	DMAudio_ShutUpPlayerTalking(FALSE);
	CVehicle::bDisableRemoteDetonation = false;
	CVehicle::bDisableRemoteDetonationOnContact = false;
	GameLogic_ClearShortCut();
	TheScripts_RiotIntensity = 0;
	TheScripts_StoreVehicleIndex = -1;
	TheScripts_StoreVehicleWasRandom = true;
	TheScripts_UpsideDownCars.Init();
	TheScripts_StuckCars.Init();
	for (int i = 0; i < MAX_CLEANUP; i++){
		if (m_sEntities[i].type == CLEANUP_UNUSED)
			continue;
		switch (m_sEntities[i].type) {
		case CLEANUP_CAR:
		{
						// Access raw storage through the C store or pool API
			CVehicle* v = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), m_sEntities[i].id));
						if (v)
				TheScripts_CleanUpThisVehicle(v);
			break;
		}
		case CLEANUP_CHAR:
		{
						// Access raw storage through the C store or pool API
			CPed* p = ((CPed*)CPool_GetAt(Pools_GetPedPool(), m_sEntities[i].id));
						if (p)
				TheScripts_CleanUpThisPed(p);
			break;
		}
		case CLEANUP_OBJECT:
		{
						// Access raw storage through the C store or pool API
			CObject* o = ((CObject*)CPool_GetAt(Pools_GetObjectPool(), m_sEntities[i].id));
						if (o)
				TheScripts_CleanUpThisObject(o);
			break;
		}
		default:
			break;
		}
		RemoveEntityFromList(m_sEntities[i].id, m_sEntities[i].type);
	}
#ifdef SECUROM
	if ((myrand() & 3) == 2){
		// if pirated game
		Weather_ForceHurricaneWeather();
	}
#endif
}

/* NB: CUpsideDownCarCheck is not used by actual script at all
 * It has a weird usage: AreAnyCarsUpsideDown would fail any mission
 * just like death or arrest. */

void CUpsideDownCarCheck::Init()
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		m_sCars[i].m_nVehicleIndex = -1;
		m_sCars[i].m_nUpsideDownTimer = 0;
	}
}

bool CUpsideDownCarCheck::IsCarUpsideDown(int32 id)
{
		// Access raw storage through the C store or pool API
	CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), id));
		return IsCarUpsideDown(pVehicle);
}

bool CUpsideDownCarCheck::IsCarUpsideDown(CVehicle* pVehicle)
{
	assert(pVehicle);
	// Delegate the check that the car is upside down to the C implementation
	return ScriptMath_IsUpsideDown(pVehicle->GetUp().z,
		pVehicle->GetMoveSpeed().x, pVehicle->GetMoveSpeed().y, pVehicle->GetMoveSpeed().z,
		pVehicle->GetTurnSpeed().x, pVehicle->GetTurnSpeed().y, pVehicle->GetTurnSpeed().z,
		UPSIDEDOWN_UP_THRESHOLD, UPSIDEDOWN_MOVE_SPEED_THRESHOLD, UPSIDEDOWN_TURN_SPEED_THRESHOLD);
}

void CUpsideDownCarCheck::UpdateTimers()
{
	uint32 timeStep = Timer_GetTimeStepInMilliseconds();
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
				// Access raw storage through the C store or pool API
		CVehicle* v = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), m_sCars[i].m_nVehicleIndex));
				if (v){
			if (IsCarUpsideDown(m_sCars[i].m_nVehicleIndex))
				m_sCars[i].m_nUpsideDownTimer += timeStep;
			else
				m_sCars[i].m_nUpsideDownTimer = 0;
		}else{
			m_sCars[i].m_nVehicleIndex = -1;
			m_sCars[i].m_nUpsideDownTimer = 0;
		}
	}
}

bool CUpsideDownCarCheck::AreAnyCarsUpsideDown()
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex >= 0 && m_sCars[i].m_nUpsideDownTimer > UPSIDEDOWN_TIMER_THRESHOLD)
			return true;
	}
	return false;
}

void CUpsideDownCarCheck::AddCarToCheck(int32 id)
{
	uint16 index = 0;
	while (index < MAX_UPSIDEDOWN_CAR_CHECKS && m_sCars[index].m_nVehicleIndex >= 0)
		index++;
#ifdef FIX_BUGS
	if (index >= MAX_UPSIDEDOWN_CAR_CHECKS)
		return;
#endif
	m_sCars[index].m_nVehicleIndex = id;
	m_sCars[index].m_nUpsideDownTimer = 0;
}

void CUpsideDownCarCheck::RemoveCarFromCheck(int32 id)
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id){
			m_sCars[i].m_nVehicleIndex = -1;
			m_sCars[i].m_nUpsideDownTimer = 0;
		}
	}
}

bool CUpsideDownCarCheck::HasCarBeenUpsideDownForAWhile(int32 id)
{
	for (int i = 0; i < MAX_UPSIDEDOWN_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id)
			return m_sCars[i].m_nUpsideDownTimer > UPSIDEDOWN_TIMER_THRESHOLD;
	}
	return false;
}

void stuck_car_data::Reset()
{
	m_nVehicleIndex = -1;
	m_vecPos = CVector(-5000.0f, -5000.0f, -5000.0f);
	m_nLastCheck = -1;
	m_fRadius = 0.0f;
	m_nStuckTime = 0;
	m_bStuck = false;
}

void CStuckCarCheck::Init()
{
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++) {
		m_sCars[i].Reset();
	}
}

void CStuckCarCheck::Process()
{
	uint32 timer = (*Timer_GetTimeInMilliseconds());
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex < 0)
			continue;
		if (timer <= m_sCars[i].m_nStuckTime + m_sCars[i].m_nLastCheck)
			continue;
				// Access raw storage through the C store or pool API
		CVehicle* pv = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), m_sCars[i].m_nVehicleIndex));
				if (!pv){
			m_sCars[i].Reset();
			continue;
		}
		// Delegate the distance to the C implementation
		float distance = ScriptMath_Distance3D(pv->GetPosition().x, pv->GetPosition().y, pv->GetPosition().z, m_sCars[i].m_vecPos.x, m_sCars[i].m_vecPos.y, m_sCars[i].m_vecPos.z);
		m_sCars[i].m_bStuck = distance < m_sCars[i].m_fRadius;
		m_sCars[i].m_vecPos = pv->GetPosition();
		m_sCars[i].m_nLastCheck = timer;
	}
}

void CStuckCarCheck::AddCarToCheck(int32 id, float radius, uint32 time)
{
		// Access raw storage through the C store or pool API
	CVehicle* pv = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), id));
		if (!pv)
		return;
	int index = 0;
	while (index < MAX_STUCK_CAR_CHECKS && m_sCars[index].m_nVehicleIndex >= 0)
		index++;
#ifdef FIX_BUGS
	if (index >= MAX_STUCK_CAR_CHECKS)
		return;
#endif
	m_sCars[index].m_nVehicleIndex = id;
	m_sCars[index].m_vecPos = pv->GetPosition();
	m_sCars[index].m_nLastCheck = (*Timer_GetTimeInMilliseconds());
	m_sCars[index].m_fRadius = radius;
	m_sCars[index].m_nStuckTime = time;
	m_sCars[index].m_bStuck = false;
}

void CStuckCarCheck::RemoveCarFromCheck(int32 id)
{
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id){
			m_sCars[i].Reset();
		}
	}
}

bool CStuckCarCheck::HasCarBeenStuckForAWhile(int32 id)
{
	for (int i = 0; i < MAX_STUCK_CAR_CHECKS; i++){
		if (m_sCars[i].m_nVehicleIndex == id)
			return m_sCars[i].m_bStuck;
	}
	return false;
}

static_assert(SCRIPTBC_ARG_INT32 == ARGUMENT_INT32 && SCRIPTBC_ARG_GLOBALVAR == ARGUMENT_GLOBALVAR && SCRIPTBC_ARG_LOCALVAR == ARGUMENT_LOCALVAR &&
	SCRIPTBC_ARG_INT8 == ARGUMENT_INT8 && SCRIPTBC_ARG_INT16 == ARGUMENT_INT16 && SCRIPTBC_ARG_FLOAT == ARGUMENT_FLOAT && SCRIPTBC_ARG_END == ARGUMENT_END &&
	SCRIPTBC_VAR_LOCAL == VAR_LOCAL && SCRIPTBC_VAR_GLOBAL == VAR_GLOBAL, "Script byte code constants differ from the C decoder");

void CRunningScript::CollectParameters(uint32* pIp, int16 total)
{
	// Delegate to the C implementation
	int result = ScriptBC_CollectParameters(TheScripts_ScriptSpace, TheScripts_GetSizeOfVariableSpace(), m_anLocalVariables, ARRAY_SIZE(m_anLocalVariables), pIp, total, ScriptParams);
	script_assert(result == SCRIPTBC_OK);
}

int32 CRunningScript::CollectNextParameterWithoutIncreasingPC(uint32 ip)
{
	// Delegate to the C implementation
	int result;
	int32 value = ScriptBC_CollectNextParameterWithoutIncreasingPC(TheScripts_ScriptSpace, m_anLocalVariables, ip, &result);
	script_assert(result == SCRIPTBC_OK);
	return value;
}

void CRunningScript::StoreParameters(uint32* pIp, int16 number)
{
	// Delegate to the C implementation
	int result = ScriptBC_StoreParameters(TheScripts_ScriptSpace, m_anLocalVariables, pIp, number, ScriptParams);
	script_assert(result == SCRIPTBC_OK);
}

int32 *CRunningScript::GetPointerToScriptVariable(uint32* pIp, int16 type)
{
	// Delegate to the C implementation
	int result;
	int32 *ptr = ScriptBC_GetPointerToVariable(TheScripts_ScriptSpace, m_anLocalVariables, pIp, type, &result);
	script_assert(result == SCRIPTBC_OK);
	return ptr;
}

void CRunningScript::Init()
{
	strcpy(m_abScriptName, "noname");
	next = prev = nil;
	SetIP(0);
	for (int i = 0; i < MAX_STACK_DEPTH; i++)
		m_anStack[i] = 0;
	m_nStackPointer = 0;
	m_nWakeTime = 0;
	m_bIsActive = false;
	m_bCondResult = false;
	m_bIsMissionScript = false;
	m_bSkipWakeTime = false;
	for (int i = 0; i < NUM_LOCAL_VARS + NUM_TIMERS; i++)
		m_anLocalVariables[i] = 0;
	m_nAndOrState = 0;
	m_bNotFlag = false;
	m_bDeatharrestEnabled = true;
	m_bDeatharrestExecuted = false;
	m_bMissionFlag = false;
}

#ifdef USE_DEBUG_SCRIPT_LOADER
int TheScripts_ScriptToLoad = 0;

int TheScripts_OpenScript()
{
	FileMgr_ChangeDir("\\");
	switch (TheScripts_ScriptToLoad) {
	case 0: return FileMgr_OpenFile("data\\main.scm", "rb");
	case 1: return FileMgr_OpenFile("data\\freeroam_miami.scm", "rb");
	case 2: return FileMgr_OpenFile("data\\main_d.scm", "rb");
	}
	return FileMgr_OpenFile("data\\main.scm", "rb");
}
#endif

void TheScripts_Init()
{
	for (int i = 0; i < SIZE_SCRIPT_SPACE; i++)
		TheScripts_ScriptSpace[i] = 0;
	TheScripts_pActiveScripts = TheScripts_pIdleScripts = nil;
	for (int i = 0; i < MAX_NUM_SCRIPTS; i++){
		TheScripts_ScriptsArray[i].Init();
		TheScripts_ScriptsArray[i].AddScriptToList(&TheScripts_pIdleScripts);
	}
	TheScripts_MissionCleanUp.Init();
	TheScripts_UpsideDownCars.Init();
	TheScripts_StuckCars.Init();
#ifdef USE_DEBUG_SCRIPT_LOADER
	// glfwGetKey doesn't work because of Game_Initialise is blocking
	CPad::UpdatePads();
	if(Input_GetChar('G')) TheScripts_ScriptToLoad = 0; // rouz edit (ChatGPT)
	if(Input_GetChar('R')) TheScripts_ScriptToLoad = 1; // rouz edit (ChatGPT)
	if(Input_GetChar('D')) TheScripts_ScriptToLoad = 2; // rouz edit (ChatGPT)

	int mainf = TheScripts_OpenScript();
#else
	FileMgr_SetDir("data");
	int mainf = FileMgr_OpenFile("main.scm", "rb");
#endif
	FileMgr_Read(mainf, (char*)TheScripts_ScriptSpace, SIZE_MAIN_SCRIPT);
	FileMgr_CloseFile(mainf);
	FileMgr_SetDir("");
	TheScripts_StoreVehicleIndex = -1;
	TheScripts_StoreVehicleWasRandom = true;
	TheScripts_OnAMissionFlag = 0;
	TheScripts_LastMissionPassedTime = (uint32)-1;
	TheScripts_LastRandomPedId = -1;
	for (int i = 0; i < MAX_NUM_USED_OBJECTS; i++){
		memset(&TheScripts_UsedObjectArray[i].name, 0, sizeof(TheScripts_UsedObjectArray[i].name));
		TheScripts_UsedObjectArray[i].index = 0;
	}
	TheScripts_NumberOfUsedObjects = 0;
	TheScripts_ReadObjectNamesFromScript();
	TheScripts_UpdateObjectIndices();
	TheScripts_bAlreadyRunningAMissionScript = false;
	TheScripts_bUsingAMultiScriptFile = true;
	for (int i = 0; i < MAX_NUM_MISSION_SCRIPTS; i++)
		TheScripts_MultiScriptArray[i] = 0;
	TheScripts_NumberOfExclusiveMissionScripts = 0;
	TheScripts_NumberOfMissionScripts = 0;
	TheScripts_LargestMissionScriptSize = 0;
	TheScripts_MainScriptSize = 0;
	TheScripts_ReadMultiScriptFileOffsetsFromScript();
	TheScripts_FailCurrentMission = 0;
	TheScripts_DbgFlag = false;
	TheScripts_NumScriptDebugLines = 0;
	TheScripts_RiotIntensity = 0;
	TheScripts_bPlayerHasMetDebbieHarry = false;
	TheScripts_bPlayerIsInTheStatium = false;
	for (int i = 0; i < MAX_NUM_SCRIPT_SPHERES; i++){
		TheScripts_ScriptSphereArray[i].m_bInUse = false;
		TheScripts_ScriptSphereArray[i].m_Index = 1;
		TheScripts_ScriptSphereArray[i].m_Id = 0;
		TheScripts_ScriptSphereArray[i].m_vecCenter = CVector(0.0f, 0.0f, 0.0f);
		TheScripts_ScriptSphereArray[i].m_fRadius = 0.0f;
	}
	for (int i = 0; i < MAX_NUM_INTRO_TEXT_LINES; i++){
		TheScripts_IntroTextLines[i].Reset();
	}
	TheScripts_NumberOfIntroTextLinesThisFrame = 0;
	TheScripts_UseTextCommands = 0;
	for (int i = 0; i < MAX_NUM_INTRO_RECTANGLES; i++){
		TheScripts_IntroRectangles[i].m_bIsUsed = false;
		TheScripts_IntroRectangles[i].m_bBeforeFade = false;
		TheScripts_IntroRectangles[i].m_nTextureId = -1;
		TheScripts_IntroRectangles[i].m_sRect = Rect_Make(0.0f, 0.0f, 0.0f, 0.0f);
		TheScripts_IntroRectangles[i].m_sColor = CRGBA(255, 255, 255, 255);
	}
	TheScripts_NumberOfIntroRectanglesThisFrame = 0;
	TheScripts_RemoveScriptTextureDictionary();
	for (int i = 0; i < MAX_NUM_BUILDING_SWAPS; i++){
		TheScripts_BuildingSwapArray[i].m_pBuilding = nil;
		TheScripts_BuildingSwapArray[i].m_nNewModel = -1;
		TheScripts_BuildingSwapArray[i].m_nOldModel = -1;
	}
	for (int i = 0; i < MAX_NUM_INVISIBILITY_SETTINGS; i++)
		TheScripts_InvisibilitySettingArray[i] = nil;

#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	TheScripts_LogAfterScriptInitializing();
#endif
#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
	UsingMobileScript = false;
	AlreadySavedGame = false;
#endif
}

void TheScripts_RemoveScriptTextureDictionary()
{
	for (int i = 0; i < ARRAY_SIZE(TheScripts_ScriptSprites); i++)
		([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(TheScripts_ScriptSprites[i]));
	// Use the complete C texture dictionary owner with the original live arguments
	int slot = TxdStore_FindTxdSlot("script");
	if (slot != -1)
		// Use the complete C texture dictionary owner with the original live arguments
		TxdStore_RemoveTxd(slot);
}

void CRunningScript::RemoveScriptFromList(CRunningScript** ppScript)
{
	// Delegate to the C implementation
	DList_Remove((void**)ppScript, this, offsetof(CRunningScript, prev), offsetof(CRunningScript, next));
}

void CRunningScript::AddScriptToList(CRunningScript** ppScript)
{
	// Delegate to the C implementation
	DList_PushFront((void**)ppScript, this, offsetof(CRunningScript, prev), offsetof(CRunningScript, next));
}

CRunningScript* TheScripts_StartNewScript(uint32 ip)
{
	CRunningScript* pNew = TheScripts_pIdleScripts;
	script_assert(pNew);
	pNew->RemoveScriptFromList(&TheScripts_pIdleScripts);
	pNew->Init();
	pNew->SetIP(ip);
	pNew->AddScriptToList(&TheScripts_pActiveScripts);
	pNew->m_bIsActive = true;
	return pNew;
}

void TheScripts_Process()
{
	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;
	TheScripts_CommandsExecuted = 0;
	TheScripts_ScriptsUpdated = 0;
	float timeStep = Timer_GetTimeStepInMilliseconds();
	TheScripts_UpsideDownCars.UpdateTimers();
	TheScripts_StuckCars.Process();
	TheScripts_MissionCleanUp.CheckIfCollisionHasLoadedForMissionObjects();
	TheScripts_DrawScriptSpheres();
	if (TheScripts_FailCurrentMission)
		--TheScripts_FailCurrentMission;
	if (TheScripts_UseTextCommands){
		for (int i = 0; i < MAX_NUM_INTRO_TEXT_LINES; i++)
			TheScripts_IntroTextLines[i].Reset();
		TheScripts_NumberOfIntroTextLinesThisFrame = 0;
		for (int i = 0; i < MAX_NUM_INTRO_RECTANGLES; i++){
			TheScripts_IntroRectangles[i].m_bIsUsed = false;
			TheScripts_IntroRectangles[i].m_bBeforeFade = false;
		}
		TheScripts_NumberOfIntroRectanglesThisFrame = 0;
		if (TheScripts_UseTextCommands == 1)
			TheScripts_UseTextCommands = 0;
	}

#ifdef MISSION_REPLAY
	static uint32 TimeToWaitTill;
	static bool AlreadyResetHealth;
	switch (AllowMissionReplay) {
	case MISSION_RETRY_STAGE_START_PROCESSING:
		AllowMissionReplay = MISSION_RETRY_STAGE_WAIT_FOR_DELAY;
		TimeToWaitTill = (*Timer_GetTimeInMilliseconds()) + (AddExtraDeathDelay() > 1000 ? 4000 : 2500);
		break;
	case MISSION_RETRY_STAGE_WAIT_FOR_DELAY:
		if (TimeToWaitTill < (*Timer_GetTimeInMilliseconds()))
			AllowMissionReplay = MISSION_RETRY_STAGE_WAIT_FOR_MENU;
		break;
	case MISSION_RETRY_STAGE_WAIT_FOR_MENU:
		AllowMissionReplay = MISSION_RETRY_STAGE_WAIT_FOR_USER;
		RetryMission(MISSION_RETRY_TYPE_SUGGEST_TO_PLAYER);
		break;
	case MISSION_RETRY_STAGE_START_RESTARTING:
		AllowMissionReplay = MISSION_RETRY_STAGE_WAIT_FOR_TIMER_AFTER_RESTART;
		AlreadyResetHealth = false;
		TimeToWaitTill = (*Timer_GetTimeInMilliseconds()) + 500;
		break;
	case MISSION_RETRY_STAGE_WAIT_FOR_TIMER_AFTER_RESTART:
		if (!AlreadyResetHealth) {
			AlreadyResetHealth = true;
			CPlayerPed* pPlayerPed = FindPlayerPed();
			if (pPlayerPed) {
				CPlayerInfo* pPlayerInfo = pPlayerPed->GetPlayerInfoForThisPlayerPed();
				if (pPlayerInfo)
					pPlayerPed->m_fHealth = pPlayerInfo->m_nMaxHealth;
			}
		}
		if (TimeToWaitTill < (*Timer_GetTimeInMilliseconds())) {
			AllowMissionReplay = MISSION_RETRY_STAGE_NORMAL;
			return;
		}
		break;
	}
	if (WaitForMissionActivate) {
		if (WaitForMissionActivate > (*Timer_GetTimeInMilliseconds()))
			return;
		WaitForMissionActivate = 0;
		WaitForSave = (*Timer_GetTimeInMilliseconds()) + 3000;
	}
	if (WaitForSave && WaitForSave > (*Timer_GetTimeInMilliseconds()))
		WaitForSave = 0;
#endif

#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	TheScripts_LogBeforeScriptProcessing();
#endif

	CRunningScript* script = TheScripts_pActiveScripts;
	while (script != nil){
		CRunningScript* next = script->GetNext();
		++TheScripts_ScriptsUpdated;
		script->UpdateTimers(timeStep);
		script->Process();
		script = next;
		if (script && !script->m_bIsActive)
			script = nil;
	}
	TheScripts_DbgFlag = false;

#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	TheScripts_LogAfterScriptProcessing();
#endif
}

CRunningScript* TheScripts_StartTestScript()
{
	return TheScripts_StartNewScript(0);
}

bool TheScripts_IsPlayerOnAMission()
{
	return TheScripts_OnAMissionFlag && *(int32*)&TheScripts_ScriptSpace[TheScripts_OnAMissionFlag] == 1;
}

void CRunningScript::Process()
{
#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	LogOnStartProcessing();
#endif
	if (m_bIsMissionScript)
		DoDeatharrestCheck();
	if (m_bMissionFlag && TheScripts_FailCurrentMission == 1 && m_nStackPointer == 1)
		SetIP(m_anStack[--m_nStackPointer]);
	if ((*Timer_GetTimeInMilliseconds()) >= m_nWakeTime){
		while (!ProcessOneCommand())
			;
		return;
	}
	if (!m_bSkipWakeTime)
		return;
	if (!CPad::GetPad(0)->GetCrossJustDown())
		return;
	m_nWakeTime = 0;
	for (int i = 0; i < NUMBIGMESSAGES; i++){
		if (Messages_BIGMessages[i].m_Stack[0].m_pText != nil)
			Messages_BIGMessages[i].m_Stack[0].m_nStartTime = 0;
	}
	if (Messages_BriefMessages[0].m_pText != nil)
		Messages_BriefMessages[0].m_nStartTime = 0;
}

int8 CRunningScript::ProcessOneCommand()
{
	int8 retval = -1;
	++TheScripts_CommandsExecuted;
	// Delegate the reading and grouping of the command to the C implementation
	int notFlag;
	int32 command = ScriptBC_ReadCommand(TheScripts_ScriptSpace, &m_nIp, &notFlag);
	m_bNotFlag = notFlag;
#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
	LogBeforeProcessingCommand(command);
#endif
	// Delegate the timed additions and the absolute values to the C implementation
	if (ScriptBC_IsTimedOpCommand(command)) {
		int result = ScriptBC_TimedOp(command, TheScripts_ScriptSpace, TheScripts_GetSizeOfVariableSpace(), m_anLocalVariables, ARRAY_SIZE(m_anLocalVariables), &m_nIp, ScriptParams, (*Timer_GetTimeStep()));
		script_assert(result == SCRIPTBC_OK);
		retval = 0;
	} else
	// Delegate the operations between two script variables to the C implementation
	if (ScriptBC_IsVariableOpCommand(command)) {
		int result = ScriptBC_VariableOp(command, TheScripts_ScriptSpace, m_anLocalVariables, &m_nIp);
		script_assert(result == SCRIPTBC_OK);
		retval = 0;
	} else
	switch (ScriptBC_CommandGroup(command)) {
	case 0:
		retval = ProcessCommands0To99(command);
		break;
	case 1:
		retval = ProcessCommands100To199(command);
		break;
	case 2:
		retval = ProcessCommands200To299(command);
		break;
	case 3:
		retval = ProcessCommands300To399(command);
		break;
	case 4:
		retval = ProcessCommands400To499(command);
		break;
	case 5:
		retval = ProcessCommands500To599(command);
		break;
	case 6:
		retval = ProcessCommands600To699(command);
		break;
	case 7:
		retval = ProcessCommands700To799(command);
		break;
	case 8:
		retval = ProcessCommands800To899(command);
		break;
	case 9:
		retval = ProcessCommands900To999(command);
		break;
	case 10:
		retval = ProcessCommands1000To1099(command);
		break;
	case 11:
		retval = ProcessCommands1100To1199(command);
		break;
	case 12:
		retval = ProcessCommands1200To1299(command);
		break;
	case 13:
		retval = ProcessCommands1300To1399(command);
		break;
	case 14:
		retval = ProcessCommands1400To1499(command);
		break;
	default:
		break;
	}
#ifdef USE_MISSION_REPLAY_OVERRIDE_FOR_NON_MOBILE_SCRIPT
	if (!AlreadySavedGame) // we need to ignore first "fake" command which actually just saves the game
#endif
	{
#ifdef USE_ADVANCED_SCRIPT_DEBUG_OUTPUT
		LogAfterProcessingCommand(command);
#elif defined USE_BASIC_SCRIPT_DEBUG_OUTPUT
		if (m_bMissionFlag) {
			char tmp[128];
			sprintf(tmp, "Comm %d Cmp %d", command, m_bCondResult);
			Debug_DebugAddText(tmp);
		}
#endif
	}
	return retval;
}

int8 CRunningScript::ProcessCommands0To99(int32 command)
{
	float *fScriptVar1;
	int *nScriptVar1;

	// Delegate the comparisons of variables and numbers to the C implementation
	if (ScriptBC_IsCompareCommand(command)) {
		int flag;
		int result = ScriptBC_Compare(command, TheScripts_ScriptSpace, TheScripts_GetSizeOfVariableSpace(), m_anLocalVariables, ARRAY_SIZE(m_anLocalVariables), &m_nIp, ScriptParams, &flag);
		script_assert(result == SCRIPTBC_OK);
		UpdateCompareFlag(flag != 0);
		return 0;
	}

	// Delegate the arithmetic on script variables to the C implementation
	if (command >= COMMAND_SET_VAR_INT && command <= COMMAND_DIV_FLOAT_LVAR_BY_VAL) {
		int result = ScriptBC_Arithmetic(command, TheScripts_ScriptSpace, TheScripts_GetSizeOfVariableSpace(), m_anLocalVariables, ARRAY_SIZE(m_anLocalVariables), &m_nIp, ScriptParams);
		script_assert(result == SCRIPTBC_OK);
		return 0;
	}

	switch (command) {
	case COMMAND_NOP:
		return 0;
	case COMMAND_WAIT:
		CollectParameters(&m_nIp, 1);
		m_nWakeTime = (*Timer_GetTimeInMilliseconds()) + ScriptParams[0];
		m_bSkipWakeTime = false;
		return 1;
	case COMMAND_GOTO:
		CollectParameters(&m_nIp, 1);
		// Delegate to the C implementation
		ScriptBC_Jump(&m_nIp, ScriptParams[0], SIZE_MAIN_SCRIPT);
		/* Known issue: GOTO to 0. It might have been "better" to use > instead of >= */
		/* simply because it never makes sense to jump to start of the script */
		/* but jumping to start of a custom mission is an issue for simple mission-like scripts */
		/* However, it's not an issue for actual mission scripts, because they follow a structure */
		/* and never start with a loop. */
		return 0;
	case COMMAND_SHAKE_CAM:
		CollectParameters(&m_nIp, 1);
		CamShakeNoPos(&TheCamera, ScriptParams[0] / 1000.0f);
		return 0;
	//case COMMAND_IS_INT_VAR_NOT_EQUAL_TO_NUMBER:
	//case COMMAND_IS_INT_LVAR_NOT_EQUAL_TO_NUMBER:
	//case COMMAND_IS_INT_VAR_NOT_EQUAL_TO_INT_VAR:
	//case COMMAND_IS_INT_LVAR_NOT_EQUAL_TO_INT_LVAR:
	//case COMMAND_IS_INT_VAR_NOT_EQUAL_TO_INT_LVAR:
	//case COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_NUMBER:
	//case COMMAND_IS_FLOAT_LVAR_NOT_EQUAL_TO_NUMBER:
	//case COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_FLOAT_VAR:
	//case COMMAND_IS_FLOAT_LVAR_NOT_EQUAL_TO_FLOAT_LVAR:
	//case COMMAND_IS_FLOAT_VAR_NOT_EQUAL_TO_FLOAT_LVAR:
	/*
	case COMMAND_GOTO_IF_TRUE:
		CollectParameters(&m_nIp, 1);
		if (m_bCondResult)
			SetIP(ScriptParams[0] >= 0 ? ScriptParams[0] : SIZE_MAIN_SCRIPT - ScriptParams[0]);
		return 0;
	*/
	case COMMAND_GOTO_IF_FALSE:
		CollectParameters(&m_nIp, 1);
		if (!m_bCondResult)
			ScriptBC_Jump(&m_nIp, ScriptParams[0], SIZE_MAIN_SCRIPT);
		/* Check COMMAND_GOTO note. */
		return 0;
	case COMMAND_TERMINATE_THIS_SCRIPT:
		if (m_bMissionFlag)
			TheScripts_bAlreadyRunningAMissionScript = false;
		RemoveScriptFromList(&TheScripts_pActiveScripts);
		AddScriptToList(&TheScripts_pIdleScripts);
		m_bIsActive = false;
#ifdef MISSION_REPLAY
		if (m_bMissionFlag) {
			CPlayerInfo* pPlayerInfo = &World_Players[World_PlayerInFocus];
#if 0 // makeing autosave is pointless and is a bit buggy
			if (pPlayerInfo->m_pPed->GetPedState() != PED_DEAD && pPlayerInfo->m_WBState == WBSTATE_PLAYING && !m_bDeatharrestExecuted)
				SaveGameForPause(SAVE_TYPE_QUICKSAVE);
#endif
			oldTargetX = oldTargetY = 0.0f;
			if (AllowMissionReplay == MISSION_RETRY_STAGE_WAIT_FOR_SCRIPT_TO_TERMINATE)
				AllowMissionReplay = MISSION_RETRY_STAGE_START_PROCESSING;
			// I am fairly sure they forgot to set return value here
		}
#endif
		return 1;
	case COMMAND_START_NEW_SCRIPT:
	{
		CollectParameters(&m_nIp, 1);
		script_assert(ScriptParams[0] >= 0);
		CRunningScript* pNew = TheScripts_StartNewScript(ScriptParams[0]);
		pNew->m_bIsActive = true;
		// Delegate the copying of the arguments to the C implementation
		ScriptBC_CopyArgumentsToLocals(TheScripts_ScriptSpace, m_anLocalVariables, pNew->m_anLocalVariables, &m_nIp);
		return 0;
	}
	case COMMAND_GOSUB:
		CollectParameters(&m_nIp, 1);
		{
			// Delegate to the C implementation
			int result = ScriptBC_Gosub(m_anStack, &m_nStackPointer, MAX_STACK_DEPTH, &m_nIp, ScriptParams[0], SIZE_MAIN_SCRIPT);
			script_assert(result == SCRIPTBC_OK);
		}
		return 0;
	case COMMAND_RETURN:
	{
		// No more SSU
		int result = ScriptBC_Return(m_anStack, &m_nStackPointer, &m_nIp);
		script_assert(result == SCRIPTBC_OK);
		return 0;
	}
	case COMMAND_LINE:
		CollectParameters(&m_nIp, 6);
		/* Something must have been here */
		return 0;
	case COMMAND_CREATE_PLAYER:
	{
		CollectParameters(&m_nIp, 4);
		int32 index = ScriptParams[0];
		script_assert(index < NUMPLAYERS);
		printf("&&&&&&&&&&&&&Creating player: %d\n", index);
		if (!Streaming_HasModelLoaded(MI_PLAYER)) { // rouz edit (ChatGPT)
			Streaming_RequestSpecialModel(MI_PLAYER, "player", STREAMFLAGS_DONT_REMOVE | STREAMFLAGS_DEPENDENCY);
			Streaming_LoadAllRequestedModels(false);
		}
		CPlayerPed::SetupPlayerPed(index);
		World_Players[index].m_pPed->CharCreatedBy = MISSION_CHAR;
		CPlayerPed::DeactivatePlayerPed(index);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		pos.z += World_Players[index].m_pPed->GetDistanceFromCentreOfMassToBaseOfModel();
		World_Players[index].m_pPed->SetPosition(pos);
		TheScripts_ClearSpaceForMissionEntity(pos, World_Players[index].m_pPed);
		CPlayerPed::ReactivatePlayerPed(index);
		ScriptParams[0] = index;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_GET_PLAYER_COORDINATES:
	{
		CVector pos;
		CollectParameters(&m_nIp, 1);
		if (World_Players[ScriptParams[0]].m_pPed->bInVehicle)
			pos = World_Players[ScriptParams[0]].m_pPed->m_pMyVehicle->GetPosition();
		else
			pos = World_Players[ScriptParams[0]].m_pPed->GetPosition();
		*(CVector*)&ScriptParams[0] = pos;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_PLAYER_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[1];
		int index = ScriptParams[0];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		CPlayerPed* ped = World_Players[index].m_pPed;
		if (ped->bInVehicle) {
			pos.z += ped->m_pMyVehicle->GetDistanceFromCentreOfMassToBaseOfModel();
			ped->m_pMyVehicle->Teleport(pos); // removed dumb stuff that was present here
			TheScripts_ClearSpaceForMissionEntity(pos, ped->m_pMyVehicle);
			return 0;
		}
		pos.z += ped->GetDistanceFromCentreOfMassToBaseOfModel();
		CVector vOldPos = ped->GetPosition();
		ped->Teleport(pos);
		TheScripts_ClearSpaceForMissionEntity(pos, ped);
		if (ped) { // great time to check
			for (int i = 0; i < ped->m_numNearPeds; i++) {
				CPed* pTestedPed = ped->m_nearPeds[i];
				if (!pTestedPed || !IsPedPointerValid(pTestedPed))
					continue;
				if (pTestedPed->m_pedInObjective == ped && pTestedPed->m_objective == OBJECTIVE_FOLLOW_CHAR_IN_FORMATION) {
					CVector vFollowerPos = pTestedPed->GetFormationPosition();
					TheScripts_ClearSpaceForMissionEntity(vFollowerPos, ped);
					bool bFound = false;
					vFollowerPos.z = World_FindGroundZFor3DCoord(vFollowerPos.x, vFollowerPos.y, vFollowerPos.z + 1.0f, &bFound) + 1.0f;
					if (bFound) {
						if (World_GetIsLineOfSightClear(vFollowerPos, ped->GetPosition(), true, false, false, true, false, false)) {
							pTestedPed->Teleport(vFollowerPos);
						}
					}
				}
				else if (pTestedPed->m_leader == ped) {
					CVector vFollowerPos;
					if (pTestedPed->m_pedFormation)
						vFollowerPos = pTestedPed->GetFormationPosition();
					else
						vFollowerPos = ped->GetPosition() + pTestedPed->GetPosition() - vOldPos;
					TheScripts_ClearSpaceForMissionEntity(vFollowerPos, ped);
					bool bFound = false;
					vFollowerPos.z = World_FindGroundZFor3DCoord(vFollowerPos.x, vFollowerPos.y, vFollowerPos.z + 1.0f, &bFound) + 1.0f;
					if (bFound) {
						if (World_GetIsLineOfSightClear(vFollowerPos, ped->GetPosition(), true, false, false, true, false, false)) {
							pTestedPed->Teleport(vFollowerPos);
						}
					}
				}
			}
		}
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_AREA_2D:
	{
		CollectParameters(&m_nIp, 6);
		CPlayerPed* ped = World_Players[ScriptParams[0]].m_pPed;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		if (!ped->bInVehicle)
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, x2, y2));
		else
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, x2, y2));
		if (ScriptParams[5])
			TheScripts_HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (TheScripts_DbgFlag)
			TheScripts_DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
		CPlayerPed* ped = World_Players[ScriptParams[0]].m_pPed;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		if (ped->bInVehicle)
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		else
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (ScriptParams[7])
			TheScripts_HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
		if (TheScripts_DbgFlag)
			TheScripts_DrawDebugCube(x1, y1, z1, x2, y2, z2);
		return 0;
	}
	default:
		script_assert(0);
		break;
	}
	return -1;
}

int8 CRunningScript::ProcessCommands100To199(int32 command)
{
	float *fScriptVar1;
	int *nScriptVar1;
	switch (command) {
	case COMMAND_GENERATE_RANDOM_FLOAT:
	{
		float* ptr = (float*)GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL);
		General_GetRandomNumber();
		General_GetRandomNumber();
		General_GetRandomNumber(); /* To make it EXTRA random! */
#ifdef FIX_BUGS
		*ptr = General_GetRandomNumberInRangeFloat(0.0f, 1.0f);
#else
		*ptr = General_GetRandomNumber() / 65536.0f;
		/* Between 0 and 0.5 on PC (oh well...), never used in original script. */
#endif

		return 0;
	}
	case COMMAND_GENERATE_RANDOM_INT:
		*GetPointerToScriptVariable(&m_nIp, VAR_GLOBAL) = General_GetRandomNumber();
		return 0;
	case COMMAND_CREATE_CHAR:
	{
		CollectParameters(&m_nIp, 5);
		switch (ScriptParams[1]) {
		case MI_COP:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_STREET;
			break;
		case MI_SWAT:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_SWAT;
			break;
		case MI_FBI:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_FBI;
			break;
		case MI_ARMY:
			if (ScriptParams[0] == PEDTYPE_COP)
				ScriptParams[1] = COP_ARMY;
			break;
		case MI_MEDIC:
			if (ScriptParams[0] == PEDTYPE_EMERGENCY)
				ScriptParams[1] = PEDTYPE_EMERGENCY;
			break;
		case MI_FIREMAN:
			if (ScriptParams[0] == PEDTYPE_FIREMAN)
				ScriptParams[1] = PEDTYPE_FIREMAN;
			break;
		default:
			break;
		}
		CPed* ped;
		if (ScriptParams[0] == PEDTYPE_COP) {
			// Allocate the scripted cop without invoking C++ new.
						// Access raw storage through the C store or pool API
			ped = ((CPed*)CPool_New(Pools_GetPedPool()));
						assert(ped);
			std::allocator<CCopPed>().construct((CCopPed*)ped, (eCopType)ScriptParams[1]);
		} else if (ScriptParams[0] == PEDTYPE_EMERGENCY || ScriptParams[0] == PEDTYPE_FIREMAN) {
			// Allocate the scripted emergency ped without invoking C++ new.
						// Access raw storage through the C store or pool API
			ped = ((CPed*)CPool_New(Pools_GetPedPool()));
						assert(ped);
			std::allocator<CEmergencyPed>().construct((CEmergencyPed*)ped, ScriptParams[1]);
		} else {
			// Allocate the scripted civilian ped without invoking C++ new.
						// Access raw storage through the C store or pool API
			ped = ((CPed*)CPool_New(Pools_GetPedPool()));
						assert(ped);
			std::allocator<CCivilianPed>().construct((CCivilianPed*)ped, (ePedType)ScriptParams[0], ScriptParams[1]);
		}
		ped->CharCreatedBy = MISSION_CHAR;
		ped->bRespondsToThreats = false;
		ped->bAllowMedicsToReviveMe = false;
		ped->bIsPlayerFriend = false;
		CVector pos = *(CVector*)&ScriptParams[2];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		pos.z += 1.0f;
		ped->SetPosition(pos);
		ped->SetOrientation(0.0f, 0.0f, 0.0f);
		TheScripts_ClearSpaceForMissionEntity(pos, ped);
		if (m_bIsMissionScript)
			ped->bIsStaticWaitingForCollision = true;
		World_Add(ped);
		ped->m_nZoneLevel = Zones_GetLevelFromPosition(ZonesPosition_FromVector(*(&pos)));
		Population_nTotalMissionPeds++; // rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
		ScriptParams[0] = CPool_GetIndex(Pools_GetPedPool(), ped);
				StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			TheScripts_MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_DELETE_CHAR:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				TheScripts_RemoveThisPed(ped);
		if (m_bIsMissionScript)
			TheScripts_MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_CHAR_WANDER_DIR:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		ped->ClearAll();
		int8 path = ScriptParams[1];
		if (ScriptParams[1] < 0 || ScriptParams[1] > 7)
			// Max number GetRandomNumberInRange returns is max-1
#ifdef FIX_BUGS
			path = General_GetRandomNumberInRangeInt(0, 8);
#else
			path = General_GetRandomNumberInRangeInt(0, 7);
#endif

		ped->SetWanderPath(path);
		return 0;
	}
	//case COMMAND_CHAR_WANDER_RANGE:
	case COMMAND_CHAR_FOLLOW_PATH:
	{
		CollectParameters(&m_nIp, 6);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		if (ped->GetPedState() == PED_ATTACK || ped->GetPedState() == PED_FIGHT || !ped->IsPedInControl())
			return 0;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		float radius = *(float*)&ScriptParams[4];
		eMoveState state;
		switch (ScriptParams[5]) {
		case 0: state = PEDMOVE_WALK; break;
		case 1: state = PEDMOVE_RUN; break;
		default: assert(0);
		}
		ped->ClearAll();
		ped->m_pathNodeTimer = 0;
		ped->SetFollowPath(pos, radius, state, nil, nil, 999999);
		return 0;
	}
	case COMMAND_CHAR_SET_IDLE:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		ped->bScriptObjectiveCompleted = false;
		ped->SetObjective(OBJECTIVE_WAIT_ON_FOOT);
		return 0;
	}
	case COMMAND_GET_CHAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		CVehicle* vehicle;
		CVector pos;
		/* Seems a bit clumsy but I'll leave original flow */
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		if (vehicle)
			pos = vehicle->GetPosition();
		else
			pos = ped->GetPosition();
		*(CVector*)&ScriptParams[0] = pos;
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_CHAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		CVehicle* vehicle;
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		// removed dumb stuff again
		if (!vehicle) {
			pos.z += ped->GetDistanceFromCentreOfMassToBaseOfModel();
			ped->Teleport(pos);
			TheScripts_ClearSpaceForMissionEntity(pos, ped);
			for (int i = 0; i < ped->m_numNearPeds; i++) {
				CPed* pNearPed = ped->m_nearPeds[i];
				if (pNearPed->m_leader == ped) {
					pNearPed->Teleport(pos);
					pNearPed->PositionAnyPedOutOfCollision();
				}
			}
		}
		else {
			pos.z += vehicle->GetDistanceFromCentreOfMassToBaseOfModel();
			vehicle->Teleport(pos);
			TheScripts_ClearSpaceForMissionEntity(pos, vehicle);
		}
		return 0;
	}
	/*
	case COMMAND_IS_CHAR_STILL_ALIVE:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(CPools::GetPedPool(), ScriptParams[0]));
				UpdateCompareFlag(ped && ped->GetPedState() != PED_DEAD && ped->GetPedState() != PED_DIE);
		return 0;
	}
	*/
	case COMMAND_IS_CHAR_IN_AREA_2D:
	{
		CollectParameters(&m_nIp, 6);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		CVehicle* vehicle;
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		if (vehicle)
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, x2, y2));
		else
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, x2, y2));
		if (ScriptParams[5])
			TheScripts_HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (TheScripts_DbgFlag)
			TheScripts_DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		CVehicle* vehicle;
		if (ped->bInVehicle)
			vehicle = ped->m_pMyVehicle;
		else
			vehicle = nil;
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		if (vehicle)
			UpdateCompareFlag(ped->m_pMyVehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		else
			UpdateCompareFlag(ped->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (ScriptParams[7])
			TheScripts_HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
		if (TheScripts_DbgFlag)
			TheScripts_DrawDebugCube(x1, y1, z1, x2, y2, z2);
		return 0;
	}
	case COMMAND_CREATE_CAR:
	{
		CollectParameters(&m_nIp, 4);
		int32 handle;
		if (ModelRegistry_IsBoatModel(ScriptParams[0])) { // rouz edit (ChatGPT)
			// Allocate the scripted boat without invoking C++ new.
						// Access raw storage through the C store or pool API
			CBoat* boat = (CBoat*)((CVehicle*)CPool_New(Pools_GetVehiclePool()));
						assert(boat);
			std::allocator<CBoat>().construct(boat, ScriptParams[0], MISSION_VEHICLE);
			CVector pos = *(CVector*)&ScriptParams[1];
			if (pos.z <= MAP_Z_LOW_LIMIT)
				pos.z = World_FindGroundZForCoord(pos.x, pos.y);
			pos.z += boat->GetDistanceFromCentreOfMassToBaseOfModel();
			boat->SetPosition(pos);
			TheScripts_ClearSpaceForMissionEntity(pos, boat);
			boat->SetStatus(STATUS_ABANDONED);
			boat->bIsLocked = true;
			boat->AutoPilot.m_nCarMission = MISSION_NONE;
			boat->AutoPilot.m_nTempAction = TEMPACT_NONE;
			boat->AutoPilot.m_nCruiseSpeed = boat->AutoPilot.m_fMaxTrafficSpeed = 20.0f;
			if (m_bIsMissionScript)
				boat->bIsStaticWaitingForCollision = true;
			World_Add(boat);
						// Access raw storage through the C store or pool API
			handle = CPool_GetIndex(Pools_GetVehiclePool(), boat);
					}
		else {
			CVehicle* car;

			if (!ModelRegistry_IsBikeModel(ScriptParams[0])) { // rouz edit (ChatGPT)
				// Allocate the scripted car without invoking C++ new.
								// Access raw storage through the C store or pool API
				car = ((CVehicle*)CPool_New(Pools_GetVehiclePool()));
								assert(car);
				std::allocator<CAutomobile>().construct((CAutomobile*)car, ScriptParams[0], MISSION_VEHICLE);
			} else {
				// Allocate the scripted bike without invoking C++ new.
								// Access raw storage through the C store or pool API
				car = ((CVehicle*)CPool_New(Pools_GetVehiclePool()));
								assert(car);
				std::allocator<CBike>().construct((CBike*)car, ScriptParams[0], MISSION_VEHICLE);
				((CBike*)(car))->bIsStanding = true;
			}
			CVector pos = *(CVector*)&ScriptParams[1];
			if (pos.z <= MAP_Z_LOW_LIMIT)
				pos.z = World_FindGroundZForCoord(pos.x, pos.y);
			pos.z += car->GetDistanceFromCentreOfMassToBaseOfModel();
			car->SetPosition(pos);
			TheScripts_ClearSpaceForMissionEntity(pos, car);
			car->SetStatus(STATUS_ABANDONED);
			car->bIsLocked = true;
			CarCtrl_JoinCarWithRoadSystem(car);
			car->AutoPilot.m_nCarMission = MISSION_NONE;
			car->AutoPilot.m_nTempAction = TEMPACT_NONE;
			car->AutoPilot.m_nDrivingStyle = DRIVINGSTYLE_STOP_FOR_CARS;
			car->AutoPilot.m_nCruiseSpeed = car->AutoPilot.m_fMaxTrafficSpeed = 9.0f;
			car->AutoPilot.m_nCurrentLane = car->AutoPilot.m_nNextLane = 0;
			car->bEngineOn = false;
			car->m_nZoneLevel = Zones_GetLevelFromPosition(ZonesPosition_FromVector(*(&pos)));
			car->bHasBeenOwnedByPlayer = true;
			if (m_bIsMissionScript)
				car->bIsStaticWaitingForCollision = true;
			World_Add(car);
						// Access raw storage through the C store or pool API
			handle = CPool_GetIndex(Pools_GetVehiclePool(), car);
					}
		ScriptParams[0] = handle;
		StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			TheScripts_MissionCleanUp.AddEntityToList(handle, CLEANUP_CAR);
		return 0;
	}
	case COMMAND_DELETE_CAR:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				if (car) {
			World_Remove(car);
			World_RemoveReferencesToDeletedObject(car);
			// Destroy and release the scripted vehicle without invoking C++ delete.
			car->~CVehicle();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), car);
					}
		if (m_bIsMissionScript)
			TheScripts_MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_CAR);
		return 0;
	}
	case COMMAND_CAR_GOTO_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		pos.z += car->GetDistanceFromCentreOfMassToBaseOfModel();
		if (CarCtrl_JoinCarWithRoadSystemGotoCoors(car, pos, false))
			car->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
		else
			car->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS;
		car->SetStatus(STATUS_PHYSICS);
		car->bEngineOn = true;
		car->AutoPilot.m_nCruiseSpeed = General_MaxInt(1, car->AutoPilot.m_nCruiseSpeed);
		car->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
		return 0;
	}
	case COMMAND_CAR_WANDER_RANDOMLY:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		CarCtrl_JoinCarWithRoadSystem(car);
		car->AutoPilot.m_nCarMission = MISSION_CRUISE;
		car->bEngineOn = true;
		car->AutoPilot.m_nCruiseSpeed = General_MaxInt(1, car->AutoPilot.m_nCruiseSpeed);
		car->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
		return 0;
	}
	case COMMAND_CAR_SET_IDLE:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		car->AutoPilot.m_nCarMission = MISSION_NONE;
		return 0;
	}
	case COMMAND_GET_CAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		*(CVector*)&ScriptParams[0] = car->GetPosition();
		StoreParameters(&m_nIp, 3);
		return 0;
	}
	case COMMAND_SET_CAR_COORDINATES:
	{
		CollectParameters(&m_nIp, 4);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		pos.z += car->GetDistanceFromCentreOfMassToBaseOfModel();
		car->SetIsStatic(false);
		/* Again weird usage of virtual functions. */
		if (car->IsBoat()) {
			car->Teleport(pos);
			TheScripts_ClearSpaceForMissionEntity(pos, car);
		}
		else {
			car->Teleport(pos);
			TheScripts_ClearSpaceForMissionEntity(pos, car);
			/* May the following be inlined CCarCtrl function? */
			switch (car->AutoPilot.m_nCarMission) {
			case MISSION_CRUISE:
				CarCtrl_JoinCarWithRoadSystem(car);
				break;
			case MISSION_RAMPLAYER_FARAWAY:
			case MISSION_RAMPLAYER_CLOSE:
			case MISSION_BLOCKPLAYER_FARAWAY:
			case MISSION_BLOCKPLAYER_CLOSE:
			case MISSION_BLOCKPLAYER_HANDBRAKESTOP:
				CarCtrl_JoinCarWithRoadSystemGotoCoors(car, FindPlayerCoors(), false);
				break;
			case MISSION_GOTOCOORDS:
			case MISSION_GOTOCOORDS_STRAIGHT:
				CarCtrl_JoinCarWithRoadSystemGotoCoors(car, AutoPilot_ToVector(car->AutoPilot.m_vecDestinationCoors), false);
				break;
			case MISSION_GOTOCOORDS_ACCURATE:
			case MISSION_GOTO_COORDS_STRAIGHT_ACCURATE:
				CarCtrl_JoinCarWithRoadSystemGotoCoors(car, AutoPilot_ToVector(car->AutoPilot.m_vecDestinationCoors), false);
				break;
			case MISSION_RAMCAR_FARAWAY:
			case MISSION_RAMCAR_CLOSE:
			case MISSION_BLOCKCAR_FARAWAY:
			case MISSION_BLOCKCAR_CLOSE:
			case MISSION_BLOCKCAR_HANDBRAKESTOP:
				CarCtrl_JoinCarWithRoadSystemGotoCoors(car, car->AutoPilot.m_pTargetCar->GetPosition(), false);
				break;
			default:
				break;
			}
		}
		return 0;
	}
	/*
	case COMMAND_IS_CAR_STILL_ALIVE:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(CPools::GetVehiclePool(), ScriptParams[0]));
				UpdateCompareFlag(car && car->GetStatus() != STATUS_WRECKED && (car->IsBoat() || !car->bIsInWater));
		return 0;
	}
	*/
	case COMMAND_SET_CAR_CRUISE_SPEED:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		car->AutoPilot.m_nCruiseSpeed = Min(*(float*)&ScriptParams[1], 60.0f * car->pHandling->Transmission.fMaxCruiseVelocity);
		return 0;
	}
	case COMMAND_SET_CAR_DRIVING_STYLE:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		car->AutoPilot.m_nDrivingStyle = (uint8)ScriptParams[1];
		return 0;
	}
	case COMMAND_SET_CAR_MISSION:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CVehicle* car = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(car);
		car->AutoPilot.m_nCarMission = (uint8)ScriptParams[1];
		car->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
		car->bEngineOn = true;
		return 0;
	}
	case COMMAND_IS_CAR_IN_AREA_2D:
	{
		CollectParameters(&m_nIp, 6);
				// Access raw storage through the C store or pool API
		CVehicle* vehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(vehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float x2 = *(float*)&ScriptParams[3];
		float y2 = *(float*)&ScriptParams[4];
		UpdateCompareFlag(vehicle->IsWithinArea(x1, y1, x2, y2));
		if (ScriptParams[5])
			TheScripts_HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, MAP_Z_LOW_LIMIT);
		if (TheScripts_DbgFlag)
			TheScripts_DrawDebugSquare(x1, y1, x2, y2);
		return 0;
	}
	case COMMAND_IS_CAR_IN_AREA_3D:
	{
		CollectParameters(&m_nIp, 8);
				// Access raw storage through the C store or pool API
		CVehicle* vehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(vehicle);
		float x1 = *(float*)&ScriptParams[1];
		float y1 = *(float*)&ScriptParams[2];
		float z1 = *(float*)&ScriptParams[3];
		float x2 = *(float*)&ScriptParams[4];
		float y2 = *(float*)&ScriptParams[5];
		float z2 = *(float*)&ScriptParams[6];
		UpdateCompareFlag(vehicle->IsWithinArea(x1, y1, z1, x2, y2, z2));
		if (ScriptParams[7])
			TheScripts_HighlightImportantArea((uintptr)this + m_nIp, x1, y1, x2, y2, (z1 + z2) / 2);
		if (TheScripts_DbgFlag)
			TheScripts_DrawDebugCube(x1, y1, z1, x2, y2, z2);
		return 0;
	}
	case COMMAND_SPECIAL_0:
	case COMMAND_SPECIAL_1:
	case COMMAND_SPECIAL_2:
	case COMMAND_SPECIAL_3:
	case COMMAND_SPECIAL_4:
	case COMMAND_SPECIAL_5:
	case COMMAND_SPECIAL_6:
	case COMMAND_SPECIAL_7:
		script_assert(0);
		return 0;
	case COMMAND_PRINT_BIG:
	{
		wchar* key = TheScripts_GetTextByKeyFromScript(&m_nIp);
#ifdef MISSION_REPLAY
		if (strcmp((char*)&TheScripts_ScriptSpace[m_nIp - KEY_LENGTH_IN_SCRIPT], "M_FAIL") == 0) {
			if (AllowMissionReplay == MISSION_RETRY_STAGE_WAIT_FOR_TIMER_AFTER_RESTART)
				AllowMissionReplay = MISSION_RETRY_STAGE_NORMAL;
			if (CanAllowMissionReplay())
				AllowMissionReplay = MISSION_RETRY_STAGE_WAIT_FOR_SCRIPT_TO_TERMINATE;
		}
#endif
		CollectParameters(&m_nIp, 2);
		Messages_AddBigMessage(key, ScriptParams[0], ScriptParams[1] - 1);
		return 0;
	}
	case COMMAND_PRINT:
	{
		wchar* key = TheScripts_GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		Messages_AddMessage(key, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	case COMMAND_PRINT_NOW:
	{
		wchar* key = TheScripts_GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		Messages_AddMessageJumpQ(key, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	/*
	case COMMAND_PRINT_SOON:
	{
		wchar* key = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		CollectParameters(&m_nIp, 2);
		Messages_AddMessageSoon(key, ScriptParams[0], ScriptParams[1]);
		return 0;
	}
	*/
	case COMMAND_CLEAR_PRINTS:
		Messages_ClearMessages();
		return 0;
	case COMMAND_GET_TIME_OF_DAY:
		ScriptParams[0] = Clock_GetHours();
		ScriptParams[1] = Clock_GetMinutes();
		StoreParameters(&m_nIp, 2);
		return 0;
	case COMMAND_SET_TIME_OF_DAY:
		CollectParameters(&m_nIp, 2);
		Clock_SetGameClock(ScriptParams[0], ScriptParams[1]);
		return 0;
	case COMMAND_GET_MINUTES_TO_TIME_OF_DAY:
		CollectParameters(&m_nIp, 2);
		ScriptParams[0] = Clock_GetGameClockMinutesUntil(ScriptParams[0], ScriptParams[1]);
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_IS_POINT_ON_SCREEN:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[0];
		if (pos.z <= -100)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		UpdateCompareFlag(TheCamera.IsSphereVisible(pos, *(float*)&ScriptParams[3]));
		return 0;
	}
	case COMMAND_DEBUG_ON:
		TheScripts_DbgFlag = true;
		return 0;
	case COMMAND_DEBUG_OFF:
		TheScripts_DbgFlag = false;
		return 0;
	/*
	case COMMAND_RETURN_TRUE:
		UpdateCompareFlag(true);
		return 0;
	case COMMAND_RETURN_FALSE:
		UpdateCompareFlag(false);
		return 0;
	*/
	//case COMMAND_VAR_INT:
	default:
		script_assert(0);
		break;
	}
	return -1;
}

int8 CRunningScript::ProcessCommands200To299(int32 command)
{
	switch (command) {
	/* Special commands.
	case COMMAND_VAR_FLOAT:
	case COMMAND_LVAR_INT:
	case COMMAND_LVAR_FLOAT:
	case COMMAND_LBRACKET:
	case COMMAND_RBRACKET:
	case COMMAND_REPEAT:
	case COMMAND_ENDREPEAT:
	case COMMAND_IF:
	case COMMAND_IFNOT:
	case COMMAND_ELSE:
	case COMMAND_ENDIF:
	case COMMAND_WHILE:
	case COMMAND_WHILENOT:
	case COMMAND_ENDWHILE:
	*/
	case COMMAND_ANDOR:
	{
		CollectParameters(&m_nIp, 1);
		// Delegate to the C implementation
		int condResult = m_bCondResult;
		int result = ScriptBC_AndOr(ScriptParams[0], &m_nAndOrState, &condResult);
		m_bCondResult = condResult != 0;
		script_assert(result == SCRIPTBC_OK && "COMMAND_ANDOR: invalid ANDOR state");
		return 0;
	}
	case COMMAND_LAUNCH_MISSION:
	{
		CollectParameters(&m_nIp, 1);
		CRunningScript* pNew = TheScripts_StartNewScript(ScriptParams[0]);
		pNew->m_bIsMissionScript = true;
		return 0;
	}
	case COMMAND_MISSION_HAS_FINISHED:
	{
		if (!m_bIsMissionScript)
			return 0;
		TheScripts_MissionCleanUp.Process();
		return 0;
	}
	case COMMAND_STORE_CAR_CHAR_IS_IN:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* ped = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(ped);
		CVehicle* pCurrent = nil;
		if (ped->bInVehicle) {
			pCurrent = ped->m_pMyVehicle;
		}
		script_assert(pCurrent); // GetIndex(0) doesn't look good
				// Access raw storage through the C store or pool API
		int handle = CPool_GetIndex(Pools_GetVehiclePool(), pCurrent);
				if (handle != TheScripts_StoreVehicleIndex && m_bIsMissionScript){
						// Access raw storage through the C store or pool API
			CVehicle* pOld = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), TheScripts_StoreVehicleIndex));
						if (pOld){
				CarCtrl_RemoveFromInterestingVehicleList(pOld);
				if (pOld->VehicleCreatedBy == MISSION_VEHICLE && TheScripts_StoreVehicleWasRandom){
					pOld->VehicleCreatedBy = RANDOM_VEHICLE;
					pOld->bIsLocked = false;
					CarCtrl_NumRandomCars++;
					CarCtrl_NumMissionCars--;
					TheScripts_MissionCleanUp.RemoveEntityFromList(TheScripts_StoreVehicleIndex, CLEANUP_CAR);
				}
			}

			TheScripts_StoreVehicleIndex = handle;
			switch (pCurrent->VehicleCreatedBy){
			case RANDOM_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CarCtrl_NumMissionCars++;
				CarCtrl_NumRandomCars--;
				TheScripts_StoreVehicleWasRandom = true;
				TheScripts_MissionCleanUp.AddEntityToList(TheScripts_StoreVehicleIndex, CLEANUP_CAR);
				break;
			case PARKED_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CarCtrl_NumMissionCars++;
				CarCtrl_NumParkedCars--;
				TheScripts_StoreVehicleWasRandom = true;
				TheScripts_MissionCleanUp.AddEntityToList(TheScripts_StoreVehicleIndex, CLEANUP_CAR);
				break;
			case MISSION_VEHICLE:
			case PERMANENT_VEHICLE:
				TheScripts_StoreVehicleWasRandom = false;
				break;
			default:
				break;
			}
		}
		ScriptParams[0] = TheScripts_StoreVehicleIndex;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_STORE_CAR_PLAYER_IS_IN:
	{
		CollectParameters(&m_nIp, 1);
		CPed* ped = World_Players[ScriptParams[0]].m_pPed;
		script_assert(ped);
		if (!ped->bInVehicle)
			return 0; // No value written to output variable
		CVehicle* pCurrent = ped->m_pMyVehicle;
		script_assert(pCurrent); // Here pCurrent shouldn't be NULL anyway
				// Access raw storage through the C store or pool API
		int handle = CPool_GetIndex(Pools_GetVehiclePool(), pCurrent);
				if (handle != TheScripts_StoreVehicleIndex && m_bIsMissionScript) {
						// Access raw storage through the C store or pool API
			CVehicle* pOld = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), TheScripts_StoreVehicleIndex));
						if (pOld){
				CarCtrl_RemoveFromInterestingVehicleList(pOld);
				if (pOld->VehicleCreatedBy == MISSION_VEHICLE && TheScripts_StoreVehicleWasRandom){
					pOld->VehicleCreatedBy = RANDOM_VEHICLE;
					pOld->bIsLocked = false;
					CarCtrl_NumRandomCars++;
					CarCtrl_NumMissionCars--;
					TheScripts_MissionCleanUp.RemoveEntityFromList(TheScripts_StoreVehicleIndex, CLEANUP_CAR);
				}
			}

			TheScripts_StoreVehicleIndex = handle;
			switch (pCurrent->VehicleCreatedBy) {
			case RANDOM_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CarCtrl_NumMissionCars++;
				CarCtrl_NumRandomCars--;
				TheScripts_StoreVehicleWasRandom = true;
				TheScripts_MissionCleanUp.AddEntityToList(TheScripts_StoreVehicleIndex, CLEANUP_CAR);
				break;
			case PARKED_VEHICLE:
				pCurrent->VehicleCreatedBy = MISSION_VEHICLE;
				CarCtrl_NumMissionCars++;
				CarCtrl_NumParkedCars--;
				TheScripts_StoreVehicleWasRandom = true;
				TheScripts_MissionCleanUp.AddEntityToList(TheScripts_StoreVehicleIndex, CLEANUP_CAR);
				break;
			case MISSION_VEHICLE:
			case PERMANENT_VEHICLE:
				TheScripts_StoreVehicleWasRandom = false;
				break;
			default:
				break;
			}
		}
		ScriptParams[0] = TheScripts_StoreVehicleIndex;
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_CAR:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
		CVehicle* pCheckedVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[1]));
				CVehicle* pActualVehicle = pPed->bInVehicle ? pPed->m_pMyVehicle : nil;
		UpdateCompareFlag(pActualVehicle && pActualVehicle == pCheckedVehicle);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_CAR:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CVehicle* pCheckedVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[1]));
				CPed* pPed = World_Players[ScriptParams[0]].m_pPed;
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle == pCheckedVehicle);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_MODEL:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				CVehicle* pActualVehicle = pPed->bInVehicle ? pPed->m_pMyVehicle : nil;
		UpdateCompareFlag(pActualVehicle && pActualVehicle->GetModelIndex() == ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_MODEL:
	{
		CollectParameters(&m_nIp, 2);
		CPed* pPed = World_Players[ScriptParams[0]].m_pPed;
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle->GetModelIndex() == ScriptParams[1]);
		return 0;
	}
	case COMMAND_IS_CHAR_IN_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle);
		return 0;
	}
	case COMMAND_IS_PLAYER_IN_ANY_CAR:
	{
		CollectParameters(&m_nIp, 1);
		CPed* pPed = World_Players[ScriptParams[0]].m_pPed;
		UpdateCompareFlag(pPed->bInVehicle && pPed->m_pMyVehicle);
		return 0;
	}
	case COMMAND_IS_BUTTON_PRESSED:
	{
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(GetPadState(ScriptParams[0], ScriptParams[1]) != 0);
		return 0;
	}
	/*
	case COMMAND_GET_PAD_STATE:
	{
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = GetPadState(ScriptParams[0], ScriptParams[1]);
		StoreParameters(&m_nIp, 1);
		return 0;
	}
	*/
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_2D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_2D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_2D:
		LocatePlayerCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_2D:
		LocatePlayerCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_2D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_2D:
	case COMMAND_LOCATE_CHAR_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
		LocateCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_2D:
		LocateCharCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
		LocatePlayerCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_3D:
		LocatePlayerCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
		LocateCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
		LocateCharCharCommand(command, &m_nIp);
		return 0;
	case COMMAND_CREATE_OBJECT:
	{
		CollectParameters(&m_nIp, 4);
		int mi = ScriptParams[0] >= 0 ? ScriptParams[0] : TheScripts_UsedObjectArray[-ScriptParams[0]].index;
		// Allocate the scripted object without invoking C++ new.
		CObjectPool *objectPool = Pools_GetObjectPool();
				// Access raw storage through the C store or pool API
		CObject* pObj = ((CObject*)CPool_New(objectPool));
		#ifdef FIX_BUGS
		if (!pObj) {
						// Access raw storage through the C store or pool API
			for (int32 i = 0; i < CPool_GetSize(objectPool); i++) {
				// Access raw storage through the C store or pool API
				CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
							if (existing && existing->ObjectCreatedBy == TEMP_OBJECT) {
										// Access raw storage through the C store or pool API
					int32 handle = CPool_GetIndex(objectPool, existing);
										World_Remove(existing);
					existing->~CObject();
										// Access raw storage through the C store or pool API
					CPool_Delete(objectPool, existing);
					pObj = ((CObject*)CPool_NewAt(objectPool, handle));
										break;
				}
			}
		}
		if (pObj)
#endif
			std::allocator<CObject>().construct(pObj, mi, false);
		assert(pObj);
		pObj->ObjectCreatedBy = MISSION_OBJECT;
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		pos.z += pObj->GetDistanceFromCentreOfMassToBaseOfModel();
		pObj->SetPosition(pos);
		pObj->SetOrientation(0.0f, 0.0f, 0.0f);
		pObj->GetMatrix().UpdateRW();
		pObj->UpdateRwFrame();
		CBaseModelInfo* pModelInfo = ((CBaseModelInfo*)ModelRegistry_Get(mi)); // rouz edit (ChatGPT)
		if (pModelInfo->IsBuilding() && ((CSimpleModelInfo*)pModelInfo)->m_isBigBuilding)
			pObj->SetupBigBuilding();
		TheScripts_ClearSpaceForMissionEntity(pos, pObj);
		World_Add(pObj);
				// Access raw storage through the C store or pool API
		ScriptParams[0] = CPool_GetIndex(Pools_GetObjectPool(), pObj);
				StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			TheScripts_MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_DELETE_OBJECT:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CObject* pObj = ((CObject*)CPool_GetAt(Pools_GetObjectPool(), ScriptParams[0]));
				if (pObj){
			World_Remove(pObj);
			World_RemoveReferencesToDeletedObject(pObj);
			// Destroy and release the scripted object without invoking C++ delete.
			pObj->~CObject();
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetObjectPool(), pObj);
					}
		if (m_bIsMissionScript)
			TheScripts_MissionCleanUp.RemoveEntityFromList(ScriptParams[0], CLEANUP_OBJECT);
		return 0;
	}
	case COMMAND_ADD_SCORE:
		CollectParameters(&m_nIp, 2);
		World_Players[ScriptParams[0]].m_nMoney += ScriptParams[1];
		if (World_Players[ScriptParams[0]].m_nMoney < 0)
			World_Players[ScriptParams[0]].m_nMoney = 0;
		return 0;
	case COMMAND_IS_SCORE_GREATER:
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(World_Players[ScriptParams[0]].m_nMoney > ScriptParams[1]);
		return 0;
	case COMMAND_STORE_SCORE:
		CollectParameters(&m_nIp, 1);
		ScriptParams[0] = World_Players[ScriptParams[0]].m_nMoney;
		StoreParameters(&m_nIp, 1);
		return 0;
	case COMMAND_GIVE_REMOTE_CONTROLLED_CAR_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 5);
		CVector pos = *(CVector*)&ScriptParams[1];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		Remote_GivePlayerRemoteControlledCar(pos.x, pos.y, pos.z, DEGTORAD(*(float*)&ScriptParams[4]), MI_RCBANDIT); // rouz edit (ChatGPT)
		return 0;
	}
	case COMMAND_ALTER_WANTED_LEVEL:
		CollectParameters(&m_nIp, 2);
		World_Players[ScriptParams[0]].m_pPed->SetWantedLevel(ScriptParams[1]);
		return 0;
	case COMMAND_ALTER_WANTED_LEVEL_NO_DROP:
		CollectParameters(&m_nIp, 2);
		World_Players[ScriptParams[0]].m_pPed->SetWantedLevelNoDrop(ScriptParams[1]);
		return 0;
	case COMMAND_IS_WANTED_LEVEL_GREATER:
		CollectParameters(&m_nIp, 2);
		UpdateCompareFlag(Wanted_GetWantedLevel(World_Players[ScriptParams[0]].m_pPed->m_pWanted) > ScriptParams[1]);
		return 0;
	case COMMAND_CLEAR_WANTED_LEVEL:
		CollectParameters(&m_nIp, 1);
		World_Players[ScriptParams[0]].m_pPed->SetWantedLevel(0);
		return 0;
	case COMMAND_SET_DEATHARREST_STATE:
		CollectParameters(&m_nIp, 1);
		m_bDeatharrestEnabled = (ScriptParams[0] == 1);
		return 0;
	case COMMAND_HAS_DEATHARREST_BEEN_EXECUTED:
		UpdateCompareFlag(m_bDeatharrestExecuted);
		return 0;
	/*
	case COMMAND_ADD_AMMO_TO_PLAYER:
	{
		CollectParameters(&m_nIp, 3);
		CWorld::Players[ScriptParams[0]].m_pPed->GrantAmmo((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	*/
	case COMMAND_ADD_AMMO_TO_CHAR:
	{
		CollectParameters(&m_nIp, 3);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(pPed);
		pPed->GrantAmmo((eWeaponType)ScriptParams[1], ScriptParams[2]);
		return 0;
	}
	//case COMMAND_ADD_AMMO_TO_CAR:
	//case COMMAND_IS_PLAYER_STILL_ALIVE:
	case COMMAND_IS_PLAYER_DEAD:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(World_Players[ScriptParams[0]].m_WBState == WBSTATE_WASTED);
		return 0;
	case COMMAND_IS_CHAR_DEAD:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				UpdateCompareFlag(!pPed || pPed->DyingOrDead());
		return 0;
	}
	case COMMAND_IS_CAR_DEAD:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				UpdateCompareFlag(!pVehicle || pVehicle->GetStatus() == STATUS_WRECKED || pVehicle->bIsDrowning);
		return 0;
	}
	case COMMAND_SET_CHAR_THREAT_SEARCH:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(pPed);
		pPed->m_fearFlags |= ScriptParams[1];
		return 0;
	}
	//case COMMAND_SET_CHAR_THREAT_REACTION:
	case COMMAND_SET_CHAR_OBJ_NO_OBJ:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(pPed);
		pPed->bScriptObjectiveCompleted = false;
		pPed->ClearObjective();
		return 0;
	}
	//case COMMAND_ORDER_DRIVER_OUT_OF_CAR:
	//case COMMAND_ORDER_CHAR_TO_DRIVE_CAR:
	//case COMMAND_ADD_PATROL_POINT:
	//case COMMAND_IS_PLAYER_IN_GANGZONE:
	case COMMAND_IS_PLAYER_IN_ZONE:
	{
		CollectParameters(&m_nIp, 1);
		CPlayerInfo* pPlayer = &World_Players[ScriptParams[0]];
		char label[12];
		TheScripts_ReadTextLabelFromScript(&m_nIp, label);
		int zoneToCheck = Zones_FindZoneByLabelAndReturnIndex(label, ZONE_DEFAULT);
		if (zoneToCheck != -1)
			m_nIp += KEY_LENGTH_IN_SCRIPT; /* why only if zone != -1? */
		CVector pos = pPlayer->GetPos();
		CZone* pZone = Zones_GetNavigationZone(zoneToCheck);
		UpdateCompareFlag(Zones_PointLiesWithinZone(ZonesPosition_FromVector(*(&pos)), pZone));
		return 0;
	}
	case COMMAND_IS_PLAYER_PRESSING_HORN:
		CollectParameters(&m_nIp, 1);
		UpdateCompareFlag(World_Players[ScriptParams[0]].m_pPed->GetPedState() == PED_DRIVING &&
			CPad::GetPad(ScriptParams[0])->GetHorn());
		return 0;
	case COMMAND_HAS_CHAR_SPOTTED_PLAYER:
	{
		CollectParameters(&m_nIp, 2);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(pPed);
		UpdateCompareFlag(pPed->OurPedCanSeeThisOne(World_Players[ScriptParams[1]].m_pPed));
		return 0;
	}
#ifdef SUPPORT_GINPUT_SCRIPT
	case COMMAND_HAS_PAD_IN_HANDS:
		UpdateCompareFlag(Input_affectedByController); // rouz edit (ChatGPT)
		return 0;
#else
	//case COMMAND_ORDER_CHAR_TO_BACKDOOR:
#endif
	//case COMMAND_ADD_CHAR_TO_GANG:
	case COMMAND_IS_CHAR_OBJECTIVE_PASSED:
	{
		CollectParameters(&m_nIp, 1);
				// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
				script_assert(pPed);
		UpdateCompareFlag(pPed->bScriptObjectiveCompleted);
		return 0;
	}
	/* Not implemented.
	case COMMAND_SET_CHAR_DRIVE_AGGRESSION:
	case COMMAND_SET_CHAR_MAX_DRIVESPEED:
	*/
	case COMMAND_CREATE_CHAR_INSIDE_CAR:
	{
		CollectParameters(&m_nIp, 3);
				// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
				script_assert(pVehicle);
		switch (ScriptParams[2]) {
		case MI_COP:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_STREET;
			break;
		case MI_SWAT:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_SWAT;
			break;
		case MI_FBI:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_FBI;
			break;
		case MI_ARMY:
			if (ScriptParams[1] == PEDTYPE_COP)
				ScriptParams[2] = COP_ARMY;
			break;
		case MI_MEDIC:
			if (ScriptParams[1] == PEDTYPE_EMERGENCY)
				ScriptParams[2] = PEDTYPE_EMERGENCY;
			break;
		case MI_FIREMAN:
			if (ScriptParams[1] == PEDTYPE_FIREMAN)
				ScriptParams[2] = PEDTYPE_FIREMAN;
			break;
		default:
			break;
		}
		CPed* pPed;
		if (ScriptParams[1] == PEDTYPE_COP) {
			// Allocate the scripted driver cop without invoking C++ new.
						// Access raw storage through the C store or pool API
			pPed = ((CPed*)CPool_New(Pools_GetPedPool()));
						assert(pPed);
			std::allocator<CCopPed>().construct((CCopPed*)pPed, (eCopType)ScriptParams[2]);
		} else if (ScriptParams[1] == PEDTYPE_EMERGENCY || ScriptParams[1] == PEDTYPE_FIREMAN) {
			// Allocate the scripted driver emergency ped without invoking C++ new.
						// Access raw storage through the C store or pool API
			pPed = ((CPed*)CPool_New(Pools_GetPedPool()));
						assert(pPed);
			std::allocator<CEmergencyPed>().construct((CEmergencyPed*)pPed, ScriptParams[2]);
		} else {
			// Allocate the scripted driver civilian ped without invoking C++ new.
						// Access raw storage through the C store or pool API
			pPed = ((CPed*)CPool_New(Pools_GetPedPool()));
						assert(pPed);
			std::allocator<CCivilianPed>().construct((CCivilianPed*)pPed, (ePedType)ScriptParams[1], ScriptParams[2]);
		}
		pPed->CharCreatedBy = MISSION_CHAR;
		pPed->bRespondsToThreats = false;
		pPed->bAllowMedicsToReviveMe = false;
		pPed->bIsPlayerFriend = false;
		if (pVehicle->bIsBus)
			pPed->bRenderPedInCar = false;
		pPed->SetPosition(pVehicle->GetPosition());
		pPed->SetOrientation(0.0f, 0.0f, 0.0f);
		pPed->SetPedState(PED_DRIVING);
		Population_nTotalMissionPeds++; // rouz edit (ChatGPT)
		script_assert(!pVehicle->pDriver);
		pVehicle->pDriver = pPed;
		pVehicle->pDriver->RegisterReference((CEntity**)&pVehicle->pDriver);
		pPed->m_pMyVehicle = pVehicle;
		pPed->m_pMyVehicle->RegisterReference((CEntity**)&pPed->m_pMyVehicle);
		pPed->bInVehicle = true;
		pVehicle->SetStatus(STATUS_PHYSICS);
		if (!pVehicle->IsBoat())
			pVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
		pVehicle->bEngineOn = true;
		pPed->bUsesCollision = false;
		pPed->AddInCarAnims(pVehicle, true);
		pPed->m_nZoneLevel = Zones_GetLevelFromPosition(ZonesPosition_FromVector(*(&pPed->GetPosition())));
		World_Add(pPed);
				// Access raw storage through the C store or pool API
		ScriptParams[0] = CPool_GetIndex(Pools_GetPedPool(), pPed);
				StoreParameters(&m_nIp, 1);
		if (m_bIsMissionScript)
			TheScripts_MissionCleanUp.AddEntityToList(ScriptParams[0], CLEANUP_CHAR);
		return 0;
	}
	case COMMAND_WARP_PLAYER_FROM_CAR_TO_COORD:
	{
		CollectParameters(&m_nIp, 4);
		CVector pos = *(CVector*)&ScriptParams[1];
		CPlayerInfo* pPlayer = &World_Players[ScriptParams[0]];
		if (pos.z <= MAP_Z_LOW_LIMIT)
			pos.z = World_FindGroundZForCoord(pos.x, pos.y);
		if (pPlayer->m_pPed->bInVehicle){
			script_assert(pPlayer->m_pPed->m_pMyVehicle);
			if (pPlayer->m_pPed->m_pMyVehicle->bIsBus)
				pPlayer->m_pPed->bRenderPedInCar = true;
			if (pPlayer->m_pPed->m_pMyVehicle->pDriver == pPlayer->m_pPed){
				pPlayer->m_pPed->m_pMyVehicle->RemoveDriver();
				pPlayer->m_pPed->m_pMyVehicle->SetStatus(STATUS_ABANDONED);
				pPlayer->m_pPed->m_pMyVehicle->bEngineOn = false;
				pPlayer->m_pPed->m_pMyVehicle->AutoPilot.m_nCruiseSpeed = 0;
			}else{
				pPlayer->m_pPed->m_pMyVehicle->RemovePassenger(pPlayer->m_pPed);
			}
		}
		pPlayer->m_pPed->RemoveInCarAnims();
		pPlayer->m_pPed->bInVehicle = false;
		pPlayer->m_pPed->m_pMyVehicle = nil;
		pPlayer->m_pPed->SetPedState(PED_IDLE);
		pPlayer->m_pPed->bUsesCollision = true;
		pPlayer->m_pPed->SetMoveSpeed(0.0f, 0.0f, 0.0f);
		pPlayer->m_pPed->ReplaceWeaponWhenExitingVehicle();
		if (pPlayer->m_pPed->m_pVehicleAnim)
			pPlayer->m_pPed->m_pVehicleAnim->blendDelta = -1000.0f;
		pPlayer->m_pPed->m_pVehicleAnim = nil;
		pPlayer->m_pPed->SetMoveState(PEDMOVE_NONE);
		// Use the complete C animation manager at the original call boundary
		AnimManager_BlendAnimation(pPlayer->m_pPed->GetClump(), pPlayer->m_pPed->m_animGroup, ANIM_STD_IDLE, 1000.0f);
		pPlayer->m_pPed->RestartNonPartialAnims();
		AudioPlayerEngine_Left(&AudioManager);
		pos.z += pPlayer->m_pPed->GetDistanceFromCentreOfMassToBaseOfModel();
		pPlayer->m_pPed->Teleport(pos);
		TheScripts_ClearSpaceForMissionEntity(pos, pPlayer->m_pPed);
		return 0;
	}
	//case COMMAND_MAKE_CHAR_DO_NOTHING:
	default:
		script_assert(0);
		break;
	}
	return -1;
}

#ifdef MISSION_REPLAY

bool CRunningScript::CanAllowMissionReplay()
{
	if (AllowMissionReplay != MISSION_RETRY_STAGE_NORMAL)
		return false;
	for (int i = 0; i < ARRAY_SIZE(MissionScripts); i++) {
		if (!General_faststricmp(m_abScriptName, MissionScripts[i]))
			return true;
	}
	return false;
}

uint32 AddExtraDeathDelay()
{
	return 1000;
}

void RetryMission(int type, int unk)
{
	if (type == MISSION_RETRY_TYPE_SUGGEST_TO_PLAYER) {
		doingMissionRetry = true;
		FrontEndMenuManager.m_nCurrScreen = MENUPAGE_MISSION_RETRY;
		FrontEndMenuManager.m_bAttemptingMissionRetry = true;
		FrontEndMenuManager.RequestFrontEndStartUp();
	}
	else if (type == MISSION_RETRY_TYPE_BEGIN_RESTARTING) {
		doingMissionRetry = false;
		AllowMissionReplay = MISSION_RETRY_STAGE_START_RESTARTING;
		TheScripts_MissionCleanUp.Process();
	}
}

#endif

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
