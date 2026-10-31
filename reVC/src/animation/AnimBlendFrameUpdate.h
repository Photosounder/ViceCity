//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendNode.h"
#include "AnimBlendFrameData.h"
struct CAnimBlendClumpData;
typedef struct AnimBlendFrameUpdateData {
    int foobar;
    CAnimBlendNode *nodes[16];
} AnimBlendFrameUpdateData;
#ifdef __cplusplus
extern "C" {
#endif
extern struct CAnimBlendClumpData *gpAnimBlendClump;
void FrameUpdateCallBackNonSkinned(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackWithVelocityExtractionNonSkinned(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackWith3dVelocityExtractionNonSkinned(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackSkinned(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackWithVelocityExtractionSkinned(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackWith3dVelocityExtractionSkinned(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackOffscreen(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackNonSkinnedCompressed(AnimBlendFrameData *frame,void *arg);
void FrameUpdateCallBackSkinnedCompressed(AnimBlendFrameData *frame,void *arg);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
