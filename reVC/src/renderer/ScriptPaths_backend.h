//+ rouz edit (ChatGPT)
#pragma once
#include "ScriptPaths.h"
#include "Pool.h"
typedef struct ScriptPathGeometry { ScriptPathVector pos1,pos2,right,up,forward; } ScriptPathGeometry;
typedef void (*ScriptPathObjectCallback)(ScriptPath *,int32_t,ScriptPathGeometry,void *);
enum { SCRIPT_PATH_NODE_SIZE=20 };
#ifdef __cplusplus
extern "C" {
#endif
float ScriptPaths_BackendNodeT(ScriptPath *path,int32_t index);
float ScriptPaths_BackendNodeAxis(ScriptPath *path,int32_t index,int32_t axis);
void ScriptPaths_BackendFreeNodes(CPlaneNode *nodes);
CPlaneNode * ScriptPaths_BackendLoadNodes(const char *file,int32_t *count,float *length);
CPlaneNode * ScriptPaths_BackendAllocateNodes(int32_t count);
void ScriptPaths_BackendReadNode(CPlaneNode *nodes,int32_t index,const void *buffer);
void ScriptPaths_BackendWriteNode(void *buffer,CPlaneNode *nodes,int32_t index);
CPool * ScriptPaths_BackendObjectPool(void);
void ScriptPaths_BackendClearPhysics(CObject *object);
void ScriptPaths_BackendRegister(CObject *object,CObject **slot);
ScriptPathVector ScriptPaths_BackendPosition(CObject *object);
ScriptPathVector ScriptPaths_BackendForward(CObject *object);
void ScriptPaths_BackendSetPosition(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendUpdateRW(CObject *object);
void ScriptPaths_BackendUpdateFrame(CObject *object);
bool ScriptPaths_BackendBigBuilding(CObject *object);
void ScriptPaths_BackendRemoveAndAdd(CObject *object);
ScriptPathVector ScriptPaths_BackendPreviousPosition(void *matrix);
ScriptPathVector ScriptPaths_BackendPreviousForward(void *matrix);
void ScriptPaths_BackendSetRight(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendSetUp(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendSetForward(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendSetMoveSpeed(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendSetTurnSpeed(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendSetMoveFriction(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendSetTurnFriction(CObject *object,ScriptPathVector value);
void ScriptPaths_BackendWithPrevious(ScriptPath *path,int32_t index,ScriptPathGeometry geometry,ScriptPathObjectCallback callback);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
