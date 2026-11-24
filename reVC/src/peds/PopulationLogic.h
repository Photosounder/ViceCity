//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "EntityLayout.h"
#include "PedLogic.h"
#include "Pool.h"
#include "ObjectLayout.h"
#include "ObjectLogic.h"
#include "EntityLogic.h"

#ifdef __cplusplus
extern "C" {
#endif
// Pool walkers of the ped population using the C views of the peds
// Closest deletable ped to a point, walking the pool backwards, or NULL when there is none
CPedStruct *Population_FindClosestDeletablePed(const CPool *pedPool, float cx, float cy, float cz);
// Per frame object management for the slice of the pool picked by frameMod32: action 1 removes a temporary object, action 2 asks for the conversion to a dummy object, the caller applies them in order
int Population_SelectObjectActions(const CPool *objectPool, int frameMod32, float px, float py, float pz, uint32_t timeMs, const VectorMath3 normals[4], const float cameraMatrix[4][4], void **out, int *action, int maxOut);

// Slice of the dummy pool picked by the frame: dummies of the current area close to the player that must become real objects
int Population_SelectDummiesToConvert(const CPool *dummyPool, int frameMod32, float px, float py, float pz, int currentArea, void **out, int maxOut);

// Per frame ped management parameters computed by the caller
typedef struct PopulationPedParams {
	float px, py, pz;			// player position
	uint32_t timeMs;			// game time
	bool frenzyOnGoing;			// a killing frenzy is running
	float removeDistSpecial;		// distance beyond which peds flagged cull-extra-far are removed
	float removeDist;			// distance beyond which any ped is removed
	float offscreenDist;			// distance beyond which off screen peds are removed when the camera allows
	bool offscreenDespawnAllowed;		// the camera mode allows off screen despawning
	const VectorMath3 *normals;		// frustum normals of the camera
	const float (*cameraMatrix)[4];		// matrix of the camera
	int (*getClumpAlpha)(void *ped);	// alpha of the ped clump
} PopulationPedParams;
// Walks the ped pool backwards: action 1 removes a ped that finished fading out, action 3 handles a ped that is too far away
int Population_SelectPedActions(const CPool *pedPool, const PopulationPedParams *params, void **out, int *action, int maxOut);

// Radius of the circle that gang members standing next to each other fit on
float Population_CircleRadius(int pedAmount);
// Offset of the gang member number i on the circle from its two random values in -0.2 to 0.2
void Population_CircleOffset(int i, float angleRandom, float radiusRandom, int pedAmount, float circleR, float *xOffset, float *yOffset);
// Whether any of the obstacles is not one of the already created peds
bool Population_HasRealObstacle(void *const *obstacles, int numObstacles, void *const *createdPeds, int numCreated);

// Front and side vectors of the outline shadow of a dead ped drawn at a random angle: (0.9 cos, 0.9 sin) and (0.9 sin, -0.9 cos)
void Population_OutlineAxes(float angle, float axes[4]);
// Multiplier of the population from the rain: fewer peds when it rains
float Population_RainMultiplier(float rain);
// Maximum number of peds for an area, from the sum of the ped and car densities of the zone, the density of the roads, the density multiplier, the ped number multiplier (1 during a frenzy), the multiplier of the mission and the weather, limited by the maximum
float Population_MaxPedsForArea(float zoneDensitySum, float roadDensity, float densityMult, float pedNumberMult, float missionAndWeatherMult, int selectedMaxPeds);
// Height raised to the ground height when it is under it
float Population_RaisedToGround(float z, float groundZ);
// Spawn rate of a ped on a path link: the lower of the spawn rates of its two nodes
unsigned char Population_NodeSpawnRate(unsigned char rate1, unsigned char rate2);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
