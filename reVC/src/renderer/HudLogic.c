//+ rouz edit (Claude)
#define _CRT_SECURE_NO_WARNINGS
#include "HudLogic.h"
#include <stdio.h>
#include "CameraModes.h"
#include "WeaponType.h"
#include "PedStates.h"

void Hud_CrosshairFlags(int32_t mode, int32_t weaponType, bool weaponIsMelee, bool using3rdPersonMouseCam, bool lookingBehind, bool playerIsInGarage, int32_t pedState, bool *drawCrossHair, bool *drawCrossHairPC)
{
	*drawCrossHair = false;
	*drawCrossHairPC = false;

	// The aiming camera modes show the crosshair unless the weapon is a melee one
	if ((mode == CAMERA_MODE_SNIPER || mode == CAMERA_MODE_ROCKETLAUNCHER || mode == CAMERA_MODE_M16_1STPERSON || mode == CAMERA_MODE_HELICANNON_1STPERSON || mode == CAMERA_MODE_CAMERA)
		&& !weaponIsMelee)
		*drawCrossHair = true;

	if (mode == CAMERA_MODE_M16_1STPERSON_RUNABOUT || mode == CAMERA_MODE_ROCKETLAUNCHER_RUNABOUT || mode == CAMERA_MODE_SNIPER_RUNABOUT)
		*drawCrossHairPC = true;

	// Firearms aimed with the mouse or in first person also show it, but not while getting into a car
	if ((using3rdPersonMouseCam && (!lookingBehind || playerIsInGarage)) || mode == CAMERA_MODE_1STPERSON_RUNABOUT) {
		if (pedState != PED_ENTER_CAR && pedState != PED_CARJACK) {
			if ((weaponType >= WEAPONTYPE_COLT45 && weaponType <= WEAPONTYPE_RUGER)
				|| weaponType == WEAPONTYPE_M60 || weaponType == WEAPONTYPE_MINIGUN
				|| weaponType == WEAPONTYPE_FLAMETHROWER) {
				*drawCrossHairPC = true;
			}
		}
	}
}
void Hud_FormatAmmo(char *out, int32_t ammoAmount, int32_t weaponType, int32_t ammoInClip, int32_t totalAmmo)
{
	int32_t ammo, clip;

	if (ammoAmount <= 1 || ammoAmount >= 1000)
		sprintf(out, "%d", totalAmmo);
	else {
		if (weaponType == WEAPONTYPE_FLAMETHROWER) {
			clip = ammoInClip / 10;

			ammo = (totalAmmo - ammoInClip) / 10;
			if (ammo > 9999) ammo = 9999;
		} else {
			clip = ammoInClip;

			ammo = totalAmmo - ammoInClip;
			if (ammo > 9999) ammo = 9999;
		}

		sprintf(out, "%d-%d", ammo, clip);
	}
}

bool Hud_ShowHealth(bool healthIsFlashing, uint32_t frameCounter, float health)
{
	bool outer = (healthIsFlashing && (frameCounter & 8)) || !healthIsFlashing || (health < 10 && (frameCounter & 8));
	bool inner = health >= 10 || (health < 10 && (frameCounter & 8));

	return outer && inner;
}

bool Hud_ShowArmour(bool armourIsFlashing, uint32_t frameCounter)
{
	return (armourIsFlashing && (frameCounter & 8)) || !armourIsFlashing;
}

bool Hud_ShowLossIcon(uint32_t timeLastLoss, uint32_t now, uint32_t frameCounter)
{
	return !timeLastLoss || now > timeLastLoss + 2000 || (frameCounter & 4);
}

float Hud_ZoneNameFadeStep(uint32_t *state, int32_t *fadeTimer, uint32_t nameTimer, float timeStepMs, bool *restorePrint)
{
	float alpha = 255.0f;

	*restorePrint = false;

	switch (*state) {
	case 1:
		alpha = 255.0f;
		*fadeTimer = 1000;
		if (nameTimer > 10000.0f) {
			*fadeTimer = 1000;
			*state = 3;
		}
		break;
	case 2:
		*fadeTimer = (int32_t)(*fadeTimer + timeStepMs);
		if (*fadeTimer > 1000.0f) {
			*state = 1;
			*fadeTimer = 1000;
		}
		alpha = *fadeTimer * 0.001f * 255.0f;
		break;
	case 3:
		*fadeTimer = (int32_t)(*fadeTimer - timeStepMs);
		if (*fadeTimer < 0.0f) {
			*state = 0;
			*fadeTimer = 0;
		}
		alpha = *fadeTimer * 0.001f * 255.0f;
		break;
	case 4:
		*fadeTimer = (int32_t)(*fadeTimer - timeStepMs);
		if (*fadeTimer < 0.0f) {
			*fadeTimer = 0;
			*restorePrint = true;
			*state = 2;
		}
		alpha = *fadeTimer * 0.001f * 255.0f;
		break;
	default:
		break;
	}

	return alpha;
}

float Hud_VehicleNameFadeStep(uint32_t *state, int32_t *fadeTimer, uint32_t *nameTimer, float timeStepMs, bool *restorePrint)
{
	float alpha = 0.0f;

	*restorePrint = false;

	switch (*state) {
	case 1:
		if (*nameTimer > 10000) {
			*fadeTimer = 1000;
			*state = 3;
		}
		alpha = 255.0f;
		break;
	case 2:
		*fadeTimer = (int32_t)(*fadeTimer + timeStepMs);
		if (*fadeTimer > 1000) {
			*state = 1;
			*fadeTimer = 1000;
		}
		alpha = *fadeTimer * 0.001f * 255.0f;
		break;
	case 3:
		*fadeTimer = (int32_t)(*fadeTimer - timeStepMs);
		if (*fadeTimer < 0) {
			*state = 0;
			*fadeTimer = 0;
		}
		alpha = *fadeTimer * 0.001f * 255.0f;
		break;
	case 4:
		*fadeTimer = (int32_t)(*fadeTimer - timeStepMs);
		if (*fadeTimer < 0) {
			*fadeTimer = 0;
			*restorePrint = true;
			*nameTimer = 0;
			*state = 2;
		}
		alpha = *fadeTimer * 0.001f * 255.0f;
		break;
	default:
		break;
	}

	return alpha;
}

bool Hud_FlashTimerStep(bool *onLastFrame, uint16_t *flashTimer, uint32_t frameCounter)
{
	// Start blinking when it was not shown the frame before
	if (!*onLastFrame)
		*flashTimer = 1;

	*onLastFrame = true;

	if (*flashTimer != 0) {
		if (++*flashTimer > 50)
			*flashTimer = 0;
	}

	return (frameCounter & 4) || *flashTimer == 0;
}

bool Hud_ZoneNameChanged(uint32_t *zoneState, uint32_t *zoneNameTimer, int32_t *zoneFadeTimer, uint32_t *vehicleState)
{
	bool setPrint = false;

	switch (*zoneState) {
	case 0:
		*zoneState = 2;
		setPrint = true;
		*zoneNameTimer = 0;
		*zoneFadeTimer = 0;
		if (*vehicleState == 1 || *vehicleState == 2)
			*vehicleState = 3;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		*zoneNameTimer = 5;
		*zoneState = 4;
		break;
	default:
		break;
	}

	return setPrint;
}

bool Hud_VehicleNameChanged(uint32_t *vehicleState, uint32_t *vehicleNameTimer, int32_t *vehicleFadeTimer, uint32_t *zoneState)
{
	bool setPrint = false;

	switch (*vehicleState) {
	case 0:
		*vehicleState = 2;
		setPrint = true;
		*vehicleNameTimer = 0;
		*vehicleFadeTimer = 0;
		if (*zoneState == 1 || *zoneState == 2)
			*zoneState = 3;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		*vehicleNameTimer = 0;
		*vehicleState = 4;
		break;
	default:
		break;
	}

	return setPrint;
}

bool Hud_OddJob2Step(float *offTimer, int16_t *on, float *xOffset, uint16_t *timer, bool hasMessage, float timeStepMs)
{
	float fStep;

	if (*offTimer > 0)
		*offTimer -= timeStepMs;

	if (!(hasMessage && *offTimer <= 0.0f))
		return false;

	switch (*on) {
	case 0:
		*on = 1;
		*xOffset = 380.0f;
		break;
	case 1:
		if (*xOffset <= 2.0f) {
			*timer = 0;
			*on = 2;
		} else {
			fStep = *xOffset / 6.0f;
			if (fStep > 40.0f) fStep = 40.0f;
			*xOffset = *xOffset - fStep;
		}
		break;
	case 2:
		*timer = (uint16_t)(*timer + timeStepMs);
		if (*timer > 1500) {
			*on = 3;
		}
		break;
	case 3:
		fStep = *xOffset / 5.0f;
		if (fStep < 30.0f) fStep = 30.0f;

		*xOffset = *xOffset - fStep;

		if (*xOffset < -380.0f) {
			*offTimer = 5000.0f;
			*on = 0;
		}
		break;
	default:
		break;
	}

	return true;
}

void Hud_MissionTitleStep(float *x, float *inUse, float *alpha, float stopX, float timeStep, float timeStepMs, float xStep, bool *clear)
{
	*clear = false;

	if (*x >= stopX) {
		// Once in place it is shown for a while and then fades out
		*inUse += timeStep;

		if (*inUse >= 120.0f) {
			*inUse = 120.0f;
			*alpha -= timeStepMs;
		}
		if (*alpha <= 0.0f) {
			*clear = true;
			*inUse = 0.0f;
			*alpha = 0.0f;
		}
	} else {
		// Slide in while fading in
		*x += xStep;
		*alpha += timeStepMs;

		if (*alpha > 255.0f)
			*alpha = 255.0f;
	}
}

bool Hud_HelpMessageChanged(uint32_t *state, int32_t *fadeTimer, uint32_t *timer, float *displayTime, int32_t length)
{
	bool newStart = false;

	switch (*state) {
	case 0:
		*fadeTimer = 0;
		*state = 2;
		*timer = 0;
		newStart = true;
		*displayTime = length * 0.05f + 3.0f;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
		*timer = 5;
		*state = 4;
		break;
	default:
		break;
	}

	return newStart;
}

float Hud_HelpMessageFadeStep(uint32_t *state, uint32_t timer, int32_t *fadeTimer, float displayTime, bool displayForever, bool quick, bool wideScreen, float timeStepMs, bool *showNew)
{
	float alpha = 225.0f;

	*showNew = false;

	switch (*state) {
	case 1:
		alpha = 225.0f;
		*fadeTimer = 600;
		if ((!displayForever && timer > displayTime * 1000.0f) ||
			(quick && timer > 1500.0f)) {

			*fadeTimer = 600;
			*state = 3;
		}
		break;
	case 2:
		if (wideScreen)
			break;

		*fadeTimer = (int32_t)(*fadeTimer + 2 * timeStepMs);
		if (*fadeTimer > 0) {
			*state = 1;
			*fadeTimer = 0;
		}
		alpha = *fadeTimer * 0.001f * 225.0f;
		break;
	case 3:
		*fadeTimer = (int32_t)(*fadeTimer - 2 * timeStepMs);
		if (*fadeTimer < 0 || wideScreen) {
			*state = 0;
			*fadeTimer = 0;
		}
		alpha = *fadeTimer * 0.001f * 225.0f;
		break;
	case 4:
		*fadeTimer = (int32_t)(*fadeTimer - 2 * timeStepMs);
		if (*fadeTimer < 0) {
			*state = 2;
			*fadeTimer = 0;
			*showNew = true;
		}
		alpha = *fadeTimer * 0.001f * 225.0f;
		break;
	default:
		break;
	}

	return alpha;
}

void Hud_BigMessage0Step(float *x, float *inUse, float *alpha, float stopX, float timeStep, float timeStepMs, float xStep, bool *clear)
{
	*clear = false;

	if (*x >= stopX) {
		*inUse += timeStep;

		// After a short while the text fades out
		if (*inUse >= 120.0f) {
			*inUse = 120.0f;
			*alpha -= (timeStepMs * 0.3f);
		}

		if (*alpha <= 0.0f) {
			*clear = true;
			*alpha = 0.0f;
		}
	} else {
		*x += xStep;
		*alpha += (timeStepMs * 0.3f);

		if (*alpha > 255.0f)
			*alpha = 255.0f;
	}
}

void Hud_BigMessage2Step(float *alpha, float timeStepMs)
{
	*alpha += (timeStepMs * 0.4f);

	if (*alpha > 255.0f)
		*alpha = 255.0f;
}

//- rouz edit (Claude)
