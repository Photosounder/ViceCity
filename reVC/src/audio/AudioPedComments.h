#pragma once
//+ rouz edit (ChatGPT)
#include <stdint.h>
#include "../core/config.h"

typedef struct PedCommentPosition {
    float x, y, z;
} PedCommentPosition;

typedef struct tPedComment {
	uint32_t m_nSampleIndex;
	int32_t m_nEntityIndex;
	PedCommentPosition m_vecPos;
	float m_fDistance;
	uint8_t m_nVolume;
	int8_t m_nLoadingTimeout; // how many iterations we gonna wait until dropping the sample if it's still not loaded (only useful on PS2)
#if defined(EXTERNAL_3D_SOUND) && defined(FIX_BUGS)
	uint8_t m_nEmittingVolume;
#endif
} tPedComment;


typedef struct cPedComments {
	tPedComment m_aPedCommentQueue[NUM_SOUND_QUEUES][NUM_PED_COMMENTS_SLOTS];
	uint8_t m_aPedCommentOrderList[NUM_SOUND_QUEUES][NUM_PED_COMMENTS_SLOTS];
	uint8_t m_nPedCommentCount[NUM_SOUND_QUEUES];
	uint8_t m_nActiveQueue;
#ifdef GTA_PC
	uint8_t m_bDelay;
	uint32_t m_nDelayTimer;
#endif

} cPedComments;

#ifdef __cplusplus
static_assert(sizeof(tPedComment) == 28, "Ped comment layout");
extern "C" {
#else
_Static_assert(sizeof(tPedComment) == 28, "Ped comment layout");
#endif

void PedComments_Init(cPedComments *comments);
void PedComments_Add(cPedComments *comments, const tPedComment *com);
void PedComments_Advance(cPedComments *comments);
void PedComments_Process(cPedComments *comments);

#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
