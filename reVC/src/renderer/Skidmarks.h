//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
enum { SKIDMARK_LENGTH = 16 };
typedef enum eSkidmarkType {
	SKIDMARK_NORMAL,
	SKIDMARK_MUDDY,
	SKIDMARK_SANDY,
	SKIDMARK_BLOODY
} eSkidmarkType;
typedef struct SkidmarksVector { float x, y, z; } SkidmarksVector;
typedef struct Skidmark {
	SkidmarksVector m_pos[SKIDMARK_LENGTH];
	float m_sideX[SKIDMARK_LENGTH];
	float m_sideY[SKIDMARK_LENGTH];
	uintptr_t m_id;
	uint32_t m_lastUpdate;
	uint32_t m_fadeStart;
	uint32_t m_fadeEnd;
	uint32_t m_type;
	int16_t m_last;
	uint8_t m_state;
	bool m_wasUpdated;
} Skidmark;
#ifdef __cplusplus
extern "C" {
#endif
void Skidmarks_Init(void);
void Skidmarks_Shutdown(void);
void Skidmarks_Clear(void);
void Skidmarks_Update(void);
void Skidmarks_Render(void);
void Skidmarks_RegisterOneType(uintptr_t id, const void *pos, float fwdX, float fwdY, eSkidmarkType type, bool *isBloody);
void Skidmarks_RegisterOneFlags(uintptr_t id, const void *pos, float fwdX, float fwdY, bool *isMuddy, bool *isBloody);
extern uint16_t SkidmarkIndexList[SKIDMARK_LENGTH * 6];
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
