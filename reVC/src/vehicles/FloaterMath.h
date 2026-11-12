//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "VectorMath.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum FloaterBoatKind {
	FLOATER_BOAT_PREDATOR,	// also the default
	FLOATER_BOAT_SPEEDER,
	FLOATER_BOAT_REEFER,
	FLOATER_BOAT_RIO,
	FLOATER_BOAT_SQUALO,
	FLOATER_BOAT_TROPIC,
	FLOATER_BOAT_SKIMMER,
	FLOATER_BOAT_COASTG,
	FLOATER_BOAT_DINGHY,
	FLOATER_BOAT_MARQUIS,
	FLOATER_BOAT_JETMAX,
} FloaterBoatKind;

typedef struct FloaterState {
	VectorMath3 m_position;
	float m_matrix[4][4];
	int m_field_54;
	VectorMath3 m_positionZ;
	float m_waterlevel;
	float m_waterLevelInc;
	float m_buoyancy;
	VectorMath3 m_dimMax;
	VectorMath3 m_dimMin;
	float m_numPartialVolumes;
	int m_field_8C;
	int m_field_90;
	int m_field_94;
	bool m_haveVolume;
	VectorMath3 m_step;
	VectorMath3 m_stepRatio;
	float m_numSteps;
	bool m_flipAverage;
	char m_field_B9;
	bool m_isBoat;
	float m_volumeUnderWater;
	VectorMath3 m_impulsePoint;
} FloaterState;

// Water level of a cell relative to the floater's bounding box
typedef enum FloaterWaterLevel {
	FLOATER_C_ABOVE_WATER,
	FLOATER_C_IN_WATER,
	FLOATER_C_UNDER_WATER,
} FloaterWaterLevel;

// Set up the bounding dimensions, step sizes and per-frame accumulators
void Floater_PreCalcSetup(FloaterState *s, VectorMath3 boxMin, VectorMath3 boxMax, bool isBoat, FloaterBoatKind kind, VectorMath3 position, float buoyancy);
// Clamp a local water height to the bounding box and classify it
FloaterWaterLevel Floater_ClampWaterLevel(const FloaterState *s, float *waterZ);
// Local offset of a cell rotated into world orientation
VectorMath3 Floater_RotateOffset(const FloaterState *s, VectorMath3 local);
// Accumulate one cell's submerged volume and return it
float Floater_SumBuoyancyData(FloaterState *s, VectorMath3 waterLevel, float volMultiplier);
// Normalise the accumulated volume by the box volume
void Floater_NormaliseVolume(FloaterState *s);
// Produce the buoyancy force point and impulse, returns false when nothing is submerged
bool Floater_CalcForce(const FloaterState *s, float timeStep, VectorMath3 *point, VectorMath3 *impulse);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
