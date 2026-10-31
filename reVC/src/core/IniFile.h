//+ rouz edit (ChatGPT)
#pragma once
#define DEFAULT_MAX_NUMBER_OF_PEDS 25.0f
#define DEFAULT_MAX_NUMBER_OF_PEDS_INTERIOR 40.0f
#define DEFAULT_MAX_NUMBER_OF_CARS 12.0f
#ifdef __cplusplus
extern "C" {
#endif
void IniFile_LoadIniFile(void);
extern float IniFile_PedNumberMultiplier;
extern float IniFile_CarNumberMultiplier;
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
