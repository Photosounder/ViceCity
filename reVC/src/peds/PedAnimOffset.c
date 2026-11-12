//+ rouz edit (Claude)
#include "PedAnimOffset.h"
#include "AnimManager.h"
#include "AnimBlendSequence.h"
#include "KeyFrames.h"

bool PedAnim_LoadOffset(int32_t groupId, int32_t animId, VectorMath3 *out)
{
	CAnimBlendHierarchy *hierarchy = AnimManager_GetAnimAssociationById(groupId, animId)->hierarchy;
	CAnimBlendSequence *seq = hierarchy->sequences;

	AnimManager_UncompressAnimation(hierarchy);
	if (seq->numFrames <= 0)
		return false;

	if (!AnimSequence_HasTranslation(seq)) {
		out->x = 0.0f;
		out->y = 0.0f;
		out->z = 0.0f;
	} else {
		// The movement is the translation of the last key frame
		KeyFrameTrans *lastFrame = (KeyFrameTrans *)AnimSequence_GetKeyFrame(seq, seq->numFrames - 1);

		out->x = lastFrame->translation.x;
		out->y = lastFrame->translation.y;
		out->z = lastFrame->translation.z;
	}

	return true;
}
//- rouz edit (Claude)
