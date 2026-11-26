#pragma once

class CAutomobile;

//+ rouz edit (Claude)
// Formerly the static class CGameLogic, now C globals and functions shared with C
enum {
		SHORTCUT_NONE = 0,
		SHORTCUT_INIT,
		SHORTCUT_IDLE,
		SHORTCUT_GETTING_IN,
		SHORTCUT_TRANSITION,
		SHORTCUT_ARRIVING,
		SHORTCUT_GETTING_OUT
	};
#ifdef __cplusplus
extern "C" {
#endif
void GameLogic_InitAtStartOfGame();
void GameLogic_PassTime(uint32 time);
void GameLogic_Update();
void GameLogic_RestorePlayerStuffDuringResurrection(class CPlayerPed *pPlayerPed, CVector pos, float angle);
void GameLogic_ClearShortCut();
void GameLogic_SetUpShortCut(CVector, float, CVector, float);
void GameLogic_AbandonShortCutIfTaxiHasBeenMessedWith();
void GameLogic_AbandonShortCutIfPlayerMilesAway();
void GameLogic_UpdateShortCut();
void GameLogic_AddShortCutPointAfterDeath(CVector, float);
void GameLogic_AddShortCutDropOffPointForMission(CVector, float);
void GameLogic_RemoveShortCutDropOffPointForMission();
void GameLogic_AfterDeathArrestSetUpShortCutTaxi();
void GameLogic_Save(uint8*, uint32*);
void GameLogic_Load(uint8*, uint32);
extern uint8 GameLogic_ActivePlayers;
extern uint8 GameLogic_ShortCutState;
extern CAutomobile* GameLogic_pShortCutTaxi;
extern uint32 GameLogic_NumAfterDeathStartPoints;
extern CVector GameLogic_ShortCutStart;
extern float GameLogic_ShortCutStartOrientation;
extern CVector GameLogic_ShortCutDestination;
extern float GameLogic_ShortCutDestinationOrientation;
extern uint32 GameLogic_ShortCutTimer;
extern CVector GameLogic_AfterDeathStartPoints[NUM_SHORTCUT_START_POINTS];
extern float GameLogic_AfterDeathStartPointOrientation[NUM_SHORTCUT_START_POINTS];
extern CVector GameLogic_ShortCutDropOffForMission;
extern float GameLogic_ShortCutDropOffOrientationForMission;
extern bool GameLogic_MissionDropOffReadyToBeUsed;
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
void GameLogic_SortOutStreamingAndMemory(const CVector &pos);
#endif
//- rouz edit (Claude)