//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
extern bool Cutscene_running;
extern bool Cutscene_processing;
static inline bool Cutscene_IsRunning(void)
{
    // Read the live cutscene running flag
    return Cutscene_running;
}
static inline bool Cutscene_IsProcessing(void)
{
    // Read the live cutscene processing flag
    return Cutscene_processing;
}
static inline void Cutscene_StartProcessing(void)
{
    // Set the original processing flag
    Cutscene_processing = true;
}
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
