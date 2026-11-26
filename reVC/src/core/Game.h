//+ rouz edit (ChatGPT)
#pragma once

#include "GameLevels.h"

#include "GameAreas.h"

//+ rouz edit (Claude)
// Game state and the top-level game init, shutdown and per-frame processing, formerly the static class CGame, shared with C
#include <stdbool.h>
#include "VectorFwd.h"

#ifdef __cplusplus
extern "C" {
#endif

extern eLevelName Game_currLevel;
extern int32 Game_currArea;
extern bool Game_bDemoMode;
extern bool Game_nastyGame;
extern bool Game_frenchGame;
extern bool Game_germanGame;
#ifdef MORE_LANGUAGES
extern bool Game_russianGame;
extern bool Game_japaneseGame;
#endif
extern bool Game_noProstitutes;
extern bool Game_playingIntro;
extern char Game_aDatFile[32];

#ifndef MASTER
extern CVector Game_PlayerCoords;
extern bool8 Game_VarUpdatePlayerCoords;
#endif

bool Game_InitialiseOnceBeforeRW(void);
bool Game_InitialiseRenderWare(void);
void Game_ShutdownRenderWare(void);
bool Game_InitialiseOnceAfterRW(void);
void Game_FinalShutdown(void);
bool Game_Initialise(const char *datFile);
bool Game_ShutDown(void);
void Game_ReInitGameObjectVariables(void);
void Game_ReloadIPLs(void);
void Game_ShutDownForRestart(void);
void Game_InitialiseWhenRestarting(void);
void Game_Process(void);

void Game_InitAfterFocusLoss(void);

static inline bool Game_IsInInterior(void) { return Game_currArea != AREA_MAIN_MAP; }
bool Game_CanSeeWaterFromCurrArea(void);
bool Game_CanSeeOutSideFromCurrArea(void);

// NB: these do something on PS2
void Game_TidyUpMemory(bool moveTextures, bool flushDraw);
void Game_DrasticTidyUpMemory(bool flushDraw);
void Game_ProcessTidyUpMemory(void);

static inline bool IsAreaVisible(int area) { return area == Game_currArea || area == AREA_EVERYWHERE; }

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)

//- rouz edit (ChatGPT)
