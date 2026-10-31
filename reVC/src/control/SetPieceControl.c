//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "SetPieces_backend.h"
#include "Timer.h"
#include "EntityStatus.h"
#include <stddef.h>
#include <math.h>
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
#define TIME_BETWEEN_SETPIECE_SPAWNS 20000
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
bool SetPieces_bDebug;
uint32_t SetPieces_NumSetPieces;
SetPiece SetPieces_aSetPieces[NUM_SETPIECES];
static SetPiecePoint SetPiece_Offset(SetPieceVector position,SetPiecePoint origin)
{
    // Copy the original two-dimensional component differences
    SetPiecePoint result={position.x-origin.x,position.y-origin.y};return result;
}
static float SetPiece_Dot2(SetPiecePoint a,SetPiecePoint b)
{
    // Preserve the original ordered two-dimensional products and addition
    return a.x*b.x+a.y*b.y;
}
static SetPieceVector SetPiece_ScaledForward(SetPieceVector value)
{
    // Keep the original multiply before divide for each native forward component
    SetPieceVector result={(2.0f*value.x)/3.0f,(2.0f*value.y)/3.0f,(2.0f*value.z)/3.0f};return result;
}
static void SetPiece_Normalise(SetPieceVector *value)
{
    // Preserve the complete original three-component normalization and zero-direction fallback
    float sq=value->x*value->x+value->y*value->y+value->z*value->z;
    if(sq>0.0f){
        // Multiply all original components by the reciprocal square root
        float invsqrt=1.0f/sqrtf(sq);value->x*=invsqrt;value->y*=invsqrt;value->z*=invsqrt;
    }else{
        // Retain the original fallback without resetting the other components
        value->x=1.0f;
    }
}
void SetPieces_Init(void)
{
    // Preserve every original set-piece decision, live access and transfer order
	SetPieces_bDebug = false;
	SetPieces_NumSetPieces = 0;
#ifndef MASTER
	SetPieces_BackendRegisterDebug(&SetPieces_bDebug); // rouz edit (ChatGPT)
#endif
}

void SetPieces_AddOne(uint8_t type, SetPiecePoint vTriggerInf, SetPiecePoint vTriggerSup, SetPiecePoint vSpawn1, SetPiecePoint vTarget1, SetPiecePoint vSpawn2, SetPiecePoint vTarget2)
{
    // Preserve every original set-piece decision, live access and transfer order
	if (SetPieces_NumSetPieces >= NUM_SETPIECES)
		return;
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_nType = type;
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vTriggerInf.x = Min(vTriggerInf.x, vTriggerSup.x);
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vTriggerInf.y = Min(vTriggerInf.y, vTriggerSup.y);
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vTriggerSup.x = Max(vTriggerInf.x, vTriggerSup.x);
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vTriggerSup.y = Max(vTriggerInf.y, vTriggerSup.y);
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vSpawn1 = vSpawn1;
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vSpawn2 = vSpawn2;
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vTarget1 = vTarget1;
	SetPieces_aSetPieces[SetPieces_NumSetPieces].m_vTarget2 = vTarget2;
	++SetPieces_NumSetPieces;
}

void SetPieces_Update(void)
{
    // Preserve every original set-piece decision, live access and transfer order
	int nFirst = SetPieces_NumSetPieces * ((*Timer_GetFrameCounter()) % 8) / 8;
	int nLast = SetPieces_NumSetPieces * ((*Timer_GetFrameCounter()) % 8 + 1) / 8;
	for (int i = nFirst; i < nLast; i++)
		SetPiece_Update(&SetPieces_aSetPieces[i]);
#ifndef MASTER
	// TODO: debug code from mobile
#endif // !MASTER
}

void SetPieces_Save(uint8_t* buf, uint32_t* size)
{
    // Preserve every original set-piece decision, live access and transfer order
INITSAVEBUF
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &SetPieces_NumSetPieces, sizeof(SetPieces_NumSetPieces));
	//- rouz edit (ChatGPT)
	for (int i = 0; i < NUM_SETPIECES; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		{
			// Preserve record assignment semantics and compiler padding behavior
			*(SetPiece*)buf = SetPieces_aSetPieces[i];
			SkipSaveBuf(&buf, sizeof(SetPieces_aSetPieces[i]));
		}
		//- rouz edit (ChatGPT)
	*size = sizeof(SetPieces_NumSetPieces) + NUM_SETPIECES * sizeof(SetPiece);
VALIDATESAVEBUF(*size)
}

void SetPieces_Load(uint8_t* buf, uint32_t size)
{
    // Preserve every original set-piece decision, live access and transfer order
INITSAVEBUF
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&SetPieces_NumSetPieces, &buf, sizeof(SetPieces_NumSetPieces));
	//- rouz edit (ChatGPT)
	for (int i = 0; i < NUM_SETPIECES; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		{
			// Preserve record assignment semantics and compiler padding behavior
			SetPieces_aSetPieces[i] = *(SetPiece*)buf;
			SkipSaveBuf(&buf, sizeof(SetPieces_aSetPieces[i]));
		}
		//- rouz edit (ChatGPT)
VALIDATESAVEBUF(size)
}

void SetPiece_Update(SetPiece *record)
{
    // Preserve every original set-piece decision, live access and transfer order
	if (record->m_nLastTimeCreated != 0 && (*Timer_GetTimeInMilliseconds()) <= record->m_nLastTimeCreated + TIME_BETWEEN_SETPIECE_SPAWNS)
		return;
	SetPieceVector pos = SetPieces_BackendPlayerCoors();
	if (pos.x < record->m_vTriggerInf.x || pos.x > record->m_vTriggerSup.x ||
		pos.y < record->m_vTriggerInf.y || pos.y > record->m_vTriggerSup.y)
		return;
	switch (record->m_nType) {
	case SETPIECE_TWOCOPCARSINALLEY:
	{
		if (SetPieces_BackendWanted() < 1 || SetPieces_BackendPlayerVehicle())
			return;
		CVehicle* pVehicle1 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn1, record->m_vTarget1);
		if (!pVehicle1)
			return;
		CVehicle* pVehicle2 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn2, record->m_vTarget2);
		if (!pVehicle2) {
			SetPieces_BackendRemoveVehicle(pVehicle1);
//+ rouz edit (ChatGPT)
			// Destroy and release the generated cop car without invoking C++ delete.
			SetPieces_BackendDestroyVehicle(pVehicle1);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			return;
		}
		SetPieces_BackendVehicleStatus(pVehicle1,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle1) = *SetPieces_BackendCruiseSpeed(pVehicle1) = 4;
		*SetPieces_BackendDrivingStyle(pVehicle1) = SETPIECE_DRIVINGSTYLE_SLOW_DOWN_FOR_CARS;
		*SetPieces_BackendCarMission(pVehicle1) = SETPIECE_MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1;
		*SetPieces_BackendDestinationX(pVehicle1) = record->m_vTarget1.x;
		*SetPieces_BackendDestinationY(pVehicle1) = record->m_vTarget1.y;
		*SetPieces_BackendDestinationZ(pVehicle1) = 0.0f;
		*SetPieces_BackendVehicleRangeTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 25000;
		SetPieces_BackendOccupants(pVehicle1);
		SetPieces_BackendVehicleStatus(pVehicle2,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle2) = *SetPieces_BackendCruiseSpeed(pVehicle2) = 4;
		*SetPieces_BackendDrivingStyle(pVehicle2) = SETPIECE_DRIVINGSTYLE_SLOW_DOWN_FOR_CARS;
		*SetPieces_BackendCarMission(pVehicle2) = SETPIECE_MISSION_SLOWLY_DRIVE_TOWARDS_PLAYER_1;
		*SetPieces_BackendDestinationX(pVehicle2) = record->m_vTarget2.x;
		*SetPieces_BackendDestinationY(pVehicle2) = record->m_vTarget2.y;
		*SetPieces_BackendDestinationZ(pVehicle2) = 0.0f;
		*SetPieces_BackendVehicleRangeTime(pVehicle2) = (*Timer_GetTimeInMilliseconds()) + 25000;
		SetPieces_BackendOccupants(pVehicle2);
		record->m_nLastTimeCreated = (*Timer_GetTimeInMilliseconds());
		break;
	}
	case SETPIECE_CARBLOCKINGPLAYERFROMSIDE:
	{
		if (SetPieces_BackendWanted() < 2)
			return;
		if (!SetPieces_BackendPlayerVehicle())
			return;
		if (SetPiece_Dot2(SetPieces_BackendPlayerSpeed(),SetPiece_Offset(SetPieces_BackendPlayerCoors(),record->m_vSpawn1)) >= 0.0f)
			return;
		CVehicle* pVehicle1 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn1, record->m_vTarget1);
		if (!pVehicle1)
			return;
		SetPieces_BackendVehicleStatus(pVehicle1,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle1) = *SetPieces_BackendCruiseSpeed(pVehicle1) = 16;
		*SetPieces_BackendDrivingStyle(pVehicle1) = SETPIECE_DRIVINGSTYLE_PLOUGH_THROUGH;
		*SetPieces_BackendCarMission(pVehicle1) = SETPIECE_MISSION_BLOCKPLAYER_FORWARDANDBACK;
		*SetPieces_BackendTempAction(pVehicle1) = SETPIECE_TEMPACT_GOFORWARD;
		*SetPieces_BackendTempTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 100;
		*SetPieces_BackendVehicleRangeTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 10000;
		SetPieces_BackendMoveSpeed(pVehicle1,SetPiece_ScaledForward(SetPieces_BackendForward(pVehicle1)));
		SetPieces_BackendOccupants(pVehicle1);
		record->m_nLastTimeCreated = (*Timer_GetTimeInMilliseconds());
		return;
	}
	case SETPIECE_CARRAMMINGPLAYERFROMSIDE:
	{
		if (SetPieces_BackendWanted() < 2)
			return;
		if (!SetPieces_BackendPlayerVehicle())
			return;
		if (SetPiece_Dot2(SetPieces_BackendPlayerSpeed(),SetPiece_Offset(SetPieces_BackendPlayerCoors(),record->m_vSpawn1)) >= 0.0f)
			return;
		CVehicle* pVehicle1 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn1, record->m_vTarget1);
		if (!pVehicle1)
			return;
		SetPieces_BackendVehicleStatus(pVehicle1,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle1) = *SetPieces_BackendCruiseSpeed(pVehicle1) = 16;
		*SetPieces_BackendDrivingStyle(pVehicle1) = SETPIECE_DRIVINGSTYLE_AVOID_CARS;
		*SetPieces_BackendCarMission(pVehicle1) = SETPIECE_MISSION_RAMCAR_CLOSE;
		*SetPieces_BackendTempAction(pVehicle1) = SETPIECE_TEMPACT_GOFORWARD;
		*SetPieces_BackendTempTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 100;
		*SetPieces_BackendVehicleRangeTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 10000;
		SetPieces_BackendMoveSpeed(pVehicle1,SetPiece_ScaledForward(SetPieces_BackendForward(pVehicle1)));
		SetPieces_BackendOccupants(pVehicle1);
		record->m_nLastTimeCreated = (*Timer_GetTimeInMilliseconds());
		return;
	}
	case SETPIECE_CREATECOPPERONFOOT:
	{
		if (SetPieces_BackendWanted() < 1 || SetPieces_BackendPlayerVehicle())
			return;
		CCopPed* pCop = SetPiece_TryToGenerateCopPed(record,record->m_vSpawn1);
		if (!pCop)
			return;
		float z = SetPieces_BackendGroundZ(record->m_vTarget1.x, record->m_vTarget1.y);
		SetPieces_BackendObjectiveCompleted(pCop,false);
		SetPieces_BackendObjective(pCop, (SetPieceVector){record->m_vTarget1.x,record->m_vTarget1.y,z});
		*SetPieces_BackendCopRangeTime(pCop) = (*Timer_GetTimeInMilliseconds()) + 10000;
		record->m_nLastTimeCreated = (*Timer_GetTimeInMilliseconds());
		return;
	}
	case SETPIECE_CREATETWOCOPPERSONFOOT:
	{
		if (SetPieces_BackendWanted() < 1 || SetPieces_BackendPlayerVehicle())
			return;
		CCopPed* pCop = SetPiece_TryToGenerateCopPed(record,record->m_vSpawn1);
		if (!pCop)
			return;
		float z = SetPieces_BackendGroundZ(record->m_vTarget1.x, record->m_vTarget1.y);
		SetPieces_BackendObjectiveCompleted(pCop,false);
		SetPieces_BackendObjective(pCop, (SetPieceVector){record->m_vTarget1.x,record->m_vTarget1.y,z});
		*SetPieces_BackendCopRangeTime(pCop) = (*Timer_GetTimeInMilliseconds()) + 10000;
		CCopPed* pCop2 = SetPiece_TryToGenerateCopPed(record,record->m_vSpawn2);
		if (!pCop2) {
			SetPieces_BackendRemoveCop(pCop);
//+ rouz edit (ChatGPT)
			// Destroy and release the generated cop without invoking C++ delete.
			SetPieces_BackendDestroyCop(pCop);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			return;
		}
		z = SetPieces_BackendGroundZ(record->m_vTarget2.x, record->m_vTarget2.y);
		SetPieces_BackendObjectiveCompleted(pCop2,false);
		SetPieces_BackendObjective(pCop2, (SetPieceVector){record->m_vTarget2.x,record->m_vTarget2.y,z});
		*SetPieces_BackendCopRangeTime(pCop2) = (*Timer_GetTimeInMilliseconds()) + 10000;
		record->m_nLastTimeCreated = (*Timer_GetTimeInMilliseconds());
		return;
	}
	case SETPIECE_TWOCARSBLOCKINGPLAYERFROMSIDE:
	{
		if (SetPieces_BackendWanted() < 2)
			return;
		if (!SetPieces_BackendPlayerVehicle())
			return;
		if (SetPiece_Dot2(SetPieces_BackendPlayerSpeed(),SetPiece_Offset(SetPieces_BackendPlayerCoors(),record->m_vSpawn1)) >= 0.0f)
			return;
		CVehicle* pVehicle1 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn1, record->m_vTarget1);
		if (!pVehicle1)
			return;
		SetPieces_BackendVehicleStatus(pVehicle1,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle1) = *SetPieces_BackendCruiseSpeed(pVehicle1) = 16;
		*SetPieces_BackendDrivingStyle(pVehicle1) = SETPIECE_DRIVINGSTYLE_PLOUGH_THROUGH;
		*SetPieces_BackendCarMission(pVehicle1) = SETPIECE_MISSION_BLOCKPLAYER_FORWARDANDBACK;
		*SetPieces_BackendTempAction(pVehicle1) = SETPIECE_TEMPACT_GOFORWARD;
		*SetPieces_BackendTempTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 100;
		*SetPieces_BackendVehicleRangeTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 10000;
		SetPieces_BackendMoveSpeed(pVehicle1,SetPiece_ScaledForward(SetPieces_BackendForward(pVehicle1)));
		SetPieces_BackendOccupants(pVehicle1);
		CVehicle* pVehicle2 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn2, record->m_vTarget2);
		if (!pVehicle2) {
			SetPieces_BackendRemoveVehicle(pVehicle1);
//+ rouz edit (ChatGPT)
			// Destroy and release the generated cop car without invoking C++ delete.
			SetPieces_BackendDestroyVehicle(pVehicle1);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			return;
		}
		SetPieces_BackendVehicleStatus(pVehicle2,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle2) = *SetPieces_BackendCruiseSpeed(pVehicle1) = 16;
		*SetPieces_BackendDrivingStyle(pVehicle2) = SETPIECE_DRIVINGSTYLE_PLOUGH_THROUGH;
		*SetPieces_BackendCarMission(pVehicle2) = SETPIECE_MISSION_BLOCKPLAYER_FORWARDANDBACK;
		*SetPieces_BackendTempAction(pVehicle2) = SETPIECE_TEMPACT_GOFORWARD;
		*SetPieces_BackendTempTime(pVehicle2) = (*Timer_GetTimeInMilliseconds()) + 100;
		*SetPieces_BackendVehicleRangeTime(pVehicle2) = (*Timer_GetTimeInMilliseconds()) + 10000;
		SetPieces_BackendMoveSpeed(pVehicle2,SetPiece_ScaledForward(SetPieces_BackendForward(pVehicle2)));
		SetPieces_BackendOccupants(pVehicle2);
		record->m_nLastTimeCreated = (*Timer_GetTimeInMilliseconds());
		return;
	}
	case SETPIECE_TWOCARSRAMMINGPLAYERFROMSIDE:
	{
		if (SetPieces_BackendWanted() < 2)
			return;
		if (!SetPieces_BackendPlayerVehicle())
			return;
		if (SetPiece_Dot2(SetPieces_BackendPlayerSpeed(),SetPiece_Offset(SetPieces_BackendPlayerCoors(),record->m_vSpawn1)) >= 0.0f)
			return;
		CVehicle* pVehicle1 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn1, record->m_vTarget1);
		if (!pVehicle1)
			return;
		SetPieces_BackendVehicleStatus(pVehicle1,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle1) = *SetPieces_BackendCruiseSpeed(pVehicle1) = 16;
		*SetPieces_BackendDrivingStyle(pVehicle1) = SETPIECE_DRIVINGSTYLE_AVOID_CARS;
		*SetPieces_BackendCarMission(pVehicle1) = SETPIECE_MISSION_RAMCAR_CLOSE;
		*SetPieces_BackendTempAction(pVehicle1) = SETPIECE_TEMPACT_GOFORWARD;
		*SetPieces_BackendTempTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 100;
		*SetPieces_BackendVehicleRangeTime(pVehicle1) = (*Timer_GetTimeInMilliseconds()) + 10000;
		SetPieces_BackendMoveSpeed(pVehicle1,SetPiece_ScaledForward(SetPieces_BackendForward(pVehicle1)));
		SetPieces_BackendOccupants(pVehicle1);
		CVehicle* pVehicle2 = SetPiece_TryToGenerateCopCar(record,record->m_vSpawn2, record->m_vTarget2);
		if (!pVehicle2) {
			SetPieces_BackendRemoveVehicle(pVehicle1);
//+ rouz edit (ChatGPT)
			// Destroy and release the generated cop car without invoking C++ delete.
			SetPieces_BackendDestroyVehicle(pVehicle1);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			return;
		}
		SetPieces_BackendVehicleStatus(pVehicle2,STATUS_PHYSICS);
		*SetPieces_BackendMaxSpeed(pVehicle2) = *SetPieces_BackendCruiseSpeed(pVehicle2) = 16;
		*SetPieces_BackendDrivingStyle(pVehicle2) = SETPIECE_DRIVINGSTYLE_AVOID_CARS;
		*SetPieces_BackendCarMission(pVehicle2) = SETPIECE_MISSION_RAMCAR_CLOSE;
		*SetPieces_BackendTempAction(pVehicle2) = SETPIECE_TEMPACT_GOFORWARD;
		*SetPieces_BackendTempTime(pVehicle2) = (*Timer_GetTimeInMilliseconds()) + 100;
		*SetPieces_BackendVehicleRangeTime(pVehicle2) = (*Timer_GetTimeInMilliseconds()) + 10000;
		SetPieces_BackendMoveSpeed(pVehicle2,SetPiece_ScaledForward(SetPieces_BackendForward(pVehicle2)));
		SetPieces_BackendOccupants(pVehicle2);
		record->m_nLastTimeCreated = (*Timer_GetTimeInMilliseconds());
		return;
	}
	}
}

CVehicle* SetPiece_TryToGenerateCopCar(SetPiece *record,SetPiecePoint vSpawn, SetPiecePoint vTarget)
{
    // Preserve every original set-piece decision, live access and transfer order
    // Construct the original native spawn before applying all placement and acceptance decisions in C
    CVehicle *pVehicle=SetPieces_BackendConstructVehicle();
	SetPieceVector pos={vSpawn.x,vSpawn.y,1000.0f};
	float hitHeight;
	if (SetPieces_BackendVerticalLine(pos,&hitHeight))
		pos.z = hitHeight + SetPieces_BackendHeightAboveRoad(pVehicle);
	SetPieceVector vDirection={vTarget.x-vSpawn.x,vTarget.y-vSpawn.y,0.0f};
	SetPiece_Normalise(&vDirection);
	SetPieces_BackendSetForward(pVehicle,(SetPieceVector){vDirection.x, vDirection.y, 0.0f});
	SetPieces_BackendSetRight(pVehicle,(SetPieceVector){vDirection.y, -vDirection.x, 0.0f});
	SetPieces_BackendSetUp(pVehicle,(SetPieceVector){0.0f, 0.0f, 1.0f});
	SetPieces_BackendVehiclePosition(pVehicle,pos);
	int16_t total;
	SetPieces_BackendColliding(pos,SetPieces_BackendVehicleRadius(pVehicle),&total);
	if (total != 0) {
//+ rouz edit (ChatGPT)
		// Destroy and release the set-piece police car without invoking C++ delete.
		SetPieces_BackendDestroyVehicle(pVehicle);
		//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
		return NULL;
	}
	SetPieces_BackendLawEnforcer(pVehicle,true);
	SetPieces_BackendAddVehicle(pVehicle);
	return pVehicle;
}

CCopPed* SetPiece_TryToGenerateCopPed(SetPiece *record,SetPiecePoint vSpawn)
{
    // Preserve every original set-piece decision, live access and transfer order
    // Construct the original native spawn before applying all placement and acceptance decisions in C
    CCopPed *pCop=SetPieces_BackendConstructCop();
	SetPieceVector pos={vSpawn.x,vSpawn.y,1000.0f};
	float hitHeight;
	if (SetPieces_BackendVerticalLine(pos,&hitHeight))
		pos.z = hitHeight + 0.9f;
	SetPieces_BackendCopPosition(pCop,pos);
	int16_t total;
	SetPieces_BackendColliding(pos,SetPieces_BackendCopRadius(pCop),&total);
	if (total != 0) {
//+ rouz edit (ChatGPT)
		// Destroy and release the set-piece cop without invoking C++ delete.
		SetPieces_BackendDestroyCop(pCop);
		//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
		return NULL;
	}
	SetPieces_BackendAddCop(pCop);
	return pCop;
}
//- rouz edit (ChatGPT)
