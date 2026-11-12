//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
// Component rule decoding and random component choice of a vehicle model, raining tells whether rule 2 is valid
bool VehicleComp_IsValidRule(int rule, bool raining);
int32_t VehicleComp_CountInRule(int comps);
int32_t VehicleComp_ChooseFromRule(int32_t rule, int32_t comps);
int32_t VehicleComp_GetUnusedList(uint32_t comprules, int32_t numComps, int32_t *comps, bool raining);
// Choose the first or second component, the forced component array is cleared when used
int32_t VehicleComp_ChooseFirst(int8_t *compsToUse, uint32_t compRules, int8_t numComps, bool raining);
int32_t VehicleComp_ChooseSecond(int8_t *compsToUse, uint32_t compRules, int8_t numComps, bool raining);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
