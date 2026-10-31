//+ rouz edit (ChatGPT)
#pragma once

#include "Transmission.h"

enum tVehicleType
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
};

enum tField // most likely a handling field enum, never used so :shrug:
{

};

#include "HandlingFlags.h"

struct tHandlingData
{
	tVehicleType nIdentifier;
	float fMass;
	float fInvMass;
	float fTurnMass;
	CVector Dimension;
	CVector CentreOfMass;
	int8 nPercentSubmerged;
	float fBuoyancy;
	float fTractionMultiplier;
	cTransmission Transmission;
	float fBrakeDeceleration;
	float fBrakeBias;
	int8 bABS;
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
	uint32 Flags;
	float fSeatOffsetDistance;
	int32 nMonetaryValue;
	int8 FrontLights;
	int8 RearLights;
};

struct tBikeHandlingData
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
};

struct tBoatHandlingData
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
	CVector vecMoveRes;
	CVector vecTurnRes;
};

struct  tFlyingHandlingData
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
	CVector vecTurnRes;
	CVector vecSpeedRes;
};

class CVehicle;

class cHandlingDataMgr
{
	float field_0;	// unused it seems
public:
	float fWheelFriction;	// wheel related
private:
	float field_8;	//
	float field_C;	// unused it seems
	float field_10;	//
	tHandlingData HandlingData[NUMHANDLINGS];
	tBikeHandlingData BikeHandlingData[NUMBIKEHANDLINGS];
	tFlyingHandlingData FlyingHandlingData[NUMFLYINGHANDLINGS];
	tBoatHandlingData BoatHandlingData[NUMBOATHANDLINGS];

public:
	cHandlingDataMgr(void);
	void Initialise(void);
	void LoadHandlingData(void);
	int FindExactWord(const char *word, const char *words, int wordLen, int numWords);
	void ConvertDataToWorldUnits(tHandlingData *handling);
	void ConvertDataToGameUnits(tHandlingData *handling);
	void ConvertBikeDataToGameUnits(tBikeHandlingData *handling);
	int32 GetHandlingId(const char *name);
	tHandlingData *GetHandlingData(tVehicleType id) { return &HandlingData[id]; }
	tBikeHandlingData *GetBikePointer(uint8 id) { return &BikeHandlingData[id-HANDLING_BIKE]; }
	tFlyingHandlingData *GetFlyingPointer(uint8 id);
	tBoatHandlingData *GetBoatPointer(uint8 id);
	bool HasRearWheelDrive(tVehicleType id) { return HandlingData[id].Transmission.nDriveType != 'F'; }
	bool HasFrontWheelDrive(tVehicleType id) { return HandlingData[id].Transmission.nDriveType != 'R'; }
};
extern cHandlingDataMgr mod_HandlingManager;
//- rouz edit (ChatGPT)
