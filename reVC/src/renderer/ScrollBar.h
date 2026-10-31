//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#define NUMSCROLLBARS 1
typedef struct ScrollBarVector { float x,y,z; } ScrollBarVector;
typedef struct ScrollBar {
    uint8_t m_Counter;
    const char *m_pMessage;
    ScrollBarVector m_Position;
    uint32_t m_MessageCurrentChar,m_MessageLength;
    ScrollBarVector m_Size;
    float m_fIntensity;
    uint8_t m_MessageBar[40];
    uint8_t m_Type;
    bool m_bVisible;
    uint8_t m_uRed,m_uGreen,m_uBlue;
    float m_fScale;
} ScrollBar;
#ifdef __cplusplus
extern "C" {
#endif
extern ScrollBar aScrollBars[NUMSCROLLBARS];
extern int32_t ScrollBar_TonightsEvent;
void ScrollBar_Init(ScrollBar *bar,ScrollBarVector first,ScrollBarVector last,uint8_t type,uint8_t r,uint8_t g,uint8_t b,float scale);
void ScrollBar_Update(ScrollBar *bar);
void ScrollBar_Render(ScrollBar *bar);
void ScrollBar_SetVisibility(ScrollBar *bar,bool visible);
bool ScrollBar_IsVisible(ScrollBar *bar);
#ifdef REVC_SOFTWARE_POLYGONS
void ScrollBar_RenderForEnvMap(ScrollBar *bar,void *camera);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
