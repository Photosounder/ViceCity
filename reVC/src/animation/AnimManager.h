//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendHierarchy.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendAssocGroup.h"
#include "AnimBlock.h"
#include "AnimationId.h"
#include "AnimGroups.h"
#include "AnimManagerData.h"
#ifdef __cplusplus
extern "C" {
#endif
void AnimManager_Initialise(void);
void AnimManager_Shutdown(void);
void AnimManager_UncompressAnimation(CAnimBlendHierarchy *hier);
void AnimManager_RemoveFromUncompressedCache(CAnimBlendHierarchy *hier);
CAnimBlock * AnimManager_GetAnimationBlockById(int32_t block);
CAnimBlock * AnimManager_GetAnimationBlockByName(const char *name);
int32_t AnimManager_GetAnimationBlockIndex(const char *name);
int32_t AnimManager_RegisterAnimBlock(const char *name);
int32_t AnimManager_GetNumRefsToAnimBlock(int32_t block);
void AnimManager_AddAnimBlockRef(int32_t block);
void AnimManager_RemoveAnimBlockRefWithoutDelete(int32_t block);
void AnimManager_RemoveAnimBlockRef(int32_t block);
void AnimManager_RemoveAnimBlock(int32_t block);
CAnimBlendHierarchy * AnimManager_GetAnimationByName(const char *name,CAnimBlock *animBlock);
CAnimBlendHierarchy * AnimManager_GetAnimationById(int32_t n);
const char * AnimManager_GetAnimGroupName(int32_t groupId);
CAnimBlendAssociation * AnimManager_CreateAnimAssociation(int32_t groupId,int32_t animId);
CAnimBlendAssociation * AnimManager_GetAnimAssociationById(int32_t groupId,int32_t animId);
CAnimBlendAssociation * AnimManager_GetAnimAssociationByName(int32_t groupId,const char *name);
CAnimBlendAssociation * AnimManager_AddAnimation(void *clump,int32_t groupId,int32_t animId);
CAnimBlendAssociation * AnimManager_AddAnimationAndSync(void *clump,CAnimBlendAssociation *syncanim,int32_t groupId,int32_t animId);
CAnimBlendAssociation * AnimManager_BlendAnimation(void *clump,int32_t groupId,int32_t animId,float delta);
void AnimManager_LoadAnimFiles(void);
void AnimManager_LoadAnimFileByName(const char *filename);
void AnimManager_LoadAnimStream(void *stream,bool compress,char (*uncompressedAnims)[32]);
void AnimManager_CreateAnimAssocGroups(void);
void AnimManager_RemoveLastAnimFile(void);
CAnimBlendAssocGroup * AnimManager_GetAnimAssocGroups(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
