//+ rouz edit (ChatGPT)
#include "PopulationCounts.h"
#include "PopulationGroups.h"
#include "CutsceneState.h"
#include "ReplayState.h"
uint32_t Population_nTotalMissionPeds;
uint32_t Population_nNumCivMale;
uint32_t Population_nNumCivFemale;
uint32_t Population_nNumCop;
uint32_t Population_nNumEmergency;
uint32_t Population_nNumGang1;
uint32_t Population_nNumGang2;
uint32_t Population_nTotalPeds;
uint32_t Population_nNumGang3;
uint32_t Population_nTotalGangPeds;
uint32_t Population_nNumGang4;
uint32_t Population_nTotalCivPeds;
uint32_t Population_nNumGang5;
uint32_t Population_nNumDummy;
uint32_t Population_nNumGang6;
uint32_t Population_nNumGang9;
uint32_t Population_nNumGang7;
uint32_t Population_nNumGang8;
uint32_t Population_nTotalCarPassengerPeds;

bool Population_givePedsWeapons;
int32_t Population_allRandomPedsType = -1;
float Population_pedDensityMultiplier = 1.0f;
bool Population_zoneChangeHasHappened;
int8_t Population_countDownToPedsAtStart;
uint32_t Population_numMiamiViceCops;

void Population_Initialise(void)
{
    // Preserve the original initialization order and untouched settings
    PopulationCounts_InitialiseLog(true);
    Population_ResetCounts();
    Population_countDownToPedsAtStart = 2;
    Population_zoneChangeHasHappened = false;
    Population_allRandomPedsType = -1;
    Population_pedDensityMultiplier = 1.0f;
    Population_LoadPedGroups();
    PopulationCounts_InitialiseLog(false);
}

void Population_Update(bool addPeds)
{
    // Skip all population work during replay playback
    if (!Replay_IsPlayingBack()) {
        // Run maintenance before the startup countdown or normal spawning
        PopulationHost_Manage();
        if (Population_countDownToPedsAtStart != 0) {
            // Generate the initial population when the countdown reaches zero
            if (--Population_countDownToPedsAtStart == 0)
                Population_GeneratePedsAtStartOfGame();
        } else {
            // Update totals before the original cutscene and add-peds checks
            Population_RecalculateTotals();
            if (!Cutscene_IsRunning() && addPeds)
                {
                // Calculate regular spawn distances using live camera values
                float pcdm = Population_PedCreationDistMultiplier();
                PopulationHost_AddToPopulation(pcdm * (40.0f * PopulationHost_CameraGenerationMultiplier()),
                    pcdm * (50.0f * PopulationHost_CameraGenerationMultiplier()),
                    pcdm * 50.0f * 0.5f - 10.0f,
                    pcdm * 50.0f * 0.5f);
            }
        }
    }
}

void Population_GeneratePedsAtStartOfGame(void)
{
    // Recalculate and attempt population generation one hundred times
    int i;
    for (i = 0; i < 100; i++) {
        // Preserve the original per-attempt totals and world call
        Population_RecalculateTotals();
        PopulationHost_AddToPopulation(10.0f, Population_PedCreationDistMultiplier() * 50.0f,
            10.0f, Population_PedCreationDistMultiplier() * 50.0f);
    }
}

float Population_PedCreationDistMultiplier(void)
{
    // Keep the original speed adjustment and clamp behavior
    float speed, value;
    if (!PopulationHost_PlayerVehicleSpeed(&speed))
        return 1.0f;
    value = speed - 0.1f + 1.0f;
    return value < 1.0f ? 1.0f : value > 1.5f ? 1.5f : value;
}

void Population_ResetCounts(void)
{
    // Reset counters in the same order as population initialization
    Population_nNumCivMale = 0;
    Population_nNumCivFemale = 0;
    Population_nNumCop = 0;
    Population_nNumEmergency = 0;
    Population_nNumGang1 = 0;
    Population_nNumGang2 = 0;
    Population_nNumGang3 = 0;
    Population_nNumGang4 = 0;
    Population_nNumGang5 = 0;
    Population_nNumGang6 = 0;
    Population_nNumGang7 = 0;
    Population_nNumGang8 = 0;
    Population_nNumGang9 = 0;
    Population_nNumDummy = 0;
    Population_nTotalCarPassengerPeds = 0;
    Population_nTotalCivPeds = 0;
    Population_nTotalGangPeds = 0;
    Population_nTotalPeds = 0;
    Population_nTotalMissionPeds = 0;
}

void Population_RecalculateTotals(void)
{
    // Preserve the original unsigned sums and passenger subtraction
    Population_nTotalCivPeds = Population_nNumCivFemale + Population_nNumCivMale;
    Population_nTotalGangPeds = Population_nNumGang9 + Population_nNumGang8 + Population_nNumGang7 +
        Population_nNumGang6 + Population_nNumGang5 + Population_nNumGang4 + Population_nNumGang3 +
        Population_nNumGang2 + Population_nNumGang1;
    Population_nTotalPeds = Population_nNumDummy + Population_nNumEmergency + Population_nNumCop +
        Population_nTotalGangPeds + Population_nNumCivFemale + Population_nNumCivMale;
    Population_nTotalPeds -= Population_nTotalCarPassengerPeds;
}

void
Population_UpdatePedCount(ePedType pedType, bool decrease)
{
	// Preserve the original pedestrian-type counter updates
	if (decrease) {
		switch (pedType) {
			case PEDTYPE_PLAYER1:
			case PEDTYPE_PLAYER2:
			case PEDTYPE_PLAYER3:
			case PEDTYPE_PLAYER4:
			case PEDTYPE_UNUSED1:
			case PEDTYPE_SPECIAL:
				return;
			case PEDTYPE_CIVMALE:
				--Population_nNumCivMale;
				break;
			case PEDTYPE_CIVFEMALE:
				--Population_nNumCivFemale;
				break;
			case PEDTYPE_COP:
				--Population_nNumCop;
				break;
			case PEDTYPE_GANG1:
				--Population_nNumGang1;
				break;
			case PEDTYPE_GANG2:
				--Population_nNumGang2;
				break;
			case PEDTYPE_GANG3:
				--Population_nNumGang3;
				break;
			case PEDTYPE_GANG4:
				--Population_nNumGang4;
				break;
			case PEDTYPE_GANG5:
				--Population_nNumGang5;
				break;
			case PEDTYPE_GANG6:
				--Population_nNumGang6;
				break;
			case PEDTYPE_GANG7:
				--Population_nNumGang7;
				break;
			case PEDTYPE_GANG8:
				--Population_nNumGang8;
				break;
			case PEDTYPE_GANG9:
				--Population_nNumGang9;
				break;
			case PEDTYPE_EMERGENCY:
			case PEDTYPE_FIREMAN:
				--Population_nNumEmergency;
				break;
			case PEDTYPE_CRIMINAL:
				--Population_nNumCivMale;
				break;
			case PEDTYPE_PROSTITUTE:
				--Population_nNumCivFemale;
				break;
			case PEDTYPE_UNUSED2:
				--Population_nNumDummy;
				break;
			default:
				PopulationCounts_UnknownPedType();
				break;
		}
	} else {
		switch (pedType) {
			case PEDTYPE_PLAYER1:
			case PEDTYPE_PLAYER2:
			case PEDTYPE_PLAYER3:
			case PEDTYPE_PLAYER4:
			case PEDTYPE_UNUSED1:
			case PEDTYPE_SPECIAL:
				return;
			case PEDTYPE_CIVMALE:
				++Population_nNumCivMale;
				break;
			case PEDTYPE_CIVFEMALE:
				++Population_nNumCivFemale;
				break;
			case PEDTYPE_COP:
				++Population_nNumCop;
				break;
			case PEDTYPE_GANG1:
				++Population_nNumGang1;
				break;
			case PEDTYPE_GANG2:
				++Population_nNumGang2;
				break;
			case PEDTYPE_GANG3:
				++Population_nNumGang3;
				break;
			case PEDTYPE_GANG4:
				++Population_nNumGang4;
				break;
			case PEDTYPE_GANG5:
				++Population_nNumGang5;
				break;
			case PEDTYPE_GANG6:
				++Population_nNumGang6;
				break;
			case PEDTYPE_GANG7:
				++Population_nNumGang7;
				break;
			case PEDTYPE_GANG8:
				++Population_nNumGang8;
				break;
			case PEDTYPE_GANG9:
				++Population_nNumGang9;
				break;
			case PEDTYPE_EMERGENCY:
			case PEDTYPE_FIREMAN:
				++Population_nNumEmergency;
				break;
			case PEDTYPE_CRIMINAL:
				++Population_nNumCivMale;
				break;
			case PEDTYPE_PROSTITUTE:
				++Population_nNumCivFemale;
				break;
			case PEDTYPE_UNUSED2:
				++Population_nNumDummy;
				break;
			default:
				PopulationCounts_UnknownPedType();
				break;
		}
	}
}


//- rouz edit (ChatGPT)
