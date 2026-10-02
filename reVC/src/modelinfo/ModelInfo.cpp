#include "common.h"

#include "General.h"
#include "TempColModels.h"
#include "ModelIndices.h"
#include "ModelInfo.h"

CBaseModelInfo *CModelInfo::ms_modelInfoPtrs[MODELINFOSIZE];

//+ rouz edit (ChatGPT)
static CSimpleModelInfo simpleModelEntries[SIMPLEMODELSIZE];
CStore CModelInfo::ms_simpleModelStore = { 0, simpleModelEntries, sizeof(CSimpleModelInfo), SIMPLEMODELSIZE };
static CTimeModelInfo timeModelEntries[TIMEMODELSIZE];
CStore CModelInfo::ms_timeModelStore = { 0, timeModelEntries, sizeof(CTimeModelInfo), TIMEMODELSIZE };
static CWeaponModelInfo weaponModelEntries[WEAPONMODELSIZE];
CStore CModelInfo::ms_weaponModelStore = { 0, weaponModelEntries, sizeof(CWeaponModelInfo), WEAPONMODELSIZE };
static CClumpModelInfo clumpModelEntries[CLUMPMODELSIZE];
CStore CModelInfo::ms_clumpModelStore = { 0, clumpModelEntries, sizeof(CClumpModelInfo), CLUMPMODELSIZE };
static CPedModelInfo pedModelEntries[PEDMODELSIZE];
CStore CModelInfo::ms_pedModelStore = { 0, pedModelEntries, sizeof(CPedModelInfo), PEDMODELSIZE };
static CVehicleModelInfo vehicleModelEntries[VEHICLEMODELSIZE];
CStore CModelInfo::ms_vehicleModelStore = { 0, vehicleModelEntries, sizeof(CVehicleModelInfo), VEHICLEMODELSIZE };
static C2dEffect effectEntries[TWODFXSIZE];
CStore CModelInfo::ms_2dEffectStore = { 0, effectEntries, sizeof(C2dEffect), TWODFXSIZE };
//- rouz edit (ChatGPT)

void
CModelInfo::Initialise(void)
{
	int i;
	CSimpleModelInfo *m;

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	debug("sizeof SimpleModelStore %d\n", sizeof(simpleModelEntries));
	debug("sizeof TimeModelStore %d\n", sizeof(timeModelEntries));
	debug("sizeof WeaponModelStore %d\n", sizeof(weaponModelEntries));
	debug("sizeof ClumpModelStore %d\n", sizeof(clumpModelEntries));
	debug("sizeof VehicleModelStore %d\n", sizeof(vehicleModelEntries));
	debug("sizeof PedModelStore %d\n", sizeof(pedModelEntries));
	debug("sizeof 2deffectsModelStore %d\n", sizeof(effectEntries));
	//- rouz edit (ChatGPT)

	for(i = 0; i < MODELINFOSIZE; i++)
		ms_modelInfoPtrs[i] = nil;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CStore_Clear(&ms_2dEffectStore);
	CStore_Clear(&ms_simpleModelStore);
	CStore_Clear(&ms_timeModelStore);
	CStore_Clear(&ms_weaponModelStore);
	CStore_Clear(&ms_clumpModelStore);
	CStore_Clear(&ms_pedModelStore);
	CStore_Clear(&ms_vehicleModelStore);
	//- rouz edit (ChatGPT)

	m = AddSimpleModel(MI_CAR_DOOR);
	m->SetColModel(&CTempColModels::ms_colModelDoor1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_BUMPER);
	m->SetColModel(&CTempColModels::ms_colModelBumper1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_PANEL);
	m->SetColModel(&CTempColModels::ms_colModelPanel1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_BONNET);
	m->SetColModel(&CTempColModels::ms_colModelBonnet1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_BOOT);
	m->SetColModel(&CTempColModels::ms_colModelBoot1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_CAR_WHEEL);
	m->SetColModel(&CTempColModels::ms_colModelWheel1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_BODYPARTA);
	m->SetColModel(&CTempColModels::ms_colModelBodyPart1);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;

	m = AddSimpleModel(MI_BODYPARTB);
	m->SetColModel(&CTempColModels::ms_colModelBodyPart2);
	m->SetTexDictionary("generic");
	m->SetNumAtomics(1);
	m->m_lodDistances[0] = 80.0f;
}

void
CModelInfo::ShutDown(void)
{
	int i;
	for(i = 0; i < ms_simpleModelStore.allocPtr; i++)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		simpleModelEntries[i].Shutdown();
		//- rouz edit (ChatGPT)
	for(i = 0; i < ms_timeModelStore.allocPtr; i++)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		timeModelEntries[i].Shutdown();
		//- rouz edit (ChatGPT)
	for(i = 0; i < ms_weaponModelStore.allocPtr; i++)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		weaponModelEntries[i].Shutdown();
		//- rouz edit (ChatGPT)
	for(i = 0; i < ms_clumpModelStore.allocPtr; i++)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		clumpModelEntries[i].Shutdown();
		//- rouz edit (ChatGPT)
	for(i = 0; i < ms_vehicleModelStore.allocPtr; i++)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		vehicleModelEntries[i].Shutdown();
		//- rouz edit (ChatGPT)
	for(i = 0; i < ms_pedModelStore.allocPtr; i++)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		pedModelEntries[i].Shutdown();
		//- rouz edit (ChatGPT)
	for(i = 0; i < ms_2dEffectStore.allocPtr; i++)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		effectEntries[i].Shutdown();
		//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CStore_Clear(&ms_2dEffectStore);
	CStore_Clear(&ms_simpleModelStore);
	CStore_Clear(&ms_timeModelStore);
	CStore_Clear(&ms_weaponModelStore);
	CStore_Clear(&ms_pedModelStore);
	CStore_Clear(&ms_clumpModelStore);
	CStore_Clear(&ms_vehicleModelStore);
	//- rouz edit (ChatGPT)
}

CSimpleModelInfo*
CModelInfo::AddSimpleModel(int id)
{
	CSimpleModelInfo *modelinfo;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	modelinfo = (CSimpleModelInfo*)CStore_Alloc(&CModelInfo::ms_simpleModelStore);
	//- rouz edit (ChatGPT)
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->Init();
	return modelinfo;
}

CTimeModelInfo*
CModelInfo::AddTimeModel(int id)
{
	CTimeModelInfo *modelinfo;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	modelinfo = (CTimeModelInfo*)CStore_Alloc(&CModelInfo::ms_timeModelStore);
	//- rouz edit (ChatGPT)
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->Init();
	return modelinfo;
}

CWeaponModelInfo*
CModelInfo::AddWeaponModel(int id)
{
	CWeaponModelInfo *modelinfo;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	modelinfo = (CWeaponModelInfo*)CStore_Alloc(&CModelInfo::ms_weaponModelStore);
	//- rouz edit (ChatGPT)
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->Init();
	return modelinfo;
}

CClumpModelInfo*
CModelInfo::AddClumpModel(int id)
{
	CClumpModelInfo *modelinfo;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	modelinfo = (CClumpModelInfo*)CStore_Alloc(&CModelInfo::ms_clumpModelStore);
	//- rouz edit (ChatGPT)
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	return modelinfo;
}

CPedModelInfo*
CModelInfo::AddPedModel(int id)
{
	CPedModelInfo *modelinfo;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	modelinfo = (CPedModelInfo*)CStore_Alloc(&CModelInfo::ms_pedModelStore);
	//- rouz edit (ChatGPT)
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	return modelinfo;
}

CVehicleModelInfo*
CModelInfo::AddVehicleModel(int id)
{
	CVehicleModelInfo *modelinfo;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	modelinfo = (CVehicleModelInfo*)CStore_Alloc(&CModelInfo::ms_vehicleModelStore);
	//- rouz edit (ChatGPT)
	CModelInfo::ms_modelInfoPtrs[id] = modelinfo;
	modelinfo->m_clump = nil;
	modelinfo->m_vehicleType = -1;
	modelinfo->m_wheelId = -1;
	modelinfo->m_materials1[0] = nil;
	modelinfo->m_materials2[0] = nil;
	modelinfo->m_bikeSteerAngle = 999.99f;
	return modelinfo;
}

CBaseModelInfo*
CModelInfo::GetModelInfo(const char *name, int *id)
{
	CBaseModelInfo *modelinfo;
	for(int i = 0; i < MODELINFOSIZE; i++){
		modelinfo = CModelInfo::ms_modelInfoPtrs[i];
	 	if(modelinfo && !CGeneral::faststricmp(modelinfo->GetModelName(), name)){
			if(id)
				*id = i;
			return modelinfo;
		}
	}
	return nil;
}

CBaseModelInfo*
CModelInfo::GetModelInfo(const char *name, int minIndex, int maxIndex)
{
	if (minIndex > maxIndex)
		return 0;

	CBaseModelInfo *modelinfo;
	for(int i = minIndex; i <= maxIndex; i++){
		modelinfo = CModelInfo::ms_modelInfoPtrs[i];
	 	if(modelinfo && !CGeneral::faststricmp(modelinfo->GetModelName(), name))
			return modelinfo;
	}
	return nil;
}

bool
CModelInfo::IsBoatModel(int32 id)
{
	return GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
		((CVehicleModelInfo*)GetModelInfo(id))->m_vehicleType == VEHICLE_TYPE_BOAT;
}

bool
CModelInfo::IsBikeModel(int32 id)
{
	return GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
		((CVehicleModelInfo*)GetModelInfo(id))->m_vehicleType == VEHICLE_TYPE_BIKE;
}

bool
CModelInfo::IsCarModel(int32 id)
{
	return GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
		((CVehicleModelInfo*)GetModelInfo(id))->m_vehicleType == VEHICLE_TYPE_CAR;
}

bool
CModelInfo::IsHeliModel(int32 id)
{
	return GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
		((CVehicleModelInfo*)GetModelInfo(id))->m_vehicleType == VEHICLE_TYPE_HELI;
}

bool
CModelInfo::IsPlaneModel(int32 id)
{
	return GetModelInfo(id)->GetModelType() == MITYPE_VEHICLE &&
		((CVehicleModelInfo*)GetModelInfo(id))->m_vehicleType == VEHICLE_TYPE_PLANE;
}

void
CModelInfo::ReInit2dEffects()
{
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CStore_Clear(&ms_2dEffectStore);
	//- rouz edit (ChatGPT)

	for (int i = 0; i < MODELINFOSIZE; i++) {
		if (ms_modelInfoPtrs[i])
			ms_modelInfoPtrs[i]->Init2dEffects();
	}
}
