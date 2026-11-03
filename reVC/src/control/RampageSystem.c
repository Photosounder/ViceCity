//+ rouz edit (ChatGPT)
#include "ReplayState.h"
#include "ModelRegistry.h"
#include "Darkel_backend.h"
#include "SharedText.h"
#include "FontText.h"
#include "Timer.h"
#include "Text.h"
#include "DMAudio.h"
#include "Draw.h"
#include "Draw_backend.h"
#include "Draw_constants.h"
#include "FontStyles.h"
#include "PedType.h"
#include "VehicleAppearance.h"
#include <stdio.h>
#define FRENZY_ANY_PED -1
#define FRENZY_ANY_CAR -2
#define Min(a,b) ((a)<(b)?(a):(b))
#define SCREEN_WIDTH (Draw_BackendScreenWidth())
#define SCREEN_HEIGHT (Draw_BackendScreenHeight())
#define SCREEN_STRETCH_X(a) ((a)*(float)SCREEN_WIDTH/DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a) ((a)*(float)SCREEN_HEIGHT/DEFAULT_SCREEN_HEIGHT)
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_RIGHT(a) (SCREEN_WIDTH-SCREEN_SCALE_X(a))
#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a)*DEFAULT_ASPECT_RATIO/Draw_GetAspectRatio())
#if defined(PROPER_SCALING) && !defined(FORCE_PC_SCALING)
#undef SCREEN_SCALE_Y
#define SCREEN_SCALE_Y(a) Draw_ScaleY(SCREEN_STRETCH_Y(a))
#endif
#else
#define SCREEN_SCALE_AR(a) (a)
#endif
int32_t Darkel_TimeLimit;
int32_t Darkel_PreviousTime;
int32_t Darkel_TimeOfFrenzyStart;
int32_t Darkel_WeaponType;
int32_t Darkel_AmmoInterruptedWeapon;
int32_t Darkel_KillsNeeded;
int32_t Darkel_InterruptedWeaponType;
int32_t Darkel_InterruptedWeaponSelected;
bool Darkel_bStandardSoundAndMessages;
bool Darkel_bNeedHeadShot;
bool Darkel_bProperKillFrenzy;
uint16_t Darkel_Status;
uint16_t Darkel_RegisteredKills[NUM_DEFAULT_MODELS];
int32_t Darkel_ModelToKill;
int32_t Darkel_ModelToKill2;
int32_t Darkel_ModelToKill3;
int32_t Darkel_ModelToKill4;
uint16_t *Darkel_pStartMessage;
uint8_t Darkel_CalcFade(uint32_t time,uint32_t start,uint32_t end)
{
    // Preserve the complete original CalcFade policy and each ordered external access
	if (time >= start && time <= end) {
		if (time >= start + 500) {
			if (time <= end - 500)
				return 255;
			else
				return 255 * (end - time) / 500;
		} else
			return 255 * (time - start) / 500;
	} else
		return 0;
}

void Darkel_DrawMessages(void)
{
    // Preserve the complete original DrawMessages policy and each ordered external access
	if (Replay_IsPlayingBack())
		return;

	switch (Darkel_Status) {
		case KILLFRENZY_ONGOING:
		{
			Darkel_BackendJustifyOff();
			Darkel_BackendBackgroundOff();
#ifdef FIX_BUGS
			Darkel_BackendCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 30));
#else
			Darkel_BackendCentreSize(SCREEN_WIDTH - 30);
#endif
			Darkel_BackendCentreOn();
			Darkel_BackendPropOn();
			uint32_t timePassedSinceStart = (*Timer_GetTimeInMilliseconds()) - Darkel_TimeOfFrenzyStart;
			if (Darkel_bStandardSoundAndMessages) {
				if (timePassedSinceStart >= 3000 && timePassedSinceStart < 11000) {
#ifdef FIX_BUGS
					Darkel_BackendScale(SCREEN_SCALE_X(1.3f), SCREEN_SCALE_Y(1.3f));
#else
					Darkel_BackendScale(1.3f, 1.3f);
#endif
					Darkel_BackendJustifyOff();
					Darkel_BackendColor(255, 255, 128, Darkel_CalcFade(timePassedSinceStart, 3000, 11000));
					Darkel_BackendFontStyle(FONT_STANDARD);
					if (Darkel_pStartMessage) {
						Darkel_BackendPrint(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, Darkel_pStartMessage);
					}
				}
			} else {
				if (timePassedSinceStart < 8000) {
#ifdef FIX_BUGS
					Darkel_BackendScale(SCREEN_SCALE_X(1.3f), SCREEN_SCALE_Y(1.3f));
#else
					Darkel_BackendScale(1.3f, 1.3f);
#endif
					Darkel_BackendJustifyOff();
					Darkel_BackendColor(255, 255, 128, Darkel_CalcFade(timePassedSinceStart, 0, 8000));
					Darkel_BackendFontStyle(FONT_STANDARD);
					if (Darkel_pStartMessage) {
						Darkel_BackendPrint(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, Darkel_pStartMessage);
					}
				}
			}
#ifdef FIX_BUGS
			Darkel_BackendScale(SCREEN_SCALE_X(0.75f), SCREEN_SCALE_Y(1.5f));
#else
			Darkel_BackendScale(0.75f, 1.5f);
#endif
			Darkel_BackendCentreOff();
			Darkel_BackendRightJustifyOn();
			Darkel_BackendFontStyle(FONT_HEADING);
			if (Darkel_TimeLimit >= 0) {
				uint32_t timeLeft = Darkel_TimeLimit - ((*Timer_GetTimeInMilliseconds()) - Darkel_TimeOfFrenzyStart);
				sprintf(gString, "%d:%02d", timeLeft / 60000, timeLeft % 60000 / 1000);
				AsciiToUnicode(gString, gUString);
				if (timeLeft > 4000 || (*Timer_GetFrameCounter()) & 1) {
					Darkel_BackendColor(0, 0, 0, 255);
					Darkel_BackendPrint(SCREEN_SCALE_FROM_RIGHT(35.0f), SCREEN_SCALE_Y(109.0f), gUString);
					Darkel_BackendColor(0, 207, 133, 255);
					Darkel_BackendPrint(SCREEN_SCALE_FROM_RIGHT(34.0f), SCREEN_SCALE_Y(108.0f), gUString);
				}
			}
			sprintf(gString, "%d", (Darkel_KillsNeeded >= 0 ? Darkel_KillsNeeded : 0));
			AsciiToUnicode(gString, gUString);
			Darkel_BackendColor(0, 0, 0, 255);
			Darkel_BackendPrint(SCREEN_SCALE_FROM_RIGHT(35.0f), SCREEN_SCALE_Y(144.0f), gUString);
			Darkel_BackendColor(156, 91, 40, 255);
			Darkel_BackendPrint(SCREEN_SCALE_FROM_RIGHT(34.0f), SCREEN_SCALE_Y(143.0f), gUString);
			break;
		}
		case KILLFRENZY_PASSED:
		{
			if (Darkel_bStandardSoundAndMessages) {
				uint32_t timePassedSinceStart = (*Timer_GetTimeInMilliseconds()) - Darkel_TimeOfFrenzyStart;
				if ((*Timer_GetTimeInMilliseconds()) - Darkel_TimeOfFrenzyStart < 5000) {
					Darkel_BackendBackgroundOff();
#ifdef FIX_BUGS
					Darkel_BackendCentreSize(SCREEN_SCALE_X(DEFAULT_SCREEN_WIDTH - 20));
#else
					Darkel_BackendCentreSize(SCREEN_WIDTH - 20);
#endif
					Darkel_BackendCentreOn();
#ifdef FIX_BUGS
					Darkel_BackendScale(SCREEN_SCALE_X(1.5f), SCREEN_SCALE_Y(1.5f));
#else
					Darkel_BackendScale(1.5f, 1.5f);
#endif
					Darkel_BackendJustifyOff();
					Darkel_BackendColor(128, 255, 128, Darkel_CalcFade(timePassedSinceStart, 0, 5000));
					Darkel_BackendFontStyle(FONT_STANDARD);
#ifdef FIX_BUGS
					int y = SCREEN_HEIGHT / 2 + SCREEN_SCALE_Y(25.0f - timePassedSinceStart * 0.01f);
#else
					int y = (SCREEN_HEIGHT / 2 + 25) - (timePassedSinceStart * 0.01f);
#endif
					Darkel_BackendPrint(SCREEN_WIDTH / 2, y, Text_Get(&TheText,"KF_3"));
				}
			}
			break;
		}
		default:
			break;
	}
}

bool Darkel_FrenzyOnGoing(void)
{
    // Preserve the complete original FrenzyOnGoing policy and each ordered external access
	return Darkel_Status == KILLFRENZY_ONGOING;
}

void Darkel_Init(void)
{
    // Preserve the complete original Init policy and each ordered external access
	Darkel_Status = KILLFRENZY_NONE;
}

uint16_t Darkel_QueryModelsKilledByPlayer(int32_t modelId)
{
    // Preserve the complete original QueryModelsKilledByPlayer policy and each ordered external access
	return Darkel_RegisteredKills[modelId];
}

uint16_t Darkel_ReadStatus(void)
{
    // Preserve the complete original ReadStatus policy and each ordered external access
	return Darkel_Status;
}

void Darkel_RegisterCarBlownUpByPlayer(CVehicle *vehicle)
{
    // Preserve the complete original RegisterCarBlownUpByPlayer policy and each ordered external access
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack())
		return;
#endif
	if (Darkel_FrenzyOnGoing()) {
		int32_t model = Darkel_BackendVehicleModel(vehicle);
		if (Darkel_ModelToKill == FRENZY_ANY_CAR || Darkel_ModelToKill == model || Darkel_ModelToKill2 == model || Darkel_ModelToKill3 == model || Darkel_ModelToKill4 == model) {
			Darkel_KillsNeeded--;
			DMAudio_PlayFrontEndSound(SOUND_RAMPAGE_CAR_BLOWN, 0);
		}
	}
	Darkel_RegisteredKills[Darkel_BackendVehicleModel(vehicle)]++;
	switch (Darkel_BackendVehicleAppearance(vehicle)) {
	case VEHICLE_APPEARANCE_CAR:
	case VEHICLE_APPEARANCE_BIKE:
		(*Darkel_BackendCarsExploded())++;;
		break;
	case VEHICLE_APPEARANCE_HELI:
	case VEHICLE_APPEARANCE_PLANE:
		(*Darkel_BackendHelisDestroyed())++;
		break;
	case VEHICLE_APPEARANCE_BOAT:
		(*Darkel_BackendBoatsExploded())++;
		break;
	}
	
}

void Darkel_RegisterKillByPlayer(CPed *victim,enum eWeaponType weapon,bool headshot)
{
    // Preserve the complete original RegisterKillByPlayer policy and each ordered external access
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack())
		return;
#endif
	if (Darkel_FrenzyOnGoing() && (weapon == Darkel_WeaponType
			|| weapon == WEAPONTYPE_EXPLOSION
			|| weapon == WEAPONTYPE_UZI_DRIVEBY && Darkel_WeaponType == WEAPONTYPE_UZI
			|| weapon == WEAPONTYPE_RAMMEDBYCAR && Darkel_WeaponType == WEAPONTYPE_RUNOVERBYCAR
			|| weapon == WEAPONTYPE_RUNOVERBYCAR && Darkel_WeaponType == WEAPONTYPE_RAMMEDBYCAR
			|| weapon == WEAPONTYPE_FLAMETHROWER && Darkel_WeaponType == WEAPONTYPE_MOLOTOV)) {
		int32_t model = Darkel_BackendPedModel(victim);
		if (Darkel_ModelToKill == FRENZY_ANY_PED || Darkel_ModelToKill == model || Darkel_ModelToKill2 == model || Darkel_ModelToKill3 == model || Darkel_ModelToKill4 == model) {
			if (!Darkel_bNeedHeadShot || headshot) {
				Darkel_KillsNeeded--;
				DMAudio_PlayFrontEndSound(SOUND_RAMPAGE_KILL, 0);
			}
		}
	}
	(*Darkel_BackendPeopleKilledByPlayer())++;
	Darkel_RegisteredKills[Darkel_BackendPedModel(victim)]++;
	Darkel_BackendPedsKilledOfThisType()[Darkel_BackendChrisCriminal(victim) ? PEDTYPE_CRIMINAL : Darkel_BackendPedType(victim)]++;
	if (headshot)
		(*Darkel_BackendHeadsPopped())++;
	(*Darkel_BackendKillsSinceLastCheckpoint())++;
}

void Darkel_RegisterKillNotByPlayer(CPed *victim,enum eWeaponType weapontype)
{
    // Preserve the complete original RegisterKillNotByPlayer policy and each ordered external access
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack())
		return;
#endif
	(*Darkel_BackendPeopleKilledByOthers())++;
}

void Darkel_ResetModelsKilledByPlayer(void)
{
    // Preserve the complete original ResetModelsKilledByPlayer policy and each ordered external access
	for (int i = 0; i < NUM_DEFAULT_MODELS; i++)
		Darkel_RegisteredKills[i] = 0;
}

void Darkel_ResetOnPlayerDeath(void)
{
    // Preserve the complete original ResetOnPlayerDeath policy and each ordered external access
	if (Darkel_Status != KILLFRENZY_ONGOING)
		return;

	(*Darkel_BackendRandomPedType()) = -1;
	Darkel_Status = KILLFRENZY_FAILED;
	Darkel_TimeOfFrenzyStart = (*Timer_GetTimeInMilliseconds());

	Darkel_DealWithWeaponChangeAtEndOfFrenzy();
}

void Darkel_StartFrenzy(enum eWeaponType weaponType,int32_t time,uint16_t kill,int32_t modelId0,uint16_t *text,int32_t modelId2,int32_t modelId3,int32_t modelId4,bool standardSound,bool needHeadShot)
{
    // Preserve the complete original StartFrenzy policy and each ordered external access
	Darkel_BackendClearShortCut();
	Darkel_BackendRemoveDropOff();
	enum eWeaponType fixedWeapon;
	if (weaponType == WEAPONTYPE_UZI_DRIVEBY)
		fixedWeapon = WEAPONTYPE_UZI;
	else
		fixedWeapon = weaponType;

	Darkel_WeaponType = weaponType;
	Darkel_Status = KILLFRENZY_ONGOING;
	Darkel_KillsNeeded = kill;
	Darkel_ModelToKill = modelId0;
	Darkel_ModelToKill2 = modelId2;
	Darkel_ModelToKill3 = modelId3;
	Darkel_ModelToKill4 = modelId4;
	Darkel_pStartMessage = text;

	if (text == Text_Get(&TheText,"PAGE_00")) {
		Darkel_bProperKillFrenzy = true;
		Darkel_pStartMessage = NULL;
	} else
		Darkel_bProperKillFrenzy = false;
	
	Darkel_bStandardSoundAndMessages = standardSound;
	Darkel_bNeedHeadShot = needHeadShot;
	Darkel_TimeOfFrenzyStart = (*Timer_GetTimeInMilliseconds());
	Darkel_TimeLimit = time;
	Darkel_PreviousTime = time / 1000;

	CPlayerPed *player = Darkel_BackendPlayer();
	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS) {
		Darkel_InterruptedWeaponSelected = Darkel_BackendWeaponType(Darkel_BackendCurrentWeapon(player));
#if (defined FIX_BUGS || !defined GTA_PS2)
		Darkel_BackendRemoveWeaponAnims(player,Darkel_InterruptedWeaponSelected, -1000.0f);
#endif
		Darkel_InterruptedWeaponType = Darkel_BackendWeaponType(Darkel_BackendWeaponAt(player,Darkel_BackendWeaponSlot(player,fixedWeapon)));
		Darkel_AmmoInterruptedWeapon = Darkel_BackendAmmoTotal(Darkel_BackendWeaponAt(player,Darkel_BackendWeaponSlot(player,fixedWeapon)));
		if (Darkel_InterruptedWeaponType)
			Darkel_BackendAddRef(ModelRegistry_Get(WeaponInfo_GetWeaponInfo((enum eWeaponType)Darkel_InterruptedWeaponType)->m_nModelId));
#if (!defined FIX_BUGS && defined GTA_PS2)
		Darkel_BackendRemoveWeaponAnims(player,Darkel_InterruptedWeaponSelected, -1000.0f);
#endif
		Darkel_BackendGiveWeapon(player,fixedWeapon,30000,true);
		Darkel_BackendCurrentType(player,fixedWeapon);
		Darkel_BackendMakeChanges(player,Darkel_BackendSelectedSlot(player));

		if (Darkel_BackendPlayerVehicle()) {
			Darkel_BackendCurrentSlot(player,Darkel_BackendSelectedSlot(Darkel_BackendPlayer()));
			Darkel_BackendSetAmmo(player,fixedWeapon, Min(Darkel_BackendAmmoTotal(Darkel_BackendCurrentWeapon(player)), WeaponInfo_GetWeaponInfo(Darkel_BackendWeaponType(Darkel_BackendCurrentWeapon(player)))->m_nAmountofAmmunition));
			Darkel_BackendClearTarget(player);
		}
	}
	if (Darkel_bStandardSoundAndMessages)
		DMAudio_PlayFrontEndSound(SOUND_RAMPAGE_START, 0);
}

void Darkel_Update(void)
{
    // Preserve the complete original Update policy and each ordered external access
#ifdef FIX_BUGS
	if (Replay_IsPlayingBack())
		return;
#endif

	if (Darkel_Status != KILLFRENZY_ONGOING)
		return;

	int32_t FrameTime = Darkel_TimeLimit - ((*Timer_GetTimeInMilliseconds()) - Darkel_TimeOfFrenzyStart);
	if (FrameTime > 0 || Darkel_TimeLimit < 0) {

		DMAudio_PlayFrontEndSound(SOUND_RAMPAGE_ONGOING, FrameTime);

		int32_t PrevTime = FrameTime / 1000;

		if (PrevTime != Darkel_PreviousTime) {
			if (Darkel_PreviousTime < 12)
				DMAudio_PlayFrontEndSound(SOUND_CLOCK_TICK, PrevTime);
			Darkel_PreviousTime = PrevTime;
		}

	} else {
		(*Darkel_BackendRandomPedType()) = -1;
		Darkel_Status = KILLFRENZY_FAILED;
		Darkel_TimeOfFrenzyStart = (*Timer_GetTimeInMilliseconds());
		Darkel_DealWithWeaponChangeAtEndOfFrenzy();

		if (Darkel_bStandardSoundAndMessages)
			DMAudio_PlayFrontEndSound(SOUND_RAMPAGE_FAILED, 0);
	}

	if (Darkel_KillsNeeded <= 0) {
		(*Darkel_BackendRandomPedType()) = -1;
		Darkel_Status = KILLFRENZY_PASSED;

		if (Darkel_bProperKillFrenzy)
			Darkel_BackendFrenzyPassed();

		Darkel_TimeOfFrenzyStart = (*Timer_GetTimeInMilliseconds());

		Wanted_SetWantedLevel(Darkel_BackendWanted(Darkel_BackendPlayer()),0);

		Darkel_DealWithWeaponChangeAtEndOfFrenzy();

		if (Darkel_bStandardSoundAndMessages)
			DMAudio_PlayFrontEndSound(SOUND_RAMPAGE_PASSED, 0);
	}
}

void Darkel_DealWithWeaponChangeAtEndOfFrenzy(void)
{
    // Preserve the complete original DealWithWeaponChangeAtEndOfFrenzy policy and each ordered external access
	enum eWeaponType fixedWeapon;
	if (Darkel_WeaponType == WEAPONTYPE_UZI_DRIVEBY)
		fixedWeapon = WEAPONTYPE_UZI;
	else
		fixedWeapon = (enum eWeaponType)Darkel_WeaponType;

	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS && Darkel_InterruptedWeaponType)
		Darkel_BackendRemoveRef(ModelRegistry_Get(WeaponInfo_GetWeaponInfo((enum eWeaponType)Darkel_InterruptedWeaponType)->m_nModelId));

	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS) {
		int slot = WeaponInfo_GetWeaponInfo(fixedWeapon)->m_nWeaponSlot;
		Darkel_BackendRemoveSlotModel(slot);
		Darkel_BackendSetWeaponType(Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot),WEAPONTYPE_UNARMED);
		Darkel_BackendSetAmmoTotal(Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot),0);
		Darkel_BackendSetAmmoInClip(Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot),0);
		Darkel_BackendSetWeaponState(Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot),WEAPONSTATE_READY);
		Darkel_BackendRemoveWeaponAnims(Darkel_BackendPlayer(),fixedWeapon, -1000.0f);
		Darkel_BackendRemoveRef(ModelRegistry_Get(WeaponInfo_GetWeaponInfo(fixedWeapon)->m_nModelId));
	}

	CPlayerPed* player = Darkel_BackendPlayer();
	if (fixedWeapon < WEAPONTYPE_TOTALWEAPONS) {
		Darkel_BackendSetSelectedSlot(player,WeaponInfo_GetWeaponInfo((enum eWeaponType)Darkel_InterruptedWeaponSelected)->m_nWeaponSlot);
		Darkel_BackendGiveWeapon(player,(enum eWeaponType)Darkel_InterruptedWeaponType,Darkel_AmmoInterruptedWeapon,true);
	}

	if (Darkel_BackendPlayerVehicle()) {
		Darkel_BackendRemoveModel(player,WeaponInfo_GetWeaponInfo(Darkel_BackendWeaponType(Darkel_BackendCurrentWeapon(player)))->m_nModelId);
		if (Darkel_BackendWeaponType(Darkel_BackendWeaponAt(Darkel_BackendPlayer(),WEAPONSLOT_SUBMACHINEGUN)))
			Darkel_BackendSetSelectedSlot(Darkel_BackendPlayer(),WEAPONSLOT_SUBMACHINEGUN);
		else
			Darkel_BackendSetSelectedSlot(Darkel_BackendPlayer(),WEAPONSLOT_UNARMED);
		Darkel_BackendCurrentSlot(player,Darkel_BackendSelectedSlot(Darkel_BackendPlayer()));
		Darkel_BackendMakeChanges(player,Darkel_BackendCurrentSlotIndex(player));
		Darkel_BackendRemoveDriveby(player);
	}
}
//- rouz edit (ChatGPT)
