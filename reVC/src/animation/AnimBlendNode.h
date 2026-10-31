//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "KeyFrames.h"
#include "AnimBlendSequence.h"
#ifdef __cplusplus
struct CAnimBlendAssociation;
#else
typedef struct CAnimBlendAssociation CAnimBlendAssociation;
#endif
typedef struct CAnimBlendNode {
    float theta,invSin;
    int32_t frameA,frameB;
    float remainingTime;
    CAnimBlendSequence *sequence;
    CAnimBlendAssociation *association;
} CAnimBlendNode;
#ifdef __cplusplus
extern "C" {
#endif
void AnimBlendNode_Init(CAnimBlendNode *self);
bool AnimBlendNode_Update(CAnimBlendNode *self,KeyFrameVectorOut trans,CQuaternion *rot,float weight);
bool AnimBlendNode_UpdateCompressed(CAnimBlendNode *self,KeyFrameVectorOut trans,CQuaternion *rot,float weight);
bool AnimBlendNode_NextKeyFrame(CAnimBlendNode *self);
bool AnimBlendNode_NextKeyFrameCompressed(CAnimBlendNode *self);
bool AnimBlendNode_FindKeyFrame(CAnimBlendNode *self,float t);
bool AnimBlendNode_SetupKeyFrameCompressed(CAnimBlendNode *self);
void AnimBlendNode_CalcDeltas(CAnimBlendNode *self);
void AnimBlendNode_CalcDeltasCompressed(CAnimBlendNode *self);
void AnimBlendNode_GetCurrentTranslation(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight);
void AnimBlendNode_GetCurrentTranslationCompressed(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight);
void AnimBlendNode_GetEndTranslation(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight);
void AnimBlendNode_GetEndTranslationCompressed(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
