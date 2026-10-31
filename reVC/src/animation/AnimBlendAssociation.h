//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendList.h"
#include "AnimBlendNode.h"
#include "AnimBlendHierarchy.h"
#include "AnimBlendAssociation_flags.h"
enum {ANIM_ASSOC_CB_NONE=0,ANIM_ASSOC_CB_FINISH=1,ANIM_ASSOC_CB_DELETE=2};
typedef struct CAnimBlendAssociation {
    CAnimBlendLink link;
    int16_t numNodes,groupId;
    CAnimBlendNode *nodes;
    CAnimBlendHierarchy *hierarchy;
    float blendAmount,blendDelta,currentTime,speed,timeStep;
    int16_t animId,flags;
    int32_t callbackType;
    void (*callback)(struct CAnimBlendAssociation*,void*);
    void *callbackArg;
} CAnimBlendAssociation;
typedef void (*AnimAssociationCallback)(CAnimBlendAssociation*,void*);
#ifdef __cplusplus
extern "C" {
#endif
void AnimAssociation_Init(CAnimBlendAssociation *self);
void AnimAssociation_CopyConstruct(CAnimBlendAssociation *self,CAnimBlendAssociation *other);
void AnimAssociation_Destroy(CAnimBlendAssociation *self);
void AnimAssociation_AllocateAnimBlendNodeArray(CAnimBlendAssociation *self,int32_t n);
void AnimAssociation_FreeAnimBlendNodeArray(CAnimBlendAssociation *self);
void AnimAssociation_BindClump(CAnimBlendAssociation *self,void *clump,CAnimBlendHierarchy *hier);
void AnimAssociation_InitFrom(CAnimBlendAssociation *self,CAnimBlendAssociation *other);
void AnimAssociation_SetBlend(CAnimBlendAssociation *self,float amount,float delta);
void AnimAssociation_SetFinishCallback(CAnimBlendAssociation *self,AnimAssociationCallback cb,void *arg);
void AnimAssociation_SetDeleteCallback(CAnimBlendAssociation *self,AnimAssociationCallback cb,void *arg);
void AnimAssociation_SetCurrentTime(CAnimBlendAssociation *self,float time);
void AnimAssociation_SyncAnimation(CAnimBlendAssociation *self,CAnimBlendAssociation *other);
void AnimAssociation_Start(CAnimBlendAssociation *self,float time);
void AnimAssociation_UpdateTimeStep(CAnimBlendAssociation *self,float timeDelta,float relSpeed);
bool AnimAssociation_UpdateTime(CAnimBlendAssociation *self,float timeDelta,float relSpeed);
bool AnimAssociation_UpdateBlend(CAnimBlendAssociation *self,float timeDelta);
bool AnimAssociation_IsRunning(CAnimBlendAssociation *self);
bool AnimAssociation_IsRepeating(CAnimBlendAssociation *self);
bool AnimAssociation_IsPartial(CAnimBlendAssociation *self);
bool AnimAssociation_IsMovement(CAnimBlendAssociation *self);
bool AnimAssociation_HasTranslation(CAnimBlendAssociation *self);
bool AnimAssociation_HasXTranslation(CAnimBlendAssociation *self);
float AnimAssociation_GetBlendAmount(CAnimBlendAssociation *self,float weight);
CAnimBlendNode * AnimAssociation_GetNode(CAnimBlendAssociation *self,int32_t i);
void AnimAssociation_SetRun(CAnimBlendAssociation *self);
float AnimAssociation_GetTimeLeft(CAnimBlendAssociation *self);
float AnimAssociation_GetProgress(CAnimBlendAssociation *self);
CAnimBlendAssociation * AnimAssociation_FromLink(CAnimBlendLink *link);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
