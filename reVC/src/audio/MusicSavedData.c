//+ rouz edit (ChatGPT)
#include "MusicSavedData.h"
#include "DMAudio.h"

float MusicSavedData_FavoriteRadioStationList[NUM_RADIOS];
int32_t RadioStationPosition[NUM_RADIOS];

float MusicSavedData_GetFavoriteRadioStationList(int32_t station)
{
	// Preserve the original saved-radio array access and iteration order
	return MusicSavedData_FavoriteRadioStationList[station];
}

void
InitRadioStationPositionList(void)
{
	// Preserve the original saved-radio array access and iteration order
	for (int i = 0; i < NUM_RADIOS; i++)
		RadioStationPosition[i] = -1;
}

int32_t
GetSavedRadioStationPosition(int32_t station)
{
	// Preserve the original saved-radio array access and iteration order
	return RadioStationPosition[station];
}

void
PopulateRadioStationPositionList(void)
{
	// Preserve the original saved-radio array access and iteration order
	for (int i = 0; i < NUM_RADIOS; i++)
		RadioStationPosition[i] = DMAudio_GetRadioPosition(i);
}

//- rouz edit (ChatGPT)
