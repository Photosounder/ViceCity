//+ rouz edit (Claude)
#include <math.h>
#include <stddef.h>
#include "PopulationLogic.h"
#include "ModelIndices.h"
#include "config.h"
#include "GameAreas.h"
#include "PedStates.h"
#include "PedType.h"

CPedStruct *Population_FindClosestDeletablePed(const CPool *pedPool, float cx, float cy, float cz)
{
	CPedStruct *closestPed = NULL;
	float closestDist = 10000000.0f;
	int poolSize = CPool_GetSize(pedPool);
	int i;

	for (i = poolSize - 1; i >= 0; i--) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, i);
		if (ped && PedLogic_CanBeDeleted(&ped->ped)) {
			// Distance from the point to the ped, the first strictly closer ped wins
			float dx = cx - ped->base.base.placeable.m_matrix.px;
			float dy = cy - ped->base.base.placeable.m_matrix.py;
			float dz = cz - ped->base.base.placeable.m_matrix.pz;
			float dist = sqrtf(dx * dx + dy * dy + dz * dz);
			if (dist < closestDist) {
				closestDist = dist;
				closestPed = ped;
			}
		}
	}
	return closestPed;
}

static float Dist3(float ax, float ay, float az, float bx, float by, float bz)
{
	// Length of the difference with the same operation order as the engine vector
	float dx = ax - bx;
	float dy = ay - by;
	float dz = az - bz;
	return sqrtf(dx * dx + dy * dy + dz * dz);
}

int Population_SelectObjectActions(const CPool *objectPool, int frameMod32, float px, float py, float pz, uint32_t timeMs, const VectorMath3 normals[4], const float cameraMatrix[4][4], void **out, int *action, int maxOut)
{
	int n = 0;
	int i;
	int objectPoolSize = CPool_GetSize(objectPool);
	for (i = objectPoolSize * frameMod32 / 32; i < objectPoolSize * (frameMod32 + 1) / 32; i++) {
		CObjectStruct *obj = (CObjectStruct *)CPool_GetSlot(objectPool, i);
		if (obj && Object_CanBeDeleted(&obj->object)) {
			const CMatrixData *m = &obj->base.base.placeable.m_matrix;
			float objPlayerDist = Dist3(m->px, m->py, m->pz, px, py, pz);
			int act = 0;
			if (obj->object.ObjectCreatedBy == TEMP_OBJECT) {
				// Temporary objects go away when far, off screen or too old
				int16_t model = obj->base.base.entity.m_modelIndex;
				if (model != MI_ROADWORKBARRIER1 && model != MI_BEACHBALL) {
					if (objPlayerDist > 51.0f || (objPlayerDist > 25.0f && !Entity_GetIsOnScreen(&obj->base.base.placeable, &obj->base.base.entity, normals, cameraMatrix)) || timeMs > obj->object.m_nEndOfLifeTime)
						act = 1;
				} else if (objPlayerDist > 120.0f)
					act = 1;
			} else if (objPlayerDist > 80.0f && Dist3(obj->object.m_objectMatrix.px, obj->object.m_objectMatrix.py, obj->object.m_objectMatrix.pz, px, py, pz) > 80.0f) {
				// Far away objects may be turned into dummy objects, the caller tests the room first
				if (obj->object.ObjectCreatedBy != CUTSCENE_OBJECT)
					act = 2;
			}
			if (act) {
				if (n < maxOut) {
					out[n] = obj;
					action[n] = act;
				}
				n++;
			}
		}
	}
	return n;
}

int Population_SelectDummiesToConvert(const CPool *dummyPool, int frameMod32, float px, float py, float pz, int currentArea, void **out, int maxOut)
{
	int n = 0;
	int i;
	int dummyPoolSize = CPool_GetSize(dummyPool);
	for (i = dummyPoolSize * frameMod32 / 32; i < dummyPoolSize * (frameMod32 + 1) / 32; i++) {
		CEntityStruct *dummy = (CEntityStruct *)CPool_GetSlot(dummyPool, i);
		if (dummy && (dummy->entity.m_area == currentArea || dummy->entity.m_area == AREA_EVERYWHERE)) {
			if (Dist3(dummy->placeable.m_matrix.px, dummy->placeable.m_matrix.py, dummy->placeable.m_matrix.pz, px, py, pz) < 80.0f) {
				if (n < maxOut)
					out[n] = dummy;
				n++;
			}
		}
	}
	return n;
}

int Population_SelectPedActions(const CPool *pedPool, const PopulationPedParams *params, void **out, int *action, int maxOut)
{
	int n = 0;
	int poolIndex;
	int pedPoolSize = CPool_GetSize(pedPool);
	for (poolIndex = pedPoolSize - 1; poolIndex >= 0; poolIndex--) {
		CPedStruct *ped = (CPedStruct *)CPool_GetSlot(pedPool, poolIndex);
		if (ped && !PedLogic_IsPlayer(&ped->ped) && PedLogic_CanBeDeleted(&ped->ped) && !ped->ped.bInVehicle) {
			const CMatrixData *m = &ped->base.base.placeable.m_matrix;
			uint32_t timeSinceDeath = params->timeMs - ped->ped.m_bloodyFootprintCountOrDeathTime;
			float dx, dy, dist;
			bool pedIsFarAway = false;

			// Dead peds fade out after a while
			if (ped->ped.m_nPedState == PED_DEAD && (timeSinceDeath > 30000 || (params->frenzyOnGoing && timeSinceDeath > 15000)))
				ped->ped.bFadeOut = true;

			// Fully faded peds are removed
			if (ped->ped.bFadeOut && params->getClumpAlpha(ped) == 0) {
				if (n < maxOut) {
					out[n] = ped;
					action[n] = 1;
				}
				n++;
				continue;
			}

			dx = m->px - params->px;
			dy = m->py - params->py;
			dist = sqrtf(dx * dx + dy * dy);

			if (PedLogic_IsGangMember(&ped->ped))
				dist -= 30.0f;
			else if (ped->ped.bDeadPedInFrontOfCar && ped->ped.m_vehicleInAccident)
				dist = 0.0f;

			if (params->removeDistSpecial < dist ||
			    (!ped->ped.bCullExtraFarAway && params->removeDist < dist)) {
				pedIsFarAway = true;
			}
#ifndef EXTENDED_OFFSCREEN_DESPAWN_RANGE
			else if (params->offscreenDist < dist) {
				if (params->timeMs > ped->ped.m_nExtendedRangeTimer && !Entity_GetIsOnScreen(&ped->base.base.placeable, &ped->base.base.entity, params->normals, params->cameraMatrix)) {
					if (params->offscreenDespawnAllowed)
						pedIsFarAway = true;
				}
			}
#endif
			else {
				ped->ped.m_nExtendedRangeTimer = ped->ped.m_nPedType == PEDTYPE_COP ? params->timeMs + 10000 : params->timeMs + 4000;
			}

			if (!pedIsFarAway)
				continue;

			if (n < maxOut) {
				out[n] = ped;
				action[n] = 3;
			}
			n++;
		}
	}
	return n;
}
float Population_CircleRadius(int pedAmount)
{
	const float pi = (float)3.14159265358979323846;
	float circleSector = (pi*2) / pedAmount;
	return sqrtf(0.5f / (1.0f - cosf(circleSector)));
}

void Population_CircleOffset(int i, float angleRandom, float radiusRandom, int pedAmount, float circleR, float *xOffset, float *yOffset)
{
	const float pi = (float)3.14159265358979323846;
	float circleSector = (pi*2) / pedAmount;
	float angleMult = i + angleRandom;
	float randomR = circleR + radiusRandom * circleR;
	*xOffset = randomR * cosf(angleMult * circleSector);
	*yOffset = randomR * sinf(angleMult * circleSector);
}

bool Population_HasRealObstacle(void *const *obstacles, int numObstacles, void *const *createdPeds, int numCreated)
{
	for (int m = 0; m < numObstacles; m++) {
		void *obstacle = obstacles[m];
		if (obstacle) {
			bool obstacleIsHarmless = false;

			// An obstacle that is one of the created peds is harmless
			for (int n = 0; n < numCreated; n++) {
				if (obstacle == createdPeds[n])
					obstacleIsHarmless = true;
			}
			if (!obstacleIsHarmless)
				return true;
		}
	}
	return false;
}

void Population_OutlineAxes(float angle, float axes[4])
{
	axes[0] = 0.9f * cosf(angle);
	axes[1] = 0.9f * sinf(angle);
	axes[2] = 0.9f * sinf(angle);
	axes[3] = -0.9f * cosf(angle);
}
float Population_RainMultiplier(float rain)
{
	return -0.8f * sqrtf(rain) + 1.0f;
}

float Population_MaxPedsForArea(float zoneDensitySum, float roadDensity, float densityMult, float pedNumberMult, float missionAndWeatherMult, int selectedMaxPeds)
{
	float v = zoneDensitySum * roadDensity * densityMult * pedNumberMult * missionAndWeatherMult;
	float m = (float)selectedMaxPeds;

	return v < m ? v : m;
}
float Population_RaisedToGround(float z, float groundZ)
{
	return z > groundZ ? z : groundZ;
}
unsigned char Population_NodeSpawnRate(unsigned char rate1, unsigned char rate2)
{
	return rate1 < rate2 ? rate1 : rate2;
}
//- rouz edit (Claude)
