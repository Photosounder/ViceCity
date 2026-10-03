#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>

#define NUMAUDIOCOLLISIONS 10

#ifdef __cplusplus
class CEntity;
#else
typedef struct CEntity CEntity;
#endif

typedef struct AudioCollisionPosition {
	float x, y, z;
} AudioCollisionPosition;

typedef struct cAudioCollision {
	CEntity *m_pEntity1;
	CEntity *m_pEntity2;
	uint8_t m_bSurface1;
	uint8_t m_bSurface2;
	float m_fIntensity1;
	float m_fIntensity2;
	AudioCollisionPosition m_vecPosition;
	float m_fDistance;
	int32_t m_nBaseVolume;
} cAudioCollision;

typedef struct cAudioCollisionManager {
	cAudioCollision m_asCollisions1[NUMAUDIOCOLLISIONS];
	cAudioCollision m_asCollisions2[NUMAUDIOCOLLISIONS];
	uint8_t m_bIndicesTable[NUMAUDIOCOLLISIONS];
	uint8_t m_bCollisionsInQueue;
	cAudioCollision m_sQueue;
} cAudioCollisionManager;

#ifdef __cplusplus
static_assert(sizeof(void *) != 4 || sizeof(cAudioCollision) == 40, "Win32 collision layout");
static_assert(sizeof(void *) != 4 || sizeof(cAudioCollisionManager) == 0x354, "Win32 collision manager layout");
extern "C" {
#else
_Static_assert(sizeof(void *) != 4 || sizeof(cAudioCollision) == 40, "Win32 collision layout");
_Static_assert(sizeof(void *) != 4 || sizeof(cAudioCollisionManager) == 0x354, "Win32 collision manager layout");
#endif

void AudioCollision_Reset(cAudioCollision *collision);
void AudioCollisionManager_Init(cAudioCollisionManager *manager);
void AudioCollisionManager_AddCollisionToRequestedQueue(cAudioCollisionManager *manager);
float AudioCollision_GetOneShotRatio(uint32_t surface, float intensity);
float AudioCollision_GetLoopingRatio(uint32_t surface1, uint32_t surface2, float intensity);
float AudioCollision_GetRatio(float intensity, float minimum, float maximum, float range);

#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
