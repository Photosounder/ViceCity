//+ rouz edit (Claude)
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"
#include "FileMgr.h"
#include "WorkBuffer.h"
#include "HandlingMgr.h"

void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#ifndef MASTER
#define HandlingMgr_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define HandlingMgr_Assert(expression) (expression)
#endif

#define GRAVITY (0.008f)
#define SQR(x) ((x) * (x))

static const char *HandlingFilename = "HANDLING.CFG";

static const char VehicleNames[NUMHANDLINGS][14] = {
	"LANDSTAL",
	"IDAHO",
	"STINGER",
	"LINERUN",
	"PEREN",
	"SENTINEL",
	"PATRIOT",
	"FIRETRUK",
	"TRASH",
	"STRETCH",
	"MANANA",
	"INFERNUS",
	"PONY",
	"MULE",
	"CHEETAH",
	"AMBULAN",
	"FBICAR",
	"MOONBEAM",
	"ESPERANT",
	"TAXI",
	"KURUMA",
	"BOBCAT",
	"MRWHOOP",
	"BFINJECT",
	"POLICE",
	"ENFORCER",
	"SECURICA",
	"BANSHEE",
	"BUS",
	"RHINO",
	"BARRACKS",
	"TRAIN",
	"HELI",
	"DODO",
	"COACH",
	"CABBIE",
	"STALLION",
	"RUMPO",
	"RCBANDIT",
	"MAFIA",
	"AIRTRAIN",
	"DEADDODO",
	"FLATBED",
	"YANKEE",
	"GOLFCART",
	"VOODOO",
	"WASHING",
	"CUBAN",
	"ROMERO",
	"PACKER",
	"ADMIRAL",
	"GANGBUR",
	"ZEBRA",
	"TOPFUN",
	"GLENDALE",
	"OCEANIC",
	"HERMES",
	"SABRE1",
	"SABRETUR",
	"PHEONIX",
	"WALTON",
	"REGINA",
	"COMET",
	"DELUXO",
	"BURRITO",
	"SPAND",
	"BAGGAGE",
	"KAUFMAN",
	"RANCHER",
	"FBIRANCH",
	"VIRGO",
	"GREENWOO",
	"HOTRING",
	"SANDKING",
	"BLISTAC",
	"BOXVILLE",
	"BENSON",
	"DESPERAD",
	"LOVEFIST",
	"BLOODRA",
	"BLOODRB",
	"BIKE",
	"MOPED",
	"DIRTBIKE",
	"ANGEL",
	"FREEWAY",
	"PREDATOR",
	"SPEEDER",
	"REEFER",
	"RIO",
	"SQUALO",
	"TROPIC",
	"COASTGRD",
	"DINGHY",
	"MARQUIS",
	"CUPBOAT",
	"SEAPLANE",
	"SPARROW",
	"SEASPAR",
	"MAVERICK",
	"COASTMAV",
	"POLMAV",
	"HUNTER",
	"RCBARON",
	"RCGOBLIN",
	"RCCOPTER"
};

void
HandlingMgr_Initialise(cHandlingDataMgrState *self)
{
	// Load the file then set the global constants
	HandlingMgr_LoadHandlingData(self);
	self->field_0 = 0.1f;
	self->fWheelFriction = 0.9f;
	self->field_8 = 1.0f;
	self->field_C = 0.8f;
	self->field_10 = 0.98f;
}

void
HandlingMgr_LoadHandlingData(cHandlingDataMgrState *self)
{
	// Parse HANDLING.CFG line by line
	char *start, *end;
	char line[201];	// weird value
	char delim[4];	// not sure
	char *word;
	int field, handlingId;
	int keepGoing;
	tHandlingData *handling;
	tFlyingHandlingData *flyingHandling;
	tBoatHandlingData *boatHandling;
	tBikeHandlingData *bikeHandling;

	FileMgr_SetDir("DATA");
	FileMgr_LoadFile(HandlingFilename, work_buff, sizeof(work_buff), "r");
	FileMgr_SetDir("");

	start = (char*)work_buff;
	end = start+1;
	handling = NULL;
	flyingHandling = NULL;
	boatHandling = NULL;
	bikeHandling = NULL;
	keepGoing = 1;

	while(keepGoing){
		// find end of line
		while(*end != '\n') end++;

		// get line
		strncpy(line, start, end - start);
		line[end - start] = '\0';
		start = end+1;
		end = start+1;

		// yeah, this is kinda crappy
		if(strcmp(line, ";the end") == 0)
			keepGoing = 0;
		else if(line[0] != ';'){
			if(line[0] == '!'){
				// Bike data
				field = 0;
				strcpy(delim, " \t");
				// FIX: game seems to use a do-while loop here
				for(word = strtok(line, delim); word; word = strtok(NULL, delim)){
					switch(field){
					case  0: break;
					case  1:
						handlingId = HandlingMgr_FindExactWord(word, (const char*)VehicleNames, 14, NUMHANDLINGS);
						HandlingMgr_Assert(handlingId >= 0 && handlingId < NUMHANDLINGS);
						bikeHandling = &self->BikeHandlingData[handlingId-HANDLING_BIKE];
						bikeHandling->nIdentifier = (tVehicleType)handlingId;
						break;
					case  2: bikeHandling->fLeanFwdCOM = atof(word); break;
					case  3: bikeHandling->fLeanFwdForce = atof(word); break;
					case  4: bikeHandling->fLeanBakCOM = atof(word); break;
					case  5: bikeHandling->fLeanBackForce = atof(word); break;
					case  6: bikeHandling->fMaxLean = atof(word); break;
					case  7: bikeHandling->fFullAnimLean = atof(word); break;
					case  8: bikeHandling->fDesLean = atof(word); break;
					case  9: bikeHandling->fSpeedSteer = atof(word); break;
					case 10: bikeHandling->fSlipSteer = atof(word); break;
					case 11: bikeHandling->fNoPlayerCOMz = atof(word); break;
					case 12: bikeHandling->fWheelieAng = atof(word); break;
					case 13: bikeHandling->fStoppieAng = atof(word); break;
					case 14: bikeHandling->fWheelieSteer = atof(word); break;
					case 15: bikeHandling->fWheelieStabMult = atof(word); break;
					case 16: bikeHandling->fStoppieStabMult = atof(word); break;
					}
					field++;
				}
				HandlingMgr_ConvertBikeDataToGameUnits(bikeHandling);
			}else if(line[0] == '$'){
				// Flying data
				field = 0;
				strcpy(delim, " \t");
				// FIX: game seems to use a do-while loop here
				for(word = strtok(line, delim); word; word = strtok(NULL, delim)){
					switch(field){
					case  0: break;
					case  1:
						handlingId = HandlingMgr_FindExactWord(word, (const char*)VehicleNames, 14, NUMHANDLINGS);
						HandlingMgr_Assert(handlingId >= 0 && handlingId < NUMHANDLINGS);
						flyingHandling = HandlingMgr_GetFlyingPointer(self, handlingId);
						flyingHandling->nIdentifier = (tVehicleType)handlingId;
						break;
					case  2: flyingHandling->fThrust = atof(word); break;
					case  3: flyingHandling->fThrustFallOff = atof(word); break;
					case  4: flyingHandling->fYaw = atof(word); break;
					case  5: flyingHandling->fYawStab = atof(word); break;
					case  6: flyingHandling->fSideSlip = atof(word); break;
					case  7: flyingHandling->fRoll = atof(word); break;
					case  8: flyingHandling->fRollStab = atof(word); break;
					case  9: flyingHandling->fPitch = atof(word); break;
					case 10: flyingHandling->fPitchStab = atof(word); break;
					case 11: flyingHandling->fFormLift = atof(word); break;
					case 12: flyingHandling->fAttackLift = atof(word); break;
					case 13: flyingHandling->fMoveRes = atof(word); break;
					case 14: flyingHandling->vecTurnRes.x = atof(word); break;
					case 15: flyingHandling->vecTurnRes.y = atof(word); break;
					case 16: flyingHandling->vecTurnRes.z = atof(word); break;
					case 17: flyingHandling->vecSpeedRes.x = atof(word); break;
					case 18: flyingHandling->vecSpeedRes.y = atof(word); break;
					case 19: flyingHandling->vecSpeedRes.z = atof(word); break;
					}
					field++;
				}
			}else if(line[0] == '%'){
				// Boat data
				field = 0;
				strcpy(delim, " \t");
				// FIX: game seems to use a do-while loop here
				for(word = strtok(line, delim); word; word = strtok(NULL, delim)){
					switch(field){
					case  0: break;
					case  1:
						handlingId = HandlingMgr_FindExactWord(word, (const char*)VehicleNames, 14, NUMHANDLINGS);
						HandlingMgr_Assert(handlingId >= 0 && handlingId < NUMHANDLINGS);
						boatHandling = HandlingMgr_GetBoatPointer(self, handlingId);
						boatHandling->nIdentifier = (tVehicleType)handlingId;
						break;
					case  2: boatHandling->fThrustY = atof(word); break;
					case  3: boatHandling->fThrustZ = atof(word); break;
					case  4: boatHandling->fThrustAppZ = atof(word); break;
					case  5: boatHandling->fAqPlaneForce = atof(word); break;
					case  6: boatHandling->fAqPlaneLimit = atof(word); break;
					case  7: boatHandling->fAqPlaneOffset = atof(word); break;
					case  8: boatHandling->fWaveAudioMult = atof(word); break;
					case  9: boatHandling->vecMoveRes.x = atof(word); break;
					case 10: boatHandling->vecMoveRes.y = atof(word); break;
					case 11: boatHandling->vecMoveRes.z = atof(word); break;
					case 12: boatHandling->vecTurnRes.x = atof(word); break;
					case 13: boatHandling->vecTurnRes.y = atof(word); break;
					case 14: boatHandling->vecTurnRes.z = atof(word); break;
					case 15: boatHandling->fLook_L_R_BehindCamHeight = atof(word); break;
					}
					field++;
				}
			}else{
				field = 0;
				strcpy(delim, " \t");
				// FIX: game seems to use a do-while loop here
				for(word = strtok(line, delim); word; word = strtok(NULL, delim)){
					switch(field){
					case  0:
						handlingId = HandlingMgr_FindExactWord(word, (const char*)VehicleNames, 14, NUMHANDLINGS);
						HandlingMgr_Assert(handlingId >= 0 && handlingId < NUMHANDLINGS);
						handling = &self->HandlingData[handlingId];
						handling->nIdentifier = (tVehicleType)handlingId;
						break;
					case  1: handling->fMass = atof(word); break;
					case  2: handling->Dimension.x = atof(word); break;
					case  3: handling->Dimension.y = atof(word); break;
					case  4: handling->Dimension.z = atof(word); break;
					case  5: handling->CentreOfMass.x = atof(word); break;
					case  6: handling->CentreOfMass.y = atof(word); break;
					case  7: handling->CentreOfMass.z = atof(word); break;
					case  8: handling->nPercentSubmerged = atoi(word); break;
					case  9: handling->fTractionMultiplier = atof(word); break;
					case 10: handling->fTractionLoss = atof(word); break;
					case 11: handling->fTractionBias = atof(word); break;
					case 12: handling->Transmission.nNumberOfGears = atoi(word); break;
					case 13: handling->Transmission.fMaxVelocity = atof(word); break;
					case 14: handling->Transmission.fEngineAcceleration = atof(word) * 0.4; break;
					case 15: handling->Transmission.nDriveType = word[0]; break;
					case 16: handling->Transmission.nEngineType = word[0]; break;
					case 17: handling->fBrakeDeceleration = atof(word); break;
					case 18: handling->fBrakeBias = atof(word); break;
					case 19: handling->bABS = !!atoi(word); break;
					case 20: handling->fSteeringLock = atof(word); break;
					case 21: handling->fSuspensionForceLevel = atof(word); break;
					case 22: handling->fSuspensionDampingLevel = atof(word); break;
					case 23: handling->fSeatOffsetDistance = atof(word); break;
					case 24: handling->fCollisionDamageMultiplier = atof(word); break;
					case 25: handling->nMonetaryValue = atoi(word); break;
					case 26: handling->fSuspensionUpperLimit = atof(word); break;
					case 27: handling->fSuspensionLowerLimit = atof(word); break;
					case 28: handling->fSuspensionBias = atof(word); break;
					case 29: handling->fSuspensionAntidiveMultiplier = atof(word); break;
					case 30:
						sscanf(word, "%x", &handling->Flags);
						handling->Transmission.Flags = handling->Flags;
						break;
					case 31: handling->FrontLights = atoi(word); break;
					case 32: handling->RearLights = atoi(word); break;
					}
					field++;
				}
				HandlingMgr_ConvertDataToGameUnits(handling);
			}
		}
	}
}

int
HandlingMgr_FindExactWord(const char *word, const char *words, int wordLen, int numWords)
{
	int i;

	for(i = 0; i < numWords; i++){
		// BUG: the game does something really stupid here, it's fixed here
		if(strncmp(word, words, wordLen) == 0)
			return i;
		words += wordLen;
	}
	return numWords;
}


void
HandlingMgr_ConvertDataToGameUnits(tHandlingData *handling)
{
	// convert distance to m, time to 1/50s
	float velocity, a, b;

	handling->Transmission.fEngineAcceleration *= 1.0f/(50.0f*50.0f);
	handling->Transmission.fMaxVelocity *= 1000.0f/(60.0f*60.0f * 50.0f);
	handling->fBrakeDeceleration *= 1.0f/(50.0f*50.0f);
	handling->fTurnMass = (SQR(handling->Dimension.x) + SQR(handling->Dimension.y)) * handling->fMass / 12.0f;
	if(handling->fTurnMass < 10.0f)
		handling->fTurnMass *= 5.0f;
	handling->fInvMass = 1.0f/handling->fMass;
	handling->fCollisionDamageMultiplier *= 2000.0f/handling->fMass;
	handling->fBuoyancy = 100.0f/handling->nPercentSubmerged * GRAVITY*handling->fMass;

	// Don't quite understand this. What seems to be going on is that
	// we calculate a drag (air resistance) deceleration for a given velocity and
	// find the intersection between that and the max engine acceleration.
	// at that point the car cannot accelerate any further and we've found the max velocity.
	a = 0.0f;
	b = 100.0f;
	velocity = handling->Transmission.fMaxVelocity;
	while(a < b && velocity > 0.0f){
		velocity -= 0.01f;
		// what's the 1/6?
		a = handling->Transmission.fEngineAcceleration/6.0f;
		// no density or drag coefficient here...
		float a_drag = 0.5f*SQR(velocity) * handling->Dimension.x*handling->Dimension.z / handling->fMass;
		// can't make sense of this... maybe  v - v/(drag + 1)  ? but that doesn't make so much sense either
		b = -velocity * (1.0f/(a_drag + 1.0f) - 1.0f);
	}

	if(handling->nIdentifier == HANDLING_RCBANDIT){
		handling->Transmission.fMaxCruiseVelocity = handling->Transmission.fMaxVelocity;
		handling->Transmission.fMaxReverseVelocity = -handling->Transmission.fMaxVelocity;
	}else if(handling->nIdentifier >= HANDLING_BIKE && handling->nIdentifier <= HANDLING_FREEWAY){
		handling->Transmission.fMaxCruiseVelocity = velocity;
		handling->Transmission.fMaxVelocity = velocity * 1.2f;
		handling->Transmission.fMaxReverseVelocity = -0.05f;
	}else{
		handling->Transmission.fMaxCruiseVelocity = velocity;
		handling->Transmission.fMaxVelocity = velocity * 1.2f;
		handling->Transmission.fMaxReverseVelocity = -0.2f;
	}

	if(handling->Transmission.nDriveType == '4')
		handling->Transmission.fEngineAcceleration /= 4.0f;
	else
		handling->Transmission.fEngineAcceleration /= 2.0f;

	Transmission_InitGearRatios(&handling->Transmission);
}

void
HandlingMgr_ConvertBikeDataToGameUnits(tBikeHandlingData *handling)
{
	handling->fMaxLean = sinf(DEGTORAD(handling->fMaxLean));
	handling->fFullAnimLean = DEGTORAD(handling->fFullAnimLean);
	handling->fWheelieAng = sinf(DEGTORAD(handling->fWheelieAng));
	handling->fStoppieAng = sinf(DEGTORAD(handling->fStoppieAng));
}

int32_t
HandlingMgr_GetHandlingId(const char *name)
{
	int i;
	for(i = 0; i < NUMHANDLINGS; i++)
		if(strncmp(VehicleNames[i], name, 14) == 0)
			break;
	return i;
}

tFlyingHandlingData*
HandlingMgr_GetFlyingPointer(cHandlingDataMgrState *self, uint8_t id)
{
	if(id >= HANDLING_SEAPLANE && id <= HANDLING_RCCOPTER)
		return &self->FlyingHandlingData[id-HANDLING_SEAPLANE];
	return &self->FlyingHandlingData[0];
}

tBoatHandlingData*
HandlingMgr_GetBoatPointer(cHandlingDataMgrState *self, uint8_t id)
{
	if(id >= HANDLING_PREDATOR && id <= HANDLING_SEAPLANE)
		return &self->BoatHandlingData[id-HANDLING_PREDATOR];
	return &self->BoatHandlingData[0];
}
//- rouz edit (Claude)
