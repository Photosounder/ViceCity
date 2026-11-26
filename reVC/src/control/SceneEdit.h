#pragma once
#ifdef GTA_SCENE_EDIT
class CPed;
class CVehicle;

struct CMovieCommand
{
	int32 m_nCommandId;
	CVector m_vecPosition;
	CVector m_vecCamera;
	int16 m_nActorId;
	int16 m_nActor2Id;
	int16 m_nVehicleId;
	int16 m_nModelIndex;
};

//+ rouz edit (Claude)
// Formerly the static class CSceneEdit, now C globals and functions shared with C
enum {
		MOVIE_DO_NOTHING = 0,
		MOVIE_NEW_ACTOR,
		MOVIE_MOVE_ACTOR,
		MOVIE_SELECT_ACTOR,
		MOVIE_DELETE_ACTOR,
		MOVIE_NEW_VEHICLE,
		MOVIE_MOVE_VEHICLE,
		MOVIE_SELECT_VEHICLE,
		MOVIE_DELETE_VEHICLE,
		MOVIE_GIVE_WEAPON,
		MOVIE_GOTO,
		MOVIE_GOTO_WAIT,
		MOVIE_GET_IN_CAR,
		MOVIE_GET_OUT_CAR,
		MOVIE_KILL,
		MOVIE_FLEE,
		MOVIE_WAIT,
		MOVIE_POSITION_CAMERA,
		MOVIE_SET_CAMERA_TARGET,
		MOVIE_SELECT_CAMERA_MODE,
		MOVIE_SAVE_MOVIE,
		MOVIE_LOAD_MOVIE,
		MOVIE_PLAY_MOVIE,
		MOVIE_END,
		MOVIE_TOTAL_COMMANDS
	};
enum {
		NUM_ACTORS_IN_MOVIE = 5,
		NUM_VEHICLES_IN_MOVIE = 5,
		NUM_COMMANDS_IN_MOVIE = 20
	};
#ifdef __cplusplus
extern "C" {
#endif
extern int32 SceneEdit_m_bCameraFollowActor;
extern CVector SceneEdit_m_vecCurrentPosition;
extern CVector SceneEdit_m_vecCamHeading;
extern CVector SceneEdit_m_vecGotoPosition;
extern int32 SceneEdit_m_nVehicle;
extern int32 SceneEdit_m_nVehicle2;
extern int32 SceneEdit_m_nActor;
extern int32 SceneEdit_m_nActor2;
extern int32 SceneEdit_m_nVehiclemodelId;
extern int32 SceneEdit_m_nPedmodelId;
extern int16 SceneEdit_m_nCurrentMovieCommand;
extern int16 SceneEdit_m_nCurrentCommand;
extern int16 SceneEdit_m_nCurrentVehicle;
extern int16 SceneEdit_m_nCurrentActor;
extern bool SceneEdit_m_bEditOn;
extern bool SceneEdit_m_bRecording;
extern bool SceneEdit_m_bCommandActive;
extern bool SceneEdit_m_bActorSelected;
extern bool SceneEdit_m_bActor2Selected;
extern bool SceneEdit_m_bVehicleSelected;
extern int16 SceneEdit_m_nNumActors;
extern int16 SceneEdit_m_nNumVehicles;
extern int16 SceneEdit_m_nNumMovieCommands;
extern int16 SceneEdit_m_nWeaponType;
extern CPed* SceneEdit_pActors[NUM_ACTORS_IN_MOVIE];
extern CVehicle* SceneEdit_pVehicles[NUM_VEHICLES_IN_MOVIE];
extern bool SceneEdit_m_bDrawGotoArrow;
void SceneEdit_LoadMovie(void);
void SceneEdit_SaveMovie(void);
void SceneEdit_Initialise(void);
void SceneEdit_InitPlayback(void);
void SceneEdit_ReInitialise(void);
void SceneEdit_Update(void);
void SceneEdit_Draw(void);
void SceneEdit_ProcessCommand(void);
void SceneEdit_PlayBack(void);
void SceneEdit_ClearForNewCommand(void);
void SceneEdit_SelectActor(void);
void SceneEdit_SelectActor2(void);
void SceneEdit_SelectVehicle(void);
bool SceneEdit_SelectWeapon(void);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CMovieCommand SceneEdit_Movie[NUM_COMMANDS_IN_MOVIE];
#endif
//- rouz edit (Claude)
#endif
