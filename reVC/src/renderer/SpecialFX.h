//+ rouz edit (ChatGPT)
#pragma once

//file done

#include "SpecialFX_control.h"


#include "MotionBlurStreaks.h"

#include "BulletTraces.h"

#include "Markers3D.h"


struct C3dMarker
{
	CMatrix m_Matrix;
	RpAtomic *m_pAtomic;
	RpMaterial *m_pMaterial;
	uint16 m_nType;
	bool m_bIsUsed;
	bool m_bFindZOnNextPlacement;
	uint32 m_nIdentifier;
	RwRGBA m_Color;
	uint16 m_nPulsePeriod;
	int16 m_nRotateRate;
	uint32 m_nStartTime;
	float m_fPulseFraction;
	float m_fStdSize;
	float m_fSize;
	float m_fBrightness;
	float m_fCameraRange;

};




#include "BrightLights.h"

#include "ShinyTexts.h"

#include "MoneyMessages.h"

#include "BoatFoam.h"
//- rouz edit (ChatGPT)
