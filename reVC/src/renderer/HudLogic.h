//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Which crosshair the hud shows from the camera mode and the weapon of the player: drawCrossHair is the one of the aiming camera modes, drawCrossHairPC the one of the run about modes and the mouse aimed third person camera
void Hud_CrosshairFlags(int32_t mode, int32_t weaponType, bool weaponIsMelee, bool using3rdPersonMouseCam, bool lookingBehind, bool playerIsInGarage, int32_t pedState, bool *drawCrossHair, bool *drawCrossHairPC);
// Text of the ammo counter, out has room for 16 characters: the total for weapons with a single or endless supply, otherwise the spare ammo and the ammo in the clip, flamethrower ammo being counted in tens
void Hud_FormatAmmo(char *out, int32_t ammoAmount, int32_t weaponType, int32_t ammoInClip, int32_t totalAmmo);
// Whether the health number is shown this frame: it blinks while it is the item to flash and below 10 health it blinks regardless
bool Hud_ShowHealth(bool healthIsFlashing, uint32_t frameCounter, float health);
// Whether the armour number is shown this frame, it blinks while it is the item to flash
bool Hud_ShowArmour(bool armourIsFlashing, uint32_t frameCounter);
// Whether the icon next to a health or armour number is shown: always except for two seconds after a loss, when it blinks
bool Hud_ShowLossIcon(uint32_t timeLastLoss, uint32_t now, uint32_t frameCounter);
// Fade of the zone name for a frame, states are 0 hidden, 1 shown, 2 fading in, 3 fading out and 4 fading out to show a new name, the result is the alpha, restorePrint says the name to print is the last one again
float Hud_ZoneNameFadeStep(uint32_t *state, int32_t *fadeTimer, uint32_t nameTimer, float timeStepMs, bool *restorePrint);
// Same for the vehicle name, it also restarts its name timer when a new name is shown, the alpha is 0 for the state 0
float Hud_VehicleNameFadeStep(uint32_t *state, int32_t *fadeTimer, uint32_t *nameTimer, float timeStepMs, bool *restorePrint);
// Blinking of an on screen timer or counter that just appeared: it blinks for 50 frames, the result tells whether it is shown this frame
bool Hud_FlashTimerStep(bool *onLastFrame, uint16_t *flashTimer, uint32_t frameCounter);
// New zone name to show: starts its fade in and tells the vehicle name to fade out, the result is true when the name to print has to be set to the new one
bool Hud_ZoneNameChanged(uint32_t *zoneState, uint32_t *zoneNameTimer, int32_t *zoneFadeTimer, uint32_t *vehicleState);
// New vehicle name to show, the same way
bool Hud_VehicleNameChanged(uint32_t *vehicleState, uint32_t *vehicleNameTimer, int32_t *vehicleFadeTimer, uint32_t *zoneState);
// Slide in and out of the odd job result message for a frame, the off timer counts down the pause between two showings and the on state is 0 waiting, 1 sliding in, 2 shown, 3 sliding out, the result is true while the message is to be drawn
bool Hud_OddJob2Step(float *offTimer, int16_t *on, float *xOffset, uint16_t *timer, bool hasMessage, float timeStepMs);
// Slide in, hold and fade out of the mission title for a frame, x is its position, inUse the time shown, alpha its opacity, once it is gone clear is set
void Hud_MissionTitleStep(float *x, float *inUse, float *alpha, float stopX, float timeStep, float timeStepMs, float xStep, bool *clear);
// A new help message arrives: states are 0 hidden, 1 shown, 2 fading in, 3 fading out and 4 fading out to show the new text, with newStart true the message is to be copied to the one to print and its sound played
bool Hud_HelpMessageChanged(uint32_t *state, int32_t *fadeTimer, uint32_t *timer, float *displayTime, int32_t length);
// Fade of the help message for a frame, the result is the alpha, showNew says the last message becomes the one to print
float Hud_HelpMessageFadeStep(uint32_t *state, uint32_t timer, int32_t *fadeTimer, float displayTime, bool displayForever, bool quick, bool wideScreen, float timeStepMs, bool *showNew);
// Fade in, hold and fade out of the mission complete or failed text for a frame, the text is cleared when it is gone
void Hud_BigMessage0Step(float *x, float *inUse, float *alpha, float stopX, float timeStep, float timeStepMs, float xStep, bool *clear);
// Fade in of the wasted or busted text for a frame, the alpha is limited to 255
void Hud_BigMessage2Step(float *alpha, float timeStepMs);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
