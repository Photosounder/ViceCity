//+ rouz edit (ChatGPT)
#include "common.h"

#include "SceneEdit.h"
#ifdef GTA_SCENE_EDIT
#include "Automobile.h"
#include "Camera.h"
#include "CarCtrl.h"
#include "CivilianPed.h"
#include "FileMgr.h"
#include "General.h"
#include "Font.h"
#include "ModelIndices.h"
#include "ModelInfo.h"
#include "Pad.h"
#include "Ped.h"
#include "Population.h"
#include "Text.h"
#include "Timecycle.h"
#include "Streaming.h"
#include "Vehicle.h"
#include "WeaponInfo.h"
#include "World.h"
#include "Pools.h" // rouz edit (ChatGPT)

bool SceneEdit_m_bEditOn;
int32 SceneEdit_m_bCameraFollowActor;
bool SceneEdit_m_bRecording;
CVector SceneEdit_m_vecCurrentPosition;
CVector SceneEdit_m_vecCamHeading;
CVector SceneEdit_m_vecGotoPosition;
int32 SceneEdit_m_nVehicle;
int32 SceneEdit_m_nVehicle2;
int32 SceneEdit_m_nActor;
int32 SceneEdit_m_nActor2;
int32 SceneEdit_m_nVehiclemodelId;
int32 SceneEdit_m_nPedmodelId;
int16 SceneEdit_m_nCurrentMovieCommand;
int16 SceneEdit_m_nNumActors;
int16 SceneEdit_m_nNumMovieCommands;
int16 SceneEdit_m_nCurrentCommand;
int16 SceneEdit_m_nCurrentVehicle;
int16 SceneEdit_m_nCurrentActor;
int16 SceneEdit_m_nWeaponType;
bool SceneEdit_m_bCommandActive;
bool SceneEdit_m_bActorSelected;
bool SceneEdit_m_bActor2Selected;
bool SceneEdit_m_bVehicleSelected;
int16 SceneEdit_m_nNumVehicles;
CPed* SceneEdit_pActors[NUM_ACTORS_IN_MOVIE];
CVehicle* SceneEdit_pVehicles[NUM_VEHICLES_IN_MOVIE];
bool SceneEdit_m_bDrawGotoArrow;
CMovieCommand SceneEdit_Movie[NUM_COMMANDS_IN_MOVIE];

#define SHADOW_OFFSET (2.0f)
#define ACTION_MESSAGE_X_RIGHT (60.0f)
#define ACTION_MESSAGE_Y (8.0f)
#define SELECTED_MESSAGE_X_RIGHT (60.0f)
#define SELECTED_MESSAGE_Y (248.0f)
#define COMMAND_NAME_X_RIGHT (60.0f)
#define COMMAND_NAME_Y (38.0f)
#define COMMAND_NAME_HEIGHT (16.0f)

#define NUM_COMMANDS_TO_DRAW (9)

static const char* pCommandStrings[] = {
	"do-nothing", "New Actor", "Move Actor", "Select Actor", "Delete Actor",
	"New Vehicle", "Move Vehicle", "Select Vehicle", "Delete Vehicle", "Give Weapon",
	"Goto", "Goto (wait)", "Get In Car", "Get Out Car", "Kill",
	"Flee", "Wait", "Position Camera", "Set Camera Target", "Select Camera Mode",
	"Save Movie", "Load Movie", "Play Movie", "END"
};

#ifdef CHECK_STRUCT_SIZES
static_assert(ARRAY_SIZE(pCommandStrings) == MOVIE_TOTAL_COMMANDS, "Scene edit: not all commands have names");
#endif

static int32 NextValidModelId(int32 mi, int32 step)
{
	int32 result = -1;
	int32 i = mi;
	while (result == -1) {
		i += step;
		if (i < 0 || i > MODELINFOSIZE) {
			step = -step;
			continue;
		}
		CBaseModelInfo* pInfo = ((CBaseModelInfo*)ModelRegistry_Get(i)); // rouz edit (ChatGPT)
		CVehicleModelInfo* pVehicleInfo = (CVehicleModelInfo*)pInfo;
		if (!pInfo)
			continue;
		if (pInfo->GetModelType() == MITYPE_PED
#ifdef FIX_BUGS
			&& !(i >= MI_SPECIAL01 && i <= MI_SPECIAL21)
#endif
			|| pInfo->GetModelType() == MITYPE_VEHICLE &&
#ifdef FIX_BUGS
			(pVehicleInfo->m_vehicleType == VEHICLE_TYPE_CAR || pVehicleInfo->m_vehicleType == VEHICLE_TYPE_BOAT))
#else // && and || priority failure it seems, also crashes on special models
			pVehicleInfo->m_vehicleType == VEHICLE_TYPE_CAR || pVehicleInfo->m_vehicleType == VEHICLE_TYPE_BOAT)
#endif
			result = i;
	}
	return result;
}

void SceneEdit_LoadMovie(void)
{
	SceneEdit_ReInitialise();
	FileMgr_SetDir("DATA");
	int fid = FileMgr_OpenFile("movie.dat", "r");
#ifdef FIX_BUGS
	if (fid >= 0)
#endif
	{
		FileMgr_Read(fid, (char*)&SceneEdit_Movie, sizeof(SceneEdit_Movie));
		FileMgr_Read(fid, (char*)&SceneEdit_m_nNumMovieCommands, sizeof(SceneEdit_m_nNumMovieCommands));
		FileMgr_CloseFile(fid);
	}
	FileMgr_SetDir("");
	SceneEdit_m_bCommandActive = false;
}

void SceneEdit_SaveMovie(void)
{
	FileMgr_SetDir("DATA");
	int fid = FileMgr_OpenFileForWriting("movie.dat");
	if (fid >= 0) {
		FileMgr_Write(fid, (char*)&SceneEdit_Movie, sizeof(SceneEdit_Movie));
		FileMgr_Write(fid, (char*)&SceneEdit_m_nNumMovieCommands, sizeof(SceneEdit_m_nNumMovieCommands));
		FileMgr_CloseFile(fid);
	}
	FileMgr_SetDir("");
	SceneEdit_m_bCommandActive = false;
}

void SceneEdit_Initialise(void)
{
	SceneEdit_m_nActor = -1;
	SceneEdit_m_nActor2 = -1;
	SceneEdit_m_nVehicle = -1;
	SceneEdit_m_nVehicle2 = -1;
	SceneEdit_m_nCurrentCommand = MOVIE_NEW_ACTOR;
	SceneEdit_m_nVehiclemodelId = MI_INFERNUS;
	SceneEdit_m_nPedmodelId = MI_MALE01;
	SceneEdit_m_nNumVehicles = 0;
	SceneEdit_m_nNumActors = 0;
	SceneEdit_m_nNumMovieCommands = 0;
	SceneEdit_m_bCommandActive = false;
	SceneEdit_m_bRecording = true;
	SceneEdit_m_bEditOn = false;
	for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++)
		SceneEdit_pActors[i] = nil;
	for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++)
		SceneEdit_pVehicles[i] = nil;
	SceneEdit_m_vecCamHeading = TheCamera.Cams[TheCamera.ActiveCam].Front;
	SceneEdit_m_vecGotoPosition = CVector(0.0f, 0.0f, 0.0f);
	SceneEdit_m_bCameraFollowActor = false;
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true;
	SceneEdit_m_bDrawGotoArrow = false;
}

void SceneEdit_InitPlayback(void)
{
	SceneEdit_m_nVehiclemodelId = MI_INFERNUS;
	SceneEdit_m_nPedmodelId = MI_MALE01;
	SceneEdit_m_bCommandActive = false;
	SceneEdit_m_nNumActors = 0;
	SceneEdit_m_nNumVehicles = 0;
	SceneEdit_m_nActor = -1;
	SceneEdit_m_nActor2 = -1;
	SceneEdit_m_nVehicle = -1;
	SceneEdit_m_nVehicle2 = -1;
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true;
	SceneEdit_m_vecCamHeading = TheCamera.Cams[TheCamera.ActiveCam].Front;
	for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
		if (SceneEdit_pActors[i]) {
			Population_RemovePed(SceneEdit_pActors[i]);
			SceneEdit_pActors[i] = nil;
		}
	}
	SceneEdit_m_nCurrentActor = 0;
	for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
		if (SceneEdit_pVehicles[i]) {
			World_Remove(SceneEdit_pVehicles[i]);
//+ rouz edit (ChatGPT)
			// Destroy and release the scene vehicle without invoking C++ delete.
			SceneEdit_pVehicles[i]->~CVehicle();
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), SceneEdit_pVehicles[i]);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			SceneEdit_pVehicles[i] = nil;
		}
	}
	SceneEdit_m_nCurrentVehicle = 0;
	SceneEdit_m_vecGotoPosition = CVector(0.0f, 0.0f, 0.0f);
	SceneEdit_m_nCurrentMovieCommand = MOVIE_DO_NOTHING;
	SceneEdit_m_bDrawGotoArrow = false;
}

void SceneEdit_ReInitialise(void)
{
	SceneEdit_m_nVehiclemodelId = MI_INFERNUS;
	SceneEdit_m_nPedmodelId = MI_MALE01;
	SceneEdit_m_nCurrentCommand = MOVIE_NEW_ACTOR;
	SceneEdit_m_bEditOn = true;
	SceneEdit_m_bRecording = true;
	SceneEdit_m_bCommandActive = false;
#ifdef FIX_BUGS
	SceneEdit_m_bCameraFollowActor = false;
	TheCamera.Cams[TheCamera.ActiveCam].ResetStatics = true; // not enough...
#endif
	SceneEdit_m_nActor = -1;
	SceneEdit_m_nActor2 = -1;
	SceneEdit_m_nVehicle = -1;
	SceneEdit_m_nVehicle2 = -1;
	SceneEdit_m_nNumMovieCommands = 0;
	SceneEdit_m_nCurrentMovieCommand = MOVIE_DO_NOTHING;
	SceneEdit_m_nNumActors = 0;
	SceneEdit_m_nNumVehicles = 0;
	for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
		if (SceneEdit_pActors[i]) {
			Population_RemovePed(SceneEdit_pActors[i]);
			SceneEdit_pActors[i] = nil;
		}
	}
	for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
		if (SceneEdit_pVehicles[i]) {
			World_Remove(SceneEdit_pVehicles[i]);
//+ rouz edit (ChatGPT)
			// Destroy and release the scene vehicle without invoking C++ delete.
			SceneEdit_pVehicles[i]->~CVehicle();
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), SceneEdit_pVehicles[i]);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			SceneEdit_pVehicles[i] = nil;
		}
	}
	for (int i = 0; i < NUM_COMMANDS_IN_MOVIE; i++) {
		SceneEdit_Movie[i].m_nCommandId = MOVIE_DO_NOTHING;
		SceneEdit_Movie[i].m_vecPosition = CVector(0.0f, 0.0f, 0.0f);
		SceneEdit_Movie[i].m_vecCamera = CVector(0.0f, 0.0f, 0.0f);
		SceneEdit_Movie[i].m_nActorId = -1;
		SceneEdit_Movie[i].m_nActor2Id = -1;
		SceneEdit_Movie[i].m_nVehicleId = -1;
		SceneEdit_Movie[i].m_nModelIndex = 0;
	}
	SceneEdit_m_vecGotoPosition = CVector(0.0f, 0.0f, 0.0f);
	SceneEdit_m_bDrawGotoArrow = false;
}

void SceneEdit_Update(void)
{
	if (!SceneEdit_m_bEditOn)
		return;
	if (SceneEdit_m_bRecording)
		SceneEdit_ProcessCommand();
	else {
		if (SceneEdit_m_bCameraFollowActor && SceneEdit_m_nActor != -1) {
			if (SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle)
				TheCamera.TakeControl(SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle, CCam::MODE_BEHINDCAR, JUMP_CUT, CAMCONTROL_SCRIPT);
			else
				TheCamera.TakeControl(SceneEdit_pActors[SceneEdit_m_nActor], CCam::MODE_FOLLOWPED, JUMP_CUT, CAMCONTROL_SCRIPT);
		}
		SceneEdit_PlayBack();
	}
}

void SceneEdit_Draw(void)
{
	char str[200];
	wchar wstr[200];
	if (TheCamera.m_WideScreenOn)
		return;
#ifndef FIX_BUGS
	Font_SetPropOff();
#endif
	Font_SetBackgroundOff();
	Font_SetScale(SCREEN_SCALE_X(0.8f), SCREEN_SCALE_Y(1.35f));
	Font_SetCentreOn();
	Font_SetRightJustifyOn();
	Font_SetRightJustifyWrap(0.0f);
	Font_SetBackGroundOnlyTextOff();
#ifdef FIX_BUGS
	Font_SetFontStyle(FONT_STANDARD);
	Font_SetPropOn();
	Font_SetDropColor(CRGBA(0, 0, 0, 255));
	Font_SetDropShadowPosition(1);
#else
	Font_SetFontStyle(FONT_HEADING);
	Font_SetPropOff();
#endif
	sprintf(str, "Action");
	AsciiToUnicode(str, wstr);
	Font_SetColor(CRGBA(0, 0, 0, 255));
#ifdef FIX_BUGS
	Font_PrintString(SCREEN_SCALE_FROM_RIGHT(ACTION_MESSAGE_X_RIGHT - SHADOW_OFFSET), SCREEN_SCALE_Y(ACTION_MESSAGE_Y + SHADOW_OFFSET), wstr);
#else
	Font_PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-ACTION_MESSAGE_X_RIGHT) + SHADOW_OFFSET, SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-ACTION_MESSAGE_Y) + SHADOW_OFFSET, wstr);
#endif
	Font_SetColor(CRGBA(193, 164, 120, 255));
#ifdef FIX_BUGS
	Font_PrintString(SCREEN_SCALE_FROM_RIGHT(ACTION_MESSAGE_X_RIGHT), SCREEN_SCALE_Y(ACTION_MESSAGE_Y), wstr);
#else
	Font_PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-ACTION_MESSAGE_X_RIGHT), SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-ACTION_MESSAGE_Y), wstr);
#endif
	sprintf(str, "Selected");
	AsciiToUnicode(str, wstr);
	Font_SetColor(CRGBA(0, 0, 0, 255));
#ifdef FIX_BUGS
	Font_PrintString(SCREEN_SCALE_FROM_RIGHT(SELECTED_MESSAGE_X_RIGHT - SHADOW_OFFSET), SCREEN_SCALE_Y(SELECTED_MESSAGE_Y + SHADOW_OFFSET), wstr);
#else
	Font_PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-SELECTED_MESSAGE_X_RIGHT) + SHADOW_OFFSET, SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-SELECTED_MESSAGE_Y) + SHADOW_OFFSET, wstr);
#endif
	Font_SetColor(CRGBA(193, 164, 120, 255));
#ifdef FIX_BUGS
	Font_PrintString(SCREEN_SCALE_FROM_RIGHT(SELECTED_MESSAGE_X_RIGHT), SCREEN_SCALE_Y(SELECTED_MESSAGE_Y), wstr);
#else
	Font_PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-SELECTED_MESSAGE_X_RIGHT), SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-SELECTED_MESSAGE_Y), wstr);
#endif
	Font_SetCentreOff();
#ifdef FIX_BUGS
	Font_SetScale(SCREEN_SCALE_X(0.7f), SCREEN_SCALE_Y(0.7f));
#else
	Font_SetScale(0.7f, 0.7f);
#endif
#ifdef FIX_BUGS
	Font_SetFontStyle(FONT_STANDARD);
#else
	Font_SetFontStyle(FONT_HEADING);
#endif
	Font_SetColor(CRGBA(0, 0, 0, 255));
	for (int i = 0; i < NUM_COMMANDS_TO_DRAW; i++) {
		int16 nCommandDrawn = SceneEdit_m_nCurrentCommand + i - NUM_COMMANDS_TO_DRAW / 2;
		if (nCommandDrawn >= MOVIE_TOTAL_COMMANDS)
			nCommandDrawn -= (MOVIE_TOTAL_COMMANDS - 1);
		if (nCommandDrawn <= MOVIE_DO_NOTHING)
			nCommandDrawn += (MOVIE_TOTAL_COMMANDS - 1);
		sprintf(str, "%s", pCommandStrings[nCommandDrawn]);
		AsciiToUnicode(str, wstr);
		Font_SetColor(CRGBA(0, 0, 0, 255));
#ifdef FIX_BUGS
		Font_PrintString(SCREEN_SCALE_FROM_RIGHT(COMMAND_NAME_X_RIGHT - SHADOW_OFFSET), SCREEN_SCALE_Y(COMMAND_NAME_Y + SHADOW_OFFSET + i * COMMAND_NAME_HEIGHT), wstr);
#else
		Font_PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-COMMAND_NAME_X_RIGHT) + SHADOW_OFFSET, SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-COMMAND_NAME_Y) + SHADOW_OFFSET + i * COMMAND_NAME_HEIGHT, wstr);
#endif
		if (nCommandDrawn == SceneEdit_m_nCurrentCommand)
			Font_SetColor(CRGBA(156, 91, 40, 255));
		else
			Font_SetColor(CRGBA(193, 164, 120, 255));
#ifdef FIX_BUGS
		Font_PrintString(SCREEN_SCALE_FROM_RIGHT(COMMAND_NAME_X_RIGHT), SCREEN_SCALE_Y(COMMAND_NAME_Y + i * COMMAND_NAME_HEIGHT), wstr);
#else
		Font_PrintString(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH-COMMAND_NAME_X_RIGHT), SCREEN_SCALE_FROM_BOTTOM(DEFAULT_SCREEN_HEIGHT-COMMAND_NAME_Y) + i * COMMAND_NAME_HEIGHT, wstr);
#endif
	}
}

void SceneEdit_ProcessCommand(void)
{
	if (!SceneEdit_m_bCommandActive) {
		SceneEdit_ClearForNewCommand();
		if (CPad::GetPad(1)->GetDPadUpJustDown()) {
			if (--SceneEdit_m_nCurrentCommand == MOVIE_DO_NOTHING)
				SceneEdit_m_nCurrentCommand = MOVIE_END;
		}
		if (CPad::GetPad(1)->GetDPadDownJustDown()) {
			if (++SceneEdit_m_nCurrentCommand == MOVIE_TOTAL_COMMANDS)
				SceneEdit_m_nCurrentCommand = MOVIE_NEW_ACTOR;
		}
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			if (SceneEdit_m_nCurrentCommand != MOVIE_DO_NOTHING)
				SceneEdit_m_bCommandActive = true;
		}
		return;
	}
	switch (SceneEdit_m_nCurrentCommand) {
	case MOVIE_DO_NOTHING:
		SceneEdit_m_bCommandActive = false;
		break;
	case MOVIE_NEW_ACTOR:
		if (SceneEdit_m_nActor == -1) {
			if (SceneEdit_m_nNumActors == NUM_ACTORS_IN_MOVIE)
				break;
			if (!Streaming_HasModelLoaded(SceneEdit_m_nPedmodelId)) { // rouz edit (ChatGPT)
				Streaming_RequestModel(SceneEdit_m_nPedmodelId, 0); // rouz edit (ChatGPT)
#ifdef FIX_BUGS
				Streaming_LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
				break;
			}
//+ rouz edit (ChatGPT)
			// Allocate the scene actor without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPed* pPed = ((CPed*)CPool_New(Pools_GetPedPool()));
			//- rouz edit (ChatGPT)
			assert(pPed);
			std::allocator<CCivilianPed>().construct((CCivilianPed*)pPed, PEDTYPE_SPECIAL, SceneEdit_m_nPedmodelId);
//- rouz edit (ChatGPT)
			pPed->CharCreatedBy = MISSION_CHAR;
			pPed->SetPosition(SceneEdit_m_vecCurrentPosition);
			pPed->SetOrientation(0.0f, 0.0f, 0.0f);
			World_Add(pPed);
			pPed->bUsesCollision = false;
			pPed->bAffectedByGravity = false;
			for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
				if (SceneEdit_pActors[i] == nil) {
					SceneEdit_m_nActor = i;
					SceneEdit_pActors[i] = pPed;
					break;
				}
			}
		}
		else {
			SceneEdit_pActors[SceneEdit_m_nActor]->SetPosition(SceneEdit_m_vecCurrentPosition);
			SceneEdit_pActors[SceneEdit_m_nActor]->SetOrientation(0.0f, 0.0f, 0.0f);
			int32 mi = SceneEdit_m_nPedmodelId;
			if (CPad::GetPad(1)->GetLeftShoulder1JustDown())
				mi = NextValidModelId(SceneEdit_m_nPedmodelId, -1);
			else if (CPad::GetPad(1)->GetRightShoulder1JustDown())
				mi = NextValidModelId(SceneEdit_m_nPedmodelId, 1);
			if (mi == SceneEdit_m_nPedmodelId) {
				if (CPad::GetPad(1)->GetTriangleJustDown()) {
					SceneEdit_pActors[SceneEdit_m_nActor]->bUsesCollision = true;
					SceneEdit_pActors[SceneEdit_m_nActor]->bAffectedByGravity = true;
					++SceneEdit_m_nNumActors;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_NEW_ACTOR;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_vecPosition = SceneEdit_m_vecCurrentPosition;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nModelIndex = SceneEdit_m_nPedmodelId;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nActorId = SceneEdit_m_nActor;
					SceneEdit_m_nActor = -1;
					SceneEdit_m_bCommandActive = false;
				}
				if (CPad::GetPad(1)->GetCircleJustDown()) {
					World_Remove(SceneEdit_pActors[SceneEdit_m_nActor]);
//+ rouz edit (ChatGPT)
					// Destroy and release the scene actor without invoking C++ delete.
					SceneEdit_pActors[SceneEdit_m_nActor]->~CPed();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(Pools_GetPedPool(), SceneEdit_pActors[SceneEdit_m_nActor]);
					//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
					SceneEdit_pActors[SceneEdit_m_nActor] = nil;
					SceneEdit_m_nActor = -1;
					SceneEdit_m_bCommandActive = false;
				}
			}
			else {
				SceneEdit_m_nPedmodelId = mi;
				if (SceneEdit_pActors[SceneEdit_m_nActor]) {
					World_Remove(SceneEdit_pActors[SceneEdit_m_nActor]);
//+ rouz edit (ChatGPT)
					// Destroy and release the scene actor without invoking C++ delete.
					SceneEdit_pActors[SceneEdit_m_nActor]->~CPed();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(Pools_GetPedPool(), SceneEdit_pActors[SceneEdit_m_nActor]);
					//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
				}
				SceneEdit_pActors[SceneEdit_m_nActor] = nil;
				SceneEdit_m_nActor = -1;
			}
		}
		break;
	case MOVIE_MOVE_ACTOR:
		SceneEdit_SelectActor();
		if (SceneEdit_m_bCommandActive)
			break;
		SceneEdit_pActors[SceneEdit_m_nActor]->SetPosition(SceneEdit_m_vecCurrentPosition);
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			SceneEdit_m_bCommandActive = false;
#ifndef FIX_BUGS // why? it crashes, also makes no sense
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
#endif
			SceneEdit_SelectActor();
		}
		break;
	case MOVIE_SELECT_ACTOR:
		SceneEdit_SelectActor();
		break;
	case MOVIE_DELETE_ACTOR:
		SceneEdit_SelectActor();
		if (SceneEdit_m_bActorSelected) {
			Population_RemovePed(SceneEdit_pActors[SceneEdit_m_nActor]);
			SceneEdit_m_nCurrentActor = 0;
			--SceneEdit_m_nNumActors;
#ifdef FIX_BUGS
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
			SceneEdit_m_nActor = -1;
#else
			SceneEdit_m_nActor = -1;
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
#endif
			SceneEdit_SelectActor();
			SceneEdit_m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_m_nActor = -1;
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_NEW_VEHICLE:
		if (SceneEdit_m_nVehicle == -1) {
			if (SceneEdit_m_nNumVehicles == NUM_VEHICLES_IN_MOVIE)
				break;
			if (!Streaming_HasModelLoaded(SceneEdit_m_nVehiclemodelId)) { // rouz edit (ChatGPT)
				Streaming_RequestModel(SceneEdit_m_nVehiclemodelId, 0); // rouz edit (ChatGPT)
#ifdef FIX_BUGS
				Streaming_LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
				break;
			}
//+ rouz edit (ChatGPT)
			// Allocate the scene vehicle without invoking C++ new.
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CVehicle* pVehicle = ((CVehicle*)CPool_New(Pools_GetVehiclePool()));
			//- rouz edit (ChatGPT)
			assert(pVehicle);
			std::allocator<CAutomobile>().construct((CAutomobile*)pVehicle, SceneEdit_m_nVehiclemodelId, MISSION_VEHICLE);
//- rouz edit (ChatGPT)
			pVehicle->SetStatus(STATUS_PHYSICS);
			pVehicle->SetPosition(SceneEdit_m_vecCurrentPosition);
			pVehicle->SetOrientation(0.0f, 0.0f, 0.0f);
			World_Add(pVehicle);
			pVehicle->bUsesCollision = false;
			pVehicle->bAffectedByGravity = false;
			for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
				if (SceneEdit_pVehicles[i] == nil) {
					SceneEdit_m_nVehicle = i;
					SceneEdit_pVehicles[i] = pVehicle;
					break;
				}
			}
		}
		else {
			SceneEdit_pVehicles[SceneEdit_m_nVehicle]->SetPosition(SceneEdit_m_vecCurrentPosition);
			SceneEdit_pVehicles[SceneEdit_m_nVehicle]->SetOrientation(0.0f, 0.0f, 0.0f);
			int32 mi = SceneEdit_m_nVehiclemodelId;
			if (CPad::GetPad(1)->GetLeftShoulder1JustDown())
				mi = NextValidModelId(SceneEdit_m_nVehiclemodelId, -1);
			else if (CPad::GetPad(1)->GetRightShoulder1JustDown())
				mi = NextValidModelId(SceneEdit_m_nVehiclemodelId, 1);
			if (mi == SceneEdit_m_nVehiclemodelId) {
				if (CPad::GetPad(1)->GetTriangleJustDown()) {
					SceneEdit_pVehicles[SceneEdit_m_nVehicle]->bUsesCollision = true;
					SceneEdit_pVehicles[SceneEdit_m_nVehicle]->bAffectedByGravity = true;
					++SceneEdit_m_nNumVehicles;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_NEW_VEHICLE;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_vecPosition = SceneEdit_m_vecCurrentPosition;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nModelIndex = SceneEdit_m_nVehiclemodelId;
					SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nVehicleId = SceneEdit_m_nVehicle;
					SceneEdit_m_nVehicle = -1;
					SceneEdit_m_bCommandActive = false;
				}
				if (CPad::GetPad(1)->GetCircleJustDown()) {
					World_Remove(SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
//+ rouz edit (ChatGPT)
					// Destroy and release the scene vehicle without invoking C++ delete.
					SceneEdit_pVehicles[SceneEdit_m_nVehicle]->~CVehicle();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(Pools_GetVehiclePool(), SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
					//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
					SceneEdit_pVehicles[SceneEdit_m_nVehicle] = nil;
					SceneEdit_m_nVehicle = -1;
					SceneEdit_m_bCommandActive = false;
				}
			}
			else {
				SceneEdit_m_nVehiclemodelId = mi;
				if (SceneEdit_pVehicles[SceneEdit_m_nVehicle]) {
					World_Remove(SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
//+ rouz edit (ChatGPT)
					// Destroy and release the scene vehicle without invoking C++ delete.
					SceneEdit_pVehicles[SceneEdit_m_nVehicle]->~CVehicle();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(Pools_GetVehiclePool(), SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
					//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
				}
				SceneEdit_pVehicles[SceneEdit_m_nVehicle] = nil;
				SceneEdit_m_nVehicle = -1;
			}
		}
		break;
	case MOVIE_MOVE_VEHICLE:
		SceneEdit_SelectVehicle();
		if (SceneEdit_m_bCommandActive)
			break;
		SceneEdit_pVehicles[SceneEdit_m_nVehicle]->SetPosition(SceneEdit_m_vecCurrentPosition);
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			SceneEdit_m_bCommandActive = false;
#ifndef FIX_BUGS // again, why? works wrong
			SceneEdit_pVehicles[SceneEdit_m_nVehicle] = nil;
#endif
			SceneEdit_m_nVehicle = -1;
		}
		break;
	case MOVIE_SELECT_VEHICLE:
		SceneEdit_SelectVehicle();
		break;
	case MOVIE_DELETE_VEHICLE:
		SceneEdit_SelectVehicle();
		if (SceneEdit_m_bVehicleSelected) {
			World_Remove(SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
//+ rouz edit (ChatGPT)
			// Destroy and release the selected scene vehicle without invoking C++ delete.
			SceneEdit_pVehicles[SceneEdit_m_nVehicle]->~CVehicle();
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			SceneEdit_m_nCurrentVehicle = 0;
			--SceneEdit_m_nNumVehicles;
			SceneEdit_pVehicles[SceneEdit_m_nVehicle] = nil;
			SceneEdit_m_nVehicle = -1;
			SceneEdit_SelectVehicle();
			SceneEdit_m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_pVehicles[SceneEdit_m_nVehicle] = nil;
			SceneEdit_m_nVehicle = -1;
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_GIVE_WEAPON:
		if (SceneEdit_m_bActorSelected) {
			if (SceneEdit_SelectWeapon()) {
				SceneEdit_m_bCommandActive = false;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_GIVE_WEAPON;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nActorId = SceneEdit_m_nActor;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nModelIndex = SceneEdit_m_nWeaponType;
			}
		}
		else {
			SceneEdit_SelectActor();
			SceneEdit_m_bCommandActive = true;
		}
		break;
	case MOVIE_GOTO:
	case MOVIE_GOTO_WAIT:
		if (!SceneEdit_m_bActorSelected) {
			SceneEdit_m_bDrawGotoArrow = true;
			SceneEdit_SelectActor();
			if (SceneEdit_m_nActor == -1)
				SceneEdit_m_bCommandActive = true;
		}
		else {
			SceneEdit_m_vecGotoPosition = SceneEdit_m_vecCurrentPosition;
			if (CPad::GetPad(1)->GetTriangleJustDown()) {
				if (SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle) {
					if (CarCtrl_JoinCarWithRoadSystemGotoCoors(SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle, SceneEdit_m_vecGotoPosition, false))
						SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
					else
						SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS;
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->SetStatus(STATUS_PHYSICS);
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->bEngineOn = true;
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed = General_MaxInt(16, SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed);
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
					TheCamera.TakeControl(SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle, CCam::MODE_BEHINDCAR, JUMP_CUT, CAMCONTROL_SCRIPT);
				}
				else {
					SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, SceneEdit_m_vecGotoPosition);
					TheCamera.TakeControl(SceneEdit_pActors[SceneEdit_m_nActor], CCam::MODE_FOLLOWPED, JUMP_CUT, CAMCONTROL_SCRIPT);
				}
				SceneEdit_m_bDrawGotoArrow = false;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_GOTO;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nActorId = SceneEdit_m_nActor;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_vecPosition = SceneEdit_m_vecGotoPosition;
			}
			if (!SceneEdit_m_bDrawGotoArrow) {
				if (SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle && SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission == MISSION_NONE ||
					!SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle && SceneEdit_pActors[SceneEdit_m_nActor]->m_objective == OBJECTIVE_NONE) {
					if (SceneEdit_pActors[SceneEdit_m_nActor]) // if there is something that requires this check the least, it's this one
						SceneEdit_m_vecCamHeading = TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity->GetPosition() - TheCamera.Cams[TheCamera.ActiveCam].Source;
					SceneEdit_m_bCommandActive = false;
					TheCamera.Cams[TheCamera.ActiveCam].Mode = CCam::MODE_FIGHT_CAM_RUNABOUT;
					SceneEdit_m_vecCurrentPosition = SceneEdit_pActors[SceneEdit_m_nActor]->GetPosition();
					SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_NONE);
					if (SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle)
						SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_NONE;
				}
			}
			if (CPad::GetPad(1)->GetCircleJustDown()) {
				SceneEdit_pActors[SceneEdit_m_nActor] = nil;
				SceneEdit_m_nActor = -1;
				SceneEdit_m_bCommandActive = false;
			}
		}
		break;
	case MOVIE_GET_IN_CAR:
		if (SceneEdit_m_bActorSelected)
			SceneEdit_SelectVehicle();
		else {
			SceneEdit_SelectActor();
			if (SceneEdit_m_nActor != -1)
				SceneEdit_m_bCommandActive = true;
		}
		if (SceneEdit_m_bVehicleSelected) {
			SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_GET_IN_CAR;
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nActorId = SceneEdit_m_nActor;
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nVehicleId = SceneEdit_m_nVehicle;
			SceneEdit_m_nVehicle = -1;
			SceneEdit_m_bCommandActive = false;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_pVehicles[SceneEdit_m_nVehicle] = nil;
			SceneEdit_m_nVehicle = -1;
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
			SceneEdit_m_nActor = -1;
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_GET_OUT_CAR:
		SceneEdit_SelectActor();
		if (SceneEdit_m_bActorSelected) {
			if (SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle) {
				SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_LEAVE_CAR);
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_GET_OUT_CAR;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nActorId = SceneEdit_m_nActor;
			}
			SceneEdit_m_nActor = -1;
			SceneEdit_m_bCommandActive = false;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_pVehicles[SceneEdit_m_nVehicle] = nil;
			SceneEdit_m_nVehicle = -1;
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
			SceneEdit_m_nActor = -1;
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_KILL:
		if (!SceneEdit_m_bActorSelected) {
			SceneEdit_SelectActor();
			SceneEdit_m_bCommandActive = true;
		}
		else if (!SceneEdit_m_bActor2Selected) {
			SceneEdit_SelectActor2();
			if (SceneEdit_m_bActorSelected && SceneEdit_m_bActor2Selected && SceneEdit_m_nActor != -1 && SceneEdit_m_nActor2 != -1 && SceneEdit_m_nActor != SceneEdit_m_nActor2) {
				SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, SceneEdit_pActors[SceneEdit_m_nActor2]);
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_KILL;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nActorId = SceneEdit_m_nActor;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nActor2Id = SceneEdit_m_nActor2;
				SceneEdit_m_bCommandActive = false;
			}
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
			SceneEdit_m_nActor = -1;
			SceneEdit_pActors[SceneEdit_m_nActor2] = nil;
			SceneEdit_m_nActor2 = -1;
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_FLEE:
		if (!SceneEdit_m_bActorSelected) {
			SceneEdit_SelectActor();
			SceneEdit_m_bCommandActive = true;
		}
		else if (!SceneEdit_m_bActor2Selected) {
			SceneEdit_SelectActor2();
			if (SceneEdit_m_bActorSelected && SceneEdit_m_bActor2Selected && SceneEdit_m_nActor != -1 && SceneEdit_m_nActor2 != -1 && SceneEdit_m_nActor != SceneEdit_m_nActor2) {
				SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, SceneEdit_pActors[SceneEdit_m_nActor2]);
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_FLEE;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nActorId = SceneEdit_m_nActor;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nActor2Id = SceneEdit_m_nActor2;
				SceneEdit_m_bCommandActive = false;
			}
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
			SceneEdit_m_nActor = -1;
			SceneEdit_pActors[SceneEdit_m_nActor2] = nil;
			SceneEdit_m_nActor2 = -1;
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_WAIT:
		SceneEdit_SelectActor();
		if (SceneEdit_m_bActorSelected) {
			SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_WAIT_ON_FOOT);
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_WAIT;
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nActorId = SceneEdit_m_nActor;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_pActors[SceneEdit_m_nActor] = nil;
			SceneEdit_m_nActor = -1;
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_POSITION_CAMERA:
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_POSITION_CAMERA;
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_vecPosition = TheCamera.Cams[TheCamera.ActiveCam].Source;
			SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_vecCamera = SceneEdit_m_vecCamHeading;
			SceneEdit_m_bCommandActive = false;
		}
		if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_m_bCommandActive = false;
		}
		break;
	case MOVIE_SET_CAMERA_TARGET:
		if (!SceneEdit_m_bActorSelected) {
			SceneEdit_SelectActor();
			SceneEdit_m_bCommandActive = true;
		}
		else {
			TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity = SceneEdit_pActors[SceneEdit_m_nActor];
			if (CPad::GetPad(1)->GetTriangleJustDown()) {
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nCommandId = MOVIE_SET_CAMERA_TARGET;
				SceneEdit_Movie[SceneEdit_m_nNumMovieCommands++].m_nActorId = SceneEdit_m_nActor;
				SceneEdit_m_bCommandActive = false;
			}
		}
		break;
	case MOVIE_SELECT_CAMERA_MODE:
		SceneEdit_m_bCommandActive = false;
		break;
	case MOVIE_SAVE_MOVIE:
		SceneEdit_SaveMovie();
		break;
	case MOVIE_LOAD_MOVIE:
		SceneEdit_LoadMovie();
		break;
	case MOVIE_PLAY_MOVIE:
		SceneEdit_InitPlayback();
		SceneEdit_LoadMovie();
		SceneEdit_m_bRecording = false;
		break;
	case MOVIE_END:
		SceneEdit_m_bRecording = false;
		break;
	default:
		assert(0);
	}
}

void SceneEdit_PlayBack(void)
{
	SceneEdit_m_nCurrentCommand = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nCommandId;
	if (SceneEdit_m_nCurrentMovieCommand >= SceneEdit_m_nNumMovieCommands) {
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			SceneEdit_m_nCurrentCommand = MOVIE_DO_NOTHING;
			SceneEdit_m_bRecording = true;
			SceneEdit_ReInitialise();
		}
		return;
	}
	switch (SceneEdit_m_nCurrentCommand) {
	case MOVIE_DO_NOTHING:
	case MOVIE_MOVE_ACTOR:
	case MOVIE_SELECT_ACTOR:
	case MOVIE_DELETE_ACTOR:
	case MOVIE_MOVE_VEHICLE:
	case MOVIE_SELECT_VEHICLE:
	case MOVIE_DELETE_VEHICLE:
		break;
	case MOVIE_NEW_ACTOR:
	{
		SceneEdit_m_nPedmodelId = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nModelIndex;
		SceneEdit_m_vecCurrentPosition = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_vecPosition;
		if (!Streaming_HasModelLoaded(SceneEdit_m_nPedmodelId)) { // rouz edit (ChatGPT)
			Streaming_RequestModel(SceneEdit_m_nPedmodelId, 0); // rouz edit (ChatGPT)
#ifdef FIX_BUGS
			Streaming_LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
			break;
		}
//+ rouz edit (ChatGPT)
		// Allocate the replayed scene actor without invoking C++ new.
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_New(Pools_GetPedPool()));
		//- rouz edit (ChatGPT)
		assert(pPed);
		std::allocator<CCivilianPed>().construct((CCivilianPed*)pPed, PEDTYPE_SPECIAL, SceneEdit_m_nPedmodelId);
//- rouz edit (ChatGPT)
		pPed->CharCreatedBy = MISSION_CHAR;
		World_Add(pPed);
		pPed->SetPosition(SceneEdit_m_vecCurrentPosition);
		pPed->SetOrientation(0.0f, 0.0f, 0.0f);
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (SceneEdit_pActors[i] == nil) {
				SceneEdit_m_nActor = i;
				SceneEdit_pActors[i] = pPed;
				break;
			}
		}
		SceneEdit_m_nNumActors++;
		SceneEdit_m_nCurrentMovieCommand++;
		break;
	}
	case MOVIE_NEW_VEHICLE:
	{
		SceneEdit_m_nVehiclemodelId = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nModelIndex;
		SceneEdit_m_vecCurrentPosition = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_vecPosition;
		if (!Streaming_HasModelLoaded(SceneEdit_m_nVehiclemodelId)) { // rouz edit (ChatGPT)
			Streaming_RequestModel(SceneEdit_m_nVehiclemodelId, 0); // rouz edit (ChatGPT)
#ifdef FIX_BUGS
			Streaming_LoadAllRequestedModels(false); // otherwise gets stuck :(
#endif
			break;
		}
//+ rouz edit (ChatGPT)
		// Allocate the replayed scene vehicle without invoking C++ new.
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CVehicle* pVehicle = ((CVehicle*)CPool_New(Pools_GetVehiclePool()));
		//- rouz edit (ChatGPT)
		assert(pVehicle);
		std::allocator<CAutomobile>().construct((CAutomobile*)pVehicle, SceneEdit_m_nVehiclemodelId, MISSION_VEHICLE);
//- rouz edit (ChatGPT)
		pVehicle->SetStatus(STATUS_PHYSICS);
		pVehicle->SetPosition(SceneEdit_m_vecCurrentPosition);
		pVehicle->SetOrientation(0.0f, 0.0f, 0.0f);
		World_Add(pVehicle);
		for (int i = 0; i < NUM_VEHICLES_IN_MOVIE; i++) {
			if (SceneEdit_pVehicles[i] == nil) {
				SceneEdit_m_nVehicle = i;
				SceneEdit_pVehicles[i] = pVehicle;
				break;
			}
		}
		SceneEdit_m_nNumVehicles++;
		SceneEdit_m_nCurrentMovieCommand++;
		break;
	}
	case MOVIE_GIVE_WEAPON:
		SceneEdit_m_nActor = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActorId;
		SceneEdit_m_nWeaponType = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nModelIndex;
		SceneEdit_pActors[SceneEdit_m_nActor]->GiveWeapon((eWeaponType)SceneEdit_m_nWeaponType, 1000);
		SceneEdit_pActors[SceneEdit_m_nActor]->AddWeaponModel(WeaponInfo_GetWeaponInfo(SceneEdit_pActors[SceneEdit_m_nActor]->GetWeapon()->m_eWeaponType)->m_nModelId);
		SceneEdit_pActors[SceneEdit_m_nActor]->SetCurrentWeapon(SceneEdit_m_nWeaponType);
		SceneEdit_m_nCurrentMovieCommand++;
		break;
	case MOVIE_GOTO:
	case MOVIE_GOTO_WAIT:
		SceneEdit_m_nActor = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActorId;
		SceneEdit_m_vecGotoPosition = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_vecPosition;
		if (SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle) {
			if (SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission != MISSION_GOTOCOORDS &&
				SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission != MISSION_GOTOCOORDS_STRAIGHT) {
				if ((SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->GetPosition() - SceneEdit_m_vecGotoPosition).Magnitude() < 5.0f) {
					if (CarCtrl_JoinCarWithRoadSystemGotoCoors(SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle, SceneEdit_m_vecGotoPosition, false))
						SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS_STRAIGHT;
					else
						SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_GOTOCOORDS;
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->SetStatus(STATUS_PHYSICS);
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->bEngineOn = true;
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed = General_MaxInt(16, SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nCruiseSpeed);
					SceneEdit_pActors[SceneEdit_m_nActor]->m_pMyVehicle->AutoPilot.m_nAntiReverseTimer = (*Timer_GetTimeInMilliseconds());
					if (SceneEdit_m_nCurrentCommand != MOVIE_GOTO_WAIT)
						++SceneEdit_m_nCurrentMovieCommand;
				}
				else
					++SceneEdit_m_nCurrentMovieCommand;
			}
		}
		else {
			if (SceneEdit_pActors[SceneEdit_m_nActor]->m_objective != OBJECTIVE_GOTO_AREA_ON_FOOT) {
				SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_GOTO_AREA_ON_FOOT, SceneEdit_m_vecGotoPosition);
				++SceneEdit_m_nCurrentMovieCommand;
			}
		}
		break;
	case MOVIE_GET_IN_CAR:
		SceneEdit_m_nActor = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActorId;
		if (!SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle){
			SceneEdit_m_nVehicle = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nVehicleId;
			SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_ENTER_CAR_AS_DRIVER, SceneEdit_pVehicles[SceneEdit_m_nVehicle]);
		}
		else
			++SceneEdit_m_nCurrentMovieCommand;
		break;
	case MOVIE_GET_OUT_CAR:
		SceneEdit_m_nActor = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActorId;
		if (SceneEdit_pActors[SceneEdit_m_nActor]->bInVehicle)
			SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_LEAVE_CAR);
		else
			++SceneEdit_m_nCurrentMovieCommand;
		break;
	case MOVIE_KILL:
		SceneEdit_m_nActor = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActorId;
		SceneEdit_m_nActor2 = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActor2Id;
		SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_KILL_CHAR_ON_FOOT, SceneEdit_pActors[SceneEdit_m_nActor2]);
		if (SceneEdit_pActors[SceneEdit_m_nActor2]->GetPedState() == PED_DEAD)
			++SceneEdit_m_nCurrentMovieCommand;
		break;
	case MOVIE_FLEE:
		SceneEdit_m_nActor = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActorId;
		SceneEdit_m_nActor2 = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActor2Id;
		SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_FLEE_CHAR_ON_FOOT_ALWAYS, SceneEdit_pActors[SceneEdit_m_nActor2]);
		++SceneEdit_m_nCurrentMovieCommand;
		break;
	case MOVIE_WAIT:
		SceneEdit_m_nActor = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_nActorId;
		SceneEdit_pActors[SceneEdit_m_nActor]->SetObjective(OBJECTIVE_WAIT_ON_FOOT);
		++SceneEdit_m_nCurrentMovieCommand;
		break;
	case MOVIE_POSITION_CAMERA:
		TheCamera.Cams[TheCamera.ActiveCam].Source = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_vecPosition;
		SceneEdit_m_vecCamHeading = SceneEdit_Movie[SceneEdit_m_nCurrentMovieCommand].m_vecCamera;
		TheCamera.Cams[TheCamera.ActiveCam].Front = SceneEdit_m_vecCamHeading;
		++SceneEdit_m_nCurrentMovieCommand;
		break;
	case MOVIE_SET_CAMERA_TARGET:
		SceneEdit_m_bCameraFollowActor = true;
		TheCamera.Cams[TheCamera.ActiveCam].CamTargetEntity = SceneEdit_pActors[SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nActorId];
		TheCamera.pTargetEntity = SceneEdit_pActors[SceneEdit_Movie[SceneEdit_m_nNumMovieCommands].m_nActorId];
		TheCamera.m_bLookingAtPlayer = false;
		++SceneEdit_m_nCurrentMovieCommand;
		break;
	case MOVIE_SELECT_CAMERA_MODE:
		SceneEdit_m_bCommandActive = false; // this is wrong
		break;
	}
}

void SceneEdit_ClearForNewCommand(void)
{
	SceneEdit_m_nActor = -1;
	SceneEdit_m_nActor2 = -1;
	SceneEdit_m_nVehicle = -1;
	SceneEdit_m_bActorSelected = false;
	SceneEdit_m_bActor2Selected = false;
	SceneEdit_m_bVehicleSelected = false;
	SceneEdit_m_bDrawGotoArrow = false;
}
void SceneEdit_SelectActor(void)
{
	SceneEdit_m_bActorSelected = false;
	if (SceneEdit_m_nActor != -1) {
		if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (--SceneEdit_m_nActor < 0)
					SceneEdit_m_nActor = NUM_ACTORS_IN_MOVIE - 1;
				pPed = SceneEdit_pActors[SceneEdit_m_nActor];
			} while (pPed == nil);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - SceneEdit_m_vecCamHeading;
		}
		else if (CPad::GetPad(1)->GetRightShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (++SceneEdit_m_nActor == NUM_ACTORS_IN_MOVIE)
					SceneEdit_m_nActor = 0;
				pPed = SceneEdit_pActors[SceneEdit_m_nActor];
			} while (pPed == nil);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - SceneEdit_m_vecCamHeading;
		}
		SceneEdit_m_vecCurrentPosition = SceneEdit_pActors[SceneEdit_m_nActor]->GetPosition();
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			SceneEdit_m_bActorSelected = true;
			SceneEdit_m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_m_nActor = -1;
		}
	}
	else if (SceneEdit_m_nNumActors != 0) {
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (SceneEdit_pActors[i] != nil) {
				SceneEdit_m_nActor = i;
				break;
			}
		}
		TheCamera.Cams[TheCamera.ActiveCam].Source = SceneEdit_pActors[SceneEdit_m_nActor]->GetPosition() - SceneEdit_m_vecCamHeading;
		if (SceneEdit_m_nNumActors == 1) {
			SceneEdit_m_bActorSelected = true;
			SceneEdit_m_bCommandActive = false;
		}
	}
	else {
		SceneEdit_m_bCommandActive = false;
	}
}

void SceneEdit_SelectActor2(void)
{
	SceneEdit_m_bActor2Selected = false;
	if (SceneEdit_m_nNumActors <= 1) {
		SceneEdit_m_bCommandActive = false;
		return;
	}
	if (SceneEdit_m_nActor2 != -1) {
		if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (--SceneEdit_m_nActor2 < 0)
					SceneEdit_m_nActor2 = NUM_ACTORS_IN_MOVIE - 1;
				pPed = SceneEdit_pActors[SceneEdit_m_nActor2];
			} while (pPed == nil || pPed == SceneEdit_pActors[SceneEdit_m_nActor]);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - SceneEdit_m_vecCamHeading;
		}
		else if (CPad::GetPad(1)->GetRightShoulder1JustDown()) {
			CPed* pPed;
			do {
				if (++SceneEdit_m_nActor2 == NUM_ACTORS_IN_MOVIE)
					SceneEdit_m_nActor2 = 0;
				pPed = SceneEdit_pActors[SceneEdit_m_nActor2];
			} while (pPed == nil || pPed == SceneEdit_pActors[SceneEdit_m_nActor]);
			TheCamera.Cams[TheCamera.ActiveCam].Source = pPed->GetPosition() - SceneEdit_m_vecCamHeading;
		}
		SceneEdit_m_vecCurrentPosition = SceneEdit_pActors[SceneEdit_m_nActor2]->GetPosition();
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			SceneEdit_m_bActor2Selected = true;
			SceneEdit_m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_m_nActor2 = -1;
		}
	}
	else {
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (SceneEdit_pActors[i] != nil && SceneEdit_pActors[SceneEdit_m_nActor] != SceneEdit_pActors[i] ) {
				SceneEdit_m_nActor2 = i;
				break;
			}
		}
		TheCamera.Cams[TheCamera.ActiveCam].Source = SceneEdit_pActors[SceneEdit_m_nActor2]->GetPosition() - SceneEdit_m_vecCamHeading;
	}
}

void SceneEdit_SelectVehicle(void)
{
	SceneEdit_m_bVehicleSelected = false;
	if (SceneEdit_m_nVehicle != -1) {
		if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
			CVehicle* pVehicle;
			do {
				if (--SceneEdit_m_nVehicle < 0)
					SceneEdit_m_nVehicle = NUM_VEHICLES_IN_MOVIE - 1;
				pVehicle = SceneEdit_pVehicles[SceneEdit_m_nVehicle];
			} while (pVehicle == nil);
		}
		else if (CPad::GetPad(1)->GetRightShoulder1JustDown()) {
			CVehicle* pVehicle;
			do {
				if (++SceneEdit_m_nVehicle == NUM_VEHICLES_IN_MOVIE)
					SceneEdit_m_nVehicle = 0;
				pVehicle = SceneEdit_pVehicles[SceneEdit_m_nVehicle];
			} while (pVehicle == nil);
		}
		SceneEdit_m_vecCurrentPosition = SceneEdit_pVehicles[SceneEdit_m_nVehicle]->GetPosition();
		TheCamera.Cams[TheCamera.ActiveCam].Source = SceneEdit_pVehicles[SceneEdit_m_nVehicle]->GetPosition() - SceneEdit_m_vecCamHeading;
		if (CPad::GetPad(1)->GetTriangleJustDown()) {
			SceneEdit_m_bVehicleSelected = true;
			SceneEdit_m_bCommandActive = false;
		}
		else if (CPad::GetPad(1)->GetCircleJustDown()) {
			SceneEdit_m_nVehicle = -1;
		}
	}
	else if (SceneEdit_m_nNumVehicles != 0) {
		for (int i = 0; i < NUM_ACTORS_IN_MOVIE; i++) {
			if (SceneEdit_pVehicles[i] != nil) {
				SceneEdit_m_nVehicle = i;
				break;
			}
		}
	}
}

bool SceneEdit_SelectWeapon(void)
{
	if (SceneEdit_m_nWeaponType == WEAPONTYPE_UNARMED) {
		SceneEdit_m_nWeaponType = WEAPONTYPE_COLT45;
		return false;
	}
	if (CPad::GetPad(1)->GetLeftShoulder1JustDown()) {
		if (++SceneEdit_m_nWeaponType >= WEAPONTYPE_DETONATOR)
			SceneEdit_m_nWeaponType = WEAPONTYPE_BRASSKNUCKLE;
		SceneEdit_pActors[SceneEdit_m_nActor]->ClearWeapons();
		SceneEdit_pActors[SceneEdit_m_nActor]->GiveWeapon((eWeaponType)SceneEdit_m_nWeaponType, 1000);
		SceneEdit_pActors[SceneEdit_m_nActor]->AddWeaponModel(WeaponInfo_GetWeaponInfo(SceneEdit_pActors[SceneEdit_m_nActor]->GetWeapon()->m_eWeaponType)->m_nModelId);
		SceneEdit_pActors[SceneEdit_m_nActor]->SetCurrentWeapon(SceneEdit_m_nWeaponType);
	}
	else if (CPad::GetPad(1)->GetRightShoulder1JustDown()){
		if (--SceneEdit_m_nWeaponType <= WEAPONTYPE_UNARMED)
			SceneEdit_m_nWeaponType = WEAPONTYPE_MINIGUN;
		SceneEdit_pActors[SceneEdit_m_nActor]->ClearWeapons();
		SceneEdit_pActors[SceneEdit_m_nActor]->GiveWeapon((eWeaponType)SceneEdit_m_nWeaponType, 1000);
		SceneEdit_pActors[SceneEdit_m_nActor]->AddWeaponModel(WeaponInfo_GetWeaponInfo(SceneEdit_pActors[SceneEdit_m_nActor]->GetWeapon()->m_eWeaponType)->m_nModelId);
		SceneEdit_pActors[SceneEdit_m_nActor]->SetCurrentWeapon(SceneEdit_m_nWeaponType);
	}
	if (CPad::GetPad(1)->GetTriangleJustDown()) {
		SceneEdit_m_bCommandActive = false;
		return true;
	}
	if (CPad::GetPad(1)->GetCircleJustDown()) {
		SceneEdit_pActors[SceneEdit_m_nActor]->ClearWeapons();
		SceneEdit_m_nWeaponType = WEAPONTYPE_UNARMED;
		SceneEdit_m_bCommandActive = false;
		return false;
	}
	return false;
}
#endif

//- rouz edit (ChatGPT)
