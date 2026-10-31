//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CObject;
struct CPlaneNode;
#else
typedef struct CObject CObject;
typedef struct CPlaneNode CPlaneNode;
#endif
enum { SCRIPT_PATH_DISABLED=0,SCRIPT_PATH_INITIALIZED,SCRIPT_PATH_ACTIVE };
typedef struct ScriptPathVector { float x,y,z; } ScriptPathVector;
typedef struct ScriptPath {
    int32_t m_numNodes;
    CPlaneNode *m_pNode;
    float m_fTotalLength,m_fSpeed,m_fPosition,m_fObjectLength;
    int32_t m_state;
    CObject *m_pObjects[6];
} ScriptPath;
#ifdef __cplusplus
extern "C" {
#endif
extern ScriptPath ScriptPaths_aArray[3];
extern CObject *g_pScriptPathObjects[18];
void ScriptPath_FindCoorsFromDistanceOnPath(ScriptPath *path,float t,float *x,float *y,float *z);
void ScriptPath_Update(ScriptPath *path);
void ScriptPath_Clear(ScriptPath *path);
void ScriptPath_InitialiseOne(ScriptPath *path,int32_t numNodes,float length);
void ScriptPath_SetObjectToControl(ScriptPath *path,CObject *object);
void ScriptPaths_Init(void);
void ScriptPaths_Shutdown(void);
void ScriptPaths_Update(void);
bool ScriptPaths_IsOneActive(void);
void ScriptPaths_Load(uint8_t *buffer,uint32_t size);
void ScriptPaths_Save(uint8_t *buffer,uint32_t *size);
void ScriptPaths_Load_ForReplay(void);
void ScriptPaths_Save_ForReplay(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
