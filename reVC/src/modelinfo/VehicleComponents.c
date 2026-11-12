//+ rouz edit (Claude)
#include "config.h"
#include "General.h"
#include "VehicleComponents.h"

#define COMPRULE_RULE(comprule) (((comprule) >> 12) & 0xF)
#define COMPRULE_COMPS(comprule) ((comprule) & 0xFFF)
#define COMPRULE_COMPN(comps, n) (((comps) >> 4*(n)) & 0xF)
#define COMPRULE2_RULE(comprule) (((comprule) >> (12+16)) & 0xF)
#define COMPRULE2_COMPS(comprule) ((comprule >> 16) & 0xFFF)

bool VehicleComp_IsValidRule(int rule, bool raining)
{
	// Rule 2 is only valid in the rain
	if (rule == 2)
		return raining;
	return true;
}

int32_t VehicleComp_CountInRule(int comps)
{
	// Count the nibbles that are not 0xF
	int32_t n;
	for (n = 0; comps != 0; comps >>= 4)
		if ((comps & 0xF) != 0xF)
			n++;
	return n;
}

int32_t VehicleComp_ChooseFromRule(int32_t rule, int32_t comps)
{
	int32_t n;
	switch (rule) {
	// identical cases....
	case 1:
		n = General_GetRandomNumberInRangeInt(0, VehicleComp_CountInRule(comps));
		return COMPRULE_COMPN(comps, n);
	case 2:
		// only valid in rain
		n = General_GetRandomNumberInRangeInt(0, VehicleComp_CountInRule(comps));
		return COMPRULE_COMPN(comps, n);
	case 3:
		n = General_GetRandomNumberInRangeInt(0, 1 + VehicleComp_CountInRule(comps));
		if (n != 0)
			return COMPRULE_COMPN(comps, n - 1);
		return -1;
	case 4:
#ifdef FIX_BUGS
		return General_GetRandomNumberInRangeInt(0, 6);
#else
		return General_GetRandomNumberInRangeInt(0, 5);
#endif
	}
	return -1;
}

int32_t VehicleComp_GetUnusedList(uint32_t comprules, int32_t numComps, int32_t *comps, bool raining)
{
	int32_t i, n;
	int32_t unused[6] = { 0, 1, 2, 3, 4, 5 };

	// first comprule
	if (COMPRULE_RULE(comprules) && VehicleComp_IsValidRule(COMPRULE_RULE(comprules), raining))
		for (i = 0; i < 3; i++) {
			n = COMPRULE_COMPN(comprules, i);
			if (n != 0xF)
				unused[n] = 0xF;
		}
	// second comprule
	comprules >>= 16;
	if (COMPRULE_RULE(comprules) && VehicleComp_IsValidRule(COMPRULE_RULE(comprules), raining))
		for (i = 0; i < 3; i++) {
			n = COMPRULE_COMPN(comprules, i);
			if (n != 0xF)
				unused[n] = 0xF;
		}

	n = 0;
	for (i = 0; i < numComps; i++)
		if (unused[i] != 0xF)
			comps[n++] = unused[i];
	return n;
}

int32_t VehicleComp_ChooseFirst(int8_t *compsToUse, uint32_t compRules, int8_t numComps, bool raining)
{
	int32_t comp;
	int32_t comps[8];
	int32_t n;

	comp = -1;
	if (compsToUse[0] == -2) {
		if (COMPRULE_RULE(compRules) && VehicleComp_IsValidRule(COMPRULE_RULE(compRules), raining))
			comp = VehicleComp_ChooseFromRule(COMPRULE_RULE(compRules), COMPRULE_COMPS(compRules));
		else if (General_GetRandomNumberInRangeInt(0, 3) < 2) {
			n = VehicleComp_GetUnusedList(compRules, numComps, comps, raining);
			if (n)
				comp = comps[(int)General_GetRandomNumberInRangeInt(0, n)];
		}
	} else {
		comp = compsToUse[0];
		compsToUse[0] = -2;
	}
	return comp;
}

int32_t VehicleComp_ChooseSecond(int8_t *compsToUse, uint32_t compRules, int8_t numComps, bool raining)
{
	int32_t comp;
	int32_t comps[8];
	int32_t n;

	comp = -1;
	if (compsToUse[1] == -2) {
		if (COMPRULE2_RULE(compRules) && VehicleComp_IsValidRule(COMPRULE2_RULE(compRules), raining))
			comp = VehicleComp_ChooseFromRule(COMPRULE2_RULE(compRules), COMPRULE2_COMPS(compRules));
		else if (COMPRULE_RULE(compRules) && VehicleComp_IsValidRule(COMPRULE_RULE(compRules), raining) &&
		         General_GetRandomNumberInRangeInt(0, 3) < 2) {

			n = VehicleComp_GetUnusedList(compRules, numComps, comps, raining);
			if (n)
				comp = comps[(int)General_GetRandomNumberInRangeInt(0, n)];
		}
	} else {
		comp = compsToUse[1];
		compsToUse[1] = -2;
	}
	return comp;
}
//- rouz edit (Claude)
