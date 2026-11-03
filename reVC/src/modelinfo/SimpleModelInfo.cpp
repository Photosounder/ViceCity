//+ rouz edit (ChatGPT)
#include "common.h"

#include "General.h"
#include "Camera.h"
#include "Renderer.h"
#include "ModelInfo.h"
#include "SimpleModelPolicy.h"
#include "SimpleModel_backend.h"
#include "AnimManager.h"
#include "custompipes.h"
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h"
#endif
//- rouz edit (ChatGPT)

void
CSimpleModelInfo::DeleteRwObject(void)
{
	int i;
	RwFrame *f;
	for(i = 0; i < m_numAtomics; i++)
		if(m_atomics[i]){
			f = RpAtomicGetFrame(m_atomics[i]);
			RpAtomicDestroy(m_atomics[i]);
			RwFrameDestroy(f);
			m_atomics[i] = nil;
			RemoveTexDictionaryRef();
			if(GetAnimFileIndex() != -1)
				// Use the complete C animation manager at the original call boundary
				AnimManager_RemoveAnimBlockRef(GetAnimFileIndex());
		}
}

RwObject*
CSimpleModelInfo::CreateInstance(void)
{
	RpAtomic *atomic;
	if(m_atomics[0] == nil)
		return nil;
	atomic = RpAtomicClone(m_atomics[0]);
	RpAtomicSetFrame(atomic, RwFrameCreate());
	return (RwObject*)atomic;
}

RwObject*
CSimpleModelInfo::CreateInstance(RwMatrix *matrix)
{
	RpAtomic *atomic;
	RwFrame *frame;

	if(m_atomics[0] == nil)
		return nil;
	atomic = RpAtomicClone(m_atomics[0]);
	frame = RwFrameCreate();
	*RwFrameGetMatrix(frame) = *matrix;
	RpAtomicSetFrame(atomic, frame);
	return (RwObject*)atomic;
}

void
CSimpleModelInfo::Init(void)
{
	m_atomics[0] = nil;
	m_atomics[1] = nil;
	m_atomics[2] = nil;
	m_numAtomics = 0;
	m_firstDamaged  = 0;
	m_wetRoadReflection    = 0;
	m_isDamaged     = 0;
	m_isBigBuilding = 0;
	m_noFade        = 0;
	m_drawLast      = 0;
	m_additive      = 0;
	m_isSubway      = 0;
	m_ignoreLight   = 0;
	m_noZwrite      = 0;
	m_noShadows     = 0;
	m_ignoreDrawDist  = 0;
	m_isCodeGlass     = 0;
	m_isArtistGlass   = 0;
}

void
CSimpleModelInfo::SetAtomic(int n, RpAtomic *atomic)
{
	AddTexDictionaryRef();
	m_atomics[n] = atomic;
	if(GetAnimFileIndex() != -1)
		// Use the complete C animation manager at the original call boundary
		AnimManager_AddAnimBlockRef(GetAnimFileIndex());
	RpGeometry *geo = RpAtomicGetGeometry(atomic);
	if(m_ignoreLight)
		RpGeometrySetFlags(geo, RpGeometryGetFlags(geo) & ~rpGEOMETRYLIGHT);
	if(RpGeometryGetFlags(geo) & rpGEOMETRYNORMALS &&
	   RpGeometryGetNumTriangles(geo) > 200)
		debug("%s has %d polys\n", m_name, RpGeometryGetNumTriangles(geo));

#ifdef EXTENDED_PIPELINES
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	// Attach world models to the CPU pipeline and preserve wet-road gloss state // rouz edit (ChatGPT)
	SoftwarePolygons::Attach(atomic, m_wetRoadReflection != 0); // rouz edit (ChatGPT)
#else
	//- rouz edit (ChatGPT)
	if(m_wetRoadReflection)
		CustomPipes::AttachGlossPipe(atomic);
	else
		CustomPipes::AttachWorldPipe(atomic);
	//+ rouz edit (ChatGPT)
#endif
	//- rouz edit (ChatGPT)
#endif
}

RpAtomic*
CSimpleModelInfo::GetAtomicFromDistance(float dist)
{
    // Resolve the selected native atomic after the C level-of-detail search
    int index=SimpleModel_AtomicIndex(m_lodDistances,m_numAtomics,m_firstDamaged,m_isDamaged,dist,TheCamera.LODDistMultiplier);
    return index<0 ? nil : m_atomics[index];
}

void SimpleModel_BackendRequireSimple(void *model)
{
    // Preserve the native assertion on the selected related model
    assert(((CBaseModelInfo*)model)->IsSimple());
}

void SimpleModel_BackendSetRelated(void *model,void *related)
{
    // Store the selected related model through the original native operation
    ((CSimpleModelInfo*)((CBaseModelInfo*)model))->m_atomics[2]=(RpAtomic*)((CSimpleModelInfo*)((CBaseModelInfo*)related));
}

void *SimpleModel_BackendRelated(void *model)
{
    // Return the existing related model using the common base pointer
    CSimpleModelInfo *related=(CSimpleModelInfo*)((CSimpleModelInfo*)((CBaseModelInfo*)model))->m_atomics[2];
    return (CBaseModelInfo*)related;
}

void SimpleModel_BackendMarkBig(void *model)
{
    // Set the native big-building flag
    ((CSimpleModelInfo*)((CBaseModelInfo*)model))->m_isBigBuilding=1;
}

float SimpleModel_BackendRelatedNearDistance(void *related)
{
    // Preserve the original scaled distance and camera division
    CSimpleModelInfo *simple=(CSimpleModelInfo*)((CBaseModelInfo*)related);
    return SimpleModel_LargestDistance(simple->m_lodDistances,simple->m_numAtomics,simple->m_firstDamaged,simple->m_isDamaged,TheCamera.LODDistMultiplier)/TheCamera.LODDistMultiplier;
}

void SimpleModel_BackendSetNearDistance(void *model,float distance)
{
    // Store the near distance in the original model
    ((CSimpleModelInfo*)((CBaseModelInfo*)model))->m_lodDistances[2]=distance;
}

int SimpleModel_BackendDrawLast(void *model)
{
    // Read the original draw-last flag after assigning the near distance
    return ((CSimpleModelInfo*)((CBaseModelInfo*)model))->m_drawLast;
}

void SimpleModel_BackendClearDrawLast(void *model)
{
    // Clear the flag and retain its native debug message
    CSimpleModelInfo *simple=(CSimpleModelInfo*)((CBaseModelInfo*)model);
    simple->m_drawLast=false;
    debug("%s was draw last\n",simple->GetModelName());
}

//- rouz edit (ChatGPT)
