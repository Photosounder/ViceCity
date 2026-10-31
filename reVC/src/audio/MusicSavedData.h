//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "audio_enums.h"

#ifdef __cplusplus
extern "C" {
#endif
extern float MusicSavedData_FavoriteRadioStationList[NUM_RADIOS];
extern int32_t RadioStationPosition[NUM_RADIOS];
float MusicSavedData_GetFavoriteRadioStationList(int32_t station);
void InitRadioStationPositionList(void);
int32_t GetSavedRadioStationPosition(int32_t station);
void PopulateRadioStationPositionList(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
