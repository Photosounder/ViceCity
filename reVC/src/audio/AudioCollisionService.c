//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "SurfaceTypes.h"
#include <stddef.h>

void
AudioCollisionService_Run(cAudioManager *manager)
{
	// Preserve collision matching, history mutations and sound-generator callback order
	int i, j;
	uint8_t abRepeatedCollision1[NUMAUDIOCOLLISIONS];
	uint8_t abRepeatedCollision2[NUMAUDIOCOLLISIONS];

	manager->m_sQueueSample.m_nEntityIndex = manager->m_nCollisionEntity;

	for (int i = 0; i < NUMAUDIOCOLLISIONS; i++)
		abRepeatedCollision1[i] = abRepeatedCollision2[i] = 0;

	for (i = 0; i < manager->m_sCollisionManager.m_bCollisionsInQueue; i++) {
		for (j = 0; j < NUMAUDIOCOLLISIONS; j++) {
			int index = manager->m_sCollisionManager.m_bIndicesTable[i];
			if ((manager->m_sCollisionManager.m_asCollisions1[index].m_pEntity1 == manager->m_sCollisionManager.m_asCollisions2[j].m_pEntity1)
				&& (manager->m_sCollisionManager.m_asCollisions1[index].m_pEntity2 == manager->m_sCollisionManager.m_asCollisions2[j].m_pEntity2)
				&& (manager->m_sCollisionManager.m_asCollisions1[index].m_bSurface1 == manager->m_sCollisionManager.m_asCollisions2[j].m_bSurface1)
				&& (manager->m_sCollisionManager.m_asCollisions1[index].m_bSurface2 == manager->m_sCollisionManager.m_asCollisions2[j].m_bSurface2)
				) {
				abRepeatedCollision1[index] = 1;
				abRepeatedCollision2[j] = 1;
				manager->m_sCollisionManager.m_asCollisions1[index].m_nBaseVolume = ++manager->m_sCollisionManager.m_asCollisions2[j].m_nBaseVolume;
				// Generate the looping request from the current collision record and counter
				AudioCollisionSounds_Looping(manager, &manager->m_sCollisionManager.m_asCollisions1[index], j);
				break;
			}
		}
	}

	for (i = 0; i < NUMAUDIOCOLLISIONS; i++) {
		if (!abRepeatedCollision2[i]) {
			manager->m_sCollisionManager.m_asCollisions2[i].m_pEntity1 = NULL;
			manager->m_sCollisionManager.m_asCollisions2[i].m_pEntity2 = NULL;
			manager->m_sCollisionManager.m_asCollisions2[i].m_bSurface1 = SURFACE_DEFAULT;
			manager->m_sCollisionManager.m_asCollisions2[i].m_bSurface2 = SURFACE_DEFAULT;
			manager->m_sCollisionManager.m_asCollisions2[i].m_fIntensity2 = 0.0f;
			manager->m_sCollisionManager.m_asCollisions2[i].m_fIntensity1 = 0.0f;
			// Clear the C position components without constructing a vector
			manager->m_sCollisionManager.m_asCollisions2[i].m_vecPosition.x = 0.0f;
			manager->m_sCollisionManager.m_asCollisions2[i].m_vecPosition.y = 0.0f;
			manager->m_sCollisionManager.m_asCollisions2[i].m_vecPosition.z = 0.0f;
			manager->m_sCollisionManager.m_asCollisions2[i].m_fDistance = 0.0f;
		}
	}

	for (i = 0; i < manager->m_sCollisionManager.m_bCollisionsInQueue; i++) {
		int index = manager->m_sCollisionManager.m_bIndicesTable[i];
		if (!abRepeatedCollision1[index]) {
			for (j = 0; j < NUMAUDIOCOLLISIONS; j++) {
				if (!abRepeatedCollision2[j]) {
					manager->m_sCollisionManager.m_asCollisions2[j].m_nBaseVolume = 1;
					manager->m_sCollisionManager.m_asCollisions2[j].m_pEntity1 = manager->m_sCollisionManager.m_asCollisions1[index].m_pEntity1;
					manager->m_sCollisionManager.m_asCollisions2[j].m_pEntity2 = manager->m_sCollisionManager.m_asCollisions1[index].m_pEntity2;
					manager->m_sCollisionManager.m_asCollisions2[j].m_bSurface1 = manager->m_sCollisionManager.m_asCollisions1[index].m_bSurface1;
					manager->m_sCollisionManager.m_asCollisions2[j].m_bSurface2 = manager->m_sCollisionManager.m_asCollisions1[index].m_bSurface2;
					break;
				}
			}
			// Generate the one-shot request directly through the C collision routine
			AudioCollisionSounds_OneShot(manager, &manager->m_sCollisionManager.m_asCollisions1[index]);
			// Generate the looping request from the current collision record and counter
			AudioCollisionSounds_Looping(manager, &manager->m_sCollisionManager.m_asCollisions1[index], j);
		}
	}

	for (int i = 0; i < NUMAUDIOCOLLISIONS; i++)
		manager->m_sCollisionManager.m_bIndicesTable[i] = NUMAUDIOCOLLISIONS;
	manager->m_sCollisionManager.m_bCollisionsInQueue = 0;
}
//- rouz edit (ChatGPT)
