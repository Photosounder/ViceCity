#pragma once

#include "CrimeTypes.h" // rouz edit (ChatGPT)

class CCrimeBeingQd
{
public:
	eCrimeType m_nType;
	int32 m_nId;
	uint32 m_nTime;
	CVector m_vecPosn;
	bool m_bReported;
	bool m_bPoliceDoesntCare;

	CCrimeBeingQd() { };
	~CCrimeBeingQd() { };
};
