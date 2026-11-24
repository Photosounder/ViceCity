//+ rouz edit (Claude)
#include <math.h>
#include "PhoneLogic.h"

int Phone_FindNearestFreePhone(const CPhoneInfoData *info, float x, float y)
{
	int nearestPhoneId = -1;
	float nearestPhoneDist = 60.0f;
	int phoneId;

	for (phoneId = 0; phoneId < info->m_nMax; phoneId++) {
		if (info->m_aPhones[phoneId].m_nState == PHONE_STATE_FREE) {
			// Flat distance to the phone, the first strictly closer one wins
			float dx = info->m_aPhones[phoneId].m_vecPos.x - x;
			float dy = info->m_aPhones[phoneId].m_vecPos.y - y;
			float phoneDist = sqrtf(dx * dx + dy * dy);

			if (phoneDist < nearestPhoneDist) {
				nearestPhoneDist = phoneDist;
				nearestPhoneId = phoneId;
			}
		}
	}
	return nearestPhoneId;
}

bool Phone_PhoneAtThisPosition(const CPhoneInfoData *info, float x, float y)
{
	int phoneId;
	for (phoneId = 0; phoneId < info->m_nMax; phoneId++) {
		if (x == info->m_aPhones[phoneId].m_vecPos.x && y == info->m_aPhones[phoneId].m_vecPos.y)
			return true;
	}
	return false;
}

bool Phone_HasMessageBeenDisplayed(const CPhoneInfoData *info, int phoneId, bool displayingPhoneMessage)
{
	int state;
	if (displayingPhoneMessage)
		return false;

	state = info->m_aPhones[phoneId].m_nState;

	return state == PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE ||
		state == PHONE_STATE_ONETIME_MESSAGE_STARTED ||
		state == PHONE_STATE_REPEATED_MESSAGE_STARTED;
}

void Phone_SetMessage(CPhoneInfoData *info, int phoneId, uint16_t *const msgs[6], int stateWhenSet)
{
	// If there is at least one message, it should be msg1.
	if (msgs[0]) {
		int i;
		for (i = 0; i < 6; i++)
			info->m_aPhones[phoneId].m_apMessages[i] = msgs[i];
		info->m_aPhones[phoneId].m_nState = stateWhenSet;
	} else {
		info->m_aPhones[phoneId].m_nState = PHONE_STATE_MESSAGE_REMOVED;
	}
}

int Phone_GrabPhone(CPhoneInfoData *info, float x, float y)
{
	// "Grab" means allocating the phone closest to the position to the script, not picking it up
	int nearestPhoneId = -1;
	float nearestPhoneDist = 100.0f;
	int phoneId;
	CPhoneData oldFirstPhone;

	for (phoneId = info->m_nScriptPhonesMax; phoneId < info->m_nMax; phoneId++) {
		float dx = info->m_aPhones[phoneId].m_vecPos.x - x;
		float dy = info->m_aPhones[phoneId].m_vecPos.y - y;
		float phoneDistance = sqrtf(dx * dx + dy * dy);
		if (phoneDistance < nearestPhoneDist) {
			nearestPhoneDist = phoneDistance;
			nearestPhoneId = phoneId;
		}
	}
	info->m_aPhones[nearestPhoneId].m_nState = PHONE_STATE_MESSAGE_REMOVED;

	// Swap the grabbed phone to the first free script slot
	oldFirstPhone = *(CPhoneData *)&info->m_aPhones[info->m_nScriptPhonesMax];
	info->m_aPhones[info->m_nScriptPhonesMax] = info->m_aPhones[nearestPhoneId];
	info->m_aPhones[nearestPhoneId] = *(PHONEINFO_PHONE *)&oldFirstPhone;
	info->m_nScriptPhonesMax++;
	return info->m_nScriptPhonesMax - 1;
}
bool Phone_PlayerIsClose(float distX, float distY)
{
	return fabsf(distX) < 1.0f && fabsf(distY) < 1.0f;
}

bool Phone_PlayerFacesPhone(float distX, float distY, float forwardX, float forwardY, float distance)
{
	return (distX * forwardX + distY * forwardY) / distance < -0.85f;
}
//- rouz edit (Claude)
