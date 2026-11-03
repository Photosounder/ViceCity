//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
class CEntity;
extern "C" {
#else
typedef struct CEntity CEntity;
#endif
enum AnimViewerButton { ANIMVIEWER_SQUARE,ANIMVIEWER_CROSS,ANIMVIEWER_CIRCLE,ANIMVIEWER_UP,ANIMVIEWER_DOWN,ANIMVIEWER_START,ANIMVIEWER_SHOULDER,ANIMVIEWER_TRIANGLE,ANIMVIEWER_LEFT,ANIMVIEWER_RIGHT };
typedef struct AnimViewerState { int modelId,animId;bool reloadIFP; } AnimViewerState;
extern AnimViewerState AnimViewer_state;
extern int AnimViewer_animTxdSlot;
extern CEntity *AnimViewer_target;
void AnimViewer_Initialise(void);
void AnimViewer_InitTextures(void);
void AnimViewer_ConvertModelAnimations(void);
void AnimViewer_LoadSpecialCharacters(void);
void AnimViewer_LoadVehicleAnimations(void);
void AnimViewer_PlayAnimation(void *clump,int animGroup,int anim);
void AnimViewer_PreviousAnimation(void);
void AnimViewer_NextAnimation(void);
void AnimViewer_ClampAnimation(int count);
void AnimViewer_HandlePedInput(void *pad,int animGroup,int *nextModelId);
void AnimViewer_HandleVehicleInput(void *pad,int *nextModelId);
void AnimViewer_HandleModelNavigation(void *pad,int *nextModelId);
bool AnimViewer_ApplyModelChange(int nextModelId);
void AnimViewer_Render(void);
void AnimViewer_RemoveTarget(void);
void AnimViewer_ReloadAnimations(void);
void AnimViewer_Shutdown(void);
void AnimViewer_Update(void);
void AnimViewer_UpdateWorld(void);
void AnimViewer_PrepareTarget(void);
void AnimViewer_CreateTarget(void *model);
void AnimViewer_PlaceTarget(void);
void AnimViewer_CreateVehicleTarget(void *model);
void AnimViewer_AllocateVehicle(int type);
void AnimViewer_CreatePedTarget(void);
void AnimViewer_CreatePlayer(void);
void AnimViewer_CreateObjectTarget(void *model);
void AnimViewer_ReclaimObject(void *pool);
void AnimViewer_AllocateObject(void *pool);
int AnimViewer_NextModelId(int modelId,int wantedChange);
int AnimViewer_LastPedModelId(int modelId);
int AnimViewer_FirstCarModelId(int modelId);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
