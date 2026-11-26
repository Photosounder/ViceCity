//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma once

#if !defined FINAL && defined __cplusplus // rouz edit (Claude)
#include "RwHelper.h"
#define PUSH_RENDERGROUP(str) PushRendergroup(str)
#define POP_RENDERGROUP() PopRendergroup()
#else
#define PUSH_RENDERGROUP(str)
#define POP_RENDERGROUP()
#endif

//+ rouz edit (Claude)
// Everything here has C linkage so C files can use it, and the RenderWare pointers in the scene are opaque to C
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#define GLOBALSCENE_WORLD_TYPE RpWorld
#define GLOBALSCENE_CAMERA_TYPE RwCamera
#else
#define GLOBALSCENE_WORLD_TYPE void
#define GLOBALSCENE_CAMERA_TYPE void
#endif

typedef struct GlobalScene
{
	GLOBALSCENE_WORLD_TYPE *world;
	GLOBALSCENE_CAMERA_TYPE *camera;
} GlobalScene;
//- rouz edit (Claude)
extern GlobalScene Scene;

#include "WorkBuffer.h" // rouz edit (ChatGPT)
#include "SharedText.h" // rouz edit (ChatGPT)
extern bool gbPrintShite;
extern bool gbModelViewer;
#ifdef TIMEBARS
extern bool gbShowTimebars;
#else
#define gbShowTimebars false
#endif

#ifndef FINAL
extern bool gbPrintMemoryUsage;
#endif

typedef struct Sprite2d Sprite2d; // rouz edit (Claude)

bool DoRWStuffStartOfFrame(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
bool DoRWStuffStartOfFrame_Horizon(int16 TopRed, int16 TopGreen, int16 TopBlue, int16 BottomRed, int16 BottomGreen, int16 BottomBlue, int16 Alpha);
void DoRWStuffEndOfFrame(void);
void PreAllocateRwObjects(void);
void InitialiseGame(void);
void LoadingScreen(const char *str1, const char *str2, const char *splashscreen);
void LoadingIslandScreen(const char *levelName);
Sprite2d *LoadSplash(const char *name);
void DestroySplashScreen(void);
Const char *GetLevelSplashScreen(int level);
Const char *GetRandomSplashScreen(void);
void LittleTest(void);
void ValidateVersion();
void ResetLoadingScreenBar(void);
#ifndef MASTER
void TheModelViewer(void);
#endif

#ifdef LOAD_INI_SETTINGS
bool LoadINISettings();
void SaveINISettings();
void LoadINIControllerSettings();
void SaveINIControllerSettings();
#endif

#ifdef NEW_RENDERER
extern bool gbNewRenderer;
bool FredIsInFirstPersonCam(void);
#endif

#ifdef DRAW_GAME_VERSION_TEXT
extern bool gbDrawVersionText;
#endif

#ifdef NO_MOVIES
extern bool gbNoMovies;
#endif

extern void rouz_veh_apply_controls_from_window(void *veh_ptr);
extern void rouz_init();
extern void rouz_update();
extern float get_airflow_angle_of_attack(void *p_ptr, void *normal_vec_ptr);
extern float get_horizon_angle_of_attack(void *p_ptr);
extern int rouz_acceleration_cheats(void *ptr);

//+ rouz edit (Claude)
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
