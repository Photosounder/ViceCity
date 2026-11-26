//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"

#include "Camera.h"
#include "DMAudio.h"
#include "Clock.h"
#include "Darkel.h"
#include "Hud.h"
#include "HudMessages.h"
#include "HudLogic.h"
#include "Messages.h"
#include "Frontend.h"
#include "Font.h"
#include "Pad.h"
#include "Radar.h"
#include "Replay.h"
#include "Wanted.h"
#include "Sprite.h"
#include "Sprite2d.h"

static Sprite2d *Sprite2d_HudLocalTarget;
static void Sprite2d_ExitHudLocal(void)
{
    // Destroy the same lazily initialized C record at its original process-exit position
    Sprite2d_Deinit(Sprite2d_HudLocalTarget);
}

#include "Text.h"
#include "Timer.h"
#include "Script.h"
#include "TxdStore.h"
#include "User.h"
#include "World.h"
#include "CutsceneMgr.h"
#include "Stats.h"
#include "main.h"
#include "General.h"
#include "VarConsole.h"

#if defined(FIX_BUGS)
	#define SCREEN_SCALE_X_FIX(a) SCREEN_SCALE_X(a)
	#define SCREEN_SCALE_Y_FIX(a) SCREEN_SCALE_Y(a)
	#define SCALE_AND_CENTER_X_FIX(a) SCALE_AND_CENTER_X(a)
#else
	#define SCREEN_SCALE_X_FIX(a) (a)
	#define SCREEN_SCALE_Y_FIX(a) (a)
	#define SCALE_AND_CENTER_X_FIX(a) (a)
#endif

#ifdef FIX_BUGS
#define FRAMECOUNTER Timer_GetLogicalFrameCounter()
#else
#define FRAMECOUNTER (*Timer_GetFrameCounter())
#endif

// Game has colors inlined in code.
// For easier modification we collect them here:
CRGBA MONEY_COLOR(0, 207, 133, 255);
CRGBA AMMO_COLOR(255, 150, 225, 255);
CRGBA HEALTH_COLOR(255, 150, 225, 255);
CRGBA ARMOUR_COLOR(185, 185, 185, 255);
CRGBA NOTWANTED_COLOR(27, 89, 130, 255);
CRGBA WANTED_COLOR_FLASH(62, 141, 181, 255);
CRGBA WANTED_COLOR(97, 194, 247, 255);
CRGBA ZONE_COLOR(45, 155, 90, 255);
CRGBA VEHICLE_COLOR(97, 194, 247, 255);
CRGBA CLOCK_COLOR(97, 194, 247, 255);
CRGBA TIMER_COLOR(97, 194, 247, 255);
CRGBA COUNTER_COLOR(97, 194, 247, 255);
CRGBA PAGER_COLOR(32, 162, 66, 205);
CRGBA RADARDISC_COLOR(255, 255, 255, 255);
CRGBA BIGMESSAGE_COLOR(255, 150, 225, 255);
CRGBA WASTEDBUSTED_COLOR(0, 207, 133, 255);
CRGBA ODDJOB_COLOR(0, 207, 133, 255);
CRGBA ODDJOB2_COLOR(97, 194, 247, 255);
CRGBA MISSIONTITLE_COLOR(220, 172, 2, 255);

wchar Hud_m_HelpMessage[HELP_MSG_LENGTH];
wchar Hud_m_LastHelpMessage[HELP_MSG_LENGTH];
uint32 Hud_m_HelpMessageState;
uint32 Hud_m_HelpMessageTimer;
int32 Hud_m_HelpMessageFadeTimer;
wchar Hud_m_HelpMessageToPrint[HELP_MSG_LENGTH];
float Hud_m_HelpMessageDisplayTime;
bool Hud_m_HelpMessageDisplayForever;
bool Hud_m_HelpMessageQuick;
uint32 Hud_m_ZoneState;
int32 Hud_m_ZoneFadeTimer;
uint32 Hud_m_ZoneNameTimer;
wchar *Hud_m_pZoneName;
wchar *Hud_m_pLastZoneName;
wchar *Hud_m_ZoneToPrint;
uint32 Hud_m_VehicleState;
int32 Hud_m_VehicleFadeTimer;
uint32 Hud_m_VehicleNameTimer;
wchar *Hud_m_VehicleName;
wchar *Hud_m_pLastVehicleName;
wchar *Hud_m_pVehicleNameToPrint;
wchar Hud_m_Message[256];
wchar Hud_m_PagerMessage[256];
bool Hud_m_Wants_To_Draw_Hud;
bool Hud_m_Wants_To_Draw_3dMarkers;
wchar Hud_m_BigMessage[6][128];
int16 Hud_m_ItemToFlash;
bool Hud_m_HideRadar;
int32 Hud_m_ClockState;

// These aren't really in CHud
float Hud_BigMessageInUse[6];
float Hud_BigMessageAlpha[6];
float Hud_BigMessageX[6];
float Hud_OddJob2OffTimer;
bool Hud_CounterOnLastFrame[NUMONSCREENCOUNTERS];
float Hud_OddJob2XOffset;
uint16 Hud_CounterFlashTimer[NUMONSCREENCOUNTERS];
uint16 Hud_OddJob2Timer;
bool Hud_TimerOnLastFrame;
int16 Hud_OddJob2On;
uint16 Hud_TimerFlashTimer;
int16 Hud_PagerSoundPlayed;
int32 Hud_SpriteBrightness;
float Hud_PagerXOffset;
int16 Hud_PagerTimer;
int16 Hud_PagerOn;

wchar *prevChaseString;

uint32 Hud_m_WantedFadeTimer;
uint32 Hud_m_WantedState;
uint32 Hud_m_WantedTimer;
uint32 Hud_m_EnergyLostFadeTimer;
uint32 Hud_m_EnergyLostState;
uint32 Hud_m_EnergyLostTimer;
uint32 Hud_m_DisplayScoreFadeTimer;
uint32 Hud_m_DisplayScoreState;
uint32 Hud_m_DisplayScoreTimer;
uint32 Hud_m_WeaponFadeTimer;
uint32 Hud_m_WeaponState;
uint32 Hud_m_WeaponTimer;

uint32 Hud_m_LastDisplayScore;
uint32 Hud_m_LastWanted;
uint32 Hud_m_LastWeapon;
uint32 Hud_m_LastTimeEnergyLost;

Sprite2d Hud_Sprites[NUM_HUD_SPRITES];
static void Sprite2d_Exit_CHud_Sprites(void)
{
    // Destroy this original object group in its former reverse array order
    Sprite2d_DeinitArray(Hud_Sprites,(size_t)(NUM_HUD_SPRITES));
}
static int Sprite2d_Start_CHud_Sprites(void)
{
    // Initialize at the original definition boundary and retain its exit registration position
    Sprite2d_InitArray(Hud_Sprites,(size_t)(NUM_HUD_SPRITES));atexit(Sprite2d_Exit_CHud_Sprites);return 0;
}
static const int Sprite2d_Started_CHud_Sprites=Sprite2d_Start_CHud_Sprites();

struct
{
	const char *name;
	const char *mask;
} WeaponFilenames[] = {
  { "fist", "fistm" },
  { "brassk", "brasskA" },
  { "screw", "screwA" },
  { "golf", "golfA" },
  { "nightstick", "nightstickA" },
  { "knife", "knifeA" },
  { "bat", "batm" },
  { "hammer", "hammerA" },
  { "cleaver", "cleaverA" },
  { "machete", "macheteA" },
  { "sword", "swordA" },
  { "chainsaw", "chainsawA" },
  { "grenade", "grenadeA" },
  { "grenade", "grenadeA" },
  { "teargas", "teargasA" },
  { "molotov", "molotovA" },
  { "rocket", "rocketA" },
  { "handGun1", "handGun1A" },
  { "", "" },
  { "python", "pythonA" },
  { "chromegun", "chromegunA" },
  { "spasshotGun", "spasshotGunA" },
  { "stubshotGun", "stubshotGunA" },
  { "tec9", "tec9A" },
  { "uzi1", "uzi1A" },
  { "uzi2", "uzi2A" },
  { "mp5", "mp5A" },
  { "", "" },
  { "m4", "m4A" },
  { "ruger", "rugerA" },
  { "sniper", "sniperA" },
  { "laserscope", "laserscopeA" },
  { "", "" },
  { "rocket", "rocketA" },
  { "flamer", "flamerA" },
  { "m60", "m60A" },
  { "minigun", "minigunA" },
  { "bomb", "bombA" },
  { "", "" },
  { "camera", "cameraA" },
  { "", "" },
  { "siterocket", "siterocket" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "radardisc", "radardisc" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "", "" },
  { "sitesniper", "sitesniperm" },
  { "siteM16", "siteM16m" },
  { "sitelaser", "sitelaserm" },
  { "laserdot", "laserdotm" },
  { "viewfinder_128", "viewfinder_128m" },
  { "bleeder", "" }
};

RwTexture *gpSniperSightTex;
RwTexture *gpRocketSightTex;
RwTexture *gpLaserSightTex;
RwTexture *gpLaserDotTex;
RwTexture *gpViewFinderTex;

void Hud_Draw()
{
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

	// disable hud via second controller
	if (CPad::GetPad(1)->GetStartJustDown())
		Hud_m_Wants_To_Draw_Hud = !Hud_m_Wants_To_Draw_Hud;

	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;

	if (Hud_m_Wants_To_Draw_Hud && !TheCamera.m_WideScreenOn) {
		// unused statics in here
		bool DrawCrossHair = false;
		bool CrossHairHidesHud = false;
		bool DrawCrossHairPC = false;

		CPlayerPed *playerPed = FindPlayerPed();
		eWeaponType WeaponType = playerPed->GetWeapon()->m_eWeaponType;
		int32 Mode = TheCamera.Cams[TheCamera.ActiveCam].Mode;

		// Delegate to the C implementation
		Hud_CrosshairFlags(Mode, WeaponType, playerPed->GetWeapon()->IsTypeMelee(),
			TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam(), CPad::GetPad(0)->GetLookBehindForPed(), TheCamera.m_bPlayerIsInGarage,
			playerPed->m_nPedState, &DrawCrossHair, &DrawCrossHairPC);

		if (DrawCrossHair || DrawCrossHairPC) {
			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void *)rwFILTERLINEAR);

			// Delegate the next brightness to the C implementation
			Hud_SpriteBrightness = Hud_NextSpriteBrightness(Hud_SpriteBrightness);

			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);

			// Delegate the pulse of the crosshair to the C implementation
			float fMultBright = Hud_CrosshairPulse((*Timer_GetTimeInMilliseconds()), Hud_SpriteBrightness);
			CRect rect;
			// Initialize rectangle bounds at the former local constructor boundary
			Rect_Init(&rect);
			if (DrawCrossHairPC && TheCamera.Cams[TheCamera.ActiveCam].Using3rdPersonMouseCam()) {
				float f3rdX = SCREEN_WIDTH * TheCamera.m_f3rdPersonCHairMultX;
				float f3rdY = SCREEN_HEIGHT * TheCamera.m_f3rdPersonCHairMultY;
#ifdef ASPECT_RATIO_SCALE
				f3rdY -= SCREEN_SCALE_Y(2.0f);
#endif
				if (playerPed && (WeaponType == WEAPONTYPE_M4 || WeaponType == WEAPONTYPE_RUGER || WeaponType == WEAPONTYPE_M60)) {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.6f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f  * 0.6f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.6f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f  * 0.6f);

					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[HUD_SITEM16]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
				else {
					rect.left = f3rdX - SCREEN_SCALE_X(32.0f * 0.4f);
					rect.top = f3rdY - SCREEN_SCALE_Y(32.0f  * 0.4f);
					rect.right = f3rdX + SCREEN_SCALE_X(32.0f * 0.4f);
					rect.bottom = f3rdY + SCREEN_SCALE_Y(32.0f  * 0.4f);

					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[HUD_SITEM16]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
			} else {
				if (Mode == CCam::MODE_M16_1STPERSON ||
				    Mode == CCam::MODE_M16_1STPERSON_RUNABOUT ||
				    Mode == CCam::MODE_HELICANNON_1STPERSON) {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f);
					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[HUD_SITEM16]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
				else if (Mode == CCam::MODE_1STPERSON_RUNABOUT) {
					rect.left = (SCREEN_WIDTH / 2) - SCREEN_SCALE_X(32.0f * 0.7f);
					rect.top = (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(32.0f * 0.7f);
					rect.right = (SCREEN_WIDTH / 2) + SCREEN_SCALE_X(32.0f * 0.7f);
					rect.bottom = (SCREEN_HEIGHT / 2) + SCREEN_SCALE_Y(32.0f * 0.7f);

					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[HUD_SITEM16]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.0f, 0.0f,  1.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f);
				}
				else if (Mode == CCam::MODE_ROCKETLAUNCHER || Mode == CCam::MODE_ROCKETLAUNCHER_RUNABOUT) {
					RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
					RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDONE);
					RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDONE);
					RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
					RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
					RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpRocketSightTex));

					// Use the complete C sprite operation with the original argument values
					Sprite_RenderOneXLUSprite(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 1.0f, SCREEN_SCALE_X(40.0f), SCREEN_SCALE_Y(40.0f), (100.0f * fMultBright), (200.0f * fMultBright), (100.0f * fMultBright), 255, 1.0f, 255);
					RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
					RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
				}
				else {
					int sprite = HUD_SITESNIPER;
					float xOffset = SCREEN_SCALE_X(210.0f);
					float yOffset = SCREEN_SCALE_Y(210.0f);

					if (FindPlayerPed()->GetWeapon()->m_eWeaponType == WEAPONTYPE_LASERSCOPE)
					  sprite = HUD_SITELASER;

					if (FindPlayerPed()->GetWeapon()->m_eWeaponType == WEAPONTYPE_CAMERA) {
						sprite = HUD_VIEWFINDER;
						CrossHairHidesHud = true;
						xOffset = SCREEN_SCALE_X(256.0f);
						yOffset = SCREEN_SCALE_Y(192.0f);
					}

					rect.left = SCREEN_WIDTH/2 - xOffset;
					rect.top = SCREEN_HEIGHT/2 - yOffset;
					rect.right = SCREEN_WIDTH/2;
					rect.bottom = SCREEN_HEIGHT/2;
					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[sprite]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.01f, 0.01f,  1.0f, 0.0f,  0.01f, 1.0f,  1.0f, 1.0f);

					rect.left = SCREEN_WIDTH/2;
					rect.top = SCREEN_HEIGHT/2 - yOffset;
					rect.right = SCREEN_WIDTH/2 + xOffset;
					rect.bottom = SCREEN_HEIGHT/2;
					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[sprite]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.99f, 0.0f,  0.01f, 0.01f,  0.99f, 1.0f,  0.01f, 1.0f);

					rect.left = SCREEN_WIDTH/2 - xOffset;
					rect.top = SCREEN_HEIGHT/2;
					rect.right = SCREEN_WIDTH/2;
					rect.bottom = SCREEN_HEIGHT/2 + yOffset;
					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[sprite]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.01f, 0.99f,  1.0f, 0.99f,  0.01f, 0.01f,  1.0f, 0.01f);

					rect.left = SCREEN_WIDTH/2;
					rect.top = SCREEN_HEIGHT/2;
					rect.right = SCREEN_WIDTH/2 + xOffset;
					rect.bottom = SCREEN_HEIGHT/2 + yOffset;
					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[sprite]),Rect_CopyValue(rect), CRGBA(255, 255, 255, 255),
						0.99f, 0.99f,  0.01f, 0.99f,  0.99f, 0.01f,  0.01f, 0.01f);

					CVector dotPos;
					float size = 25.0f;
					if (FindPlayerPed()->GetWeapon()->m_eWeaponType == WEAPONTYPE_LASERSCOPE && FindPlayerPed()->GetWeapon()->LaserScopeDot(&dotPos, &size)) {
						RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
						RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
						RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)FALSE);
						RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
#ifdef FIX_BUGS
						RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
#else
						RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVDESTALPHA);
#endif
						RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(gpLaserDotTex));
#ifdef FIX_BUGS
						int intensity = General_GetRandomNumberInRangeInt(0, 37);
#else
						int intensity = General_GetRandomNumberInRangeInt(0, 35);
#endif
						// Use the complete C sprite operation with the original argument values
						Sprite_RenderOneXLUSprite(dotPos.x, dotPos.y, dotPos.z,
							SCREEN_SCALE_X(size), SCREEN_SCALE_Y(size), intensity - 36, 0, 0, intensity - 36, 1.0f, 127);

						RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
					}
				}
			}
			RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *)rwBLENDSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *)rwBLENDINVSRCALPHA);
			RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		}
		else {
			Hud_SpriteBrightness = 0;
		}

		if (CrossHairHidesHud)
			return;

		/*
			DrawMoneyCounter
		*/

		wchar sPrint[16];
		wchar sPrintIcon[16];
		char sTemp[16];
		float alpha;

		if (Hud_m_LastDisplayScore == World_Players[World_PlayerInFocus].m_nVisibleMoney) {
			alpha = Hud_DrawFadeState(HUD_SCORE_FADING, 0);
		} else {
			alpha = Hud_DrawFadeState(HUD_SCORE_FADING, 1);
			Hud_m_LastDisplayScore = World_Players[World_PlayerInFocus].m_nVisibleMoney;
		}
		if (Hud_m_DisplayScoreState != FADED_OUT) {
			sprintf(sTemp, "$%08d", World_Players[World_PlayerInFocus].m_nVisibleMoney);
			AsciiToUnicode(sTemp, sPrint);

			Font_SetPropOff();
			Font_SetBackgroundOff();
			Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
			Font_SetCentreOff();
			Font_SetRightJustifyOn();
			Font_SetRightJustifyWrap(0.0f);
			Font_SetBackGroundOnlyTextOff();
			Font_SetFontStyle(FONT_HEADING);
			Font_SetPropOff();
			Font_SetDropShadowPosition(2);
			Font_SetDropColor(CRGBA(0, 0, 0, alpha));
			MONEY_COLOR.a = alpha;
			Font_SetColor(MONEY_COLOR);

			if (FrontEndMenuManager.m_PrefsShowHud) {
				Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f), SCREEN_SCALE_Y(43.0f), sPrint);
			}
		}

		/*
			DrawAmmo
		*/
		if (Hud_m_LastWeapon == playerPed->GetWeapon()->m_eWeaponType) {
			alpha = Hud_DrawFadeState(HUD_WEAPON_FADING, 0);
		} else {
			alpha = Hud_DrawFadeState(HUD_WEAPON_FADING, 1);
			Hud_m_LastWeapon = playerPed->GetWeapon()->m_eWeaponType;
		}
		if (Hud_m_WeaponState != FADED_OUT) {
			CWeapon *weapon = playerPed->GetWeapon();
			int32 AmmoAmount = WeaponInfo_GetWeaponInfo((eWeaponType)WeaponType)->m_nAmountofAmmunition;
			int32 AmmoInClip = weapon->m_nAmmoInClip;
			int32 TotalAmmo = weapon->m_nAmmoTotal;
			// Delegate to the C implementation
			Hud_FormatAmmo(sTemp, AmmoAmount, WeaponType, AmmoInClip, TotalAmmo);

			AsciiToUnicode(sTemp, sPrint);
			CWeaponInfo *weaponInfo = WeaponInfo_GetWeaponInfo((eWeaponType)WeaponType);
			/*
				DrawWeaponIcon
			*/

			if (FrontEndMenuManager.m_PrefsShowHud) {
				if (weaponInfo->m_nModelId <= 0) {
					RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
					if (FrontEndMenuManager.m_PrefsShowHud)
						([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(Hud_Sprites[WeaponType]),
							Rect_Make(SCREEN_SCALE_FROM_RIGHT(99.0f), SCREEN_SCALE_Y(27.0f), SCREEN_SCALE_FROM_RIGHT(35.0f), SCREEN_SCALE_Y(91.0f)),
							CRGBA(255, 255, 255, alpha),
							0.015f, 0.015f,
							1.0f, 0.0f,
							0.015f, 1.0f,
							1.0f, 1.0f);
				} else {
					CBaseModelInfo *weaponModel = ((CBaseModelInfo*)ModelRegistry_Get(weaponInfo->m_nModelId)); // rouz edit (ChatGPT)
					// Use the complete C texture dictionary owner with the original live arguments
					RwTexDictionary *weaponTxd = ((RwTexDictionary*)TxdStore_GetSlot(weaponModel->GetTxdSlot())->texDict);
					if (weaponTxd) {
						RwTexture *weaponIcon = RwTexDictionaryFindNamedTexture(weaponTxd, weaponModel->GetModelName());
						if (weaponIcon) {
							RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERLINEAR);
#ifndef FIX_BUGS
							const float xSize = SCREEN_SCALE_X(64.0f / 2.0f);
							const float ySize = SCREEN_SCALE_Y(64.0f / 2.0f);
							RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE);
							RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RwTextureGetRaster(weaponIcon));
							// Use the complete C sprite operation with the original argument values
							Sprite_RenderOneXLUSprite(SCREEN_SCALE_FROM_RIGHT(99.0f) + xSize, SCREEN_SCALE_Y(25.0f) + ySize, 1.0f, xSize, ySize,
								255, 255, 255, 255, 1.0f, 255);
							RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)TRUE);
#else
							static Sprite2d sprite;
                            // Preserve the original local-static guard while initializing the C record and registering its cleanup
                            static const int sprite2dLocalStarted=[](){
                                // Initialize and register the same lazily owned C record
                                Sprite2d_Init(&sprite);Sprite2d_HudLocalTarget=&sprite;atexit(Sprite2d_ExitHudLocal);return 0;}();
                            (void)sprite2dLocalStarted;
							sprite.m_pTexture = weaponIcon;
							([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col,float u0,float v0,float u1,float v1,float u3,float v3,float u2,float v2){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRectUV(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a),u0,v0,u1,v1,u3,v3,u2,v2);
})(&(sprite),
								Rect_Make(SCREEN_SCALE_FROM_RIGHT(99.0f), SCREEN_SCALE_Y(27.0f), SCREEN_SCALE_FROM_RIGHT(35.0f), SCREEN_SCALE_Y(91.0f)),
								CRGBA(255, 255, 255, alpha),
								0.015f, 0.015f,
								1.0f, 0.0f,
								0.015f, 1.0f,
								1.0f, 1.0f);
							sprite.m_pTexture = nil;
#endif
						}
					}
				}

				Font_SetBackgroundOff();
				Font_SetScale(SCREEN_SCALE_X(0.5f), SCREEN_SCALE_Y(0.8f));
				Font_SetJustifyOff();
				Font_SetCentreOn();
				Font_SetCentreSize(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
				Font_SetPropOn();
				Font_SetDropShadowPosition(0);
				Font_SetFontStyle(FONT_STANDARD);

				if (Min(9999, TotalAmmo - AmmoInClip) != 9999 && !Darkel_FrenzyOnGoing() && weaponInfo->m_nWeaponSlot > 1 && weapon->m_eWeaponType != WEAPONTYPE_DETONATOR) {
					Font_SetDropShadowPosition(2);
					Font_SetDropColor(CRGBA(0, 0, 0, alpha));
					AMMO_COLOR.a = alpha;
					Font_SetColor(AMMO_COLOR);
					if (FrontEndMenuManager.m_PrefsShowHud)
						Font_PrintString(SCREEN_SCALE_FROM_RIGHT(66.0f), SCREEN_SCALE_Y(90.0f), sPrint);
					Font_SetDropShadowPosition(0);
				}
			}
		}

		/*
			DrawHealth
		*/
		if ( Hud_m_LastTimeEnergyLost == World_Players[World_PlayerInFocus].m_nTimeLastHealthLoss ) {
			Hud_DrawFadeState(HUD_ENERGY_FADING, 0);
		} else {
			Hud_DrawFadeState(HUD_ENERGY_FADING, 1);
			Hud_m_LastTimeEnergyLost = World_Players[World_PlayerInFocus].m_nTimeLastHealthLoss;
		}

		if (Hud_m_EnergyLostState != FADED_OUT) {
			Font_SetBackgroundOff();
			Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
			Font_SetJustifyOff();
			Font_SetCentreOff();
			Font_SetRightJustifyWrap(0.0f);
			Font_SetRightJustifyOn();
			Font_SetPropOff();
			Font_SetFontStyle(FONT_HEADING);
			Font_SetDropShadowPosition(2);
			Font_SetDropColor(CRGBA(0, 0, 0, 255));

			// Delegate the blinking tests to the C implementation
			if (Hud_ShowHealth(Hud_m_ItemToFlash == ITEM_HEALTH, FRAMECOUNTER, playerPed->m_fHealth)) {
				{

					AsciiToUnicode("{", sPrintIcon);
#ifdef FIX_BUGS
					sprintf(sTemp, "%03d", int32(playerPed->m_fHealth + 0.5f));
#else
					sprintf(sTemp, "%03d", (int32)playerPed->m_fHealth);
#endif
					AsciiToUnicode(sTemp, sPrint);

					Font_SetColor(HEALTH_COLOR);
					if (FrontEndMenuManager.m_PrefsShowHud) {
						Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f), SCREEN_SCALE_Y(65.0f), sPrint);

						if (Hud_ShowLossIcon(World_Players[World_PlayerInFocus].m_nTimeLastHealthLoss, (*Timer_GetTimeInMilliseconds()), FRAMECOUNTER)) {
							// CFont::SetColor(HEALTH_COLOR);
							Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f + 54.0f), SCREEN_SCALE_Y(65.0f), sPrintIcon);
						}
					}
				}
			}

			/*
				DrawArmour
			*/
			if (Hud_ShowArmour(Hud_m_ItemToFlash == ITEM_ARMOUR, FRAMECOUNTER)) {
				Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
				if (playerPed->m_fArmour > 1.0f) {
					AsciiToUnicode("<", sPrintIcon);
#ifdef FIX_BUGS
					sprintf(sTemp, "%03d", int32(playerPed->m_fArmour + 0.5f));
#else
					sprintf(sTemp, "%03d", (int32)playerPed->m_fArmour);
#endif
					AsciiToUnicode(sTemp, sPrint);

					Font_SetColor(ARMOUR_COLOR);
					if (FrontEndMenuManager.m_PrefsShowHud) {

						Font_PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f), SCREEN_SCALE_Y(65.0f), sPrint);

						if (Hud_ShowLossIcon(World_Players[World_PlayerInFocus].m_nTimeLastArmourLoss, (*Timer_GetTimeInMilliseconds()), FRAMECOUNTER)) {
							// CFont::SetColor(ARMOUR_COLOR);
							Font_PrintString(SCREEN_SCALE_FROM_RIGHT(182.0f + 52.0f), SCREEN_SCALE_Y(65.0f), sPrintIcon);
						}
					}
				}
			}
		}

		/*
			DrawWantedLevel
		*/
		if (Hud_m_LastWanted == Wanted_GetWantedLevel(playerPed->m_pWanted)) {
			alpha = Hud_DrawFadeState(HUD_WANTED_FADING, 0);
		} else {
			alpha = Hud_DrawFadeState(HUD_WANTED_FADING, 1);
			Hud_m_LastWanted = Wanted_GetWantedLevel(playerPed->m_pWanted);
		}

		if (Hud_m_WantedState != FADED_OUT) {
			Font_SetBackgroundOff();
			Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
			Font_SetJustifyOff();
			Font_SetCentreOff();
			Font_SetRightJustifyOn();
			Font_SetPropOn();
			Font_SetFontStyle(FONT_STANDARD);

			AsciiToUnicode(">", sPrintIcon);

			for (int i = 0; i < 6; i++) {
				if (FrontEndMenuManager.m_PrefsShowHud) {
					if (Wanted_GetWantedLevel(playerPed->m_pWanted) > i
						&& ((*Timer_GetTimeInMilliseconds()) > playerPed->m_pWanted->m_nLastWantedLevelChange
							+ 2000 || FRAMECOUNTER & 4)) {

						WANTED_COLOR.a = alpha;
						Font_SetColor(WANTED_COLOR);
						Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f + 23.0f * i), SCREEN_SCALE_Y(87.0f), sPrintIcon);

					} else if (playerPed->m_pWanted->m_nMinWantedLevel > i && FRAMECOUNTER & 4) {
						WANTED_COLOR_FLASH.a = alpha;
						Font_SetColor(WANTED_COLOR_FLASH);
						Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f + 23.0f * i), SCREEN_SCALE_Y(87.0f), sPrintIcon);

					} else if (Wanted_GetWantedLevel(playerPed->m_pWanted) <= i) {
						NOTWANTED_COLOR.a = alpha;
						Font_SetColor(NOTWANTED_COLOR);
						Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f + 23.0f * i), SCREEN_SCALE_Y(87.0f), sPrintIcon);
					}
				}
			}
		}

		static int32 nMediaLevelCounter = 0;
		if (Stats_ShowChaseStatOnScreen != 0) {
			float fCurAttentionLevel = World_Players[World_PlayerInFocus].m_fMediaAttention;
			if (0.7f * Stats_HighestChaseValue > fCurAttentionLevel
				|| fCurAttentionLevel <= 40.0f || TheScripts_IsPlayerOnAMission()) {
				nMediaLevelCounter = 0;
			}
			else {
				if (fCurAttentionLevel == Stats_HighestChaseValue) {
					sprintf(gString, "%s %d", UnicodeToAscii(Text_Get(&TheText,"CHSE")), (int32)fCurAttentionLevel);
				}
				else {
					sprintf(gString, "%s %d" "-%d-", UnicodeToAscii(Text_Get(&TheText,"CHSE")), (int32)fCurAttentionLevel, (int32)Stats_HighestChaseValue);
				}
				AsciiToUnicode(gString, gUString);
				Font_SetBackgroundOff();
				Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
				Font_SetCentreOff();
				Font_SetRightJustifyOn();
				Font_SetRightJustifyWrap(0.0f);
				Font_SetBackGroundOnlyTextOff();
				Font_SetFontStyle(FONT_HEADING);
				Font_SetPropOff();
				Font_SetDropShadowPosition(2);
				Font_SetDropColor(CRGBA(0, 0, 0, 255));

				CRGBA colour;
				if ((*Timer_GetTimeInMilliseconds()) & 0x200)
					colour = CRGBA(204, 0, 185, 180);
				else
					colour = CRGBA(178, 0, 162, 180);
				Font_SetColor(colour);
				Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f), SCREEN_SCALE_Y(113.0f), gUString);
				
				if (Stats_FindChaseString(fCurAttentionLevel) != prevChaseString) {
					prevChaseString = Stats_FindChaseString(fCurAttentionLevel);
					nMediaLevelCounter = 100;
				}

				if (nMediaLevelCounter != 0) {
					nMediaLevelCounter--;
					UnicodeMakeUpperCase(gUString, Stats_FindChaseString(fCurAttentionLevel));
					Font_PrintString(SCREEN_SCALE_FROM_RIGHT(110.0f), SCREEN_SCALE_Y(138.0f), gUString);
				}
			}
		}

		/*
			DrawZoneName
		*/
		if (Hud_m_pZoneName) {

			if (Hud_m_pZoneName != Hud_m_pLastZoneName) {
				// Delegate to the C implementation
				if (Hud_ZoneNameChanged(&Hud_m_ZoneState, &Hud_m_ZoneNameTimer, &Hud_m_ZoneFadeTimer, &Hud_m_VehicleState))
					Hud_m_ZoneToPrint = Hud_m_pZoneName;
				Hud_m_pLastZoneName = Hud_m_pZoneName;
			}

			float fZoneAlpha = 255.0f;
			if (Hud_m_ZoneState) {
				// Delegate to the C implementation
				bool restoreZonePrint;
				fZoneAlpha = Hud_ZoneNameFadeStep(&Hud_m_ZoneState, &Hud_m_ZoneFadeTimer, Hud_m_ZoneNameTimer, Timer_GetTimeStepInMilliseconds(), &restoreZonePrint);
				if (restoreZonePrint)
					Hud_m_ZoneToPrint = Hud_m_pLastZoneName;

				if (!Hud_m_Message[0] && Hud_BigMessageInUse[1] == 0.0f && Hud_BigMessageInUse[2] == 0.0f) {

					Hud_m_ZoneNameTimer += Timer_GetTimeStepInMilliseconds();
					Font_SetJustifyOff();
					Font_SetPropOn();
					Font_SetBackgroundOff();

					if (FrontEndMenuManager.m_PrefsLanguage == CMenuManager::LANGUAGE_SPANISH)
						Font_SetScale(SCREEN_SCALE_X(1.7f * 0.8f), SCREEN_SCALE_Y(1.8f));
					else
						Font_SetScale(SCREEN_SCALE_X(1.7f), SCREEN_SCALE_Y(1.8f));

					Font_SetSlantRefPoint(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(128.0f));
					Font_SetSlant(0.15f);

					Font_SetRightJustifyOn();
					Font_SetRightJustifyWrap(0.0f);
					Font_SetBackGroundOnlyTextOff();
					Font_SetDropShadowPosition(2);
					Font_SetDropColor(CRGBA(0, 0, 0, fZoneAlpha));
					Font_SetFontStyle(FONT_BANK);
					Font_SetColor(CRGBA(ZONE_COLOR.r, ZONE_COLOR.g, ZONE_COLOR.b, fZoneAlpha));

					if (!TheScripts_bPlayerIsInTheStatium)
						Font_PrintStringFromBottom(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(128.0f), Hud_m_ZoneToPrint);

					Font_SetSlant(0.f);
				} else {
					Hud_m_ZoneState = 3;
				}
			}
		}

		/*
			DrawVehicleName
		*/
		if (Hud_m_VehicleName) {
			float fVehicleAlpha = 0.0f;

			if (Hud_m_VehicleName != Hud_m_pLastVehicleName) {
				// Delegate to the C implementation
				if (Hud_VehicleNameChanged(&Hud_m_VehicleState, &Hud_m_VehicleNameTimer, &Hud_m_VehicleFadeTimer, &Hud_m_ZoneState))
					Hud_m_pVehicleNameToPrint = Hud_m_VehicleName;
				Hud_m_pLastVehicleName = Hud_m_VehicleName;
			}

			if (Hud_m_VehicleState) {
				// Delegate to the C implementation
				bool restoreVehiclePrint;
				fVehicleAlpha = Hud_VehicleNameFadeStep(&Hud_m_VehicleState, &Hud_m_VehicleFadeTimer, &Hud_m_VehicleNameTimer, Timer_GetTimeStepInMilliseconds(), &restoreVehiclePrint);
				if (restoreVehiclePrint)
					Hud_m_pVehicleNameToPrint = Hud_m_pLastVehicleName;

				if (!Hud_m_Message[0]) {
					Hud_m_VehicleNameTimer += Timer_GetTimeStepInMilliseconds();
					Font_SetJustifyOff();
					Font_SetPropOn();
					Font_SetBackgroundOff();

					if (FrontEndMenuManager.m_PrefsLanguage != CMenuManager::LANGUAGE_ITALIAN && FrontEndMenuManager.m_PrefsLanguage != CMenuManager::LANGUAGE_SPANISH)
						Font_SetScale(SCREEN_SCALE_X(1.7f), SCREEN_SCALE_Y(1.8f));
					else
						Font_SetScale(SCREEN_SCALE_X(1.7f * 0.85f), SCREEN_SCALE_Y(1.8f));

					Font_SetSlantRefPoint(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(105.0f));
					Font_SetSlant(0.15f);

					Font_SetRightJustifyOn();
					Font_SetRightJustifyWrap(0.0f);
					Font_SetBackGroundOnlyTextOff();
					Font_SetFontStyle(FONT_BANK);
					Font_SetDropShadowPosition(2);
					Font_SetColor(CRGBA(VEHICLE_COLOR.r, VEHICLE_COLOR.g, VEHICLE_COLOR.b, fVehicleAlpha));
					Font_SetDropColor(CRGBA(0, 0, 0, fVehicleAlpha));

					Font_PrintStringFromBottom(SCREEN_SCALE_FROM_RIGHT(32.0f), SCREEN_SCALE_FROM_BOTTOM(105.0f), Hud_m_pVehicleNameToPrint);

					Font_SetSlant(0.f);
				}
			}
		}
		else {
			Hud_m_pLastVehicleName = nil;
			Hud_m_VehicleState = 0;
			Hud_m_VehicleFadeTimer = 0;
			Hud_m_VehicleNameTimer = 0;
		}

		/*
			DrawClock
		*/
		if (Hud_m_ClockState) {
			Font_SetJustifyOff();
			Font_SetCentreOff();
			Font_SetBackgroundOff();
			Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
			Font_SetBackGroundOnlyTextOff();
			Font_SetPropOff();
			Font_SetFontStyle(FONT_HEADING);
			Font_SetRightJustifyOn();
			Font_SetRightJustifyWrap(0.0f);
			Font_SetDropShadowPosition(2);
			Font_SetDropColor(CRGBA(0, 0, 0, 255));

			sprintf(sTemp, "%02d:%02d", Clock_GetHours(), Clock_GetMinutes());
			AsciiToUnicode(sTemp, sPrint);

			Font_SetColor(CLOCK_COLOR);
			if (FrontEndMenuManager.m_PrefsShowHud)
				Font_PrintString(SCREEN_SCALE_FROM_RIGHT(111.0f), SCREEN_SCALE_Y(22.0f), sPrint);
		}

		/*
			DrawOnScreenTimer
		*/
		
		wchar sTimer[16];

		if (!OnscreenTimer_state.m_sClocks[0].m_bClockProcessed)
			Hud_TimerOnLastFrame = false;

		for(uint32 i = 0; i < NUMONSCREENCOUNTERS; i++) {
			if (!OnscreenTimer_state.m_sCounters[0].m_bCounterProcessed)
				Hud_CounterOnLastFrame[i] = false;
		}

		if (OnscreenTimer_state.m_bProcessed) {
			if (OnscreenTimer_state.m_sClocks[0].m_bClockProcessed) {
				// Delegate the blinking to the C implementation
				if (Hud_FlashTimerStep(&Hud_TimerOnLastFrame, &Hud_TimerFlashTimer, FRAMECOUNTER)) {
					AsciiToUnicode(OnscreenTimer_state.m_sClocks[0].m_aClockBuffer, sTimer);
					Font_SetPropOn();
					Font_SetBackgroundOff();
					Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
					Font_SetRightJustifyOn();
					Font_SetRightJustifyWrap(0.0f);
					Font_SetFontStyle(FONT_LOCALE(FONT_HEADING));
					Font_SetPropOff();
					Font_SetBackGroundOnlyTextOn();
					Font_SetDropShadowPosition(2);
					Font_SetDropColor(CRGBA(0, 0, 0, 255));
					Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
					Font_SetColor(TIMER_COLOR);
					Font_PrintString(SCREEN_SCALE_FROM_RIGHT(37.0f), SCREEN_SCALE_Y(110.0f), sTimer);
					Font_SetPropOn();

					if (OnscreenTimer_state.m_sClocks[0].m_aClockText[0]) {
						Font_SetDropShadowPosition(2);
						Font_SetDropColor(CRGBA(0, 0, 0, 255));
						Font_SetColor(TIMER_COLOR);
						Font_PrintString(SCREEN_SCALE_FROM_RIGHT(37.0f) - SCREEN_SCALE_X(80.0f), SCREEN_SCALE_Y(110.0f), Text_Get(&TheText,OnscreenTimer_state.m_sClocks[0].m_aClockText));
					}
				}
			}

			for(uint32 i = 0; i < NUMONSCREENCOUNTERS; i++) {
				if (OnscreenTimer_state.m_sCounters[i].m_bCounterProcessed) {
					// Delegate the blinking to the C implementation
					if (Hud_FlashTimerStep(&Hud_CounterOnLastFrame[i], &Hud_CounterFlashTimer[i], FRAMECOUNTER)) {
						if (OnscreenTimer_state.m_sCounters[i].m_nType == COUNTER_DISPLAY_NUMBER) {
							AsciiToUnicode(OnscreenTimer_state.m_sCounters[i].m_aCounterBuffer, sTimer);
							Font_SetPropOn();
							Font_SetBackgroundOff();
							Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
							Font_SetCentreOff();
							Font_SetRightJustifyOn();
							Font_SetRightJustifyWrap(0.0f);
							Font_SetFontStyle(FONT_LOCALE(FONT_HEADING));
							Font_SetWrapx(SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH));
							Font_SetPropOn();
							Font_SetBackGroundOnlyTextOn();
							Font_SetDropShadowPosition(2);
							Font_SetDropColor(CRGBA(0, 0, 0, 255));
							Font_SetColor(COUNTER_COLOR);
							Font_PrintString(SCREEN_SCALE_FROM_RIGHT(37.0f), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y * 20.f * i) + SCREEN_SCALE_Y(132.0f), sTimer);
						} else {
							int counter = atoi(OnscreenTimer_state.m_sCounters[i].m_aCounterBuffer);

							const float barWidth = SCREEN_SCALE_X(100.f / 2.f);
							const float right = SCREEN_SCALE_FROM_RIGHT(37.0f);
							const float left = right - barWidth;

							const float barHeight = SCREEN_SCALE_Y(11.0f);
							const float top = SCREEN_SCALE_Y(132.0f) + SCREEN_SCALE_Y(8.0f) + SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y * 20.f * i);
							const float bottom = top + barHeight;

							// shadow
							([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(Rect_Make(left + SCREEN_SCALE_X(6.0f), top + SCREEN_SCALE_Y(2.0f), right + SCREEN_SCALE_X(6.0f), bottom + SCREEN_SCALE_Y(2.0f)), CRGBA(0, 0, 0, 255));
							
							([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(Rect_Make(left + SCREEN_SCALE_X(4.0f), top, right + SCREEN_SCALE_X(4.0f), bottom), CRGBA(27, 89, 130, 255));
							([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(Rect_Make(left + SCREEN_SCALE_X(4.0f), top, left + SCREEN_SCALE_X(counter) / 2.0f + SCREEN_SCALE_X(4.0f), bottom), CRGBA(97, 194, 247, 255));
						}

						if (OnscreenTimer_state.m_sCounters[i].m_aCounterText[0]) {
							Font_SetPropOn();
							Font_SetFontStyle(FONT_LOCALE(FONT_HEADING));
							Font_SetScale(SCREEN_SCALE_X(HUD_TEXT_SCALE_X), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y));
							Font_SetDropShadowPosition(2);
							Font_SetDropColor(CRGBA(0, 0, 0, 255));
							Font_SetColor(COUNTER_COLOR);
							Font_PrintString(SCREEN_SCALE_FROM_RIGHT(37.0f) - SCREEN_SCALE_X(61.0f), SCREEN_SCALE_Y(HUD_TEXT_SCALE_Y * 20.f * i) + SCREEN_SCALE_Y(132.0f), Text_Get(&TheText,OnscreenTimer_state.m_sCounters[i].m_aCounterText));
						}
						// unused/leftover color. I wonder what was it for
						Font_SetColor(CRGBA(244, 225, 91, 255));
					}
				}
			}
		}

		/*
			DrawRadar
		*/
		if (FrontEndMenuManager.m_PrefsRadarMode != 2 &&
			!Hud_m_HideRadar && (Hud_m_ItemToFlash == ITEM_RADAR && FRAMECOUNTER & 8 || Hud_m_ItemToFlash != ITEM_RADAR)) {

			RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
			Radar_DrawMap();
			if (FrontEndMenuManager.m_PrefsRadarMode != 1) {
				CRect rect = Rect_Make(0.0f, 0.0f, SCREEN_SCALE_X(RADAR_WIDTH), SCREEN_SCALE_Y(RADAR_HEIGHT));
				
				Rect_Translate(&rect, SCREEN_SCALE_X_FIX(RADAR_LEFT), SCREEN_SCALE_FROM_BOTTOM(RADAR_BOTTOM + RADAR_HEIGHT));

#ifdef FIX_BUGS
				Rect_GrowEdges(&rect, SCREEN_SCALE_X(6.0f), SCREEN_SCALE_X(6.0f), SCREEN_SCALE_Y(6.0f), SCREEN_SCALE_Y(6.0f));
#else
				Rect_Grow(&rect, 6.0f);
#endif
				Rect_Translate(&rect, SCREEN_SCALE_X_FIX(0.0f), SCREEN_SCALE_Y_FIX(2.0f));
				([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRect(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(&(Hud_Sprites[HUD_RADARDISC]),rect, CRGBA(0, 0, 0, 255));
				Rect_Translate(&rect, SCREEN_SCALE_X_FIX(0.0f), SCREEN_SCALE_Y_FIX(-2.0f));
				([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRect(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(&(Hud_Sprites[HUD_RADARDISC]),rect, RADARDISC_COLOR);
			}
			Radar_DrawBlips();
		}
	}

	/*
		Draw3dMarkers
	*/
	if (Hud_m_Wants_To_Draw_3dMarkers && !TheCamera.m_WideScreenOn && !Hud_m_BigMessage[0][0] && !Hud_m_BigMessage[2][0]) {
		Radar_Draw3dMarkers();
	}

	/*
		DrawScriptText
	*/
	if (!Timer_GetIsUserPaused()) {
		for (int i = 0; i < ARRAY_SIZE(TheScripts_IntroTextLines); i++) {
			if (TheScripts_IntroTextLines[i].m_Text[0] && TheScripts_IntroTextLines[i].m_bTextBeforeFade) {
				Font_SetScale(SCREEN_SCALE_X(TheScripts_IntroTextLines[i].m_fScaleX), SCREEN_SCALE_Y(TheScripts_IntroTextLines[i].m_fScaleY * 0.5f));
				Font_SetColor(TheScripts_IntroTextLines[i].m_sColor);

				if (TheScripts_IntroTextLines[i].m_bJustify)
					Font_SetJustifyOn();
				else
					Font_SetJustifyOff();

				if (TheScripts_IntroTextLines[i].m_bRightJustify)
					Font_SetRightJustifyOn();
				else
					Font_SetRightJustifyOff();

				if (TheScripts_IntroTextLines[i].m_bCentered)
					Font_SetCentreOn();
				else
					Font_SetCentreOff();

				Font_SetWrapx(SCALE_AND_CENTER_X(TheScripts_IntroTextLines[i].m_fWrapX));
				Font_SetCentreSize(SCREEN_SCALE_X(TheScripts_IntroTextLines[i].m_fCenterSize));

				if (TheScripts_IntroTextLines[i].m_bBackground)
					Font_SetBackgroundOn();
				else
					Font_SetBackgroundOff();

				Font_SetBackgroundColor(TheScripts_IntroTextLines[i].m_sBackgroundColor);

				if (TheScripts_IntroTextLines[i].m_bBackgroundOnly)
					Font_SetBackGroundOnlyTextOn();
				else
					Font_SetBackGroundOnlyTextOff();

				if (TheScripts_IntroTextLines[i].m_bTextProportional)
					Font_SetPropOn();
				else
					Font_SetPropOff();

				Font_SetFontStyle(FONT_LOCALE(TheScripts_IntroTextLines[i].m_nFont));
				Font_PrintString(SCREEN_WIDTH - SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - TheScripts_IntroTextLines[i].m_fAtX), SCREEN_HEIGHT - SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT - TheScripts_IntroTextLines[i].m_fAtY), TheScripts_IntroTextLines[i].m_Text);
			}
		}
		for (int i = 0; i < ARRAY_SIZE(TheScripts_IntroRectangles); i++) {
			intro_script_rectangle &IntroRect = TheScripts_IntroRectangles[i];

			// Yeah, top and bottom changed place. R* vision
			if (IntroRect.m_bIsUsed && IntroRect.m_bBeforeFade) {
				if (IntroRect.m_nTextureId >= 0) {
					CRect rect = Rect_Make(
						IntroRect.m_sRect.left,
						IntroRect.m_sRect.bottom,
						IntroRect.m_sRect.right,
						IntroRect.m_sRect.top );

					([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRect(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(&(TheScripts_ScriptSprites[IntroRect.m_nTextureId]),rect, IntroRect.m_sColor);
				}
				else {
					CRect rect = Rect_Make(
						IntroRect.m_sRect.left,
						IntroRect.m_sRect.bottom,
						IntroRect.m_sRect.right,
						IntroRect.m_sRect.top );

					([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(rect, IntroRect.m_sColor);
				}
			}
		}

		/*
			DrawSubtitles
		*/
		if (Hud_m_Message[0] && !Hud_m_BigMessage[2][0]) {
			if (Hud_m_VehicleState != 0)
				Hud_m_VehicleState = 3;
			if (Hud_m_ZoneState != 0)
				Hud_m_ZoneState = 3;

			Font_SetJustifyOff();
			Font_SetBackgroundOff();
			Font_SetBackgroundColor(CRGBA(0, 0, 0, 128));
			Font_SetCentreOn();
			Font_SetPropOn();
#ifdef CUTSCENE_BORDERS_SWITCH
			if (!FrontEndMenuManager.m_PrefsCutsceneBorders) {
				Font_SetDropColor(CRGBA(0, 0, 0, 255));
				Font_SetDropShadowPosition(2);
			}
			else
#endif
				Font_SetDropShadowPosition(0);
			Font_SetFontStyle(FONT_LOCALE(FONT_STANDARD));
			Font_SetColor(CRGBA(225, 225, 225, 255));

			static bool onceItWasWidescreen = false;

			if (TheCamera.m_WideScreenOn) {
				onceItWasWidescreen = true;
				
				if (FrontEndMenuManager.m_PrefsShowSubtitles || !Cutscene_IsRunning()) { // rouz edit (ChatGPT)
					Font_SetCentreSize(SCREEN_WIDTH - SCREEN_SCALE_X(60.0f));
					Font_SetScale(SCREEN_SCALE_X(0.58f), SCREEN_SCALE_Y(1.2f));
					Font_PrintString(SCREEN_WIDTH / 2.f, SCREEN_SCALE_FROM_BOTTOM(80.0f), Hud_m_Message);
				}
			} else {
				if (onceItWasWidescreen)
					Hud_m_Message[0] = '\0';

				onceItWasWidescreen = false;
				Font_DrawFonts();
				Font_SetDropShadowPosition(2);
				Font_SetDropColor(CRGBA(0, 0, 0, 255));
				Font_SetScale(SCREEN_SCALE_X(0.58f), SCREEN_SCALE_Y(1.22f));

				float radarBulge = SCREEN_SCALE_X(140.0f) + SCREEN_SCALE_X(8.0f);
				float rectWidth = SCREEN_WIDTH - SCREEN_SCALE_X(20.0f) - SCREEN_SCALE_X(8.0f) - radarBulge;
				Font_SetCentreSize(rectWidth);

				Font_PrintString(rectWidth / 2.0f + radarBulge, SCREEN_SCALE_FROM_BOTTOM(105.f + 2.0f), Hud_m_Message);
			}
			Font_SetDropShadowPosition(0);
		}

		/*
			HelpMessage
		*/

		if (Hud_m_HelpMessage[0]) {
			if (!Messages_WideStringCompare(Hud_m_HelpMessage, Hud_m_LastHelpMessage, HELP_MSG_LENGTH)) {
				// Delegate to the C implementation
				if (Hud_HelpMessageChanged(&Hud_m_HelpMessageState, &Hud_m_HelpMessageFadeTimer, &Hud_m_HelpMessageTimer, &Hud_m_HelpMessageDisplayTime, Messages_GetWideStringLength(Hud_m_HelpMessage))) {
					Messages_WideStringCopy(Hud_m_HelpMessageToPrint, Hud_m_HelpMessage, HELP_MSG_LENGTH);
				
					if (TheCamera.m_ScreenReductionPercentage == 0.0f)
						DMAudio_PlayFrontEndSound(SOUND_HUD, 0);
				}
				Messages_WideStringCopy(Hud_m_LastHelpMessage, Hud_m_HelpMessage, HELP_MSG_LENGTH);
			}

			float fAlpha = 225.0f;

			if (Hud_m_HelpMessageState != 0) {
				// Delegate to the C implementation
				bool showNewHelpMessage;
				fAlpha = Hud_HelpMessageFadeStep(&Hud_m_HelpMessageState, Hud_m_HelpMessageTimer, &Hud_m_HelpMessageFadeTimer, Hud_m_HelpMessageDisplayTime, Hud_m_HelpMessageDisplayForever, Hud_m_HelpMessageQuick,
					TheCamera.m_WideScreenOn, Timer_GetTimeStepInMilliseconds(), &showNewHelpMessage);
				if (showNewHelpMessage)
					Messages_WideStringCopy(Hud_m_HelpMessageToPrint, Hud_m_LastHelpMessage, HELP_MSG_LENGTH);

				if (!TheCamera.m_WideScreenOn) {
					Hud_m_HelpMessageTimer += Timer_GetTimeStepInMilliseconds();

					Font_SetAlphaFade(fAlpha);
					Font_SetCentreOff();
					Font_SetPropOn();

					if (Game_germanGame)
						Font_SetScale(SCREEN_SCALE_X(0.52f * 0.85f), SCREEN_SCALE_Y(1.1f * 0.85f));
#ifdef MORE_LANGUAGES
					else if (Font_IsJapanese())
						Font_SetScale(SCREEN_SCALE_X(0.52f) * 1.35f, SCREEN_SCALE_Y(1.1f) * 1.25f);
#endif
					else
						Font_SetScale(SCREEN_SCALE_X(0.52f), SCREEN_SCALE_Y(1.1f));

					Font_DrawFonts();
					Font_SetColor(CRGBA(175, 175, 175, 255));
					Font_SetJustifyOff();
#ifdef MORE_LANGUAGES
					if (Font_IsJapanese())
						Font_SetWrapx(SCREEN_SCALE_X(229.0f + 34.0f - 4.0f));
					else
#endif
						Font_SetWrapx(SCREEN_SCALE_X(200.0f + 34.0f - 4.0f));
					Font_SetFontStyle(FONT_LOCALE(FONT_STANDARD));
					Font_SetBackgroundOn();
					Font_SetBackGroundOnlyTextOff();
					Font_SetDropShadowPosition(0);
					Font_SetBackgroundColor(CRGBA(0, 0, 0, fAlpha * 0.9f));
					Font_SetColor(CRGBA(175, 175, 175, 255));
					Font_PrintString(SCREEN_SCALE_X(34.0f), SCREEN_SCALE_Y(28.0f + (150.0f - Hud_PagerXOffset) * 0.6f), Hud_m_HelpMessageToPrint);
					Font_SetAlphaFade(255.0f);
					Font_SetWrapx(SCREEN_WIDTH);
				}
			}
		} else
			Hud_m_HelpMessageState = 0;

		/*
			DrawBigMessage
		*/
		// MissionCompleteFailedText
		if (Hud_m_BigMessage[0][0]) {
			if (Hud_BigMessageInUse[0] != 0.0f) {
				Font_SetJustifyOff();
				Font_SetBackgroundOff();
				Font_SetBackGroundOnlyTextOff();
				if (Game_frenchGame || Game_germanGame) {
					Font_SetScale(SCREEN_SCALE_X(1.6f), SCREEN_SCALE_Y(1.8f));
				} else {
					Font_SetScale(SCREEN_SCALE_X(1.8f), SCREEN_SCALE_Y(1.8f));
				}
				Font_SetPropOn();
				Font_SetCentreOn();
				Font_SetCentreSize(SCREEN_SCALE_X(590.0f));
				Font_SetColor(CRGBA(255, 255, 0, Hud_BigMessageAlpha[0])); // unused color
				Font_SetFontStyle(FONT_HEADING);

				// Appearently sliding text in here was abandoned very early, since this text is centered now.

				// Delegate to the C implementation
				bool clearBigMessage0;
				Hud_BigMessage0Step(&Hud_BigMessageX[0], &Hud_BigMessageInUse[0], &Hud_BigMessageAlpha[0], SCALE_AND_CENTER_X(620.0f), (*Timer_GetTimeStep()),
					Timer_GetTimeStepInMilliseconds(), SCREEN_SCALE_X((Timer_GetTimeStepInMilliseconds() * 0.3f)), &clearBigMessage0);
				if (clearBigMessage0)
					Hud_m_BigMessage[0][0] = 0;
				Font_DrawFonts();
				Font_SetDropShadowPosition(2);
				Font_SetDropColor(CRGBA(0, 0, 0, Hud_BigMessageAlpha[0]));
				Font_SetColor(CRGBA(BIGMESSAGE_COLOR.r, BIGMESSAGE_COLOR.g, BIGMESSAGE_COLOR.b, Hud_BigMessageAlpha[0]));
				Font_PrintString(SCREEN_WIDTH / 2, (SCREEN_HEIGHT / 2) - SCREEN_SCALE_Y(18.0f), Hud_m_BigMessage[0]);
			}
			else {
				Hud_BigMessageAlpha[0] = 0.0f;
				Hud_BigMessageX[0] = SCALE_AND_CENTER_X(-60.0f);
				Hud_BigMessageInUse[0] = 1.0f;
			}
		}
		else {
			Hud_BigMessageInUse[0] = 0.0f;
		}

		// WastedBustedText
		if (Hud_m_BigMessage[2][0]) {
			if (Hud_BigMessageInUse[2] != 0.0f) {
				// Delegate to the C implementation
				Hud_BigMessage2Step(&Hud_BigMessageAlpha[2], Timer_GetTimeStepInMilliseconds());

				Font_SetBackgroundOff();

				if (Game_frenchGame || Game_germanGame)
					Font_SetScale(SCREEN_SCALE_X(1.4f), SCREEN_SCALE_Y(1.4f));
				else
					Font_SetScale(SCREEN_SCALE_X(2.0f), SCREEN_SCALE_Y(2.0f));

				Font_SetPropOn();
				Font_SetRightJustifyOn();
				Font_SetFontStyle(FONT_HEADING);

				Font_SetDropShadowPosition(2);
				Font_SetDropColor(CRGBA(0, 0, 0, Hud_BigMessageAlpha[2]));

				Font_SetColor(CRGBA(WASTEDBUSTED_COLOR.r, WASTEDBUSTED_COLOR.g, WASTEDBUSTED_COLOR.b, Hud_BigMessageAlpha[2]));
				Font_PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(90.0f), Hud_m_BigMessage[2]);
			}
			else {
				Hud_BigMessageInUse[2] = 1.0f;
				Hud_BigMessageAlpha[2] = 0.0f;
				if (Hud_m_VehicleState != 0) // Hide vehicle name if wasted/busted text is displaying
					Hud_m_VehicleState = 0;
				if (Hud_m_ZoneState != 0)
					Hud_m_ZoneState = 0;
			}
		}
		else {
			Hud_BigMessageInUse[2] = 0.0f;
		}
	}
}

void Hud_DrawAfterFade()
{
	RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, (void*)rwFILTERNEAREST);
	RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

	if (Timer_GetIsUserPaused() || Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;

	for (int i = 0; i < ARRAY_SIZE(TheScripts_IntroTextLines); i++) {
		intro_text_line &line = TheScripts_IntroTextLines[i];
		if (line.m_Text[0] != '\0' && !line.m_bTextBeforeFade) {
			Font_SetScale(SCREEN_SCALE_X(line.m_fScaleX), SCREEN_SCALE_Y(line.m_fScaleY) / 2);

			Font_SetColor(line.m_sColor);
			if (line.m_bJustify)
				Font_SetJustifyOn();
			else
				Font_SetJustifyOff();

			if (line.m_bRightJustify)
				Font_SetRightJustifyOn();
			else
				Font_SetRightJustifyOff();

			if (line.m_bCentered)
				Font_SetCentreOn();
			else
				Font_SetCentreOff();

			Font_SetWrapx(SCALE_AND_CENTER_X(line.m_fWrapX));
			Font_SetCentreSize(SCREEN_SCALE_X(line.m_fCenterSize));
			if (line.m_bBackground)
				Font_SetBackgroundOn();
			else
				Font_SetBackgroundOff();

			Font_SetBackgroundColor(line.m_sBackgroundColor);
			if (line.m_bBackgroundOnly)
				Font_SetBackGroundOnlyTextOn();
			else
				Font_SetBackGroundOnlyTextOff();

			if (line.m_bTextProportional)
				Font_SetPropOn();
			else
				Font_SetPropOff();

			Font_SetFontStyle(line.m_nFont);
			Font_PrintString(SCREEN_WIDTH - SCALE_AND_CENTER_X(DEFAULT_SCREEN_WIDTH - line.m_fAtX), SCREEN_HEIGHT - SCREEN_SCALE_Y(DEFAULT_SCREEN_HEIGHT - line.m_fAtY), line.m_Text);
		}
	}
	for (int i = 0; i < ARRAY_SIZE(TheScripts_IntroRectangles); i++) {
		intro_script_rectangle &rectangle = TheScripts_IntroRectangles[i];
		if (rectangle.m_bIsUsed && !rectangle.m_bBeforeFade) {

			// Yeah, top and bottom changed place. R* vision
			if (rectangle.m_nTextureId >= 0) {
				([](Sprite2d *spriteSelf,const CRect &rect,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawRect(spriteSelf,&rect,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(&(TheScripts_ScriptSprites[rectangle.m_nTextureId]),Rect_Make(rectangle.m_sRect.left, rectangle.m_sRect.bottom,
					rectangle.m_sRect.right, rectangle.m_sRect.top), rectangle.m_sColor);
			} else {
				([](const CRect &r,const CRGBA &col){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_DrawSolidRect(&r,Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a));
})(Rect_Make(rectangle.m_sRect.left, rectangle.m_sRect.bottom,
					rectangle.m_sRect.right, rectangle.m_sRect.top), rectangle.m_sColor);
			}
		}
	}

	/*
		DrawBigMessage2
	*/
	// Oddjob
	if (Hud_m_BigMessage[3][0]) {
		Font_SetJustifyOff();
		Font_SetBackgroundOff();
		Font_SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.5f));
		Font_SetCentreOn();
		Font_SetPropOn();
		Font_SetCentreSize(SCREEN_SCALE_X(600.0f));
		Font_SetFontStyle(FONT_LOCALE(FONT_STANDARD));
		Font_SetDropShadowPosition(2);
		Font_SetDropColor(CRGBA(0, 0, 0, 255));
		Font_SetColor(ODDJOB_COLOR);
		Font_PrintString((SCREEN_WIDTH / 2), SCREEN_SCALE_Y(140.0f) - SCREEN_SCALE_Y(16.0f), Hud_m_BigMessage[3]);
	}

	if (!Hud_m_BigMessage[1][0] && Hud_m_BigMessage[4][0]) {
		Font_SetJustifyOff();
		Font_SetBackgroundOff();
		Font_SetScale(SCREEN_SCALE_X(1.2f), SCREEN_SCALE_Y(1.5f));
		Font_SetCentreOn();
		Font_SetPropOn();
		Font_SetCentreSize(SCREEN_SCALE_X(580.0f));
		Font_SetFontStyle(FONT_LOCALE(FONT_STANDARD));
		Font_SetDropShadowPosition(2);
		Font_SetDropColor(CRGBA(0, 0, 0, 255));
		Font_SetColor(ODDJOB_COLOR);
		Font_PrintString((SCREEN_WIDTH / 2), SCREEN_SCALE_Y(140.0f), Hud_m_BigMessage[4]);
	}

	// Oddjob result
	// Delegate the sliding of the message to the C implementation
	if (Hud_OddJob2Step(&Hud_OddJob2OffTimer, &Hud_OddJob2On, &Hud_OddJob2XOffset, &Hud_OddJob2Timer, Hud_m_BigMessage[5][0] != 0, Timer_GetTimeStepInMilliseconds())) {
		if (!Hud_m_BigMessage[1][0]) {
			Font_SetJustifyOff();
			Font_SetBackgroundOff();
			Font_SetScale(SCREEN_SCALE_X(1.0f), SCREEN_SCALE_Y(1.2f));
			Font_SetCentreOn();
			Font_SetPropOn();
			Font_SetCentreSize(SCREEN_SCALE_X(560.0f));
			Font_SetFontStyle(FONT_LOCALE(FONT_STANDARD));
			Font_SetDropShadowPosition(2);
			Font_SetDropColor(CRGBA(0, 0, 0, 255));
			Font_SetColor(ODDJOB2_COLOR);
			Font_PrintString(SCREEN_WIDTH / 2, SCREEN_SCALE_Y(217.0f), Hud_m_BigMessage[5]);
		}
	}

	/*
		DrawMissionTitle
	*/
	if (Hud_m_BigMessage[1][0]) {
		if (Hud_BigMessageInUse[1] != 0.0f) {
			Font_SetJustifyOff();
			Font_SetBackgroundOff();

			// will be overwritten below
			if (Game_frenchGame || FrontEndMenuManager.m_PrefsLanguage == CMenuManager::LANGUAGE_SPANISH)
				Font_SetScale(SCREEN_SCALE_X(0.884f), SCREEN_SCALE_Y(1.36f));
			else
				Font_SetScale(SCREEN_SCALE_X(1.04f), SCREEN_SCALE_Y(1.6f));

			Font_SetPropOn();
			Font_SetRightJustifyWrap(SCALE_AND_CENTER_X(0.0f));
			Font_SetRightJustifyOn();
			Font_SetFontStyle(FONT_BANK);
			Font_SetScale(FrontEndMenuManager.m_PrefsLanguage == CMenuManager::LANGUAGE_AMERICAN ? SCREEN_SCALE_X(1.7f) : SCREEN_SCALE_X(1.5f), SCREEN_SCALE_Y(1.8f));

			// Delegate to the C implementation
			bool clearMissionTitle;
			Hud_MissionTitleStep(&Hud_BigMessageX[1], &Hud_BigMessageInUse[1], &Hud_BigMessageAlpha[1], SCREEN_SCALE_FROM_RIGHT(20.0f), (*Timer_GetTimeStep()),
				Timer_GetTimeStepInMilliseconds(), SCREEN_SCALE_X((Timer_GetTimeStepInMilliseconds() * 0.3f)), &clearMissionTitle);
			if (clearMissionTitle)
				Hud_m_BigMessage[1][0] = 0;

			Font_SetColor(CRGBA(40, 40, 40, Hud_BigMessageAlpha[1])); // what was that for?

			Font_SetDropShadowPosition(2);
			Font_SetDropColor(CRGBA(0, 0, 0, Hud_BigMessageAlpha[1]));
			Font_SetColor(CRGBA(MISSIONTITLE_COLOR.r, MISSIONTITLE_COLOR.g, MISSIONTITLE_COLOR.b, Hud_BigMessageAlpha[1]));
			Font_PrintString(SCREEN_SCALE_FROM_RIGHT(20.0f), SCREEN_SCALE_FROM_BOTTOM(140.0f), Hud_m_BigMessage[1]);
		} else {
			Hud_m_ZoneFadeTimer = 0;
			Hud_BigMessageX[1] = SCREEN_SCALE_FROM_RIGHT(DEFAULT_SCREEN_WIDTH + 60.0f);
			Hud_BigMessageInUse[1] = 1.0f;
			Hud_m_ZoneState = 0;
		}
	} else {
		Hud_BigMessageInUse[1] = 0.0f;
	}
}

void Hud_GetRidOfAllHudMessages()
{
	Hud_m_ZoneNameTimer = 0;
	Hud_m_pZoneName = nil;
	Hud_m_ZoneState = 0;

	for (int i = 0; i < HELP_MSG_LENGTH; i++) {
		Hud_m_HelpMessage[i] = 0;
		Hud_m_LastHelpMessage[i] = 0;
		Hud_m_HelpMessageToPrint[i] = 0;
	}

	Hud_m_HelpMessageTimer = 0;
	Hud_m_HelpMessageFadeTimer = 0;
	Hud_m_HelpMessageState = 0;
	Hud_m_HelpMessageQuick = 0;
	Hud_m_HelpMessageDisplayForever = false;
	Hud_m_HelpMessageDisplayTime = 1.0f;
	Hud_m_VehicleName = nil;
	Hud_m_pVehicleNameToPrint = nil;
	Hud_m_VehicleNameTimer = 0;
	Hud_m_VehicleFadeTimer = 0;
	Hud_m_VehicleState = 0;

	for (int i = 0; i < ARRAY_SIZE(Hud_m_Message); i++)
		Hud_m_Message[i] = 0;

	for (int i = 0; i < 6; i++) {
		Hud_BigMessageInUse[i] = 0.0f;

		for (int j = 0; j < 128; j++)
			Hud_m_BigMessage[i][j] = 0;
	}
}

#ifdef RELOADABLES
void Hud_ReloadTXD()
{
	for (int i = 0; i < NUM_HUD_SPRITES; ++i) {
		([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Hud_Sprites[i]));
	}

	// Use the complete C texture dictionary owner with the original live arguments
	int HudTXD = TxdStore_FindTxdSlot("hud");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_RemoveTxdSlot(HudTXD);

	debug("Reloading HUD.TXD...\n");

	// Use the complete C texture dictionary owner with the original live arguments
	HudTXD = TxdStore_AddTxdSlot("hud");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_LoadFile(HudTXD, "MODELS/HUD.TXD");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_AddRef(HudTXD);
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PopCurrentTxd();
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_SetCurrentTxd(HudTXD);

	for (int i = 0; i < NUM_HUD_SPRITES; i++) {
		([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Hud_Sprites[i]),WeaponFilenames[i].name, WeaponFilenames[i].mask);
	}
}
#endif

void Hud_Initialise()
{
	Hud_m_Wants_To_Draw_Hud = true;
	Hud_m_Wants_To_Draw_3dMarkers = true;

	// Use the complete C texture dictionary owner with the original live arguments
	int HudTXD = TxdStore_AddTxdSlot("hud");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_LoadFile(HudTXD, "MODELS/HUD.TXD");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_AddRef(HudTXD);
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PopCurrentTxd();
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_SetCurrentTxd(HudTXD);

	for (int i = 0; i < NUM_HUD_SPRITES; i++) {
		([](Sprite2d *spriteSelf,const char *name,const char *mask){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_SetTextureMasked(spriteSelf,name,mask);
})(&(Hud_Sprites[i]),WeaponFilenames[i].name, WeaponFilenames[i].mask);
	}

	Hud_m_pLastZoneName = nil;
	Hud_GetRidOfAllHudMessages();
	Hud_m_pLastVehicleName = nil;

	if (gpSniperSightTex == nil)
		gpSniperSightTex = RwTextureRead("sitesniper", nil); // unused
	if (gpRocketSightTex == nil)
		gpRocketSightTex = RwTextureRead("siterocket", nil);
	if (gpLaserSightTex == nil)
		gpLaserSightTex = RwTextureRead("sitelaser", nil); // unused
	if (gpLaserDotTex == nil)
		gpLaserDotTex = RwTextureRead("laserdot", "laserdotm");
	if (gpViewFinderTex == nil)
		gpViewFinderTex = RwTextureRead("viewfinder_128", "viewfinder_128m"); // unused

	Hud_m_ClockState = 1;
	Hud_CounterOnLastFrame[0] = false;
	Hud_CounterOnLastFrame[1] = false;
	Hud_CounterOnLastFrame[2] = false;
	
	Hud_m_ItemToFlash = ITEM_NONE;
	Hud_OddJob2Timer = 0;
	Hud_OddJob2OffTimer = 0.0f;
	Hud_OddJob2On = 0;
	Hud_OddJob2XOffset = 0.0f;
	Hud_CounterFlashTimer[0] = 0;
	Hud_CounterFlashTimer[1] = 0;
	Hud_CounterFlashTimer[2] = 0;
	Hud_TimerOnLastFrame = false;
	Hud_TimerFlashTimer = 0;
	Hud_SpriteBrightness = 0;
	Hud_PagerOn = 0;
	Hud_PagerTimer = 0;
	Hud_PagerSoundPlayed = 0;
	Hud_PagerXOffset = 150.0f;

#ifdef HUD_AUTO_FADE
	Hud_m_EnergyLostState = START_FADE_OUT;
	Hud_m_WantedState = START_FADE_OUT;
	Hud_m_DisplayScoreState = START_FADE_OUT;
	Hud_m_WeaponState = START_FADE_OUT;
#else
	Hud_m_EnergyLostState = FADE_DISABLED;
	Hud_m_WantedState = FADE_DISABLED;
	Hud_m_DisplayScoreState = FADE_DISABLED;
	Hud_m_WeaponState = FADE_DISABLED;
#endif
	Hud_m_WantedFadeTimer = 0;
	Hud_m_WantedTimer = 0;
	Hud_m_EnergyLostFadeTimer = 0;
	Hud_m_EnergyLostTimer = 0;
	Hud_m_DisplayScoreFadeTimer = 0;
	Hud_m_DisplayScoreTimer = 0;
	Hud_m_WeaponFadeTimer = 0;
	Hud_m_WeaponTimer = 0;

	Hud_m_HideRadar = false;
	Hud_m_LastDisplayScore = World_Players[World_PlayerInFocus].m_nVisibleMoney;
	Hud_m_LastTimeEnergyLost = World_Players[World_PlayerInFocus].m_nTimeLastHealthLoss;
	Hud_m_LastWanted = 0;
	Hud_m_LastWeapon = 0;

#ifndef MASTER
	VarConsole_AddBool(&VarConsole,"Draw HUD", &Hud_m_Wants_To_Draw_Hud, false); // rouz edit (ChatGPT)
#endif
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PopCurrentTxd();
}

void Hud_ReInitialise() {
	Hud_m_Wants_To_Draw_Hud = true;
	Hud_m_Wants_To_Draw_3dMarkers = true;

	Hud_m_pLastZoneName = nil;
	Hud_GetRidOfAllHudMessages();
	Hud_m_pLastVehicleName = nil;

	Hud_CounterOnLastFrame[0] = false;
	Hud_CounterOnLastFrame[1] = false;
	Hud_CounterOnLastFrame[2] = false;
	Hud_m_ItemToFlash = ITEM_NONE;
	Hud_m_ClockState = 1;
	Hud_OddJob2Timer = 0;
	Hud_OddJob2OffTimer = 0.0f;
	Hud_OddJob2On = 0;
	Hud_OddJob2XOffset = 0.0f;
	Hud_CounterFlashTimer[0] = 0;
	Hud_CounterFlashTimer[1] = 0;
	Hud_CounterFlashTimer[2] = 0;
	Hud_TimerOnLastFrame = false;
	Hud_TimerFlashTimer = 0;
	Hud_SpriteBrightness = 0;
	Hud_PagerOn = 0;
	Hud_PagerTimer = 0;
	Hud_PagerSoundPlayed = 0;
	Hud_PagerXOffset = 150.0f;

#ifdef HUD_AUTO_FADE
	Hud_m_EnergyLostState = START_FADE_OUT;
	Hud_m_WantedState = START_FADE_OUT;
	Hud_m_DisplayScoreState = START_FADE_OUT;
	Hud_m_WeaponState = START_FADE_OUT;
#else
	Hud_m_EnergyLostState = FADE_DISABLED;
	Hud_m_WantedState = FADE_DISABLED;
	Hud_m_DisplayScoreState = FADE_DISABLED;
	Hud_m_WeaponState = FADE_DISABLED;
#endif
	Hud_m_WantedFadeTimer = 0;
	Hud_m_WantedTimer = 0;
	Hud_m_EnergyLostFadeTimer = 0;
	Hud_m_EnergyLostTimer = 0;
	Hud_m_DisplayScoreFadeTimer = 0;
	Hud_m_DisplayScoreTimer = 0;
	Hud_m_WeaponFadeTimer = 0;
	Hud_m_WeaponTimer = 0;

	Hud_m_HideRadar = false;
	Hud_m_LastDisplayScore = World_Players[World_PlayerInFocus].m_nVisibleMoney;
	Hud_m_LastTimeEnergyLost = World_Players[World_PlayerInFocus].m_nTimeLastHealthLoss;
	Hud_m_LastWanted = 0;
	Hud_m_LastWeapon = 0;
}

wchar LastBigMessage[6][128];

void Hud_SetBigMessage(wchar *message, uint16 style)
{
	if (Hud_BigMessageInUse[style] != 0.0f)
		return;

	// Delegate to the C implementation
	if (Hud_StoreBigMessage(Hud_m_BigMessage[style], LastBigMessage[style], message, style == 5)) {
		Hud_OddJob2On = 0;
		Hud_OddJob2OffTimer = 0.0f;
	}
}

void Hud_SetHelpMessage(wchar *message, bool quick, bool displayForever)
{
	if (!Replay_IsPlayingBack()) { // rouz edit (ChatGPT)
		for (int i = 0; i < HELP_MSG_LENGTH; i++) {
			Hud_m_HelpMessage[i] = 0;
		}
		for (int i = 0; i < HELP_MSG_LENGTH; i++) {
			Hud_m_LastHelpMessage[i] = 0;
		}
		for (int i = 0; i < HELP_MSG_LENGTH; i++) {
			Hud_m_HelpMessageToPrint[i] = 0;
		}

		Messages_WideStringCopy(Hud_m_HelpMessage, message, HELP_MSG_LENGTH);
		Messages_InsertPlayerControlKeysInString(Hud_m_HelpMessage);
		if (Hud_m_HelpMessageState == 0 || !Messages_WideStringCompare(Hud_m_HelpMessage, Hud_m_HelpMessageToPrint, HELP_MSG_LENGTH)) {
			for (int i = 0; i < HELP_MSG_LENGTH; i++) {
				Hud_m_LastHelpMessage[i] = 0;
			}

			if (!message) {
				Hud_m_HelpMessage[0] = 0;
				Hud_m_HelpMessageToPrint[0] = 0;
			}
			if (!displayForever) {
				Hud_m_HelpMessageState = displayForever;
			} else {
				Hud_m_HelpMessageState = 1;
				Messages_WideStringCopy(Hud_m_HelpMessageToPrint, Hud_m_HelpMessage, HELP_MSG_LENGTH);
				Messages_WideStringCopy(Hud_m_LastHelpMessage, Hud_m_HelpMessage, HELP_MSG_LENGTH);
			}

			Hud_m_HelpMessageQuick = quick;
			Hud_m_HelpMessageDisplayForever = displayForever;
		}

	}
}

bool Hud_IsHelpMessageBeingDisplayed(void)
{
	return Hud_m_HelpMessageState != 0;
}

void Hud_SetMessage(wchar *message)
{
	// Delegate to the C implementation
	Hud_CopyMessage(Hud_m_Message, message, ARRAY_SIZE(Hud_m_Message));
}

void Hud_SetPagerMessage(wchar *message)
{
	// Delegate to the C implementation
	Hud_CopyMessage(Hud_m_PagerMessage, message, ARRAY_SIZE(Hud_m_PagerMessage));
}

void Hud_SetVehicleName(wchar *name)
{
	Hud_m_VehicleName = name;
}

void Hud_SetZoneName(wchar *name)
{
	Hud_m_pZoneName = name;
}

void Hud_Shutdown()
{
	for (int i = 0; i < NUM_HUD_SPRITES; ++i) {
		([](Sprite2d *spriteSelf){
    // Retain actual native reference objects through the complete synchronous C operation
    Sprite2d_Delete(spriteSelf);
})(&(Hud_Sprites[i]));
	}

	RwTextureDestroy(gpSniperSightTex);
	gpSniperSightTex = nil;

	RwTextureDestroy(gpRocketSightTex);
	gpRocketSightTex = nil;

	RwTextureDestroy(gpLaserSightTex);
	gpLaserSightTex = nil;

	RwTextureDestroy(gpLaserDotTex);
	gpLaserDotTex = nil;

	RwTextureDestroy(gpViewFinderTex);
	gpViewFinderTex = nil;

	// Use the complete C texture dictionary owner with the original live arguments
	int HudTXD = TxdStore_FindTxdSlot("hud");
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_RemoveTxdSlot(HudTXD);
}

float Hud_DrawFadeState(DRAW_FADE_STATE fadingElement, int forceFadingIn)
{
	float alpha = 255.0f;
	uint32 operation, timer;
	int32 fadeTimer;

	switch (fadingElement) {
		case HUD_WANTED_FADING:
			fadeTimer = Hud_m_WantedFadeTimer;
			operation = Hud_m_WantedState;
			timer = Hud_m_WantedTimer;
			break;
		case HUD_ENERGY_FADING:
			fadeTimer = Hud_m_EnergyLostFadeTimer;
			operation = Hud_m_EnergyLostState;
			timer = Hud_m_EnergyLostTimer;
			break;
		case HUD_SCORE_FADING:
			fadeTimer = Hud_m_DisplayScoreFadeTimer;
			operation = Hud_m_DisplayScoreState;
			timer = Hud_m_DisplayScoreTimer;
			break;
		case HUD_WEAPON_FADING:
			fadeTimer = Hud_m_WeaponFadeTimer;
			operation = Hud_m_WeaponState;
			timer = Hud_m_WeaponTimer;
			break;
		default:
			break;
	}
	// The fade step itself is done in C
	alpha = Hud_DrawFadeStep(&fadeTimer, &operation, &timer, forceFadingIn, Timer_GetTimeStepInMilliseconds());

	switch (fadingElement) {
		case HUD_WANTED_FADING:
			Hud_m_WantedFadeTimer = fadeTimer;
			Hud_m_WantedState = operation;
			Hud_m_WantedTimer = timer;
			break;
		case HUD_ENERGY_FADING:
			Hud_m_EnergyLostFadeTimer = fadeTimer;
			Hud_m_EnergyLostState = operation;
			Hud_m_EnergyLostTimer = timer;
			break;
		case HUD_SCORE_FADING:
			Hud_m_DisplayScoreFadeTimer = fadeTimer;
			Hud_m_DisplayScoreState = operation;
			Hud_m_DisplayScoreTimer = timer;
			break;
		case HUD_WEAPON_FADING:
			Hud_m_WeaponFadeTimer = fadeTimer;
			Hud_m_WeaponState = operation;
			Hud_m_WeaponTimer = timer;
			break;
		default:
			break;
	}

	return alpha;
}

void
Hud_ResetWastedText(void)
{
	Hud_BigMessageInUse[2] = 0.0f;
	Hud_BigMessageInUse[0] = 0.0f;
	Hud_m_BigMessage[2][0] = 0;
	Hud_m_BigMessage[0][0] = 0;
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
