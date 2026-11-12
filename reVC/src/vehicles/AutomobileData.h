//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "CarNodes.h"
#include "DamageManager.h"
#include "Door.h"
#include "ColPoint.h"
#include "Skidmarks.h"
#include "WheelState.h"

// Types keep their engine form in C++ and plain data in C
#ifdef __cplusplus
#define AUTODATA_ENUM(type) type
#define AUTODATA_FRAME RwFrame
typedef CVector AutoVec;
class CEntity;
class CPhysical;
#else
#define AUTODATA_ENUM(type) int
#define AUTODATA_FRAME void
typedef struct AutoVec {float x,y,z;} AutoVec;
typedef struct CEntity CEntity;
typedef struct CPhysical CPhysical;
#endif

// The automobile state that follows CVehicle, as plain data usable from C
typedef struct CAutomobileData {
	CDamageManager Damage;
	CDoor Doors[VEHICLE_DOOR_COUNT];
	AUTODATA_FRAME *m_aCarNodes[NUM_CAR_NODES];
	CColPoint m_aWheelColPoints[4];
	float m_aSuspensionSpringRatio[4];
	float m_aSuspensionSpringRatioPrev[4];
	float m_aWheelTimer[4];		// set to 4.0 when wheel is touching ground, then decremented
	float m_auto_unused1;
	AUTODATA_ENUM(eSkidmarkType) m_aWheelSkidmarkType[4];
	bool m_aWheelSkidmarkBloody[4];
	bool m_aWheelSkidmarkUnk[4];
	float m_aWheelRotation[4];
	float m_aWheelPosition[4];
	float m_aWheelSpeed[4];
	uint8_t m_auto_unused2;
#if (defined GTA_PS2 && !defined FIX_BUGS)
	uint8_t m_bombType : 3;
#endif
	uint8_t bTaxiLight : 1;
	uint8_t bFixedColour : 1;
	uint8_t bBigWheels : 1;
	uint8_t bWaterTight : 1;	// no damage for non-player peds
	uint8_t bNotDamagedUpsideDown : 1;
	uint8_t bMoreResistantToDamage : 1;
	uint8_t bTankDetonateCars : 1;
	uint8_t bStuckInSand : 1;
	uint8_t bHeliDestroyed : 1;
#if (defined GTA_PS2 && !defined FIX_BUGS)
	CEntity* m_pBombRigger;
#endif
	int16_t m_doingBurnout;
	uint16_t m_hydraulicState;
	uint32_t m_nBusDoorTimerEnd;
	uint32_t m_nBusDoorTimerStart;
	float m_aSuspensionSpringLength[4];
	float m_aSuspensionLineLength[4];
	float m_fHeightAboveRoad;
	float m_fTraction;
	float m_fTireTemperature;
	float m_fOrientation;	// for heli and plane go-to
	float m_fPlaneSteer;	// related to the above
	float m_fVelocityChangeForAudio;
	float m_randomValues[6];	// used for what?
	float m_fFireBlowUpTimer;
	CPhysical *m_aGroundPhysical[4];	// physicals touching wheels
	AutoVec m_aGroundOffset[4];		// from ground object to colpoint
	CEntity *m_pSetOnFireEntity;
	float m_weaponDoorTimerLeft;	// still don't know what exactly this is
	float m_weaponDoorTimerRight;
	float m_fCarGunLR;
	float m_fCarGunUD;
	float m_fHeliOrientation;
	float m_fPropellerRotation;
	uint8_t stuff4[4];
	uint8_t m_nWheelsOnGround;
	uint8_t m_nDriveWheelsOnGround;
	uint8_t m_nDriveWheelsOnGroundPrev;
	float m_fGasPedalAudio;
	AUTODATA_ENUM(tWheelState) m_aWheelState[4];
} CAutomobileData;
//- rouz edit (Claude)
