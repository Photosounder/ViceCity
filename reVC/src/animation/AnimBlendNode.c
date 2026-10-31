//+ rouz edit (ChatGPT)
#include "AnimBlendNode.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendAssociation_flags.h"
#include <math.h>
#include <stddef.h>
static int32_t Node_Type(CAnimBlendNode *self)
{
    // Read the complete C sequence at the original lazy query boundary
    return self->sequence->type;
}
static int32_t Node_Count(CAnimBlendNode *self)
{
    // Read the complete C sequence at the original lazy query boundary
    return self->sequence->numFrames;
}
static KeyFrame *Node_Frame(CAnimBlendNode *self,int32_t index)
{
    // Read the complete C sequence at the original lazy query boundary
    // Use the complete C sequence at the original native operation boundary
    return AnimSequence_GetKeyFrame(self->sequence,index);
}
static KeyFrameCompressed *Node_PackedFrame(CAnimBlendNode *self,int32_t index)
{
    // Read the complete C sequence at the original lazy query boundary
    // Use the complete C sequence at the original native operation boundary
    return AnimSequence_GetKeyFrameCompressed(self->sequence,index);
}
static bool Node_IsRunning(CAnimBlendNode *self)
{
    // Read the complete C association at the original lazy query boundary
    // Use the complete C association at the original native operation boundary
    return AnimAssociation_IsRunning(self->association);
}
static bool Node_IsRepeating(CAnimBlendNode *self)
{
    // Read the complete C association at the original lazy query boundary
    // Use the complete C association at the original native operation boundary
    return AnimAssociation_IsRepeating(self->association);
}
static float Node_TimeStep(CAnimBlendNode *self)
{
    // Read the complete C association at the original lazy query boundary
    return self->association->timeStep;
}
static float Node_BlendAmount(CAnimBlendNode *self,float weight)
{
    // Read the complete C association at the original lazy query boundary
    // Use the complete C association at the original native operation boundary
    return AnimAssociation_GetBlendAmount(self->association,weight);
}
static KeyFrameVector3 Node_Add(KeyFrameVector3 a,KeyFrameVector3 b)
{
    // Preserve complete native addition temporaries before publishing their coordinates
    const KeyFrameVector3 result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static KeyFrameVector3 Node_Sub(KeyFrameVector3 a,KeyFrameVector3 b)
{
    // Retain the original vector difference and its component arithmetic
    const KeyFrameVector3 result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static KeyFrameVector3 Node_LeftScale(float scale,KeyFrameVector3 value)
{
    // Keep scalar-first multiplication in the interpolation expression
    const KeyFrameVector3 result={scale*value.x,scale*value.y,scale*value.z};return result;
}
static KeyFrameVector3 Node_RightScale(KeyFrameVector3 value,float scale)
{
    // Keep vector-first multiplication in blend application and endpoint queries
    const KeyFrameVector3 result={value.x*scale,value.y*scale,value.z*scale};return result;
}
static KeyFrameVector3 Node_Read(KeyFrameVectorOut value)
{
    // Read current output coordinates at the original in-place scaling boundary
    const KeyFrameVector3 result={*value.x,*value.y,*value.z};return result;
}
static void Node_Write(KeyFrameVectorOut target,KeyFrameVector3 value)
{
    // Publish each complete vector expression immediately in XYZ order
    *target.x=value.x;*target.y=value.y;*target.z=value.z;
}
static KeyFrameVectorOut Node_Output(KeyFrameVector3 *value)
{
    // Address actual C vector fields for packed translation decoding
    const KeyFrameVectorOut result={&value->x,&value->y,&value->z};return result;
}
void AnimBlendNode_Init(CAnimBlendNode *self)
{
    // Retain the complete original animation policy and its lazy field read order

	self->frameA = -1;
	self->frameB = -1;
	self->remainingTime = 0.0f;
	self->sequence = NULL;
	self->association = NULL;
}
bool AnimBlendNode_Update(CAnimBlendNode *self,KeyFrameVectorOut trans,CQuaternion *rot,float weight)
{
    // Retain the complete original animation policy and its lazy field read order

	bool looped = false;

	Node_Write(trans,(KeyFrameVector3){0.0f,0.0f,0.0f});
	// Use the C quaternion operation at the original animation boundary
	*rot = Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);

	if(Node_IsRunning(self)){
		self->remainingTime -= Node_TimeStep(self);
		if(self->remainingTime <= 0.0f)
			looped = AnimBlendNode_NextKeyFrame(self);
	}

	float blend = Node_BlendAmount(self,weight);
	if(blend > 0.0f){
		// Borrow actual common keyframe records before reading optional translation data
		KeyFrame *kfA = Node_Frame(self,self->frameA);
		KeyFrame *kfB = Node_Frame(self,self->frameB);
		float t = kfA->deltaTime == 0.0f ? 0.0f : (kfA->deltaTime - self->remainingTime)/kfA->deltaTime;
		if(Node_Type(self) & ANIM_KEYFRAME_TRANS){
			// Use the C keyframe operation and its actual embedded base fields
			Node_Write(trans,Node_Add(((const KeyFrameTrans*)kfB)->translation,Node_LeftScale(t,Node_Sub(((const KeyFrameTrans*)kfA)->translation,((const KeyFrameTrans*)kfB)->translation))));
			Node_Write(trans,Node_RightScale(Node_Read(trans),blend));
		}
		if(Node_Type(self) & ANIM_KEYFRAME_ROT){
			// Use the C quaternion operation at the original animation boundary
			Quaternion_Slerp(rot,&kfB->rotation,&kfA->rotation,self->theta,self->invSin,t);
			// Use the C quaternion operation at the original animation boundary
			Quaternion_ScaleAssign(rot,blend);
		}
	}

	return looped;
}
bool AnimBlendNode_UpdateCompressed(CAnimBlendNode *self,KeyFrameVectorOut trans,CQuaternion *rot,float weight)
{
    // Retain the complete original animation policy and its lazy field read order

	bool looped = false;

	Node_Write(trans,(KeyFrameVector3){0.0f,0.0f,0.0f});
	// Use the C quaternion operation at the original animation boundary
	*rot = Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);

	if(Node_IsRunning(self)){
		self->remainingTime -= Node_TimeStep(self);
		if(self->remainingTime <= 0.0f)
			looped = AnimBlendNode_NextKeyFrameCompressed(self);
	}

	float blend = Node_BlendAmount(self,weight);
	if(blend > 0.0f){
		// Borrow actual common keyframe records before reading optional translation data
		KeyFrameCompressed *kfA = Node_PackedFrame(self,self->frameA);
		KeyFrameCompressed *kfB = Node_PackedFrame(self,self->frameB);
		// Use the C keyframe operation and its actual embedded base fields
		float t = kfA->deltaTime == 0 ? 0.0f : (KeyFrameCompressed_GetDeltaTime(kfA) - self->remainingTime)/KeyFrameCompressed_GetDeltaTime(kfA);
		if(Node_Type(self) & ANIM_KEYFRAME_TRANS){
			KeyFrameVector3 transA, transB;
			// Use the C keyframe operation and its actual embedded base fields
			KeyFrameTransCompressed_GetTranslation((const KeyFrameTransCompressed*)kfA,Node_Output(&transA));
			// Use the C keyframe operation and its actual embedded base fields
			KeyFrameTransCompressed_GetTranslation((const KeyFrameTransCompressed*)kfB,Node_Output(&transB));
			Node_Write(trans,Node_Add(transB,Node_LeftScale(t,Node_Sub(transA,transB))));
			Node_Write(trans,Node_RightScale(Node_Read(trans),blend));
		}
		if(Node_Type(self) & ANIM_KEYFRAME_ROT){
			CQuaternion rotA, rotB;
			// Use the C keyframe operation and its actual embedded base fields
			KeyFrameCompressed_GetRotation(kfA,&rotA);
			// Use the C keyframe operation and its actual embedded base fields
			KeyFrameCompressed_GetRotation(kfB,&rotB);
			// Use the C quaternion operation at the original animation boundary
			Quaternion_Slerp(rot,&rotB,&rotA,self->theta,self->invSin,t);
			// Use the C quaternion operation at the original animation boundary
			Quaternion_ScaleAssign(rot,blend);
		}
	}

	return looped;
}
bool AnimBlendNode_NextKeyFrame(CAnimBlendNode *self)
{
    // Retain the complete original animation policy and its lazy field read order

	bool looped;

	if(Node_Count(self) <= 1)
		return false;

	looped = false;
	self->frameB = self->frameA;

	// Advance as long as we have to
	while(self->remainingTime <= 0.0f){
		self->frameA++;

		if(self->frameA >= Node_Count(self)){
			// reached end of animation
			if(!Node_IsRepeating(self)){
				self->frameA--;
				self->remainingTime = 0.0f;
				return false;
			}
			looped = true;
			self->frameA = 0;
		}

		self->remainingTime += Node_Frame(self,self->frameA)->deltaTime;
	}

	self->frameB = self->frameA - 1;
	if(self->frameB < 0)
		self->frameB += Node_Count(self);

	AnimBlendNode_CalcDeltas(self);
	return looped;
}
bool AnimBlendNode_NextKeyFrameCompressed(CAnimBlendNode *self)
{
    // Retain the complete original animation policy and its lazy field read order

	bool looped;

	if(Node_Count(self) <= 1)
		return false;

	looped = false;
	self->frameB = self->frameA;

	// Advance as long as we have to
	while(self->remainingTime <= 0.0f){
		self->frameA++;

		if(self->frameA >= Node_Count(self)){
			// reached end of animation
			if(!Node_IsRepeating(self)){
				self->frameA--;
				self->remainingTime = 0.0f;
				return false;
			}
			looped = true;
			self->frameA = 0;
		}

		// Use the C keyframe operation and its actual embedded base fields
		self->remainingTime += KeyFrameCompressed_GetDeltaTime(Node_PackedFrame(self,self->frameA));
	}

	self->frameB = self->frameA - 1;
	if(self->frameB < 0)
		self->frameB += Node_Count(self);

	AnimBlendNode_CalcDeltasCompressed(self);
	return looped;
}
bool AnimBlendNode_FindKeyFrame(CAnimBlendNode *self,float t)
{
    // Retain the complete original animation policy and its lazy field read order

	if(Node_Count(self) < 1)
		return false;

	self->frameA = 0;
	self->frameB = self->frameA;

	if(Node_Count(self) == 1){
		self->remainingTime = 0.0f;
	}else{
		// advance until t is between self->frameB and self->frameA
		while (t > Node_Frame(self,++self->frameA)->deltaTime) {
			t -= Node_Frame(self,self->frameA)->deltaTime;
			if (self->frameA + 1 >= Node_Count(self)) {
				// reached end of animation
				if (!Node_IsRepeating(self)) {
					AnimBlendNode_CalcDeltas(self);
					self->remainingTime = 0.0f;
					return false;
				}
				self->frameA = 0;
			}
			self->frameB = self->frameA;
		}

		self->remainingTime = Node_Frame(self,self->frameA)->deltaTime - t;
	}

	AnimBlendNode_CalcDeltas(self);
	return true;
}
bool AnimBlendNode_SetupKeyFrameCompressed(CAnimBlendNode *self)
{
    // Retain the complete original animation policy and its lazy field read order

	if(Node_Count(self) < 1)
		return false;

	self->frameA = 1;
	self->frameB = 0;

	if(Node_Count(self) == 1){
		self->frameA = 0;
		self->remainingTime = 0.0f;
	}else
		// Use the C keyframe operation and its actual embedded base fields
		self->remainingTime = KeyFrameCompressed_GetDeltaTime(Node_PackedFrame(self,self->frameA));

	AnimBlendNode_CalcDeltasCompressed(self);
	return true;
}
void AnimBlendNode_CalcDeltas(CAnimBlendNode *self)
{
    // Retain the complete original animation policy and its lazy field read order

	if((Node_Type(self) & ANIM_KEYFRAME_ROT) == 0)
		return;
	KeyFrame *kfA = Node_Frame(self,self->frameA);
	KeyFrame *kfB = Node_Frame(self,self->frameB);
	// Use the C quaternion operation at the original animation boundary
	float cos = Quaternion_Dot(kfA->rotation, kfB->rotation);
	if(cos > 1.0f)
		cos = 1.0f;
	self->theta = acosf(cos);
	self->invSin = self->theta == 0.0f ?  0.0f : 1.0f/sinf(self->theta);
}
void AnimBlendNode_CalcDeltasCompressed(CAnimBlendNode *self)
{
    // Retain the complete original animation policy and its lazy field read order

	if((Node_Type(self) & ANIM_KEYFRAME_ROT) == 0)
		return;
	KeyFrameCompressed *kfA = Node_PackedFrame(self,self->frameA);
	KeyFrameCompressed *kfB = Node_PackedFrame(self,self->frameB);
	CQuaternion rotA, rotB;
	// Use the C keyframe operation and its actual embedded base fields
	KeyFrameCompressed_GetRotation(kfA,&rotA);
	// Use the C keyframe operation and its actual embedded base fields
	KeyFrameCompressed_GetRotation(kfB,&rotB);
	// Use the C quaternion operation at the original animation boundary
	float cos = Quaternion_Dot(rotA, rotB);
	if(cos < 0.0f){
		// Use the C quaternion operation at the original animation boundary
		Quaternion_ScaleAssign(&rotB,-1.0f);
		// Use the C keyframe operation and its actual embedded base fields
		KeyFrameCompressed_SetRotation(kfB,&rotB);
	}
	// Use the C quaternion operation at the original animation boundary
	cos = Quaternion_Dot(rotA, rotB);
	if(cos > 1.0f)
		cos = 1.0f;
	self->theta = acosf(cos);
	self->invSin = self->theta == 0.0f ?  0.0f : 1.0f/sinf(self->theta);
}
void AnimBlendNode_GetCurrentTranslation(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight)
{
    // Retain the complete original animation policy and its lazy field read order

	Node_Write(trans,(KeyFrameVector3){0.0f,0.0f,0.0f});

	float blend = Node_BlendAmount(self,weight);
	if(blend > 0.0f){
		// Borrow actual common keyframe records before reading optional translation data
		KeyFrame *kfA = Node_Frame(self,self->frameA);
		KeyFrame *kfB = Node_Frame(self,self->frameB);
		float t = kfA->deltaTime == 0.0f ? 0.0f : (kfA->deltaTime - self->remainingTime)/kfA->deltaTime;
		if(Node_Type(self) & ANIM_KEYFRAME_TRANS){
			// Use the C keyframe operation and its actual embedded base fields
			Node_Write(trans,Node_Add(((const KeyFrameTrans*)kfB)->translation,Node_LeftScale(t,Node_Sub(((const KeyFrameTrans*)kfA)->translation,((const KeyFrameTrans*)kfB)->translation))));
			Node_Write(trans,Node_RightScale(Node_Read(trans),blend));
		}
	}
}
void AnimBlendNode_GetCurrentTranslationCompressed(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight)
{
    // Retain the complete original animation policy and its lazy field read order

	Node_Write(trans,(KeyFrameVector3){0.0f,0.0f,0.0f});

	float blend = Node_BlendAmount(self,weight);
	if(blend > 0.0f){
		// Borrow actual common keyframe records before reading optional translation data
		KeyFrameCompressed *kfA = Node_PackedFrame(self,self->frameA);
		KeyFrameCompressed *kfB = Node_PackedFrame(self,self->frameB);
		// Use the C keyframe operation and its actual embedded base fields
		float t = kfA->deltaTime == 0 ? 0.0f : (KeyFrameCompressed_GetDeltaTime(kfA) - self->remainingTime)/KeyFrameCompressed_GetDeltaTime(kfA);
		if(Node_Type(self) & ANIM_KEYFRAME_TRANS){
			KeyFrameVector3 transA, transB;
			// Use the C keyframe operation and its actual embedded base fields
			KeyFrameTransCompressed_GetTranslation((const KeyFrameTransCompressed*)kfA,Node_Output(&transA));
			// Use the C keyframe operation and its actual embedded base fields
			KeyFrameTransCompressed_GetTranslation((const KeyFrameTransCompressed*)kfB,Node_Output(&transB));
			Node_Write(trans,Node_Add(transB,Node_LeftScale(t,Node_Sub(transA,transB))));
			Node_Write(trans,Node_RightScale(Node_Read(trans),blend));
		}
	}
}
void AnimBlendNode_GetEndTranslation(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight)
{
    // Retain the complete original animation policy and its lazy field read order

	Node_Write(trans,(KeyFrameVector3){0.0f,0.0f,0.0f});

	float blend = Node_BlendAmount(self,weight);
	if(blend > 0.0f){
		KeyFrameTrans *kf = (KeyFrameTrans*)Node_Frame(self,Node_Count(self)-1);
		if(Node_Type(self) & ANIM_KEYFRAME_TRANS)
			// Use the C keyframe operation and its actual embedded base fields
			Node_Write(trans,Node_RightScale(kf->translation,blend));
	}
}
void AnimBlendNode_GetEndTranslationCompressed(CAnimBlendNode *self,KeyFrameVectorOut trans,float weight)
{
    // Retain the complete original animation policy and its lazy field read order

	Node_Write(trans,(KeyFrameVector3){0.0f,0.0f,0.0f});

	float blend = Node_BlendAmount(self,weight);
	if(blend > 0.0f){
		KeyFrameTransCompressed *kf = (KeyFrameTransCompressed*)Node_PackedFrame(self,Node_Count(self)-1);
		if(Node_Type(self) & ANIM_KEYFRAME_TRANS){
			KeyFrameVector3 pos;
			// Use the C keyframe operation and its actual embedded base fields
			KeyFrameTransCompressed_GetTranslation(kf,Node_Output(&pos));
			Node_Write(trans,Node_RightScale(pos,blend));
		}
	}
}
//- rouz edit (ChatGPT)
