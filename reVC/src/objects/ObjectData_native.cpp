//+ rouz edit (Claude)
#include "common.h"
#include "ModelRegistry.h"
#include "ModelInfo.h"
#include "Object.h"
#include "ObjectData.h"

void ObjectData_BackendSetModelObjectID(void *model, int16_t id)
{
	// Set the object info index on the native model info
	((CBaseModelInfo*)model)->SetObjectID(id);
}

int16_t ObjectData_BackendGetModelObjectID(int32_t modelId)
{
	// Read the object info index of the native model info
	return ((CBaseModelInfo*)ModelRegistry_Get(modelId))->GetObjectID();
}

void
CObjectData::SetObjectData(int32 modelId, CObject &object)
{
	const CObjectInfo *objinfo;

	if(ObjectData_BackendGetModelObjectID(modelId) == -1)
		return;

	objinfo = ObjectData_Info(ObjectData_BackendGetModelObjectID(modelId));

	object.m_fMass = objinfo->m_fMass;
	object.m_fTurnMass = objinfo->m_fTurnMass;
	object.m_fAirResistance = objinfo->m_fAirResistance;
	object.m_fElasticity = objinfo->m_fElasticity;
	object.m_fBuoyancy = objinfo->m_fBuoyancy;
	object.m_fUprootLimit = objinfo->m_fUprootLimit;
	object.m_fCollisionDamageMultiplier = objinfo->m_fCollisionDamageMultiplier;
	object.m_nCollisionDamageEffect = objinfo->m_nCollisionDamageEffect;
	object.m_nSpecialCollisionResponseCases = objinfo->m_nSpecialCollisionResponseCases;
	object.m_bCameraToAvoidThisObject = objinfo->m_bCameraToAvoidThisObject;
	if(object.m_fMass >= 99998.0f){
		object.bInfiniteMass = true;
		object.m_phy_flagA08 = true;
		object.bAffectedByGravity = false;
		object.bExplosionProof = true;
	}
}
//- rouz edit (Claude)
