//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include "Vector.h"
typedef CVector CarGenVec3;
#else
typedef struct CarGenVec3 {float x,y,z;} CarGenVec3;
#endif

// State of one parked car generator, saved as is in save games
typedef struct CCarGeneratorData
{
	int32_t m_nModelIndex;
	CarGenVec3 m_vecPos;
	float m_fAngle;
	int16_t m_nColor1;
	int16_t m_nColor2;
	uint8_t m_bForceSpawn;
	uint8_t m_nAlarm;
	uint8_t m_nDoorlock;
	int16_t m_nMinDelay;
	int16_t m_nMaxDelay;
	uint32_t m_nTimer;
	int32_t m_nVehicleHandle;
	uint16_t m_nUsesRemaining;
	bool m_bIsBlocking;
} CCarGeneratorData;
//- rouz edit (Claude)
