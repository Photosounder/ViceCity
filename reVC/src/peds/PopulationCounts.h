//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "PedType.h"
#ifdef __cplusplus
extern "C" {
#endif
extern uint32_t Population_nTotalMissionPeds;
extern uint32_t Population_nNumCivMale;
extern uint32_t Population_nNumCivFemale;
extern uint32_t Population_nNumCop;
extern uint32_t Population_nNumEmergency;
extern uint32_t Population_nNumGang1;
extern uint32_t Population_nNumGang2;
extern uint32_t Population_nTotalPeds;
extern uint32_t Population_nNumGang3;
extern uint32_t Population_nTotalGangPeds;
extern uint32_t Population_nNumGang4;
extern uint32_t Population_nTotalCivPeds;
extern uint32_t Population_nNumGang5;
extern uint32_t Population_nNumDummy;
extern uint32_t Population_nNumGang6;
extern uint32_t Population_nNumGang9;
extern uint32_t Population_nNumGang7;
extern uint32_t Population_nNumGang8;
extern uint32_t Population_nTotalCarPassengerPeds;
extern bool Population_givePedsWeapons;
extern int32_t Population_allRandomPedsType;
extern float Population_pedDensityMultiplier;
extern bool Population_zoneChangeHasHappened;
extern int8_t Population_countDownToPedsAtStart;
extern uint32_t Population_numMiamiViceCops;
void Population_Initialise(void);
void Population_Update(bool addPeds);
void Population_GeneratePedsAtStartOfGame(void);
void PopulationHost_Manage(void);
float Population_PedCreationDistMultiplier(void);
bool PopulationHost_PlayerVehicleSpeed(float *speed);
float PopulationHost_CameraGenerationMultiplier(void);
void PopulationHost_AddToPopulation(float minDist, float maxDist, float minOffscreen, float maxOffscreen);
void PopulationCounts_InitialiseLog(bool starting);
void Population_ResetCounts(void);
void Population_RecalculateTotals(void);
void Population_UpdatePedCount(ePedType pedType, bool decrease);
void PopulationCounts_UnknownPedType(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
