//+ rouz edit (ChatGPT)
#include "WeaponEffects.h"
#include "WeaponEffects_backend.h"
#include "Sprite.h"
#include "TxdStore.h"
#include "RwHelper.h"
#include "WeaponType.h"
#define TWOPI (2.0f * 3.14159265358979323846f)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)


void *gpCrossHairTex;

WeaponEffects gCrossHair;

void WeaponEffects_Construct(WeaponEffects *self)
{
    // Preserve the empty original constructor and empty vector construction
    (void)self;
}

void WeaponEffects_Deinit(WeaponEffects *self)
{
    // Preserve the empty original destructor without changing texture ownership
    (void)self;
}

void
WeaponEffects_Init(void)
{
    // Execute the complete original weapon effect operation with C-owned state
	gCrossHair.m_bActive = false;
	gCrossHair.m_vecPos = (WeaponEffectsVector){0.0f, 0.0f, 0.0f};
	gCrossHair.m_nRed = 255;
	gCrossHair.m_nGreen = 0;
	gCrossHair.m_nBlue = 0;
	gCrossHair.m_nAlpha = 127;
	gCrossHair.m_fSize = 1.0f;
	gCrossHair.m_fRotation = 0.0f;
	
	
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PushCurrentTxd();
	// Use the complete C texture dictionary owner with the original live arguments
	int32_t slot = TxdStore_FindTxdSlot("particle");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_SetCurrentTxd(slot);
	
	gpCrossHairTex    = RwHelper_ReadTexture("target256", "target256m");
	
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PopCurrentTxd();
}

void
WeaponEffects_Shutdown(void)
{
    // Execute the complete original weapon effect operation with C-owned state
	RwHelper_DestroyTexture(gpCrossHairTex);
	gpCrossHairTex = NULL;
}

void
WeaponEffects_MarkTarget(WeaponEffectsVector pos, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha, float size)
{
    // Execute the complete original weapon effect operation with C-owned state
	gCrossHair.m_bActive = true;
	gCrossHair.m_vecPos = pos;
	gCrossHair.m_fSize = size;
}

void
WeaponEffects_ClearCrossHair(void)
{
    // Execute the complete original weapon effect operation with C-owned state
	gCrossHair.m_bActive = false;
}

void
WeaponEffects_Render(void)
{
    // Execute the complete original weapon effect operation with C-owned state
	static float aCrossHairSize[WEAPONTYPE_TOTALWEAPONS] =
	{
		1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		0.4f, 0.4f,
		0.5f,
		0.3f,
		0.9f, 0.9f, 0.9f,
		0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f,
		0.1f, 0.1f,
		1.0f,
		0.6f,
		0.7f,
		0.0f, 0.0f
	};



	if ( gCrossHair.m_bActive )
	{
		float size = aCrossHairSize[WeaponEffects_HostWeaponType()];
		
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEZWRITEENABLE),      (void *)false);
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEZTESTENABLE),       (void *)false);
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void *)true);
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATESRCBLEND),          (void *)(uintptr_t)WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwBLENDSRCALPHA));
#ifdef FIX_BUGS
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEDESTBLEND),         (void *)(uintptr_t)WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwBLENDINVSRCALPHA));
#else
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEDESTBLEND),         (void *)(uintptr_t)WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwBLENDINVDESTALPHA));
#endif
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATETEXTURERASTER),     (void *)RwHelper_TextureRaster(gpCrossHairTex));

		WeaponEffectsVector pos;
		float w, h;
		if ( Sprite_CalcScreenCoors(Sprite_VectorRef(&gCrossHair.m_vecPos.x,&gCrossHair.m_vecPos.y,&gCrossHair.m_vecPos.z),Sprite_VectorOut(&pos.x,&pos.y,&pos.z),&w,&h,true) )
		{
			#ifndef FINAL
            PushRendergroup("CWeaponEffects::Render");
#endif

			float recipz = 1.0f / pos.z;
			// Use the complete C sprite operation with the original argument values
			Sprite_RenderOneXLUSprite_Rotate_Aspect(pos.x, pos.y, pos.z,
				w, h,
				255, 88, 100, 158,
				recipz, gCrossHair.m_fRotation, gCrossHair.m_nAlpha);
				
			float recipz2 = 1.0f / pos.z;
			
			// Use the complete C sprite operation with the original argument values
			Sprite_RenderOneXLUSprite_Rotate_Aspect(pos.x, pos.y, pos.z,
				size*w, size*h,
				107, 134, 247, 158,
				recipz2, TWOPI - gCrossHair.m_fRotation, gCrossHair.m_nAlpha);
						
			gCrossHair.m_fRotation += 0.02f;
			if ( gCrossHair.m_fRotation > TWOPI )
				gCrossHair.m_fRotation = 0.0;

			#ifndef FINAL
            PopRendergroup();
#endif
		}
			
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEVERTEXALPHAENABLE), (void *)false);
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEZWRITEENABLE),      (void *)true);
		RwHelper_HostRenderState(WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_rwRENDERSTATEZTESTENABLE),       (void *)true);
	}
}//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
