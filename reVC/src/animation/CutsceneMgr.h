//+ rouz edit (ChatGPT)
#pragma once
#include "CutsceneObject.h"
#include "CutsceneState.h" // rouz edit (ChatGPT)

#define CUTSCENENAMESIZE 8

struct CDirectory;
struct CAnimBlendAssocGroup;
class CCutsceneHead;

//+ rouz edit (Claude)
// Formerly the static class CCutsceneMgr, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern CCutsceneObject * CutsceneMgr_ms_pCutsceneObjects[NUMCUTSCENEOBJECTS];
extern int32 CutsceneMgr_ms_numCutsceneObjs;
extern bool CutsceneMgr_ms_loaded;
extern bool CutsceneMgr_ms_animLoaded;
extern bool CutsceneMgr_ms_useLodMultiplier;
extern char CutsceneMgr_ms_cutsceneName[CUTSCENENAMESIZE];
extern CVector CutsceneMgr_ms_cutsceneOffset;
extern float CutsceneMgr_ms_cutsceneTimer;
extern bool CutsceneMgr_ms_wasCutsceneSkipped;
extern bool CutsceneMgr_ms_useCutsceneShadows;
extern CDirectory * CutsceneMgr_ms_pCutsceneDir;
extern uint32 CutsceneMgr_ms_cutsceneLoadStatus;
bool CutsceneMgr_HasCutsceneFinished(void);
void CutsceneMgr_Initialise(void);
void CutsceneMgr_Shutdown(void);
void CutsceneMgr_LoadCutsceneData(const char *szCutsceneName);
void CutsceneMgr_FinishCutscene(void);
void CutsceneMgr_SetupCutsceneToStart(void);
void CutsceneMgr_SetCutsceneAnim(const char *animName, CObject *pObject);
void CutsceneMgr_SetCutsceneAnimToLoop(const char *animName);
CCutsceneHead *CutsceneMgr_AddCutsceneHead(CObject *pObject, int modelId);
CCutsceneObject *CutsceneMgr_CreateCutsceneObject(int modelId);
void CutsceneMgr_DeleteCutsceneData(void);
void CutsceneMgr_LoadAnimationUncompressed(char const*);
void CutsceneMgr_Update(void);
void CutsceneMgr_AttachObjectToParent(CObject *pObject, CEntity *pAttachTo);
void CutsceneMgr_AttachObjectToFrame(CObject *pObject, CEntity *pAttachTo, const char *frame);
void CutsceneMgr_AttachObjectToBone(CObject *pObject, CObject *pAttachTo, int frame);
void CutsceneMgr_RemoveEverythingFromTheWorldForTheBiggestFuckoffCutsceneEver();
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CAnimBlendAssocGroup CutsceneMgr_ms_cutsceneAssociations;
static inline bool CutsceneMgr_HasLoaded(void);
static inline bool CutsceneMgr_WasCutsceneSkipped(void);
static inline bool CutsceneMgr_UseLodMultiplier(void);
static inline CCutsceneObject* CutsceneMgr_GetCutsceneObject(int id);
static inline int CutsceneMgr_GetCutsceneTimeInMilleseconds(void);
static inline char *CutsceneMgr_GetCutsceneName(void);
static inline void CutsceneMgr_SetCutsceneOffset(const CVector& vec);
static inline void CutsceneMgr_DisableCutsceneShadows();
static inline bool CutsceneMgr_HasLoaded(void) { return CutsceneMgr_ms_loaded; }
static inline bool CutsceneMgr_WasCutsceneSkipped(void) { return CutsceneMgr_ms_wasCutsceneSkipped; }
static inline bool CutsceneMgr_UseLodMultiplier(void) { return CutsceneMgr_ms_useLodMultiplier; }
static inline CCutsceneObject* CutsceneMgr_GetCutsceneObject(int id) { return CutsceneMgr_ms_pCutsceneObjects[id]; }
static inline int CutsceneMgr_GetCutsceneTimeInMilleseconds(void) { return 1000.0f * CutsceneMgr_ms_cutsceneTimer; }
static inline char *CutsceneMgr_GetCutsceneName(void) { return CutsceneMgr_ms_cutsceneName; }
static inline void CutsceneMgr_SetCutsceneOffset(const CVector& vec) { CutsceneMgr_ms_cutsceneOffset = vec; }
static inline void CutsceneMgr_DisableCutsceneShadows() { CutsceneMgr_ms_useCutsceneShadows = false; }
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
