#pragma once

#include "2dEffect.h"
#include "SimpleModelInfo.h"
#include "MloModelInfo.h"
#include "TimeModelInfo.h"
#include "WeaponModelInfo.h"
#include "ClumpModelInfo.h"
#include "PedModelInfo.h"
#include "VehicleModelInfo.h"
#include "templates.h"

class CModelInfo
{
	static CBaseModelInfo *ms_modelInfoPtrs[MODELINFOSIZE];
	//+ rouz edit (ChatGPT)
	static CStore ms_simpleModelStore;
	static CStore ms_timeModelStore;
	static CStore ms_weaponModelStore;
	static CStore ms_clumpModelStore;
	static CStore ms_pedModelStore;
	static CStore ms_vehicleModelStore;
	static CStore ms_2dEffectStore;
	//- rouz edit (ChatGPT)

public:
	static void Initialise(void);
	static void ShutDown(void);

	static CSimpleModelInfo *AddSimpleModel(int id);
	static CTimeModelInfo *AddTimeModel(int id);
	static CWeaponModelInfo *AddWeaponModel(int id);
	static CClumpModelInfo *AddClumpModel(int id);
	static CPedModelInfo *AddPedModel(int id);
	static CVehicleModelInfo *AddVehicleModel(int id);

	//+ rouz edit (ChatGPT)
	static CStore *Get2dEffectStore(void)
	{
		// Access the store or pool through its C API
		return &ms_2dEffectStore;
	}
	//- rouz edit (ChatGPT)

	static CBaseModelInfo *GetModelInfo(const char *name, int *id);
	static CBaseModelInfo *GetModelInfo(int id){
		return ms_modelInfoPtrs[id];
	}
	static CBaseModelInfo *GetModelInfo(const char *name, int minIndex, int maxIndex);
	static CColModel *GetColModel(int id){
		return ms_modelInfoPtrs[id]->GetColModel();
	}

	static bool IsBoatModel(int32 id);
	static bool IsBikeModel(int32 id);
	static bool IsCarModel(int32 id);
	static bool IsHeliModel(int32 id);
	static bool IsPlaneModel(int32 id);
	static void ReInit2dEffects();
};
