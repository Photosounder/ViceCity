//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "KeyFrames_native.h"
#include "ColSphere_native.h"
#include "CollisionVector_native.h"

#include "General.h"
#include "CutsceneMgr.h"
#include "Directory.h"
#include "Camera.h"
#include "Streaming.h"
#include "FileMgr.h"
#include "main.h"
#include "AnimManager.h"
#include "AnimBlendAssociation.h"
#include "AnimBlendAssocGroup.h"
#include "AnimBlendClumpData.h"
#include "Pad.h"
#include "DMAudio.h"
#include "World.h"
#include "PlayerPed.h"
#include "Wanted.h"
#include "RpAnimBlend.h"
#include "ModelIndices.h"
#include "TempColModels.h"
#include "ColStore.h"
#include "Radar.h"
#include "Pools.h"

const struct {
	const char *szTrackName;
	int iTrackId;
} musicNameIdAssoc[] = {
	{ "ASS_1", STREAMED_SOUND_CUTSCENE_ASS_1 },
	{ "ASS_2", STREAMED_SOUND_CUTSCENE_ASS_2 },
	{ "BANK_1", STREAMED_SOUND_CUTSCENE_BANK_1 },
	{ "BANK_2A", STREAMED_SOUND_CUTSCENE_BANK_2A },
	{ "BANK_2B", STREAMED_SOUND_CUTSCENE_BANK_2B },
	{ "BANK_3A", STREAMED_SOUND_CUTSCENE_BANK_3A },
	{ "BANK_3B", STREAMED_SOUND_CUTSCENE_BANK_3B },
	{ "BANK_4", STREAMED_SOUND_CUTSCENE_BANK_4 },
	{ "BIKE_1", STREAMED_SOUND_CUTSCENE_BIKE_1 },
	{ "BIKE_2", STREAMED_SOUND_CUTSCENE_BIKE_2 },
	{ "BIKE_3", STREAMED_SOUND_CUTSCENE_BIKE_3 },
	{ "BUD_1", STREAMED_SOUND_CUTSCENE_BUD_1 },
	{ "BUD_2", STREAMED_SOUND_CUTSCENE_BUD_2 },
	{ "BUD_3", STREAMED_SOUND_CUTSCENE_BUD_3 },
	{ "CAP_1", STREAMED_SOUND_CUTSCENE_CAP_1 },
	{ "CAR_1", STREAMED_SOUND_CUTSCENE_CAR_1 },
	{ "CNT_1A", STREAMED_SOUND_CUTSCENE_CNT_1A },
	{ "CNT_1B", STREAMED_SOUND_CUTSCENE_CNT_1B },
	{ "CNT_2", STREAMED_SOUND_CUTSCENE_CNT_2 },
	{ "COK_1", STREAMED_SOUND_CUTSCENE_COK_1 },
	{ "COK_2A", STREAMED_SOUND_CUTSCENE_COK_2A },
	{ "COK_2B", STREAMED_SOUND_CUTSCENE_COK_2B },
	{ "COK_3", STREAMED_SOUND_CUTSCENE_COK_3 },
	{ "COK_4A", STREAMED_SOUND_CUTSCENE_COK_4A },
	{ "COK_4A2", STREAMED_SOUND_CUTSCENE_COK_4A2 },
	{ "COK_4B", STREAMED_SOUND_CUTSCENE_COK_4B },
	{ "COL_1", STREAMED_SOUND_CUTSCENE_COL_1 },
	{ "COL_2", STREAMED_SOUND_CUTSCENE_COL_2 },
	{ "COL_3A", STREAMED_SOUND_CUTSCENE_COL_3A },
	{ "COL_4A", STREAMED_SOUND_CUTSCENE_COL_4A },
	{ "COL_5A", STREAMED_SOUND_CUTSCENE_COL_5A },
	{ "COL_5B", STREAMED_SOUND_CUTSCENE_COL_5B },
	{ "CUB_1", STREAMED_SOUND_CUTSCENE_CUB_1 },
	{ "CUB_2", STREAMED_SOUND_CUTSCENE_CUB_2 },
	{ "CUB_3", STREAMED_SOUND_CUTSCENE_CUB_3 },
	{ "CUB_4", STREAMED_SOUND_CUTSCENE_CUB_4 },
	{ "DRUG_1", STREAMED_SOUND_CUTSCENE_DRUG_1 },
	{ "FIN", STREAMED_SOUND_CUTSCENE_FIN },
	{ "FIN_2", STREAMED_SOUND_CUTSCENE_FIN2 },
	{ "FINALE", STREAMED_SOUND_CUTSCENE_FINALE },
	{ "HAT_1", STREAMED_SOUND_CUTSCENE_HAT_1 },
	{ "HAT_2", STREAMED_SOUND_CUTSCENE_HAT_2 },
	{ "HAT_3", STREAMED_SOUND_CUTSCENE_HAT_3 },
	{ "ICE_1", STREAMED_SOUND_CUTSCENE_ICE_1 },
	{ "INT_A", STREAMED_SOUND_CUTSCENE_INT_A },
	{ "INT_B", STREAMED_SOUND_CUTSCENE_INT_B },
	{ "INT_D", STREAMED_SOUND_CUTSCENE_INT_D },
	{ "INT_M", STREAMED_SOUND_CUTSCENE_INT_M },
	{ "LAW_1A", STREAMED_SOUND_CUTSCENE_LAW_1A },
	{ "LAW_1B", STREAMED_SOUND_CUTSCENE_LAW_1B },
	{ "LAW_2A", STREAMED_SOUND_CUTSCENE_LAW_2A },
	{ "LAW_2B", STREAMED_SOUND_CUTSCENE_LAW_2B },
	{ "LAW_2C", STREAMED_SOUND_CUTSCENE_LAW_2C },
	{ "LAW_3", STREAMED_SOUND_CUTSCENE_LAW_3 },
	{ "LAW_4", STREAMED_SOUND_CUTSCENE_LAW_4 },
	{ "PHIL_1", STREAMED_SOUND_CUTSCENE_PHIL_1 },
	{ "PHIL_2", STREAMED_SOUND_CUTSCENE_PHIL_2 },
	{ "PORN_1", STREAMED_SOUND_CUTSCENE_PORN_1 },
	{ "PORN_2", STREAMED_SOUND_CUTSCENE_PORN_2 },
	{ "PORN_3", STREAMED_SOUND_CUTSCENE_PORN_3 },
	{ "PORN_4", STREAMED_SOUND_CUTSCENE_PORN_4 },
	{ "RESC_1A", STREAMED_SOUND_CUTSCENE_RESC_1A },
	{ "ROK_1", STREAMED_SOUND_CUTSCENE_ROK_1 },
	{ "ROK_2", STREAMED_SOUND_CUTSCENE_ROK_2 },
	{ "ROK_3A", STREAMED_SOUND_CUTSCENE_ROK_3A },
	{ "STRIPA", STREAMED_SOUND_CUTSCENE_STRIPA },
	{ "TAX_1", STREAMED_SOUND_CUTSCENE_TAX_1 },
	{ "TEX_1", STREAMED_SOUND_CUTSCENE_TEX_1 },
	{ "TEX_2", STREAMED_SOUND_CUTSCENE_TEX_2 },
	{ "TEX_3", STREAMED_SOUND_CUTSCENE_TEX_3 },
	{ "GSPOT", STREAMED_SOUND_CUTSCENE_GLIGHT },
	{ "FIST", STREAMED_SOUND_CUTSCENE_FIST },
	{ "EL_PH1", STREAMED_SOUND_CUTSCENE_ELBURRO1_PH1 },
	{ "EL_PH2", STREAMED_SOUND_CUTSCENE_ELBURRO2_PH2 },
	{ NULL, 0 }
};

int
FindCutsceneAudioTrackId(const char *szCutsceneName)
{
	for (int i = 0; musicNameIdAssoc[i].szTrackName; i++) {
		if (!General_faststricmp(musicNameIdAssoc[i].szTrackName, szCutsceneName))
			return musicNameIdAssoc[i].iTrackId;
	}
	return -1;
}

CDirectory *CutsceneMgr_ms_pCutsceneDir;
CCutsceneObject *CutsceneMgr_ms_pCutsceneObjects[NUMCUTSCENEOBJECTS];
int32 CutsceneMgr_ms_numCutsceneObjs;
bool CutsceneMgr_ms_loaded;
bool CutsceneMgr_ms_animLoaded;
bool CutsceneMgr_ms_useLodMultiplier;
char CutsceneMgr_ms_cutsceneName[CUTSCENENAMESIZE];
CAnimBlendAssocGroup CutsceneMgr_ms_cutsceneAssociations=AnimGroup_InitStaticCutscene(&CutsceneMgr_ms_cutsceneAssociations);
CVector CutsceneMgr_ms_cutsceneOffset;
float CutsceneMgr_ms_cutsceneTimer;
bool CutsceneMgr_ms_wasCutsceneSkipped;
uint32 CutsceneMgr_ms_cutsceneLoadStatus;
bool CutsceneMgr_ms_useCutsceneShadows = true;

bool bCamLoaded;
bool bIsEverythingRemovedFromTheWorldForTheBiggestFuckoffCutsceneEver; // pls don't shrink the name :P
int32 NumberOfSavedWeapons;
eWeaponType SavedWeaponIDs[TOTAL_WEAPON_SLOTS];
int32 SavedWeaponAmmo[TOTAL_WEAPON_SLOTS];
char uncompressedAnims[8][32];
uint32 numUncompressedAnims;


RpAtomic *
CalculateBoundingSphereRadiusCB(RpAtomic *atomic, void *data)
{
	float radius = RpAtomicGetBoundingSphere(atomic)->radius;
	RwV3d center = RpAtomicGetBoundingSphere(atomic)->center;

	for (RwFrame *frame = RpAtomicGetFrame(atomic); RwFrameGetParent(frame); frame = RwFrameGetParent(frame))
		RwV3dTransformPoints(&center, &center, 1, RwFrameGetMatrix(frame));

	float size = RwV3dLength(&center) + radius;
	if (size > *(float *)data)
		*(float *)data = size;
	return atomic;
}

void
CutsceneMgr_Initialise(void)
{
	CutsceneMgr_ms_numCutsceneObjs = 0;
	CutsceneMgr_ms_loaded = false;
	CutsceneMgr_ms_wasCutsceneSkipped = false;
	Cutscene_running = false; // rouz edit (ChatGPT)
	CutsceneMgr_ms_useLodMultiplier = false;
	CutsceneMgr_ms_animLoaded = false;
	Cutscene_processing = false; // rouz edit (ChatGPT)

	// Allocate the cutscene directory without invoking C++ new.
	CutsceneMgr_ms_pCutsceneDir = (CDirectory*)malloc(sizeof(CDirectory));
	assert(CutsceneMgr_ms_pCutsceneDir);
	Directory_Initialize(CutsceneMgr_ms_pCutsceneDir, CUTSCENEDIRSIZE);
	Directory_ReadDirFile(CutsceneMgr_ms_pCutsceneDir, "ANIM\\CUTS.DIR");

	numUncompressedAnims = 0;
	uncompressedAnims[0][0] = '\0';
}

void
CutsceneMgr_Shutdown(void)
{
	// Destroy and release the cutscene directory without invoking C++ delete.
	if(CutsceneMgr_ms_pCutsceneDir){
		Directory_Shutdown(CutsceneMgr_ms_pCutsceneDir);
		free(CutsceneMgr_ms_pCutsceneDir);
		CutsceneMgr_ms_pCutsceneDir = nil;
	}
}

void
CutsceneMgr_LoadCutsceneData(const char *szCutsceneName)
{
	int file;
	uint32 size;
	uint32 offset;
	CPlayerPed *pPlayerPed;

	Cutscene_processing = true; // rouz edit (ChatGPT)
	CutsceneMgr_ms_wasCutsceneSkipped = false;
	Timer_Suspend();
	if (!bIsEverythingRemovedFromTheWorldForTheBiggestFuckoffCutsceneEver)
		Streaming_RemoveCurrentZonesModels(); // rouz edit (ChatGPT)

	CutsceneMgr_ms_pCutsceneDir->numEntries = 0;
	Directory_ReadDirFile(CutsceneMgr_ms_pCutsceneDir, "ANIM\\CUTS.DIR");

	Streaming_RemoveUnusedModelsInLoadedList();
	Game_DrasticTidyUpMemory(true);

	strcpy(CutsceneMgr_ms_cutsceneName, szCutsceneName);

	RwStream *stream;
	stream = RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, "ANIM\\CUTS.IMG");
	assert(stream);

	// Load animations
	sprintf(gString, "%s.IFP", szCutsceneName);
	if (Directory_FindItem(CutsceneMgr_ms_pCutsceneDir, gString, &offset, &size)) {
		Streaming_MakeSpaceFor(size << 11);
		Streaming_ImGonnaUseStreamingMemory();
		RwStreamSkip(stream,  offset << 11);
		// Use the complete C animation manager at the original call boundary
		AnimManager_LoadAnimStream(stream, true, uncompressedAnims);
		// Use complete C group ownership at the original native call boundary
		AnimGroup_CreateForModels(&CutsceneMgr_ms_cutsceneAssociations,szCutsceneName);
		Streaming_IHaveUsedStreamingMemory();
		CutsceneMgr_ms_animLoaded = true;
	} else {
		CutsceneMgr_ms_animLoaded = false;
	}
	RwStreamClose(stream, nil);

	// Load camera data
	file = FileMgr_OpenFile("ANIM\\CUTS.IMG", "rb");
	sprintf(gString, "%s.DAT", szCutsceneName);
	if (Directory_FindItem(CutsceneMgr_ms_pCutsceneDir, gString, &offset, &size)) {
		Streaming_ImGonnaUseStreamingMemory();
		FileMgr_Seek(file, offset << 11, SEEK_SET);
		TheCamera.LoadPathSplines(file);
		Streaming_IHaveUsedStreamingMemory();
		bCamLoaded = true;
	} else {
		bCamLoaded = false;
	}

	FileMgr_CloseFile(file);

	if (General_faststricmp(CutsceneMgr_ms_cutsceneName, "finale")) {
		DMAudio_ChangeMusicMode(MUSICMODE_CUTSCENE);
		int trackId = FindCutsceneAudioTrackId(szCutsceneName);
		if (trackId != -1) {
			printf("Start preload audio %s\n", szCutsceneName);
			DMAudio_PreloadCutSceneMusic(trackId);
			printf("End preload audio %s\n", szCutsceneName);
		}
	}

	CutsceneMgr_ms_cutsceneTimer = 0.0f;
	CutsceneMgr_ms_loaded = true;
	CutsceneMgr_ms_cutsceneOffset = CVector(0.0f, 0.0f, 0.0f);

	pPlayerPed = FindPlayerPed();
	Wanted_ClearQdCrimes(pPlayerPed->m_pWanted);
	pPlayerPed->bIsVisible = false;
	pPlayerPed->m_fCurrentStamina = pPlayerPed->m_fMaxStamina;
	CPad::GetPad(0)->SetDisablePlayerControls(PLAYERCONTROL_CUTSCENE);
	World_Players[World_PlayerInFocus].MakePlayerSafe(true);

	Timer_Resume();
}

void
CutsceneMgr_FinishCutscene()
{
	CutsceneMgr_ms_wasCutsceneSkipped = true;
	if (bCamLoaded) {
		CutsceneMgr_ms_cutsceneTimer = TheCamera.GetCutSceneFinishTime() * 0.001f;
		TheCamera.FinishCutscene();
	}

	FindPlayerPed()->bIsVisible = true;
	World_Players[World_PlayerInFocus].MakePlayerSafe(false);
}

void
CutsceneMgr_SetupCutsceneToStart(void)
{
	if (bCamLoaded) {
		TheCamera.SetCamCutSceneOffSet(CutsceneMgr_ms_cutsceneOffset);
		TheCamera.TakeControlWithSpline(JUMP_CUT);
		TheCamera.SetWideScreenOn();
	}

	CutsceneMgr_ms_cutsceneOffset.z++;

	for (int i = CutsceneMgr_ms_numCutsceneObjs - 1; i >= 0; i--) {
		assert(RwObjectGetType(CutsceneMgr_ms_pCutsceneObjects[i]->m_rwObject) == rpCLUMP);
		if (CAnimBlendAssociation *pAnimBlendAssoc = RpAnimBlendClumpGetFirstAssociation((RpClump*)CutsceneMgr_ms_pCutsceneObjects[i]->m_rwObject)) {
			// Use the complete C sequence at the original native operation boundary
			assert(AnimSequence_HasTranslation(&pAnimBlendAssoc->hierarchy->sequences[0]));
			if (CutsceneMgr_ms_pCutsceneObjects[i]->m_pAttachTo != nil) {
				pAnimBlendAssoc->flags &= (~ASSOC_HAS_TRANSLATION);
			} else {
				// Use the complete C hierarchy at the original native operation boundary
				if (AnimHierarchy_IsCompressed(pAnimBlendAssoc->hierarchy)){
					// Use the complete C sequence at the original native operation boundary
					KeyFrameTransCompressed *keyFrames = ((KeyFrameTransCompressed*)AnimSequence_GetKeyFrameCompressed(&pAnimBlendAssoc->hierarchy->sequences[0],0));
					CVector trans;
					// Use the C keyframe operation and its actual embedded base fields
					KeyFrameTransCompressed_GetTranslation(keyFrames,KeyFrame_VectorOut(trans));
					CutsceneMgr_ms_pCutsceneObjects[i]->SetPosition(CutsceneMgr_ms_cutsceneOffset + trans);
				}else{
					// Use the complete C sequence at the original native operation boundary
					KeyFrameTrans *keyFrames = ((KeyFrameTrans*)AnimSequence_GetKeyFrame(&pAnimBlendAssoc->hierarchy->sequences[0],0));
					// Use the C keyframe operation and its actual embedded base fields
					CutsceneMgr_ms_pCutsceneObjects[i]->SetPosition(CutsceneMgr_ms_cutsceneOffset + KeyFrame_ToNative(keyFrames->translation));
				}
			}
			// Use the complete C association at the original native operation boundary
			AnimAssociation_SetRun(pAnimBlendAssoc);
		} else {
			CutsceneMgr_ms_pCutsceneObjects[i]->SetPosition(CutsceneMgr_ms_cutsceneOffset);
		}
		World_Add(CutsceneMgr_ms_pCutsceneObjects[i]);
		if (RwObjectGetType(CutsceneMgr_ms_pCutsceneObjects[i]->m_rwObject) == rpCLUMP) {
			CutsceneMgr_ms_pCutsceneObjects[i]->UpdateRpHAnim();
		}
	}

	Timer_Update();
	Timer_Update();
	Cutscene_running = true; // rouz edit (ChatGPT)
	CutsceneMgr_ms_cutsceneTimer = 0.0f;
}

void
CutsceneMgr_SetCutsceneAnim(const char *animName, CObject *pObject)
{
	CAnimBlendAssociation *pNewAnim;
	CAnimBlendClumpData *pAnimBlendClumpData;

	assert(RwObjectGetType(pObject->m_rwObject) == rpCLUMP);
	debug("Give cutscene anim %s\n", animName);
	RpAnimBlendClumpRemoveAllAssociations((RpClump*)pObject->m_rwObject);

	// Use complete C group ownership at the original native call boundary
	pNewAnim = AnimGroup_GetByName(&CutsceneMgr_ms_cutsceneAssociations,animName);
	if (!pNewAnim) {
		debug("\n\nHaven't I told you I can't find the fucking animation %s\n\n\n", animName);
		return;
	}

	// Use the complete C hierarchy at the original native operation boundary
	if (AnimHierarchy_IsCompressed(pNewAnim->hierarchy))
		pNewAnim->hierarchy->keepCompressed = true;

	Streaming_ImGonnaUseStreamingMemory();
	// Use complete C group ownership at the original native call boundary
	pNewAnim = AnimGroup_CopyByName(&CutsceneMgr_ms_cutsceneAssociations,animName);
	Streaming_IHaveUsedStreamingMemory();

	// Use the complete C association at the original native operation boundary
	AnimAssociation_SetCurrentTime(pNewAnim,0.0f);
	pNewAnim->flags |= ASSOC_HAS_TRANSLATION;
	pNewAnim->flags &= ~ASSOC_RUNNING;

	pAnimBlendClumpData = *RPANIMBLENDCLUMPDATA(pObject->m_rwObject);
	// Use the C animation owner at the original native operation boundary
	AnimBlendLink_Prepend(&pAnimBlendClumpData->link,&pNewAnim->link);

	if (pNewAnim->hierarchy->keepCompressed)
		// Use the C animation frame while retaining the original live SDK objects
		pAnimBlendClumpData->frames->flag |= ANIM_FRAME_COMPRESSED;
}

void
CutsceneMgr_SetCutsceneAnimToLoop(const char* animName)
{
	// Use complete C group ownership at the original native call boundary
	AnimGroup_GetByName(&CutsceneMgr_ms_cutsceneAssociations,animName)->flags |= ASSOC_REPEAT;
}

CCutsceneHead *
CutsceneMgr_AddCutsceneHead(CObject *pObject, int modelId)
{
	return nil;
}

void UpdateCutsceneObjectBoundingBox(RpClump* clump, int modelId)
{
	if (modelId >= MI_CUTOBJ01 && modelId <= MI_CUTOBJ05) {
		CColModel* pColModel = &TempColModels_ms_colModelCutObj[modelId - MI_CUTOBJ01];
		float radius = 0.0f;
		RpClumpForAllAtomics(clump, CalculateBoundingSphereRadiusCB, &radius);
		pColModel->boundingSphere.radius = radius;
		pColModel->boundingBox.min = CollisionVector_FromNative(CVector(-radius, -radius, -radius));
		pColModel->boundingBox.max = CollisionVector_FromNative(CVector(radius, radius, radius));
	}
}

CCutsceneObject *
CutsceneMgr_CreateCutsceneObject(int modelId)
{
	CBaseModelInfo *pModelInfo;
	CColModel *pColModel;
	CCutsceneObject *pCutsceneObject;

	Streaming_ImGonnaUseStreamingMemory();
	debug("Created cutscene object %s\n", ((CBaseModelInfo*)ModelRegistry_Get(modelId))->GetModelName()); // rouz edit (ChatGPT)
	if (modelId >= MI_CUTOBJ01 && modelId <= MI_CUTOBJ05) {
		pModelInfo = ((CBaseModelInfo*)ModelRegistry_Get(modelId)); // rouz edit (ChatGPT)
		pColModel = &TempColModels_ms_colModelCutObj[modelId - MI_CUTOBJ01];
		pModelInfo->SetColModel(pColModel);
		UpdateCutsceneObjectBoundingBox((RpClump*)pModelInfo->GetRwObject(), modelId);
	} else if (modelId >= MI_SPECIAL01 && modelId <= MI_SPECIAL21) {
		pModelInfo = ((CBaseModelInfo*)ModelRegistry_Get(modelId)); // rouz edit (ChatGPT)
		if (pModelInfo->GetColModel() == &TempColModels_ms_colModelPed1) {
			// Construct the cutscene collision model in pool storage without invoking C++ new.
						// Access raw storage through the C store or pool API
			CColModel *colModel = ((CColModel*)CPool_New(Pools_GetColModelPool()));
						assert(colModel);
			// Invoke the complete C collision owner at the original native lifetime or operation boundary
			ColModel_Init(colModel);
			colModel->boundingSphere.radius = 2.0f;
			colModel->boundingSphere.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.0f));
			pModelInfo->SetColModel(colModel, true);
		}
		pColModel = pModelInfo->GetColModel();
		float radius = 2.0f;
		pColModel->boundingSphere.radius = radius;
		pColModel->boundingBox.min = CollisionVector_FromNative(CVector(-radius, -radius, -radius));
		pColModel->boundingBox.max = CollisionVector_FromNative(CVector(radius, radius, radius));
	}

	// Allocate a cutscene object from the object pool without invoking C++ new.
		// Access raw storage through the C store or pool API
	pCutsceneObject = (CCutsceneObject*)((CObject*)CPool_New(Pools_GetObjectPool()));
		assert(pCutsceneObject);
	std::allocator<CCutsceneObject>().construct(pCutsceneObject);
	pCutsceneObject->SetModelIndex(modelId);
	if (CutsceneMgr_ms_useCutsceneShadows)
		pCutsceneObject->CreateShadow();
	CutsceneMgr_ms_pCutsceneObjects[CutsceneMgr_ms_numCutsceneObjs++] = pCutsceneObject;
	Streaming_IHaveUsedStreamingMemory();
	return pCutsceneObject;
}

void
CutsceneMgr_DeleteCutsceneData(void)
{
	if (!CutsceneMgr_ms_loaded) return;
	Timer_Suspend();

	Cutscene_processing = false; // rouz edit (ChatGPT)
	CutsceneMgr_ms_useLodMultiplier = false;
	CutsceneMgr_ms_useCutsceneShadows = true;

	for (--CutsceneMgr_ms_numCutsceneObjs; CutsceneMgr_ms_numCutsceneObjs >= 0; CutsceneMgr_ms_numCutsceneObjs--) {
		World_Remove(CutsceneMgr_ms_pCutsceneObjects[CutsceneMgr_ms_numCutsceneObjs]);
		CutsceneMgr_ms_pCutsceneObjects[CutsceneMgr_ms_numCutsceneObjs]->DeleteRwObject();
		// Destroy and release the cutscene object without invoking C++ delete.
		CutsceneMgr_ms_pCutsceneObjects[CutsceneMgr_ms_numCutsceneObjs]->~CCutsceneObject();
				// Access raw storage through the C store or pool API
		CPool_Delete(Pools_GetObjectPool(), CutsceneMgr_ms_pCutsceneObjects[CutsceneMgr_ms_numCutsceneObjs]);
				CutsceneMgr_ms_pCutsceneObjects[CutsceneMgr_ms_numCutsceneObjs] = nil;
	}
	CutsceneMgr_ms_numCutsceneObjs = 0;

	for (int i = MI_SPECIAL01; i < MI_SPECIAL21; i++) {
		CBaseModelInfo *minfo = ((CBaseModelInfo*)ModelRegistry_Get(i)); // rouz edit (ChatGPT)
		CColModel *colModel = minfo->GetColModel();
		if (colModel != &TempColModels_ms_colModelPed1) {
			// Destroy and release the cutscene collision model without invoking C++ delete.
			// Invoke the complete C collision owner at the original native lifetime or operation boundary
			ColModel_Destroy(colModel);
						// Access raw storage through the C store or pool API
			CPool_Delete(Pools_GetColModelPool(), colModel);
						minfo->SetColModel(&TempColModels_ms_colModelPed1);
		}
	}

	if (CutsceneMgr_ms_animLoaded)
		// Use the complete C animation manager at the original call boundary
		AnimManager_RemoveLastAnimFile();

	CutsceneMgr_ms_animLoaded = false;
	numUncompressedAnims = 0;
	uncompressedAnims[0][0] = '\0';

	if (bCamLoaded) {
		TheCamera.RestoreWithJumpCut();
		TheCamera.SetWideScreenOff();
		TheCamera.DeleteCutSceneCamDataMemory();
	}
	Cutscene_running = false; // rouz edit (ChatGPT)
	CutsceneMgr_ms_loaded = false;

	FindPlayerPed()->bIsVisible = true;
	CPad::GetPad(0)->SetEnablePlayerControls(PLAYERCONTROL_CUTSCENE);
	World_Players[World_PlayerInFocus].MakePlayerSafe(false);

	if (General_faststricmp(CutsceneMgr_ms_cutsceneName, "finale")) {
		DMAudio_StopCutSceneMusic();
		DMAudio_ChangeMusicMode(MUSICMODE_GAME);
	}

	Streaming_ms_disableStreaming = false;
	World_bProcessCutsceneOnly = false;

	if(bCamLoaded)
		Game_DrasticTidyUpMemory(TheCamera.GetScreenFadeStatus() == FADE_2);
	
	CPad::GetPad(0)->Clear(false);
	if (bIsEverythingRemovedFromTheWorldForTheBiggestFuckoffCutsceneEver) {
		Streaming_LoadInitialPeds();
		Streaming_LoadInitialWeapons();
		Streaming_LoadInitialVehicles();
		bIsEverythingRemovedFromTheWorldForTheBiggestFuckoffCutsceneEver = false;
		
		CPlayerPed *pPlayerPed = FindPlayerPed();
		for (int i = 0; i < NumberOfSavedWeapons; i++) {
			int32 weaponModelId = WeaponInfo_GetWeaponInfo(SavedWeaponIDs[i])->m_nModelId;
			uint8 flags = Streaming_modelInfo[weaponModelId].m_flags; // rouz edit (ChatGPT)
			Streaming_RequestModel(weaponModelId, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
			Streaming_LoadAllRequestedModels(false);
			if (WeaponInfo_GetWeaponInfo(SavedWeaponIDs[i])->m_nModel2Id != -1) {
				Streaming_RequestModel(WeaponInfo_GetWeaponInfo(SavedWeaponIDs[i])->m_nModel2Id, 0); // rouz edit (ChatGPT)
				Streaming_LoadAllRequestedModels(false);
			}
			if (!(flags & STREAMFLAGS_DONT_REMOVE))
				Streaming_SetModelIsDeletable(weaponModelId); // rouz edit (ChatGPT)
			pPlayerPed->GiveWeapon(SavedWeaponIDs[i], SavedWeaponAmmo[i], true);
		}
		NumberOfSavedWeapons = 0;
	}

	Timer_Resume();
}

void
CutsceneMgr_Update(void)
{
	enum {
		CUTSCENE_LOADING_0 = 0,
		CUTSCENE_LOADING_AUDIO,
		CUTSCENE_LOADING_2,
		CUTSCENE_LOADING_3,
		CUTSCENE_LOADING_4
	};

	switch (CutsceneMgr_ms_cutsceneLoadStatus) {
	case CUTSCENE_LOADING_AUDIO:
		CutsceneMgr_SetupCutsceneToStart();
		if (General_faststricmp(CutsceneMgr_ms_cutsceneName, "finale"))
			DMAudio_PlayPreloadedCutSceneMusic();
		CutsceneMgr_ms_cutsceneLoadStatus++;
		break;
	case CUTSCENE_LOADING_2:
	case CUTSCENE_LOADING_3:
		CutsceneMgr_ms_cutsceneLoadStatus++;
		break;
	case CUTSCENE_LOADING_4:
		CutsceneMgr_ms_cutsceneLoadStatus = CUTSCENE_LOADING_0;
		break;
	default:
		break;
	}

	if (!Cutscene_running) return; // rouz edit (ChatGPT)

	CutsceneMgr_ms_cutsceneTimer += Timer_GetTimeStepNonClippedInSeconds();

	for (int i = 0; i < CutsceneMgr_ms_numCutsceneObjs; i++) {
		int modelId = CutsceneMgr_ms_pCutsceneObjects[i]->GetModelIndex();
		if (modelId >= MI_CUTOBJ01 && modelId <= MI_CUTOBJ05)
			UpdateCutsceneObjectBoundingBox(CutsceneMgr_ms_pCutsceneObjects[i]->GetClump(), modelId);

		if (CutsceneMgr_ms_pCutsceneObjects[i]->m_pAttachTo != nil && modelId >= MI_SPECIAL01 && modelId <= MI_SPECIAL21)
			UpdateCutsceneObjectBoundingBox(CutsceneMgr_ms_pCutsceneObjects[i]->GetClump(), modelId);
	}

	if (bCamLoaded)
		if (General_faststricmp(CutsceneMgr_ms_cutsceneName, "finale") && TheCamera.Cams[TheCamera.ActiveCam].Mode == CCam::MODE_FLYBY && CutsceneMgr_ms_cutsceneLoadStatus == CUTSCENE_LOADING_0) {
			if (CPad::GetPad(0)->GetCrossJustDown()
				|| (Game_playingIntro && CPad::GetPad(0)->GetStartJustDown())
				|| Input_GetLeftMouseJustDown() // rouz edit (ChatGPT)
				|| Input_GetEnterJustDown() // rouz edit (ChatGPT)
				|| Input_GetCharJustDown(' ')) // rouz edit (ChatGPT)
					CutsceneMgr_FinishCutscene();
		}
}

bool CutsceneMgr_HasCutsceneFinished(void) { return !bCamLoaded || TheCamera.GetPositionAlongSpline() == 1.0f; }

void
CutsceneMgr_LoadAnimationUncompressed(char const* name)
{
	strcpy(uncompressedAnims[numUncompressedAnims], name);
	
	// Because that's how CAnimManager knows the end of array
	++numUncompressedAnims;
	assert(numUncompressedAnims < ARRAY_SIZE(uncompressedAnims));
	uncompressedAnims[numUncompressedAnims][0] = '\0';
}

void
CutsceneMgr_AttachObjectToParent(CObject *pObject, CEntity *pAttachTo)
{
	((CCutsceneObject*)pObject)->m_pAttachmentObject = nil;
	((CCutsceneObject*)pObject)->m_pAttachTo = RpClumpGetFrame(pAttachTo->GetClump());

	debug("Attach %s to %s\n", ((CBaseModelInfo*)ModelRegistry_Get(pObject->GetModelIndex()))->GetModelName(), ((CBaseModelInfo*)ModelRegistry_Get(pAttachTo->GetModelIndex()))->GetModelName()); // rouz edit (ChatGPT)
}

void
CutsceneMgr_AttachObjectToFrame(CObject *pObject, CEntity *pAttachTo, const char *frame)
{
	((CCutsceneObject*)pObject)->m_pAttachmentObject = nil;
	((CCutsceneObject*)pObject)->m_pAttachTo = RpAnimBlendClumpFindFrame(pAttachTo->GetClump(), frame)->frame;
	debug("Attach %s to component %s of %s\n",
		((CBaseModelInfo*)ModelRegistry_Get(pObject->GetModelIndex()))->GetModelName(), // rouz edit (ChatGPT)
		frame,
		((CBaseModelInfo*)ModelRegistry_Get(pAttachTo->GetModelIndex()))->GetModelName()); // rouz edit (ChatGPT)
	if (RwObjectGetType(pObject->m_rwObject) == rpCLUMP) {
		RpClump *clump = (RpClump*)pObject->m_rwObject;
		if (IsClumpSkinned(clump))
			// Use the C helper and retain the original typed SDK result
			RpAtomicGetBoundingSphere(((RpAtomic*)GetFirstAtomic(clump)))->radius *= 1.1f;
	}
}

void
CutsceneMgr_AttachObjectToBone(CObject *pObject, CObject *pAttachTo, int bone)
{
	// Use the C helper and retain the original typed SDK result
	RpHAnimHierarchy *hanim = ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump(pAttachTo->GetClump()));
	RwInt32 id = RpHAnimIDGetIndex(hanim, bone);
	RwMatrix *matrixArray = RpHAnimHierarchyGetMatrixArray(hanim);
	((CCutsceneObject*)pObject)->m_pAttachmentObject = pAttachTo;
	((CCutsceneObject*)pObject)->m_pAttachTo = &matrixArray[id];
	debug("Attach %s to %s\n",
		((CBaseModelInfo*)ModelRegistry_Get(pObject->GetModelIndex()))->GetModelName(), // rouz edit (ChatGPT)
		((CBaseModelInfo*)ModelRegistry_Get(pAttachTo->GetModelIndex()))->GetModelName()); // rouz edit (ChatGPT)
}

void
CutsceneMgr_RemoveEverythingFromTheWorldForTheBiggestFuckoffCutsceneEver()
{
	Streaming_ms_disableStreaming = true;
	ColStore_RemoveAllCollision();
	World_bProcessCutsceneOnly = true;
	Cutscene_processing = true; // rouz edit (ChatGPT)

		// Access raw storage through the C store or pool API
	for (int i = CPool_GetSize(Pools_GetPedPool()) - 1; i >= 0; i--) {
		// Access raw storage through the C store or pool API
		CPed *pPed = ((CPed*)CPool_GetSlot(Pools_GetPedPool(), i));
			if (pPed) {
			if (!pPed->IsPlayer() && pPed->CanBeDeleted()) {
				World_Remove(pPed);
				// Destroy and release the cutscene-cleared ped without invoking C++ delete.
				pPed->~CPed();
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetPedPool(), pPed);
							}
		}
	}

		// Access raw storage through the C store or pool API
	for (int i = CPool_GetSize(Pools_GetVehiclePool()) - 1; i >= 0; i--) {
		// Access raw storage through the C store or pool API
		CVehicle *pVehicle = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), i));
			if (pVehicle) {
			if (pVehicle->CanBeDeleted()) {
				World_Remove(pVehicle);
				// Destroy and release the cutscene-cleared vehicle without invoking C++ delete.
				pVehicle->~CVehicle();
								// Access raw storage through the C store or pool API
				CPool_Delete(Pools_GetVehiclePool(), pVehicle);
							}
		}
	}

	bIsEverythingRemovedFromTheWorldForTheBiggestFuckoffCutsceneEver = true;
	Streaming_RemoveCurrentZonesModels(); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_MALE01); // rouz edit (ChatGPT)
	Streaming_SetModelTxdIsDeletable(MI_MALE01); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_TAXI_D); // rouz edit (ChatGPT)
	Streaming_SetModelTxdIsDeletable(MI_TAXI_D); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_NIGHTSTICK); // rouz edit (ChatGPT)
	Streaming_SetModelTxdIsDeletable(MI_NIGHTSTICK); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_MISSILE); // rouz edit (ChatGPT)
	Streaming_SetModelTxdIsDeletable(MI_MISSILE); // rouz edit (ChatGPT)
	Streaming_SetModelIsDeletable(MI_POLICE); // rouz edit (ChatGPT)
	Streaming_SetModelTxdIsDeletable(MI_POLICE); // rouz edit (ChatGPT)

	while (Streaming_RemoveLoadedVehicle()) ; // rouz edit (ChatGPT)

	Radar_RemoveRadarSections();

		// Access raw storage through the C store or pool API
	for (int i = CPool_GetSize(Pools_GetDummyPool()) - 1; i >= 0; i--) {
		// Access raw storage through the C store or pool API
		CDummy* pDummy = ((CDummy*)CPool_GetSlot(Pools_GetDummyPool(), i));
			if (pDummy)
			pDummy->DeleteRwObject();
	}

		// Access raw storage through the C store or pool API
	for (int i = CPool_GetSize(Pools_GetObjectPool()) - 1; i >= 0; i--) {
		// Access raw storage through the C store or pool API
		CObject* pObject = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), i));
			if (pObject)
			pObject->DeleteRwObject();
	}

		// Access raw storage through the C store or pool API
	for (int i = CPool_GetSize(Pools_GetBuildingPool()) - 1; i >= 0; i--) {
		// Access raw storage through the C store or pool API
		CBuilding* pBuilding = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), i));
			if (pBuilding && pBuilding->m_rwObject != nil && pBuilding->bIsBIGBuilding && pBuilding->bStreamBIGBuilding) {
			if (pBuilding->bIsBIGBuilding)
				Streaming_RequestModel(pBuilding->GetModelIndex(), 0); // rouz edit (ChatGPT)
			if (!pBuilding->bImBeingRendered)
				pBuilding->DeleteRwObject();
		}
	}

	CPlayerPed *pPlayerPed = FindPlayerPed();
	pPlayerPed->RemoveWeaponAnims(0, -1000.0f);
	NumberOfSavedWeapons = 0;

	for (int i = 0; i < TOTAL_WEAPON_SLOTS; i++) {
		if (pPlayerPed->m_weapons[i].m_eWeaponType != WEAPONTYPE_UNARMED) {
			SavedWeaponIDs[NumberOfSavedWeapons] = pPlayerPed->m_weapons[i].m_eWeaponType;
			SavedWeaponAmmo[NumberOfSavedWeapons] = pPlayerPed->m_weapons[i].m_nAmmoTotal;
			NumberOfSavedWeapons++;
		}
	}

	pPlayerPed->ClearWeapons();
	Game_DrasticTidyUpMemory(true);
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
