//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "AnimBlock.h"
#include "AnimBlendAssociation.h"
typedef struct CAnimBlendAssocGroup {
    CAnimBlock *animBlock;
    CAnimBlendAssociation *assocList;
    int32_t numAssociations,firstAnimId,groupId;
} CAnimBlendAssocGroup;
#ifdef __cplusplus
extern "C" {
#endif
void AnimGroup_Init(CAnimBlendAssocGroup *self);
void AnimGroup_Destroy(CAnimBlendAssocGroup *self);
void AnimGroup_DestroyAssociations(CAnimBlendAssocGroup *self);
CAnimBlendAssociation * AnimGroup_GetById(CAnimBlendAssocGroup *self,uint32_t id);
CAnimBlendAssociation * AnimGroup_GetByName(CAnimBlendAssocGroup *self,const char *name);
CAnimBlendAssociation * AnimGroup_CopyById(CAnimBlendAssocGroup *self,uint32_t id);
CAnimBlendAssociation * AnimGroup_CopyByName(CAnimBlendAssocGroup *self,const char *name);
void AnimGroup_CreateForModels(CAnimBlendAssocGroup *self,const char *name);
void AnimGroup_CreateForClump(CAnimBlendAssocGroup *self,const char *blockName,void *clump,const char **animNames,int32_t numAssocs);
CAnimBlendAssocGroup AnimGroup_InitStaticCutscene(CAnimBlendAssocGroup *self);
bool AnimGroup_CompareIgnoringDigits(const char *s1,const char *s2);
void *AnimGroup_FindModel(const char *name);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
