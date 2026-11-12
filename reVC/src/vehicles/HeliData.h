//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "HeliNodes.h"

#ifdef __cplusplus
#define HELIDATA_FRAME RwFrame
#else
#define HELIDATA_FRAME void
#endif

// The helicopter state that follows CVehicle, as plain data usable from C
typedef struct CHeliData {
	HELIDATA_FRAME *m_aHeliNodes[NUM_HELI_NODES];
	int8_t m_heliStatus;
	float m_fSearchLightX;
	float m_fSearchLightY;
	uint32_t m_nExplosionTimer;
	float m_fRotation;
	float m_fAngularSpeed;
	float m_fTargetZ;
	float m_fSearchLightIntensity;
	int8_t m_nHeliId;
	int8_t m_heliType;
	int8_t m_pathState;
	int8_t m_numSwat;
	uint8_t m_aSwatState[4];
	float m_aSearchLightHistoryX[6];
	float m_aSearchLightHistoryY[6];
	uint32_t m_nSearchLightTimer;
	uint32_t m_nShootTimer;
	uint32_t m_nLastShotTime;
	uint32_t m_nBulletDamage;
	float m_fRotorRotation;
	float m_fHeliDustZ[8];
	uint32_t m_nPoliceShoutTimer;
	float m_fTargetOffset;
	bool m_bTestRight;
} CHeliData;
//- rouz edit (Claude)
