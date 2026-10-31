//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
enum { CONSOLE_MAX_LINES=8,CONSOLE_MAX_STR_LEN=40 };
typedef struct Console {
    uint8_t m_nLineCount;
    uint8_t m_nCurrentLine;
    uint16_t Buffers[CONSOLE_MAX_LINES][CONSOLE_MAX_STR_LEN];
    uint32_t m_aTimer[CONSOLE_MAX_LINES];
    uint8_t m_aRed[CONSOLE_MAX_LINES];
    uint8_t m_aGreen[CONSOLE_MAX_LINES];
    uint8_t m_aBlue[CONSOLE_MAX_LINES];
} Console;
#ifdef __cplusplus
extern "C" {
#endif
extern Console TheConsole;
void Console_Init(Console *console);
void Console_AddLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b);
void Console_AddOneLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b);
void Console_Display(Console *console);
void cprintf(char *format,...);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
