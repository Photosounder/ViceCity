//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include "AnimBlendAssocGroup.h"
#ifdef __cplusplus
extern "C" {
#endif
CAnimBlock *AnimGroup_HostBlock(const char *name);
CAnimBlendHierarchy *AnimGroup_HostAnimationId(int32_t id);
CAnimBlendHierarchy *AnimGroup_HostAnimationName(const char *name,CAnimBlock *block);
void *AnimGroup_HostModelAt(int32_t id);
const char *AnimGroup_HostModelName(void *model);
void *AnimGroup_HostModelObject(void *model);
bool AnimGroup_HostObjectIsClump(void *object);
void *AnimGroup_HostCreateInstance(void *model);
void AnimGroup_HostInitClump(void *clump);
bool AnimGroup_HostIsSkinned(void *clump);
void AnimGroup_HostRemoveSkinAnimation(void *clump);
void AnimGroup_HostDestroyClump(void *clump);
int AnimGroup_HostPrefixCompare(const char *a,const char *b,size_t count);
void re3_debug(const char *format,...);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
