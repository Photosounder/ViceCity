//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendSequence.h"
#include "AnimBlendSequence_backend.h"
#include <stddef.h>
#include <string.h>
void AnimSequence_Init(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
	self->type = 0;
	self->numFrames = 0;
	self->keyFrames = NULL;
	self->keyFramesCompressed = NULL;
	self->boneTag = -1;
}
void AnimSequence_Destroy(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
	if(self->keyFrames)
		AnimSequence_BackendFree(self->keyFrames);
	if(self->keyFramesCompressed)
		AnimSequence_BackendFree(self->keyFramesCompressed);
}
void AnimSequence_SetName(CAnimBlendSequence *self,char *name)
{
    // Retain the complete original sequence policy and its observable operation order
	strncpy(self->name, name, 24);
}
void AnimSequence_SetNumFrames(CAnimBlendSequence *self,int32_t numFrames,bool translation,bool compressed)
{
    // Retain the complete original sequence policy and its observable operation order
	if(translation){
		self->type |= ANIM_KEYFRAME_ROT | ANIM_KEYFRAME_TRANS;
		if(compressed)
			self->keyFramesCompressed = AnimSequence_BackendAllocate(sizeof(KeyFrameTrans) * numFrames);
		else
			self->keyFrames = AnimSequence_BackendAllocate(sizeof(KeyFrameTrans) * numFrames);
	}else{
		self->type |= ANIM_KEYFRAME_ROT;
		if(compressed)
			self->keyFramesCompressed = AnimSequence_BackendAllocate(sizeof(KeyFrame) * numFrames);
		else
			self->keyFrames = AnimSequence_BackendAllocate(sizeof(KeyFrame) * numFrames);
	}
	self->numFrames = numFrames;
}
void AnimSequence_RemoveQuaternionFlips(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
	int i;
	CQuaternion last;
	KeyFrame *frame;

	if(self->numFrames < 2)
		return;

	frame = AnimSequence_GetKeyFrame(self,0);
	last = frame->rotation;
	for(i = 1; i < self->numFrames; i++){
		frame = AnimSequence_GetKeyFrame(self,i);
		// Use the C quaternion operation at the original animation boundary
		if(Quaternion_Dot(last, frame->rotation) < 0.0f)
			// Use the C quaternion operation at the original animation boundary
			frame->rotation = Quaternion_Negate(frame->rotation);
		last = frame->rotation;
	}
}
KeyFrame * AnimSequence_GetKeyFrame(CAnimBlendSequence *self,int32_t n)
{
    // Retain the complete original sequence policy and its observable operation order
return self->type&ANIM_KEYFRAME_TRANS?&((KeyFrameTrans*)self->keyFrames)[n].base:&((KeyFrame*)self->keyFrames)[n];
}
KeyFrameCompressed * AnimSequence_GetKeyFrameCompressed(CAnimBlendSequence *self,int32_t n)
{
    // Retain the complete original sequence policy and its observable operation order
return self->type&ANIM_KEYFRAME_TRANS?&((KeyFrameTransCompressed*)self->keyFramesCompressed)[n].base:&((KeyFrameCompressed*)self->keyFramesCompressed)[n];
}
bool AnimSequence_HasTranslation(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
return !!(self->type&ANIM_KEYFRAME_TRANS);
}
void AnimSequence_Uncompress(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
	int i;

	if(self->numFrames == 0)
		return;

	AnimSequence_BackendPushAnimation();

	float rotScale = 1.0f/4096.0f;
	float timeScale = 1.0f/60.0f;
	float transScale = 1.0f/1024.0f;
	if(self->type & ANIM_KEYFRAME_TRANS){
		void *newKfs = AnimSequence_BackendAllocate(self->numFrames * sizeof(KeyFrameTrans));
		KeyFrameTransCompressed *ckf = (KeyFrameTransCompressed*)self->keyFramesCompressed;
		KeyFrameTrans *kf = (KeyFrameTrans*)newKfs;
		for(i = 0; i < self->numFrames; i++){
			// Use the C keyframe operation and its actual embedded base fields
			kf->base.rotation.x = ckf->base.rot[0]*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			kf->base.rotation.y = ckf->base.rot[1]*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			kf->base.rotation.z = ckf->base.rot[2]*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			kf->base.rotation.w = ckf->base.rot[3]*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			kf->base.deltaTime = ckf->base.deltaTime*timeScale;
			kf->translation.x = ckf->trans[0]*transScale;
			kf->translation.y = ckf->trans[1]*transScale;
			kf->translation.z = ckf->trans[2]*transScale;
			kf++;
			ckf++;
		}
		self->keyFrames = newKfs;
	}else{
		void *newKfs = AnimSequence_BackendAllocate(self->numFrames * sizeof(KeyFrame));
		KeyFrameCompressed *ckf = (KeyFrameCompressed*)self->keyFramesCompressed;
		KeyFrame *kf = (KeyFrame*)newKfs;
		for(i = 0; i < self->numFrames; i++){
			kf->rotation.x = ckf->rot[0]*rotScale;
			kf->rotation.y = ckf->rot[1]*rotScale;
			kf->rotation.z = ckf->rot[2]*rotScale;
			kf->rotation.w = ckf->rot[3]*rotScale;
			kf->deltaTime = ckf->deltaTime*timeScale;
			kf++;
			ckf++;
		}
		self->keyFrames = newKfs;
	}
	AnimSequence_BackendRegister(&self->keyFrames);

	AnimSequence_BackendFree(self->keyFramesCompressed);
	self->keyFramesCompressed = NULL;

	AnimSequence_BackendPopMemory();
}
void AnimSequence_CompressKeyframes(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
	int i;

	if(self->numFrames == 0)
		return;

	AnimSequence_BackendPushAnimation();

	float rotScale = 4096.0f;
	float timeScale = 60.0f;
	float transScale = 1024.0f;
	if(self->type & ANIM_KEYFRAME_TRANS){
		void *newKfs = AnimSequence_BackendAllocate(self->numFrames * sizeof(KeyFrameTransCompressed));
		KeyFrameTransCompressed *ckf = (KeyFrameTransCompressed*)newKfs;
		KeyFrameTrans *kf = (KeyFrameTrans*)self->keyFrames;
		for(i = 0; i < self->numFrames; i++){
			// Use the C keyframe operation and its actual embedded base fields
			ckf->base.rot[0] = kf->base.rotation.x*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			ckf->base.rot[1] = kf->base.rotation.y*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			ckf->base.rot[2] = kf->base.rotation.z*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			ckf->base.rot[3] = kf->base.rotation.w*rotScale;
			// Use the C keyframe operation and its actual embedded base fields
			ckf->base.deltaTime = kf->base.deltaTime*timeScale + 0.5f;
			ckf->trans[0] = kf->translation.x*transScale;
			ckf->trans[1] = kf->translation.y*transScale;
			ckf->trans[2] = kf->translation.z*transScale;
			kf++;
			ckf++;
		}
		self->keyFramesCompressed = newKfs;
	}else{
		void *newKfs = AnimSequence_BackendAllocate(self->numFrames * sizeof(KeyFrameCompressed));
		KeyFrameCompressed *ckf = (KeyFrameCompressed*)newKfs;
		KeyFrame *kf = (KeyFrame*)self->keyFrames;
		for(i = 0; i < self->numFrames; i++){
			ckf->rot[0] = kf->rotation.x*rotScale;
			ckf->rot[1] = kf->rotation.y*rotScale;
			ckf->rot[2] = kf->rotation.z*rotScale;
			ckf->rot[3] = kf->rotation.w*rotScale;
			ckf->deltaTime = kf->deltaTime*timeScale + 0.5f;
			kf++;
			ckf++;
		}
		self->keyFramesCompressed = newKfs;
	}
	AnimSequence_BackendRegister(&self->keyFramesCompressed);

	AnimSequence_BackendPopMemory();
}
void AnimSequence_RemoveUncompressedData(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
	if(self->numFrames == 0)
		return;
	AnimSequence_CompressKeyframes(self);
	AnimSequence_BackendFree(self->keyFrames);
	self->keyFrames = NULL;
}
#ifdef USE_CUSTOM_ALLOCATOR
bool AnimSequence_MoveMemory(CAnimBlendSequence *self)
{
    // Retain the complete original sequence policy and its observable operation order
	if(self->keyFrames){
		void *newaddr = AnimSequence_BackendMoveMemory(self->keyFrames);
		if(newaddr != self->keyFrames){
			self->keyFrames = newaddr;
			return true;
		}
	}else if(self->keyFramesCompressed){
		void *newaddr = AnimSequence_BackendMoveMemory(self->keyFramesCompressed);
		if(newaddr != self->keyFramesCompressed){
			self->keyFramesCompressed = newaddr;
			return true;
		}
	}
	return false;
}
#endif
void AnimSequence_SetBoneTag(CAnimBlendSequence *self,int32_t tag)
{
    // Retain the complete original sequence policy and its observable operation order
self->boneTag=tag;
}
//- rouz edit (ChatGPT)
