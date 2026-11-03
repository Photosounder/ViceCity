//+ rouz edit (ChatGPT)
#include "InputState.h"
#include "Stats_backend.h"
#include "SharedText.h"
#include "FontText.h"
#include "Text.h"
#include "TextLanguages.h"
#include "DMAudio.h"
#include "Timer.h"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#define ARRAY_SIZE(value) (sizeof(value)/sizeof((value)[0]))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#ifdef MASTER
#define assert(expression) (expression)
#else
#define assert(expression) (void)((!!(expression))||(Stats_BackendAssertFailed(#expression,__FILE__,__LINE__,__func__),0))
#endif
#ifdef USE_PRECISE_MEASUREMENT_CONVERTION
#define MILES_IN_METER 0.000621371192f
#define FEET_IN_METER 3.28084f
#else
#define MILES_IN_METER (1 / 1670.f)
#define FEET_IN_METER 3.33f
#endif

int32_t Stats_SeagullsKilled;
int32_t Stats_DaysPassed;
int32_t Stats_HeadsPopped;
int32_t Stats_CommercialPassed;
int32_t Stats_IndustrialPassed;
int32_t Stats_SuburbanPassed;
int32_t Stats_NumberKillFrenziesPassed;
int32_t Stats_PeopleKilledByOthers;
int32_t Stats_HelisDestroyed;
int32_t Stats_PedsKilledOfThisType[NUM_PEDTYPES];
int32_t Stats_TimesDied;
int32_t Stats_TimesArrested;
int32_t Stats_KillsSinceLastCheckpoint;
float Stats_DistanceTravelledByCar;
float Stats_DistanceTravelledByHelicoptor;
float Stats_DistanceTravelledByBike;
float Stats_DistanceTravelledByBoat;
float Stats_DistanceTravelledByPlane;
float Stats_DistanceTravelledByGolfCart;
float Stats_DistanceTravelledOnFoot;
int32_t Stats_FlightTime;
int32_t Stats_TimesDrowned;
int32_t Stats_PhotosTaken;
float Stats_LoanSharks;
float Stats_StoresKnockedOff;
float Stats_MovieStunts;
float Stats_Assassinations;
float Stats_PizzasDelivered;
float Stats_GarbagePickups;
float Stats_IceCreamSold;
float Stats_TopShootingRangeScore;
float Stats_ShootingRank;
int32_t Stats_CarsExploded;
int32_t Stats_BoatsExploded;
int32_t Stats_WantedStarsAttained;
int32_t Stats_WantedStarsEvaded;
int32_t Stats_PeopleKilledByPlayer;
float Stats_ProgressMade;
float Stats_TotalProgressInGame;
float Stats_MaximumJumpDistance;
float Stats_MaximumJumpHeight;
int32_t Stats_MaximumJumpFlips;
int32_t Stats_MaximumJumpSpins;
int32_t Stats_BestStuntJump;
int32_t Stats_NumberOfUniqueJumpsFound;
int32_t Stats_TotalNumberOfUniqueJumps;
int32_t Stats_PassengersDroppedOffWithTaxi;
int32_t Stats_MoneyMadeWithTaxi;
int32_t Stats_MissionsGiven;
int32_t Stats_MissionsPassed;
char Stats_LastMissionPassedName[8];
int32_t Stats_TotalLegitimateKills;
int32_t Stats_LivesSavedWithAmbulance;
int32_t Stats_CriminalsCaught;
int32_t Stats_HighestLevelAmbulanceMission;
int32_t Stats_HighestLevelVigilanteMission;
int32_t Stats_HighestLevelFireMission;
int32_t Stats_FiresExtinguished;
int32_t Stats_TotalNumberKillFrenzies;
int32_t Stats_TotalNumberMissions;
int32_t Stats_RoundsFiredByPlayer;
int32_t Stats_KgsOfExplosivesUsed;
int32_t Stats_BulletsThatHit;
int32_t Stats_BestTimeBombDefusal;
int32_t Stats_FastestTimes[Stats_TOTAL_FASTEST_TIMES];
int32_t Stats_HighestScores[Stats_TOTAL_HIGHEST_SCORES];
int32_t Stats_BestPositions[Stats_TOTAL_BEST_POSITIONS];
bool Stats_PropertyOwned[Stats_TOTAL_PROPERTIES];
int32_t Stats_NumPropertyOwned;
int32_t Stats_PropertyDestroyed;
float Stats_HighestChaseValue;
int32_t Stats_CheatedCount;
int32_t Stats_ShowChaseStatOnScreen;
int32_t Stats_PamphletMissionPassed;
bool Stats_abSonyCDs[1];
int32_t Stats_BloodRingKills;
int32_t Stats_BloodRingTime;
int32_t Stats_Sprayings;
float Stats_AutoPaintingBudget;
int32_t Stats_NoMoreHurricanes;
float Stats_FashionBudget;
float Stats_PropertyBudget;
float Stats_WeaponBudget;
int32_t Stats_SafeHouseVisits;
int32_t Stats_TyresPopped;
int32_t Stats_LongestWheelie;
int32_t Stats_LongestStoppie;
int32_t Stats_Longest2Wheel;
float Stats_LongestWheelieDist;
float Stats_LongestStoppieDist;
float Stats_Longest2WheelDist;

void Stats_Init(void)
{
    // Preserve the complete original Init operation and every ordered external access
	Stats_PeopleKilledByOthers = 0;
	Stats_PeopleKilledByPlayer = 0;
	Stats_CarsExploded = 0;
	Stats_BoatsExploded = 0;
	Stats_RoundsFiredByPlayer = 0;
	for (int i = 0; i < NUM_PEDTYPES; i++)
		Stats_PedsKilledOfThisType[i] = 0;
	Stats_HelisDestroyed = 0;
	Stats_ProgressMade = 0.0f;
	Stats_KgsOfExplosivesUsed = 0;
	Stats_BulletsThatHit = 0;
	Stats_TyresPopped = 0;
	Stats_HeadsPopped = 0;
	Stats_WantedStarsAttained = 0;
	Stats_WantedStarsEvaded = 0;
	Stats_TimesArrested = 0;
	Stats_TimesDied = 0;
	Stats_DaysPassed = 0;
	Stats_SafeHouseVisits = 0;
	Stats_Sprayings = 0;
	Stats_MaximumJumpDistance = 0;
	Stats_MaximumJumpHeight = 0;
	Stats_MaximumJumpFlips = 0;
	Stats_MaximumJumpSpins = 0;
	Stats_BestStuntJump = 0;
	Stats_NumberOfUniqueJumpsFound = 0;
	Stats_TotalNumberOfUniqueJumps = 0;
	Stats_MissionsGiven = 0;
	Stats_MissionsPassed = 0;
	Stats_PassengersDroppedOffWithTaxi = 0;
	Stats_MoneyMadeWithTaxi = 0;
	Stats_DistanceTravelledOnFoot = 0;
	Stats_DistanceTravelledByCar = 0;
	Stats_DistanceTravelledByBike = 0;
	Stats_DistanceTravelledByBoat = 0;
	Stats_DistanceTravelledByGolfCart = 0;
	Stats_DistanceTravelledByHelicoptor = 0;
#ifdef FIX_BUGS
	Stats_DistanceTravelledByPlane = 0;
#endif
	Stats_LivesSavedWithAmbulance = 0;
	Stats_CriminalsCaught = 0;
	Stats_HighestLevelVigilanteMission = 0;
	Stats_HighestLevelAmbulanceMission = 0;
	Stats_HighestLevelFireMission = 0;
	Stats_FiresExtinguished = 0;
	Stats_PhotosTaken = 0;
	Stats_NumberKillFrenziesPassed = 0;
	Stats_TotalNumberKillFrenzies = 0;
	Stats_TotalNumberMissions = 0;
	Stats_FlightTime = 0;
	Stats_TimesDrowned = 0;
	Stats_SeagullsKilled = 0;
	Stats_WeaponBudget = 0.0f;
	Stats_FashionBudget = 0.0f;
	Stats_LoanSharks = 0.0f;
	Stats_StoresKnockedOff = 0.0f;
	Stats_MovieStunts = 0.0f;
	Stats_Assassinations = 0.0f;
	Stats_PizzasDelivered = 0.0f;
	Stats_GarbagePickups = 0.0f;
	Stats_IceCreamSold = 0.0f;
	Stats_TopShootingRangeScore = 0.0f;
	Stats_ShootingRank = 0.0f;
	Stats_LongestWheelie = 0;
	Stats_LongestStoppie = 0;
	Stats_Longest2Wheel = 0;
	Stats_LongestWheelieDist = 0.0f;
	Stats_LongestStoppieDist = 0.0f;
	Stats_Longest2WheelDist = 0.0f;
	Stats_PropertyBudget = 0.0f;
	Stats_AutoPaintingBudget = 0.0f;
	Stats_PropertyDestroyed = 0;
	Stats_HighestChaseValue = 0.0f;
	Stats_CheatedCount = 0;

	for (int i = 0; i < Stats_TOTAL_FASTEST_TIMES; i++)
		Stats_FastestTimes[i] = 0;
	for (int i = 0; i < Stats_TOTAL_HIGHEST_SCORES; i++)
		Stats_HighestScores[i] = 0;
	for (int i = 0; i < Stats_TOTAL_BEST_POSITIONS; i++)
		Stats_BestPositions[i] = INT_MAX;

	Stats_KillsSinceLastCheckpoint = 0;
	Stats_TotalLegitimateKills = 0;

	for (int i = 0; i < ARRAY_SIZE(Stats_LastMissionPassedName); i++)
		Stats_LastMissionPassedName[i] = 0;

	Stats_IndustrialPassed = 0;
	Stats_CommercialPassed = 0;
	Stats_SuburbanPassed = 0;
	Stats_PamphletMissionPassed = 0;
	Stats_NoMoreHurricanes = 0;
	Stats_ShowChaseStatOnScreen = 0;
	for (int i = 0; i < ARRAY_SIZE(Stats_abSonyCDs); i++)
		Stats_abSonyCDs[i] = 0;
	Stats_PopulateFavoriteRadioStationList();

	Stats_NumPropertyOwned = 0;
	for (int i = 0; i < Stats_TOTAL_PROPERTIES; i++)
		Stats_PropertyOwned[i] = false;

	Stats_BloodRingKills = 0;
	Stats_BloodRingTime = 0;
}

void Stats_RegisterFastestTime(int32_t index, int32_t time)
{
    // Preserve the complete original RegisterFastestTime operation and every ordered external access
	assert(index >= 0 && index < Stats_TOTAL_FASTEST_TIMES);
	if (Stats_FastestTimes[index] == 0)
		Stats_FastestTimes[index] = time;
	else
		Stats_FastestTimes[index] = Min(Stats_FastestTimes[index], time);
}

void Stats_RegisterHighestScore(int32_t index, int32_t score)
{
    // Preserve the complete original RegisterHighestScore operation and every ordered external access
	assert(index >= 0 && index < Stats_TOTAL_HIGHEST_SCORES);
	Stats_HighestScores[index] = Max(Stats_HighestScores[index], score);
}

void Stats_RegisterBestPosition(int32_t index, int32_t position)
{
    // Preserve the complete original RegisterBestPosition operation and every ordered external access
	assert(index >= 0 && index < Stats_TOTAL_BEST_POSITIONS);
	Stats_BestPositions[index] = Min(Stats_BestPositions[index], position);
}

void Stats_AnotherLifeSavedWithAmbulance(void)
{
    // Preserve the complete original AnotherLifeSavedWithAmbulance operation and every ordered external access
	++Stats_LivesSavedWithAmbulance;
}

void Stats_AnotherCriminalCaught(void)
{
    // Preserve the complete original AnotherCriminalCaught operation and every ordered external access
	++Stats_CriminalsCaught;
}

void Stats_RegisterLevelAmbulanceMission(int32_t level)
{
    // Preserve the complete original RegisterLevelAmbulanceMission operation and every ordered external access
	Stats_HighestLevelAmbulanceMission = Max(Stats_HighestLevelAmbulanceMission, level);
}

void Stats_RegisterLevelVigilanteMission(int32_t level)
{
    // Preserve the complete original RegisterLevelVigilanteMission operation and every ordered external access
	Stats_HighestLevelVigilanteMission = Max(Stats_HighestLevelVigilanteMission, level);
}

void Stats_RegisterLevelFireMission(int32_t level)
{
    // Preserve the complete original RegisterLevelFireMission operation and every ordered external access
	Stats_HighestLevelFireMission = Max(Stats_HighestLevelFireMission, level);
}

void Stats_AnotherFireExtinguished(void)
{
    // Preserve the complete original AnotherFireExtinguished operation and every ordered external access
	++Stats_FiresExtinguished;
}

void Stats_AnotherKillFrenzyPassed(void)
{
    // Preserve the complete original AnotherKillFrenzyPassed operation and every ordered external access
	++Stats_NumberKillFrenziesPassed;
}

void Stats_SetTotalNumberKillFrenzies(int32_t total)
{
    // Preserve the complete original SetTotalNumberKillFrenzies operation and every ordered external access
	Stats_TotalNumberKillFrenzies = total;
}

void Stats_SetTotalNumberMissions(int32_t total)
{
    // Preserve the complete original SetTotalNumberMissions operation and every ordered external access
	Stats_TotalNumberMissions = total;
}

uint16_t * Stats_FindCriminalRatingString(void)
{
    // Preserve the complete original FindCriminalRatingString operation and every ordered external access
	int rating = Stats_FindCriminalRatingNumber();

	if (rating < 0) {
		if (rating > -500) return Text_Get(&TheText,"RATNG53");
		if (rating > -2000) return Text_Get(&TheText,"RATNG54");
		if (rating > -4000) return Text_Get(&TheText,"RATNG55");
		if (rating > -6000) return Text_Get(&TheText,"RATNG56");
		return Text_Get(&TheText,"RATNG57");
	}
	if (rating < 20) return Text_Get(&TheText,"RATNG1");
	if (rating < 50) return Text_Get(&TheText,"RATNG2");
	if (rating < 75) return Text_Get(&TheText,"RATNG3");
	if (rating < 100) return Text_Get(&TheText,"RATNG4");
	if (rating < 120) return Text_Get(&TheText,"RATNG5");
	if (rating < 150) return Text_Get(&TheText,"RATNG6");
	if (rating < 200) return Text_Get(&TheText,"RATNG7");
	if (rating < 240) return Text_Get(&TheText,"RATNG8");
	if (rating < 270) return Text_Get(&TheText,"RATNG9");
	if (rating < 300) return Text_Get(&TheText,"RATNG10");
	if (rating < 335) return Text_Get(&TheText,"RATNG11");
	if (rating < 370) return Text_Get(&TheText,"RATNG12");
	if (rating < 400) return Text_Get(&TheText,"RATNG13");
	if (rating < 450) return Text_Get(&TheText,"RATNG14");
	if (rating < 500) return Text_Get(&TheText,"RATNG15");
	if (rating < 550) return Text_Get(&TheText,"RATNG16");
	if (rating < 600) return Text_Get(&TheText,"RATNG17");
	if (rating < 610) return Text_Get(&TheText,"RATNG18");
	if (rating < 650) return Text_Get(&TheText,"RATNG19");
	if (rating < 700) return Text_Get(&TheText,"RATNG20");
	if (rating < 850) return Text_Get(&TheText,"RATNG21");
	if (rating < 1000) return Text_Get(&TheText,"RATNG22");
	if (rating < 1005) return Text_Get(&TheText,"RATNG23");
	if (rating < 1150) return Text_Get(&TheText,"RATNG24");
	if (rating < 1300) return Text_Get(&TheText,Stats_TimesArrested > 0 ? "RATNG25" : "RATNG24");
	if (rating < 1500) return Text_Get(&TheText,"RATNG26");
	if (rating < 1700) return Text_Get(&TheText,"RATNG27");
	if (rating < 2000) return Text_Get(&TheText,"RATNG28");
	if (rating < 2100) return Text_Get(&TheText,"RATNG29");
	if (rating < 2300) return Text_Get(&TheText,"RATNG30");
	if (rating < 2500) return Text_Get(&TheText,"RATNG31");
	if (rating < 2750) return Text_Get(&TheText,"RATNG32");
	if (rating < 3000) return Text_Get(&TheText,"RATNG33");
	if (rating < 3500) return Text_Get(&TheText,"RATNG34");
	if (rating < 4000) return Text_Get(&TheText,"RATNG35");
	if (rating < 5000) return Text_Get(&TheText,"RATNG36");
	if (rating < 7500) return Text_Get(&TheText,"RATNG37");
	if (rating < 10000) return Text_Get(&TheText,"RATNG38");
	if (rating < 20000) return Text_Get(&TheText,"RATNG39");
	if (rating < 30000) return Text_Get(&TheText,"RATNG40");
	if (rating < 40000) return Text_Get(&TheText,"RATNG41");
	if (rating < 50000) return Text_Get(&TheText,"RATNG42");
	if (rating < 65000) return Text_Get(&TheText,"RATNG43");
	if (rating < 80000) return Text_Get(&TheText,"RATNG44");
	if (rating < 100000) return Text_Get(&TheText,"RATNG45");
	if (rating < 150000) return Text_Get(&TheText,"RATNG46");
	if (rating < 200000) return Text_Get(&TheText,"RATNG47");
	if (rating < 300000) return Text_Get(&TheText,"RATNG48");
	if (rating < 375000) return Text_Get(&TheText,"RATNG49");
	if (rating < 500000) return Text_Get(&TheText,Stats_FlightTime / 60000 / 60 > 10 ? "RATNG50" : "RATNG49");
	if (rating < 1000000) return Text_Get(&TheText,"RATNG51");
	return Text_Get(&TheText,Stats_BackendVisibleMoney() > 10000000 ? "RATNG52" : "RATNG51");
}

uint16_t * Stats_FindChaseString(float fMediaLevel)
{
    // Preserve the complete original FindChaseString operation and every ordered external access
	if (fMediaLevel < 20.0f) return Text_Get(&TheText,"CHASE1");
	if (fMediaLevel < 50.0f) return Text_Get(&TheText,"CHASE2");
	if (fMediaLevel < 75.0f) return Text_Get(&TheText,"CHASE3");
	if (fMediaLevel < 100.0f) return Text_Get(&TheText,"CHASE4");
	if (fMediaLevel < 150.0f) return Text_Get(&TheText,"CHASE5");
	if (fMediaLevel < 200.0f) return Text_Get(&TheText,"CHASE6");
	if (fMediaLevel < 250.0f) return Text_Get(&TheText,"CHASE7");
	if (fMediaLevel < 300.0f) return Text_Get(&TheText,"CHASE8");
	if (fMediaLevel < 350.0f) return Text_Get(&TheText,"CHASE9");
	if (fMediaLevel < 400.0f) return Text_Get(&TheText,"CHASE10");
	if (fMediaLevel < 500.0f) return Text_Get(&TheText,"CHASE11");
	if (fMediaLevel < 600.0f) return Text_Get(&TheText,"CHASE12");
	if (fMediaLevel < 700.0f) return Text_Get(&TheText,"CHASE13");
	if (fMediaLevel < 800.0f) return Text_Get(&TheText,"CHASE14");
	if (fMediaLevel < 900.0f) return Text_Get(&TheText,"CHASE15");
	if (fMediaLevel < 1000.0f) return Text_Get(&TheText,"CHASE16");
	if (fMediaLevel < 1200.0f) return Text_Get(&TheText,"CHASE17");
	if (fMediaLevel < 1400.0f) return Text_Get(&TheText,"CHASE18");
	if (fMediaLevel < 1600.0f) return Text_Get(&TheText,"CHASE19");
	if (fMediaLevel < 1800.0f) return Text_Get(&TheText,"CHASE20");
	return Text_Get(&TheText,"CHASE21");
}

int32_t Stats_FindCriminalRatingNumber(void)
{
    // Preserve the complete original FindCriminalRatingNumber operation and every ordered external access
	int32_t rating;

	rating = Stats_FiresExtinguished + 10 * Stats_HighestLevelFireMission + 10 * Stats_HighestLevelAmbulanceMission
		+ Stats_CriminalsCaught + Stats_LivesSavedWithAmbulance
		+ 30 * Stats_HelisDestroyed + Stats_TotalLegitimateKills - 3 * Stats_TimesArrested - 3 * Stats_TimesDied
		+ Stats_BackendMoney() / 5000;
	if (Input_playerHasCheated || Stats_CheatedCount > 0) {
		rating -= Stats_CheatedCount;
		if (rating <= -10000)
			rating = -10000;

	} else if (rating <= 0) {
		rating = 0;
	}

	if (Stats_RoundsFiredByPlayer > 100)
		rating += (float)Stats_BulletsThatHit / (float)Stats_RoundsFiredByPlayer * 500.0f;
	if (Stats_TotalProgressInGame)
		rating += Stats_ProgressMade / Stats_TotalProgressInGame * 1000.0f;
	return rating;
}

float Stats_GetPercentageProgress(void)
{
    // Preserve the complete original GetPercentageProgress operation and every ordered external access
	float percentCompleted = (Stats_TotalProgressInGame == 0.f ? 0.f :
		100.0f * Stats_ProgressMade / (Stats_BackendNastyGame() ? Stats_TotalProgressInGame : Stats_TotalProgressInGame - 1.0f));

	return Min(percentCompleted, 100.0f);
}

void Stats_MoneySpentOnWeapons(int32_t money)
{
    // Preserve the complete original MoneySpentOnWeapons operation and every ordered external access
	Stats_WeaponBudget += money;
}

void Stats_MoneySpentOnProperty(int32_t money)
{
    // Preserve the complete original MoneySpentOnProperty operation and every ordered external access
	Stats_PropertyBudget += money;
}

void Stats_MoneySpentOnAutoPainting(int32_t money)
{
    // Preserve the complete original MoneySpentOnAutoPainting operation and every ordered external access
	Stats_AutoPaintingBudget += money;
}

void Stats_MoneySpentOnFashion(int32_t money)
{
    // Preserve the complete original MoneySpentOnFashion operation and every ordered external access
	Stats_FashionBudget += money;
}

void Stats_NumOfVisitsFromLoanSharks(int32_t num)
{
    // Preserve the complete original NumOfVisitsFromLoanSharks operation and every ordered external access
	Stats_LoanSharks += num;
}

void Stats_NumOfStoresKnockedOff(int32_t num)
{
    // Preserve the complete original NumOfStoresKnockedOff operation and every ordered external access
	Stats_StoresKnockedOff += num;
}

void Stats_NumOfMovieStunts(int32_t num)
{
    // Preserve the complete original NumOfMovieStunts operation and every ordered external access
	Stats_MovieStunts += num;
}

void Stats_NumOfAssassinations(int32_t num)
{
    // Preserve the complete original NumOfAssassinations operation and every ordered external access
	Stats_Assassinations += num;
}

void Stats_NumOfPizzasDelivered(int32_t num)
{
    // Preserve the complete original NumOfPizzasDelivered operation and every ordered external access
	Stats_PizzasDelivered += num;
}

void Stats_NumOfGarbagePickups(int32_t num)
{
    // Preserve the complete original NumOfGarbagePickups operation and every ordered external access
	Stats_GarbagePickups += num;
}

void Stats_NumOfIceCreamSold(int32_t num)
{
    // Preserve the complete original NumOfIceCreamSold operation and every ordered external access
	Stats_IceCreamSold += num;
}

void Stats_AddNumBloodRingKills(int32_t num)
{
    // Preserve the complete original AddNumBloodRingKills operation and every ordered external access
	Stats_BloodRingKills += num;
}

void Stats_LongestTimeInBloodRing(int32_t time)
{
    // Preserve the complete original LongestTimeInBloodRing operation and every ordered external access
	if (Stats_BloodRingTime < time)
		Stats_BloodRingTime = time;
}

void Stats_AddPropertyAsOwned(int32_t id)
{
    // Preserve the complete original AddPropertyAsOwned operation and every ordered external access
	if (!Stats_PropertyOwned[id]) {
		Stats_PropertyOwned[id] = true;
		++Stats_NumPropertyOwned;
	}
}

void Stats_SaveStats(uint8_t *buf, uint32_t *size)
{
    // Preserve the complete original SaveStats operation and every ordered external access
	Stats_CheckPointReachedSuccessfully();
	uint8_t *buf_start = buf;
	*size = sizeof(Stats_PeopleKilledByPlayer) +
		sizeof(Stats_PeopleKilledByOthers) +
		sizeof(Stats_CarsExploded) +
		sizeof(Stats_BoatsExploded) +
		sizeof(Stats_TyresPopped) +
		sizeof(Stats_RoundsFiredByPlayer) +
		sizeof(Stats_PedsKilledOfThisType) +
		sizeof(Stats_HelisDestroyed) +
		sizeof(Stats_ProgressMade) +
		sizeof(Stats_TotalProgressInGame) +
		sizeof(Stats_KgsOfExplosivesUsed) +
		sizeof(Stats_BulletsThatHit) +
		sizeof(Stats_HeadsPopped) +
		sizeof(Stats_WantedStarsAttained) +
		sizeof(Stats_WantedStarsEvaded) +
		sizeof(Stats_TimesArrested) +
		sizeof(Stats_TimesDied) +
		sizeof(Stats_DaysPassed) +
		sizeof(Stats_SafeHouseVisits) +
		sizeof(Stats_Sprayings) +
		sizeof(Stats_MaximumJumpDistance) +
		sizeof(Stats_MaximumJumpHeight) +
		sizeof(Stats_MaximumJumpFlips) +
		sizeof(Stats_MaximumJumpSpins) +
		sizeof(Stats_BestStuntJump) +
		sizeof(Stats_NumberOfUniqueJumpsFound) +
		sizeof(Stats_TotalNumberOfUniqueJumps) +
		sizeof(Stats_MissionsGiven) +
		sizeof(Stats_PassengersDroppedOffWithTaxi) +
		sizeof(Stats_MoneyMadeWithTaxi) +
		sizeof(Stats_IndustrialPassed) +
		sizeof(Stats_CommercialPassed) +
		sizeof(Stats_SuburbanPassed) +
		sizeof(Stats_PamphletMissionPassed) +
		sizeof(Stats_NoMoreHurricanes) +
		sizeof(Stats_DistanceTravelledOnFoot) +
		sizeof(Stats_DistanceTravelledByCar) +
		sizeof(Stats_DistanceTravelledByBike) +
		sizeof(Stats_DistanceTravelledByBoat) +
		sizeof(Stats_DistanceTravelledByGolfCart) +
		sizeof(Stats_DistanceTravelledByHelicoptor) +
		sizeof(Stats_DistanceTravelledByPlane) +
		sizeof(Stats_LivesSavedWithAmbulance) +
		sizeof(Stats_CriminalsCaught) +
		sizeof(Stats_FiresExtinguished) +
		sizeof(Stats_HighestLevelVigilanteMission) +
		sizeof(Stats_HighestLevelAmbulanceMission) +
		sizeof(Stats_HighestLevelFireMission) +
		sizeof(Stats_PhotosTaken) +
		sizeof(Stats_NumberKillFrenziesPassed) +
		sizeof(Stats_TotalNumberKillFrenzies) +
		sizeof(Stats_TotalNumberMissions) +
		sizeof(Stats_FlightTime) +
		sizeof(Stats_TimesDrowned) +
		sizeof(Stats_SeagullsKilled) +
		sizeof(Stats_WeaponBudget) +
		sizeof(Stats_FashionBudget) +
		sizeof(Stats_LoanSharks) +
		sizeof(Stats_StoresKnockedOff) +
		sizeof(Stats_MovieStunts) +
		sizeof(Stats_Assassinations) +
		sizeof(Stats_PizzasDelivered) +
		sizeof(Stats_GarbagePickups) +
		sizeof(Stats_IceCreamSold) +
		sizeof(Stats_TopShootingRangeScore) +
		sizeof(Stats_ShootingRank) +
		sizeof(Stats_LongestWheelie) +
		sizeof(Stats_LongestStoppie) +
		sizeof(Stats_Longest2Wheel) +
		sizeof(Stats_LongestWheelieDist) +
		sizeof(Stats_LongestStoppieDist) +
		sizeof(Stats_Longest2WheelDist) +
		sizeof(Stats_PropertyBudget) +
		sizeof(Stats_AutoPaintingBudget) +
		sizeof(Stats_PropertyDestroyed) +
		sizeof(Stats_NumPropertyOwned) +
		sizeof(Stats_BloodRingKills) +
		sizeof(Stats_BloodRingTime) +
		sizeof(Stats_PropertyOwned) +
		sizeof(Stats_HighestChaseValue) +
		sizeof(Stats_FastestTimes) +
		sizeof(Stats_HighestScores) +
		sizeof(Stats_BestPositions) +
		sizeof(Stats_KillsSinceLastCheckpoint) +
		sizeof(Stats_TotalLegitimateKills) +
		sizeof(Stats_LastMissionPassedName) +
		sizeof(Stats_CheatedCount) +
		sizeof(MusicSavedData_FavoriteRadioStationList);

#define CopyToBuf(buf, data) memcpy(buf, &data, sizeof(data)); buf += sizeof(data);
	CopyToBuf(buf, Stats_PeopleKilledByPlayer);
	CopyToBuf(buf, Stats_PeopleKilledByOthers);
	CopyToBuf(buf, Stats_CarsExploded);
	CopyToBuf(buf, Stats_BoatsExploded);
	CopyToBuf(buf, Stats_TyresPopped);
	CopyToBuf(buf, Stats_RoundsFiredByPlayer);
	CopyToBuf(buf, Stats_PedsKilledOfThisType);
	CopyToBuf(buf, Stats_HelisDestroyed);
	CopyToBuf(buf, Stats_ProgressMade);
	CopyToBuf(buf, Stats_TotalProgressInGame);
	CopyToBuf(buf, Stats_KgsOfExplosivesUsed);
	CopyToBuf(buf, Stats_BulletsThatHit);
	CopyToBuf(buf, Stats_HeadsPopped);
	CopyToBuf(buf, Stats_WantedStarsAttained);
	CopyToBuf(buf, Stats_WantedStarsEvaded);
	CopyToBuf(buf, Stats_TimesArrested);
	CopyToBuf(buf, Stats_TimesDied);
	CopyToBuf(buf, Stats_DaysPassed);
	CopyToBuf(buf, Stats_SafeHouseVisits);
	CopyToBuf(buf, Stats_Sprayings);
	CopyToBuf(buf, Stats_MaximumJumpDistance);
	CopyToBuf(buf, Stats_MaximumJumpHeight);
	CopyToBuf(buf, Stats_MaximumJumpFlips);
	CopyToBuf(buf, Stats_MaximumJumpSpins);
	CopyToBuf(buf, Stats_BestStuntJump);
	CopyToBuf(buf, Stats_NumberOfUniqueJumpsFound);
	CopyToBuf(buf, Stats_TotalNumberOfUniqueJumps);
	CopyToBuf(buf, Stats_MissionsGiven);
	CopyToBuf(buf, Stats_PassengersDroppedOffWithTaxi);
	CopyToBuf(buf, Stats_MoneyMadeWithTaxi);
	CopyToBuf(buf, Stats_IndustrialPassed);
	CopyToBuf(buf, Stats_CommercialPassed);
	CopyToBuf(buf, Stats_SuburbanPassed);
	CopyToBuf(buf, Stats_PamphletMissionPassed);
	CopyToBuf(buf, Stats_NoMoreHurricanes);
	CopyToBuf(buf, Stats_DistanceTravelledOnFoot);
	CopyToBuf(buf, Stats_DistanceTravelledByCar);
	CopyToBuf(buf, Stats_DistanceTravelledByBike);
	CopyToBuf(buf, Stats_DistanceTravelledByBoat);
	CopyToBuf(buf, Stats_DistanceTravelledByGolfCart);
	CopyToBuf(buf, Stats_DistanceTravelledByHelicoptor);
	CopyToBuf(buf, Stats_DistanceTravelledByPlane);
	CopyToBuf(buf, Stats_LivesSavedWithAmbulance);
	CopyToBuf(buf, Stats_CriminalsCaught);
	CopyToBuf(buf, Stats_FiresExtinguished);
	CopyToBuf(buf, Stats_HighestLevelVigilanteMission);
	CopyToBuf(buf, Stats_HighestLevelAmbulanceMission);
	CopyToBuf(buf, Stats_HighestLevelFireMission);
	CopyToBuf(buf, Stats_PhotosTaken);
	CopyToBuf(buf, Stats_NumberKillFrenziesPassed);
	CopyToBuf(buf, Stats_TotalNumberKillFrenzies);
	CopyToBuf(buf, Stats_TotalNumberMissions);
	CopyToBuf(buf, Stats_FlightTime);
	CopyToBuf(buf, Stats_TimesDrowned);
	CopyToBuf(buf, Stats_SeagullsKilled);
	CopyToBuf(buf, Stats_WeaponBudget);
	CopyToBuf(buf, Stats_FashionBudget);
	CopyToBuf(buf, Stats_LoanSharks);
	CopyToBuf(buf, Stats_StoresKnockedOff);
	CopyToBuf(buf, Stats_MovieStunts);
	CopyToBuf(buf, Stats_Assassinations);
	CopyToBuf(buf, Stats_PizzasDelivered);
	CopyToBuf(buf, Stats_GarbagePickups);
	CopyToBuf(buf, Stats_IceCreamSold);
	CopyToBuf(buf, Stats_TopShootingRangeScore);
	CopyToBuf(buf, Stats_ShootingRank);
	CopyToBuf(buf, Stats_LongestWheelie);
	CopyToBuf(buf, Stats_LongestStoppie);
	CopyToBuf(buf, Stats_Longest2Wheel);
	CopyToBuf(buf, Stats_LongestWheelieDist);
	CopyToBuf(buf, Stats_LongestStoppieDist);
	CopyToBuf(buf, Stats_Longest2WheelDist);
	CopyToBuf(buf, Stats_PropertyBudget);
	CopyToBuf(buf, Stats_AutoPaintingBudget);
	CopyToBuf(buf, Stats_PropertyDestroyed);
	CopyToBuf(buf, Stats_NumPropertyOwned);
	CopyToBuf(buf, Stats_BloodRingKills);
	CopyToBuf(buf, Stats_BloodRingTime);
	CopyToBuf(buf, Stats_PropertyOwned);
	CopyToBuf(buf, Stats_HighestChaseValue);
	CopyToBuf(buf, Stats_FastestTimes);
	CopyToBuf(buf, Stats_HighestScores);
	CopyToBuf(buf, Stats_BestPositions);
	CopyToBuf(buf, Stats_KillsSinceLastCheckpoint);
	CopyToBuf(buf, Stats_TotalLegitimateKills);
	CopyToBuf(buf, Stats_LastMissionPassedName);
	CopyToBuf(buf, Stats_CheatedCount);
	Stats_PopulateFavoriteRadioStationList();
	CopyToBuf(buf, MusicSavedData_FavoriteRadioStationList);

	assert(buf - buf_start == *size);
#undef CopyToBuf
}

void Stats_LoadStats(uint8_t *buf, uint32_t size)
{
    // Preserve the complete original LoadStats operation and every ordered external access
	uint8_t* buf_start = buf;

#define CopyFromBuf(buf, data) memcpy(&data, buf, sizeof(data)); buf += sizeof(data);

	CopyFromBuf(buf, Stats_PeopleKilledByPlayer);
	CopyFromBuf(buf, Stats_PeopleKilledByOthers);
	CopyFromBuf(buf, Stats_CarsExploded);
	CopyFromBuf(buf, Stats_BoatsExploded);
	CopyFromBuf(buf, Stats_TyresPopped);
	CopyFromBuf(buf, Stats_RoundsFiredByPlayer);
	CopyFromBuf(buf, Stats_PedsKilledOfThisType);
	CopyFromBuf(buf, Stats_HelisDestroyed);
	CopyFromBuf(buf, Stats_ProgressMade);
	CopyFromBuf(buf, Stats_TotalProgressInGame);
	CopyFromBuf(buf, Stats_KgsOfExplosivesUsed);
	CopyFromBuf(buf, Stats_BulletsThatHit);
	CopyFromBuf(buf, Stats_HeadsPopped);
	CopyFromBuf(buf, Stats_WantedStarsAttained);
	CopyFromBuf(buf, Stats_WantedStarsEvaded);
	CopyFromBuf(buf, Stats_TimesArrested);
	CopyFromBuf(buf, Stats_TimesDied);
	CopyFromBuf(buf, Stats_DaysPassed);
	CopyFromBuf(buf, Stats_SafeHouseVisits);
	CopyFromBuf(buf, Stats_Sprayings);
	CopyFromBuf(buf, Stats_MaximumJumpDistance);
	CopyFromBuf(buf, Stats_MaximumJumpHeight);
	CopyFromBuf(buf, Stats_MaximumJumpFlips);
	CopyFromBuf(buf, Stats_MaximumJumpSpins);
	CopyFromBuf(buf, Stats_BestStuntJump);
	CopyFromBuf(buf, Stats_NumberOfUniqueJumpsFound);
	CopyFromBuf(buf, Stats_TotalNumberOfUniqueJumps);
	CopyFromBuf(buf, Stats_MissionsGiven);
	CopyFromBuf(buf, Stats_PassengersDroppedOffWithTaxi);
	CopyFromBuf(buf, Stats_MoneyMadeWithTaxi);
	CopyFromBuf(buf, Stats_IndustrialPassed);
	CopyFromBuf(buf, Stats_CommercialPassed);
	CopyFromBuf(buf, Stats_SuburbanPassed);
	CopyFromBuf(buf, Stats_PamphletMissionPassed);
	CopyFromBuf(buf, Stats_NoMoreHurricanes);
	CopyFromBuf(buf, Stats_DistanceTravelledOnFoot);
	CopyFromBuf(buf, Stats_DistanceTravelledByCar);
	CopyFromBuf(buf, Stats_DistanceTravelledByBike);
	CopyFromBuf(buf, Stats_DistanceTravelledByBoat);
	CopyFromBuf(buf, Stats_DistanceTravelledByGolfCart);
	CopyFromBuf(buf, Stats_DistanceTravelledByHelicoptor);
	CopyFromBuf(buf, Stats_DistanceTravelledByPlane);
	CopyFromBuf(buf, Stats_LivesSavedWithAmbulance);
	CopyFromBuf(buf, Stats_CriminalsCaught);
	CopyFromBuf(buf, Stats_FiresExtinguished);
	CopyFromBuf(buf, Stats_HighestLevelVigilanteMission);
	CopyFromBuf(buf, Stats_HighestLevelAmbulanceMission);
	CopyFromBuf(buf, Stats_HighestLevelFireMission);
	CopyFromBuf(buf, Stats_PhotosTaken);
	CopyFromBuf(buf, Stats_NumberKillFrenziesPassed);
	CopyFromBuf(buf, Stats_TotalNumberKillFrenzies);
	CopyFromBuf(buf, Stats_TotalNumberMissions);
	CopyFromBuf(buf, Stats_FlightTime);
	CopyFromBuf(buf, Stats_TimesDrowned);
	CopyFromBuf(buf, Stats_SeagullsKilled);
	CopyFromBuf(buf, Stats_WeaponBudget);
	CopyFromBuf(buf, Stats_FashionBudget);
	CopyFromBuf(buf, Stats_LoanSharks);
	CopyFromBuf(buf, Stats_StoresKnockedOff);
	CopyFromBuf(buf, Stats_MovieStunts);
	CopyFromBuf(buf, Stats_Assassinations);
	CopyFromBuf(buf, Stats_PizzasDelivered);
	CopyFromBuf(buf, Stats_GarbagePickups);
	CopyFromBuf(buf, Stats_IceCreamSold);
	CopyFromBuf(buf, Stats_TopShootingRangeScore);
	CopyFromBuf(buf, Stats_ShootingRank);
	CopyFromBuf(buf, Stats_LongestWheelie);
	CopyFromBuf(buf, Stats_LongestStoppie);
	CopyFromBuf(buf, Stats_Longest2Wheel);
	CopyFromBuf(buf, Stats_LongestWheelieDist);
	CopyFromBuf(buf, Stats_LongestStoppieDist);
	CopyFromBuf(buf, Stats_Longest2WheelDist);
	CopyFromBuf(buf, Stats_PropertyBudget);
	CopyFromBuf(buf, Stats_AutoPaintingBudget);
	CopyFromBuf(buf, Stats_PropertyDestroyed);
	CopyFromBuf(buf, Stats_NumPropertyOwned);
	CopyFromBuf(buf, Stats_BloodRingKills);
	CopyFromBuf(buf, Stats_BloodRingTime);
	CopyFromBuf(buf, Stats_PropertyOwned);
	CopyFromBuf(buf, Stats_HighestChaseValue);
	CopyFromBuf(buf, Stats_FastestTimes);
	CopyFromBuf(buf, Stats_HighestScores);
	CopyFromBuf(buf, Stats_BestPositions);
	CopyFromBuf(buf, Stats_KillsSinceLastCheckpoint);
	CopyFromBuf(buf, Stats_TotalLegitimateKills);
	CopyFromBuf(buf, Stats_LastMissionPassedName);
	CopyFromBuf(buf, Stats_CheatedCount);
	CopyFromBuf(buf, MusicSavedData_FavoriteRadioStationList);

	assert(buf - buf_start == size);
#undef CopyFromBuf
}

void Stats_PopulateFavoriteRadioStationList(void)
{
    // Preserve the complete original PopulateFavoriteRadioStationList operation and every ordered external access
	float* pListenTimeArray = DMAudio_GetListenTimeArray();
	for (int i = 0; i < NUM_RADIOS; i++)
		MusicSavedData_FavoriteRadioStationList[i] = pListenTimeArray[i];
}

void Stats_BuildStatLine(const char *text, void *stat, int displayType, void *stat2, int isTime)
{
    // Preserve the complete original BuildStatLine operation and every ordered external access
#define STAT_D *(int*)stat
#define STAT_F *(float*)stat
#define STAT2_D *(int*)stat2
#define STAT2_F *(float*)stat2
	if (!text)
		return;

	gString2[0] = '\0';
	if (isTime == 1) {
		if (*((int*)stat2) >= 10)
			sprintf(gString2, " %d:%d", STAT_D, STAT2_D);
		else
			sprintf(gString2, " %d:0%d", STAT_D, STAT2_D);

	} else if (stat2) {
#ifdef MORE_LANGUAGES
		if (Stats_BackendJapanese()) {
			switch (displayType) {
				case 0:
				case 4:
				sprintf(gString2, "  %d/%d", STAT_D, STAT2_D);
				break;
				case 1:
				sprintf(gString2, "  %.2f/%.2f", STAT_F, STAT2_F);
				break;
				case 2:
				sprintf(gString2, "  %d%%/%d%%", STAT_D, STAT2_D);
				break;
				case 3:
				sprintf(gString2, "  $%.2f/$%.2f", STAT_F, STAT2_F);
				break;
				default:
				break;
			}
		} else
#endif
		{
			switch (displayType) {
				case 0:
				sprintf(gString2, "  %d %s %d", STAT_D, UnicodeToAscii(Text_Get(&TheText,"FEST_OO")), STAT2_D);
				break;
				case 1:
				sprintf(gString2, "  %.2f %s %.2f", STAT_F, UnicodeToAscii(Text_Get(&TheText,"FEST_OO")), STAT2_F);
				break;
				case 2:
				sprintf(gString2, "  %d%% %s %d%%", STAT_D, UnicodeToAscii(Text_Get(&TheText,"FEST_OO")), STAT2_D);
				break;
				case 3:
				sprintf(gString2, "  $%.2f %s $%.2f", STAT_F, UnicodeToAscii(Text_Get(&TheText,"FEST_OO")), STAT2_F);
				break;
				case 4:
				sprintf(gString2, "  %d_ %s %d_", STAT_D, UnicodeToAscii(Text_Get(&TheText,"FEST_OO")), STAT2_D);
				break;
				default:
				break;
			}
		}
	} else if (stat) {
		switch (displayType) {
			case 0:
			sprintf(gString2, "%d", STAT_D);
			break;
			case 1:
			sprintf(gString2, "%.2f", STAT_F);
			break;
			case 2:
			sprintf(gString2, "%d%%", STAT_D);
			break;
			case 3:
			sprintf(gString2, "$%.2f", STAT_F);
			break;
			case 4:
#ifdef MORE_LANGUAGES
			if (Stats_BackendJapanese())
				sprintf(gString2, "%d", STAT_D);
			else
#endif
				sprintf(gString2, "%d_", STAT_D);
			break;
			default:
			break;
		}
	}
	UnicodeStrcpy(gUString, Text_Get(&TheText,text));
	Stats_BackendFilter(gUString);
	AsciiToUnicode(gString2, gUString2);
#undef STAT_D
#undef STAT_F
#undef STAT2_D
#undef STAT2_F
}

int Stats_ConstructStatLine(int rowIdx)
{
    // Preserve the complete original ConstructStatLine operation and every ordered external access

#define STAT_LINE_1(varType, left, right1, type) \
	do { \
		if(counter == rowIdx){ \
			varType a = right1; \
			Stats_BuildStatLine(left, &a, type, NULL, 0); \
			return 0; \
		} counter++; \
	} while(0)

#define STAT_LINE_2(varType, left, right1, type, right2, time) \
	do { \
		if(counter == rowIdx){ \
			varType a = right1; \
			varType b = right2; \
			Stats_BuildStatLine(left, &a, type, &b, time); \
			return 0; \
		} counter++; \
	} while(0)

#define TEXT_ON_LEFT_GXT(name) \
	do { \
		if(counter == rowIdx){ \
			Stats_BuildStatLine(name, NULL, 0, NULL, 0); \
			return 0; \
		} counter++; \
	} while(0)

#define TEXT_ON_RIGHT(text) \
	do { \
		if(counter == rowIdx){ \
			gUString[0] = '\0'; \
			UnicodeStrcpy(gUString2, text); \
			return 0; \
		} counter++; \
	} while(0)

#define FASTEST_TIME(id, str) \
	do { \
		if(Stats_FastestTimes[id]) { \
			if(counter == rowIdx){ \
				int hour = 0, minute; \
				for (int i = Stats_FastestTimes[id]; i > 59; i -= 60) hour++; \
				for (minute = Stats_FastestTimes[id]; minute > 59; minute -= 60); \
				if (minute < 0) minute = -minute; \
				Stats_BuildStatLine(str, &hour, 0, &minute, 1); \
				return 0; \
			} \
			counter++; \
		} \
	} while(0)

	switch (rowIdx) {
		case 0: {
			int percentCompleted = Stats_GetPercentageProgress();
			Stats_BuildStatLine("PER_COM", &percentCompleted, 2, NULL, 0);
			return 0;
		}
		case 1: {
			Stats_BuildStatLine("NMISON", &Stats_MissionsGiven, 0, NULL, 0);
			return 0;
		}
		case 2: {
			int hour = ((*Timer_GetTimeInMilliseconds()) / 60000) / 60;
			int minute = ((*Timer_GetTimeInMilliseconds()) / 60000) % 60;
			Stats_BuildStatLine("ST_TIME", &hour, 0, &minute, 1);
			return 0;
		}
		case 3: {
			Stats_BuildStatLine("DAYSPS", &Stats_DaysPassed, 0, NULL, 0);
			return 0;
		}
		case 4: {
			Stats_BuildStatLine("NUMSHV", &Stats_SafeHouseVisits, 0, NULL, 0);
			return 0;
		}
	}
	int counter = 5;
	
	if (Stats_BackendNastyGame()) {
		STAT_LINE_2(int, "FEST_RP", Stats_NumberKillFrenziesPassed, 0, Stats_TotalNumberKillFrenzies, 0);
	}
	
	CPlayerInfo *player = Stats_BackendPlayerInfo();

	// Hidden packages shouldn't be shown with percent
#ifdef FIX_BUGS
	STAT_LINE_2(int, "PERPIC", Stats_BackendCollectedPackages(player), 0, Stats_BackendTotalPackages(player), 0);
#else
	float fPackagesPercent = 0.0f;
	if (Stats_BackendTotalPackages(player) != 0)
		fPackagesPercent = Stats_BackendCollectedPackages(player) * 100.0f / Stats_BackendTotalPackages(player);

	STAT_LINE_2(int, "PERPIC", fPackagesPercent, 0, 100, 0);
#endif

	if (Stats_BackendNastyGame()) {
		STAT_LINE_1(int, "PE_WAST", Stats_PeopleKilledByPlayer, 0);
		STAT_LINE_1(int, "PE_WSOT", Stats_PeopleKilledByOthers, 0);
	}
	STAT_LINE_1(int, "CAR_EXP", Stats_CarsExploded, 0);
	STAT_LINE_1(int, "BOA_EXP", Stats_BoatsExploded, 0);
	STAT_LINE_1(int, "HEL_DST", Stats_HelisDestroyed, 0);
	STAT_LINE_1(int, "TYREPOP", Stats_TyresPopped, 0);
	STAT_LINE_1(int, "ST_STAR", Stats_WantedStarsAttained, 0);
	STAT_LINE_1(int, "ST_STGN", Stats_WantedStarsEvaded, 0);
	STAT_LINE_1(int, "TM_BUST", Stats_TimesArrested, 0);
	STAT_LINE_1(int, "TM_DED", Stats_TimesDied, 0);

#ifdef MORE_LANGUAGES
	// JP version removed it altogether actually
	if (!Stats_BackendJapanese())
#endif
	STAT_LINE_1(int, "ST_HEAD", Stats_HeadsPopped, 0);

	static uint32_t lastProcessedDay = UINT32_MAX;
	static uint32_t lastPoliceSpending = 0;

	// What a random stat...
	if (lastProcessedDay != Stats_DaysPassed) {
		lastProcessedDay = Stats_DaysPassed;
		lastPoliceSpending = ((*Timer_GetTimeInMilliseconds()) & 255 + 80) * 255.44f;
	}
	STAT_LINE_1(float, "DAYPLC", lastPoliceSpending, 3);

	int mostPatheticGang = 0;
	int mostKill = 0;
	for (int i = PEDTYPE_GANG1; i < PEDTYPE_GANG9; ++i) {
		if (Stats_PedsKilledOfThisType[i] > mostKill) {
			mostKill = Stats_PedsKilledOfThisType[i];
			mostPatheticGang = i;
		}
	}
	if (mostPatheticGang > 0) {
		TEXT_ON_LEFT_GXT("ST_GANG");

		switch (mostPatheticGang) {
			case PEDTYPE_GANG1:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG1"));
				break;
			case PEDTYPE_GANG2:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG2"));
				break;
			case PEDTYPE_GANG3:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG3"));
				break;
			case PEDTYPE_GANG4:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG4"));
				break;
			case PEDTYPE_GANG5:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG5"));
				break;
			case PEDTYPE_GANG6:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG6"));
				break;
			case PEDTYPE_GANG7:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG7"));
				break;
			case PEDTYPE_GANG8:
				TEXT_ON_RIGHT(Text_Get(&TheText,"ST_GNG8"));
				break;
			default:
				break;
		}
	}

	STAT_LINE_1(int, "GNG_WST", Stats_PedsKilledOfThisType[PEDTYPE_GANG9] + Stats_PedsKilledOfThisType[PEDTYPE_GANG8]
			+ Stats_PedsKilledOfThisType[PEDTYPE_GANG7] + Stats_PedsKilledOfThisType[PEDTYPE_GANG6]
			+ Stats_PedsKilledOfThisType[PEDTYPE_GANG5] + Stats_PedsKilledOfThisType[PEDTYPE_GANG4]
			+ Stats_PedsKilledOfThisType[PEDTYPE_GANG3] + Stats_PedsKilledOfThisType[PEDTYPE_GANG2]
			+ Stats_PedsKilledOfThisType[PEDTYPE_GANG1], 0);

	STAT_LINE_1(int, "DED_CRI", Stats_PedsKilledOfThisType[PEDTYPE_CRIMINAL], 0);
	STAT_LINE_1(int, "KGS_EXP", Stats_KgsOfExplosivesUsed, 0);
	STAT_LINE_1(int, "BUL_FIR", Stats_RoundsFiredByPlayer, 0);
	STAT_LINE_1(int, "BUL_HIT", Stats_BulletsThatHit, 0);
;
	STAT_LINE_1(int, "ACCURA", Stats_RoundsFiredByPlayer == 0 ? 0 : (Stats_BulletsThatHit * 100.0f / (float)Stats_RoundsFiredByPlayer), 2);

	switch (Stats_BackendLanguage()) {
		case TEXT_LANGUAGE_AMERICAN:
#ifndef USE_MEASUREMENTS_IN_METERS
			STAT_LINE_1(float, "FEST_DF", Stats_DistanceTravelledOnFoot * MILES_IN_METER, 1);
			STAT_LINE_1(float, "FEST_DC", Stats_DistanceTravelledByCar * MILES_IN_METER, 1);
			STAT_LINE_1(float, "DISTBIK", Stats_DistanceTravelledByBike * MILES_IN_METER, 1);
			STAT_LINE_1(float, "DISTBOA", Stats_DistanceTravelledByBoat * MILES_IN_METER, 1);
			STAT_LINE_1(float, "DISTGOL", Stats_DistanceTravelledByGolfCart * MILES_IN_METER, 1);
			STAT_LINE_1(float, "DISTHEL", Stats_DistanceTravelledByHelicoptor * MILES_IN_METER, 1);
#ifdef FIX_BUGS
			STAT_LINE_1(float, "TOT_DIS", (Stats_DistanceTravelledOnFoot + Stats_DistanceTravelledByCar + Stats_DistanceTravelledByBoat + Stats_DistanceTravelledByBike
				+ Stats_DistanceTravelledByGolfCart + Stats_DistanceTravelledByHelicoptor + Stats_DistanceTravelledByPlane) * MILES_IN_METER, 1);
			STAT_LINE_1(float, "MXCARD", Stats_MaximumJumpDistance * FEET_IN_METER, 1);
			STAT_LINE_1(float, "MXCARJ", Stats_MaximumJumpHeight * FEET_IN_METER, 1);
#else
			STAT_LINE_1(float, "TOT_DIS", (Stats_DistanceTravelledOnFoot + Stats_DistanceTravelledByCar + Stats_DistanceTravelledByBoat + Stats_DistanceTravelledByBike
				+ Stats_DistanceTravelledByGolfCart + Stats_DistanceTravelledByHelicoptor) * MILES_IN_METER, 1);
#endif
			break;
#endif
		case TEXT_LANGUAGE_FRENCH:
		case TEXT_LANGUAGE_GERMAN:
		case TEXT_LANGUAGE_ITALIAN:
		case TEXT_LANGUAGE_SPANISH:
#ifdef MORE_LANGUAGES
		case TEXT_LANGUAGE_POLISH:
		case TEXT_LANGUAGE_RUSSIAN:
		case TEXT_LANGUAGE_JAPANESE:
#endif
			STAT_LINE_1(float, "FESTDFM", Stats_DistanceTravelledOnFoot, 1);
			STAT_LINE_1(float, "FESTDCM", Stats_DistanceTravelledByCar, 1);
			STAT_LINE_1(float, "DISTBIM", Stats_DistanceTravelledByBike, 1);
			STAT_LINE_1(float, "DISTBOM", Stats_DistanceTravelledByBoat, 1);
			STAT_LINE_1(float, "DISTGOM", Stats_DistanceTravelledByGolfCart, 1);
			STAT_LINE_1(float, "DISTHEM", Stats_DistanceTravelledByHelicoptor, 1);
#ifdef FIX_BUGS
			STAT_LINE_1(float, "TOTDISM", Stats_DistanceTravelledOnFoot + Stats_DistanceTravelledByCar + Stats_DistanceTravelledByBoat
				+ Stats_DistanceTravelledByBike + Stats_DistanceTravelledByGolfCart + Stats_DistanceTravelledByHelicoptor + Stats_DistanceTravelledByPlane, 1);
			STAT_LINE_1(float, "MXCARDM", Stats_MaximumJumpDistance, 1);
			STAT_LINE_1(float, "MXCARJM", Stats_MaximumJumpHeight, 1);
#else
			STAT_LINE_1(float, "TOTDISM", Stats_DistanceTravelledOnFoot + Stats_DistanceTravelledByCar + Stats_DistanceTravelledByBoat
				+ Stats_DistanceTravelledByGolfCart + Stats_DistanceTravelledByHelicoptor, 1);
#endif
			break;
		default:
			break;
	}

	// They were selecting the unit according to language in III, but they deleted the feet code in VC. Weird
#ifndef FIX_BUGS
	STAT_LINE_1(float, "MXCARDM", Stats_MaximumJumpDistance, 1);
	STAT_LINE_1(float, "MXCARJM", Stats_MaximumJumpHeight, 1);
#endif
	STAT_LINE_1(int, "MXFLIP", Stats_MaximumJumpFlips, 0);
	STAT_LINE_2(int, "NOUNIF", Stats_NumberOfUniqueJumpsFound, 0, Stats_TotalNumberOfUniqueJumps, 0);
	STAT_LINE_1(int, "MXJUMP", Stats_MaximumJumpSpins, 4);

	TEXT_ON_LEFT_GXT("BSTSTU");
	switch (Stats_BestStuntJump) {
		case 1:
			TEXT_ON_RIGHT(Text_Get(&TheText,"INSTUN"));
			break;
		case 2:
			TEXT_ON_RIGHT(Text_Get(&TheText,"PRINST"));
			break;
		case 3:
			TEXT_ON_RIGHT(Text_Get(&TheText,"DBINST"));
			break;
		case 4:
			TEXT_ON_RIGHT(Text_Get(&TheText,"DBPINS"));
			break;
		case 5:
			TEXT_ON_RIGHT(Text_Get(&TheText,"TRINST"));
			break;
		case 6:
			TEXT_ON_RIGHT(Text_Get(&TheText,"PRTRST"));
			break;
		case 7:
			TEXT_ON_RIGHT(Text_Get(&TheText,"QUINST"));
			break;
		case 8:
			TEXT_ON_RIGHT(Text_Get(&TheText,"PQUINS"));
			break;
		default:
			TEXT_ON_RIGHT(Text_Get(&TheText,"NOSTUC"));
			break;
	}
	STAT_LINE_1(int, "ST_WHEE", Stats_LongestWheelie, 0);
	STAT_LINE_1(float, "ST_WHED", Stats_LongestWheelieDist, 1);
	STAT_LINE_1(int, "ST_STOP", Stats_LongestStoppie, 0);
	STAT_LINE_1(float, "ST_STOD", Stats_LongestStoppieDist, 1);
	STAT_LINE_1(int, "ST_2WHE", Stats_Longest2Wheel, 0);
	STAT_LINE_1(float, "ST_2WHD", Stats_Longest2WheelDist, 1);

	if (Stats_LoanSharks > 0.0f)
		STAT_LINE_1(int, "ST_LOAN", Stats_LoanSharks, 0);

	STAT_LINE_1(int, "FEST_CC", Stats_CriminalsCaught, 0);
	STAT_LINE_1(int, "FEST_HV", Stats_HighestLevelVigilanteMission, 0);
	STAT_LINE_1(int, "PASDRO", Stats_PassengersDroppedOffWithTaxi, 0);
	STAT_LINE_1(float, "MONTAX", Stats_MoneyMadeWithTaxi, 3);
	STAT_LINE_1(int, "FEST_LS", Stats_LivesSavedWithAmbulance, 0);
	STAT_LINE_1(int, "FEST_HA", Stats_HighestLevelAmbulanceMission, 0);
	STAT_LINE_1(int, "FEST_FE", Stats_FiresExtinguished, 0);
	STAT_LINE_1(int, "FIRELVL", Stats_HighestLevelFireMission, 0);

	STAT_LINE_2(int, "ST_STOR", Stats_StoresKnockedOff, 0, 15, 0);

	if (Stats_MovieStunts > 0.0f)
		STAT_LINE_1(int, "ST_MOVI", Stats_MovieStunts, 0);

	STAT_LINE_2(int, "ST_ASSI", Stats_Assassinations, 0, 5, 0);

	if (Stats_PhotosTaken > 0)
		STAT_LINE_1(int, "ST_PHOT", Stats_PhotosTaken, 0);

	if (Stats_PizzasDelivered > 0.0f)
		STAT_LINE_1(int, "ST_PIZZ", Stats_PizzasDelivered, 0);

	if (Stats_GarbagePickups > 0.0f)
		STAT_LINE_1(int, "ST_GARB", Stats_GarbagePickups, 0);

	if (Stats_IceCreamSold > 0.0f)
		STAT_LINE_1(int, "ST_ICEC", Stats_IceCreamSold, 0);

	if (Stats_HighestScores[1])
		STAT_LINE_1(int, "STHC_02", Stats_HighestScores[1], 0);

	FASTEST_TIME(0, "STFT_01");
	FASTEST_TIME(1, "STFT_02");
	FASTEST_TIME(2, "STFT_03");
	FASTEST_TIME(3, "STFT_04");
	FASTEST_TIME(4, "STFT_05");
	FASTEST_TIME(5, "STFT_06");
	FASTEST_TIME(6, "STFT_07");
	FASTEST_TIME(7, "STFT_08");
	FASTEST_TIME(8, "STFT_09");
	FASTEST_TIME(9, "STFT_10");
	FASTEST_TIME(10, "STFT_11");
	FASTEST_TIME(11, "STFT_12");
	FASTEST_TIME(12, "STFT_13");
	FASTEST_TIME(13, "STFT_14");
	FASTEST_TIME(14, "STFT_15");
	FASTEST_TIME(15, "STFT_16");
	FASTEST_TIME(16, "STFT_17");
	FASTEST_TIME(17, "STFT_18");
	FASTEST_TIME(18, "STFT_19");
	FASTEST_TIME(19, "STFT_20");
	FASTEST_TIME(22, "STFT_23");

	if (Stats_HighestScores[0])
		STAT_LINE_1(int, "STHC_01", Stats_HighestScores[0], 0);

	if (Stats_HighestScores[3])
		STAT_LINE_1(int, "STHC_04", Stats_HighestScores[3], 0);

	if (Stats_HighestScores[2])
		STAT_LINE_1(int, "STHC_03", Stats_HighestScores[2], 0);

	if (Stats_BestPositions[0] != INT_MAX)
		STAT_LINE_1(int, "STHC_05", Stats_BestPositions[0], 0);

	FASTEST_TIME(20, "STFT_21");

	if (Stats_FastestTimes[21])
#ifdef FIX_BUGS
		STAT_LINE_1(float, "STFT_22", floorf(Stats_FastestTimes[21] / 10) / 100, 1);
#else
		STAT_LINE_1(float, "STFT_22", Stats_FastestTimes[21] / 1000, 1);
#endif

	if (Stats_TopShootingRangeScore > 0.0f)
		STAT_LINE_1(int, "TOP_SHO", Stats_TopShootingRangeScore, 0);

	if (Stats_ShootingRank > 0.0f)
		STAT_LINE_1(int, "SHO_RAN", Stats_ShootingRank, 0);

	int flightMinute = (Stats_FlightTime / 60000) % 60;
	int flightHour = (Stats_FlightTime / 60000) / 60;
	STAT_LINE_2(int, "ST_FTIM", flightHour, 0, flightMinute, 1);

	// We always have pilot rank if we flew more then 5 minutes
#ifndef FIX_BUGS
	if (flightHour != 0)
		TEXT_ON_LEFT_GXT("ST_PRAN");
#endif

#ifdef FIX_BUGS
#define FL_TIME_MORE_THAN(hour, minute) (flightHour > hour || (flightHour == hour && flightMinute >= minute))
#else
#define FL_TIME_MORE_THAN(hour, minute) (flightHour > hour || flightMinute >= minute)
#endif

	if (FL_TIME_MORE_THAN(0,5)) {

#ifdef FIX_BUGS
		TEXT_ON_LEFT_GXT("ST_PRAN");
#endif
		if (!FL_TIME_MORE_THAN(0,10)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR01"));
		else if (!FL_TIME_MORE_THAN(0,20)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR02"));
		else if (!FL_TIME_MORE_THAN(0,30)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR03"));
		else if (!FL_TIME_MORE_THAN(1,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR04"));
		else if (!FL_TIME_MORE_THAN(1,30)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR05"));
		else if (!FL_TIME_MORE_THAN(2,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR06"));
		else if (!FL_TIME_MORE_THAN(2,30)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR07"));
		else if (!FL_TIME_MORE_THAN(3,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR08"));
		else if (!FL_TIME_MORE_THAN(3,30)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR09"));
		else if (!FL_TIME_MORE_THAN(4,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR10"));
		else if (!FL_TIME_MORE_THAN(5,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR11"));
		else if (!FL_TIME_MORE_THAN(10,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR12"));
		else if (!FL_TIME_MORE_THAN(20,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR13"));
		else if (!FL_TIME_MORE_THAN(25,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR14"));
		else if (!FL_TIME_MORE_THAN(30,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR15"));
		else if (!FL_TIME_MORE_THAN(49,2)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR16"));
		else if (!FL_TIME_MORE_THAN(50,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR17"));
		else if (!FL_TIME_MORE_THAN(100,0)) TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR18"));
		else TEXT_ON_RIGHT(Text_Get(&TheText,"ST_PR19"));
	}
#undef FL_TIME_MORE_THAN

	if (Stats_BloodRingKills > 0)
		STAT_LINE_1(int, "ST_BRK", Stats_BloodRingKills, 0);

	if (Stats_BloodRingTime > 0)
		STAT_LINE_1(int, "ST_LTBR", Stats_BloodRingTime, 0);

	STAT_LINE_1(int, "ST_DRWN", Stats_TimesDrowned, 0);

	if (Stats_SeagullsKilled > 0)
		STAT_LINE_1(int, "SEAGULL", Stats_SeagullsKilled, 0);

	bool playerHatesRadio = true;
	float* pListenTimeArray = DMAudio_GetListenTimeArray();
	for (int i = 0; i < NUM_RADIOS; i++) {
		MusicSavedData_FavoriteRadioStationList[i] = pListenTimeArray[i];
		if (MusicSavedData_FavoriteRadioStationList[i] != 0.0) // double
			playerHatesRadio = false;
	}

	if (!playerHatesRadio) {
		// Most listened
		TEXT_ON_LEFT_GXT("FST_MFR");
		float mostListenTime = MusicSavedData_FavoriteRadioStationList[0];
		int mostListenedRadio = 0;
		for (int i = 0; i < NUM_RADIOS; i++) {
			if (MusicSavedData_FavoriteRadioStationList[i] > mostListenTime) {
				mostListenTime = MusicSavedData_FavoriteRadioStationList[i];
				mostListenedRadio = i;
			}
		}
		switch (mostListenedRadio) {
			case WILDSTYLE:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM0"));
				break;
			case FLASH_FM:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM1"));
				break;
			case KCHAT:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM2"));
				break;
			case FEVER:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM3"));
				break;
			case V_ROCK:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM4"));
				break;
			case VCPR:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM5"));
				break;
			case RADIO_ESPANTOSO:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM6"));
				break;
			case EMOTION:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM7"));
				break;
			case WAVE:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM8"));
				break;
			case USERTRACK:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_MP3"));
				break;
			default:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM8")); // heh
				break;
		}

		// Least listened
		TEXT_ON_LEFT_GXT("FST_LFR");
		float leastListenTime = MusicSavedData_FavoriteRadioStationList[0];
		int leastListenedRadio = 0;
		for (int i = 0; i < NUM_RADIOS; i++) {
#ifdef FIX_BUGS
			if (!DMAudio_IsMP3RadioChannelAvailable() && i == USERTRACK)
				continue;
#endif
			if (MusicSavedData_FavoriteRadioStationList[i] < leastListenTime) {
				leastListenTime = MusicSavedData_FavoriteRadioStationList[i];
				leastListenedRadio = i;
			}
		}
#ifndef FIX_BUGS
		if (!DMAudio_IsMP3RadioChannelAvailable() && leastListenedRadio == USERTRACK)
			leastListenedRadio = WAVE;
#endif

		switch (leastListenedRadio) {
			case WILDSTYLE:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM0"));
				break;
			case FLASH_FM:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM1"));
				break;
			case KCHAT:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM2"));
				break;
			case FEVER:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM3"));
				break;
			case V_ROCK:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM4"));
				break;
			case VCPR:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM5"));
				break;
			case RADIO_ESPANTOSO:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM6"));
				break;
			case EMOTION:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM7"));
				break;
			case WAVE:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM8"));
				break;
			case USERTRACK:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_MP3"));
				break;
			default:
				TEXT_ON_RIGHT(Text_Get(&TheText,"FEA_FM8")); // heh
				break;
		}
	}
	STAT_LINE_1(int, "SPRAYIN", Stats_Sprayings, 0);
	STAT_LINE_1(float, "ST_WEAP", Stats_WeaponBudget, 3);
	STAT_LINE_1(float, "ST_FASH", Stats_FashionBudget, 3);
	STAT_LINE_1(float, "ST_PROP", Stats_PropertyBudget, 3);
	STAT_LINE_1(float, "ST_AUTO", Stats_AutoPaintingBudget, 3);
	STAT_LINE_1(float, "ST_DAMA", Stats_PropertyBudget, 3);

	if (Stats_NumPropertyOwned > 0) {
		STAT_LINE_1(int, "PROPOWN", Stats_NumPropertyOwned, 0);
		if (Stats_PropertyOwned[0]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_1"));
		if (Stats_PropertyOwned[1]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_2"));
		if (Stats_PropertyOwned[2]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_3"));
		if (Stats_PropertyOwned[3]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_4"));
		if (Stats_PropertyOwned[4]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_5"));
		if (Stats_PropertyOwned[5]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_6"));
		if (Stats_PropertyOwned[6]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_7"));
		if (Stats_PropertyOwned[7]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_8"));
		if (Stats_PropertyOwned[8]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_9"));
		if (Stats_PropertyOwned[9]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_10"));
		if (Stats_PropertyOwned[10]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_11"));
		if (Stats_PropertyOwned[11]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_12"));
		if (Stats_PropertyOwned[12]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_13"));
		if (Stats_PropertyOwned[13]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_14"));
		if (Stats_PropertyOwned[14]) TEXT_ON_RIGHT(Text_Get(&TheText,"STPR_15"));
	}
	STAT_LINE_1(int, "CHASE", Stats_HighestChaseValue, 0);
	TEXT_ON_RIGHT(Stats_FindChaseString(Stats_HighestChaseValue));

	return counter;

#undef STAT_LINE_1
#undef STAT_LINE_2
#undef TEXT_ON_LEFT_GXT
#undef TEXT_ON_RIGHT
#undef FASTEST_TIME
}

void Stats_CheckPointReachedSuccessfully(void)
{
    // Preserve the complete original CheckPointReachedSuccessfully operation and every ordered external access
Stats_TotalLegitimateKills += Stats_KillsSinceLastCheckpoint; Stats_KillsSinceLastCheckpoint = 0;
}

void Stats_CheckPointReachedUnsuccessfully(void)
{
    // Preserve the complete original CheckPointReachedUnsuccessfully operation and every ordered external access
Stats_KillsSinceLastCheckpoint = 0;
}
//- rouz edit (ChatGPT)
