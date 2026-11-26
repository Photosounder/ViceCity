//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "Script.h"
#include "ScriptCommands.h"

#include "CarCtrl.h"
#include "BulletInfo.h"
#include "General.h"
#include "Lines.h"
#include "Messages.h"
#include "Pad.h"
#include "Pools.h"
#include "Population.h"
#include "RpAnimBlend.h"
#include "SaveBuf.h"
#include "ScriptArea.h"
#include "Shadows.h"
#include "SpecialFX.h"
#include "World.h"
#include "main.h"
#include "SaveBuf.h"

void CRunningScript::UpdateCompareFlag(bool flag)
{
	// Delegate to the C implementation
	int condResult = m_bCondResult;
	ScriptBC_UpdateCompareFlag(flag, m_bNotFlag, &m_nAndOrState, &condResult);
	m_bCondResult = condResult != 0;
}

void CRunningScript::LocatePlayerCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CPlayerInfo* pPlayerInfo = &World_Players[ScriptParams[0]];
	switch (command) {
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
		if (!TheScripts_IsPlayerStopped(pPlayerInfo)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	} else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (!decided) {
		CVector pos = pPlayerInfo->GetPos();
		result = false;
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
		} else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
		}
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_PLAYER_ANY_MEANS_2D:
			case COMMAND_LOCATE_PLAYER_ANY_MEANS_3D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_2D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ANY_MEANS_3D:
				result = true;
				break;
			case COMMAND_LOCATE_PLAYER_ON_FOOT_2D:
			case COMMAND_LOCATE_PLAYER_ON_FOOT_3D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_2D:
			case COMMAND_LOCATE_STOPPED_PLAYER_ON_FOOT_3D:
				result = !pPlayerInfo->m_pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_PLAYER_IN_CAR_2D:
			case COMMAND_LOCATE_PLAYER_IN_CAR_3D:
			case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_2D:
			case COMMAND_LOCATE_STOPPED_PLAYER_IN_CAR_3D:
				result = pPlayerInfo->m_pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocatePlayerCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPlayerInfo* pPlayerInfo = &World_Players[ScriptParams[0]];
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pTarget = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[1]));
	//- rouz edit (ChatGPT)
	script_assert(pTarget);
	CVector pos = pPlayerInfo->GetPos();
	if (pTarget->bInVehicle) {
		X = pTarget->m_pMyVehicle->GetPosition().x;
		Y = pTarget->m_pMyVehicle->GetPosition().y;
		Z = pTarget->m_pMyVehicle->GetPosition().z;
	} else {
		X = pTarget->GetPosition().x;
		Y = pTarget->GetPosition().y;
		Z = pTarget->GetPosition().z;
	}
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
	}
	else {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_2D:
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CHAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_2D:
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CHAR_3D:
			result = !pPlayerInfo->m_pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_2D:
		case COMMAND_LOCATE_PLAYER_IN_CAR_CHAR_3D:
			result = pPlayerInfo->m_pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
#ifdef FIX_BUGS
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
#else
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dX, b3D ? Z : MAP_Z_LOW_LIMIT);
#endif
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocatePlayerCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_3D:
	case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_3D:
	case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CPlayerInfo* pPlayerInfo = &World_Players[ScriptParams[0]];
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CVehicle* pTarget = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[1]));
	//- rouz edit (ChatGPT)
	script_assert(pTarget);
	CVector pos = pPlayerInfo->GetPos();
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
	}
	else {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_2D:
		case COMMAND_LOCATE_PLAYER_ANY_MEANS_CAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_2D:
		case COMMAND_LOCATE_PLAYER_ON_FOOT_CAR_3D:
			result = !pPlayerInfo->m_pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_2D:
		case COMMAND_LOCATE_PLAYER_IN_CAR_CAR_3D:
			result = pPlayerInfo->m_pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pPed);
	CVector pos = pPed->InVehicle() ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	switch (command) {
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
		if (!TheScripts_IsPedStopped(pPed)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	}
	else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
		}
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_CHAR_ANY_MEANS_2D:
			case COMMAND_LOCATE_CHAR_ANY_MEANS_3D:
			case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_2D:
			case COMMAND_LOCATE_STOPPED_CHAR_ANY_MEANS_3D:
				result = true;
				break;
			case COMMAND_LOCATE_CHAR_ON_FOOT_2D:
			case COMMAND_LOCATE_CHAR_ON_FOOT_3D:
			case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_2D:
			case COMMAND_LOCATE_STOPPED_CHAR_ON_FOOT_3D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_CHAR_IN_CAR_2D:
			case COMMAND_LOCATE_CHAR_IN_CAR_3D:
			case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_2D:
			case COMMAND_LOCATE_STOPPED_CHAR_IN_CAR_3D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pPed);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pTarget = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[1]));
	//- rouz edit (ChatGPT)
	script_assert(pTarget);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	if (pTarget->bInVehicle) {
		X = pTarget->m_pMyVehicle->GetPosition().x;
		Y = pTarget->m_pMyVehicle->GetPosition().y;
		Z = pTarget->m_pMyVehicle->GetPosition().z;
	}
	else {
		X = pTarget->GetPosition().x;
		Y = pTarget->GetPosition().y;
		Z = pTarget->GetPosition().z;
	}
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
	}
	else {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_2D:
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CHAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_2D:
		case COMMAND_LOCATE_CHAR_ON_FOOT_CHAR_3D:
			result = !pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_2D:
		case COMMAND_LOCATE_CHAR_IN_CAR_CHAR_3D:
			result = pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
#ifdef FIX_BUGS
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
#else
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dX, b3D ? Z : MAP_Z_LOW_LIMIT);
#endif
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pPed);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CVehicle* pTarget = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[1]));
	//- rouz edit (ChatGPT)
	script_assert(pTarget);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
	}
	else {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_2D:
		case COMMAND_LOCATE_CHAR_ANY_MEANS_CAR_3D:
			result = true;
			break;
		case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_2D:
		case COMMAND_LOCATE_CHAR_ON_FOOT_CAR_3D:
			result = !pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_CHAR_IN_CAR_CAR_2D:
		case COMMAND_LOCATE_CHAR_IN_CAR_CAR_3D:
			result = pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCharObjectCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
	case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pPed);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CObject* pTarget = ((CObject*)CPool_GetAt(Pools_GetObjectPool(), ScriptParams[1]));
	//- rouz edit (ChatGPT)
	script_assert(pTarget);
	CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = false;
	bool in_area;
	if (b3D) {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
	}
	else {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
	}
	if (in_area) {
		switch (command) {
		case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_2D:
		case COMMAND_LOCATE_CHAR_ANY_MEANS_OBJECT_3D:
			result = true;
			break;
		case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_2D:
		case COMMAND_LOCATE_CHAR_ON_FOOT_OBJECT_3D:
			result = !pPed->bInVehicle;
			break;
		case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_2D:
		case COMMAND_LOCATE_CHAR_IN_CAR_OBJECT_3D:
			result = pPed->bInVehicle;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_CAR_3D:
	case COMMAND_LOCATE_STOPPED_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pVehicle);
	CVector pos = pVehicle->GetPosition();
	switch (command) {
	case COMMAND_LOCATE_STOPPED_CAR_2D:
	case COMMAND_LOCATE_STOPPED_CAR_3D:
		if (!TheScripts_IsVehicleStopped(pVehicle)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	}
	else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
		}
		result = in_area;
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateObjectCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_OBJECT_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CObject* pObject = ((CObject*)CPool_GetAt(Pools_GetObjectPool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pObject);
	CVector pos = pObject->GetPosition();
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	}
	else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	result = false;
	bool in_area;
	if (b3D) {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
	}
	else {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
	}
	result = in_area;
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateSniperBulletCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_SNIPER_BULLET_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 7 : 5);
	X = *(float*)&ScriptParams[0];
	Y = *(float*)&ScriptParams[1];
	if (b3D) {
		Z = *(float*)&ScriptParams[2];
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		dZ = *(float*)&ScriptParams[5];
		debug = ScriptParams[6];
	}
	else {
		dX = *(float*)&ScriptParams[2];
		dY = *(float*)&ScriptParams[3];
		debug = ScriptParams[4];
	}
	// Call the standalone bullet operation
	result = BulletInfo_TestForSniperBullet(X - dX, X + dX, Y - dY, Y + dY, b3D ? Z - dZ : -1000.0f, b3D ? Z + dZ : 1000.0f);
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::PlayerInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_PLAYER_IN_AREA_3D:
	case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	CPlayerInfo* pPlayerInfo = &World_Players[ScriptParams[0]];
	switch (command) {
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_2D:
		if (!TheScripts_IsPlayerStopped(pPlayerInfo)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		// Delegate the sorting of the corners to the C implementation
		ScriptArea_SortPair(&infZ, &supZ);
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infX, &supX);
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infY, &supY);
	if (!decided) {
		CVector pos = pPlayerInfo->GetPos();
		result = false;
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InMinMaxBox3D(infX, infY, infZ, supX, supY, supZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InMinMaxBox2D(infX, infY, supX, supY, pos.x, pos.y);
		}
		if (in_area) {
			switch (command) {
			case COMMAND_IS_PLAYER_IN_AREA_2D:
			case COMMAND_IS_PLAYER_IN_AREA_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_3D:
				result = true;
				break;
			case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_IN_AREA_ON_FOOT_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_ON_FOOT_3D:
				result = !pPlayerInfo->m_pPed->bInVehicle;
				break;
			case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_IN_AREA_IN_CAR_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_AREA_IN_CAR_3D:
				result = pPlayerInfo->m_pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			TheScripts_DrawDebugSquare(infX, infY, supX, supY);
	}
}

void CRunningScript::PlayerInAngledAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ, side2length;
	switch (command) {
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 9 : 7);
	CPlayerInfo* pPlayerInfo = &World_Players[ScriptParams[0]];
	switch (command) {
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
	case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
		if (!TheScripts_IsPlayerStopped(pPlayerInfo)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		// Delegate the sorting of the corners to the C implementation
		ScriptArea_SortPair(&infZ, &supZ);
		side2length = *(float*)&ScriptParams[7];
		debug = ScriptParams[8];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		side2length = *(float*)&ScriptParams[5];
		debug = ScriptParams[6];
	}
	// Delegate the geometry of the angled area to the C implementation
	ScriptAngledArea angled;
	ScriptArea_AngledGeometry(infX, infY, supX, supY, side2length, &angled);
	float rotatedSupX = angled.rotatedSupX, rotatedSupY = angled.rotatedSupY, rotatedInfX = angled.rotatedInfX, rotatedInfY = angled.rotatedInfY;
	if (!decided) {
		CVector pos = pPlayerInfo->GetPos();
		result = false;
		// Delegate the test of the point to the C implementation
		bool in_area = ScriptArea_InAngledArea(&angled, pos.x, pos.y, pos.z, infZ, supZ, b3D);

		if (in_area) {
			switch (command) {
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_2D:
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_3D:
				result = true;
				break;
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_ON_FOOT_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_ON_FOOT_3D:
				result = !pPlayerInfo->m_pPed->bInVehicle;
				break;
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_IN_ANGLED_AREA_IN_CAR_3D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_2D:
			case COMMAND_IS_PLAYER_STOPPED_IN_ANGLED_AREA_IN_CAR_3D:
				result = pPlayerInfo->m_pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantAngledArea((uintptr)this + m_nIp, infX, infY, supX, supY,
			rotatedSupX, rotatedSupY, rotatedInfX, rotatedInfY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugAngledCube(infX, infY, infZ, supX, supY, supZ,
				rotatedSupX, rotatedSupY, rotatedInfX, rotatedInfY);
		else
			TheScripts_DrawDebugAngledSquare(infX, infY, supX, supY,
				rotatedSupX, rotatedSupY, rotatedInfX, rotatedInfY);
	}
}

void CRunningScript::CharInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_CHAR_IN_AREA_3D:
	case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pPed);
	CVector pos = pPed->InVehicle() ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
	switch (command) {
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
		if (!TheScripts_IsPedStopped(pPed)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		// Delegate the sorting of the corners to the C implementation
		ScriptArea_SortPair(&infZ, &supZ);
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infX, &supX);
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infY, &supY);
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InMinMaxBox3D(infX, infY, infZ, supX, supY, supZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InMinMaxBox2D(infX, infY, supX, supY, pos.x, pos.y);
		}
		if (in_area) {
			switch (command) {
			case COMMAND_IS_CHAR_IN_AREA_2D:
			case COMMAND_IS_CHAR_IN_AREA_3D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_2D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_3D:
				result = true;
				break;
			case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_CHAR_IN_AREA_ON_FOOT_3D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_2D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_ON_FOOT_3D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_IS_CHAR_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_CHAR_IN_AREA_IN_CAR_3D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_2D:
			case COMMAND_IS_CHAR_STOPPED_IN_AREA_IN_CAR_3D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			TheScripts_DrawDebugSquare(infX, infY, supX, supY);
	}
}

void CRunningScript::CarInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_CAR_IN_AREA_3D:
	case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pVehicle);
	CVector pos = pVehicle->GetPosition();
	switch (command) {
	case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
	case COMMAND_IS_CAR_STOPPED_IN_AREA_2D:
		if (!TheScripts_IsVehicleStopped(pVehicle)) {
			result = false;
			decided = true;
		}
		break;
	default:
		break;
	}
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		// Delegate the sorting of the corners to the C implementation
		ScriptArea_SortPair(&infZ, &supZ);
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infX, &supX);
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infY, &supY);
	if (!decided) {
		result = false;
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InMinMaxBox3D(infX, infY, infZ, supX, supY, supZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InMinMaxBox2D(infX, infY, supX, supY, pos.x, pos.y);
		}
		if (in_area) {
			switch (command) {
			case COMMAND_IS_CAR_IN_AREA_2D:
			case COMMAND_IS_CAR_IN_AREA_3D:
			case COMMAND_IS_CAR_STOPPED_IN_AREA_2D:
			case COMMAND_IS_CAR_STOPPED_IN_AREA_3D:
				result = true;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			TheScripts_DrawDebugSquare(infX, infY, supX, supY);
	}
}

void CRunningScript::ObjectInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_OBJECT_IN_AREA_3D:
		b3D = true;
		break;
	default:
		b3D = false;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CObject* pObject = ((CObject*)CPool_GetAt(Pools_GetObjectPool(), ScriptParams[0]));
	//- rouz edit (ChatGPT)
	script_assert(pObject);
	CVector pos = pObject->GetPosition();
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		// Delegate the sorting of the corners to the C implementation
		ScriptArea_SortPair(&infZ, &supZ);
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infX, &supX);
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infY, &supY);
	result = false;
	bool in_area;
	if (b3D) {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InMinMaxBox3D(infX, infY, infZ, supX, supY, supZ, pos.x, pos.y, pos.z);
	}
	else {
		// Delegate the test of the box to the C implementation
		in_area = ScriptArea_InMinMaxBox2D(infX, infY, supX, supY, pos.x, pos.y);
	}
	if (in_area) {
		switch (command) {
		case COMMAND_IS_OBJECT_IN_AREA_2D:
		case COMMAND_IS_OBJECT_IN_AREA_3D:
			result = true;
			break;
		default:
			script_assert(false);
			break;
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			TheScripts_DrawDebugSquare(infX, infY, supX, supY);
	}
}

void CRunningScript::DoDeatharrestCheck()
{
	if (!m_bDeatharrestEnabled)
		return;
	if (!TheScripts_IsPlayerOnAMission())
		return;
	CPlayerInfo* pPlayer = &World_Players[World_PlayerInFocus];
	if (!pPlayer->IsRestartingAfterDeath() && !pPlayer->IsRestartingAfterArrest())
		return;
#ifdef MISSION_REPLAY
	if (AllowMissionReplay != MISSION_RETRY_STAGE_WAIT_FOR_TIMER_AFTER_RESTART && AllowMissionReplay != MISSION_RETRY_STAGE_NORMAL)
		return;
	if (AllowMissionReplay == MISSION_RETRY_STAGE_WAIT_FOR_TIMER_AFTER_RESTART)
		AllowMissionReplay = MISSION_RETRY_STAGE_NORMAL;
	if (CanAllowMissionReplay())
		AllowMissionReplay = MISSION_RETRY_STAGE_WAIT_FOR_SCRIPT_TO_TERMINATE;
#endif
	script_assert(m_nStackPointer > 0);
	while (m_nStackPointer > 1)
		--m_nStackPointer;
	m_nIp = m_anStack[--m_nStackPointer];
	Messages_ClearSmallMessagesOnly();
	*(int32*)&TheScripts_ScriptSpace[TheScripts_OnAMissionFlag] = 0;
	m_bDeatharrestExecuted = true;
	m_nWakeTime = 0;
}

int16 CRunningScript::GetPadState(uint16 pad, uint16 button)
{
	CPad* pPad = CPad::GetPad(pad);
	switch (button) {
	case 0: return pPad->NewState.LeftStickX;
	case 1: return pPad->NewState.LeftStickY;
	case 2: return pPad->NewState.RightStickX;
	case 3: return pPad->NewState.RightStickY;
	case 4: return pPad->NewState.LeftShoulder1;
	case 5: return pPad->NewState.LeftShoulder2;
	case 6: return pPad->NewState.RightShoulder1;
	case 7: return pPad->NewState.RightShoulder2;
	case 8: return pPad->NewState.DPadUp;
	case 9: return pPad->NewState.DPadDown;
	case 10: return pPad->NewState.DPadLeft;
	case 11: return pPad->NewState.DPadRight;
	case 12: return pPad->NewState.Start;
	case 13: return pPad->NewState.Select;
	case 14: return pPad->NewState.Square;
	case 15: return pPad->NewState.Triangle;
	case 16: return pPad->NewState.Cross;
	case 17: return pPad->NewState.Circle;
	case 18: return pPad->NewState.LeftShock;
	case 19: return pPad->NewState.RightShock;
	default: break;
	}
	return 0;
}

#ifdef GTA_SCRIPT_COLLECTIVE
void CRunningScript::LocateCollectiveCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
	case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
	case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	X = *(float*)&ScriptParams[1];
	Y = *(float*)&ScriptParams[2];
	if (b3D) {
		Z = *(float*)&ScriptParams[3];
		dX = *(float*)&ScriptParams[4];
		dY = *(float*)&ScriptParams[5];
		dZ = *(float*)&ScriptParams[6];
		debug = ScriptParams[7];
	}
	else {
		dX = *(float*)&ScriptParams[3];
		dY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CTheScripts::CollectiveArray[i].pedIndex));
		//- rouz edit (ChatGPT)
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		switch (command) {
		case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
		case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
		case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
			if (!TheScripts_IsPedStopped(pPed)) {
				result = false;
				decided = true;
			}
			break;
		default:
			break;
		}
		if (!decided) {
			bool in_area;
			if (b3D) {
				// Delegate the test of the box to the C implementation
				in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
			}
			else {
				// Delegate the test of the box to the C implementation
				in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
			}
			result = false;
			if (in_area) {
				switch (command) {
				case COMMAND_LOCATE_COLL_ANY_MEANS_2D:
				case COMMAND_LOCATE_STOPPED_COLL_ANY_MEANS_2D:
					result = true;
					break;
				case COMMAND_LOCATE_COLL_ON_FOOT_2D:
				case COMMAND_LOCATE_STOPPED_COLL_ON_FOOT_2D:
					result = !pPed->bInVehicle;
					break;
				case COMMAND_LOCATE_COLL_IN_CAR_2D:
				case COMMAND_LOCATE_STOPPED_COLL_IN_CAR_2D:
					result = pPed->bInVehicle;
					break;
				default:
					script_assert(false);
					break;
				}
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCollectiveCharCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_CHAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CHAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CHAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPed* pTarget = ((CPed*)CPool_GetAt(Pools_GetPedPool(), ScriptParams[1]));
	//- rouz edit (ChatGPT)
	script_assert(pTarget);
	if (pTarget->bInVehicle) {
		X = pTarget->m_pMyVehicle->GetPosition().x;
		Y = pTarget->m_pMyVehicle->GetPosition().y;
		Z = pTarget->m_pMyVehicle->GetPosition().z;
	}
	else {
		X = pTarget->GetPosition().x;
		Y = pTarget->GetPosition().y;
		Z = pTarget->GetPosition().z;
	}
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CTheScripts::CollectiveArray[i].pedIndex));
		//- rouz edit (ChatGPT)
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
		}
		result = false;
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_COLL_ANY_MEANS_CHAR_2D:
				result = true;
				break;
			case COMMAND_LOCATE_COLL_ON_FOOT_CHAR_2D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_COLL_IN_CAR_CHAR_2D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCollectiveCarCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_CAR_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_CAR_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_CAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CVehicle* pTarget = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), ScriptParams[1]));
	//- rouz edit (ChatGPT)
	script_assert(pTarget);
	X = pTarget->GetPosition().x;
	Y = pTarget->GetPosition().y;
	Z = pTarget->GetPosition().z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CTheScripts::CollectiveArray[i].pedIndex));
		//- rouz edit (ChatGPT)
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
		}
		result = false;
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_COLL_ANY_MEANS_CAR_2D:
				result = true;
				break;
			case COMMAND_LOCATE_COLL_ON_FOOT_CAR_2D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_COLL_IN_CAR_CAR_2D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::LocateCollectivePlayerCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug;
	float X, Y, Z, dX, dY, dZ;
	switch (command) {
	case COMMAND_LOCATE_COLL_ANY_MEANS_PLAYER_2D:
	case COMMAND_LOCATE_COLL_ON_FOOT_PLAYER_2D:
	case COMMAND_LOCATE_COLL_IN_CAR_PLAYER_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 6 : 5);
	CVector pos = World_Players[ScriptParams[1]].GetPos();
	X = pos.x;
	Y = pos.y;
	Z = pos.z;
	dX = *(float*)&ScriptParams[2];
	dY = *(float*)&ScriptParams[3];
	if (b3D) {
		dZ = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	else {
		debug = ScriptParams[4];
	}
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CTheScripts::CollectiveArray[i].pedIndex));
		//- rouz edit (ChatGPT)
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		bool in_area;
		if (b3D) {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox3D(X, Y, Z, dX, dY, dZ, pos.x, pos.y, pos.z);
		}
		else {
			// Delegate the test of the box to the C implementation
			in_area = ScriptArea_InCenteredBox2D(X, Y, dX, dY, pos.x, pos.y);
		}
		result = false;
		if (in_area) {
			switch (command) {
			case COMMAND_LOCATE_COLL_ANY_MEANS_PLAYER_2D:
				result = true;
				break;
			case COMMAND_LOCATE_COLL_ON_FOOT_PLAYER_2D:
				result = !pPed->bInVehicle;
				break;
			case COMMAND_LOCATE_COLL_IN_CAR_PLAYER_2D:
				result = pPed->bInVehicle;
				break;
			default:
				script_assert(false);
				break;
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, X - dX, Y - dY, X + dX, Y + dY, b3D ? Z : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(X - dX, Y - dY, Z - dZ, X + dX, Y + dY, Z + dZ);
		else
			TheScripts_DrawDebugSquare(X - dX, Y - dY, X + dX, Y + dY);
	}
}

void CRunningScript::CollectiveInAreaCheckCommand(int32 command, uint32* pIp)
{
	bool b3D, result, debug, decided = false;
	float infX, infY, infZ, supX, supY, supZ;
	switch (command) {
	case COMMAND_IS_COLL_IN_AREA_2D:
	case COMMAND_IS_COLL_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_IN_AREA_IN_CAR_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
	case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
		b3D = false;
		break;
	default:
		b3D = true;
		break;
	}
	CollectParameters(pIp, b3D ? 8 : 6);
	infX = *(float*)&ScriptParams[1];
	infY = *(float*)&ScriptParams[2];
	if (b3D) {
		infZ = *(float*)&ScriptParams[3];
		supX = *(float*)&ScriptParams[4];
		supY = *(float*)&ScriptParams[5];
		supZ = *(float*)&ScriptParams[6];
		// Delegate the sorting of the corners to the C implementation
		ScriptArea_SortPair(&infZ, &supZ);
		debug = ScriptParams[7];
	}
	else {
		supX = *(float*)&ScriptParams[3];
		supY = *(float*)&ScriptParams[4];
		debug = ScriptParams[5];
	}
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infX, &supX);
	// Delegate the sorting of the corners to the C implementation
	ScriptArea_SortPair(&infY, &supY);
	result = true;
	for (int i = 0; i < MAX_NUM_COLLECTIVES && result; i++) {
		if (ScriptParams[0] != CTheScripts::CollectiveArray[i].colIndex)
			continue;
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CTheScripts::CollectiveArray[i].pedIndex));
		//- rouz edit (ChatGPT)
		if (!pPed) {
			CTheScripts::CollectiveArray[i].colIndex = -1;
			CTheScripts::CollectiveArray[i].pedIndex = 0;
			continue;
		}
		CVector pos = pPed->bInVehicle ? pPed->m_pMyVehicle->GetPosition() : pPed->GetPosition();
		switch (command) {
		case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
		case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
		case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
			if (!TheScripts_IsPedStopped(pPed)) {
				result = false;
				decided = true;
			}
			break;
		default:
			break;
		}
		if (!decided) {
			bool in_area;
			if (b3D) {
				// Delegate the test of the box to the C implementation
				in_area = ScriptArea_InMinMaxBox3D(infX, infY, infZ, supX, supY, supZ, pos.x, pos.y, pos.z);
			}
			else {
				// Delegate the test of the box to the C implementation
				in_area = ScriptArea_InMinMaxBox2D(infX, infY, supX, supY, pos.x, pos.y);
			}
			result = false;
			if (in_area) {
				switch (command) {
				case COMMAND_IS_COLL_IN_AREA_2D:
				case COMMAND_IS_COLL_STOPPED_IN_AREA_2D:
					result = true;
					break;
				case COMMAND_IS_COLL_IN_AREA_ON_FOOT_2D:
				case COMMAND_IS_COLL_STOPPED_IN_AREA_ON_FOOT_2D:
					result = !pPed->bInVehicle;
					break;
				case COMMAND_IS_COLL_IN_AREA_IN_CAR_2D:
				case COMMAND_IS_COLL_STOPPED_IN_AREA_IN_CAR_2D:
					result = pPed->bInVehicle;
					break;
				default:
					script_assert(false);
					break;
				}
			}
		}
	}
	UpdateCompareFlag(result);
	if (debug)
		TheScripts_HighlightImportantArea((uintptr)this + m_nIp, infX, infY, supX, supY, b3D ? (infZ + supZ) / 2 : MAP_Z_LOW_LIMIT);
	if (TheScripts_DbgFlag) {
		if (b3D)
			TheScripts_DrawDebugCube(infX, infY, infZ, supX, supY, supZ);
		else
			TheScripts_DrawDebugSquare(infX, infY, supX, supY);
	}
}
#endif

bool CRunningScript::CheckDamagedWeaponType(int32 actual, int32 type)
{
	if (actual == -1)
		return false;

	if (type == WEAPONTYPE_ANYMELEE) {
		if (actual <= WEAPONTYPE_CHAINSAW)
			return true;
		if (actual >= WEAPONTYPE_GRENADE && actual <= WEAPONTYPE_UNIDENTIFIED)
			return false;
		return false;
	}

	if (type != WEAPONTYPE_ANYWEAPON)
		return false;

	switch (actual) {
	case WEAPONTYPE_UNARMED:
	case WEAPONTYPE_BRASSKNUCKLE:
	case WEAPONTYPE_SCREWDRIVER:
	case WEAPONTYPE_GOLFCLUB:
	case WEAPONTYPE_NIGHTSTICK:
	case WEAPONTYPE_KNIFE:
	case WEAPONTYPE_BASEBALLBAT:
	case WEAPONTYPE_HAMMER:
	case WEAPONTYPE_CLEAVER:
	case WEAPONTYPE_MACHETE:
	case WEAPONTYPE_KATANA:
	case WEAPONTYPE_CHAINSAW:
	case WEAPONTYPE_GRENADE:
	case WEAPONTYPE_DETONATOR_GRENADE:
	case WEAPONTYPE_TEARGAS:
	case WEAPONTYPE_MOLOTOV:
	case WEAPONTYPE_ROCKET:
	case WEAPONTYPE_COLT45:
	case WEAPONTYPE_PYTHON:
	case WEAPONTYPE_SHOTGUN:
	case WEAPONTYPE_SPAS12_SHOTGUN:
	case WEAPONTYPE_STUBBY_SHOTGUN:
	case WEAPONTYPE_TEC9:
	case WEAPONTYPE_UZI:
	case WEAPONTYPE_SILENCED_INGRAM:
	case WEAPONTYPE_MP5:
	case WEAPONTYPE_M4:
	case WEAPONTYPE_RUGER:
	case WEAPONTYPE_SNIPERRIFLE:
	case WEAPONTYPE_LASERSCOPE:
	case WEAPONTYPE_ROCKETLAUNCHER:
	case WEAPONTYPE_FLAMETHROWER:
	case WEAPONTYPE_M60:
	case WEAPONTYPE_MINIGUN:
	case WEAPONTYPE_DETONATOR:
	case WEAPONTYPE_HELICANNON:
	case WEAPONTYPE_CAMERA:
	case WEAPONTYPE_EXPLOSION:
	case WEAPONTYPE_UZI_DRIVEBY:
		return true;
	case WEAPONTYPE_HEALTH:
	case WEAPONTYPE_ARMOUR:
	case WEAPONTYPE_RAMMEDBYCAR:
	case WEAPONTYPE_RUNOVERBYCAR:
	case WEAPONTYPE_DROWNING:
	case WEAPONTYPE_FALL:
	case WEAPONTYPE_UNIDENTIFIED:
		return false;
	}

	return false;
}

void TheScripts_PrintListSizes()
{
	int active = 0;
	int idle = 0;

	for (CRunningScript* pScript = TheScripts_pActiveScripts; pScript; pScript = pScript->GetNext())
		active++;
	for (CRunningScript* pScript = TheScripts_pIdleScripts; pScript; pScript = pScript->GetNext())
		idle++;

	debug("active: %d, idle: %d", active, idle);
}

uint32 DbgLineColour = 0x0000FFFF; // r = 0, g = 0, b = 255, a = 255

void TheScripts_DrawDebugSquare(float infX, float infY, float supX, float supY)
{
	CColPoint tmpCP;
	CEntity* tmpEP;
	CVector p1, p2, p3, p4;
	p1 = CVector(infX, infY, -1000.0f);
	World_ProcessVerticalLine(p1, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p1.z = 2.0f + tmpCP.point.z;
	p2 = CVector(supX, supY, -1000.0f);
	World_ProcessVerticalLine(p2, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p2.z = 2.0f + tmpCP.point.z;
	p3 = CVector(infX, supY, -1000.0f);
	World_ProcessVerticalLine(p3, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p3.z = 2.0f + tmpCP.point.z;
	p4 = CVector(supX, infY, -1000.0f);
	World_ProcessVerticalLine(p4, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p4.z = 2.0f + tmpCP.point.z;
	TheScripts_ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(p3.x, p3.y, p3.z, p4.x, p4.y, p4.z, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(p4.x, p4.y, p4.z, p1.x, p1.y, p1.z, DbgLineColour, DbgLineColour);
}

void TheScripts_DrawDebugAngledSquare(float infX, float infY, float supX, float supY, float rotSupX, float rotSupY, float rotInfX, float rotInfY)
{
	CColPoint tmpCP;
	CEntity* tmpEP;
	CVector p1, p2, p3, p4;
	p1 = CVector(infX, infY, -1000.0f);
	World_ProcessVerticalLine(p1, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p1.z = 2.0f + tmpCP.point.z;
	p2 = CVector(supX, supY, -1000.0f);
	World_ProcessVerticalLine(p2, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p2.z = 2.0f + tmpCP.point.z;
	p3 = CVector(rotSupX, rotSupY, -1000.0f);
	World_ProcessVerticalLine(p3, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p3.z = 2.0f + tmpCP.point.z;
	p4 = CVector(rotInfX, rotInfY, -1000.0f);
	World_ProcessVerticalLine(p4, 1000.0f, tmpCP, tmpEP, true, false, false, false, true, false, nil);
	p4.z = 2.0f + tmpCP.point.z;
	TheScripts_ScriptDebugLine3D(p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(p2.x, p2.y, p2.z, p3.x, p3.y, p3.z, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(p3.x, p3.y, p3.z, p4.x, p4.y, p4.z, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(p4.x, p4.y, p4.z, p1.x, p1.y, p1.z, DbgLineColour, DbgLineColour);
}

void TheScripts_DrawDebugCube(float infX, float infY, float infZ, float supX, float supY, float supZ)
{
	TheScripts_ScriptDebugLine3D(infX, infY, infZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, infY, infZ, supX, supY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, supY, infZ, infX, supY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(infX, supY, infZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(infX, infY, supZ, supX, infY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, infY, supZ, supX, supY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, supY, supZ, infX, supY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(infX, supY, supZ, infX, infY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(infX, infY, supZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, infY, supZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, supY, supZ, supX, supY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(infX, supY, supZ, infX, supY, infZ, DbgLineColour, DbgLineColour);
}

void TheScripts_DrawDebugAngledCube(float infX, float infY, float infZ, float supX, float supY, float supZ, float rotSupX, float rotSupY, float rotInfX, float rotInfY)
{
	TheScripts_ScriptDebugLine3D(infX, infY, infZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, infY, infZ, rotSupX, rotSupY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(rotSupX, rotSupY, infZ, rotInfX, rotInfY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(rotInfX, rotInfY, infZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(infX, infY, supZ, supX, infY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, infY, supZ, rotSupX, rotSupY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(rotSupX, rotSupY, rotInfX, rotInfY, supY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(rotInfX, rotInfY, supZ, infX, infY, supZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(infX, infY, supZ, infX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(supX, infY, supZ, supX, infY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(rotSupX, rotSupY, supZ, rotSupX, rotSupY, infZ, DbgLineColour, DbgLineColour);
	TheScripts_ScriptDebugLine3D(rotInfX, rotInfY, supZ, rotInfX, rotInfY, infZ, DbgLineColour, DbgLineColour);
}

void TheScripts_ScriptDebugLine3D(float x1, float y1, float z1, float x2, float y2, float z2, uint32 col, uint32 col2)
{
	if (TheScripts_NumScriptDebugLines >= MAX_NUM_STORED_LINES)
		return;
	TheScripts_aStoredLines[TheScripts_NumScriptDebugLines].vecInf = CVector(x1, y1, z1);
	TheScripts_aStoredLines[TheScripts_NumScriptDebugLines].vecSup = CVector(x2, y2, z2);
	TheScripts_aStoredLines[TheScripts_NumScriptDebugLines].color1 = col;
	TheScripts_aStoredLines[TheScripts_NumScriptDebugLines++].color2 = col2;
}

void TheScripts_RenderTheScriptDebugLines()
{
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)1);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)1);
	for (int i = 0; i < TheScripts_NumScriptDebugLines; i++) {
		// Draw the original endpoints and colors through the C line API
		Lines_RenderLineWithClipping(
			TheScripts_aStoredLines[i].vecInf.x,
			TheScripts_aStoredLines[i].vecInf.y,
			TheScripts_aStoredLines[i].vecInf.z,
			TheScripts_aStoredLines[i].vecSup.x,
			TheScripts_aStoredLines[i].vecSup.y,
			TheScripts_aStoredLines[i].vecSup.z,
			TheScripts_aStoredLines[i].color1,
			TheScripts_aStoredLines[i].color2);
	}
	TheScripts_NumScriptDebugLines = 0;
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)0);
}

#define SCRIPT_DATA_SIZE sizeof(TheScripts_OnAMissionFlag) +\
	4 * sizeof(uint32) * MAX_NUM_BUILDING_SWAPS + 2 * sizeof(uint32) * MAX_NUM_INVISIBILITY_SETTINGS + 5 * sizeof(uint32)

void TheScripts_SaveAllScripts(uint8* buf, uint32* size)
{
INITSAVEBUF
	uint32 varSpace = TheScripts_GetSizeOfVariableSpace();
	uint32 runningScripts = 0;
	for (CRunningScript* pScript = TheScripts_pActiveScripts; pScript; pScript = pScript->GetNext())
		runningScripts++;
	*size = CRunningScript::nSaveStructSize * runningScripts + varSpace + SCRIPT_DATA_SIZE + SAVE_HEADER_SIZE + 3 * sizeof(uint32);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveHeader(&buf, 'S', 'C', 'R', '\0', *size - SAVE_HEADER_SIZE);
	WriteSaveBuf(&buf, &varSpace, sizeof(varSpace));
	//- rouz edit (ChatGPT)
	for (uint32 i = 0; i < varSpace; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &TheScripts_ScriptSpace[i], sizeof(TheScripts_ScriptSpace[i]));
		//- rouz edit (ChatGPT)
#ifdef CHECK_STRUCT_SIZES
	static_assert(SCRIPT_DATA_SIZE == 968, "CTheScripts::SaveAllScripts");
#endif
	uint32 script_data_size = SCRIPT_DATA_SIZE;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &script_data_size, sizeof(script_data_size));
	WriteSaveBuf(&buf, &TheScripts_OnAMissionFlag, sizeof(TheScripts_OnAMissionFlag));
	WriteSaveBuf(&buf, &TheScripts_LastMissionPassedTime, sizeof(TheScripts_LastMissionPassedTime));
	//- rouz edit (ChatGPT)
	for (uint32 i = 0; i < MAX_NUM_BUILDING_SWAPS; i++) {
		CBuilding* pBuilding = TheScripts_BuildingSwapArray[i].m_pBuilding;
		uint32 type, handle;
		if (!pBuilding) {
			type = 0;
			handle = 0;
		} else if (pBuilding->GetIsATreadable()) {
			type = 1;
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			handle = CPool_GetJustIndex_NoFreeAssert(Pools_GetTreadablePool(), (CTreadable*)pBuilding) + 1;
			//- rouz edit (ChatGPT)
		} else {
			type = 2;
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			handle = CPool_GetJustIndex_NoFreeAssert(Pools_GetBuildingPool(), pBuilding) + 1;
			//- rouz edit (ChatGPT)
		}
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &type, sizeof(type));
		WriteSaveBuf(&buf, &handle, sizeof(handle));
		WriteSaveBuf(&buf, &TheScripts_BuildingSwapArray[i].m_nNewModel, sizeof(TheScripts_BuildingSwapArray[i].m_nNewModel));
		WriteSaveBuf(&buf, &TheScripts_BuildingSwapArray[i].m_nOldModel, sizeof(TheScripts_BuildingSwapArray[i].m_nOldModel));
		//- rouz edit (ChatGPT)
	}
	for (uint32 i = 0; i < MAX_NUM_INVISIBILITY_SETTINGS; i++) {
		CEntity* pEntity = TheScripts_InvisibilitySettingArray[i];
		uint32 type, handle;
		if (!pEntity) {
			type = 0;
			handle = 0;
		} else {
			switch (pEntity->GetType()) {
			case ENTITY_TYPE_BUILDING:
				if (((CBuilding*)pEntity)->GetIsATreadable()) {
					type = 1;
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					handle = CPool_GetJustIndex_NoFreeAssert(Pools_GetTreadablePool(), (CTreadable*)pEntity) + 1;
					//- rouz edit (ChatGPT)
				} else {
					type = 2;
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					handle = CPool_GetJustIndex_NoFreeAssert(Pools_GetBuildingPool(), (CBuilding*)pEntity) + 1;
					//- rouz edit (ChatGPT)
				}
				break;
			case ENTITY_TYPE_OBJECT:
				type = 3;
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				handle = CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), (CObject*)pEntity) + 1;
				//- rouz edit (ChatGPT)
				break;
			case ENTITY_TYPE_DUMMY:
				type = 4;
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				handle = CPool_GetJustIndex_NoFreeAssert(Pools_GetDummyPool(), (CDummy*)pEntity) + 1;
				//- rouz edit (ChatGPT)
			default: break;
			}
		}
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &type, sizeof(type));
		WriteSaveBuf(&buf, &handle, sizeof(handle));
		//- rouz edit (ChatGPT)
	}
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &TheScripts_bUsingAMultiScriptFile, sizeof(TheScripts_bUsingAMultiScriptFile));
	WriteSaveBuf(&buf, &TheScripts_bPlayerHasMetDebbieHarry, sizeof(TheScripts_bPlayerHasMetDebbieHarry));
	{
		// Materialize the saved value with its original serialized type
		uint16 saveValue = (uint16)0;
		WriteSaveBuf(&buf, &saveValue, sizeof(saveValue));
	}
	WriteSaveBuf(&buf, &TheScripts_MainScriptSize, sizeof(TheScripts_MainScriptSize));
	WriteSaveBuf(&buf, &TheScripts_LargestMissionScriptSize, sizeof(TheScripts_LargestMissionScriptSize));
	WriteSaveBuf(&buf, &TheScripts_NumberOfMissionScripts, sizeof(TheScripts_NumberOfMissionScripts));
	WriteSaveBuf(&buf, &TheScripts_NumberOfExclusiveMissionScripts, sizeof(TheScripts_NumberOfExclusiveMissionScripts));
	WriteSaveBuf(&buf, &runningScripts, sizeof(runningScripts));
	//- rouz edit (ChatGPT)
	for (CRunningScript* pScript = TheScripts_pActiveScripts; pScript; pScript = pScript->GetNext())
		pScript->Save(buf);
VALIDATESAVEBUF(*size)
}

void TheScripts_LoadAllScripts(uint8* buf, uint32 size)
{
	TheScripts_Init();
INITSAVEBUF
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	CheckSaveHeader(&buf, 'S', 'C', 'R', '\0', size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)
	uint32 varSpace, type, handle;
	uint32 tmp;

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&varSpace, &buf, sizeof(varSpace));
	//- rouz edit (ChatGPT)
	for (uint32 i = 0; i < varSpace; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&TheScripts_ScriptSpace[i], &buf, sizeof(TheScripts_ScriptSpace[i]));
		//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tmp, &buf, sizeof(tmp));
	//- rouz edit (ChatGPT)
	script_assert(tmp == SCRIPT_DATA_SIZE);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&TheScripts_OnAMissionFlag, &buf, sizeof(TheScripts_OnAMissionFlag));
	ReadSaveBuf(&TheScripts_LastMissionPassedTime, &buf, sizeof(TheScripts_LastMissionPassedTime));
	//- rouz edit (ChatGPT)
	for (uint32 i = 0; i < MAX_NUM_BUILDING_SWAPS; i++) {
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&type, &buf, sizeof(type));
		ReadSaveBuf(&handle, &buf, sizeof(handle));
		//- rouz edit (ChatGPT)
		switch (type) {
		case 0:
			TheScripts_BuildingSwapArray[i].m_pBuilding = nil;
			break;
		case 1:
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			TheScripts_BuildingSwapArray[i].m_pBuilding = ((CTreadable*)CPool_GetSlot(Pools_GetTreadablePool(), handle - 1));
			//- rouz edit (ChatGPT)
			break;
		case 2:
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			TheScripts_BuildingSwapArray[i].m_pBuilding = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), handle - 1));
			//- rouz edit (ChatGPT)
			break;
		default:
			script_assert(false);
		}
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&TheScripts_BuildingSwapArray[i].m_nNewModel, &buf, sizeof(TheScripts_BuildingSwapArray[i].m_nNewModel));
		ReadSaveBuf(&TheScripts_BuildingSwapArray[i].m_nOldModel, &buf, sizeof(TheScripts_BuildingSwapArray[i].m_nOldModel));
		//- rouz edit (ChatGPT)
		if (TheScripts_BuildingSwapArray[i].m_pBuilding)
			TheScripts_BuildingSwapArray[i].m_pBuilding->ReplaceWithNewModel(TheScripts_BuildingSwapArray[i].m_nNewModel);
	}
	for (uint32 i = 0; i < MAX_NUM_INVISIBILITY_SETTINGS; i++) {
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&type, &buf, sizeof(type));
		ReadSaveBuf(&handle, &buf, sizeof(handle));
		//- rouz edit (ChatGPT)
		switch (type) {
		case 0:
			TheScripts_InvisibilitySettingArray[i] = nil;
			break;
		case 1:
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			TheScripts_InvisibilitySettingArray[i] = ((CTreadable*)CPool_GetSlot(Pools_GetTreadablePool(), handle - 1));
			//- rouz edit (ChatGPT)
			break;
		case 2:
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			TheScripts_InvisibilitySettingArray[i] = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), handle - 1));
			//- rouz edit (ChatGPT)
			break;
		case 3:
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			TheScripts_InvisibilitySettingArray[i] = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), handle - 1));
			//- rouz edit (ChatGPT)
			break;
		case 4:
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			TheScripts_InvisibilitySettingArray[i] = ((CDummy*)CPool_GetSlot(Pools_GetDummyPool(), handle - 1));
			//- rouz edit (ChatGPT)
			break;
		default:
			script_assert(false);
		}
		if (TheScripts_InvisibilitySettingArray[i])
			TheScripts_InvisibilitySettingArray[i]->bIsVisible = false;
	}
	bool tmpBool;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tmpBool, &buf, sizeof(tmpBool));
	//- rouz edit (ChatGPT)
	script_assert(tmpBool == TheScripts_bUsingAMultiScriptFile);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&TheScripts_bPlayerHasMetDebbieHarry, &buf, sizeof(TheScripts_bPlayerHasMetDebbieHarry));
	SkipSaveBuf(&buf, 2);
	ReadSaveBuf(&tmp, &buf, sizeof(tmp));
	//- rouz edit (ChatGPT)
	script_assert(tmp == TheScripts_MainScriptSize);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tmp, &buf, sizeof(tmp));
	//- rouz edit (ChatGPT)
	script_assert(tmp == TheScripts_LargestMissionScriptSize);
	uint16 tmp16;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tmp16, &buf, sizeof(tmp16));
	//- rouz edit (ChatGPT)
	script_assert(tmp16 == TheScripts_NumberOfMissionScripts);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&tmp16, &buf, sizeof(tmp16));
	//- rouz edit (ChatGPT)
	script_assert(tmp16 == TheScripts_NumberOfExclusiveMissionScripts);
	uint32 runningScripts;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&runningScripts, &buf, sizeof(runningScripts));
	//- rouz edit (ChatGPT)
	for (uint32 i = 0; i < runningScripts; i++)
		TheScripts_StartNewScript(0)->Load(buf);
VALIDATESAVEBUF(size)
}

#undef SCRIPT_DATA_SIZE

void CRunningScript::Save(uint8*& buf)
{
#ifdef COMPATIBLE_SAVES
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ZeroSaveBuf(&buf, 8);
	//- rouz edit (ChatGPT)
	for (int i = 0; i < 8; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &m_abScriptName[i], sizeof(m_abScriptName[i]));
		//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &m_nIp, sizeof(m_nIp));
	//- rouz edit (ChatGPT)
#ifdef CHECK_STRUCT_SIZES
	static_assert(MAX_STACK_DEPTH == 6, "Compatibility loss: MAX_STACK_DEPTH != 6");
#endif
	for (int i = 0; i < MAX_STACK_DEPTH; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &m_anStack[i], sizeof(m_anStack[i]));
		//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &m_nStackPointer, sizeof(m_nStackPointer));
	ZeroSaveBuf(&buf, 2);
	//- rouz edit (ChatGPT)
#ifdef CHECK_STRUCT_SIZES
	static_assert(NUM_LOCAL_VARS + NUM_TIMERS == 18, "Compatibility loss: NUM_LOCAL_VARS + NUM_TIMERS != 18");
#endif
	for (int i = 0; i < NUM_LOCAL_VARS + NUM_TIMERS; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &m_anLocalVariables[i], sizeof(m_anLocalVariables[i]));
		//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &m_bIsActive, sizeof(m_bIsActive));
	WriteSaveBuf(&buf, &m_bCondResult, sizeof(m_bCondResult));
	WriteSaveBuf(&buf, &m_bIsMissionScript, sizeof(m_bIsMissionScript));
	WriteSaveBuf(&buf, &m_bSkipWakeTime, sizeof(m_bSkipWakeTime));
	WriteSaveBuf(&buf, &m_nWakeTime, sizeof(m_nWakeTime));
	WriteSaveBuf(&buf, &m_nAndOrState, sizeof(m_nAndOrState));
	WriteSaveBuf(&buf, &m_bNotFlag, sizeof(m_bNotFlag));
	WriteSaveBuf(&buf, &m_bDeatharrestEnabled, sizeof(m_bDeatharrestEnabled));
	WriteSaveBuf(&buf, &m_bDeatharrestExecuted, sizeof(m_bDeatharrestExecuted));
	WriteSaveBuf(&buf, &m_bMissionFlag, sizeof(m_bMissionFlag));
	ZeroSaveBuf(&buf, 2);
	//- rouz edit (ChatGPT)
#else
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	// Preserve record assignment semantics and compiler padding behavior
	*(CRunningScript*)buf = *this;
	SkipSaveBuf(&buf, sizeof(*this));
	//- rouz edit (ChatGPT)
#endif
}

void CRunningScript::Load(uint8*& buf)
{
#ifdef COMPATIBLE_SAVES
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	SkipSaveBuf(&buf, 8);
	//- rouz edit (ChatGPT)
	for (int i = 0; i < 8; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&m_abScriptName[i], &buf, sizeof(m_abScriptName[i]));
		//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&m_nIp, &buf, sizeof(m_nIp));
	//- rouz edit (ChatGPT)
#ifdef CHECK_STRUCT_SIZES
	static_assert(MAX_STACK_DEPTH == 6, "Compatibility loss: MAX_STACK_DEPTH != 6");
#endif
	for (int i = 0; i < MAX_STACK_DEPTH; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&m_anStack[i], &buf, sizeof(m_anStack[i]));
		//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&m_nStackPointer, &buf, sizeof(m_nStackPointer));
	SkipSaveBuf(&buf, 2);
	//- rouz edit (ChatGPT)
#ifdef CHECK_STRUCT_SIZES
	static_assert(NUM_LOCAL_VARS + NUM_TIMERS == 18, "Compatibility loss: NUM_LOCAL_VARS + NUM_TIMERS != 18");
#endif
	for (int i = 0; i < NUM_LOCAL_VARS + NUM_TIMERS; i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&m_anLocalVariables[i], &buf, sizeof(m_anLocalVariables[i]));
		//- rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&m_bIsActive, &buf, sizeof(m_bIsActive));
	ReadSaveBuf(&m_bCondResult, &buf, sizeof(m_bCondResult));
	ReadSaveBuf(&m_bIsMissionScript, &buf, sizeof(m_bIsMissionScript));
	ReadSaveBuf(&m_bSkipWakeTime, &buf, sizeof(m_bSkipWakeTime));
	ReadSaveBuf(&m_nWakeTime, &buf, sizeof(m_nWakeTime));
	ReadSaveBuf(&m_nAndOrState, &buf, sizeof(m_nAndOrState));
	ReadSaveBuf(&m_bNotFlag, &buf, sizeof(m_bNotFlag));
	ReadSaveBuf(&m_bDeatharrestEnabled, &buf, sizeof(m_bDeatharrestEnabled));
	ReadSaveBuf(&m_bDeatharrestExecuted, &buf, sizeof(m_bDeatharrestExecuted));
	ReadSaveBuf(&m_bMissionFlag, &buf, sizeof(m_bMissionFlag));
	SkipSaveBuf(&buf, 2);
	//- rouz edit (ChatGPT)
#else
	CRunningScript* n = next;
	CRunningScript* p = prev;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	// Preserve record assignment semantics and compiler padding behavior
	*this = *(CRunningScript*)buf;
	SkipSaveBuf(&buf, sizeof(*this));
	//- rouz edit (ChatGPT)
	next = n;
	prev = p;
#endif
}

void TheScripts_ClearSpaceForMissionEntity(const CVector& pos, CEntity* pEntity)
{
	static CColPoint aTempColPoints[MAX_COLLISION_POINTS];
	int16 entities = 0;
	CEntity* aEntities[16];
	World_FindObjectsKindaColliding(pos, pEntity->GetBoundRadius(), false, &entities, 16, aEntities, false, true, true, false, false);
	if (entities <= 0)
		return;
	for (uint16 i = 0; i < entities; i++) {
		if (aEntities[i] != pEntity && aEntities[i]->IsPed() && ((CPed*)aEntities[i])->bInVehicle)
			aEntities[i] = nil;
	}
	for (uint16 i = 0; i < entities; i++) {
		if (aEntities[i] == pEntity || !aEntities[i])
			continue;
		CEntity* pFound = aEntities[i];
		int cols;
		if (pEntity->GetColModel()->numLines <= 0)
			cols = Collision_ProcessColModels(pEntity->GetMatrix(), *pEntity->GetColModel(),
				pFound->GetMatrix(), *pFound->GetColModel(), aTempColPoints, nil, nil);
		else {
			float lines[4];
			lines[0] = lines[1] = lines[2] = lines[3] = 1.0f;
			CColPoint tmp[4];
			cols = Collision_ProcessColModels(pEntity->GetMatrix(), *pEntity->GetColModel(),
				pFound->GetMatrix(), *pFound->GetColModel(), aTempColPoints,tmp, lines);
		}
		if (cols <= 0)
			continue;
		switch (pFound->GetType()) {
		case ENTITY_TYPE_VEHICLE:
		{
			printf("Will try to delete a vehicle where a mission entity should be\n");
			CVehicle* pVehicle = (CVehicle*)pFound;
			if (pVehicle->bIsLocked || !pVehicle->CanBeDeleted())
				break;
			if (pVehicle->pDriver) {
				Population_RemovePed(pVehicle->pDriver);
				pVehicle->pDriver = nil;
			}
			for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
				if (pVehicle->pPassengers[i]) {
					Population_RemovePed(pVehicle->pPassengers[i]);
					pVehicle->pPassengers[i] = 0;
					pVehicle->m_nNumPassengers--;
				}
			}
			CarCtrl_RemoveFromInterestingVehicleList(pVehicle);
			World_Remove(pVehicle);
//+ rouz edit (ChatGPT)
			// Destroy and release the displaced vehicle without invoking C++ delete.
			pVehicle->~CVehicle();
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetVehiclePool(), pVehicle);
			//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
			break;
		}
		case ENTITY_TYPE_PED:
		{
			CPed* pPed = (CPed*)pFound;
			if (pPed->IsPlayer() || !pPed->CanBeDeleted())
				break;
			Population_RemovePed(pPed);
			printf("Deleted a ped where a mission entity should be\n");
			break;
		}
		default: break;
		}
	}
}

void TheScripts_HighlightImportantArea(uint32 id, float x1, float y1, float x2, float y2, float z)
{
	float infX, infY, supX, supY;
	// Delegate the bounds of the area to the C implementation
	ScriptArea_OrderedBounds(x1, y1, x2, y2, &infX, &supX, &infY, &supY);
	CVector center;
	center.x = (infX + supX) / 2;
	center.y = (infY + supY) / 2;
	center.z = (z <= MAP_Z_LOW_LIMIT) ? World_FindGroundZForCoord(center.x, center.y) : z;
	CShadows::RenderIndicatorShadow(id, 2, gpGoalTex, &center, supX - center.x, 0.0f, 0.0f, center.y - supY, 0);
}

void TheScripts_HighlightImportantAngledArea(uint32 id, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float z)
{
	float infX, infY, supX, supY, X, Y;
	// Delegate the bounds of the angled area to the C implementation
	ScriptArea_AngledBounds(x1, y1, x2, y2, x3, y3, x4, y4, &infX, &supX, &infY, &supY);
	CVector center;
	center.x = (infX + supX) / 2;
	center.y = (infY + supY) / 2;
	center.z = (z <= MAP_Z_LOW_LIMIT) ? World_FindGroundZForCoord(center.x, center.y) : z;
	CShadows::RenderIndicatorShadow(id, 2, gpGoalTex, &center, supX - center.x, 0.0f, 0.0f, center.y - supY, 0);
}

#ifdef GTA_SCRIPT_COLLECTIVE
int TheScripts_AddPedsInVehicleToCollective(int index)
{
	int colIndex = TheScripts_NextFreeCollectiveIndex;
	TheScripts_AdvanceCollectiveIndex();
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CVehicle* pVehicle = ((CVehicle*)CPool_GetAt(Pools_GetVehiclePool(), index));
	//- rouz edit (ChatGPT)
	script_assert(pVehicle);
	CPed* pDriver = pVehicle->pDriver;
	if (pDriver && !pDriver->IsPlayer() && pDriver->CharCreatedBy != MISSION_CHAR && pDriver->m_nPedType != PEDTYPE_COP) {
		int index = TheScripts_FindFreeSlotInCollectiveArray();
		if (index > -1) {
			CollectiveArray[index].colIndex = colIndex;
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CollectiveArray[index].pedIndex = CPool_GetIndex(Pools_GetPedPool(), pDriver);
			//- rouz edit (ChatGPT)
		}
	}
	for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
		CPed* pPassenger = pVehicle->pPassengers[i];
		if (pPassenger && !pPassenger->IsPlayer() && pPassenger->CharCreatedBy != MISSION_CHAR && pPassenger->m_nPedType != PEDTYPE_COP) {
			int index = TheScripts_FindFreeSlotInCollectiveArray();
			if (index > -1) {
				CollectiveArray[index].colIndex = colIndex;
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CollectiveArray[index].pedIndex = CPool_GetIndex(Pools_GetPedPool(), pPassenger);
				//- rouz edit (ChatGPT)
			}
		}
	}
	return colIndex;
}

int TheScripts_AddPedsInAreaToCollective(float x, float y, float z, float radius)
{
	int16 numFound;
	CEntity* pEntities[64];
	int colIndex = TheScripts_NextFreeCollectiveIndex;
	TheScripts_AdvanceCollectiveIndex();
	World_FindObjectsInRange(CVector(x, y, z), radius, true, &numFound, 64, pEntities, false, true, true, false, false);
	for (int16 i = 0; i < numFound; i++) {
		if (pEntities[i]->GetType() == ENTITY_TYPE_PED) {
			CPed* pPed = (CPed*)pEntities[i];
			if (pPed && !pPed->IsPlayer() && pPed->CharCreatedBy != MISSION_CHAR && pPed->m_nPedType != PEDTYPE_COP) {
				int index = TheScripts_FindFreeSlotInCollectiveArray();
				if (index > -1) {
					CollectiveArray[index].colIndex = colIndex;
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CollectiveArray[index].pedIndex = CPool_GetIndex(Pools_GetPedPool(), pPed);
					//- rouz edit (ChatGPT)
				}
			}
		}
		else if (pEntities[i]->GetType() == ENTITY_TYPE_VEHICLE) {
			CVehicle* pVehicle = (CVehicle*)pEntities[i];
			CPed* pDriver = pVehicle->pDriver;
			if (pDriver && !pDriver->IsPlayer() && pDriver->CharCreatedBy != MISSION_CHAR && pDriver->m_nPedType != PEDTYPE_COP) {
				int index = TheScripts_FindFreeSlotInCollectiveArray();
				if (index > -1) {
					CollectiveArray[index].colIndex = colIndex;
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CollectiveArray[index].pedIndex = CPool_GetIndex(Pools_GetPedPool(), pDriver);
					//- rouz edit (ChatGPT)
				}
			}
			for (int i = 0; i < pVehicle->m_nNumMaxPassengers; i++) {
				CPed* pPassenger = pVehicle->pPassengers[i];
				if (pPassenger && !pPassenger->IsPlayer() && pPassenger->CharCreatedBy != MISSION_CHAR && pPassenger->m_nPedType != PEDTYPE_COP) {
					int index = TheScripts_FindFreeSlotInCollectiveArray();
					if (index > -1) {
						CollectiveArray[index].colIndex = colIndex;
						//+ rouz edit (ChatGPT)
						// Access raw storage through the C store or pool API
						CollectiveArray[index].pedIndex = CPool_GetIndex(Pools_GetPedPool(), pPassenger);
						//- rouz edit (ChatGPT)
					}
				}
			}
		}
	}
	return colIndex;
}

int TheScripts_FindFreeSlotInCollectiveArray()
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == -1)
			return i;
	}
	return -1;
}

void TheScripts_SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, int16 p1, int16 p2)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CollectiveArray[i].pedIndex));
			//- rouz edit (ChatGPT)
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1, p2);
			}
		}
	}
}

void TheScripts_SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, CVector p1, float p2)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CollectiveArray[i].pedIndex));
			//- rouz edit (ChatGPT)
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1, p2);
			}
		}
	}
}

void TheScripts_SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, CVector p1)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CollectiveArray[i].pedIndex));
			//- rouz edit (ChatGPT)
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1);
			}
		}
	}
}

void TheScripts_SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective, void* p1)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CollectiveArray[i].pedIndex));
			//- rouz edit (ChatGPT)
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective, p1);
			}
		}
	}
}

void TheScripts_SetObjectiveForAllPedsInCollective(int colIndex, eObjective objective)
{
	for (int i = 0; i < MAX_NUM_COLLECTIVES; i++) {
		if (CollectiveArray[i].colIndex == colIndex) {
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			CPed* pPed = ((CPed*)CPool_GetAt(Pools_GetPedPool(), CollectiveArray[i].pedIndex));
			//- rouz edit (ChatGPT)
			if (pPed == nil) {
				CollectiveArray[i].colIndex = -1;
				CollectiveArray[i].pedIndex = 0;
			}
			else {
				pPed->bScriptObjectiveCompleted = false;
				pPed->SetObjective(objective);
			}
		}
	}
}
#endif //GTA_SCRIPT_COLLECTIVE

bool TheScripts_IsPedStopped(CPed* pPed)
{
	if (pPed->InVehicle())
		return TheScripts_IsVehicleStopped(pPed->m_pMyVehicle);
	return (pPed->m_nMoveState == PEDMOVE_NONE || pPed->m_nMoveState == PEDMOVE_STILL) &&
		!pPed->bIsInTheAir && !pPed->bIsLanding && pPed->bIsStanding && pPed->m_vecAnimMoveDelta.x == 0.0f && pPed->m_vecAnimMoveDelta.y == 0.0f;
}

bool TheScripts_IsPlayerStopped(CPlayerInfo* pPlayer)
{
	CPed* pPed = pPlayer->m_pPed;
	if (pPed->InVehicle())
		return TheScripts_IsVehicleStopped(pPed->m_pMyVehicle);
	if (RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_RUNSTOP1) ||
		RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_RUNSTOP2) ||
		RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_JUMP_LAUNCH) ||
		RpAnimBlendClumpGetAssociation(pPed->GetClump(), ANIM_STD_JUMP_GLIDE))
		return false;
	return (pPed->m_nMoveState == PEDMOVE_NONE || pPed->m_nMoveState == PEDMOVE_STILL) &&
		!pPed->bIsInTheAir && !pPed->bIsLanding && pPed->bIsStanding && pPed->m_vecAnimMoveDelta.x == 0.0f && pPed->m_vecAnimMoveDelta.y == 0.0f;
}

bool TheScripts_IsVehicleStopped(CVehicle* pVehicle)
{
	return 0.01f * (*Timer_GetTimeStep()) >= pVehicle->m_fDistanceTravelled;
}

void TheScripts_RemoveThisPed(CPed* pPed)
{
	if (pPed) {
		bool bWasMissionPed = pPed->CharCreatedBy == MISSION_CHAR;
		if (pPed->InVehicle() && pPed->m_pMyVehicle) {
			if (pPed->m_pMyVehicle->pDriver == pPed) {
				pPed->m_pMyVehicle->RemoveDriver();
				pPed->m_pMyVehicle->SetStatus(STATUS_ABANDONED);
				if (pPed->m_pMyVehicle->m_nDoorLock == CARLOCK_LOCKED_INITIALLY)
					pPed->m_pMyVehicle->m_nDoorLock = CARLOCK_UNLOCKED;
				if (pPed->m_nPedType == PEDTYPE_COP && pPed->m_pMyVehicle->IsLawEnforcementVehicle())
					pPed->m_pMyVehicle->ChangeLawEnforcerState(0);
			}
			else {
				pPed->m_pMyVehicle->RemovePassenger(pPed);
			}
		}
		World_RemoveReferencesToDeletedObject(pPed);
//+ rouz edit (ChatGPT)
		// Destroy and release the scripted ped without invoking C++ delete.
		pPed->~CPed();
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Delete(Pools_GetPedPool(), pPed);
		//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
		if (bWasMissionPed)
			--Population_nTotalMissionPeds; // rouz edit (ChatGPT)
	}
}

void TheScripts_CleanUpThisPed(CPed* pPed)
{
	if (!pPed)
		return;
	if (pPed->CharCreatedBy != MISSION_CHAR)
		return;
	pPed->CharCreatedBy = RANDOM_CHAR;
	if (pPed->m_nPedType == PEDTYPE_PROSTITUTE)
		pPed->m_objectiveTimer = (*Timer_GetTimeInMilliseconds()) + 30000;
	if (pPed->InVehicle()) {
		if (pPed->m_pMyVehicle->pDriver == pPed) {
			if (pPed->m_pMyVehicle->m_vehType == VEHICLE_TYPE_CAR) {
				CarCtrl_JoinCarWithRoadSystem(pPed->m_pMyVehicle);
				pPed->m_pMyVehicle->AutoPilot.m_nCarMission = MISSION_CRUISE;
			}
		}
		else {
			if (pPed->m_pMyVehicle->m_vehType == VEHICLE_TYPE_CAR) {
				pPed->SetObjective(OBJECTIVE_LEAVE_CAR, pPed->m_pMyVehicle);
				pPed->bWanderPathAfterExitingCar = true;
			}
		}
	}
	bool flees = false;
	PedState state;
	eMoveState ms;
	if (pPed->m_nPedState == PED_FLEE_ENTITY || pPed->m_nPedState == PED_FLEE_POS) {
		ms = pPed->m_nMoveState;
		state = pPed->m_nPedState;
		flees = true;
	}
	pPed->ClearObjective();
	pPed->bRespondsToThreats = true;
	pPed->bScriptObjectiveCompleted = false;
	pPed->bKindaStayInSamePlace = false;
	pPed->ClearLeader();
	if (pPed->IsPedInControl())
		pPed->SetWanderPath(General_GetRandomNumber() & 7);
	if (flees) {
		pPed->SetPedState(state);
		pPed->SetMoveState(ms);
	}
	--Population_nTotalMissionPeds; // rouz edit (ChatGPT)
}

void TheScripts_CleanUpThisVehicle(CVehicle* pVehicle)
{
	if (!pVehicle)
		return;
	if (pVehicle->VehicleCreatedBy != MISSION_VEHICLE)
		return;
	pVehicle->bIsLocked = false;
	CarCtrl_RemoveFromInterestingVehicleList(pVehicle);
	pVehicle->VehicleCreatedBy = RANDOM_VEHICLE;
	++CarCtrl_NumRandomCars;
	--CarCtrl_NumMissionCars;
}

void TheScripts_CleanUpThisObject(CObject* pObject)
{
	if (!pObject)
		return;
	if (pObject->ObjectCreatedBy != MISSION_OBJECT)
		return;
	pObject->ObjectCreatedBy = TEMP_OBJECT;
	pObject->m_nEndOfLifeTime = (*Timer_GetTimeInMilliseconds()) + 20000000;
	pObject->m_nRefModelIndex = -1;
	pObject->bUseVehicleColours = false;
	++CObject::nNoTempObjects;
}

void TheScripts_ReadObjectNamesFromScript()
{
	int32 varSpace = TheScripts_GetSizeOfVariableSpace();
	uint32 ip = varSpace + 8;
	TheScripts_NumberOfUsedObjects = TheScripts_Read2BytesFromScript(&ip);
	ip += 2;
	for (uint16 i = 0; i < TheScripts_NumberOfUsedObjects; i++) {
		for (int j = 0; j < USED_OBJECT_NAME_LENGTH; j++)
			TheScripts_UsedObjectArray[i].name[j] = TheScripts_ScriptSpace[ip++];
		TheScripts_UsedObjectArray[i].index = 0;
	}
}

void TheScripts_UpdateObjectIndices()
{
	char name[USED_OBJECT_NAME_LENGTH];
	char error[112];
	for (int i = 1; i < TheScripts_NumberOfUsedObjects; i++) {
		bool found = false;
		for (int j = 0; j < MODELINFOSIZE && !found; j++) {
			CBaseModelInfo* pModel = ((CBaseModelInfo*)ModelRegistry_Get(j)); // rouz edit (ChatGPT)
			if (!pModel)
				continue;
			strcpy(name, pModel->GetModelName());
#ifdef FIX_BUGS
			for (int k = 0; k < USED_OBJECT_NAME_LENGTH && name[k]; k++)
#else
			for (int k = 0; k < USED_OBJECT_NAME_LENGTH; k++)
#endif
				name[k] = toupper(name[k]);
			if (strcmp(name, TheScripts_UsedObjectArray[i].name) == 0) {
				found = true;
				TheScripts_UsedObjectArray[i].index = j;
			}
		}
		if (!found) {
			sprintf(error, "CTheScripts::UpdateObjectIndices - Couldn't find %s", TheScripts_UsedObjectArray[i].name);
			debug("%s\n", error);
		}
	}
}

void TheScripts_ReadMultiScriptFileOffsetsFromScript()
{
	int32 varSpace = TheScripts_GetSizeOfVariableSpace();
	uint32 ip = varSpace + 3;
	int32 objectSize = TheScripts_Read4BytesFromScript(&ip);
	ip = objectSize + 8;
	TheScripts_MainScriptSize = TheScripts_Read4BytesFromScript(&ip);
	TheScripts_LargestMissionScriptSize = TheScripts_Read4BytesFromScript(&ip);
	TheScripts_NumberOfMissionScripts = TheScripts_Read2BytesFromScript(&ip);
	TheScripts_NumberOfExclusiveMissionScripts = TheScripts_Read2BytesFromScript(&ip);
	for (int i = 0; i < TheScripts_NumberOfMissionScripts; i++) {
		TheScripts_MultiScriptArray[i] = TheScripts_Read4BytesFromScript(&ip);
	}
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
