//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct MotionBlurStreakVector { float x,y,z; } MotionBlurStreakVector;
typedef struct MotionBlurStreak {
    uintptr_t m_id;
    uint8_t m_red,m_green,m_blue;
    MotionBlurStreakVector m_pos1[3],m_pos2[3];
    bool m_isValid[3];
} MotionBlurStreak;
#ifdef __cplusplus
extern "C" {
#endif
void MotionBlurStreak_Update(MotionBlurStreak *streak);
void MotionBlurStreak_Render(MotionBlurStreak *streak);
void MotionBlurStreaks_InitGeometry(void);
void MotionBlurStreaks_Init(void);
void MotionBlurStreaks_Update(void);
void MotionBlurStreaks_RegisterStreak(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,MotionBlurStreakVector p1,MotionBlurStreakVector p2);
bool MotionBlurStreaks_IsRegisteredForCurrentFrame(uintptr_t id);
void MotionBlurStreaks_Render(void);
#ifdef __cplusplus
}
class CVector;
MotionBlurStreakVector MotionBlurStreaks_FromVector(CVector value);
void MotionBlurStreaks_RegisterStreakLegacy(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,CVector p1,CVector p2);
#endif
//- rouz edit (ChatGPT)
