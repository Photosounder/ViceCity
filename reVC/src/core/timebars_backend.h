//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void TimeBars_PrepareFont(void);
void TimeBars_SetTextColor(void);
bool TimeBars_IsVisible(void);
void TimeBars_Print(float y, uint16_t *text);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
