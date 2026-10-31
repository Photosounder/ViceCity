//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"

#include "RwHelper.h"
#include "General.h"
#include "Bones.h"
#include "SurfaceTable.h"
#include "Ped.h"
#include "NodeName.h"
#include "VisibilityPlugins.h"
#include "ModelInfo.h"
#include "custompipes.h"
#ifdef REVC_SOFTWARE_POLYGONS
#include "SoftwarePolygons.h" // rouz edit (ChatGPT)
#endif
#include "Pools.h" // rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
CPedModelInfo::~CPedModelInfo(void)
{
	if(m_hitColModel){
		// Destroy and release the hit collision model without invoking C++ delete.
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_Destroy(m_hitColModel);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Delete(CPools::GetColModelPool(), m_hitColModel);
		//- rouz edit (ChatGPT)
	}
}
//- rouz edit (ChatGPT)

void
CPedModelInfo::DeleteRwObject(void)
{
	CClumpModelInfo::DeleteRwObject();
	if(m_hitColModel)
//+ rouz edit (ChatGPT)
		// Destroy and release the hit collision model without invoking C++ delete.
	{
		// Invoke the complete C collision owner at the original native lifetime or operation boundary
		ColModel_Destroy(m_hitColModel);
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Delete(CPools::GetColModelPool(), m_hitColModel);
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)
	m_hitColModel = nil;
}

// leftover...
RwObjectNameIdAssocation CPedModelInfo::m_pPedIds[PED_NODE_MAX] = {
	{ "Smid",	PED_MID, 0, },	// that is strange...
	{ "Shead",	PED_HEAD, 0, },
	{ "Supperarml",	PED_UPPERARML, 0, },
	{ "Supperarmr",	PED_UPPERARMR, 0, },
	{ "SLhand",	PED_HANDL, 0, },
	{ "SRhand",	PED_HANDR, 0, },
	{ "Supperlegl",	PED_UPPERLEGL, 0, },
	{ "Supperlegr",	PED_UPPERLEGR, 0, },
	{ "Sfootl",	PED_FOOTL, 0, },
	{ "Sfootr",	PED_FOOTR, 0, },
	{ "Slowerlegr",	PED_LOWERLEGR, 0, },
	{ nil,	0, 0, },
};

void
CPedModelInfo::SetClump(RpClump *clump)
{
#ifdef EXTENDED_PIPELINES
	// Preserve the ped rim-light effect in the CPU renderer
	//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
	SoftwarePolygons::AttachRimlightClump(clump);
#else
	CustomPipes::AttachRimPipe(clump);
#endif
	//- rouz edit (ChatGPT)
#endif
	CClumpModelInfo::SetClump(clump);
	SetFrameIds(m_pPedIds);	// not needed in VC actually
	if(m_hitColModel == nil)
		CreateHitColModelSkinned(clump);
	RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPedCB);
	if(strcmp(GetModelName(), "player") == 0)
		RpClumpForAllAtomics(m_clump, SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPlayerCB);
}

struct ColNodeInfo
{
	Const char *name;
	int pedNode;
	int pieceType;
	float x, z;
	float radius;
};

#define NUMPEDINFONODES 10
ColNodeInfo m_pColNodeInfos[NUMPEDINFONODES] = {
	{ nil, PED_HEAD,	PEDPIECE_HEAD,	0.0f,   0.05f, 0.15f },
	{ nil, PED_MID,		PEDPIECE_TORSO,	0.0f,   0.15f, 0.2f },
	{ nil, PED_MID,		PEDPIECE_TORSO,	0.0f,  -0.05f, 0.25f },
	{ nil, PED_MID,		PEDPIECE_MID,	0.0f,  -0.25f, 0.25f },
	{ nil, PED_UPPERARML,	PEDPIECE_LEFTARM,	0.03f, -0.05f,  0.16f },
	{ nil, PED_UPPERARMR,	PEDPIECE_RIGHTARM,	-0.03f, -0.05f,  0.16f },
	{ nil, PED_LOWERLEGL,	PEDPIECE_LEFTLEG,	0.0f,   0.15f, 0.2f },
	{ nil, PED_LOWERLEGR,	PEDPIECE_RIGHTLEG,	0.0f,   0.15f, 0.2f },
	{ nil, PED_FOOTL,	PEDPIECE_LEFTLEG,	0.0f,   0.15f, 0.15f },
	{ nil, PED_FOOTR,	PEDPIECE_RIGHTLEG,	0.0f,   0.15f, 0.15f },
};

void
CPedModelInfo::CreateHitColModelSkinned(RpClump *clump)
{
	// Use the C helper and retain the original typed SDK result
	RpHAnimHierarchy *hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(clump));
//+ rouz edit (ChatGPT)
	// Construct the hit collision model in pool storage without invoking C++ new.
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CColModel *colmodel = ((CColModel*)CPool_New(CPools::GetColModelPool()));
	//- rouz edit (ChatGPT)
	assert(colmodel);
	// Invoke the complete C collision owner at the original native lifetime or operation boundary
	ColModel_Init(colmodel);
//- rouz edit (ChatGPT)
	CColSphere *spheres = (CColSphere*)RwMalloc(NUMPEDINFONODES*sizeof(CColSphere));
	RwFrame *root = RpClumpGetFrame(m_clump);
	RwMatrix *invmat = RwMatrixCreate();
	RwMatrix *mat = RwMatrixCreate();
	RwMatrixInvert(invmat, RwFrameGetMatrix(RpClumpGetFrame(clump)));

	for(int i = 0; i < NUMPEDINFONODES; i++){
		*mat = *invmat;

		// From LCS. Otherwise gives FPE
#ifdef FIX_BUGS
		spheres[i].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.0f));
#else
		int id = ConvertPedNode2BoneTag(m_pColNodeInfos[i].pedNode);	// this is wrong, wtf R* ???
		int idx = RpHAnimIDGetIndex(hier, id);

		// This doesn't really work as the positions are not initialized yet
		RwMatrixTransform(mat, &RpHAnimHierarchyGetMatrixArray(hier)[idx], rwCOMBINEPRECONCAT);
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RwV3dTransformPoints(&pos, &pos, 1, mat);

		spheres[i].bounds.center = CollisionVector_FromNative(pos + CVector(m_pColNodeInfos[i].x, 0.0f, m_pColNodeInfos[i].z));
#endif
		spheres[i].bounds.radius = m_pColNodeInfos[i].radius;
		spheres[i].surface = SURFACE_PED;
		spheres[i].piece = m_pColNodeInfos[i].pieceType;
	}
	RwMatrixDestroy(invmat);
	RwMatrixDestroy(mat);
	colmodel->spheres = spheres;
	colmodel->numSpheres = NUMPEDINFONODES;
	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter0=CVector(0.0f, 0.0f, 0.0f);
	    Sphere_Set(&colmodel->boundingSphere,2.0f,COLLISION_POINT_REF(sphereCenter0));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum0=CVector(-0.5f, -0.5f, -1.2f);
	    const CVector &boxMaximum0=CVector(0.5f, 0.5f, 1.2f);
	    Box_Set(&colmodel->boundingBox,COLLISION_POINT_REF(boxMinimum0),COLLISION_POINT_REF(boxMaximum0));
	}
	colmodel->level = LEVEL_GENERIC;
	m_hitColModel = colmodel;
}

CColModel*
CPedModelInfo::AnimatePedColModelSkinned(RpClump *clump)
{
	if(m_hitColModel == nil){
		CreateHitColModelSkinned(clump);
		return m_hitColModel;
	}
	RwMatrix *invmat, *mat;
	CColSphere *spheres = m_hitColModel->spheres;
	// Use the C helper and retain the original typed SDK result
	RpHAnimHierarchy *hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(clump));
	invmat = RwMatrixCreate();
	mat = RwMatrixCreate();
	RwMatrixInvert(invmat, RwFrameGetMatrix(RpClumpGetFrame(clump)));

	for(int i = 0; i < NUMPEDINFONODES; i++){
		*mat = *invmat;
		int id = ConvertPedNode2BoneTag(m_pColNodeInfos[i].pedNode);
		int idx = RpHAnimIDGetIndex(hier, id);

		RwMatrixTransform(mat, &RpHAnimHierarchyGetMatrixArray(hier)[idx], rwCOMBINEPRECONCAT);
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RwV3dTransformPoints(&pos, &pos, 1, mat);

		spheres[i].bounds.center = CollisionVector_FromNative(pos + CVector(m_pColNodeInfos[i].x, 0.0f, m_pColNodeInfos[i].z));
	}
	RwMatrixDestroy(invmat);
	RwMatrixDestroy(mat);
	return m_hitColModel;
}

CColModel*
CPedModelInfo::AnimatePedColModelSkinnedWorld(RpClump *clump)
{
	if(m_hitColModel == nil)
		CreateHitColModelSkinned(clump);
	CColSphere *spheres = m_hitColModel->spheres;
	// Use the C helper and retain the original typed SDK result
	RpHAnimHierarchy *hier = ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(clump));
	RwMatrix *mat;

	for(int i = 0; i < NUMPEDINFONODES; i++){
		int id = ConvertPedNode2BoneTag(m_pColNodeInfos[i].pedNode);
		int idx = RpHAnimIDGetIndex(hier, id);

		mat = &RpHAnimHierarchyGetMatrixArray(hier)[idx];
		RwV3d pos = { 0.0f, 0.0f, 0.0f };
		RwV3dTransformPoints(&pos, &pos, 1, mat);

		spheres[i].bounds.center = CollisionVector_FromNative(pos + CVector(m_pColNodeInfos[i].x, 0.0f, m_pColNodeInfos[i].z));
	}
	return m_hitColModel;
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
