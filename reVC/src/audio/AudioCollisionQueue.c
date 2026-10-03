//+ rouz edit (ChatGPT)
#include <stddef.h>
#include <string.h>
#include "AudioCollision.h"

void
AudioCollision_Reset(cAudioCollision *collision)
{
	// Reset the original fields while preserving the stored base volume
	collision->m_pEntity1 = NULL;
	collision->m_pEntity2 = NULL;
	collision->m_bSurface1 = 0;
	collision->m_bSurface2 = 0;
	collision->m_fIntensity1 = collision->m_fIntensity2 = 0.0f;
	collision->m_vecPosition.x = 0.0f;
	collision->m_vecPosition.y = 0.0f;
	collision->m_vecPosition.z = 0.0f;
	collision->m_fDistance = 0.0f;
}

void
AudioCollisionManager_Init(cAudioCollisionManager *manager)
{
	// Initialize both collision arrays explicitly instead of invoking member constructors
	for (int i = 0; i < NUMAUDIOCOLLISIONS; i++) {
		// Reset both records and retain the original empty index sentinel
		AudioCollision_Reset(&manager->m_asCollisions1[i]);
		AudioCollision_Reset(&manager->m_asCollisions2[i]);
		manager->m_bIndicesTable[i] = NUMAUDIOCOLLISIONS;
	}
	AudioCollision_Reset(&manager->m_sQueue);
	manager->m_bCollisionsInQueue = 0;
}

void
AudioCollisionManager_AddCollisionToRequestedQueue(cAudioCollisionManager *manager)
{
	// Preserve the existing distance-based insertion and full-queue replacement rules
	uint32_t collisionsIndex;
	uint32_t i;
	if (manager->m_bCollisionsInQueue < NUMAUDIOCOLLISIONS)
		collisionsIndex = manager->m_bCollisionsInQueue++;
	else {
		// Reject a full-queue request unless it is nearer than the existing last entry
		collisionsIndex = manager->m_bIndicesTable[NUMAUDIOCOLLISIONS - 1];
		if (manager->m_sQueue.m_fDistance >= manager->m_asCollisions1[collisionsIndex].m_fDistance) return;
	}
	manager->m_asCollisions1[collisionsIndex] = manager->m_sQueue;

	// Insert the chosen slot using the original index-table traversal and byte shift
	i = 0;
	if (collisionsIndex) {
		while (manager->m_asCollisions1[manager->m_bIndicesTable[i]].m_fDistance <= manager->m_asCollisions1[collisionsIndex].m_fDistance) {
			// Retain the original stopping condition and equal-distance ordering
			if (++i >= collisionsIndex) {
				manager->m_bIndicesTable[i] = collisionsIndex;
				return;
			}
		}
		memmove(&manager->m_bIndicesTable[i + 1], &manager->m_bIndicesTable[i], NUMAUDIOCOLLISIONS - 1 - i);
	}
	manager->m_bIndicesTable[i] = collisionsIndex;
}
//- rouz edit (ChatGPT)
