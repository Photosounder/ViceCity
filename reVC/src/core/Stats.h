//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "PedType.h"
#include "audio_enums.h"
#include "MusicSavedData.h"
enum {
Stats_TOTAL_FASTEST_TIMES = 23,
Stats_TOTAL_HIGHEST_SCORES = 5,
Stats_TOTAL_BEST_POSITIONS = 1,
Stats_TOTAL_PROPERTIES = 15
};
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t Stats_SeagullsKilled;
extern int32_t Stats_DaysPassed;
extern int32_t Stats_HeadsPopped;
extern int32_t Stats_CommercialPassed;
extern int32_t Stats_IndustrialPassed;
extern int32_t Stats_SuburbanPassed;
extern int32_t Stats_NumberKillFrenziesPassed;
extern int32_t Stats_PeopleKilledByOthers;
extern int32_t Stats_HelisDestroyed;
extern int32_t Stats_PedsKilledOfThisType[NUM_PEDTYPES];
extern int32_t Stats_TimesDied;
extern int32_t Stats_TimesArrested;
extern int32_t Stats_KillsSinceLastCheckpoint;
extern float Stats_DistanceTravelledByCar;
extern float Stats_DistanceTravelledByHelicoptor;
extern float Stats_DistanceTravelledByBike;
extern float Stats_DistanceTravelledByBoat;
extern float Stats_DistanceTravelledByPlane;
extern float Stats_DistanceTravelledByGolfCart;
extern float Stats_DistanceTravelledOnFoot;
extern int32_t Stats_FlightTime;
extern int32_t Stats_TimesDrowned;
extern int32_t Stats_PhotosTaken;
extern float Stats_LoanSharks;
extern float Stats_StoresKnockedOff;
extern float Stats_MovieStunts;
extern float Stats_Assassinations;
extern float Stats_PizzasDelivered;
extern float Stats_GarbagePickups;
extern float Stats_IceCreamSold;
extern float Stats_TopShootingRangeScore;
extern float Stats_ShootingRank;
extern int32_t Stats_CarsExploded;
extern int32_t Stats_BoatsExploded;
extern int32_t Stats_WantedStarsAttained;
extern int32_t Stats_WantedStarsEvaded;
extern int32_t Stats_PeopleKilledByPlayer;
extern float Stats_ProgressMade;
extern float Stats_TotalProgressInGame;
extern float Stats_MaximumJumpDistance;
extern float Stats_MaximumJumpHeight;
extern int32_t Stats_MaximumJumpFlips;
extern int32_t Stats_MaximumJumpSpins;
extern int32_t Stats_BestStuntJump;
extern int32_t Stats_NumberOfUniqueJumpsFound;
extern int32_t Stats_TotalNumberOfUniqueJumps;
extern int32_t Stats_PassengersDroppedOffWithTaxi;
extern int32_t Stats_MoneyMadeWithTaxi;
extern int32_t Stats_MissionsGiven;
extern int32_t Stats_MissionsPassed;
extern char Stats_LastMissionPassedName[8];
extern int32_t Stats_TotalLegitimateKills;
extern int32_t Stats_LivesSavedWithAmbulance;
extern int32_t Stats_CriminalsCaught;
extern int32_t Stats_HighestLevelAmbulanceMission;
extern int32_t Stats_HighestLevelVigilanteMission;
extern int32_t Stats_HighestLevelFireMission;
extern int32_t Stats_FiresExtinguished;
extern int32_t Stats_TotalNumberKillFrenzies;
extern int32_t Stats_TotalNumberMissions;
extern int32_t Stats_RoundsFiredByPlayer;
extern int32_t Stats_KgsOfExplosivesUsed;
extern int32_t Stats_BulletsThatHit;
extern int32_t Stats_BestTimeBombDefusal;
extern int32_t Stats_FastestTimes[Stats_TOTAL_FASTEST_TIMES];
extern int32_t Stats_HighestScores[Stats_TOTAL_HIGHEST_SCORES];
extern int32_t Stats_BestPositions[Stats_TOTAL_BEST_POSITIONS];
extern bool Stats_PropertyOwned[Stats_TOTAL_PROPERTIES];
extern int32_t Stats_NumPropertyOwned;
extern int32_t Stats_PropertyDestroyed;
extern float Stats_HighestChaseValue;
extern int32_t Stats_CheatedCount;
extern int32_t Stats_ShowChaseStatOnScreen;
extern int32_t Stats_PamphletMissionPassed;
extern bool Stats_abSonyCDs[1];
extern int32_t Stats_BloodRingKills;
extern int32_t Stats_BloodRingTime;
extern int32_t Stats_Sprayings;
extern float Stats_AutoPaintingBudget;
extern int32_t Stats_NoMoreHurricanes;
extern float Stats_FashionBudget;
extern float Stats_PropertyBudget;
extern float Stats_WeaponBudget;
extern int32_t Stats_SafeHouseVisits;
extern int32_t Stats_TyresPopped;
extern int32_t Stats_LongestWheelie;
extern int32_t Stats_LongestStoppie;
extern int32_t Stats_Longest2Wheel;
extern float Stats_LongestWheelieDist;
extern float Stats_LongestStoppieDist;
extern float Stats_Longest2WheelDist;
void Stats_Init(void);
void Stats_RegisterFastestTime(int32_t index, int32_t time);
void Stats_RegisterHighestScore(int32_t index, int32_t score);
void Stats_RegisterBestPosition(int32_t index, int32_t position);
void Stats_AnotherLifeSavedWithAmbulance(void);
void Stats_AnotherCriminalCaught(void);
void Stats_RegisterLevelAmbulanceMission(int32_t level);
void Stats_RegisterLevelVigilanteMission(int32_t level);
void Stats_RegisterLevelFireMission(int32_t level);
void Stats_AnotherFireExtinguished(void);
void Stats_AnotherKillFrenzyPassed(void);
void Stats_SetTotalNumberKillFrenzies(int32_t total);
void Stats_SetTotalNumberMissions(int32_t total);
uint16_t * Stats_FindCriminalRatingString(void);
uint16_t * Stats_FindChaseString(float fMediaLevel);
int32_t Stats_FindCriminalRatingNumber(void);
float Stats_GetPercentageProgress(void);
void Stats_MoneySpentOnWeapons(int32_t money);
void Stats_MoneySpentOnProperty(int32_t money);
void Stats_MoneySpentOnAutoPainting(int32_t money);
void Stats_MoneySpentOnFashion(int32_t money);
void Stats_NumOfVisitsFromLoanSharks(int32_t num);
void Stats_NumOfStoresKnockedOff(int32_t num);
void Stats_NumOfMovieStunts(int32_t num);
void Stats_NumOfAssassinations(int32_t num);
void Stats_NumOfPizzasDelivered(int32_t num);
void Stats_NumOfGarbagePickups(int32_t num);
void Stats_NumOfIceCreamSold(int32_t num);
void Stats_AddNumBloodRingKills(int32_t num);
void Stats_LongestTimeInBloodRing(int32_t time);
void Stats_AddPropertyAsOwned(int32_t id);
void Stats_SaveStats(uint8_t *buf, uint32_t *size);
void Stats_LoadStats(uint8_t *buf, uint32_t size);
void Stats_PopulateFavoriteRadioStationList(void);
void Stats_BuildStatLine(const char *text, void *stat, int displayType, void *stat2, int isTime);
int Stats_ConstructStatLine(int rowIdx);
void Stats_CheckPointReachedSuccessfully(void);
void Stats_CheckPointReachedUnsuccessfully(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
