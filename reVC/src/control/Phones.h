//+ rouz edit (ChatGPT)
#pragma once

#include "Physical.h"

class CPed;
struct CAnimBlendAssociation;

#include "PhoneData.h"

class CPhone : public CPhoneData
{
public:

	CPhone() { }
	~CPhone() { }
};

VALIDATE_SIZE(CPhone, 0x34);

#include "PhoneInfoData.h"

class CPhoneInfo : public CPhoneInfoData {
public:
	static bool bDisplayingPhoneMessage;
	static uint32 PhoneEnableControlsTimer;
	static CPhone *pPhoneDisplayingMessages;
	static bool bPickingUpPhone;
	static CPed *pCallBackPed;


	CPhoneInfo() { }
	~CPhoneInfo() { }

	int FindNearestFreePhone(CVector*);
	bool PhoneAtThisPosition(CVector);
	bool HasMessageBeenDisplayed(int);
	bool IsMessageBeingDisplayed(int);
	void Load(uint8 *buf, uint32 size);
	void Save(uint8 *buf, uint32 *size);
	void SetPhoneMessage_JustOnce(int phoneId, wchar *msg1, wchar *msg2, wchar *msg3, wchar *msg4, wchar *msg5, wchar *msg6);
	void SetPhoneMessage_Repeatedly(int phoneId, wchar *msg1, wchar *msg2, wchar *msg3, wchar *msg4, wchar *msg5, wchar *msg6);
	int GrabPhone(float, float);
	void Initialise(void);
	void Shutdown(void);
	void Update(void);
};

extern CPhoneInfo gPhoneInfo;

void PhonePutDownCB(CAnimBlendAssociation *assoc, void *arg);
void PhonePickUpCB(CAnimBlendAssociation *assoc, void *arg);
//- rouz edit (ChatGPT)
