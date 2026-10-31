//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "AnimBlendFrameData_native.h"

#include "main.h"
#include "RwHelper.h"
#include "Lights.h"
#include "PointLights.h"
#include "RpAnimBlend.h"
#include "AnimBlendClumpData.h"
#include "Bones.h"
#include "Renderer.h"
#include "ModelIndices.h"
#include "Shadows.h"
#include "Timecycle.h"
#include "CutsceneShadow.h"
#include "CutsceneObject.h"
#include "ModelIndices.h"
#include "RpAnimBlend.h"


CCutsceneObject::CCutsceneObject(void)
{
	SetStatus(STATUS_SIMPLE);
	bUsesCollision = false;
	bStreamingDontDelete = true;
	ObjectCreatedBy = CUTSCENE_OBJECT;
	m_fMass = 1.0f;
	m_fTurnMass = 1.0f;
	
	m_pAttachTo = nil;
	m_pAttachmentObject = nil;
	m_pShadow = nil;
}

CCutsceneObject::~CCutsceneObject(void)
{
	if ( m_pShadow )
	{
//+ rouz edit (ChatGPT)
		// Destroy and release the cutscene shadow without invoking C++ delete.
		// Use the C shadow operation with its canonical record and explicit lifecycle
		CutsceneShadow_Deinit(m_pShadow);
		free(m_pShadow);
//- rouz edit (ChatGPT)
		m_pShadow = nil;
	}
}

void
CCutsceneObject::SetModelIndex(uint32 id)
{
	CEntity::SetModelIndex(id);
	assert(RwObjectGetType(m_rwObject) == rpCLUMP);
	RpAnimBlendClumpInit((RpClump*)m_rwObject);
	(*RPANIMBLENDCLUMPDATA(m_rwObject))->velocity3d = &m_vecMoveSpeed;
	// Use the C animation frame while retaining the original live SDK objects
	(*RPANIMBLENDCLUMPDATA(m_rwObject))->frames[0].flag |= ANIM_FRAME_VELOCITY_EXTRACTION_3D;
}

void
CCutsceneObject::CreateShadow(void)
{
	if ( IsPedModel(GetModelIndex()) )
	{
//+ rouz edit (ChatGPT)
		// Allocate the cutscene shadow without invoking C++ new.
		m_pShadow = (CutsceneShadow*)malloc(sizeof(CutsceneShadow));
		assert(m_pShadow);
		// Use the C shadow operation with its canonical record and explicit lifecycle
		CutsceneShadow_Init(m_pShadow);
//- rouz edit (ChatGPT)
		// Use the C shadow operation with its canonical record and explicit lifecycle
		if (!CutsceneShadow_IsInitialized(m_pShadow))
			// Use the C shadow operation with its canonical record and explicit lifecycle
			CutsceneShadow_Create(m_pShadow,m_rwObject, 6, true, 4, true);
	}
}

void
CCutsceneObject::ProcessControl(void)
{
	CPhysical::ProcessControl();

	if ( m_pAttachTo )
	{
		if ( m_pAttachmentObject )
			GetMatrix() = CMatrix((RwMatrix*)m_pAttachTo);
		else
			GetMatrix() = CMatrix(RwFrameGetLTM((RwFrame*)m_pAttachTo));
	}
	else
	{
		if((*Timer_GetTimeStep()) < 1/100.0f)
			m_vecMoveSpeed *= 100.0f;
		else
			m_vecMoveSpeed *= 1.0f/(*Timer_GetTimeStep());
	
		ApplyMoveSpeed();
	}
}

static RpMaterial*
MaterialSetAlpha(RpMaterial *material, void *data)
{
	((RwRGBA*)RpMaterialGetColor(material))->alpha = (uint8)(uintptr)data;
	return material;
}

void
CCutsceneObject::PreRender(void)
{
	if ( m_pAttachTo )
	{
		if ( m_pAttachmentObject )
		{
			m_pAttachmentObject->UpdateRpHAnim();
			GetMatrix() = CMatrix((RwMatrix*)m_pAttachTo);
		}
		else
			GetMatrix() = CMatrix(RwFrameGetLTM((RwFrame*)m_pAttachTo));
		
		if ( RwObjectGetType(m_rwObject) == rpCLUMP && IsClumpSkinned(GetClump()) )
		{
			// Use the C helper and retain the original typed SDK result
			RpAtomic *atomic = ((RpAtomic*)GetFirstAtomic(GetClump()));
			// Use the C animation frame while retaining the original live SDK objects
			atomic->boundingSphere.center = AnimBlendFrame_GetHAnim(&(*RPANIMBLENDCLUMPDATA(GetClump()))->frames[0])->t;
		}
	}
	
	if ( RwObjectGetType(m_rwObject) == rpCLUMP )
		UpdateRpHAnim();
	
	if(IsPedModel(GetModelIndex()))
	{
		if ( m_pShadow == nil )
		{
			CShadows::StoreShadowForPedObject(this,
				TimeCycle_m_fShadowDisplacementX[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowDisplacementY[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowFrontX[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowFrontY[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowSideX[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowSideY[TimeCycle_m_CurrentStoredValue]);
		}
		else
		{
			// Use the C shadow operation with its canonical record and explicit lifecycle
			if ( CutsceneShadow_IsInitialized(m_pShadow) )
				// Use the C shadow operation with its canonical record and explicit lifecycle
				CutsceneShadow_UpdateForCutscene(m_pShadow);
			
			CShadows::StoreShadowForCutscenePedObject(this,
				TimeCycle_m_fShadowDisplacementX[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowDisplacementY[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowFrontX[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowFrontY[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowSideX[TimeCycle_m_CurrentStoredValue],
				TimeCycle_m_fShadowSideY[TimeCycle_m_CurrentStoredValue]);
		}
			
		// For some reason xbox/android limbs are transparent here...
		// Use the C helper and retain the original typed SDK result
		RpGeometry *geometry = RpAtomicGetGeometry(((RpAtomic*)GetFirstAtomic(GetClump())));
		RpGeometrySetFlags(geometry, RpGeometryGetFlags(geometry) | rpGEOMETRYMODULATEMATERIALCOLOR);
		RpGeometryForAllMaterials(geometry, MaterialSetAlpha, (void*)255);
	}
}

void
CCutsceneObject::Render(void)
{
	SetCullMode(rwCULLMODECULLNONE);
	CObject::Render();
	SetCullMode(rwCULLMODECULLBACK);
}

bool
CCutsceneObject::SetupLighting(void)
{
	// Use the complete C lighting owner and its borrowed SDK handles
	Lights_ActivateDirectional();
	// Use the complete C lighting owner and its borrowed SDK handles
	Lights_SetAmbientColoursForPedsCarsAndObjects();

	if(bRenderScorched){
		// Use the complete C lighting owner and its borrowed SDK handles
		Lights_WorldReplaceNormalLightsWithScorched(Scene.world, 0.1f);
	}else{
		CVector coors = GetPosition();
		// Enter the C point-light algorithm with the original owner expressions
		float lighting = PointLights_GenerateLightsAffectingObjectLegacy(&coors);
		if(lighting != 1.0f){
			// Use the complete C lighting owner and its borrowed SDK handles
			Lights_SetAmbientAndDirectionalColours(lighting);
			return true;
		}
	}

	return false;
}

void
CCutsceneObject::RemoveLighting(bool reset)
{
	CRenderer::RemoveVehiclePedLights(this, reset);
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
