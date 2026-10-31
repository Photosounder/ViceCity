//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct MoneyMessageVector { float x,y,z; } MoneyMessageVector;
typedef struct MoneyMessageColor { uint8_t r,g,b,a; } MoneyMessageColor;
typedef struct MoneyMessage {
    uint32_t m_nTimeRegistered;
    MoneyMessageVector m_vecPosition;
    uint16_t m_aText[16];
    MoneyMessageColor m_Colour;
    float m_fSize,m_fOpacity;
} MoneyMessage;
#ifdef __cplusplus
extern "C" {
#endif
void MoneyMessage_Render(MoneyMessage *message);
void MoneyMessages_Init(void);
void MoneyMessages_Render(void);
void MoneyMessages_RegisterOne(MoneyMessageVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity);
#ifdef __cplusplus
}
class CVector;
MoneyMessageVector MoneyMessages_FromVector(CVector position);
void MoneyMessages_RegisterOneLegacy(CVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity);
#endif
//- rouz edit (ChatGPT)
