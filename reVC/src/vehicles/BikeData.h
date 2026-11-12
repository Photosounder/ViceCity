//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "BikeNodes.h"
#include "MatrixData.h"
#include "ColPoint.h"
#include "Skidmarks.h"
#include "WheelState.h"
#include "HandlingMgr.h"

// Types keep their engine form in C++ and plain data in C
#ifdef __cplusplus
#include "AnimGroups.h"
#include "Matrix.h"
typedef CMatrix BikeMatrix;
#define BIKEDATA_ENUM(type) type
#define BIKEDATA_FRAME RwFrame
typedef CVector BikeVec3;
class CEntity;
class CPhysical;
#else
#define BIKEDATA_ENUM(type) int
#define BIKEDATA_FRAME void
typedef struct BikeVec3 {float x,y,z;} BikeVec3;
typedef CMatrixData BikeMatrix;
typedef struct CEntity CEntity;
typedef struct CPhysical CPhysical;
#endif

// The bike state that follows CVehicle, as plain data usable from C
typedef struct CBikeData {
	BIKEDATA_FRAME *m_aBikeNodes[BIKE_NUM_NODES];
	bool bLeanMatrixClean;
	BikeMatrix m_leanMatrix;
	BikeVec3 m_vecAvgSurfaceNormal;
	BikeVec3 m_vecAvgSurfaceRight;
	tBikeHandlingData *pBikeHandling;
	BIKEDATA_ENUM(AssocGroupId) m_bikeAnimType;
	uint8_t m_wheelStatus[2];
	CColPoint m_aWheelColPoints[4];
	float m_aSuspensionSpringRatio[4];
	float m_aSuspensionSpringRatioPrev[4];
	float m_aWheelTimer[4];
	float m_bike_unused1;
	BIKEDATA_ENUM(eSkidmarkType) m_aWheelSkidmarkType[2];
	bool m_aWheelSkidmarkBloody[2];
	bool m_aWheelSkidmarkUnk[2];
	float m_aWheelRotation[2];
	float m_aWheelSpeed[2];
	float m_aWheelPosition[2];
	float m_aWheelBasePosition[2];
	float m_aSuspensionSpringLength[4];
	float m_aSuspensionLineLength[4];
	float m_fHeightAboveRoad;
	float m_fTraction;
	float m_fRearForkLength;
	float m_fFrontForkY;
	float m_fFrontForkZ;
	float m_fFrontForkSlope;
	float m_fWheelAngle;
	float m_fLeanLRAngle;
	float m_fLeanLRAngle2;
	float m_fLeanInput;
	float m_fPedLeanAmountLR;
	float m_fPedLeanAmountUD;
	uint8_t m_bike_unused2;
	uint8_t unused[3];	// looks like padding..but for what?
	uint8_t m_bike_flag01 : 1;
	uint8_t m_bike_flag02 : 1;
	uint8_t bWaterTight : 1;
	uint8_t bIsBeingPickedUp : 1;
	uint8_t bIsStanding : 1;
	uint8_t bExtraSpeed : 1;	// leaning forward
	uint8_t bIsOnFire : 1;
	uint8_t bWheelieCam : 1;
	int16_t m_doingBurnout;
	float m_fTireTemperature;
	float m_fBrakeDestabilization;
	float m_fVelocityChangeForAudio;
	float m_fFireBlowUpTimer;
	CPhysical *m_aGroundPhysical[4];
	BikeVec3 m_aGroundOffset[4];
	CEntity *m_pSetOnFireEntity;
	uint8_t m_nWheelsOnGround;
	uint8_t m_nDriveWheelsOnGround;
	uint8_t m_nDriveWheelsOnGroundPrev;
	float m_fGasPedalAudio;
	BIKEDATA_ENUM(tWheelState) m_aWheelState[2];
} CBikeData;
//- rouz edit (Claude)
