//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>

typedef struct PagerMessage {
    uint16_t *m_pText;
    uint16_t m_nSpeedMs;
    int16_t m_nCurrentPosition;
    uint16_t m_nStringLength;
    uint16_t m_nPriority;
    uint32_t m_nTimeToChangePosition;
    int16_t unused;
    int32_t m_nNumber[6];
} PagerMessage;
#define NUMPAGERMESSAGES 8
typedef struct CPager {
    int16_t m_nNumDisplayLetters;
    PagerMessage m_messages[NUMPAGERMESSAGES];
} CPager;
#ifdef __cplusplus
extern "C" {
#endif
extern CPager Pager_state;
void Pager_Init(CPager *self);
void Pager_Process(CPager *self);
void Pager_Display(CPager *self);
void Pager_AddMessage(CPager *self,uint16_t *str,uint16_t speed,uint16_t priority,uint16_t a5);
void Pager_AddMessageWithNumber(CPager *self,uint16_t *str,int32_t n1,int32_t n2,int32_t n3,int32_t n4,int32_t n5,int32_t n6,uint16_t speed,uint16_t priority,uint16_t a11);
void Pager_ClearMessages(CPager *self);
void Pager_RestartCurrentMessage(CPager *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
