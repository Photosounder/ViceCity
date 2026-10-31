//+ rouz edit (ChatGPT)
#include "Wanted_backend.h"
#include "Timer.h"
#include "ZoneCull.h"
#include "General.h"
#include "ModelIndices.h"
#include "EntityStatus.h"
#include "DMAudio.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define ARRAY_SIZE(value) (sizeof(value)/sizeof((value)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
int32_t Wanted_MaximumWantedLevel=6;
int32_t Wanted_nMaximumWantedLevel=9600;
static WantedVector Wanted_Sub(WantedVector a,WantedVector b)
{
    // Preserve all three original vector subtractions before the distance calculation
    WantedVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float Wanted_Magnitude(WantedVector value)
{
    // Preserve the original ordered squared length and single-precision square root
    return sqrtf(value.x*value.x+value.y*value.y+value.z*value.z);
}
static WantedPositionRef Wanted_ReferencePlain(const WantedVector *value)
{
    // Retain a canonical queued-crime coordinate until its original later read point
    WantedPositionRef result={value,NULL};return result;
}
static WantedVector Wanted_ReadPosition(WantedPositionRef value)
{
    // Read either actual native coordinates or the canonical C record at the original copy statement
    return value.native?Wanted_BackendNativePosition(value.native):*value.plain;
}
static void Wanted_ReportAudio(enum eCrimeType type,WantedPositionRef position)
{
    // Preserve deferred audio eligibility checks before reading either live coordinate representation
    if(position.native)DMAudio_ReportCrime(type,position.native);
    else DMAudio_ReportCrimeAt(type,position.plain);
}
void Wanted_ConstructValue(CWanted *self)
{
    // Preserve the original allocator value-initialization before explicit gameplay initialization
    static const CWanted zero;memcpy(self,&zero,sizeof(*self));
}
void Wanted_Copy(CWanted *self,const CWanted *source)
{
    // Copy every original named record field and pointer for replay restoration
    *self=*source;
}
void Wanted_Initialise(CWanted *self)
{
    // Preserve the complete original Initialise operation and every ordered external access
	self->m_nChaos = 0;
	self->m_nMinChaos = 0;
	self->m_nLastUpdateTime = 0;
	self->m_nLastWantedLevelChange = 0;
	self->m_nLastTimeSuspended = 0;
	self->m_CurrentCops = 0;
	self->m_MaxCops = 0;
	self->m_MaximumLawEnforcerVehicles = 0;
	self->m_RoadblockDensity = 0;
	self->m_bIgnoredByCops = false;
	self->m_bIgnoredByEveryone = false;
	self->m_bSwatRequired = false;
	self->m_bFbiRequired = false;
	self->m_bArmyRequired = false;
	self->m_fCrimeSensitivity = 1.0f;
	self->m_nWantedLevel = 0;
	self->m_nMinWantedLevel = 0;
	self->m_CopsBeatingSuspect = 0;

	for (int i = 0; i < ARRAY_SIZE(self->m_pCops); i++)
		self->m_pCops[i] = NULL;

	Wanted_ClearQdCrimes(self);
}

bool Wanted_AreMiamiViceRequired(CWanted *self)
{
    // Preserve the complete original AreMiamiViceRequired operation and every ordered external access
	return self->m_nWantedLevel >= 3;
}

bool Wanted_AreSwatRequired(CWanted *self)
{
    // Preserve the complete original AreSwatRequired operation and every ordered external access
	return self->m_nWantedLevel == 4 || self->m_bSwatRequired;
}

bool Wanted_AreFbiRequired(CWanted *self)
{
    // Preserve the complete original AreFbiRequired operation and every ordered external access
	return self->m_nWantedLevel == 5 || self->m_bFbiRequired;
}

bool Wanted_AreArmyRequired(CWanted *self)
{
    // Preserve the complete original AreArmyRequired operation and every ordered external access
	return self->m_nWantedLevel == 6 || self->m_bArmyRequired;
}

int32_t Wanted_NumOfHelisRequired(CWanted *self)
{
    // Preserve the complete original NumOfHelisRequired operation and every ordered external access
	if (self->m_bIgnoredByCops || self->m_bIgnoredByEveryone)
		return 0;

	switch (self->m_nWantedLevel) {
	case 3:
	case 4:
		return 1;
	case 5:
	case 6:
		return 1;
	default:
		return 0;
	}
}

void Wanted_SetWantedLevel(CWanted *self,int32_t level)
{
    // Preserve the complete original SetWantedLevel operation and every ordered external access
	if (level > Wanted_MaximumWantedLevel)
		level = Wanted_MaximumWantedLevel;

	Wanted_ClearQdCrimes(self);
	switch (level) {
	case 0:
		self->m_nChaos = 0;
		break;
	case 1:
		self->m_nChaos = 70;
		break;
	case 2:
		self->m_nChaos = 200;
		break;
	case 3:
		self->m_nChaos = 570;
		break;
	case 4:
		self->m_nChaos = 1220;
		break;
	case 5:
		self->m_nChaos = 2420;
		break;
	case 6:
		self->m_nChaos = 4820;
		break;
	default:
		break;
	}
	Wanted_UpdateWantedLevel(self);
}

void Wanted_SetWantedLevelNoDrop(CWanted *self,int32_t level)
{
    // Preserve the complete original SetWantedLevelNoDrop operation and every ordered external access
	if (self->m_nWantedLevel < self->m_nMinWantedLevel)
		Wanted_SetWantedLevel(self,self->m_nMinWantedLevel);

	if (level > self->m_nWantedLevel)
		Wanted_SetWantedLevel(self,level);
}

int32_t Wanted_GetWantedLevel(CWanted *self)
{
    // Preserve the complete original GetWantedLevel operation and every ordered external access
	return self->m_nWantedLevel;
}

void Wanted_CheatWantedLevel(CWanted *self,int32_t level)
{
    // Preserve the complete original CheatWantedLevel operation and every ordered external access
	Wanted_SetWantedLevel(self,level);
	Wanted_UpdateWantedLevel(self);
}

void Wanted_RegisterCrime(CWanted *self,enum eCrimeType type,WantedPositionRef coors,uint32_t id,bool policeDoesntCare)
{
    // Preserve the complete original RegisterCrime operation and every ordered external access
	Wanted_AddCrimeToQ(self,type, id, coors, false, policeDoesntCare);
}

void Wanted_RegisterCrime_Immediately(CWanted *self,enum eCrimeType type,WantedPositionRef coors,uint32_t id,bool policeDoesntCare)
{
    // Preserve the complete original RegisterCrime_Immediately operation and every ordered external access
#ifdef FIX_SIGNIFICANT_BUGS
	if(!Wanted_AddCrimeToQ(self,type, id, coors, true, policeDoesntCare))
#else
	if(!Wanted_AddCrimeToQ(self,type, id, coors, false, policeDoesntCare))
#endif
		Wanted_ReportCrimeNow(self,type, coors, policeDoesntCare);
}

void Wanted_ClearQdCrimes(CWanted *self)
{
    // Preserve the complete original ClearQdCrimes operation and every ordered external access
	for (int i = 0; i < 16; i++)
		self->m_aCrimes[i].m_nType = CRIME_NONE;
}

bool Wanted_AddCrimeToQ(CWanted *self,enum eCrimeType type,int32_t id,WantedPositionRef coors,bool reported,bool policeDoesntCare)
{
    // Preserve the complete original AddCrimeToQ operation and every ordered external access
	int i;

	for(i = 0; i < 16; i++)
		if(self->m_aCrimes[i].m_nType == type && self->m_aCrimes[i].m_nId == id){
			if(self->m_aCrimes[i].m_bReported)
				return true;
			if(reported)
				self->m_aCrimes[i].m_bReported = reported;
			return false;
		}

	for(i = 0; i < 16; i++)
		if(self->m_aCrimes[i].m_nType == CRIME_NONE)
			break;
	if(i < 16){
		self->m_aCrimes[i].m_nType = type;
		self->m_aCrimes[i].m_nId = id;
		self->m_aCrimes[i].m_vecPosn = Wanted_ReadPosition(coors);
		self->m_aCrimes[i].m_nTime = (*Timer_GetTimeInMilliseconds());
		self->m_aCrimes[i].m_bReported = reported;
		self->m_aCrimes[i].m_bPoliceDoesntCare = policeDoesntCare;
	}
	return false;
}

void Wanted_ReportCrimeNow(CWanted *self,enum eCrimeType type,WantedPositionRef coors,bool policeDoesntCare)
{
    // Preserve the complete original ReportCrimeNow operation and every ordered external access
	float sensitivity, chaos;
	int wantedLevelDrop;

	if(Wanted_BackendFrenzy())
		sensitivity = self->m_fCrimeSensitivity*0.3f;
	else
		sensitivity = self->m_fCrimeSensitivity;

	wantedLevelDrop = Min(CullZones_GetWantedLevelDrop(), 100);

	chaos = (1.0f - wantedLevelDrop/100.0f) * sensitivity;
	if (policeDoesntCare)
		chaos *= 0.333f;
	switch(type){
	case CRIME_POSSESSION_GUN:
		break;
	case CRIME_HIT_PED:
		self->m_nChaos += 5.0f*chaos;
		break;
	case CRIME_HIT_COP:
		self->m_nChaos += 45.0f*chaos;
		break;
	case CRIME_SHOOT_PED:
		self->m_nChaos += 30.0f*chaos;
		break;
	case CRIME_SHOOT_COP:
		self->m_nChaos += 80.0f*chaos;
		break;
	case CRIME_STEAL_CAR:
		self->m_nChaos += 15.0f*chaos;
		break;
	case CRIME_RUN_REDLIGHT:
		self->m_nChaos += 10.0f*chaos;
		break;
	case CRIME_RECKLESS_DRIVING:
		self->m_nChaos += 5.0f*chaos;
		break;
	case CRIME_SPEEDING:
		self->m_nChaos += 5.0f*chaos;
		break;
	case CRIME_RUNOVER_PED:
		self->m_nChaos += 18.0f*chaos;
		break;
	case CRIME_RUNOVER_COP:
		self->m_nChaos += 80.0f*chaos;
		break;
	case CRIME_SHOOT_HELI:
		self->m_nChaos += 400.0f*chaos;
		break;
	case CRIME_PED_BURNED:
		self->m_nChaos += 20.0f*chaos;
		break;
	case CRIME_COP_BURNED:
		self->m_nChaos += 80.0f*chaos;
		break;
	case CRIME_VEHICLE_BURNED:
		self->m_nChaos += 20.0f*chaos;
		break;
	case CRIME_DESTROYED_CESSNA:
		self->m_nChaos += 500.0f*chaos;
		break;
	case CRIME_EXPLOSION:
		self->m_nChaos += 25.0f * chaos;
		break;
	case CRIME_HIT_PED_NASTYWEAPON:
		self->m_nChaos += 35.0f * chaos;
		break;
	case CRIME_HIT_COP_NASTYWEAPON:
		self->m_nChaos += 100.0f * chaos;
		break;
	default:
	//	Wanted_BackendError("Undefined crime type, RegisterCrime, Crime.cpp");	// different file for some reason
		Wanted_BackendError("Undefined crime type, RegisterCrime, Wanted.cpp");
	}
	self->m_nChaos = Max(self->m_nChaos, self->m_nMinChaos);
	Wanted_ReportAudio(type,coors);
	Wanted_UpdateWantedLevel(self);
}

void Wanted_UpdateWantedLevel(CWanted *self)
{
    // Preserve the complete original UpdateWantedLevel operation and every ordered external access
	int32_t CurrWantedLevel = self->m_nWantedLevel;

	if (self->m_nChaos > Wanted_nMaximumWantedLevel)
		self->m_nChaos = Wanted_nMaximumWantedLevel;

	if (self->m_nChaos >= 0 && self->m_nChaos < 50) {
		if (self->m_nWantedLevel == 1)
			++(*Wanted_BackendEvaded());
		self->m_nWantedLevel = 0;
		self->m_MaximumLawEnforcerVehicles = 0;
		self->m_MaxCops = 0;
		self->m_RoadblockDensity = 0;
	} else if (self->m_nChaos >= 50 && self->m_nChaos < 180) {
		(*Wanted_BackendAttained()) += 1 - self->m_nWantedLevel;
		self->m_nWantedLevel = 1;
		self->m_MaximumLawEnforcerVehicles = 1;
		self->m_MaxCops = 1;
		self->m_RoadblockDensity = 0;
	} else if (self->m_nChaos >= 180 && self->m_nChaos < 550) {
		(*Wanted_BackendAttained()) += 2 - self->m_nWantedLevel;
		self->m_nWantedLevel = 2;
		self->m_MaximumLawEnforcerVehicles = 2;
		self->m_MaxCops = 3;
		self->m_RoadblockDensity = 0;
	} else if (self->m_nChaos >= 550 && self->m_nChaos < 1200) {
		(*Wanted_BackendAttained()) += 3 - self->m_nWantedLevel;
		self->m_nWantedLevel = 3;
		self->m_MaximumLawEnforcerVehicles = 2;
		self->m_MaxCops = 4;
		self->m_RoadblockDensity = 12;
	} else if (self->m_nChaos >= 1200 && self->m_nChaos < 2400) {
		(*Wanted_BackendAttained()) += 4 - self->m_nWantedLevel;
		self->m_nWantedLevel = 4;
		self->m_MaximumLawEnforcerVehicles = 2;
		self->m_MaxCops = 6;
		self->m_RoadblockDensity = 18;
	} else if (self->m_nChaos >= 2400 && self->m_nChaos < 4800) {
		(*Wanted_BackendAttained()) += 5 - self->m_nWantedLevel;
		self->m_nWantedLevel = 5;
		self->m_MaximumLawEnforcerVehicles = 3;
		self->m_MaxCops = 8;
		self->m_RoadblockDensity = 24;
	} else if (self->m_nChaos >= 4800) {
		(*Wanted_BackendAttained()) += 6 - self->m_nWantedLevel;
		self->m_nWantedLevel = 6;
		self->m_MaximumLawEnforcerVehicles = 3;
		self->m_MaxCops = 10;
		self->m_RoadblockDensity = 30;
	}

	if (CurrWantedLevel != self->m_nWantedLevel)
		self->m_nLastWantedLevelChange = (*Timer_GetTimeInMilliseconds());
}

void Wanted_Reset(CWanted *self)
{
    // Preserve the complete original Reset operation and every ordered external access
	Wanted_ResetPolicePursuit(self);
	Wanted_Initialise(self);
}

void Wanted_ResetPolicePursuit(CWanted *self)
{
    // Preserve the complete original ResetPolicePursuit operation and every ordered external access
	for(int i = 0; i < ARRAY_SIZE(self->m_pCops); i++) {
		CCopPed *cop = self->m_pCops[i];
		if (!cop)
			continue;

		Wanted_BackendClearCopState(cop);
		if (!Wanted_BackendCopDying(cop)) {
			Wanted_BackendCopWander(cop,General_GetRandomNumberInRangeFloat(0.0f, 8.0f));
		}
		self->m_pCops[i] = NULL;
	}
	self->m_CurrentCops = 0;
}

void Wanted_UpdateCrimesQ(CWanted *self)
{
    // Preserve the complete original UpdateCrimesQ operation and every ordered external access
	for(int i = 0; i < ARRAY_SIZE(self->m_aCrimes); i++) {

		CCrimeBeingQd *crime = &self->m_aCrimes[i];
		if (crime->m_nType != CRIME_NONE) {
			if ((*Timer_GetTimeInMilliseconds()) > crime->m_nTime + 500 && !crime->m_bReported) {
				Wanted_ReportCrimeNow(self,crime->m_nType, Wanted_ReferencePlain(&crime->m_vecPosn), crime->m_bPoliceDoesntCare);
				crime->m_bReported = true;
			}
			if ((*Timer_GetTimeInMilliseconds()) > crime->m_nTime + 10000)
				crime->m_nType = CRIME_NONE;
		}
	}
}

void Wanted_Update(CWanted *self)
{
    // Preserve the complete original Update operation and every ordered external access
	if ((*Timer_GetTimeInMilliseconds()) > self->m_nLastTimeSuspended + 20000) {
		self->m_nMinChaos = 0;
		self->m_nMinWantedLevel = 0;
	}
	if ((*Timer_GetTimeInMilliseconds()) - self->m_nLastUpdateTime > 1000) {
		if (self->m_nWantedLevel > 1) {
			self->m_nLastUpdateTime = (*Timer_GetTimeInMilliseconds());
		} else {
			float radius = 18.0f;
			WantedVector playerPos = Wanted_BackendPlayerCoors();
			if (Wanted_WorkOutPolicePresence(playerPos, radius) == 0) {
				self->m_nLastUpdateTime = (*Timer_GetTimeInMilliseconds());
				self->m_nChaos = Max(0, self->m_nChaos - 1);
				Wanted_UpdateWantedLevel(self);
			}
		}
		Wanted_UpdateCrimesQ(self);
		bool orderMessedUp = false;
		int currCopNum = 0;
		bool foundEmptySlot = false;
		for (int i = 0; i < ARRAY_SIZE(self->m_pCops); i++) {
			if (self->m_pCops[i]) {
				++currCopNum;
				if (foundEmptySlot)
					orderMessedUp = true;
			} else {
				foundEmptySlot = true;
			}
		}
		if (currCopNum != self->m_CurrentCops) {
			printf("CopPursuit total messed up: re-setting\n");
			self->m_CurrentCops = currCopNum;
		}
		if (orderMessedUp) {
			printf("CopPursuit pointer list messed up: re-sorting\n");
			bool fixed = true;
			for (int i = 0; i < ARRAY_SIZE(self->m_pCops); i++) {
				if (!self->m_pCops[i]) {
					for (int j = i; j < ARRAY_SIZE(self->m_pCops); j++) {
						if (self->m_pCops[j]) {
							self->m_pCops[i] = self->m_pCops[j];
							self->m_pCops[j] = NULL;
							fixed = false;
							break;
						}
					}
					if (fixed)
						break;
				}
			}
		}
	}
}

void Wanted_Suspend(CWanted *self)
{
    // Preserve the complete original Suspend operation and every ordered external access
	(*Wanted_BackendEvaded()) += self->m_nWantedLevel;
	self->m_nMinChaos = self->m_nChaos;
	self->m_nMinWantedLevel = self->m_nWantedLevel;
	self->m_nLastTimeSuspended = (*Timer_GetTimeInMilliseconds());
	self->m_nChaos = 0;
	self->m_nWantedLevel = 0;
	Wanted_ResetPolicePursuit(self);
}

bool Wanted_IsIgnored(CWanted *self)
{
    // Preserve the complete original IsIgnored operation and every ordered external access
	return self->m_bIgnoredByCops || self->m_bIgnoredByEveryone;
}

int32_t Wanted_WorkOutPolicePresence(WantedVector posn,float radius)
{
    // Preserve the complete original WorkOutPolicePresence operation and every ordered external access
	int i;
	CPed *ped;
	CVehicle *vehicle;
	int numPolice = 0;

		// Access raw storage through the C store or pool API
	i = CPool_GetSize(Wanted_BackendPedPool());
		while(--i >= 0){
				// Access raw storage through the C store or pool API
		ped = ((CPed*)CPool_GetSlot(Wanted_BackendPedPool(), i));
				if(ped &&
		   IsPolicePedModel(Wanted_BackendPedModel(ped)) &&
		   Wanted_Magnitude(Wanted_Sub(posn,Wanted_BackendPedPosition(ped))) < radius)
			numPolice++;
	}

		// Access raw storage through the C store or pool API
	i = CPool_GetSize(Wanted_BackendVehiclePool());
		while(--i >= 0){
				// Access raw storage through the C store or pool API
		vehicle = ((CVehicle*)CPool_GetSlot(Wanted_BackendVehiclePool(), i));
				if(vehicle &&
		   Wanted_BackendLawEnforcer(vehicle) &&
		   IsPoliceVehicleModel(Wanted_BackendVehicleModel(vehicle)) &&
		   vehicle != Wanted_BackendPlayerVehicle() &&
		   Wanted_BackendVehicleStatus(vehicle) != STATUS_ABANDONED && Wanted_BackendVehicleStatus(vehicle) != STATUS_WRECKED &&
		   Wanted_Magnitude(Wanted_Sub(posn,Wanted_BackendVehiclePosition(vehicle))) < radius)
			numPolice++;
	}

	return numPolice;
}

void Wanted_SetMaximumWantedLevel(int32_t level)
{
    // Preserve the complete original SetMaximumWantedLevel operation and every ordered external access
	switch(level){
	case 0:
		Wanted_nMaximumWantedLevel = 0;
		Wanted_MaximumWantedLevel = 0;
		break;
	case 1:
		Wanted_nMaximumWantedLevel = 115;
		Wanted_MaximumWantedLevel = 1;
		break;
	case 2:
		Wanted_nMaximumWantedLevel = 365;
		Wanted_MaximumWantedLevel = 2;
		break;
	case 3:
		Wanted_nMaximumWantedLevel = 875;
		Wanted_MaximumWantedLevel = 3;
		break;
	case 4:
		Wanted_nMaximumWantedLevel = 1800;
		Wanted_MaximumWantedLevel = 4;
		break;
	case 5:
		Wanted_nMaximumWantedLevel = 3600;
		Wanted_MaximumWantedLevel = 5;
		break;
	case 6:
		Wanted_nMaximumWantedLevel = 7200;
		Wanted_MaximumWantedLevel = 6;
		break;
	}
}
//- rouz edit (ChatGPT)
