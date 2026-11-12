//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "AutoPilot.h"
#include "HandlingMgr.h"
#include "StoredCollPoly.h"

// Enums keep their engine type in C++ and int in C, pointers to engine classes use the forward declared name
#ifdef __cplusplus
#define VEHICLEDATA_ENUM(type) type
#else
#define VEHICLEDATA_ENUM(type) int
typedef struct CEntity CEntity;
typedef struct CPed CPed;
typedef struct CFire CFire;
#endif

// The vehicle state that follows CPhysical, as plain data usable from C
typedef struct CVehicleData {
	tHandlingData *pHandling;
	tFlyingHandlingData *pFlyingHandling;
	CAutoPilot AutoPilot;
	uint8_t m_currentColour1;
	uint8_t m_currentColour2;
	int8_t m_aExtras[2];
	int16_t m_nAlarmState;
	int16_t m_nRouteSeed;
	CPed *pDriver;
	CPed *pPassengers[8];
	uint8_t m_nNumPassengers;
	int8_t m_nNumGettingIn;
	int8_t m_nGettingInFlags;
	int8_t m_nGettingOutFlags;
	uint8_t m_nNumMaxPassengers;
	float field_1D0[4];
	CEntity *m_pCurGroundEntity;
	CFire *m_pCarFire;
	float m_fSteerAngle;
	float m_fGasPedal;
	float m_fBrakePedal;
	uint8_t VehicleCreatedBy;

	// cf. https://github.com/DK22Pac/plugin-sdk/blob/master/plugin_sa/game_sa/CVehicle.h from R*
	uint8_t bIsLawEnforcer: 1; // Is this guy chasing the player at the moment
	uint8_t bIsAmbulanceOnDuty: 1; // Ambulance trying to get to an accident
	uint8_t bIsFireTruckOnDuty: 1; // Firetruck trying to get to a fire
	uint8_t bIsLocked: 1; // Is this guy locked by the script (cannot be removed)
	uint8_t bEngineOn: 1; // For sound purposes. Parked cars have their engines switched off (so do destroyed cars)
	uint8_t bIsHandbrakeOn: 1; // How's the handbrake doing ?
	uint8_t bLightsOn: 1; // Are the lights switched on ?
	uint8_t bFreebies: 1; // Any freebies left in this vehicle ?

	uint8_t bIsVan: 1; // Is this vehicle a van (doors at back of vehicle)
	uint8_t bIsBus: 1; // Is this vehicle a bus
	uint8_t bIsBig: 1; // Is this vehicle a bus
	uint8_t bLowVehicle: 1; // Need this for sporty type cars to use low getting-in/out anims
	uint8_t bComedyControls : 1; // Will make the car hard to control (hopefully in a funny way)
	uint8_t bWarnedPeds : 1; // Has scan and warn peds of danger been processed?
	uint8_t bCraneMessageDone : 1; // A crane message has been printed for this car allready
	uint8_t bExtendedRange : 1; // This vehicle needs to be a bit further away to get deleted

	uint8_t bTakeLessDamage : 1; // This vehicle is stronger (takes about 1/4 of damage)
	uint8_t bIsDamaged : 1; // This vehicle has been damaged and is displaying all its components
	uint8_t bHasBeenOwnedByPlayer : 1;// To work out whether stealing it is a crime
	uint8_t bFadeOut : 1; // Fade vehicle out
	uint8_t bIsBeingCarJacked : 1; // Fade vehicle out
	uint8_t bCreateRoadBlockPeds : 1; // If this vehicle gets close enough we will create peds (coppers or gang members) round it
	uint8_t bCanBeDamaged : 1; // Set to FALSE during cut scenes to avoid explosions
	uint8_t bUsingSpecialColModel : 1;// Is player vehicle using special collision model, stored in player strucure

	uint8_t bOccupantsHaveBeenGenerated : 1;  // Is true if the occupants have already been generated. (Shouldn't happen again)
	uint8_t bGunSwitchedOff : 1;  // Level designers can use this to switch off guns on boats
	uint8_t bVehicleColProcessed : 1;// Has ProcessEntityCollision been processed for this car?
	uint8_t bIsCarParkVehicle : 1; // Car has been created using the special CAR_PARK script command
	uint8_t bHasAlreadyBeenRecorded : 1; // Used for replays
	uint8_t bPartOfConvoy : 1;
	uint8_t bHeliMinimumTilt : 1; // This heli should have almost no tilt really
	uint8_t bAudioChangingGear : 1; // sounds like vehicle is changing gear

	uint8_t bIsDrowning : 1; // is vehicle occupants taking damage in water (i.e. vehicle is dead in water)
	uint8_t bTyresDontBurst : 1; // If this is set the tyres are invincible
	uint8_t bCreatedAsPoliceVehicle : 1;// True if this guy was created as a police vehicle (enforcer, policecar, miamivice car etc)
	uint8_t bRestingOnPhysical : 1; // Dont go static cause car is sitting on a physical object that might get removed
	uint8_t bParking : 1;
	uint8_t bCanPark : 1;
#if (!defined GTA_PS2 || defined FIX_BUGS)
	uint8_t m_bombType : 3;
#endif
	uint8_t bDriverLastFrame : 1;

	int8_t m_numPedsUseItAsCover;
	uint8_t m_nAmmoInClip;    // Used to make the guns on boat do a reload (20 by default)
	int8_t m_nPacManPickupsCarried;
	uint8_t m_nRoadblockType;
	float m_fHealth;           // 1000.0f = full health. 250.0f = fire. 0 -> explode
	uint8_t m_nCurrentGear;
	float m_fChangeGearTime;
#if (!defined GTA_PS2 || defined FIX_BUGS)
	CEntity *m_pBombRigger;
#endif
	uint32_t m_nSetPieceExtendedRangeTime;
	uint32_t m_nGunFiringTime;    // last time when gun on vehicle was fired (used on boats)
	uint32_t m_nTimeOfDeath;
	uint16_t m_nTimeBlocked;
	int16_t m_nBombTimer;        // goes down with each frame
	CEntity *m_pBlowUpEntity;
	float m_fMapObjectHeightAhead;	// front Z?
	float m_fMapObjectHeightBehind;	// rear Z?
	VEHICLEDATA_ENUM(eCarLock) m_nDoorLock;
	int8_t m_nLastWeaponDamage; // see eWeaponType, -1 if no damage
	CEntity *m_pLastDamageEntity;
	uint8_t m_nRadioStation;
	uint8_t m_bRainAudioCounter;
	uint8_t m_bRainSamplesCounter;
	uint32_t m_nCarHornTimer;
	uint8_t m_nCarHornPattern;
	bool m_bSirenOrAlarm;
	uint8_t m_nCarHornDelay;
	int8_t m_comedyControlState;
	CStoredCollPoly m_aCollPolys[2];     // poly which is under front/rear part of car
	float m_fSteerInput;
	VEHICLEDATA_ENUM(eVehicleType) m_vehType;
} CVehicleData;
//- rouz edit (Claude)
