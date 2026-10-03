#pragma once

#include "SurfaceTypes.h" // rouz edit (ChatGPT)

struct CColPoint;

inline bool
IsSeeThrough(uint8 surfType)
{
	switch(surfType)
	case SURFACE_GLASS:
	case SURFACE_TRANSPARENT_CLOTH:
	case SURFACE_METAL_CHAIN_FENCE:
	case SURFACE_TRANSPARENT_STONE:
	case SURFACE_SCAFFOLD_POLE:
		return true;
	return false;
}

// I think the necessity of this function is really a bug
inline bool
IsSeeThroughVertical(uint8 surfType)
{
	switch(surfType)
	case SURFACE_GLASS:
	case SURFACE_TRANSPARENT_CLOTH:
		return true;
	return false;
}

inline bool
IsShootThrough(uint8 surfType)
{
	switch(surfType)
	case SURFACE_TRANSPARENT_CLOTH:
	case SURFACE_METAL_CHAIN_FENCE:
	case SURFACE_TRANSPARENT_STONE:
	case SURFACE_SCAFFOLD_POLE:
		return true;
	return false;
}

class CSurfaceTable
{
	static float ms_aAdhesiveLimitTable[NUMADHESIVEGROUPS][NUMADHESIVEGROUPS];
public:
	static void Initialise(Const char *filename);
	static int GetAdhesionGroup(uint8 surfaceType);
	static float GetWetMultiplier(uint8 surfaceType);
	static float GetAdhesiveLimit(CColPoint &colpoint);
	static bool IsSoftLanding(uint8 surf);
};
