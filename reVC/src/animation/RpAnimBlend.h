//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendAssociation.h"
#include "AnimBlendFrameUpdate.h"
typedef struct CAnimBlendClumpData CAnimBlendClumpData;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t ClumpOffset;
static inline CAnimBlendClumpData **RpAnimBlendClumpDataSlot(void *clump)
{
    // Address only the SDK-registered plugin slot using its original byte offset
    return (CAnimBlendClumpData**)((unsigned char*)clump+ClumpOffset);
}
#define RPANIMBLENDCLUMPDATA(object) (RpAnimBlendClumpDataSlot((void*)(object)))
bool RpAnimBlendPluginAttach(void);
CAnimBlendAssociation * RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc);
CAnimBlendAssociation * RpAnimBlendGetNextAssociationWithMask(CAnimBlendAssociation *assoc,uint32_t mask);
void RpAnimBlendAllocateData(void *clump);
void RpAnimBlendClumpSetBlendDeltas(void *clump,uint32_t mask,float delta);
void RpAnimBlendClumpRemoveAllAssociations(void *clump);
void RpAnimBlendClumpRemoveAssociations(void *clump,uint32_t mask);
void RpAnimBlendClumpInit(void *clump);
bool RpAnimBlendClumpIsInitialized(void *clump);
void RpAnimBlendClumpFillFrameArray(void *clump,AnimBlendFrameData **frames);
AnimBlendFrameData * RpAnimBlendClumpFindFrame(void *clump,const char *name);
AnimBlendFrameData * RpAnimBlendClumpFindBone(void *clump,uint32_t boneTag);
CAnimBlendAssociation * RpAnimBlendClumpGetAssociation(void *clump,uint32_t id);
CAnimBlendAssociation * RpAnimBlendClumpGetMainAssociation(void *clump,CAnimBlendAssociation **assocRet,float *blendRet);
CAnimBlendAssociation * RpAnimBlendClumpGetMainPartialAssociation(void *clump);
CAnimBlendAssociation * RpAnimBlendClumpGetMainAssociation_N(void *clump,int n);
CAnimBlendAssociation * RpAnimBlendClumpGetMainPartialAssociation_N(void *clump,int n);
CAnimBlendAssociation * RpAnimBlendClumpGetFirstAssociationWithMask(void *clump,uint32_t mask);
CAnimBlendAssociation * RpAnimBlendClumpGetFirstAssociation(void *clump);
void RpAnimBlendNodeUpdateKeyframes(AnimBlendFrameData *frames,AnimBlendFrameUpdateData *updateData,int32_t numNodes);
void RpAnimBlendClumpUpdateAnimations(void *clump,float timeDelta,bool doRender);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
