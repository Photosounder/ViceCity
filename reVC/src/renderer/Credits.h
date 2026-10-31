//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Credits_Init(void);
void Credits_Start(void);
void Credits_Stop(void);
bool Credits_AreCreditsDone(void);
void Credits_Render(void);
void Credits_PrintCreditSpace(float space,uint32_t *line);
void Credits_PrintCreditText(float scaleX,float scaleY,uint16_t *text,uint32_t *lineoffset,float scrolloffset);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
