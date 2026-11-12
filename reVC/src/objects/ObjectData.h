//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

typedef struct CObjectInfo
{
	float m_fMass;
	float m_fTurnMass;
	float m_fAirResistance;
	float m_fElasticity;
	float m_fBuoyancy;
	float m_fUprootLimit;
	float m_fCollisionDamageMultiplier;
	uint8_t m_nCollisionDamageEffect;
	uint8_t m_nSpecialCollisionResponseCases;
	bool m_bCameraToAvoidThisObject;
} CObjectInfo;

#ifdef __cplusplus
VALIDATE_SIZE(CObjectInfo, 0x20);

extern "C" {
#endif
void ObjectData_Initialise(const char *filename);
const CObjectInfo *ObjectData_Info(int id);
// Native model access used by the C reader
void ObjectData_BackendSetModelObjectID(void *model, int16_t id);
int16_t ObjectData_BackendGetModelObjectID(int32_t modelId);
#ifdef __cplusplus
}

class CObject;

class CObjectData
{
public:
	static void Initialise(const char *filename) { ObjectData_Initialise(filename); }
	static void SetObjectData(int32 modelId, CObject &object);
};
#endif
//- rouz edit (Claude)
