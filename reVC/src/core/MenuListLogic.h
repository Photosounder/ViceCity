//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Scrolling of a menu list with a scrollbar, the rows are counted from zero and visible is the number of rows shown at once
void MenuList_ScrollUpByOne(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, float scrollbarMaxHeight);
void MenuList_ScrollDownByOne(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, int32_t visible, float scrollbarMaxHeight);
// Page up or down, the result tells whether the selection sound has to be played
bool MenuList_PageUp(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, int32_t visible, float scrollbarMaxHeight);
bool MenuList_PageDown(int32_t *selectedRow, int32_t *firstVisibleRow, float *scrollbarTop, int32_t totalRows, int32_t visible, float scrollbarMaxHeight);
// Radio station after a step to the next or previous one wrapped around the stations that can be picked, the user tracks are one of them when MP3 playback is available
int32_t MenuLogic_WrapRadioStation(int32_t station, bool mp3Available);
// Moves the cursor of a multiple choice control by one option, past the end the cursor is cleared and the result is false so the focus moves to another control
bool MenuLogic_CursorNext(int32_t *cursor, int32_t numOptions);
bool MenuLogic_CursorPrev(int32_t *cursor);
// New value of a settings slider after value steps (value is negative to the left), bars is the number of steps of the slider
int32_t MenuLogic_SliderBrightness(int32_t brightness, int value, int bars);
float MenuLogic_SliderLOD(float lod, int value, int bars);
int8_t MenuLogic_SliderVolume(int8_t volume, int value, int bars);
float MenuLogic_SliderMouseSens(float mouseAccelHorizontal, int value);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
