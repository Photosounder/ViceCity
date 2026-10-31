//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct EscalatorVector { float x,y,z; } EscalatorVector;
#ifdef __cplusplus
class CObject;
#else
typedef struct CObject CObject;
#endif
typedef struct Escalator {
    EscalatorVector m_pos0,m_pos1,m_pos2,m_pos3;
    bool m_bIsActive,m_bIsMovingDown;
    int32_t m_stepsCount;
    float m_lowerEnd,m_upperEnd;
    EscalatorVector m_midPoint;
    float m_radius;
    CObject *m_pSteps[24];
} Escalator;
#ifdef __cplusplus
extern "C" {
#endif
extern Escalator Escalators_aEscalators[NUM_ESCALATORS];
extern bool deletingEscalator;
void Escalator_AddThisOne(int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool movingDown);
void Escalator_Update(int32_t index);
void Escalator_SwitchOff(int32_t index);
bool Escalator_IsActive(int32_t index);
EscalatorVector Escalator_GetPosition(int32_t index);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
