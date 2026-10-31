//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "KeyFrames.h"
typedef struct CAnimBlendSequence {
    int32_t type;
    char name[24];
    int32_t numFrames;
    int16_t boneTag;
    void *keyFrames,*keyFramesCompressed;
} CAnimBlendSequence;
#ifdef __cplusplus
extern "C" {
#endif
void AnimSequence_Init(CAnimBlendSequence *self);
void AnimSequence_Destroy(CAnimBlendSequence *self);
void AnimSequence_SetName(CAnimBlendSequence *self,char *name);
void AnimSequence_SetNumFrames(CAnimBlendSequence *self,int32_t numFrames,bool translation,bool compressed);
void AnimSequence_RemoveQuaternionFlips(CAnimBlendSequence *self);
KeyFrame * AnimSequence_GetKeyFrame(CAnimBlendSequence *self,int32_t n);
KeyFrameCompressed * AnimSequence_GetKeyFrameCompressed(CAnimBlendSequence *self,int32_t n);
bool AnimSequence_HasTranslation(CAnimBlendSequence *self);
void AnimSequence_Uncompress(CAnimBlendSequence *self);
void AnimSequence_CompressKeyframes(CAnimBlendSequence *self);
void AnimSequence_RemoveUncompressedData(CAnimBlendSequence *self);
bool AnimSequence_MoveMemory(CAnimBlendSequence *self);
void AnimSequence_SetBoneTag(CAnimBlendSequence *self,int32_t tag);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
