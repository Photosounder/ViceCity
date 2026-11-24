//+ rouz edit (Claude)
#include <math.h>
#include "FloaterMath.h"

static float Sq(float x)
{
	// Square a value
	return x * x;
}

void Floater_PreCalcSetup(FloaterState *s, VectorMath3 boxMin, VectorMath3 boxMax, bool isBoat, FloaterBoatKind kind, VectorMath3 position, float buoyancy)
{
	// Take the model's bounding box and tune it for each boat type
	s->m_isBoat = isBoat;
	s->m_dimMin = boxMin;
	s->m_dimMax = boxMax;

	if (isBoat) {
		switch (kind) {
		case FLOATER_BOAT_PREDATOR:
		default:
			s->m_dimMax.y *= 1.05f;
			s->m_dimMin.y *= 0.9f;
			break;
		case FLOATER_BOAT_SPEEDER:
			s->m_dimMax.y *= 1.25f;
			s->m_dimMin.y *= 0.83f;
			break;
		case FLOATER_BOAT_REEFER:
			s->m_dimMin.y *= 0.9f;
			break;
		case FLOATER_BOAT_RIO:
			s->m_dimMax.y *= 0.9f;
			s->m_dimMin.y *= 0.9f;
			s->m_dimMax.z += 0.25f;
			s->m_dimMin.z -= 0.2f;
			break;
		case FLOATER_BOAT_SQUALO:
			s->m_dimMax.y *= 0.9f;
			s->m_dimMin.y *= 0.9f;
			break;
		case FLOATER_BOAT_TROPIC:
			s->m_dimMax.y *= 1.3f;
			s->m_dimMin.y *= 0.82f;
			s->m_dimMin.z -= 0.2f;
			break;
		case FLOATER_BOAT_SKIMMER:
			s->m_dimMin.y = -s->m_dimMax.y;
			s->m_dimMax.y *= 1.2f;
			break;
		case FLOATER_BOAT_COASTG:
			s->m_dimMax.y *= 1.1f;
			s->m_dimMin.y *= 0.9f;
			s->m_dimMin.z -= 0.3f;
			break;
		case FLOATER_BOAT_DINGHY:
			s->m_dimMax.y *= 1.3f;
			s->m_dimMin.y *= 0.9f;
			s->m_dimMin.z -= 0.2f;
			break;
		case FLOATER_BOAT_MARQUIS:
			s->m_dimMax.y *= 1.3f;
			s->m_dimMin.y *= 0.9f;
			break;
		case FLOATER_BOAT_JETMAX:
			s->m_dimMin.y *= 0.9f;
			break;
		}
	}

	// Split the box into steps and find their ratios against the largest one
	s->m_step.x = (s->m_dimMax.x - s->m_dimMin.x) / s->m_numSteps;
	s->m_step.y = (s->m_dimMax.y - s->m_dimMin.y) / s->m_numSteps;
	s->m_step.z = (s->m_dimMax.z - s->m_dimMin.z) / s->m_numSteps;

	if (s->m_step.z > s->m_step.x && s->m_step.z > s->m_step.y) {
		s->m_stepRatio.x = s->m_step.x / s->m_step.z;
		s->m_stepRatio.y = s->m_step.y / s->m_step.z;
		s->m_stepRatio.z = 1.0f;
	} else if (s->m_step.y > s->m_step.x && s->m_step.y > s->m_step.z) {
		s->m_stepRatio.x = s->m_step.x / s->m_step.y;
		s->m_stepRatio.y = 1.0f;
		s->m_stepRatio.z = s->m_step.z / s->m_step.y;
	} else {
		s->m_stepRatio.x = 1.0f;
		s->m_stepRatio.y = s->m_step.y / s->m_step.x;
		s->m_stepRatio.z = s->m_step.z / s->m_step.x;
	}

	// Reset the accumulators for this frame
	s->m_haveVolume = false;
	s->m_numPartialVolumes = 1.0f;
	s->m_volumeUnderWater = 0.0f;
	s->m_impulsePoint.x = s->m_impulsePoint.y = s->m_impulsePoint.z = 0.0f;
	s->m_position = position;
	s->m_positionZ.x = 0.0f;
	s->m_positionZ.y = 0.0f;
	s->m_positionZ.z = position.z;
	s->m_buoyancy = buoyancy;
	s->m_waterlevel += s->m_waterLevelInc;
}

VectorMath3 Floater_RotateOffset(const FloaterState *s, VectorMath3 local)
{
	// Rotate a local offset by the floater's matrix
	return VectorMath_MatrixTimesVector3x3(s->m_matrix, local);
}

FloaterWaterLevel Floater_ClampWaterLevel(const FloaterState *s, float *waterZ)
{
	// Clamp to the box and report whether the cell is above, in or under water
	if (*waterZ > s->m_dimMax.z) {
		*waterZ = s->m_dimMax.z;
		return FLOATER_C_UNDER_WATER;
	} else if (*waterZ < s->m_dimMin.z) {
		*waterZ = s->m_dimMin.z;
		return FLOATER_C_ABOVE_WATER;
	}
	return FLOATER_C_IN_WATER;
}

float Floater_SumBuoyancyData(FloaterState *s, VectorMath3 waterLevel, float volMultiplier)
{
	// Subtract empty space from the submerged depth
	float submerged = fabsf(waterLevel.z - s->m_dimMin.z);
	float thisVolume = submerged - (1.0f - volMultiplier);
	if (thisVolume < 0.0f)
		return 0.0f;

	if (s->m_isBoat) {
		thisVolume *= volMultiplier;
		thisVolume = Sq(thisVolume);
	}

	s->m_volumeUnderWater += thisVolume;

	// Running average of the cells' centres weighted by volume
	VectorMath3 average;
	average.x = waterLevel.x * s->m_stepRatio.x;
	average.y = waterLevel.y * s->m_stepRatio.y;
	average.z = (waterLevel.z + s->m_dimMin.z) / 2.0f * s->m_stepRatio.z;

	if (s->m_flipAverage) {
		average.x = -average.x;
		average.y = -average.y;
		average.z = -average.z;
	}

	float fraction = 1.0f / s->m_numPartialVolumes;
	float remaining = 1.0f - fraction;
	s->m_impulsePoint.x = s->m_impulsePoint.x * remaining + average.x * thisVolume * fraction;
	s->m_impulsePoint.y = s->m_impulsePoint.y * remaining + average.y * thisVolume * fraction;
	s->m_impulsePoint.z = s->m_impulsePoint.z * remaining + average.z * thisVolume * fraction;
	s->m_numPartialVolumes += 1.0f;
	s->m_haveVolume = true;
	return thisVolume;
}

void Floater_NormaliseVolume(FloaterState *s)
{
	// Divide by the box volume used for the cell sampling
	s->m_volumeUnderWater /= (s->m_dimMax.z - s->m_dimMin.z) * Sq(s->m_numSteps + 1.0f);
}

bool Floater_CalcForce(const FloaterState *s, float timeStep, VectorMath3 *point, VectorMath3 *impulse)
{
	// Report the buoyancy point and an upward impulse
	if (!s->m_haveVolume)
		return false;

	*point = Floater_RotateOffset(s, s->m_impulsePoint);
	impulse->x = 0.0f;
	impulse->y = 0.0f;
	impulse->z = s->m_volumeUnderWater * s->m_buoyancy * timeStep;
	return true;
}
float Floater_DampedImpulse(float upImpulse, float speedAlongNormal, float dampingLevel)
{
	float damp = 1.0f - speedAlongNormal * dampingLevel;

	return upImpulse * (damp > 0.0f ? damp : 0.0f);
}
//- rouz edit (Claude)
