#include "common.h"

#include "Building.h"
#include "Streaming.h"
#include "Pools.h"

void
CBuilding::ReplaceWithNewModel(int32 id)
{
	DeleteRwObject();

	if (((CBaseModelInfo*)ModelRegistry_Get(m_modelIndex))->GetNumRefs() == 0) // rouz edit (ChatGPT)
		Streaming_RemoveModel(m_modelIndex); // rouz edit (ChatGPT)
	m_modelIndex = id;

	if(bIsBIGBuilding)
		if(m_level == LEVEL_GENERIC || m_level == Game_currLevel)
			Streaming_RequestModel(id, STREAMFLAGS_DONT_REMOVE); // rouz edit (ChatGPT)
}

bool
IsBuildingPointerValid(CBuilding* pBuilding)
{
	if (!pBuilding)
		return false;
	if (pBuilding->GetIsATreadable()) {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		int index = CPool_GetJustIndex_NoFreeAssert(Pools_GetTreadablePool(), (CTreadable*)pBuilding);
		//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		return index >= 0 && index < CPool_GetSize(Pools_GetTreadablePool());
		//- rouz edit (ChatGPT)
#else
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		return index >= 0 && index <= CPool_GetSize(Pools_GetTreadablePool());
		//- rouz edit (ChatGPT)
#endif
	} else {
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		int index = CPool_GetJustIndex_NoFreeAssert(Pools_GetBuildingPool(), pBuilding);
		//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		return index >= 0 && index < CPool_GetSize(Pools_GetBuildingPool());
		//- rouz edit (ChatGPT)
#else
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		return index >= 0 && index <= CPool_GetSize(Pools_GetBuildingPool());
		//- rouz edit (ChatGPT)
#endif
	}
}
