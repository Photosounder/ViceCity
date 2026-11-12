//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "Transmission.h"

// The vector members keep the engine HandlingVec type in C++ and the same three floats in C
#ifdef __cplusplus
typedef CVector HandlingVec;
#else
typedef struct HandlingVec {float x,y,z;} HandlingVec;
#endif

typedef enum tVehicleType
{
	HANDLING_LANDSTAL,
	HANDLING_IDAHO,
	HANDLING_STINGER,
	HANDLING_LINERUN,
	HANDLING_PEREN,
	HANDLING_SENTINEL,
	HANDLING_PATRIOT,
	HANDLING_FIRETRUK,
	HANDLING_TRASH,
	HANDLING_STRETCH,
	HANDLING_MANANA,
	HANDLING_INFERNUS,
	HANDLING_PONY,
	HANDLING_MULE,
	HANDLING_CHEETAH,
	HANDLING_AMBULAN,
	HANDLING_FBICAR,
	HANDLING_MOONBEAM,
	HANDLING_ESPERANT,
	HANDLING_TAXI,
	HANDLING_KURUMA,
	HANDLING_BOBCAT,
	HANDLING_MRWHOOP,
	HANDLING_BFINJECT,
	HANDLING_POLICE,
	HANDLING_ENFORCER,
	HANDLING_SECURICA,
	HANDLING_BANSHEE,
	HANDLING_BUS,
	HANDLING_RHINO,
	HANDLING_BARRACKS,
	HANDLING_TRAIN,
	HANDLING_HELI,
	HANDLING_DODO,
	HANDLING_COACH,
	HANDLING_CABBIE,
	HANDLING_STALLION,
	HANDLING_RUMPO,
	HANDLING_RCBANDIT,
	HANDLING_MAFIA,
	HANDLING_AIRTRAIN,
	HANDLING_DEADDODO,
	HANDLING_FLATBED,
	HANDLING_YANKEE,
	HANDLING_GOLFCART,
	HANDLING_VOODOO,
	HANDLING_WASHING,
	HANDLING_CUBAN,
	HANDLING_ROMERO,
	HANDLING_PACKER,
	HANDLING_ADMIRAL,
	HANDLING_GANGBUR,
	HANDLING_ZEBRA,
	HANDLING_TOPFUN,
	HANDLING_GLENDALE,
	HANDLING_OCEANIC,
	HANDLING_HERMES,
	HANDLING_SABRE1,
	HANDLING_SABRETUR,
	HANDLING_PHEONIX,
	HANDLING_WALTON,
	HANDLING_REGINA,
	HANDLING_COMET,
	HANDLING_DELUXO,
	HANDLING_BURRITO,
	HANDLING_SPAND,
	HANDLING_BAGGAGE,
	HANDLING_KAUFMAN,
	HANDLING_RANCHER,
	HANDLING_FBIRANCH,
	HANDLING_VIRGO,
	HANDLING_GREENWOO,
	HANDLING_HOTRING,
	HANDLING_SANDKING,
	HANDLING_BLISTAC,
	HANDLING_BOXVILLE,
	HANDLING_BENSON,
	HANDLING_DESPERAD,
	HANDLING_LOVEFIST,
	HANDLING_BLOODRA,
	HANDLING_BLOODRB,

	HANDLING_BIKE,
	HANDLING_MOPED,
	HANDLING_DIRTBIKE,
	HANDLING_ANGEL,
	HANDLING_FREEWAY,

	HANDLING_PREDATOR,
	HANDLING_SPEEDER,
	HANDLING_REEFER,
	HANDLING_RIO,
	HANDLING_SQUALO,
	HANDLING_TROPIC,
	HANDLING_COASTGRD,
	HANDLING_DINGHY,
	HANDLING_MARQUIS,
	HANDLING_CUPBOAT,
	HANDLING_SEAPLANE,	// both boat and plane!
	HANDLING_SPARROW,
	HANDLING_SEASPAR,
	HANDLING_MAVERICK,
	HANDLING_COASTMAV,
	HANDLING_POLMAV,
	HANDLING_HUNTER,
	HANDLING_RCBARON,
	HANDLING_RCGOBLIN,
	HANDLING_RCCOPTER,

	NUMHANDLINGS,

	NUMBIKEHANDLINGS = HANDLING_FREEWAY+1 - HANDLING_BIKE,
	NUMFLYINGHANDLINGS = HANDLING_RCCOPTER+1 - HANDLING_SEAPLANE,
	NUMBOATHANDLINGS = HANDLING_SEAPLANE+1 - HANDLING_PREDATOR,
} tVehicleType;

#include "HandlingFlags.h"

typedef struct tHandlingData
{
	tVehicleType nIdentifier;
	float fMass;
	float fInvMass;
	float fTurnMass;
	HandlingVec Dimension;
	HandlingVec CentreOfMass;
	int8_t nPercentSubmerged;
	float fBuoyancy;
	float fTractionMultiplier;
	cTransmission Transmission;
	float fBrakeDeceleration;
	float fBrakeBias;
	int8_t bABS;
	float fSteeringLock;
	float fTractionLoss;
	float fTractionBias;
	float fUnused;
	float fSuspensionForceLevel;
	float fSuspensionDampingLevel;
	float fSuspensionUpperLimit;
	float fSuspensionLowerLimit;
	float fSuspensionBias;
	float fSuspensionAntidiveMultiplier;
	float fCollisionDamageMultiplier;
	uint32_t Flags;
	float fSeatOffsetDistance;
	int32_t nMonetaryValue;
	int8_t FrontLights;
	int8_t RearLights;
} tHandlingData;

typedef struct tBikeHandlingData
{
	tVehicleType nIdentifier;
	float fLeanFwdCOM;
	float fLeanFwdForce;
	float fLeanBakCOM;
	float fLeanBackForce;
	float fMaxLean;
	float fFullAnimLean;
	float fDesLean;
	float fSpeedSteer;
	float fSlipSteer;
	float fNoPlayerCOMz;
	float fWheelieAng;
	float fStoppieAng;
	float fWheelieSteer;
	float fWheelieStabMult;
	float fStoppieStabMult;
} tBikeHandlingData;

typedef struct tBoatHandlingData
{
	tVehicleType nIdentifier;
	float fThrustY;
	float fThrustZ;
	float fThrustAppZ;
	float fAqPlaneForce;
	float fAqPlaneLimit;
	float fAqPlaneOffset;
	float fWaveAudioMult;
	float fLook_L_R_BehindCamHeight;
	HandlingVec vecMoveRes;
	HandlingVec vecTurnRes;
} tBoatHandlingData;

typedef struct tFlyingHandlingData
{
	tVehicleType nIdentifier;
	float fThrust;
	float fThrustFallOff;
	float fYaw;
	float fYawStab;
	float fSideSlip;
	float fRoll;
	float fRollStab;
	float fPitch;
	float fPitchStab;
	float fFormLift;
	float fAttackLift;
	float fMoveRes;
	HandlingVec vecTurnRes;
	HandlingVec vecSpeedRes;
} tFlyingHandlingData;

typedef struct cHandlingDataMgrState
{
	float field_0;	// unused it seems
	float fWheelFriction;	// wheel related
	float field_8;	//
	float field_C;	// unused it seems
	float field_10;	//
	tHandlingData HandlingData[NUMHANDLINGS];
	tBikeHandlingData BikeHandlingData[NUMBIKEHANDLINGS];
	tFlyingHandlingData FlyingHandlingData[NUMFLYINGHANDLINGS];
	tBoatHandlingData BoatHandlingData[NUMBOATHANDLINGS];
} cHandlingDataMgrState;

#ifdef __cplusplus
extern "C" {
#endif
void HandlingMgr_Initialise(cHandlingDataMgrState *self);
void HandlingMgr_LoadHandlingData(cHandlingDataMgrState *self);
int HandlingMgr_FindExactWord(const char *word, const char *words, int wordLen, int numWords);
void HandlingMgr_ConvertDataToGameUnits(tHandlingData *handling);
void HandlingMgr_ConvertBikeDataToGameUnits(tBikeHandlingData *handling);
int32_t HandlingMgr_GetHandlingId(const char *name);
tFlyingHandlingData *HandlingMgr_GetFlyingPointer(cHandlingDataMgrState *self, uint8_t id);
tBoatHandlingData *HandlingMgr_GetBoatPointer(cHandlingDataMgrState *self, uint8_t id);
#ifdef __cplusplus
}

class cHandlingDataMgr : public cHandlingDataMgrState
{
public:
	cHandlingDataMgr(void) { memset((void*)this, 0, sizeof(*this)); }
	void Initialise(void) { HandlingMgr_Initialise(this); }
	void LoadHandlingData(void) { HandlingMgr_LoadHandlingData(this); }
	int FindExactWord(const char *word, const char *words, int wordLen, int numWords) { return HandlingMgr_FindExactWord(word, words, wordLen, numWords); }
	void ConvertDataToGameUnits(tHandlingData *handling) { HandlingMgr_ConvertDataToGameUnits(handling); }
	void ConvertBikeDataToGameUnits(tBikeHandlingData *handling) { HandlingMgr_ConvertBikeDataToGameUnits(handling); }
	int32_t GetHandlingId(const char *name) { return HandlingMgr_GetHandlingId(name); }
	tHandlingData *GetHandlingData(tVehicleType id) { return &HandlingData[id]; }
	tBikeHandlingData *GetBikePointer(uint8_t id) { return &BikeHandlingData[id-HANDLING_BIKE]; }
	tFlyingHandlingData *GetFlyingPointer(uint8_t id) { return HandlingMgr_GetFlyingPointer(this, id); }
	tBoatHandlingData *GetBoatPointer(uint8_t id) { return HandlingMgr_GetBoatPointer(this, id); }
	bool HasRearWheelDrive(tVehicleType id) { return HandlingData[id].Transmission.nDriveType != 'F'; }
	bool HasFrontWheelDrive(tVehicleType id) { return HandlingData[id].Transmission.nDriveType != 'R'; }
};
extern cHandlingDataMgr mod_HandlingManager;
#endif
//- rouz edit (Claude)
