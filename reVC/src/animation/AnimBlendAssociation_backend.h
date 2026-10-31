//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendAssociation.h"
#include "AnimBlendClumpData.h"
#ifdef __cplusplus
extern "C" {
#endif
CAnimBlendClumpData *AnimAssociation_HostClumpData(void *clump);
AnimBlendFrameData *AnimAssociation_HostFindFrame(void *clump,const char *name);
AnimBlendFrameData *AnimAssociation_HostFindBone(void *clump,uint32_t tag);
void AnimAssociation_HostUncompressAnimation(CAnimBlendHierarchy *hierarchy);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
