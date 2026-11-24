//+ rouz edit (Claude)
#include "MenuListLogic.h"
#include "audio_enums.h"

void MenuList_ScrollUpByOne(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, float scrollbarMaxHeight)
{
	// At the top of the visible rows the list scrolls, otherwise only the selection moves
	if (*selectedRow == *firstVisibleRow) {
		if (*firstVisibleRow > 0) {
			(*selectedRow)--;
			(*firstVisibleRow)--;
			*scrollbarTop -= scrollbarMaxHeight / totalRows;
		}
	} else {
		(*selectedRow)--;
	}
}

void MenuList_ScrollDownByOne(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, int32_t visible, float scrollbarMaxHeight)
{
	// At the bottom of the visible rows the list scrolls, otherwise only the selection moves
	if (*selectedRow == *firstVisibleRow + visible - 1) {
		if (*firstVisibleRow < totalRows - visible) {
			(*selectedRow)++;
			(*firstVisibleRow)++;
			*scrollbarTop += scrollbarMaxHeight / totalRows;
		}
	} else {
		if (*selectedRow < totalRows - 1)
			(*selectedRow)++;
	}
}

bool MenuList_PageUp(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, int32_t visible, float scrollbarMaxHeight)
{
	bool playSound = false;

	// Lists that fit on the screen do not page
	if (totalRows > visible) {
		if (*firstVisibleRow > 0) {
			int32_t first = *firstVisibleRow - visible;
			int32_t lastVisible;
			playSound = true;
			*firstVisibleRow = first > 0 ? first : 0;
			lastVisible = *firstVisibleRow + visible - 1;
			*selectedRow = *selectedRow < lastVisible ? *selectedRow : lastVisible;
		} else {
			*firstVisibleRow = 0;
			*selectedRow = 0;
		}
		*scrollbarTop = (scrollbarMaxHeight / totalRows) * *firstVisibleRow;
	}
	return playSound;
}

bool MenuList_PageDown(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, int32_t visible, float scrollbarMaxHeight)
{
	bool playSound = false;

	// Lists that fit on the screen do not page
	if (totalRows > visible) {
		if (*firstVisibleRow < totalRows - visible) {
			int32_t first = *firstVisibleRow + visible;
			int32_t limit = totalRows - visible;
			playSound = true;
			*firstVisibleRow = first < limit ? first : limit;
			*selectedRow = *selectedRow > *firstVisibleRow ? *selectedRow : *firstVisibleRow;
		} else {
			*firstVisibleRow = totalRows - visible;
			*selectedRow = totalRows - 1;
		}
		*scrollbarTop = (scrollbarMaxHeight / totalRows) * *firstVisibleRow;
	}
	return playSound;
}
int32_t MenuLogic_WrapRadioStation(int32_t station, bool mp3Available)
{
	if (mp3Available) {
		if (station < WILDSTYLE)
			station = USERTRACK;
		if (station > USERTRACK)
			station = WILDSTYLE;
	} else {
		if (station < WILDSTYLE)
			station = WAVE;
		if (station > WAVE)
			station = WILDSTYLE;
	}

	return station;
}

bool MenuLogic_CursorNext(int32_t *cursor, int32_t numOptions)
{
	if (*cursor == numOptions - 1) {
		*cursor = -1;
		return false;
	} else {
		(*cursor)++;
		return true;
	}
}

bool MenuLogic_CursorPrev(int32_t *cursor)
{
	if (*cursor == 0) {
		*cursor = -1;
		return false;
	} else {
		(*cursor)--;
		return true;
	}
}

int32_t MenuLogic_SliderBrightness(int32_t brightness, int value, int bars)
{
	brightness += value * (384 / bars);
	return brightness < 0 ? 0 : brightness > 384 ? 384 : brightness;
}

float MenuLogic_SliderLOD(float lod, int value, int bars)
{
	if (value > 0)
		lod += ((1.8f - 0.925f) / bars);
	else
		lod -= ((1.8f - 0.925f) / bars);
	return lod < 0.925f ? 0.925f : lod > 1.8f ? 1.8f : lod;
}

int8_t MenuLogic_SliderVolume(int8_t volume, int value, int bars)
{
	// The volume can be at most 65, one more than the 64 the steps add up to
	volume += value * (64 / bars);
	return volume < 0 ? 0 : volume > 65 ? 65 : volume;
}

float MenuLogic_SliderMouseSens(float mouseAccelHorizontal, int value)
{
	// Dividing by 15 instead of 16 gives more accurate steps
	mouseAccelHorizontal += value * 1.0f/200.0f/15.0f;
	return mouseAccelHorizontal < 1.0f/3200.0f ? 1.0f/3200.0f : mouseAccelHorizontal > 1.0f/200.0f ? 1.0f/200.0f : mouseAccelHorizontal;
}

float Menu_ClampMapCenter(float center, float halfScreen, float mapSize, float minMapSize)
{
	float low = halfScreen - (mapSize - minMapSize);
	float high = mapSize - minMapSize + halfScreen;

	return center < low ? low : center > high ? high : center;
}
int Menu_NextFirstStartCounter(int counter)
{
	int v = counter + 20;

	return v < 255 ? v : 255;
}
float Menu_MapSizeZoomedIn(float mapSize, float step, float maxSize)
{
	float v = mapSize + step;

	return maxSize < v ? maxSize : v;
}
float Menu_MapSizeZoomedOut(float mapSize, float step, float minSize)
{
	float v = mapSize - step;

	return minSize > v ? minSize : v;
}
int Menu_FadedAlpha(int menuFadeAlpha, int alpha)
{
	return menuFadeAlpha < alpha ? menuFadeAlpha : alpha;
}
//- rouz edit (Claude)
