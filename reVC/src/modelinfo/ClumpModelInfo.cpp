//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "RwHelper.h"
#include "General.h"
#include "NodeName.h"
#include "VisibilityPlugins.h"
#include "ModelInfo.h"
#include "AnimManager.h"

void
CClumpModelInfo::DeleteRwObject(void)
{
	if(m_clump){
		RpClumpDestroy(m_clump);
		m_clump = nil;
		RemoveTexDictionaryRef();
		if(GetAnimFileIndex() != -1)
			// Use the complete C animation manager at the original call boundary
			AnimManager_RemoveAnimBlockRef(GetAnimFileIndex());
	}
}

static RpAtomic*
SetHierarchyForSkinAtomic(RpAtomic *atomic, void *data)
{
	RpSkinAtomicSetHAnimHierarchy(atomic, (RpHAnimHierarchy*)data);
	return nil;
}

RwObject*
CClumpModelInfo::CreateInstance(void)
{
	if(m_clump == nil)
		return nil;
	RpClump *clone = RpClumpClone(m_clump);
	if(IsClumpSkinned(clone)){
		RpHAnimHierarchy *hier;
		RpHAnimAnimation *anim;

		// Use the C helper and retain the original typed SDK result
		hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromClump(clone));
		assert(hier);
		RpClumpForAllAtomics(clone, SetHierarchyForSkinAtomic, hier);
		// Use the C helper and retain the original typed SDK result
		anim = ((RpHAnimAnimation*)HAnimAnimationCreateForHierarchy(hier));
		RpHAnimHierarchySetCurrentAnim(hier, anim);
		RpHAnimHierarchySetFlags(hier, (RpHAnimHierarchyFlag)(rpHANIMHIERARCHYUPDATEMODELLINGMATRICES|rpHANIMHIERARCHYUPDATELTMS));
	}
	return (RwObject*)clone;
}

RwObject*
CClumpModelInfo::CreateInstance(RwMatrix *m)
{
	if(m_clump){
		RpClump *clump = (RpClump*)CreateInstance();
		*RwFrameGetMatrix(RpClumpGetFrame(clump)) = *m;
		return (RwObject*)clump;
	}
	return nil;
}

RpAtomic*
CClumpModelInfo::SetAtomicRendererCB(RpAtomic *atomic, void *data)
{
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, (RpAtomicCallBackRender)data);
	return atomic;
}

void
CClumpModelInfo::SetClump(RpClump *clump)
{
	m_clump = clump;
	CVisibilityPlugins::SetClumpModelInfo(m_clump, this);
	AddTexDictionaryRef();
	if(GetAnimFileIndex() != -1)
		// Use the complete C animation manager at the original call boundary
		AnimManager_AddAnimBlockRef(GetAnimFileIndex());
	if(IsClumpSkinned(clump)){
		int i;
		RpHAnimHierarchy *hier;
		RpAtomic *skinAtomic;
		RpSkin *skin;

		// Use the C helper and retain the original typed SDK result
		hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromClump(clump));
		assert(hier);
		RpClumpForAllAtomics(clump, SetHierarchyForSkinAtomic, hier);
		// Use the C helper and retain the original typed SDK result
		skinAtomic = ((RpAtomic*)GetFirstAtomic(clump));

		assert(skinAtomic);
		skin = RpSkinGeometryGetSkin(RpAtomicGetGeometry(skinAtomic));
		// ignore const
		for(i = 0; i < RpGeometryGetNumVertices(RpAtomicGetGeometry(skinAtomic)); i++){
			RwMatrixWeights *weights = (RwMatrixWeights*)&RpSkinGetVertexBoneWeights(skin)[i];
			float sum = weights->w0 + weights->w1 + weights->w2 + weights->w3;
			weights->w0 /= sum;
			weights->w1 /= sum;
			weights->w2 /= sum;
			weights->w3 /= sum;
		}
		RpHAnimHierarchySetFlags(hier, (RpHAnimHierarchyFlag)(rpHANIMHIERARCHYUPDATEMODELLINGMATRICES|rpHANIMHIERARCHYUPDATELTMS));
	}
}

void
CClumpModelInfo::SetAnimFile(const char *file)
{
    // Apply the shared C animation-file policy
    ModelAnimFile_Set(&m_animFile,file);
}

void
CClumpModelInfo::ConvertAnimFileIndex(void)
{
    // Resolve the shared C animation-file state
    ModelAnimFile_Convert(&m_animFile);
}

void
CClumpModelInfo::SetFrameIds(RwObjectNameIdAssocation *assocs)
{
	int32 i;
	RwObjectNameAssociation objname;

	for(i = 0; assocs[i].name; i++)
		if((assocs[i].flags & CLUMP_FLAG_NO_HIERID) == 0){
			objname.frame = nil;
			objname.name = assocs[i].name;
			RwFrameForAllChildren(RpClumpGetFrame(m_clump), FindFrameFromNameWithoutIdCB, &objname);
			if(objname.frame)
				CVisibilityPlugins::SetFrameHierarchyId(objname.frame, assocs[i].hierId);
		}
}

RwFrame*
CClumpModelInfo::FindFrameFromIdCB(RwFrame *frame, void *data)
{
	RwObjectIdAssociation *assoc = (RwObjectIdAssociation*)data;

	if(CVisibilityPlugins::GetFrameHierarchyId(frame) == assoc->id){
		assoc->frame = frame;
		return nil;
	}
	RwFrameForAllChildren(frame, FindFrameFromIdCB, assoc);
	return assoc->frame ? nil : frame;
}

// unused
RwFrame*
CClumpModelInfo::FindFrameFromNameCB(RwFrame *frame, void *data)
{
	RwObjectNameAssociation *assoc = (RwObjectNameAssociation*)data;

	if(!General_faststricmp(GetFrameNodeName(frame), assoc->name)){
		assoc->frame = frame;
		return nil;
	}
	RwFrameForAllChildren(frame, FindFrameFromNameCB, assoc);
	return assoc->frame ? nil : frame;
}

RwFrame*
CClumpModelInfo::FindFrameFromNameWithoutIdCB(RwFrame *frame, void *data)
{
	RwObjectNameAssociation *assoc = (RwObjectNameAssociation*)data;

	if(CVisibilityPlugins::GetFrameHierarchyId(frame) == 0 &&
	   !General_faststricmp(GetFrameNodeName(frame), assoc->name)){
		assoc->frame = frame;
		return nil;
	}
	RwFrameForAllChildren(frame, FindFrameFromNameWithoutIdCB, assoc);
	return assoc->frame ? nil : frame;
}

RwFrame*
CClumpModelInfo::FillFrameArrayCB(RwFrame *frame, void *data)
{
	int32 id;
	RwFrame **frames = (RwFrame**)data;
	id = CVisibilityPlugins::GetFrameHierarchyId(frame);
	if(id > 0)
		frames[id] = frame;
	RwFrameForAllChildren(frame, FillFrameArrayCB, data);
	return frame;
}

void
CClumpModelInfo::FillFrameArray(RpClump *clump, RwFrame **frames)
{
	RwFrameForAllChildren(RpClumpGetFrame(clump), FillFrameArrayCB, frames);
}

RwFrame*
CClumpModelInfo::GetFrameFromId(RpClump *clump, int32 id)
{
	RwObjectIdAssociation assoc;
	assoc.id = id;
	assoc.frame = nil;
	RwFrameForAllChildren(RpClumpGetFrame(clump), FindFrameFromIdCB, &assoc);
	return assoc.frame;
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
