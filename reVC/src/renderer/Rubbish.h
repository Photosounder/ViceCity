//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CVehicle;
#else
typedef struct CVehicle CVehicle;
#endif
enum {
#ifdef SQUEEZE_PERFORMANCE
	NUM_RUBBISH_SHEETS = 32
#else
	NUM_RUBBISH_SHEETS = 64
#endif
};
typedef struct RubbishVector { float x, y, z; } RubbishVector;
typedef struct RubbishVector2 { float x, y; } RubbishVector2;
typedef struct RubbishSheet {
	RubbishVector m_basePos;
	RubbishVector m_animatedPos;
	float m_targetZ;
	int8_t m_state;
	int8_t m_animationType;
	uint32_t m_moveStart;
	uint32_t m_moveDuration;
	float m_animHeight;
	float m_xDist;
	float m_yDist;
	float m_angle;
	bool m_isVisible;
	bool m_targetIsVisible;
	struct RubbishSheet *m_next;
	struct RubbishSheet *m_prev;
} RubbishSheet;
#ifdef __cplusplus
extern "C" {
#endif
void RubbishSheet_AddToList(RubbishSheet *sheet, RubbishSheet *list);
void RubbishSheet_RemoveFromList(RubbishSheet *sheet);
void Rubbish_Render(void);
void Rubbish_StirUp(CVehicle *veh);
void Rubbish_Update(void);
void Rubbish_SetVisibility(bool visible);
void Rubbish_Init(void);
void Rubbish_Shutdown(void);
extern void *gpRubbishTexture[4];
extern uint16_t RubbishIndexList[6];
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
