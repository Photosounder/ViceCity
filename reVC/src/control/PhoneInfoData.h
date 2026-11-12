//+ rouz edit (Claude)
#pragma once
#include "PhoneData.h"

// The state of the phone table, in C++ this must come after the CPhone class
typedef struct CPhoneInfoData {
	int32_t m_nMax;
	int32_t m_nScriptPhonesMax;
	PHONEINFO_PHONE m_aPhones[NUMPHONES];
} CPhoneInfoData;
//- rouz edit (Claude)
