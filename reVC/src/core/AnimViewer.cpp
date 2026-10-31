//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "Font.h"
#include "Pad.h"
#include "Text.h"
#include "main.h"
#include "Timer.h"
#include "DMAudio.h"
#include "FileMgr.h"
#include "Streaming.h"
#include "TxdStore.h"
#include "General.h"
#include "Camera.h"
#include "Vehicle.h"
#include "Bike.h"
#include "PlayerSkin.h"
#include "PlayerInfo.h"
#include "World.h"
#include "Renderer.h"
#include "AnimManager.h"
#include "AnimBlendAssocGroup.h"
#include "AnimViewer.h"
#include "PlayerPed.h"
#include "Pools.h"
#include "References.h"
#include "PathFind.h"
#include "HandlingMgr.h"
#include "TempColModels.h"
#include "Particle.h"
#include "CdStream.h"
#include "Messages.h"
#include "CarCtrl.h"
#include "FileLoader.h"
#include "ModelIndices.h"
#include "Clock.h"
#include "Timecycle.h"
#include "RpAnimBlend.h"
#include "AnimBlendAssociation.h"
#include "Shadows.h"
#include "Radar.h"
#include "Hud.h"
#include "debugmenu.h"

int CAnimViewer::animTxdSlot = 0;
CEntity *CAnimViewer::pTarget = nil;

void
CAnimViewer::Render(void) {
	if (pTarget) {
		if (pTarget) {
#ifdef FIX_BUGS
			if(pTarget->IsPed())
				((CPed*)pTarget)->UpdateRpHAnim();
#endif
			pTarget->Render();
			CRenderer::RenderOneNonRoad(pTarget);
		}
	}
}

void
CAnimViewer::Initialise(void) {

	// we need messages, messages needs hud, hud needs those
	// Use the complete C texture dictionary owner with the original live arguments
	int hudSlot = TxdStore_AddTxdSlot("hud");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_LoadFile(hudSlot, "MODELS/HUD.TXD");
	CHud::m_Wants_To_Draw_Hud = false;

	// Use the complete C texture dictionary owner with the original live arguments
	animTxdSlot = TxdStore_AddTxdSlot("generic");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_Create(animTxdSlot);
	// Use the complete C texture dictionary owner with the original live arguments
	int particleSlot = TxdStore_AddTxdSlot("particle");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_LoadFile(particleSlot, "MODELS/PARTICLE.TXD");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_SetCurrentTxd(animTxdSlot);
	CPools::Initialise();
	References_Init();
	TheCamera.Init();
	TheCamera.SetRwCamera(Scene.camera);
	TheCamera.Cams[TheCamera.ActiveCam].Distance = 5.0f;
	ThePaths.Init();
	ThePaths.AllocatePathFindInfoMem(4500);
	CCollision::Init();
	CWorld::Initialise();
	mod_HandlingManager.Initialise();
	CTempColModels::Initialise();
	// Use the complete C animation manager at the original call boundary
	AnimManager_Initialise();
	CModelInfo::Initialise();
	CParticle::Initialise();
	CCarCtrl::Init();
	PedStats_Initialise();
	Messages_Init();
	CdStreamAddImage("MODELS\\GTA3.IMG");
	CFileLoader::LoadLevel("DATA\\DEFAULT.DAT");
	CFileLoader::LoadLevel("DATA\\ANIMVIEWER.DAT");
	CStreaming::Init();
	for(int i = 0; i < MODELINFOSIZE; i++)
		if(CModelInfo::GetModelInfo(i))
			CModelInfo::GetModelInfo(i)->ConvertAnimFileIndex();
	CStreaming::LoadInitialPeds();
	CStreaming::RequestSpecialModel(MI_PLAYER, "player", STREAMFLAGS_DONT_REMOVE);
	CStreaming::LoadAllRequestedModels(false);
	CRenderer::Init();
	CVehicleModelInfo::LoadVehicleColours();
#ifdef FIX_BUGS
	CVehicleModelInfo::LoadEnvironmentMaps();
#endif
	// Use the complete C animation manager at the original call boundary
	AnimManager_LoadAnimFiles();
	CWorld::PlayerInFocus = 0;
	CWeapon::InitialiseWeapons();
	CPed::Initialise();
	Timer_Initialise();
	Clock_Initialise(60000);
	TimeCycle_Initialise();
	CCarCtrl::Init();
//+ rouz edit (ChatGPT)
	// Allocate the anim viewer player without invoking C++ new.
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPlayerPed *player = (CPlayerPed*)((CPed*)CPool_New(CPools::GetPedPool()));
	//- rouz edit (ChatGPT)
	assert(player);
	std::allocator<CPlayerPed>().construct(player);
//- rouz edit (ChatGPT)
	player->SetPosition(1000.0f, 1000.0f, 1000.0f);
	CWorld::Players[0].m_pPed = player;
	Draw_SetFOV(120.0f);
	Draw_ms_fLODDistance = 500.0f;

	int fd = FileMgr_OpenFile("DATA\\SPECIAL.TXT", "r");
	char animGroup[32], modelName[32];
	if (fd) {
		for (int lineId = 0; lineId < NUM_OF_SPECIAL_CHARS; lineId++) {
			if (!FileMgr_ReadLine(fd, gString, 255))
				break;

			sscanf(gString, "%s %s", modelName, animGroup);
			int groupId;
			for (groupId = 0; groupId < NUM_ANIM_ASSOC_GROUPS; groupId++) {
				// Use the complete C animation manager at the original call boundary
				if (!strcmp(animGroup, AnimManager_GetAnimGroupName((AssocGroupId)groupId)))
					break;
			}

			if (groupId != NUM_ANIM_ASSOC_GROUPS)
				((CPedModelInfo*)CModelInfo::GetModelInfo(MI_SPECIAL01 + lineId))->m_animGroup = groupId;

			CStreaming::RequestSpecialChar(lineId, modelName, STREAMFLAGS_DONT_REMOVE);
		}
		FileMgr_CloseFile(fd);
	} else {
		// TODO? maybe request some special models here so the thing doesn't crash
	}

	// From LCS. idk if needed
	// Use the complete C animation manager at the original call boundary
	int vanBlock = AnimManager_GetAnimationBlockIndex("van");
	// Use the complete C animation manager at the original call boundary
	int bikesBlock = AnimManager_GetAnimationBlockIndex("bikes");
	// Use the complete C animation manager at the original call boundary
	int bikevBlock = AnimManager_GetAnimationBlockIndex("bikev");
	// Use the complete C animation manager at the original call boundary
	int bikehBlock = AnimManager_GetAnimationBlockIndex("bikeh");
	// Use the complete C animation manager at the original call boundary
	int bikedBlock = AnimManager_GetAnimationBlockIndex("biked");
	CStreaming::FlushRequestList();
	CStreaming::RequestAnim(vanBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(bikesBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(bikevBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(bikehBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::RequestAnim(bikedBlock, STREAMFLAGS_DEPENDENCY);
	CStreaming::LoadAllRequestedModels(false);
	// Use the complete C animation manager at the original call boundary
	AnimManager_AddAnimBlockRef(vanBlock);
	// Use the complete C animation manager at the original call boundary
	AnimManager_AddAnimBlockRef(bikesBlock);
	// Use the complete C animation manager at the original call boundary
	AnimManager_AddAnimBlockRef(bikevBlock);
	// Use the complete C animation manager at the original call boundary
	AnimManager_AddAnimBlockRef(bikehBlock);
	// Use the complete C animation manager at the original call boundary
	AnimManager_AddAnimBlockRef(bikedBlock);
}

int
LastPedModelId(int modelId)
{
	CBaseModelInfo *model;
	for(;;){
		assert(modelId < MODELINFOSIZE);
		model = CModelInfo::GetModelInfo(modelId);
		if (model && model->GetModelType() == MITYPE_PED)
			break;
		modelId--;
	}
	return modelId;
}

int
FirstCarModelId(int modelId)
{
	CBaseModelInfo *model;
	for(;;){
		assert(modelId < MODELINFOSIZE);
		model = CModelInfo::GetModelInfo(modelId);
		if (model && model->GetModelType() == MITYPE_VEHICLE)
			break;
		modelId++;
	}
	return modelId;
}


int
NextModelId(int modelId, int wantedChange)
{
	// Max. 2 trials wasn't here, it's me that added it.

	int tryCount = 2;
	int ogModelId = modelId;

	while(tryCount != 0) {
		modelId += wantedChange;
		if (modelId < 0 || modelId >= MODELINFOSIZE) {
			tryCount--;
			wantedChange = -wantedChange;
		} else if (modelId != 5 && modelId != 6 && modelId != 405) {
			CBaseModelInfo *model = CModelInfo::GetModelInfo(modelId);
			if (model)
			{
				//int type = model->m_type;
				return modelId;
			}
		}
	}
	return ogModelId;
}

void
PlayAnimation(RpClump *clump, AssocGroupId animGroup, AnimationId anim)
{
	CAnimBlendAssociation *currentAssoc = RpAnimBlendClumpGetAssociation(clump, anim);

//+ rouz edit (ChatGPT)
	// Destroy the matching partial animation without invoking C++ delete.
	// Use the complete C association at the original native operation boundary
	if (currentAssoc && AnimAssociation_IsPartial(currentAssoc)){
		// Use the complete C association at the original native operation boundary
		AnimAssociation_Destroy(currentAssoc);
		free(currentAssoc);
	}
//- rouz edit (ChatGPT)

	RpAnimBlendClumpSetBlendDeltas(clump, ASSOC_PARTIAL, -8.0f);

	// Use the complete C animation manager at the original call boundary
	CAnimBlendAssociation *animAssoc = AnimManager_BlendAnimation(clump, animGroup, anim, 8.0f);
	animAssoc->flags |= ASSOC_DELETEFADEDOUT;
	// Use the complete C association at the original native operation boundary
	AnimAssociation_SetCurrentTime(animAssoc,0.0f);
	// Use the complete C association at the original native operation boundary
	AnimAssociation_SetRun(animAssoc);
}

void
CAnimViewer::Update(void)
{
	static int modelId = 0;
	static int animId = 0;
	static bool reloadIFP = false;

	AssocGroupId animGroup = ASSOCGRP_STD;
	int nextModelId = modelId;
	CBaseModelInfo *modelInfo = CModelInfo::GetModelInfo(modelId);

	if (modelInfo->GetModelType() == MITYPE_PED) {
		int animGroup = ((CPedModelInfo*)modelInfo)->m_animGroup;

		if (animId > ANIM_STD_IDLE)
			animGroup = ASSOCGRP_STD;

		if (reloadIFP) {
			if (pTarget) {
				CWorld::Remove(pTarget);
//+ rouz edit (ChatGPT)
				// Destroy and release the anim viewer target without invoking C++ delete.
				if (pTarget->IsVehicle()) {
					pTarget->~CEntity();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(CPools::GetVehiclePool(), (CVehicle*)pTarget);
					//- rouz edit (ChatGPT)
				} else if (pTarget->IsPed()) {
					pTarget->~CEntity();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(CPools::GetPedPool(), (CPed*)pTarget);
					//- rouz edit (ChatGPT)
				} else if (pTarget->IsObject()) {
					pTarget->~CEntity();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					CPool_Delete(CPools::GetObjectPool(), (CObject*)pTarget);
					//- rouz edit (ChatGPT)
				}
//- rouz edit (ChatGPT)
			}
			pTarget = nil;
			
			// These calls were inside of LoadIFP function.
			// Use the complete C animation manager at the original call boundary
			AnimManager_Shutdown();
			// Use the complete C animation manager at the original call boundary
			AnimManager_Initialise();
			// Use the complete C animation manager at the original call boundary
			AnimManager_LoadAnimFiles();

			reloadIFP = false;
		}
	} else {
		animGroup = ASSOCGRP_STD;
	}
	CPad::UpdatePads();
	CPad* pad = CPad::GetPad(0);
#ifdef DEBUGMENU
	DebugMenuProcess();
#endif

	CStreaming::UpdateForAnimViewer();
	CStreaming::RequestModel(modelId, 0);
	if (CStreaming::HasModelLoaded(modelId)) {

		if (!pTarget) {

			if (modelInfo->GetModelType() == MITYPE_VEHICLE) {

				CVehicleModelInfo* veh = (CVehicleModelInfo*)modelInfo;
				if (veh->m_vehicleType == VEHICLE_TYPE_CAR) {
//+ rouz edit (ChatGPT)
					// Allocate the anim viewer car without invoking C++ new.
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pTarget = ((CVehicle*)CPool_New(CPools::GetVehiclePool()));
					//- rouz edit (ChatGPT)
					assert(pTarget);
					std::allocator<CAutomobile>().construct((CAutomobile*)pTarget, modelId, RANDOM_VEHICLE);
//- rouz edit (ChatGPT)
				} else if (veh->m_vehicleType == VEHICLE_TYPE_BOAT) {
//+ rouz edit (ChatGPT)
					// Allocate the anim viewer boat without invoking C++ new.
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pTarget = ((CVehicle*)CPool_New(CPools::GetVehiclePool()));
					//- rouz edit (ChatGPT)
					assert(pTarget);
					std::allocator<CBoat>().construct((CBoat*)pTarget, modelId, RANDOM_VEHICLE);
//- rouz edit (ChatGPT)
				} else if (veh->m_vehicleType == VEHICLE_TYPE_BIKE) {
//+ rouz edit (ChatGPT)
					// Allocate the anim viewer bike without invoking C++ new.
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pTarget = ((CVehicle*)CPool_New(CPools::GetVehiclePool()));
					//- rouz edit (ChatGPT)
					assert(pTarget);
					std::allocator<CBike>().construct((CBike*)pTarget, modelId, RANDOM_VEHICLE);
//- rouz edit (ChatGPT)
				} else {
//+ rouz edit (ChatGPT)
					// Allocate the anim viewer object without invoking C++ new.
					CObjectPool *objectPool = CPools::GetObjectPool();
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					pTarget = ((CObject*)CPool_New(objectPool));
					//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
					if (!pTarget) {
						//+ rouz edit (ChatGPT)
						// Access raw storage through the C store or pool API
						for (int32 i = 0; i < CPool_GetSize(objectPool); i++) {
							// Access raw storage through the C store or pool API
							CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
						//- rouz edit (ChatGPT)
							if (existing && existing->ObjectCreatedBy == TEMP_OBJECT) {
								//+ rouz edit (ChatGPT)
								// Access raw storage through the C store or pool API
								int32 handle = CPool_GetIndex(objectPool, existing);
								//- rouz edit (ChatGPT)
								CWorld::Remove(existing);
								existing->~CObject();
								//+ rouz edit (ChatGPT)
								// Access raw storage through the C store or pool API
								CPool_Delete(objectPool, existing);
								pTarget = ((CObject*)CPool_NewAt(objectPool, handle));
								//- rouz edit (ChatGPT)
								break;
							}
						}
					}
					if (pTarget)
#endif
						std::allocator<CObject>().construct((CObject*)pTarget, modelId, true);
					assert(pTarget);
//- rouz edit (ChatGPT)
					if (!modelInfo->GetColModel()) {
						modelInfo->SetColModel(&CTempColModels::ms_colModelWheel1);
					}
				}
				pTarget->SetStatus(STATUS_ABANDONED);
			} else if (modelInfo->GetModelType() == MITYPE_PED) {
//+ rouz edit (ChatGPT)
				// Allocate the anim viewer ped without invoking C++ new.
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				pTarget = ((CPed*)CPool_New(CPools::GetPedPool()));
				//- rouz edit (ChatGPT)
				assert(pTarget);
				std::allocator<CPed>().construct((CPed*)pTarget, PEDTYPE_CIVMALE);
//- rouz edit (ChatGPT)
				pTarget->SetModelIndex(modelId);
			} else {
//+ rouz edit (ChatGPT)
				// Allocate the anim viewer object without invoking C++ new.
				CObjectPool *objectPool = CPools::GetObjectPool();
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				pTarget = ((CObject*)CPool_New(objectPool));
				//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
				if (!pTarget) {
					//+ rouz edit (ChatGPT)
					// Access raw storage through the C store or pool API
					for (int32 i = 0; i < CPool_GetSize(objectPool); i++) {
						// Access raw storage through the C store or pool API
						CObject *existing = ((CObject*)CPool_GetSlot(objectPool, i));
					//- rouz edit (ChatGPT)
						if (existing && existing->ObjectCreatedBy == TEMP_OBJECT) {
							//+ rouz edit (ChatGPT)
							// Access raw storage through the C store or pool API
							int32 handle = CPool_GetIndex(objectPool, existing);
							//- rouz edit (ChatGPT)
							CWorld::Remove(existing);
							existing->~CObject();
							//+ rouz edit (ChatGPT)
							// Access raw storage through the C store or pool API
							CPool_Delete(objectPool, existing);
							pTarget = ((CObject*)CPool_NewAt(objectPool, handle));
							//- rouz edit (ChatGPT)
							break;
						}
					}
				}
				if (pTarget)
#endif
					std::allocator<CObject>().construct((CObject*)pTarget, modelId, true);
				assert(pTarget);
//- rouz edit (ChatGPT)
				if (!modelInfo->GetColModel())
				{
					modelInfo->SetColModel(&CTempColModels::ms_colModelWheel1);
				}
				pTarget->SetStatus(STATUS_ABANDONED);
			}
			pTarget->SetPosition(0.0f, 0.0f, 0.0f);
			CWorld::Add(pTarget);
			TheCamera.TakeControl(pTarget, CCam::MODE_MODELVIEW, JUMP_CUT, CAMCONTROL_SCRIPT);
		}
		if (pTarget->IsVehicle() || pTarget->IsPed() || pTarget->IsObject()) {
			((CPhysical*)pTarget)->m_vecMoveSpeed = CVector(0.0f, 0.0f, 0.0f);
		}
#ifdef FIX_BUGS
		// so we don't end up in the water
		pTarget->GetMatrix().GetPosition().z = 10.0f;
#else
		pTarget->GetMatrix().GetPosition().z = 0.0f;
#endif

		if (modelInfo->GetModelType() == MITYPE_PED) {
			((CPed*)pTarget)->bKindaStayInSamePlace = true;

			// Triangle in mobile
			if (pad->GetSquareJustDown()) {
				reloadIFP = true;
				AsciiToUnicode("IFP reloaded", gUString);
				Messages_AddMessage(gUString, 1000, 0);

			} else if (pad->GetCrossJustDown()) {
				PlayAnimation(pTarget->GetClump(), animGroup, (AnimationId)animId);
				AsciiToUnicode("Animation restarted", gUString);
				Messages_AddMessage(gUString, 1000, 0);

			} else if (pad->GetCircleJustDown()) {
				PlayAnimation(pTarget->GetClump(), animGroup, ANIM_STD_IDLE);
				AsciiToUnicode("Idle animation playing", gUString);
				Messages_AddMessage(gUString, 1000, 0);

			} else if (pad->GetDPadUpJustDown()) {
				animId--;
				if (animId < 0) {
					animId = ANIM_STD_NUM - 1;
				}
				PlayAnimation(pTarget->GetClump(), animGroup, (AnimationId)animId);

				sprintf(gString, "Current anim: %d", animId);
				AsciiToUnicode(gString, gUString);
				Messages_AddMessage(gUString, 1000, 0);

			} else if (pad->GetDPadDownJustDown()) {
				animId = (animId == (ANIM_STD_NUM - 1) ? 0 : animId + 1);
				PlayAnimation(pTarget->GetClump(), animGroup, (AnimationId)animId);

				sprintf(gString, "Current anim: %d", animId);
				AsciiToUnicode(gString, gUString);
				Messages_AddMessage(gUString, 1000, 0);

			} else if (pad->GetStartJustDown()) {

			} else if (pad->GetLeftShoulder1JustDown()) {
				nextModelId = FirstCarModelId(modelId);
				AsciiToUnicode("Switched to vehicles", gUString);
				Messages_AddMessage(gUString, 1000, 0);
				// Originally it was GetPad(1)->LeftShoulder2
			} else if (pad->NewState.Triangle) {
				((CPedModelInfo *)CModelInfo::GetModelInfo(pTarget->GetModelIndex()))->AnimatePedColModelSkinned(pTarget->GetClump());
				AsciiToUnicode("Ped Col model will be animated as long as you hold the button", gUString);
				Messages_AddMessage(gUString, 100, 0);
			}

			// From LCS
			// Use the complete C animation manager at the original call boundary
			if (AnimManager_GetAnimAssocGroups()[animGroup].numAssociations <= animId)
				animId = 0;

		} else if (modelInfo->GetModelType() == MITYPE_VEHICLE) {

			if (pad->GetLeftShoulder1JustDown()) {
				nextModelId = LastPedModelId(modelId);
				AsciiToUnicode("Switched to peds", gUString);
				Messages_AddMessage(gUString, 1000, 0);
				// Start in mobile
			} else if (pad->GetSquareJustDown()) {
				CVehicleModelInfo::LoadVehicleColours();
				AsciiToUnicode("Carcols.dat reloaded", gUString);
				Messages_AddMessage(gUString, 1000, 0);
			}
		}
	}

	if (pad->GetDPadLeftJustDown()) {
		nextModelId = NextModelId(modelId, -1);

		sprintf(gString, "Current model ID: %d", nextModelId);
		AsciiToUnicode(gString, gUString);
		Messages_AddMessage(gUString, 1000, 0);

	} else if (pad->GetDPadRightJustDown()) {
		nextModelId = NextModelId(modelId, 1);

		sprintf(gString, "Current model ID: %d", nextModelId);
		AsciiToUnicode(gString, gUString);
		Messages_AddMessage(gUString, 1000, 0);
	}
	// There were extra codes here to let us change model id by 50, but xbox CPad struct is different, so I couldn't port.

	if (nextModelId != modelId) {
		modelId = nextModelId;
		if (pTarget) {
			CWorld::Remove(pTarget);
//+ rouz edit (ChatGPT)
			// Destroy and release the anim viewer target without invoking C++ delete.
			if (pTarget->IsVehicle()) {
				pTarget->~CEntity();
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(CPools::GetVehiclePool(), (CVehicle*)pTarget);
				//- rouz edit (ChatGPT)
			} else if (pTarget->IsPed()) {
				pTarget->~CEntity();
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(CPools::GetPedPool(), (CPed*)pTarget);
				//- rouz edit (ChatGPT)
			} else if (pTarget->IsObject()) {
				pTarget->~CEntity();
				//+ rouz edit (ChatGPT)
				// Access raw storage through the C store or pool API
				CPool_Delete(CPools::GetObjectPool(), (CObject*)pTarget);
				//- rouz edit (ChatGPT)
			}
//- rouz edit (ChatGPT)
		}
		pTarget = nil;
		return;
	}

	TimeCycle_Update();
	CWorld::Process();
	if (pTarget)
		TheCamera.Process();
}

void
CAnimViewer::Shutdown(void)
{
//+ rouz edit (ChatGPT)
	if (CWorld::Players[0].m_pPed) {
		// Destroy and release the anim viewer player without invoking C++ delete.
		CWorld::Players[0].m_pPed->~CPlayerPed();
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		CPool_Delete(CPools::GetPedPool(), CWorld::Players[0].m_pPed);
		//- rouz edit (ChatGPT)
	}
//- rouz edit (ChatGPT)

	CWorld::ShutDown();
	CModelInfo::ShutDown();
	// Use the complete C animation manager at the original call boundary
	AnimManager_Shutdown();
	Timer_Shutdown();
	CStreaming::Shutdown();
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_RemoveTxdSlot(animTxdSlot);
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
