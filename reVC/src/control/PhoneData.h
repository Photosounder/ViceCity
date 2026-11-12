//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

enum PhoneState {
	PHONE_STATE_FREE,
	PHONE_STATE_REPORTING_CRIME, // CCivilianPed::ProcessControl sets it but unused
	PHONE_STATE_2,
	PHONE_STATE_MESSAGE_REMOVED,
	PHONE_STATE_ONETIME_MESSAGE_SET,
	PHONE_STATE_REPEATED_MESSAGE_SET,
	PHONE_STATE_REPEATED_MESSAGE_SHOWN_ONCE,
	PHONE_STATE_ONETIME_MESSAGE_STARTED,
	PHONE_STATE_REPEATED_MESSAGE_STARTED,
	PHONE_STATE_9 // just rings, picking being handled via script. most of the time game uses this
};

// Types keep their engine form in C++ and plain data in C
#ifdef __cplusplus
#define PHONEDATA_ENUM(type) type
#define PHONEINFO_PHONE CPhone
typedef CVector PhoneVec;
class CEntity;
class CPhone;
#else
#define PHONEDATA_ENUM(type) int
#define PHONEINFO_PHONE CPhoneData
typedef struct PhoneVec {float x,y,z;} PhoneVec;
typedef struct CEntity CEntity;
#endif

typedef struct CPhoneData {
	PhoneVec m_vecPos;
	uint16_t *m_apMessages[6];
	uint32_t m_repeatedMessagePickupStart;
	CEntity *m_pEntity; // stored as building pool index in save files
	PHONEDATA_ENUM(PhoneState) m_nState;
	bool m_visibleToCam;
} CPhoneData;
//- rouz edit (Claude)
