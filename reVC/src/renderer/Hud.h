//+ rouz edit (ChatGPT)
#pragma once
#include "Sprite2d.h"

#define HELP_MSG_LENGTH 256

#define HUD_TEXT_SCALE_X 0.7f
#define HUD_TEXT_SCALE_Y 1.25f

enum eItems
{
	ITEM_NONE = -1,
	ITEM_ARMOUR = 3,
	ITEM_HEALTH = 4,
	ITEM_RADAR = 8
};

// Thanks for vague name, R*
enum DRAW_FADE_STATE
{
	HUD_WANTED_FADING = 0,
	HUD_ENERGY_FADING,
	HUD_SCORE_FADING,
	HUD_WEAPON_FADING,
};

// My name
#include "HudFade.h"

enum eSprites 
{
	HUD_FIST,
	HUD_SITEROCKET = 41,
	HUD_RADARDISC = 50,
	HUD_SITESNIPER = 63,
	HUD_SITEM16,
	HUD_SITELASER,
	HUD_LASERDOT,
	HUD_VIEWFINDER,
	HUD_BLEEDER,
	NUM_HUD_SPRITES = 69,
};

//+ rouz edit (Claude)
// Formerly the static class CHud, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
extern Sprite2d Hud_Sprites[NUM_HUD_SPRITES];
extern wchar Hud_m_HelpMessage[HELP_MSG_LENGTH];
extern wchar Hud_m_LastHelpMessage[HELP_MSG_LENGTH];
extern uint32 Hud_m_HelpMessageState;
extern uint32 Hud_m_HelpMessageTimer;
extern int32 Hud_m_HelpMessageFadeTimer;
extern wchar Hud_m_HelpMessageToPrint[HELP_MSG_LENGTH];
extern float Hud_m_HelpMessageDisplayTime;
extern bool Hud_m_HelpMessageDisplayForever;
extern bool Hud_m_HelpMessageQuick;
extern uint32 Hud_m_ZoneState;
extern int32 Hud_m_ZoneFadeTimer;
extern uint32 Hud_m_ZoneNameTimer;
extern wchar * Hud_m_pZoneName;
extern wchar * Hud_m_pLastZoneName;
extern wchar * Hud_m_ZoneToPrint;
extern wchar * Hud_m_VehicleName;
extern wchar * Hud_m_pLastVehicleName;
extern wchar * Hud_m_pVehicleNameToPrint;
extern uint32 Hud_m_VehicleState;
extern int32 Hud_m_VehicleFadeTimer;
extern uint32 Hud_m_VehicleNameTimer;
extern wchar Hud_m_Message[256];
extern wchar Hud_m_PagerMessage[256];
extern bool Hud_m_Wants_To_Draw_Hud;
extern bool Hud_m_Wants_To_Draw_3dMarkers;
extern wchar Hud_m_BigMessage[6][128];
extern int16 Hud_m_ItemToFlash;
extern bool Hud_m_HideRadar;
extern int32 Hud_m_ClockState;
extern float Hud_BigMessageInUse[6];
extern float Hud_BigMessageAlpha[6];
extern float Hud_BigMessageX[6];
extern float Hud_OddJob2OffTimer;
extern bool Hud_CounterOnLastFrame[NUMONSCREENCOUNTERS];
extern float Hud_OddJob2XOffset;
extern uint16 Hud_CounterFlashTimer[NUMONSCREENCOUNTERS];
extern uint16 Hud_OddJob2Timer;
extern bool Hud_TimerOnLastFrame;
extern int16 Hud_OddJob2On;
extern uint16 Hud_TimerFlashTimer;
extern int16 Hud_PagerSoundPlayed;
extern int32 Hud_SpriteBrightness;
extern float Hud_PagerXOffset;
extern int16 Hud_PagerTimer;
extern int16 Hud_PagerOn;
extern uint32 Hud_m_WantedFadeTimer;
extern uint32 Hud_m_WantedState;
extern uint32 Hud_m_WantedTimer;
extern uint32 Hud_m_EnergyLostFadeTimer;
extern uint32 Hud_m_EnergyLostState;
extern uint32 Hud_m_EnergyLostTimer;
extern uint32 Hud_m_DisplayScoreFadeTimer;
extern uint32 Hud_m_DisplayScoreState;
extern uint32 Hud_m_DisplayScoreTimer;
extern uint32 Hud_m_WeaponFadeTimer;
extern uint32 Hud_m_WeaponState;
extern uint32 Hud_m_WeaponTimer;
extern uint32 Hud_m_LastDisplayScore;
extern uint32 Hud_m_LastWanted;
extern uint32 Hud_m_LastWeapon;
extern uint32 Hud_m_LastTimeEnergyLost;
void Hud_Draw();
void Hud_DrawAfterFade();
void Hud_GetRidOfAllHudMessages();
#ifdef RELOADABLES
void Hud_ReloadTXD();
#endif
void Hud_Initialise();
void Hud_ReInitialise();
void Hud_SetBigMessage(wchar *message, uint16 style);
bool Hud_IsHelpMessageBeingDisplayed(void);
void Hud_SetMessage(wchar *message);
void Hud_SetPagerMessage(wchar *message);
void Hud_SetVehicleName(wchar *name);
void Hud_SetZoneName(wchar *name);
void Hud_Shutdown();
float Hud_DrawFadeState(DRAW_FADE_STATE, int);
void Hud_ResetWastedText(void);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
#ifdef RELOADABLES
#endif
void Hud_SetHelpMessage(wchar *message, bool quick, bool displayForever = false);
#endif
//- rouz edit (Claude)
//- rouz edit (ChatGPT)
