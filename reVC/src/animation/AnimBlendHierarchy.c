//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendHierarchy.h"
#include "AnimBlendHierarchy_backend.h"
#include <stdlib.h>
#include <string.h>
void re3_assert(const char *expression,const char *filename,unsigned int line,const char *function);
#ifndef MASTER
#define Hierarchy_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define Hierarchy_Assert(expression) (expression)
#endif
#define Hierarchy_Max(a,b) ((a)>(b)?(a):(b))
void AnimHierarchy_Init(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
	self->sequences = NULL;
	self->numSequences = 0;
	self->compressed = 0;
	self->totalLength = 0.0f;
	self->linkPtr = NULL;
}
void AnimHierarchy_Shutdown(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
	AnimHierarchy_HostRemoveCache(self);
	AnimHierarchy_RemoveAnimSequences(self);
	self->totalLength = 0.0f;
	self->compressed = 0;
}
void AnimHierarchy_SetName(CAnimBlendHierarchy *self,char *name)
{
    // Preserve the complete original hierarchy policy and its live field reads
	strncpy(self->name, name, 24);
}
void AnimHierarchy_CalcTotalTime(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
	int i, j;

	self->totalLength = 0.0f;

	for(i = 0; i < self->numSequences; i++){
#ifdef FIX_BUGS
		if(self->sequences[i].numFrames == 0)
			continue;
#endif

		// Use the complete C sequence at the original native operation boundary
		self->totalLength = Hierarchy_Max(self->totalLength, AnimSequence_GetKeyFrame(&self->sequences[i],self->sequences[i].numFrames-1)->deltaTime);
		for(j = self->sequences[i].numFrames-1; j >= 1; j--){
			// Use the complete C sequence at the original native operation boundary
			KeyFrame *kf1 = AnimSequence_GetKeyFrame(&self->sequences[i],j);
			// Use the complete C sequence at the original native operation boundary
			KeyFrame *kf2 = AnimSequence_GetKeyFrame(&self->sequences[i],j-1);
			kf1->deltaTime -= kf2->deltaTime;
		}
	}
}
void AnimHierarchy_CalcTotalTimeCompressed(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
	int i, j;

	self->totalLength = 0.0f;

	for(i = 0; i < self->numSequences; i++){
#ifdef FIX_BUGS
		if(self->sequences[i].numFrames == 0)
			continue;
#endif

		// Use the C keyframe operation and its actual embedded base fields
		// Use the complete C sequence at the original native operation boundary
		self->totalLength = Hierarchy_Max(self->totalLength, KeyFrameCompressed_GetDeltaTime(AnimSequence_GetKeyFrameCompressed(&self->sequences[i],self->sequences[i].numFrames-1)));
		for(j = self->sequences[i].numFrames-1; j >= 1; j--){
			// Use the complete C sequence at the original native operation boundary
			KeyFrameCompressed *kf1 = AnimSequence_GetKeyFrameCompressed(&self->sequences[i],j);
			// Use the complete C sequence at the original native operation boundary
			KeyFrameCompressed *kf2 = AnimSequence_GetKeyFrameCompressed(&self->sequences[i],j-1);
			kf1->deltaTime -= kf2->deltaTime;
		}
	}
}
void AnimHierarchy_RemoveQuaternionFlips(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
	int i;

	for(i = 0; i < self->numSequences; i++)
		// Use the complete C sequence at the original native operation boundary
		AnimSequence_RemoveQuaternionFlips(&self->sequences[i]);
}
void AnimHierarchy_RemoveAnimSequences(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
//+ rouz edit (ChatGPT)
	// Destroy and release self->sequences without invoking C++ array delete.
	for(int i = 0; i < self->numSequences; i++)
		// Use the complete C sequence at the original native operation boundary
		AnimSequence_Destroy(&self->sequences[i]);
	free(self->sequences);
//- rouz edit (ChatGPT)
	self->sequences = NULL;
	self->numSequences = 0;
}
void AnimHierarchy_Uncompress(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
#ifdef ANIM_COMPRESSION
	int i;
	Hierarchy_Assert(self->compressed);
	for(i = 0; i < self->numSequences; i++)
		// Use the complete C sequence at the original native operation boundary
		AnimSequence_Uncompress(&self->sequences[i]);
#endif
	self->compressed = 0;
	if(self->totalLength == 0.0f){
		AnimHierarchy_RemoveQuaternionFlips(self);
		AnimHierarchy_CalcTotalTime(self);
	}
}
void AnimHierarchy_RemoveUncompressedData(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
#ifdef ANIM_COMPRESSION
	int i;
	Hierarchy_Assert(!self->compressed);
	for(i = 0; i < self->numSequences; i++)
		// Use the complete C sequence at the original native operation boundary
		AnimSequence_RemoveUncompressedData(&self->sequences[i]);
#endif
	self->compressed = 1;
}
#ifdef USE_CUSTOM_ALLOCATOR
void AnimHierarchy_MoveMemory(CAnimBlendHierarchy *self,bool onlyone)
{
    // Preserve the complete original hierarchy policy and its live field reads
	int i;
	for(i = 0; i < self->numSequences; i++)
		// Use the complete C sequence at the original native operation boundary
		if(AnimSequence_MoveMemory(&self->sequences[i]) && onlyone)
			return;
}
#endif
bool AnimHierarchy_IsCompressed(CAnimBlendHierarchy *self)
{
    // Preserve the complete original hierarchy policy and its live field reads
return !!self->compressed;
}
//- rouz edit (ChatGPT)
