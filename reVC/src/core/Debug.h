//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
enum {Debug_MAX_LINES=15,Debug_MAX_STR_LEN=80,Debug_MAX_SCREEN_STRS=100,Debug_MAX_DEBUG_LINES=100};
typedef struct Debug_Vector {float x,y,z;} Debug_Vector;
typedef struct Debug_ScreenStr {int x,y;char str[256];} Debug_ScreenStr;
typedef struct Debug_Line {Debug_Vector p1,p2;uint32_t c1,c2;} Debug_Line;
#ifdef __cplusplus
extern "C" {
#endif
extern bool gbDebugStuffInRelease;
extern int16_t Debug_ms_nCurrentTextLine;
extern char Debug_ms_aTextBuffer[Debug_MAX_LINES][Debug_MAX_STR_LEN];
extern Debug_ScreenStr Debug_ms_aScreenStrs[Debug_MAX_SCREEN_STRS];
extern int Debug_ms_nScreenStrs;
extern Debug_Line Debug_ms_aLines[Debug_MAX_DEBUG_LINES];
extern int Debug_ms_nLines;
void Debug_DebugInitTextBuffer(void);
void Debug_DebugAddText(const char *str);
void Debug_DebugDisplayTextBuffer(void);
void Debug_DisplayScreenStrings(void);
void Debug_PrintAt(const char *str,int x,int y);
void Debug_AddLine(Debug_Vector p1,Debug_Vector p2,uint32_t c1,uint32_t c2);
void Debug_DrawLines(void);
#ifdef __cplusplus
}
class CVector;
Debug_Vector Debug_FromVector(CVector value);
#endif
//- rouz edit (ChatGPT)
