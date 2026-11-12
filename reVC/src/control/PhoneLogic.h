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
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
