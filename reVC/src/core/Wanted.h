//+ rouz edit (ChatGPT)
#pragma once
#include "Crime.h"
#include "Validate.h"
typedef struct CWanted {
	int32_t m_nChaos;
	int32_t m_nMinChaos;
	int32_t m_nLastUpdateTime;
	uint32_t m_nLastWantedLevelChange;
	uint32_t m_nLastTimeSuspended;
	float m_fCrimeSensitivity;
	uint8_t m_CurrentCops;
	uint8_t m_MaxCops;
	uint8_t m_MaximumLawEnforcerVehicles;
	uint8_t m_CopsBeatingSuspect;
	int16_t m_RoadblockDensity;
	uint8_t m_bIgnoredByCops : 1;
	uint8_t m_bIgnoredByEveryone : 1;
	uint8_t m_bSwatRequired : 1;
	uint8_t m_bFbiRequired : 1;
	uint8_t m_bArmyRequired : 1;
	int32_t m_nWantedLevel;
	int32_t m_nMinWantedLevel;
	CCrimeBeingQd m_aCrimes[16];
	CCopPed *m_pCops[10];

} CWanted;
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Wanted_MaximumWantedLevel,Wanted_nMaximumWantedLevel;
void Wanted_Initialise(CWanted *self);
bool Wanted_AreMiamiViceRequired(CWanted *self);
bool Wanted_AreSwatRequired(CWanted *self);
bool Wanted_AreFbiRequired(CWanted *self);
bool Wanted_AreArmyRequired(CWanted *self);
int32_t Wanted_NumOfHelisRequired(CWanted *self);
void Wanted_SetWantedLevel(CWanted *self,int32_t level);
void Wanted_SetWantedLevelNoDrop(CWanted *self,int32_t level);
int32_t Wanted_GetWantedLevel(CWanted *self);
void Wanted_CheatWantedLevel(CWanted *self,int32_t level);
void Wanted_RegisterCrime(CWanted *self,enum eCrimeType type,WantedPositionRef coors,uint32_t id,bool policeDoesntCare);
void Wanted_RegisterCrime_Immediately(CWanted *self,enum eCrimeType type,WantedPositionRef coors,uint32_t id,bool policeDoesntCare);
void Wanted_ClearQdCrimes(CWanted *self);
bool Wanted_AddCrimeToQ(CWanted *self,enum eCrimeType type,int32_t id,WantedPositionRef coors,bool reported,bool policeDoesntCare);
void Wanted_ReportCrimeNow(CWanted *self,enum eCrimeType type,WantedPositionRef coors,bool policeDoesntCare);
void Wanted_UpdateWantedLevel(CWanted *self);
void Wanted_Reset(CWanted *self);
void Wanted_ResetPolicePursuit(CWanted *self);
void Wanted_UpdateCrimesQ(CWanted *self);
void Wanted_Update(CWanted *self);
void Wanted_Suspend(CWanted *self);
bool Wanted_IsIgnored(CWanted *self);
int32_t Wanted_WorkOutPolicePresence(WantedVector posn,float radius);
void Wanted_SetMaximumWantedLevel(int32_t level);
void Wanted_ConstructValue(CWanted *self);
void Wanted_Copy(CWanted *self,const CWanted *source);
#ifdef __cplusplus
}
#endif
#ifdef CHECK_STRUCT_SIZES
VALIDATE_SIZE(CWanted,0x204);
#endif
//- rouz edit (ChatGPT)
