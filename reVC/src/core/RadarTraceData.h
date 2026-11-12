//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include "Vector.h"
typedef CVector RadarVec3;
#else
typedef struct RadarVec3 {float x,y,z;} RadarVec3;
#endif

// The plain data of a radar blip, usable from C
typedef struct sRadarTraceData
{
	uint32_t m_nColor;
	uint32_t m_eBlipType; // eBlipType
	int32_t m_nEntityHandle;
	RadarVec3 m_vec2DPos;
	RadarVec3 m_vecPos;
	uint16_t m_BlipIndex;
	bool m_bDim;
	bool m_bInUse;
	bool m_bShortRange;
	bool m_unused;
	float m_Radius;
	int16_t m_wScale;
	uint16_t m_eBlipDisplay; // eBlipDisplay
	uint16_t m_eRadarSprite; // eRadarSprite
} sRadarTraceData;
//- rouz edit (Claude)
