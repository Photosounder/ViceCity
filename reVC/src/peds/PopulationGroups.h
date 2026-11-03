//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "PedType.h"
typedef struct PedGroup {
    int32_t models[NUMMODELSPERPEDGROUP];
} PedGroup;
#ifdef __cplusplus
extern "C" {
#endif
extern PedGroup Population_pedGroups[NUMPEDGROUPS];
void Population_LoadPedGroups(void);
int32_t Population_ChoosePolicePedOccupation(void);
int Population_ChooseGangOccupation(int gangId);
bool Population_CanJeerAtStripper(int32_t model);
int32_t Population_ComputeRandomisedGangSize(void);
bool Population_CanSolicitPlayerInCar(int32_t model);
bool Population_CanSolicitPlayerOnFoot(int32_t model);
bool Population_IsSecurityGuard(ePedType pedType);
bool Population_IsMale(int32_t model);
bool Population_IsFemale(int32_t model);
bool PopulationGroups_IsCivilianMale(int32_t model);
bool PopulationGroups_FemaleMatches(int32_t man, int32_t woman);
void Population_ChooseCivilianCoupleOccupations(int32_t group, int32_t *man, int32_t *woman);
bool Population_IsSunbather(int32_t model);
int32_t Population_ChooseCivilianOccupation(int32_t group);
int32_t Population_ChooseNextCivilianOccupation(int32_t group);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
