//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendAssociation_backend.h"
#include "AnimBlendClumpData_backend.h"
#include <stddef.h>
#include <stdlib.h>
#define Association_Max(a,b) ((a)>(b)?(a):(b))
#define Association_Min(a,b) ((a)<(b)?(a):(b))
void AnimAssociation_Init(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
	self->groupId = -1;
	self->nodes = NULL;
	self->blendAmount = 1.0f;
	self->blendDelta = 0.0f;
	self->currentTime = 0.0f;
	self->speed = 1.0f;
	self->timeStep = 0.0f;
	self->animId = -1;
	self->flags = 0;
	self->callbackType = ANIM_ASSOC_CB_NONE;
	// Use the C animation owner at the original native operation boundary
	AnimBlendLink_Init(&self->link);
}
void AnimAssociation_CopyConstruct(CAnimBlendAssociation *self,CAnimBlendAssociation *other)
{
    // Preserve the complete original association policy and ordered live field accesses
	self->nodes = NULL;
	self->blendAmount = 1.0f;
	self->blendDelta = 0.0f;
	self->currentTime = 0.0f;
	self->speed = 1.0f;
	self->timeStep = 0.0f;
	self->callbackType = ANIM_ASSOC_CB_NONE;
	// Use the C animation owner at the original native operation boundary
	AnimBlendLink_Init(&self->link);
	AnimAssociation_InitFrom(self,other);
}
void AnimAssociation_Destroy(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
	AnimAssociation_FreeAnimBlendNodeArray(self);
	// Use the C animation owner at the original native operation boundary
	AnimBlendLink_Remove(&self->link);
}
void AnimAssociation_AllocateAnimBlendNodeArray(CAnimBlendAssociation *self,int32_t n)
{
    // Preserve the complete original association policy and ordered live field accesses
	int i;

	self->nodes = (CAnimBlendNode*)AnimBlendClump_BackendAllocate(n*sizeof(CAnimBlendNode), 64);
	for(i = 0; i < n; i++)
		// Use the C blend-node policy at the original native call boundary
		AnimBlendNode_Init(&self->nodes[i]);
}
void AnimAssociation_FreeAnimBlendNodeArray(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
	if(self->nodes)
		AnimBlendClump_BackendFree(self->nodes);
}
void AnimAssociation_BindClump(CAnimBlendAssociation *self,void *clump,CAnimBlendHierarchy *hier)
{
    // Preserve the complete original association policy and ordered live field accesses
	int i;
	AnimBlendFrameData *frame;

	CAnimBlendClumpData *clumpData = AnimAssociation_HostClumpData(clump);
	self->numNodes = clumpData->numFrames;
	AnimAssociation_AllocateAnimBlendNodeArray(self,self->numNodes);
	for(i = 0; i < self->numNodes; i++)
		self->nodes[i].association = self;
	self->hierarchy = hier;

	// Init every node from a sequence and a Clump frame
	// NB: This is where the order of self->nodes is defined
	for(i = 0; i < hier->numSequences; i++){
		CAnimBlendSequence *seq = &hier->sequences[i];
		if(seq->boneTag == -1)
			frame = AnimAssociation_HostFindFrame(clump, seq->name);
		else
			frame = AnimAssociation_HostFindBone(clump, seq->boneTag);
		if(frame && seq->numFrames > 0)
			self->nodes[frame - clumpData->frames].sequence = seq;
	}
}
void AnimAssociation_InitFrom(CAnimBlendAssociation *self,CAnimBlendAssociation *other)
{
    // Preserve the complete original association policy and ordered live field accesses
	int i;

	self->hierarchy = other->hierarchy;
	self->numNodes = other->numNodes;
	self->flags = other->flags;
	self->animId = other->animId;
	self->groupId = other->groupId;
	AnimAssociation_AllocateAnimBlendNodeArray(self,self->numNodes);
	for(i = 0; i < self->numNodes; i++){
		self->nodes[i] = other->nodes[i];
		self->nodes[i].association = self;
	}
}
void AnimAssociation_SetBlend(CAnimBlendAssociation *self,float amount,float delta)
{
    // Preserve the complete original association policy and ordered live field accesses
	self->blendAmount = amount;
	self->blendDelta = delta;
}
void AnimAssociation_SetFinishCallback(CAnimBlendAssociation *self,AnimAssociationCallback cb,void *arg)
{
    // Preserve the complete original association policy and ordered live field accesses
	self->callbackType = ANIM_ASSOC_CB_FINISH;
	self->callback = cb;
	self->callbackArg = arg;
}
void AnimAssociation_SetDeleteCallback(CAnimBlendAssociation *self,AnimAssociationCallback cb,void *arg)
{
    // Preserve the complete original association policy and ordered live field accesses
	self->callbackType = ANIM_ASSOC_CB_DELETE;
	self->callback = cb;
	self->callbackArg = arg;
}
void AnimAssociation_SetCurrentTime(CAnimBlendAssociation *self,float time)
{
    // Preserve the complete original association policy and ordered live field accesses
	int i;

	for(self->currentTime = time; self->currentTime >= self->hierarchy->totalLength; self->currentTime -= self->hierarchy->totalLength)
		if (!AnimAssociation_IsRepeating(self)) {
			self->currentTime = self->hierarchy->totalLength;
			break;
		}

	AnimAssociation_HostUncompressAnimation(self->hierarchy);
#ifdef ANIM_COMPRESSION
	// strangely PC has self but android doesn't
	if(self->hierarchy->keepCompressed){
		for(i = 0; i < self->numNodes; i++)
			if(self->nodes[i].sequence)
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_SetupKeyFrameCompressed(&self->nodes[i]);
	}else
#endif
	{
		for(i = 0; i < self->numNodes; i++)
			if(self->nodes[i].sequence)
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_FindKeyFrame(&self->nodes[i],self->currentTime);
	}
}
void AnimAssociation_SyncAnimation(CAnimBlendAssociation *self,CAnimBlendAssociation *other)
{
    // Preserve the complete original association policy and ordered live field accesses
	AnimAssociation_SetCurrentTime(self,other->currentTime/other->hierarchy->totalLength * self->hierarchy->totalLength);
}
void AnimAssociation_Start(CAnimBlendAssociation *self,float time)
{
    // Preserve the complete original association policy and ordered live field accesses
	self->flags |= ASSOC_RUNNING;
	AnimAssociation_SetCurrentTime(self,time);
}
void AnimAssociation_UpdateTimeStep(CAnimBlendAssociation *self,float timeDelta,float relSpeed)
{
    // Preserve the complete original association policy and ordered live field accesses
	if(AnimAssociation_IsRunning(self))
		self->timeStep = (self->flags & ASSOC_MOVEMENT ? relSpeed*self->hierarchy->totalLength : self->speed) * timeDelta;
}
bool AnimAssociation_UpdateTime(CAnimBlendAssociation *self,float timeDelta,float relSpeed)
{
    // Preserve the complete original association policy and ordered live field accesses
	if(!AnimAssociation_IsRunning(self))
		return true;
	if(self->currentTime >= self->hierarchy->totalLength){
		self->flags &= ~ASSOC_RUNNING;
		return true;
	}

	self->currentTime += self->timeStep;

	if(self->currentTime >= self->hierarchy->totalLength){
		// Ran past end

		if(AnimAssociation_IsRepeating(self))
			self->currentTime -= self->hierarchy->totalLength;
		else{
			self->currentTime = self->hierarchy->totalLength;
			if(self->flags & ASSOC_FADEOUTWHENDONE){
				self->flags |= ASSOC_DELETEFADEDOUT;
				self->blendDelta = -4.0f;
			}
			if(self->callbackType == ANIM_ASSOC_CB_FINISH){
				self->callbackType = ANIM_ASSOC_CB_NONE;
				self->callback(self, self->callbackArg);
			}
		}
	}
	return true;
}
bool AnimAssociation_UpdateBlend(CAnimBlendAssociation *self,float timeDelta)
{
    // Preserve the complete original association policy and ordered live field accesses
	self->blendAmount += self->blendDelta * timeDelta;

	if(self->blendAmount <= 0.0f && self->blendDelta < 0.0f){
		// We're faded out and are not fading in
		self->blendAmount = 0.0f;
		self->blendDelta = Association_Max(0.0f, self->blendDelta);
		if(self->flags & ASSOC_DELETEFADEDOUT){
			if(self->callbackType == ANIM_ASSOC_CB_FINISH || self->callbackType == ANIM_ASSOC_CB_DELETE)
				self->callback(self, self->callbackArg);
//+ rouz edit (ChatGPT)
			// Destroy and release self association without invoking C++ delete.
			AnimAssociation_Destroy(self);
			free(self);
//- rouz edit (ChatGPT)
			return false;
		}
	}

	if(self->blendAmount > 1.0f){
		// Maximally faded in, clamp values
		self->blendAmount = 1.0f;
		self->blendDelta = Association_Min(0.0f, self->blendDelta);
	}

	return true;
}
bool AnimAssociation_IsRunning(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return !!(self->flags&ASSOC_RUNNING);
}
bool AnimAssociation_IsRepeating(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return !!(self->flags&ASSOC_REPEAT);
}
bool AnimAssociation_IsPartial(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return !!(self->flags&ASSOC_PARTIAL);
}
bool AnimAssociation_IsMovement(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return !!(self->flags&ASSOC_MOVEMENT);
}
bool AnimAssociation_HasTranslation(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return !!(self->flags&ASSOC_HAS_TRANSLATION);
}
bool AnimAssociation_HasXTranslation(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return !!(self->flags&ASSOC_HAS_X_TRANSLATION);
}
float AnimAssociation_GetBlendAmount(CAnimBlendAssociation *self,float weight)
{
    // Preserve the complete original association policy and ordered live field accesses
return AnimAssociation_IsPartial(self)?self->blendAmount:self->blendAmount*weight;
}
CAnimBlendNode * AnimAssociation_GetNode(CAnimBlendAssociation *self,int32_t i)
{
    // Preserve the complete original association policy and ordered live field accesses
return &self->nodes[i];
}
void AnimAssociation_SetRun(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
self->flags|=ASSOC_RUNNING;
}
float AnimAssociation_GetTimeLeft(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return self->hierarchy->totalLength-self->currentTime;
}
float AnimAssociation_GetProgress(CAnimBlendAssociation *self)
{
    // Preserve the complete original association policy and ordered live field accesses
return self->currentTime/self->hierarchy->totalLength;
}
CAnimBlendAssociation * AnimAssociation_FromLink(CAnimBlendLink *link)
{
    // Preserve the complete original association policy and ordered live field accesses
return (CAnimBlendAssociation*)((uint8_t*)link - offsetof(CAnimBlendAssociation,link));
}
//- rouz edit (ChatGPT)
