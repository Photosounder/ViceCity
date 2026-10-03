#pragma once
//+ rouz edit (ChatGPT)
#include <stddef.h>
#include <stdint.h>
#include "../core/config.h"

#ifdef __cplusplus
class CPed;
class CVehicle;
class cTransmission;
#else
typedef struct CPed CPed;
typedef struct CVehicle CVehicle;
typedef struct cTransmission cTransmission;
#endif

typedef struct cAudioScriptObjectManager {
	int32_t m_anScriptObjectEntityIndices[NUM_SCRIPT_MAX_ENTITIES];
	int32_t m_nScriptObjectEntityTotal;
} cAudioScriptObjectManager;

typedef struct cPedParams {
	uint8_t m_bDistanceCalculated;
	float m_fDistance;
	CPed *m_pPed;
} cPedParams;

typedef struct cVehicleParams {
	int32_t m_VehicleType;
	uint8_t m_bDistanceCalculated;
	float m_fDistance;
	CVehicle *m_pVehicle;
	cTransmission *m_pTransmission;
	uint32_t m_nIndex;
	float m_fVelocityChange;
} cVehicleParams;

static inline void
AudioScriptObjectManager_Reset(cAudioScriptObjectManager *manager)
{
	// Empty the active index list without changing its stored entries
	manager->m_nScriptObjectEntityTotal = 0;
}

static inline void
PedParams_Init(cPedParams *params)
{
	// Initialize the same fields as the former pedestrian context constructor
	params->m_bDistanceCalculated = 0;
	params->m_fDistance = 0.0f;
	params->m_pPed = NULL;
}

static inline void
VehicleParams_Init(cVehicleParams *params)
{
	// Initialize the same fields as the former vehicle context constructor
	params->m_VehicleType = -1;
	params->m_bDistanceCalculated = 0;
	params->m_fDistance = 0.0f;
	params->m_pVehicle = NULL;
	params->m_pTransmission = NULL;
	params->m_nIndex = 0;
	params->m_fVelocityChange = 0.0f;
}
//- rouz edit (ChatGPT)
