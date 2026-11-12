//+ rouz edit (Claude)
#pragma once
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

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
