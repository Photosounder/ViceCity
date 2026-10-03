//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "DMAudio.h"

void
AudioCollisionReport_Report(cAudioManager *manager, CEntity *entity1, CEntity *entity2, uint8_t surface1, uint8_t surface2, float collisionPower, float velocity)
{
    // Preserve rejection gates, game position access and queue insertion order
	float distSquared;
	AudioSoundPosition v1;
	AudioSoundPosition v2;

	// Reject unavailable audio and collisions below both intensity thresholds
	if(!manager->m_bIsInitialised || manager->m_nCollisionEntity < 0 || manager->m_bIsPaused ||
	   (velocity < 0.0016f && collisionPower < 0.01f))
		return;

	// Select the original building shortcut before copying entity positions
	if(AudioCollisionReportHost_IsBuilding(entity1)) {
		AudioCollisionReportHost_Position(entity2, &v2);
		v1 = v2;
	} else if(AudioCollisionReportHost_IsBuilding(entity2)) {
		AudioCollisionReportHost_Position(entity1, &v2);
		v1 = v2;
	} else {
		AudioCollisionReportHost_Position(entity1, &v1);
		AudioCollisionReportHost_Position(entity2, &v2);
	}
	// Retain float rounding between vector addition and midpoint scaling
	const AudioSoundPosition sum = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
	const AudioSoundPosition pos = {sum.x * 0.5f, sum.y * 0.5f, sum.z * 0.5f};
	// Queue nearby collisions with the original squared camera distance
	distSquared = AudioGeometry_DistanceSquared(&pos);
	if(distSquared < (COLLISION_MAX_DIST * COLLISION_MAX_DIST)) {
		manager->m_sCollisionManager.m_sQueue.m_pEntity1 = entity1;
		manager->m_sCollisionManager.m_sQueue.m_pEntity2 = entity2;
		manager->m_sCollisionManager.m_sQueue.m_bSurface1 = surface1;
		manager->m_sCollisionManager.m_sQueue.m_bSurface2 = surface2;
		manager->m_sCollisionManager.m_sQueue.m_fIntensity1 = collisionPower;
		manager->m_sCollisionManager.m_sQueue.m_fIntensity2 = velocity;
		// Copy the game vector components into the C collision record
		manager->m_sCollisionManager.m_sQueue.m_vecPosition.x = pos.x;
		manager->m_sCollisionManager.m_sQueue.m_vecPosition.y = pos.y;
		manager->m_sCollisionManager.m_sQueue.m_vecPosition.z = pos.z;
		manager->m_sCollisionManager.m_sQueue.m_fDistance = distSquared;
		AudioCollisionManager_AddCollisionToRequestedQueue(&manager->m_sCollisionManager);
	}
}

void
DMAudio_ReportCollision(CEntity *entityA, CEntity *entityB, uint8_t surfaceTypeA, uint8_t surfaceTypeB, float collisionPower, float velocity)
{
	// Preserve the original game audio operation through the C interface
	AudioCollisionReport_Report(&AudioManager, entityA, entityB, surfaceTypeA, surfaceTypeB, collisionPower, velocity);
}
//- rouz edit (ChatGPT)
