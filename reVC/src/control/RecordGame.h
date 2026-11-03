//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void RecordGame_Init(void);
void RecordGame_SaveOrRetrieveDataForThisFrame(void);
bool RecordGame_IsRecording(void);
bool RecordGame_IsPlayingBack(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
