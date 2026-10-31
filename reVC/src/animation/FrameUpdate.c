//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendFrameUpdate.h"
#include "FrameUpdate_backend.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation.h"
CAnimBlendClumpData *gpAnimBlendClump;

static KeyFrameVectorOut FrameUpdate_VectorOut(KeyFrameVector3 *value)
{
    // Borrow actual local C vector components for the existing interpolation functions
    KeyFrameVectorOut out={&value->x,&value->y,&value->z};return out;
}
static void FrameUpdate_AddOut(KeyFrameVectorOut destination,const KeyFrameVector3 *value)
{
    // Retain original native vector addition order and live component reads
    *destination.x+=value->x;*destination.y+=value->y;*destination.z+=value->z;
}
static void FrameUpdate_Add(KeyFrameVector3 *destination,const KeyFrameVector3 *value)
{
    // Apply complete local vector addition without C++ operators
    FrameUpdate_AddOut(FrameUpdate_VectorOut(destination),value);
}
static KeyFrameVector3 FrameUpdate_Subtract(const KeyFrameVector3 *left,const KeyFrameVector3 *right)
{
    // Preserve each original vector subtraction before selecting or storing components
    KeyFrameVector3 value={left->x-right->x,left->y-right->y,left->z-right->z};return value;
}
static void FrameUpdate_Store(KeyFrameVectorOut destination,KeyFrameVector3 value)
{
    // Assign all actual destination components in the original native vector field order
    *destination.x=value.x;*destination.y=value.y;*destination.z=value.z;
}
void FrameUpdateCallBackNonSkinned(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	// Use the C animation frame while retaining the original live SDK objects
	void *mat = FrameUpdate_HostFrameMatrix(frame);
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	// Use the C animation frame while retaining the original live SDK objects
	if(frame->flag & ANIM_FRAME_VELOCITY_EXTRACTION &&
	   gpAnimBlendClump->velocity2d){
		// Use the C animation frame while retaining the original live SDK objects
		if(frame->flag & ANIM_FRAME_VELOCITY_EXTRACTION_3D)
			FrameUpdateCallBackWith3dVelocityExtractionNonSkinned(frame, arg);
		else
			FrameUpdateCallBackWithVelocityExtractionNonSkinned(frame, arg);
		return;
	}

	if(updateData->foobar)
		for(node = updateData->nodes; *node; node++)
			// Use the complete C association at the original native operation boundary
			if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
				totalBlendAmount += (*node)->association->blendAmount;

	for(node = updateData->nodes; *node; node++){
		if((*node)->sequence){
			// Use the C blend-node policy at the original native call boundary
			AnimBlendNode_Update((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
			// Use the complete C sequence at the original native operation boundary
			if(AnimSequence_HasTranslation((*node)->sequence))
				FrameUpdate_Add(&pos,&vec);
#ifdef FIX_BUGS
			// Use the C quaternion operation at the original animation boundary
			if(Quaternion_Dot(rot, q) < 0.0f)
				// Use the C quaternion operation at the original animation boundary
				Quaternion_SubAssign(&rot,&q);
			else
#endif
				// Use the C quaternion operation at the original animation boundary
				Quaternion_AddAssign(&rot,&q);
		}
		++*node;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
		FrameUpdate_HostMatrixIdentity(mat);
		// Use the C quaternion operation at the original animation boundary
		Quaternion_Normalise(&rot);
		// Use the C quaternion operation at the original animation boundary
		Quaternion_GetMatrix(&rot,FrameUpdate_HostMatrixRotation(mat));
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
		(*FrameUpdate_HostMatrixPosition(mat).x) = pos.x;
		(*FrameUpdate_HostMatrixPosition(mat).y) = pos.y;
		(*FrameUpdate_HostMatrixPosition(mat).z) = pos.z;
		(*FrameUpdate_HostMatrixPosition(mat).x) += frame->resetPos.x;
		(*FrameUpdate_HostMatrixPosition(mat).y) += frame->resetPos.y;
		(*FrameUpdate_HostMatrixPosition(mat).z) += frame->resetPos.z;
	}
	FrameUpdate_HostMatrixUpdate(mat);
}
void FrameUpdateCallBackWithVelocityExtractionNonSkinned(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	float transx = 0.0f, transy = 0.0f;
	float curx = 0.0f, cury = 0.0f;
	float endx = 0.0f, endy = 0.0f;
	bool looped = false;
	// Use the C animation frame while retaining the original live SDK objects
	void *mat = FrameUpdate_HostFrameMatrix(frame);
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	if(updateData->foobar)
		for(node = updateData->nodes; *node; node++)
			// Use the complete C association at the original native operation boundary
			if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
				totalBlendAmount += (*node)->association->blendAmount;

	for(node = updateData->nodes; *node; node++)
		// Use the complete C sequence at the original native operation boundary
		if((*node)->sequence && AnimSequence_HasTranslation((*node)->sequence)){
			// Use the complete C association at the original native operation boundary
			if(AnimAssociation_HasTranslation((*node)->association)){
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_GetCurrentTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
				cury += vec.y;
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasXTranslation((*node)->association))
					curx += vec.x;
			}
		}

	for(node = updateData->nodes; *node; node++){
		if((*node)->sequence){
			// Use the C blend-node policy at the original native call boundary
			bool nodelooped = AnimBlendNode_Update((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
#ifdef FIX_BUGS
			// Use the C quaternion operation at the original animation boundary
			if(Quaternion_Dot(rot, q) < 0.0f)
				// Use the C quaternion operation at the original animation boundary
				Quaternion_SubAssign(&rot,&q);
			else
#endif
				// Use the C quaternion operation at the original animation boundary
				Quaternion_AddAssign(&rot,&q);
			// Use the complete C sequence at the original native operation boundary
			if(AnimSequence_HasTranslation((*node)->sequence)){
				FrameUpdate_Add(&pos,&vec);
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasTranslation((*node)->association)){
					transy += vec.y;
					// Use the complete C association at the original native operation boundary
					if(AnimAssociation_HasXTranslation((*node)->association))
						transx += vec.x;
					looped |= nodelooped;
					if(nodelooped){
						// Use the C blend-node policy at the original native call boundary
						AnimBlendNode_GetEndTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
						endy += vec.y;
						// Use the complete C association at the original native operation boundary
						if(AnimAssociation_HasXTranslation((*node)->association))
							endx += vec.x;
					}
				}
			}
		}
		++*node;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
		FrameUpdate_HostMatrixIdentity(mat);
		// Use the C quaternion operation at the original animation boundary
		Quaternion_Normalise(&rot);
		// Use the C quaternion operation at the original animation boundary
		Quaternion_GetMatrix(&rot,FrameUpdate_HostMatrixRotation(mat));
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
		(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).x) = transx - curx;
		(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).y) = transy - cury;
		if(looped){
			(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).x) += endx;
			(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).y) += endy;
		}
		(*FrameUpdate_HostMatrixPosition(mat).x) = pos.x - transx;
		(*FrameUpdate_HostMatrixPosition(mat).y) = pos.y - transy;
		(*FrameUpdate_HostMatrixPosition(mat).z) = pos.z;
		if((*FrameUpdate_HostMatrixPosition(mat).z) >= -0.8f) {
			if((*FrameUpdate_HostMatrixPosition(mat).z) < -0.4f)
				(*FrameUpdate_HostMatrixPosition(mat).z) += (2.5f * (*FrameUpdate_HostMatrixPosition(mat).z) + 2.0f) * frame->resetPos.z;
			else
				(*FrameUpdate_HostMatrixPosition(mat).z) += frame->resetPos.z;
		}
		(*FrameUpdate_HostMatrixPosition(mat).x) += frame->resetPos.x;
		(*FrameUpdate_HostMatrixPosition(mat).y) += frame->resetPos.y;
	}
	FrameUpdate_HostMatrixUpdate(mat);
}
void FrameUpdateCallBackWith3dVelocityExtractionNonSkinned(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	KeyFrameVector3 trans={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 cur={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 end={0.0f, 0.0f, 0.0f};
	bool looped = false;
	// Use the C animation frame while retaining the original live SDK objects
	void *mat = FrameUpdate_HostFrameMatrix(frame);
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	if(updateData->foobar)
		for(node = updateData->nodes; *node; node++)
			// Use the complete C association at the original native operation boundary
			if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
				totalBlendAmount += (*node)->association->blendAmount;

	for(node = updateData->nodes; *node; node++)
		// Use the complete C sequence at the original native operation boundary
		if((*node)->sequence && AnimSequence_HasTranslation((*node)->sequence)){
			// Use the complete C association at the original native operation boundary
			if(AnimAssociation_HasTranslation((*node)->association)){
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_GetCurrentTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
				FrameUpdate_Add(&cur,&vec);
			}
		}

	for(node = updateData->nodes; *node; node++){
		if((*node)->sequence){
			// Use the C blend-node policy at the original native call boundary
			bool nodelooped = AnimBlendNode_Update((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
#ifdef FIX_BUGS
			// Use the C quaternion operation at the original animation boundary
			if(Quaternion_Dot(rot, q) < 0.0f)
				// Use the C quaternion operation at the original animation boundary
				Quaternion_SubAssign(&rot,&q);
			else
#endif
				// Use the C quaternion operation at the original animation boundary
				Quaternion_AddAssign(&rot,&q);
			// Use the complete C sequence at the original native operation boundary
			if(AnimSequence_HasTranslation((*node)->sequence)){
				FrameUpdate_Add(&pos,&vec);
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasTranslation((*node)->association)){
					FrameUpdate_Add(&trans,&vec);
					looped |= nodelooped;
					if(nodelooped){
						// Use the C blend-node policy at the original native call boundary
						AnimBlendNode_GetEndTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
						FrameUpdate_Add(&end,&vec);
					}
				}
			}
		}
		++*node;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
		FrameUpdate_HostMatrixIdentity(mat);
		// Use the C quaternion operation at the original animation boundary
		Quaternion_Normalise(&rot);
		// Use the C quaternion operation at the original animation boundary
		Quaternion_GetMatrix(&rot,FrameUpdate_HostMatrixRotation(mat));
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
		FrameUpdate_Store(FrameUpdate_HostVelocity3d(gpAnimBlendClump),FrameUpdate_Subtract(&trans,&cur));
		if(looped)
			FrameUpdate_AddOut(FrameUpdate_HostVelocity3d(gpAnimBlendClump),&end);
		(*FrameUpdate_HostMatrixPosition(mat).x) = FrameUpdate_Subtract(&pos,&trans).x + frame->resetPos.x;
		(*FrameUpdate_HostMatrixPosition(mat).y) = FrameUpdate_Subtract(&pos,&trans).y + frame->resetPos.y;
		(*FrameUpdate_HostMatrixPosition(mat).z) = FrameUpdate_Subtract(&pos,&trans).z + frame->resetPos.z;
	}
	FrameUpdate_HostMatrixUpdate(mat);
}
void FrameUpdateCallBackSkinned(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	float transBlendAmount = 0.0f;
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	// Use the C animation frame while retaining the original live SDK objects
	void *xform = frame->hanimFrame;
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	// Use the C animation frame while retaining the original live SDK objects
	if(frame->flag & ANIM_FRAME_VELOCITY_EXTRACTION &&
	   gpAnimBlendClump->velocity2d){
		// Use the C animation frame while retaining the original live SDK objects
		if(frame->flag & ANIM_FRAME_VELOCITY_EXTRACTION_3D)
			FrameUpdateCallBackWith3dVelocityExtractionSkinned(frame, arg);
		else
			FrameUpdateCallBackWithVelocityExtractionSkinned(frame, arg);
		return;
	}

	if(updateData->foobar)
		for(node = updateData->nodes; *node; node++)
			// Use the complete C association at the original native operation boundary
			if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
				totalBlendAmount += (*node)->association->blendAmount;

	for(node = updateData->nodes; *node; node++){
		if((*node)->sequence){
			// Use the C blend-node policy at the original native call boundary
			AnimBlendNode_Update((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
			// Use the complete C sequence at the original native operation boundary
			if(AnimSequence_HasTranslation((*node)->sequence)){
				FrameUpdate_Add(&pos,&vec);
				transBlendAmount += (*node)->association->blendAmount;
			}
			// Use the C quaternion operation at the original animation boundary
			if(Quaternion_Dot(rot, q) < 0.0f)
				// Use the C quaternion operation at the original animation boundary
				Quaternion_SubAssign(&rot,&q);
			else
				// Use the C quaternion operation at the original animation boundary
				Quaternion_AddAssign(&rot,&q);
		}
		++*node;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
		// Use the C quaternion operation at the original animation boundary
		Quaternion_Normalise(&rot);
		(*FrameUpdate_HostPoseRotation(xform).x) = rot.x;
		(*FrameUpdate_HostPoseRotation(xform).y) = rot.y;
		(*FrameUpdate_HostPoseRotation(xform).z) = rot.z;
		(*FrameUpdate_HostPoseRotation(xform).w) = rot.w;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
		(*FrameUpdate_HostPoseTranslation(xform).x) = transBlendAmount*pos.x;
		(*FrameUpdate_HostPoseTranslation(xform).y) = transBlendAmount*pos.y;
		(*FrameUpdate_HostPoseTranslation(xform).z) = transBlendAmount*pos.z;
		(*FrameUpdate_HostPoseTranslation(xform).x) += (1.0f-transBlendAmount)*frame->resetPos.x;
		(*FrameUpdate_HostPoseTranslation(xform).y) += (1.0f-transBlendAmount)*frame->resetPos.y;
		(*FrameUpdate_HostPoseTranslation(xform).z) += (1.0f-transBlendAmount)*frame->resetPos.z;
	}
}
void FrameUpdateCallBackWithVelocityExtractionSkinned(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	float transx = 0.0f, transy = 0.0f;
	float curx = 0.0f, cury = 0.0f;
	float endx = 0.0f, endy = 0.0f;
	bool looped = false;
	// Use the C animation frame while retaining the original live SDK objects
	void *xform = frame->hanimFrame;
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	if(updateData->foobar)
		for(node = updateData->nodes; *node; node++)
			// Use the complete C association at the original native operation boundary
			if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
				totalBlendAmount += (*node)->association->blendAmount;

	for(node = updateData->nodes; *node; node++)
		// Use the complete C sequence at the original native operation boundary
		if((*node)->sequence && AnimSequence_HasTranslation((*node)->sequence)){
			// Use the complete C association at the original native operation boundary
			if(AnimAssociation_HasTranslation((*node)->association)){
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_GetCurrentTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
				cury += vec.y;
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasXTranslation((*node)->association))
					curx += vec.x;
			}
		}

	for(node = updateData->nodes; *node; node++){
		if((*node)->sequence){
			// Use the C blend-node policy at the original native call boundary
			bool nodelooped = AnimBlendNode_Update((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
			// Use the C quaternion operation at the original animation boundary
			if(Quaternion_Dot(rot, q) < 0.0f)
				// Use the C quaternion operation at the original animation boundary
				Quaternion_SubAssign(&rot,&q);
			else
				// Use the C quaternion operation at the original animation boundary
				Quaternion_AddAssign(&rot,&q);
			// Use the complete C sequence at the original native operation boundary
			if(AnimSequence_HasTranslation((*node)->sequence)){
				FrameUpdate_Add(&pos,&vec);
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasTranslation((*node)->association)){
					transy += vec.y;
					// Use the complete C association at the original native operation boundary
					if(AnimAssociation_HasXTranslation((*node)->association))
						transx += vec.x;
					looped |= nodelooped;
					if(nodelooped){
						// Use the C blend-node policy at the original native call boundary
						AnimBlendNode_GetEndTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
						endy += vec.y;
						// Use the complete C association at the original native operation boundary
						if(AnimAssociation_HasXTranslation((*node)->association))
							endx += vec.x;
					}
				}
			}
		}
		++*node;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
		// Use the C quaternion operation at the original animation boundary
		Quaternion_Normalise(&rot);
		(*FrameUpdate_HostPoseRotation(xform).x) = rot.x;
		(*FrameUpdate_HostPoseRotation(xform).y) = rot.y;
		(*FrameUpdate_HostPoseRotation(xform).z) = rot.z;
		(*FrameUpdate_HostPoseRotation(xform).w) = rot.w;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
		(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).x) = transx - curx;
		(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).y) = transy - cury;
		if(looped){
			(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).x) += endx;
			(*FrameUpdate_HostVelocity2d(gpAnimBlendClump).y) += endy;
		}
		(*FrameUpdate_HostPoseTranslation(xform).x) = pos.x - transx;
		(*FrameUpdate_HostPoseTranslation(xform).y) = pos.y - transy;
		(*FrameUpdate_HostPoseTranslation(xform).z) = pos.z;
		if((*FrameUpdate_HostPoseTranslation(xform).z) >= -0.8f) {
			if((*FrameUpdate_HostPoseTranslation(xform).z) < -0.4f)
				(*FrameUpdate_HostPoseTranslation(xform).z) += (2.5f * (*FrameUpdate_HostPoseTranslation(xform).z) + 2.0f) * frame->resetPos.z;
			else
				(*FrameUpdate_HostPoseTranslation(xform).z) += frame->resetPos.z;
		}
		(*FrameUpdate_HostPoseTranslation(xform).x) += frame->resetPos.x;
		(*FrameUpdate_HostPoseTranslation(xform).y) += frame->resetPos.y;
	}
}
void FrameUpdateCallBackWith3dVelocityExtractionSkinned(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	KeyFrameVector3 trans={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 cur={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 end={0.0f, 0.0f, 0.0f};
	bool looped = false;
	// Use the C animation frame while retaining the original live SDK objects
	void *xform = frame->hanimFrame;
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	if(updateData->foobar)
		for(node = updateData->nodes; *node; node++)
			// Use the complete C association at the original native operation boundary
			if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
				totalBlendAmount += (*node)->association->blendAmount;

	for(node = updateData->nodes; *node; node++)
		// Use the complete C sequence at the original native operation boundary
		if((*node)->sequence && AnimSequence_HasTranslation((*node)->sequence)){
			// Use the complete C association at the original native operation boundary
			if(AnimAssociation_HasTranslation((*node)->association)){
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_GetCurrentTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
				FrameUpdate_Add(&cur,&vec);
			}
		}

	for(node = updateData->nodes; *node; node++){
		if((*node)->sequence){
			// Use the C blend-node policy at the original native call boundary
			bool nodelooped = AnimBlendNode_Update((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
#ifdef FIX_BUGS
			// Use the C quaternion operation at the original animation boundary
			if(Quaternion_Dot(rot, q) < 0.0f)
				// Use the C quaternion operation at the original animation boundary
				Quaternion_SubAssign(&rot,&q);
			else
#endif
				// Use the C quaternion operation at the original animation boundary
				Quaternion_AddAssign(&rot,&q);
			// Use the complete C sequence at the original native operation boundary
			if(AnimSequence_HasTranslation((*node)->sequence)){
				FrameUpdate_Add(&pos,&vec);
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasTranslation((*node)->association)){
					FrameUpdate_Add(&trans,&vec);
					looped |= nodelooped;
					if(nodelooped){
						// Use the C blend-node policy at the original native call boundary
						AnimBlendNode_GetEndTranslation((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
						FrameUpdate_Add(&end,&vec);
					}
				}
			}
		}
		++*node;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
		// Use the C quaternion operation at the original animation boundary
		Quaternion_Normalise(&rot);
		(*FrameUpdate_HostPoseRotation(xform).x) = rot.x;
		(*FrameUpdate_HostPoseRotation(xform).y) = rot.y;
		(*FrameUpdate_HostPoseRotation(xform).z) = rot.z;
		(*FrameUpdate_HostPoseRotation(xform).w) = rot.w;
	}

	// Use the C animation frame while retaining the original live SDK objects
	if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
		FrameUpdate_Store(FrameUpdate_HostVelocity3d(gpAnimBlendClump),FrameUpdate_Subtract(&trans,&cur));
		if(looped)
			FrameUpdate_AddOut(FrameUpdate_HostVelocity3d(gpAnimBlendClump),&end);
		(*FrameUpdate_HostPoseTranslation(xform).x) = FrameUpdate_Subtract(&pos,&trans).x + frame->resetPos.x;
		(*FrameUpdate_HostPoseTranslation(xform).y) = FrameUpdate_Subtract(&pos,&trans).y + frame->resetPos.y;
		(*FrameUpdate_HostPoseTranslation(xform).z) = FrameUpdate_Subtract(&pos,&trans).z + frame->resetPos.z;
	}
}
void FrameUpdateCallBackOffscreen(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	// Use the C animation frame while retaining the original live SDK objects
	if(frame->flag & ANIM_FRAME_VELOCITY_EXTRACTION && gpAnimBlendClump->velocity2d)
		FrameUpdateCallBackWithVelocityExtractionSkinned(frame, arg);
}
void FrameUpdateCallBackNonSkinnedCompressed(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	KeyFrameVector3 trans={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 cur={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 end={0.0f, 0.0f, 0.0f};
	bool looped = false;
	// Use the C animation frame while retaining the original live SDK objects
	void *mat = FrameUpdate_HostFrameMatrix(frame);
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	// Use the C animation frame while retaining the original live SDK objects
	if(frame->flag & ANIM_FRAME_VELOCITY_EXTRACTION &&
	   gpAnimBlendClump->velocity2d){
		if(updateData->foobar)
			for(node = updateData->nodes; *node; node++)
				// Use the complete C association at the original native operation boundary
				if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
					totalBlendAmount += (*node)->association->blendAmount;

		for(node = updateData->nodes; *node; node++)
			// Use the complete C sequence at the original native operation boundary
			if((*node)->sequence && AnimSequence_HasTranslation((*node)->sequence)){
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasTranslation((*node)->association)){
					// Use the C blend-node policy at the original native call boundary
					AnimBlendNode_GetCurrentTranslationCompressed((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
					FrameUpdate_Add(&cur,&vec);
				}
			}

		for(node = updateData->nodes; *node; node++){
			if((*node)->sequence){
				// Use the C blend-node policy at the original native call boundary
				bool nodelooped = AnimBlendNode_UpdateCompressed((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
#ifdef FIX_BUGS
				// Use the C quaternion operation at the original animation boundary
				if(Quaternion_Dot(rot, q) < 0.0f)
					// Use the C quaternion operation at the original animation boundary
					Quaternion_SubAssign(&rot,&q);
				else
#endif
					// Use the C quaternion operation at the original animation boundary
					Quaternion_AddAssign(&rot,&q);
				// Use the complete C sequence at the original native operation boundary
				if(AnimSequence_HasTranslation((*node)->sequence)){
					FrameUpdate_Add(&pos,&vec);
					// Use the complete C association at the original native operation boundary
					if(AnimAssociation_HasTranslation((*node)->association)){
						FrameUpdate_Add(&trans,&vec);
						looped |= nodelooped;
						if(nodelooped){
							// Use the C blend-node policy at the original native call boundary
							AnimBlendNode_GetEndTranslationCompressed((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
							FrameUpdate_Add(&end,&vec);
						}
					}
				}
			}
			++*node;
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
			FrameUpdate_HostMatrixIdentity(mat);
			// Use the C quaternion operation at the original animation boundary
			Quaternion_Normalise(&rot);
			// Use the C quaternion operation at the original animation boundary
			Quaternion_GetMatrix(&rot,FrameUpdate_HostMatrixRotation(mat));
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
			FrameUpdate_Store(FrameUpdate_HostVelocity3d(gpAnimBlendClump),FrameUpdate_Subtract(&trans,&cur));
			if(looped)
				FrameUpdate_AddOut(FrameUpdate_HostVelocity3d(gpAnimBlendClump),&end);
			(*FrameUpdate_HostMatrixPosition(mat).x) = FrameUpdate_Subtract(&pos,&trans).x + frame->resetPos.x;
			(*FrameUpdate_HostMatrixPosition(mat).y) = FrameUpdate_Subtract(&pos,&trans).y + frame->resetPos.y;
			(*FrameUpdate_HostMatrixPosition(mat).z) = FrameUpdate_Subtract(&pos,&trans).z + frame->resetPos.z;
		}
		FrameUpdate_HostMatrixUpdate(mat);
	}else{
		if(updateData->foobar)
			for(node = updateData->nodes; *node; node++)
				// Use the complete C association at the original native operation boundary
				if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
					totalBlendAmount += (*node)->association->blendAmount;

		for(node = updateData->nodes; *node; node++){
			if((*node)->sequence){
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_UpdateCompressed((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
				// Use the complete C sequence at the original native operation boundary
				if(AnimSequence_HasTranslation((*node)->sequence))
					FrameUpdate_Add(&pos,&vec);
#ifdef FIX_BUGS
				// Use the C quaternion operation at the original animation boundary
				if(Quaternion_Dot(rot, q) < 0.0f)
					// Use the C quaternion operation at the original animation boundary
					Quaternion_SubAssign(&rot,&q);
				else
#endif
					// Use the C quaternion operation at the original animation boundary
					Quaternion_AddAssign(&rot,&q);
			}
			++*node;
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
			FrameUpdate_HostMatrixIdentity(mat);
			// Use the C quaternion operation at the original animation boundary
			Quaternion_Normalise(&rot);
			// Use the C quaternion operation at the original animation boundary
			Quaternion_GetMatrix(&rot,FrameUpdate_HostMatrixRotation(mat));
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
			(*FrameUpdate_HostMatrixPosition(mat).x) = pos.x;
			(*FrameUpdate_HostMatrixPosition(mat).y) = pos.y;
			(*FrameUpdate_HostMatrixPosition(mat).z) = pos.z;
			(*FrameUpdate_HostMatrixPosition(mat).x) += frame->resetPos.x;
			(*FrameUpdate_HostMatrixPosition(mat).y) += frame->resetPos.y;
			(*FrameUpdate_HostMatrixPosition(mat).z) += frame->resetPos.z;
		}
		FrameUpdate_HostMatrixUpdate(mat);
	}
}
void FrameUpdateCallBackSkinnedCompressed(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the full original frame accumulation policy and live field access order

	KeyFrameVector3 vec, pos={0.0f, 0.0f, 0.0f};
	// Use the C quaternion operation at the original animation boundary
	CQuaternion q, rot=Quaternion_Make(0.0f, 0.0f, 0.0f, 0.0f);
	float totalBlendAmount = 0.0f;
	KeyFrameVector3 trans={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 cur={0.0f, 0.0f, 0.0f};
	KeyFrameVector3 end={0.0f, 0.0f, 0.0f};
	bool looped = false;
	// Use the C animation frame while retaining the original live SDK objects
	void *xform = frame->hanimFrame;
	CAnimBlendNode **node;
	AnimBlendFrameUpdateData *updateData = (AnimBlendFrameUpdateData*)arg;

	// Use the C animation frame while retaining the original live SDK objects
	if(frame->flag & ANIM_FRAME_VELOCITY_EXTRACTION &&
	   gpAnimBlendClump->velocity2d){
		if(updateData->foobar)
			for(node = updateData->nodes; *node; node++)
				// Use the complete C association at the original native operation boundary
				if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
					totalBlendAmount += (*node)->association->blendAmount;

		for(node = updateData->nodes; *node; node++)
			// Use the complete C sequence at the original native operation boundary
			if((*node)->sequence && AnimSequence_HasTranslation((*node)->sequence)){
				// Use the complete C association at the original native operation boundary
				if(AnimAssociation_HasTranslation((*node)->association)){
					// Use the C blend-node policy at the original native call boundary
					AnimBlendNode_GetCurrentTranslationCompressed((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
					FrameUpdate_Add(&cur,&vec);
				}
			}

		for(node = updateData->nodes; *node; node++){
			if((*node)->sequence){
				// Use the C blend-node policy at the original native call boundary
				bool nodelooped = AnimBlendNode_UpdateCompressed((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
#ifdef FIX_BUGS
				// Use the C quaternion operation at the original animation boundary
				if(Quaternion_Dot(rot, q) < 0.0f)
					// Use the C quaternion operation at the original animation boundary
					Quaternion_SubAssign(&rot,&q);
				else
#endif
					// Use the C quaternion operation at the original animation boundary
					Quaternion_AddAssign(&rot,&q);
				// Use the complete C sequence at the original native operation boundary
				if(AnimSequence_HasTranslation((*node)->sequence)){
					FrameUpdate_Add(&pos,&vec);
					// Use the complete C association at the original native operation boundary
					if(AnimAssociation_HasTranslation((*node)->association)){
						FrameUpdate_Add(&trans,&vec);
						looped |= nodelooped;
						if(nodelooped){
							// Use the C blend-node policy at the original native call boundary
							AnimBlendNode_GetEndTranslationCompressed((*node),FrameUpdate_VectorOut(&vec),1.0f-totalBlendAmount);
							FrameUpdate_Add(&end,&vec);
						}
					}
				}
			}
			++*node;
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
			// Use the C quaternion operation at the original animation boundary
			Quaternion_Normalise(&rot);
			(*FrameUpdate_HostPoseRotation(xform).x) = rot.x;
			(*FrameUpdate_HostPoseRotation(xform).y) = rot.y;
			(*FrameUpdate_HostPoseRotation(xform).z) = rot.z;
			(*FrameUpdate_HostPoseRotation(xform).w) = rot.w;
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
			FrameUpdate_Store(FrameUpdate_HostVelocity3d(gpAnimBlendClump),FrameUpdate_Subtract(&trans,&cur));
			if(looped)
				FrameUpdate_AddOut(FrameUpdate_HostVelocity3d(gpAnimBlendClump),&end);
			(*FrameUpdate_HostPoseTranslation(xform).x) = FrameUpdate_Subtract(&pos,&trans).x + frame->resetPos.x;
			(*FrameUpdate_HostPoseTranslation(xform).y) = FrameUpdate_Subtract(&pos,&trans).y + frame->resetPos.y;
			(*FrameUpdate_HostPoseTranslation(xform).z) = FrameUpdate_Subtract(&pos,&trans).z + frame->resetPos.z;
		}
	}else{
		float transBlendAmount = 0.0f;

		if(updateData->foobar)
			for(node = updateData->nodes; *node; node++)
				// Use the complete C association at the original native operation boundary
				if((*node)->sequence && AnimAssociation_IsPartial((*node)->association))
					totalBlendAmount += (*node)->association->blendAmount;

		for(node = updateData->nodes; *node; node++){
			if((*node)->sequence){
				// Use the C blend-node policy at the original native call boundary
				AnimBlendNode_UpdateCompressed((*node),FrameUpdate_VectorOut(&vec),&q,1.0f-totalBlendAmount);
				// Use the complete C sequence at the original native operation boundary
				if(AnimSequence_HasTranslation((*node)->sequence)){
					FrameUpdate_Add(&pos,&vec);
					transBlendAmount += (*node)->association->blendAmount;
				}
				// Use the C quaternion operation at the original animation boundary
				if(Quaternion_Dot(rot, q) < 0.0f)
					// Use the C quaternion operation at the original animation boundary
					Quaternion_SubAssign(&rot,&q);
				else
					// Use the C quaternion operation at the original animation boundary
					Quaternion_AddAssign(&rot,&q);
			}
			++*node;
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_ROTATION) == 0){
			// Use the C quaternion operation at the original animation boundary
			Quaternion_Normalise(&rot);
			(*FrameUpdate_HostPoseRotation(xform).x) = rot.x;
			(*FrameUpdate_HostPoseRotation(xform).y) = rot.y;
			(*FrameUpdate_HostPoseRotation(xform).z) = rot.z;
			(*FrameUpdate_HostPoseRotation(xform).w) = rot.w;
		}

		// Use the C animation frame while retaining the original live SDK objects
		if((frame->flag & ANIM_FRAME_IGNORE_TRANSLATION) == 0){
			(*FrameUpdate_HostPoseTranslation(xform).x) = transBlendAmount*pos.x;
			(*FrameUpdate_HostPoseTranslation(xform).y) = transBlendAmount*pos.y;
			(*FrameUpdate_HostPoseTranslation(xform).z) = transBlendAmount*pos.z;
			(*FrameUpdate_HostPoseTranslation(xform).x) += (1.0f-transBlendAmount)*frame->resetPos.x;
			(*FrameUpdate_HostPoseTranslation(xform).y) += (1.0f-transBlendAmount)*frame->resetPos.y;
			(*FrameUpdate_HostPoseTranslation(xform).z) += (1.0f-transBlendAmount)*frame->resetPos.z;
		}
	}
}
//- rouz edit (ChatGPT)
