//+ rouz edit (ChatGPT)
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "config.h"
#include "RpAnimBlend.h"
#include "RpAnimBlend_backend.h"
#include "AnimBlendClumpData.h"
#include "AnimBlendAssociation_backend.h"
#include "AnimManager.h"
#include "General.h"
#include "Bones.h"
#include "PedNode.h"
#ifndef MASTER
#define AnimPlugin_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define AnimPlugin_Assert(expression) (expression)
#endif
#define AnimPlugin_Debug(...) re3_debug("[DBG]: " __VA_ARGS__)
int32_t ClumpOffset;

enum
{
	ID_RPANIMBLEND = ANIM_PLUGIN_ID,
};

void*
AnimBlendClumpCreate(void *object, int32_t offsetInObject, int32_t sizeInObject)
{
    // Preserve the entire original plugin operation and live field access order
	*(CAnimBlendClumpData**)((unsigned char*)object+offsetInObject) = NULL;
	return object;
}

void*
AnimBlendClumpDestroy(void *object, int32_t offsetInObject, int32_t sizeInObject)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *data;
	data = *RPANIMBLENDCLUMPDATA(object);
	if(data){
		RpAnimBlendClumpRemoveAllAssociations((void*)object);
//+ rouz edit (ChatGPT)
		// Destroy and release clump animation data without invoking C++ delete.
		// Use the C animation owner at the original native operation boundary
		AnimBlendClump_Destroy(data);
		free(data);
//- rouz edit (ChatGPT)
		*RPANIMBLENDCLUMPDATA(object) = NULL;
	}
	return object;
}

void *AnimBlendClumpCopy(void *dstObject, const void *srcObject, int32_t offsetInObject, int32_t sizeInObject) {
    // Preserve the original unsupported plugin copy result
    return NULL;
}

bool
RpAnimBlendPluginAttach(void)
{
    // Preserve the entire original plugin operation and live field access order
	ClumpOffset = AnimPlugin_HostRegister(sizeof(CAnimBlendClumpData*), ID_RPANIMBLEND,
		AnimBlendClumpCreate, AnimBlendClumpDestroy, AnimBlendClumpCopy);
	return ClumpOffset >= 0;
}

CAnimBlendAssociation*
RpAnimBlendGetNextAssociation(CAnimBlendAssociation *assoc)
{
    // Preserve the entire original plugin operation and live field access order
	if(assoc->link.next)
		// Use the complete C association at the original native operation boundary
		return AnimAssociation_FromLink(assoc->link.next);
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendGetNextAssociationWithMask(CAnimBlendAssociation *assoc, uint32_t mask)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendLink *link;
	for(link = assoc->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		assoc = AnimAssociation_FromLink(link);
		if(assoc->flags & mask)
			return assoc;
	}
	return NULL;
}

void
RpAnimBlendAllocateData(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
//+ rouz edit (ChatGPT)
	// Construct clump animation data without invoking C++ new.
	CAnimBlendClumpData *data = (CAnimBlendClumpData*)malloc(sizeof(CAnimBlendClumpData));
	// Use the C animation owner at the original native operation boundary
	AnimBlendClump_Init(data);
	*RPANIMBLENDCLUMPDATA(clump) = data;
//- rouz edit (ChatGPT)
}


void
RpAnimBlendClumpSetBlendDeltas(void *clump, uint32_t mask, float delta)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);
		if(mask == 0 || (assoc->flags & mask))
			assoc->blendDelta = delta;
	}
}

void
RpAnimBlendClumpRemoveAllAssociations(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
	RpAnimBlendClumpRemoveAssociations(clump, 0);
}

void
RpAnimBlendClumpRemoveAssociations(void *clump, uint32_t mask)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	CAnimBlendLink *next;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = next){
		next = link->next;
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);
		if(mask == 0 || (assoc->flags & mask))
			if(assoc)
//+ rouz edit (ChatGPT)
				// Destroy and release the association without invoking C++ delete.
			{
				// Use the complete C association at the original native operation boundary
				AnimAssociation_Destroy(assoc);
				free(assoc);
			}
//- rouz edit (ChatGPT)
	}
}

void*
FrameForAllChildrenCountCallBack(void *frame, void *data)
{
    // Preserve the entire original plugin operation and live field access order
	int *numFrames = (int*)data;
	(*numFrames)++;
	AnimPlugin_HostForChildren(frame, FrameForAllChildrenCountCallBack, data);
	return frame;
}

void*
FrameForAllChildrenFillFrameArrayCallBack(void *frame, void *data)
{
    // Preserve the entire original plugin operation and live field access order
	AnimBlendFrameData **frames = (AnimBlendFrameData**)data;
	(*frames)->frame = frame;
	(*frames)++;
	AnimPlugin_HostForChildren(frame, FrameForAllChildrenFillFrameArrayCallBack, frames);
	return frame;
}

// FrameInitCallBack on PS2
void
FrameInitCBnonskin(AnimBlendFrameData *frameData, void *argument)
{
    // Preserve the entire original plugin operation and live field access order
	frameData->flag = 0;
	// Use the C animation frame while retaining the original live SDK objects
	frameData->resetPos = AnimPlugin_HostFramePosition(frameData->frame);
}

void
FrameInitCBskin(AnimBlendFrameData *frameData, void *argument)
{
    // Preserve the entire original plugin operation and live field access order
	frameData->flag = 0;
}

void
RpAnimBlendClumpInitSkinned(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
	int i;
	KeyFrameVector3 boneTab[64];
	CAnimBlendClumpData *clumpData;
	void *atomic;
	void *skin;
	void *hier;
	int numBones;

	RpAnimBlendAllocateData(clump);
	clumpData = *RPANIMBLENDCLUMPDATA(clump);
	atomic = AnimPlugin_HostFirstAtomic(clump);
	AnimPlugin_Assert(atomic);
	skin = AnimPlugin_HostSkinForAtomic(atomic);
	AnimPlugin_Assert(skin);
	numBones = AnimPlugin_HostNumBones(skin);
	// Use the C animation owner at the original native operation boundary
	AnimBlendClump_SetNumberOfBones(clumpData,numBones);
	hier = AnimPlugin_HostHierarchy(clump);
	AnimPlugin_Assert(hier);
	memset(boneTab, 0, sizeof(boneTab));
	AnimPlugin_HostBonePositions(clump, boneTab);

	AnimBlendFrameData *frames = clumpData->frames;
	for(i = 0; i < numBones; i++){
		frames[i].nodeID = AnimPlugin_HostNodeId(hier, i);
		// Use the C animation frame while retaining the original live SDK objects
		frames[i].resetPos = boneTab[i];
		frames[i].hanimFrame = AnimPlugin_HostInterpFrame(hier, i);
	}
	// Use the C animation owner at the original native operation boundary
	AnimBlendClump_ForAllFrames(clumpData,FrameInitCBskin, NULL);
	// Use the C animation frame while retaining the original live SDK objects
	clumpData->frames[0].flag |= ANIM_FRAME_VELOCITY_EXTRACTION;
}

void
RpAnimBlendClumpInitNotSkinned(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
	int numFrames = 0;
	CAnimBlendClumpData *clumpData;
	void *root;
	AnimBlendFrameData *frames;

	RpAnimBlendAllocateData(clump);
	clumpData = *RPANIMBLENDCLUMPDATA(clump);
	root = AnimPlugin_HostClumpFrame(clump);
	AnimPlugin_HostForChildren(root, FrameForAllChildrenCountCallBack, &numFrames);
	// Use the C animation owner at the original native operation boundary
	AnimBlendClump_SetNumberOfFrames(clumpData,numFrames);
	frames = clumpData->frames;
	AnimPlugin_HostForChildren(root, FrameForAllChildrenFillFrameArrayCallBack, &frames);
	// Use the C animation owner at the original native operation boundary
	AnimBlendClump_ForAllFrames(clumpData,FrameInitCBnonskin, NULL);
	// Use the C animation frame while retaining the original live SDK objects
	clumpData->frames[0].flag |= ANIM_FRAME_VELOCITY_EXTRACTION;
}

void
RpAnimBlendClumpInit(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
	if(AnimPlugin_HostIsSkinned(clump))
		RpAnimBlendClumpInitSkinned(clump);
	else
		RpAnimBlendClumpInitNotSkinned(clump);
}

bool
RpAnimBlendClumpIsInitialized(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	return clumpData && clumpData->numFrames != 0;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetAssociation(void *clump, uint32_t id)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == NULL) return NULL;

	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);
		if(assoc->animId == id)
			return assoc;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainAssociation(void *clump, CAnimBlendAssociation **assocRet, float *blendRet)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == NULL) return NULL;

	CAnimBlendAssociation *mainAssoc = NULL;
	CAnimBlendAssociation *secondAssoc = NULL;
	float mainBlend = 0.0f;
	float secondBlend = 0.0f;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);

		// Use the complete C association at the original native operation boundary
		if(AnimAssociation_IsPartial(assoc))
			continue;

		if(assoc->blendAmount > mainBlend){
			secondBlend = mainBlend;
			mainBlend = assoc->blendAmount;

			secondAssoc = mainAssoc;
			mainAssoc = assoc;
		}else if(assoc->blendAmount > secondBlend){
			secondBlend = assoc->blendAmount;
			secondAssoc = assoc;
		}
	}
	if(assocRet) *assocRet = secondAssoc;
	if(blendRet) *blendRet = secondBlend;
	return mainAssoc;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainPartialAssociation(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == NULL) return NULL;

	CAnimBlendAssociation *mainAssoc = NULL;
	float mainBlend = 0.0f;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);

		// Use the complete C association at the original native operation boundary
		if(!AnimAssociation_IsPartial(assoc))
			continue;

		if(assoc->blendAmount > mainBlend){
			mainBlend = assoc->blendAmount;
			mainAssoc = assoc;
		}
	}
	return mainAssoc;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainAssociation_N(void *clump, int n)
{
    // Preserve the entire original plugin operation and live field access order
	int i;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == NULL) return NULL;

	i = 0;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);

		// Use the complete C association at the original native operation boundary
		if(AnimAssociation_IsPartial(assoc))
			continue;

		if(i == n)
			return assoc;
		i++;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetMainPartialAssociation_N(void *clump, int n)
{
    // Preserve the entire original plugin operation and live field access order
	int i;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == NULL) return NULL;

	i = 0;
	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);

		// Use the complete C association at the original native operation boundary
		if(!AnimAssociation_IsPartial(assoc))
			continue;

		if(i == n)
			return assoc;
		i++;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetFirstAssociationWithMask(void *clump, uint32_t mask)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);

	if(clumpData == NULL) return NULL;

	for(CAnimBlendLink *link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		CAnimBlendAssociation *assoc = AnimAssociation_FromLink(link);
		if(assoc->flags & mask)
			return assoc;
	}
	return NULL;
}

CAnimBlendAssociation*
RpAnimBlendClumpGetFirstAssociation(void *clump)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	if(!RpAnimBlendClumpIsInitialized(clump))
		return NULL;
	if(clumpData->link.next)
		// Use the complete C association at the original native operation boundary
		return AnimAssociation_FromLink(clumpData->link.next);
	return NULL;
}

// FillFrameArrayCallBack on PS2
void
FillFrameArrayCBnonskin(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the entire original plugin operation and live field access order
	AnimBlendFrameData **frames = (AnimBlendFrameData**)arg;
	// Use the C animation frame while retaining the original live SDK objects
	frames[AnimPlugin_HostFrameHierarchyId(frame->frame)] = frame;
}

void
RpAnimBlendClumpFillFrameArraySkin(void *clump, AnimBlendFrameData **frames)
{
    // Preserve the entire original plugin operation and live field access order
	int i;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	void *hier = AnimPlugin_HostHierarchy(clump);
	for(i = PED_MID; i < PED_NODE_MAX; i++)
		frames[i] = &clumpData->frames[AnimPlugin_HostFindNode(hier, ConvertPedNode2BoneTag(i))];
}

void
RpAnimBlendClumpFillFrameArray(void *clump, AnimBlendFrameData **frames)
{
    // Preserve the entire original plugin operation and live field access order
	if(AnimPlugin_HostIsSkinned(clump))
		RpAnimBlendClumpFillFrameArraySkin(clump, frames);
	else
		// Use the C animation owner at the original native operation boundary
		AnimBlendClump_ForAllFrames((*RPANIMBLENDCLUMPDATA(clump)),FillFrameArrayCBnonskin, frames);
}

AnimBlendFrameData *pFrameDataFound;

void
FrameFindByNameCBnonskin(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the entire original plugin operation and live field access order
	// Use the C animation frame while retaining the original live SDK objects
	char *nodename = AnimPlugin_HostFrameName(frame->frame);
	if(!General_faststricmp(nodename, (char*)arg))
		pFrameDataFound = frame;
}

void
FrameFindByNameCBskin(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the entire original plugin operation and live field access order
	const char *name = ConvertBoneTag2BoneName(frame->nodeID);
	if(name && General_faststricmp(name, (char*)arg) == 0)
		pFrameDataFound = frame;
}

void
FrameFindByBoneCB(AnimBlendFrameData *frame, void *arg)
{
    // Preserve the entire original plugin operation and live field access order
	if(frame->nodeID == (int32_t)(uintptr_t)arg)
		pFrameDataFound = frame;
}

AnimBlendFrameData*
RpAnimBlendClumpFindFrame(void *clump, const char *name)
{
    // Preserve the entire original plugin operation and live field access order
	pFrameDataFound = NULL;
	if(AnimPlugin_HostIsSkinned(clump))
		// Use the C animation owner at the original native operation boundary
		AnimBlendClump_ForAllFrames((*RPANIMBLENDCLUMPDATA(clump)),FrameFindByNameCBskin, (void*)name);
	else
		// Use the C animation owner at the original native operation boundary
		AnimBlendClump_ForAllFrames((*RPANIMBLENDCLUMPDATA(clump)),FrameFindByNameCBnonskin, (void*)name);
	return pFrameDataFound;
}

AnimBlendFrameData*
RpAnimBlendClumpFindBone(void *clump, uint32_t boneTag)
{
    // Preserve the entire original plugin operation and live field access order
	pFrameDataFound = NULL;
	// Use the C animation owner at the original native operation boundary
	AnimBlendClump_ForAllFrames((*RPANIMBLENDCLUMPDATA(clump)),FrameFindByBoneCB, (void*)(uintptr_t)boneTag);
	return pFrameDataFound;
}

void
RpAnimBlendNodeUpdateKeyframes(AnimBlendFrameData *frames, AnimBlendFrameUpdateData *updateData, int32_t numNodes)
{
    // Preserve the entire original plugin operation and live field access order
	CAnimBlendNode **node;
	int i;

	for(node = updateData->nodes; *node; node++){
		CAnimBlendAssociation *a = (*node)->association;
		for(i = 0; i < numNodes; i++)
			// Use the C animation frame while retaining the original live SDK objects
			if((frames[i].flag & ANIM_FRAME_VELOCITY_EXTRACTION) == 0 ||
			   gpAnimBlendClump->velocity2d == NULL){
				if((*node)[i].sequence)
					// Use the C blend-node policy at the original native call boundary
					AnimBlendNode_FindKeyFrame(&(*node)[i],a->currentTime - a->timeStep);
			}
	}
}

// TODO:
// CAnimBlendClumpData::LoadFramesIntoSPR
// CAnimBlendClumpData::ForAllFramesInSPR
void
RpAnimBlendClumpUpdateAnimations(void *clump, float timeDelta, bool doRender)
{
    // Preserve the entire original plugin operation and live field access order
	int i;
	CAnimBlendAssociation *assoc;
	AnimBlendFrameUpdateData updateData;
	float totalLength = 0.0f;
	float totalBlend = 0.0f;
	CAnimBlendLink *link, *next;
	CAnimBlendClumpData *clumpData = *RPANIMBLENDCLUMPDATA(clump);
	gpAnimBlendClump = clumpData;

	if(clumpData->link.next == NULL)
		return;

	// Update blend and get node array
	i = 0;
	updateData.foobar = 0;
	for(link = clumpData->link.next; link; link = next){
		next = link->next;
		// Use the complete C association at the original native operation boundary
		assoc = AnimAssociation_FromLink(link);
		// Use the complete C association at the original native operation boundary
		if(AnimAssociation_UpdateBlend(assoc,timeDelta)){
			if(assoc->hierarchy->sequences){
				// Use the complete C animation manager at the original call boundary
				AnimManager_UncompressAnimation(assoc->hierarchy);
				if(i < 11)
					// Use the complete C association at the original native operation boundary
					updateData.nodes[i++] = AnimAssociation_GetNode(assoc,0);
				if(assoc->flags & ASSOC_MOVEMENT){
					totalLength += assoc->hierarchy->totalLength/assoc->speed * assoc->blendAmount;
					totalBlend += assoc->blendAmount;
				}else
					updateData.foobar = 1;
			}else
				AnimPlugin_Debug("anim %s is not loaded\n", assoc->hierarchy->name);
		}
	}

	for(link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		assoc = AnimAssociation_FromLink(link);
		// Use the complete C association at the original native operation boundary
		AnimAssociation_UpdateTimeStep(assoc,timeDelta, totalLength == 0.0f ? 1.0f : totalBlend/totalLength);
	}

	updateData.nodes[i] = NULL;

#ifdef ANIM_COMPRESSION
	// Use the C animation frame while retaining the original live SDK objects
	if(clumpData->frames[0].flag & ANIM_FRAME_COMPRESSED){
		if(AnimPlugin_HostIsSkinned(clump))
			// Use the C animation owner at the original native operation boundary
			AnimBlendClump_ForAllFrames(clumpData,FrameUpdateCallBackSkinnedCompressed, &updateData);
		else
			// Use the C animation owner at the original native operation boundary
			AnimBlendClump_ForAllFrames(clumpData,FrameUpdateCallBackNonSkinnedCompressed, &updateData);
	}else
#endif
	if(doRender){
		// Use the C animation frame while retaining the original live SDK objects
		if(clumpData->frames[0].flag & ANIM_FRAME_UPDATE_KEYFRAMES)
			RpAnimBlendNodeUpdateKeyframes(clumpData->frames, &updateData, clumpData->numFrames);
		if(AnimPlugin_HostIsSkinned(clump))
			// Use the C animation owner at the original native operation boundary
			AnimBlendClump_ForAllFrames(clumpData,FrameUpdateCallBackSkinned, &updateData);
		else
			// Use the C animation owner at the original native operation boundary
			AnimBlendClump_ForAllFrames(clumpData,FrameUpdateCallBackNonSkinned, &updateData);
		// Use the C animation frame while retaining the original live SDK objects
		clumpData->frames[0].flag &= ~ANIM_FRAME_UPDATE_KEYFRAMES;
	}else{
		// Use the C animation owner at the original native operation boundary
		AnimBlendClump_ForAllFrames(clumpData,FrameUpdateCallBackOffscreen, &updateData);
		// Use the C animation frame while retaining the original live SDK objects
		clumpData->frames[0].flag |= ANIM_FRAME_UPDATE_KEYFRAMES;
	}

	for(link = clumpData->link.next; link; link = link->next){
		// Use the complete C association at the original native operation boundary
		assoc = AnimAssociation_FromLink(link);
		// Use the complete C association at the original native operation boundary
		AnimAssociation_UpdateTime(assoc,timeDelta, totalLength == 0.0f ? 1.0f : totalBlend/totalLength);
	}
	AnimPlugin_HostUpdateObjects(AnimPlugin_HostClumpFrame(clump));
}
//- rouz edit (ChatGPT)



CAnimBlendClumpData *AnimAssociation_HostClumpData(void *clump)
{
    // Borrow the actual plugin owner directly from the complete C plugin implementation
    return *RPANIMBLENDCLUMPDATA(clump);
}
AnimBlendFrameData *AnimAssociation_HostFindFrame(void *clump,const char *name)
{
    // Invoke the complete C frame-name search through its existing owner boundary
    return RpAnimBlendClumpFindFrame(clump,name);
}
AnimBlendFrameData *AnimAssociation_HostFindBone(void *clump,uint32_t tag)
{
    // Invoke the complete C bone-tag search without a native algorithm bridge
    return RpAnimBlendClumpFindBone(clump,tag);
}
//- rouz edit (ChatGPT)
