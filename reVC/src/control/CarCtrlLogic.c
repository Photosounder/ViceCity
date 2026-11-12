//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "CarCtrlLogic.h"

CVehicleStruct *CarCtrl_FindClosestRemovableCar(const CPool *vehiclePool, float cx, float cy, float cz, CarCtrlVehicleTest isInteresting, CarCtrlVehicleTest isTargetedByCrane)
{
	int i = CPool_GetSize(vehiclePool);
	float md = 10000000.0f;
	CVehicleStruct *pClosestVehicle = NULL;
	while (i--) {
		CVehicleStruct *pVehicle = (CVehicleStruct *)CPool_GetSlot(vehiclePool, i);
		float dx, dy, dz, distance;
		if (!pVehicle)
			continue;
		if (isInteresting(pVehicle) || pVehicle->vehicle.bIsLocked)
			continue;
		if (!VehicleLogic_CanBeDeleted(&pVehicle->vehicle) || isTargetedByCrane(pVehicle))
			continue;
		// Distance from the point to the vehicle, the first strictly closer one wins
		dx = cx - pVehicle->base.base.placeable.m_matrix.px;
		dy = cy - pVehicle->base.base.placeable.m_matrix.py;
		dz = cz - pVehicle->base.base.placeable.m_matrix.pz;
		distance = sqrtf(dx * dx + dy * dy + dz * dz);
		if (distance < md) {
			md = distance;
			pClosestVehicle = pVehicle;
		}
	}
	return pClosestVehicle;
}

int32_t CarCtrl_CountCarsOfType(const CPool *vehiclePool, int32_t modelIndex)
{
	int32_t total = 0;
	int i;
	for (i = CPool_GetSize(vehiclePool) - 1; i >= 0; i--) {
		CVehicleStruct *pVehicle = (CVehicleStruct *)CPool_GetSlot(vehiclePool, i);
		if (!pVehicle)
			continue;
		if (pVehicle->base.base.entity.m_modelIndex == modelIndex)
			total++;
	}
	return total;
}

void CarCtrl_GetRandomOffsetForVehicle(const CVehicleStruct *vehicle, bool next, float out[3])
{
	int32_t seed = ((next ? vehicle->vehicle.AutoPilot.m_nNextPathNodeInfo : vehicle->vehicle.AutoPilot.m_nCurrentPathNodeInfo) + vehicle->base.base.entity.m_randomSeed) & 7;
	out[0] = (seed - 3) * 0.009f;
	out[1] = ((seed >> 3) - 3) * 0.009f;
	out[2] = 0.0f;
}
float CarCtrl_FindSpeedMultiplier(float limitedAngleChange, float minAngle, float maxAngle, float coef)
{
	float angle = fabsf(limitedAngleChange);
	float n = angle - minAngle;
	float d;
	float mult;

	// Linear between the two angles
	n = n > 0.0f ? n : 0.0f;
	d = maxAngle - minAngle;
	mult = 1.0f - n / d * (1.0f - coef);
	if (n > d)
		return coef;
	return mult;
}

float CarCtrl_FindSpeedMultiplierWithSpeedFromNodes(int type)
{
	// Faster road node types
	switch (type) {
	case 1: return 1.5f;
	case 2: return 2.0f;
	}
	return 1.0f;
}

int CarCtrl_ChooseRatingFromThresholds(int rnd, const int16_t *thresholds, int numClasses, int firstRating, bool returnIndex)
{
	int i;

	// The first class whose threshold is above the random number, the last one otherwise
	for (i = 0; i < numClasses - 1; i++) {
		if (rnd < thresholds[i])
			return returnIndex ? i : firstRating + i;
	}
	return firstRating + numClasses - 1;
}
void CarCtrl_AddToLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi, int32_t freq)
{
	// Append the model, its sum includes the sum of the model before it
	loadedRow[*numLoaded] = mi;
	freqRow[*numLoaded] = freq;
	if (*numLoaded)
		freqRow[*numLoaded] += freqRow[*numLoaded - 1];
	(*numLoaded)++;
}

void CarCtrl_RemoveFromLoadedVehicleArray(int32_t *loadedRow, int32_t *freqRow, int32_t *numLoaded, int32_t mi)
{
	int index = 0;
	int32_t freq;

	// Find the model
	while (loadedRow[index] != -1) {
		if (loadedRow[index] == mi)
			break;
		index++;
	}

	// Its own frequency is its sum without the one before
	freq = freqRow[index];
	if (index > 0)
		freq -= freqRow[index - 1];

	// Close the gap and lower the sums of the models that followed
	while (loadedRow[index + 1] != -1) {
		loadedRow[index] = loadedRow[index + 1];
		freqRow[index] = freqRow[index + 1] - freq;
		index++;
	}
	--(*numLoaded);
}

int32_t CarCtrl_ChooseCarModel(const int32_t *loadedRow, const int32_t *freqRow, int32_t numLoaded, int32_t (*randomRange)(int32_t, int32_t))
{
	int32_t rnd, index = 0;

	if (numLoaded == 0)
		return -1;

	// The first model whose sum reaches the random number
	rnd = randomRange(0, freqRow[numLoaded - 1]);
	while (rnd > freqRow[index])
		index++;
	return loadedRow[index];
}

bool CarCtrl_IsInPointerList(void *const *list, int count, const void *item)
{
	int i;
	for (i = 0; i < count; i++) {
		if (list[i] == item)
			return true;
	}
	return false;
}

void CarCtrl_RemoveFromPointerList(void **list, int count, const void *item)
{
	int i;
	for (i = 0; i < count; i++) {
		if (list[i] == item)
			list[i] = NULL;
	}
}
bool CarCtrl_AdjustTrafficSpeed(float maxSpeed, float curSpeed, float timeStep, float *newSpeed)
{
	if (maxSpeed >= curSpeed) {
		// Speed up by at most 0.05 per step
		if (maxSpeed > curSpeed) {
			float limit = curSpeed + 0.05f * timeStep;
			*newSpeed = maxSpeed < limit ? maxSpeed : limit;
			return true;
		}
	} else if (curSpeed != 0.0f) {
		// Stop when nearly stopped, otherwise brake by 0.7 per step but not below the speed allowed
		if (curSpeed < 0.1f) {
			*newSpeed = 0.0f;
		} else {
			float limit = curSpeed - 0.7f * timeStep;
			*newSpeed = maxSpeed > limit ? maxSpeed : limit;
		}
		return true;
	}
	return false;
}
//- rouz edit (Claude)
