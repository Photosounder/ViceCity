//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "PhoneInfoData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Phone table helpers
int Phone_FindNearestFreePhone(const CPhoneInfoData *info, float x, float y);
bool Phone_PhoneAtThisPosition(const CPhoneInfoData *info, float x, float y);
bool Phone_HasMessageBeenDisplayed(const CPhoneInfoData *info, int phoneId, bool displayingPhoneMessage);
// Set the messages of a phone and its state, or remove the messages when the first one is missing
void Phone_SetMessage(CPhoneInfoData *info, int phoneId, uint16_t *const msgs[6], int stateWhenSet);
// Allocate the phone closest to a position to the script phones and return its new index
int Phone_GrabPhone(CPhoneInfoData *info, float x, float y);
// Whether a player is close enough to a phone to use it: less than 1 unit away on both axes
bool Phone_PlayerIsClose(float distX, float distY);
// Whether the player faces a phone: the cosine of the angle between the direction to the phone and its forward vector is below -0.85
bool Phone_PlayerFacesPhone(float distX, float distY, float forwardX, float forwardY, float distance);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
