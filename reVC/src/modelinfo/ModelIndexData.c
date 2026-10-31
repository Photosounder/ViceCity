//+ rouz edit (ChatGPT)
#include "ModelIndices_backend.h"
#include "General.h"
#define X(name, var) int16_t var = -1;
	MODELINDICES
#undef X

void
InitModelIndices(void)
{
    // Preserve the complete original model-registry operation and assignment order
#define X(name, var) var = -1;
	MODELINDICES
#undef X
}

void
MatchModelString(const char *modelname, int16_t id)
{
    // Preserve the complete original model-registry operation and assignment order
#define X(name, var) \
	if(!General_faststrcmp(name, modelname)){ \
		var = id; \
		return; \
	}
	MODELINDICES
#undef X
}

void
TestModelIndices(void)
{
    // Preserve the complete original model-registry operation and assignment order
	;
}

bool
IsGlass(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	ModelIndexModel *mi = ModelIndices_BackendModel(id);
	return ModelIndices_BackendBuilding(mi) && (ModelIndices_BackendCodeGlass(mi) || ModelIndices_BackendArtistGlass(mi));
}

bool
IsTrafficLight(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_TRAFFICLIGHTS ||
		id == MI_TRAFFICLIGHTS_VERTICAL ||
		id == MI_TRAFFICLIGHTS_MIAMI ||
		id == MI_TRAFFICLIGHTS_TWOVERTICAL;
}

bool
IsLightWithoutShift(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_TRAFFICLIGHTS ||
		id == MI_SINGLESTREETLIGHTS1 ||
		id == MI_SINGLESTREETLIGHTS2 ||
		id == MI_SINGLESTREETLIGHTS3 ||
		id == MI_DOUBLESTREETLIGHTS;
}

bool
IsLightWithPreRenderEffects(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return IsTrafficLight(id) ||
		id == MI_SINGLESTREETLIGHTS1 ||
		id == MI_SINGLESTREETLIGHTS2 ||
		id == MI_SINGLESTREETLIGHTS3 ||
		id == MI_DOUBLESTREETLIGHTS;
}

bool
IsLightThatNeedsRepositioning(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_SINGLESTREETLIGHTS1 ||
		id == MI_SINGLESTREETLIGHTS2 ||
		id == MI_SINGLESTREETLIGHTS3 ||
		id == MI_TRAFFICLIGHTS_MIAMI ||
		id == MI_TRAFFICLIGHTS_TWOVERTICAL ||
		id == MI_MLAMPPOST ||
		id == MI_STREETLAMP1 ||
		id == MI_STREETLAMP2;
}

bool
IsLightObject(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_TRAFFICLIGHTS_MIAMI ||
		id == MI_MLAMPPOST ||
		id == MI_SINGLESTREETLIGHTS1 ||
		id == MI_SINGLESTREETLIGHTS2 ||
		id == MI_SINGLESTREETLIGHTS3 ||
		id == MI_DOUBLESTREETLIGHTS ||
		id == MI_TRAFFICLIGHTS_TWOVERTICAL;
}

bool
IsLampPost(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_SINGLESTREETLIGHTS1 ||
		id == MI_SINGLESTREETLIGHTS2 ||
		id == MI_SINGLESTREETLIGHTS3 ||
		id == MI_BOLLARDLIGHT ||
		id == MI_MLAMPPOST ||
		id == MI_STREETLAMP1 ||
		id == MI_STREETLAMP2 ||
		id == MI_TELPOLE02 ||
		id == MI_TRAFFICLIGHTS_MIAMI ||
		id == MI_TRAFFICLIGHTS_TWOVERTICAL;
}

bool
IsBodyPart(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_BODYPARTA || id == MI_BODYPARTB;
}

bool
IsPedModel(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id >= MI_PLAYER && id <= MI_LAST_PED;
}
bool
IsPalmTreeModel(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return	id == MI_VEG_PALM01 ||
		id == MI_VEG_PALM02 ||
		id == MI_VEG_PALM03 ||
		id == MI_VEG_PALM04 ||
		id == MI_VEG_PALM05 ||
		id == MI_VEG_PALM06 ||
		id == MI_VEG_PALM07 ||
		id == MI_VEG_PALM08;
}

bool
IsTreeModel(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return	id == MI_TREE2 ||
		id == MI_TREE3 ||
		id == MI_TREE6 ||
		id == MI_TREE8 ||
		IsPalmTreeModel(id);
}

bool
IsPolicePedModel(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_COP ||
		id == MI_SWAT ||
		id == MI_FBI ||
		id == MI_ARMY;
}

bool
IsPoliceVehicleModel(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_CHOPPER ||
		id == MI_PREDATOR ||
		id == MI_POLICE ||
		id == MI_ENFORCER;
}

bool
IsExplosiveThingModel(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_EXPLODINGBARREL ||
		id == MI_PETROLPUMP ||
		id == MI_PETROLPUMP2;
}

bool
IsFence(int16_t id)
{
    // Preserve the original classifier and its short-circuit read order
	return id == MI_FENCE ||
		id == MI_FENCE2;
}
//- rouz edit (ChatGPT)
