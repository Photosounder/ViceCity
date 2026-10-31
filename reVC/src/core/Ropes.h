//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

typedef struct RopesVector {
	float x, y, z;
} RopesVector;
typedef struct Rope {
	bool m_bActive;
	bool m_bWasRegistered;
	bool m_unk;
	uintptr_t m_id;
	uint32_t m_updateTimer;
	RopesVector m_pos[32];
	RopesVector m_speed[32];
} Rope;
#ifdef __cplusplus
extern "C" {
#endif
void Rope_Update(Rope *rope);
void Rope_Render(Rope *rope);
void Ropes_Init(void);
void Ropes_Update(void);
void Ropes_Render(void);
bool Ropes_RegisterRope(uintptr_t id, RopesVector pos, bool setUpdateTimer);
void Ropes_SetSpeedOfTopNode(uintptr_t id, RopesVector speed);
bool Ropes_FindCoorsAlongRope(uintptr_t id, float t, RopesVector *coors);
bool Ropes_FindCoorsAlongRopeToVector(uintptr_t id, float t, void *coors);
bool Ropes_CreateRopeWithSwatComingDown(RopesVector pos);
extern uint16_t RopeIndices[64];
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
