//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendAssocGroup.h"
#include "AnimBlendAssocGroup_backend.h"
#include "AnimBlendAssociation_backend.h"
#include "ModelIndices.h"
#include "General.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wctype.h>
#define AnimGroup_Debug(format,...) re3_debug("[DBG]: " format,__VA_ARGS__)
bool AnimGroup_CompareIgnoringDigits(const char *s1,const char *s2)
{
    // Preserve paired digit skipping, original prefix matching and locale character queries

	char c1, c2;

	for(;;){
		c1 = *s1;
		c2 = *s2;
		if(c1) s1++;
		if(c2) s2++;
		if(c1 == '\0' && c2 == '\0') return true;
#ifndef ASCII_STRCMP
		if(iswdigit(c1) && iswdigit(c2))
#else
		if(__ascii_iswdigit(c1) && __ascii_iswdigit(c2))
#endif
			continue;
#ifndef ASCII_STRCMP
		c1 = toupper(c1);
		c2 = toupper(c2);
#else
		c1 = __ascii_toupper(c1);
		c2 = __ascii_toupper(c2);
#endif

		if(c1 && c2 && c1 != c2)
			return false;
	}
}
void *AnimGroup_FindModel(const char *name)
{
    // Keep the complete original player-name substitution and ordered model selection loop

	int i;
	void *mi;
	char playername[32];

	if(AnimGroup_HostPrefixCompare(name, "CSplay", 6) == 0 &&
	   AnimGroup_HostPrefixCompare(AnimGroup_HostModelName(AnimGroup_HostModelAt(MI_PLAYER)), "ig", 2) == 0){
		strcpy(playername, AnimGroup_HostModelName(AnimGroup_HostModelAt(MI_PLAYER)));
		playername[0] = 'C';
		playername[1] = 'S';
		name = playername;
	}

	for(i = 0; i < MODELINFOSIZE; i++){
		mi = AnimGroup_HostModelAt(i);
		if(mi && AnimGroup_HostModelObject(mi) && AnimGroup_HostObjectIsClump(AnimGroup_HostModelObject(mi)) &&
		   AnimGroup_CompareIgnoringDigits(AnimGroup_HostModelName(mi), name))
			return mi;
	}
	return NULL;
}
void AnimGroup_Init(CAnimBlendAssocGroup *self)
{
    // Preserve the complete original group policy and ordered ownership operations

	self->animBlock = NULL;
	self->assocList = NULL;
	self->numAssociations = 0;
	self->firstAnimId = 0;
	self->groupId = -1;
}
void AnimGroup_Destroy(CAnimBlendAssocGroup *self)
{
    // Preserve the complete original group policy and ordered ownership operations

	AnimGroup_DestroyAssociations(self);
}
void AnimGroup_DestroyAssociations(CAnimBlendAssocGroup *self)
{
    // Preserve the complete original group policy and ordered ownership operations

	if(self->assocList){
//+ rouz edit (ChatGPT)
		// Destroy and release associations without invoking C++ array delete.
		for(int i = 0; i < self->numAssociations; i++)
			// Use the complete C association at the original native operation boundary
			AnimAssociation_Destroy(&self->assocList[i]);
		free(self->assocList);
//- rouz edit (ChatGPT)
		self->assocList = NULL;
		self->numAssociations = 0;
	}
}
CAnimBlendAssociation * AnimGroup_GetById(CAnimBlendAssocGroup *self,uint32_t id)
{
    // Preserve the complete original group policy and ordered ownership operations

	return &self->assocList[id - self->firstAnimId];
}
CAnimBlendAssociation * AnimGroup_GetByName(CAnimBlendAssocGroup *self,const char *name)
{
    // Preserve the complete original group policy and ordered ownership operations

	int i;
	for(i = 0; i < self->numAssociations; i++)
		if(!General_faststricmp(self->assocList[i].hierarchy->name, name))
			return &self->assocList[i];
	AnimGroup_Debug("\n\nCan't find the fucking animation %s\n\n\n", name);
	return NULL;
}
CAnimBlendAssociation * AnimGroup_CopyById(CAnimBlendAssocGroup *self,uint32_t id)
{
    // Preserve the complete original group policy and ordered ownership operations

	CAnimBlendAssociation *anim = AnimGroup_GetById(self,id);
	if(anim == NULL)
		return NULL;
	AnimAssociation_HostUncompressAnimation(anim->hierarchy);
//+ rouz edit (ChatGPT)
	// Construct the animation copy without invoking C++ new.
	CAnimBlendAssociation *copy = (CAnimBlendAssociation*)malloc(sizeof(CAnimBlendAssociation));
	// Use the complete C association at the original native operation boundary
	AnimAssociation_CopyConstruct(copy,anim);
	return copy;
//- rouz edit (ChatGPT)
}
CAnimBlendAssociation * AnimGroup_CopyByName(CAnimBlendAssocGroup *self,const char *name)
{
    // Preserve the complete original group policy and ordered ownership operations

	CAnimBlendAssociation *anim = AnimGroup_GetByName(self,name);
	if(anim == NULL)
		return NULL;
	AnimAssociation_HostUncompressAnimation(anim->hierarchy);
//+ rouz edit (ChatGPT)
	// Construct the animation copy without invoking C++ new.
	CAnimBlendAssociation *copy = (CAnimBlendAssociation*)malloc(sizeof(CAnimBlendAssociation));
	// Use the complete C association at the original native operation boundary
	AnimAssociation_CopyConstruct(copy,anim);
	return copy;
//- rouz edit (ChatGPT)
}
void AnimGroup_CreateForModels(CAnimBlendAssocGroup *self,const char *name)
{
    // Preserve the complete original group policy and ordered ownership operations

	int i;
	CAnimBlock *animBlock;

	AnimGroup_DestroyAssociations(self);

	animBlock = AnimGroup_HostBlock(name);
//+ rouz edit (ChatGPT)
	// Allocate and construct associations without invoking C++ array new.
	self->assocList = (CAnimBlendAssociation*)malloc(sizeof(CAnimBlendAssociation)*animBlock->numAnims);
	for(i = 0; i < animBlock->numAnims; i++)
		// Use the complete C association at the original native operation boundary
		AnimAssociation_Init(&self->assocList[i]);
//- rouz edit (ChatGPT)
	self->numAssociations = 0;

	for(i = 0; i < animBlock->numAnims; i++){
		CAnimBlendHierarchy *anim = AnimGroup_HostAnimationId(animBlock->firstIndex + i);
		void *model = AnimGroup_FindModel(anim->name);
		if(model){
			AnimGroup_Debug("Associated anim %s with model %s\n", anim->name, AnimGroup_HostModelName(model));
			void *clump = AnimGroup_HostCreateInstance(model);
			AnimGroup_HostInitClump(clump);
			// Use the complete C association at the original native operation boundary
			AnimAssociation_BindClump(&self->assocList[i],clump, anim);
			if(AnimGroup_HostIsSkinned(clump))
				AnimGroup_HostRemoveSkinAnimation(clump);
			AnimGroup_HostDestroyClump(clump);
			self->assocList[i].animId = self->firstAnimId + i;
			self->assocList[i].groupId = self->groupId;
		}else
			AnimGroup_Debug("\n\nCANNOT FIND MODELINFO WITH NAME %s\n\n\n", anim->name);
	}
	self->numAssociations = animBlock->numAnims;
}
void AnimGroup_CreateForClump(CAnimBlendAssocGroup *self,const char *blockName,void *clump,const char **animNames,int32_t numAssocs)
{
    // Preserve the complete original group policy and ordered ownership operations

	int i;

	AnimGroup_DestroyAssociations(self);

	self->animBlock = AnimGroup_HostBlock(blockName);
//+ rouz edit (ChatGPT)
	// Allocate and construct associations without invoking C++ array new.
	self->assocList = (CAnimBlendAssociation*)malloc(sizeof(CAnimBlendAssociation)*numAssocs);
	for(i = 0; i < numAssocs; i++)
		// Use the complete C association at the original native operation boundary
		AnimAssociation_Init(&self->assocList[i]);
//- rouz edit (ChatGPT)

	self->numAssociations = 0;
	for(i = 0; i < numAssocs; i++){
		// Use the complete C association at the original native operation boundary
		AnimAssociation_BindClump(&self->assocList[i],clump, AnimGroup_HostAnimationName(animNames[i], self->animBlock));
		self->assocList[i].animId = self->firstAnimId + i;
		self->assocList[i].groupId = self->groupId;
	}
	self->numAssociations = numAssocs;
}

static CAnimBlendAssocGroup *cutsceneGroup;
static void AnimGroup_DestroyStaticCutscene(void)
{
    // Release the same static cutscene group at process exit through the complete C owner
    AnimGroup_Destroy(cutsceneGroup);
}
CAnimBlendAssocGroup AnimGroup_InitStaticCutscene(CAnimBlendAssocGroup *self)
{
    // Retain pre-main initialization and matching exit registration at the original native storage definition
    AnimGroup_Init(self);cutsceneGroup=self;atexit(AnimGroup_DestroyStaticCutscene);return *self;
}
//- rouz edit (ChatGPT)
