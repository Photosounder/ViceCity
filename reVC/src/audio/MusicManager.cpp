//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include <time.h>
#include "soundlist.h"
#include "MusicManager.h"
#include "AudioManager.h"
#include "AudioSoundGame.h"
#include "ControllerConfig.h"
#include "Camera.h"
#include "Font.h"
#include "Hud.h"
#include "ModelIndices.h"
#include "Replay.h"
#include "Pad.h"
#include "Text.h"
#include "Timer.h"
#include "World.h"
#include "sampman.h"
#include "Stats.h"
#include "Script.h"
#include "ZoneCull.h"
#include "Weather.h"
#include "DMAudio.h"
#include "GenericGameStorage.h"

#if !defined FIX_BUGS && (defined RADIO_SCROLL_TO_PREV_STATION || defined RADIO_OFF_TEXT)
static_assert(false, "R*'s radio implementation is quite buggy, RADIO_SCROLL_TO_PREV_STATION and RADIO_OFF_TEXT won't work without FIX_BUGS");
#endif

cMusicManager MusicManager;

cMusicManager::cMusicManager()
{
	// Initialize music state at the original compiler startup callback
	MusicState_InitState(this);
}

//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" uint32_t
MusicInitHost_PauseTime(void)
{
    // Read the original pause-mode timer after each streamed-file length and position update
    return Timer_GetTimeInMillisecondsPauseMode();
}
//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
extern "C" int32_t
MusicAmbienceHost_Area(void)
{
    // Read the current area at the original ambience selection point
    return CGame::currArea;
}
extern "C" uint8_t
MusicAmbienceHost_RiotIntensity(void)
{
    // Read the live riot intensity without caching across branches
    return CTheScripts::RiotIntensity;
}
extern "C" uint8_t
MusicAmbienceHost_AtBeach(void)
{
    // Read the existing beach flag through its boolean representation
    return CullZones_atBeachForAudio;
}
extern "C" int16_t
MusicAmbienceHost_OldWeather(void)
{
    // Read the old weather type at the original condition
    return Weather_OldWeatherType;
}
extern "C" int16_t
MusicAmbienceHost_NewWeather(void)
{
    // Read the new weather type only when the original condition needs it
    return Weather_NewWeatherType;
}
extern "C" float
MusicAmbienceHost_DistanceToWater(void)
{
    // Read the live camera distance at each original threshold or calculation
    return TheCamera.DistanceToWater;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern "C" uint8_t *
MusicRadioHost_Station(CVehicle *vehicle)
{
    // Expose the original byte station for live reads and narrowed compound assignments
    return &vehicle->m_nRadioStation;
}
extern "C" uint8_t
MusicRadioHost_UsesSiren(CVehicle *vehicle)
{
    // Query the existing vehicle siren rule after the explicit model exceptions
    return vehicle->UsesSiren();
}
extern "C" uint8_t
MusicRadioHost_HasMetDebbie(void)
{
    // Read the existing script flag only for the Kaufman taxi model
    return CTheScripts::bPlayerHasMetDebbieHarry;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern "C" uint8_t
MusicStreamHost_SlowMotion(void)
{
    // Read the original slow motion gate without the reflection subsystem's build guards
    return Timer_GetIsSlowMotionActive();
}
extern "C" CEntity *
MusicStreamHost_Target(void)
{
    // Read the live camera target at each original target access
    return TheCamera.pTargetEntity;
}
extern "C" void
MusicStreamHost_TargetPosition(CEntity *target, AudioSoundPosition *position)
{
    // Copy the original target coordinates for C distance and panning calculations
    *position = AudioSoundPosition_FromVector(target->GetPosition());
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#ifdef GTA_PC
extern "C" uint8_t
MusicServiceHost_F9(void)
{
    // Query the original keyboard key after the MP3 and station gates
    return ControlsManager.GetIsKeyboardKeyJustDown(rsF9);
}
#endif
extern "C" uint8_t
MusicServiceHost_ChangeStation(void)
{
    // Retain the original pad lookup before its station-change query
    return CPad::GetPad(0)->ChangeStationJustDown();
}
#ifdef RADIO_SCROLL_TO_PREV_STATION
extern "C" uint8_t
MusicServiceHost_ControlsDisabled(void)
{
    // Retain the original pad lookup before checking disabled controls
    return CPad::GetPad(0)->ArePlayerControlsDisabled();
}
extern "C" uint8_t
MusicServiceHost_WheelDown(void)
{
    // Retain the original repeated pad lookup before the downward wheel query
    return CPad::GetPad(0)->GetMouseWheelDownJustDown();
}
extern "C" uint8_t
MusicServiceHost_WheelUp(void)
{
    // Retain the original repeated pad lookup before the upward wheel query
    return CPad::GetPad(0)->GetMouseWheelUpJustDown();
}
extern "C" int32_t
MusicServiceHost_NextMouseKey(void)
{
    // Read the original radio-station mouse binding without normalizing its key code
    return ControlsManager.GetControllerKeyAssociatedWithAction(VEHICLE_CHANGE_RADIO_STATION, MOUSE);
}
extern "C" uint8_t
MusicServiceHost_MouseAssigned(int32_t key)
{
    // Check the captured opposite wheel key at the original vehicle-action gate
    return ControlsManager.IsAnyVehicleActionAssignedToMouseKey(key);
}
#endif
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
extern wchar WideErrorString[];
extern "C" uint8_t
MusicDisplayHost_Paused(void)
{
    // Read the original display input at its existing access point
    return Timer_GetIsPaused();
}
extern "C" uint8_t
MusicDisplayHost_Widescreen(void)
{
    // Read the original display input at its existing access point
    return TheCamera.m_WideScreenOn;
}
extern "C" uint16_t *
MusicDisplayHost_Text(const char *key)
{
    // Read the original display input at its existing access point
    return Text_Get(&TheText,key);
}
extern "C" uint16_t *
MusicDisplayHost_ErrorText(void)
{
    // Read the original display input at its existing access point
    return WideErrorString;
}
extern "C" float
MusicDisplayHost_ScaleX(float value)
{
    // Read the original display input at its existing access point
    return SCREEN_SCALE_X(value);
}
extern "C" float
MusicDisplayHost_ScaleY(float value)
{
    // Read the original display input at its existing access point
    return SCREEN_SCALE_Y(value);
}
extern "C" float
MusicDisplayHost_Width(void)
{
    // Read the original display input at its existing access point
    return SCREEN_WIDTH;
}
extern "C" float
MusicDisplayHost_CentreSize(void)
{
    // Read the original display input at its existing access point
    return SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH);
}
extern "C" void
MusicDisplayFont_JustifyOff(void)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetJustifyOff();
}
extern "C" void
MusicDisplayFont_BackgroundOff(void)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetBackgroundOff();
}
extern "C" void
MusicDisplayFont_Scale(float x, float y)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetScale(x, y);
}
extern "C" void
MusicDisplayFont_PropOn(void)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetPropOn();
}
extern "C" void
MusicDisplayFont_Standard(void)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetFontStyle(FONT_STANDARD);
}
extern "C" void
MusicDisplayFont_CentreOn(void)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetCentreOn();
}
extern "C" void
MusicDisplayFont_CentreSize(float size)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetCentreSize(size);
}
extern "C" void
MusicDisplayFont_Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    // Submit the original font operation with the captured C display arguments
    CFont::SetColor(CRGBA(r, g, b, a));
}
extern "C" void
MusicDisplayFont_Print(float x, float y, uint16_t *text)
{
    // Submit the original font operation with the captured C display arguments
    CFont::PrintString(x, y, text);
}
extern "C" void
MusicDisplayFont_Draw(void)
{
    // Submit the original font operation with the captured C display arguments
    CFont::DrawFonts();
}
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
