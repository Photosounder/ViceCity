//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendSequence.h"
#include "LinkList.h"
typedef struct CAnimBlendHierarchy {
    char name[24];
    CAnimBlendSequence *sequences;
    int16_t numSequences;
    bool compressed,keepCompressed;
    float totalLength;
    CLink *linkPtr;
} CAnimBlendHierarchy;
#ifdef __cplusplus
extern "C" {
#endif
void AnimHierarchy_Init(CAnimBlendHierarchy *self);
void AnimHierarchy_Shutdown(CAnimBlendHierarchy *self);
void AnimHierarchy_SetName(CAnimBlendHierarchy *self,char *name);
void AnimHierarchy_CalcTotalTime(CAnimBlendHierarchy *self);
void AnimHierarchy_CalcTotalTimeCompressed(CAnimBlendHierarchy *self);
void AnimHierarchy_RemoveQuaternionFlips(CAnimBlendHierarchy *self);
void AnimHierarchy_RemoveAnimSequences(CAnimBlendHierarchy *self);
void AnimHierarchy_Uncompress(CAnimBlendHierarchy *self);
void AnimHierarchy_RemoveUncompressedData(CAnimBlendHierarchy *self);
void AnimHierarchy_MoveMemory(CAnimBlendHierarchy *self,bool onlyone);
bool AnimHierarchy_IsCompressed(CAnimBlendHierarchy *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
