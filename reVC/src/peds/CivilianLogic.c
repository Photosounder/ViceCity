//+ rouz edit (Claude)
#include "CivilianLogic.h"
#include "PedAttractorTypes.h"

bool Civilian_IsAttractedTo(int type, float rain)
{
	switch (type) {
	case ATTRACTOR_ATM: return true;
	case ATTRACTOR_SEAT: return true;
	case ATTRACTOR_STOP: return true;
	case ATTRACTOR_PIZZA: return true;
	case ATTRACTOR_SHELTER: return rain >= 0.2f;
	case ATTRACTOR_ICECREAM: return false;
	}
	return false;
}

bool Civilian_AttractorSearchDue(float rain, bool attractorUnk, bool hasAttractor, int32_t *cycleState, int32_t frequency)
{
	// Only in the rain or with the flag, and not while using an attractor
	if (rain < 0.2f && !attractorUnk)
		return false;
	if (hasAttractor)
		return false;

	// Count calls until the search is due
	if (*cycleState != frequency) {
		(*cycleState)++;
		return false;
	}
	*cycleState = 0;
	return true;
}

bool Civilian_IsOnStealWishList(const uint32_t *list, int count, int32_t model)
{
	// Linear search of the wish list
	int i;
	for (i = 0; i < count; i++) {
		if ((uint32_t)model == list[i])
			return true;
	}
	return false;
}
//- rouz edit (Claude)
