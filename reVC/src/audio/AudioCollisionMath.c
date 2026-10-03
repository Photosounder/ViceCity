//+ rouz edit (ChatGPT)
#include "AudioCollision.h"
#include "../core/SurfaceTypes.h"

float
AudioCollision_GetOneShotRatio(uint32_t a, float b)
{
	// Preserve the original collision ratio calculation and thresholds
	switch(a) {
	case SURFACE_DEFAULT:
	case SURFACE_TARMAC:
	case SURFACE_PAVEMENT:
	case SURFACE_STEEP_CLIFF:
	case SURFACE_TRANSPARENT_STONE:
	case SURFACE_CONCRETE_BEACH: return AudioCollision_GetRatio(b, 10.f, 60.f, 50.f);
	case SURFACE_GRASS:
	case SURFACE_GRAVEL:
	case SURFACE_MUD_DRY:
	case SURFACE_CARDBOARDBOX: return AudioCollision_GetRatio(b, 0.f, 2.f, 2.f);
	case SURFACE_CAR: return AudioCollision_GetRatio(b, 6.f, 50.f, 44.f);
	case SURFACE_GLASS:
	case SURFACE_METAL_CHAIN_FENCE: return AudioCollision_GetRatio(b, 0.1f, 10.f, 9.9f);
	case SURFACE_TRANSPARENT_CLOTH:
	case SURFACE_THICK_METAL_PLATE: return AudioCollision_GetRatio(b, 30.f, 130.f, 100.f);
	case SURFACE_GARAGE_DOOR: return AudioCollision_GetRatio(b, 20.f, 100.f, 80.f);
	case SURFACE_CAR_PANEL: return AudioCollision_GetRatio(b, 0.f, 4.f, 4.f);
	case SURFACE_SCAFFOLD_POLE:
	case SURFACE_METAL_GATE: 
	case SURFACE_LAMP_POST: return AudioCollision_GetRatio(b, 1.f, 10.f, 9.f);
	case SURFACE_FIRE_HYDRANT: return AudioCollision_GetRatio(b, 1.f, 15.f, 14.f);
	case SURFACE_GIRDER: return AudioCollision_GetRatio(b, 8.f, 50.f, 42.f);
	case SURFACE_PED: return AudioCollision_GetRatio(b, 0.f, 20.f, 20.f);
	case SURFACE_SAND:
	case SURFACE_WATER:
	case SURFACE_RUBBER:
	case SURFACE_WHEELBASE:
	case SURFACE_SAND_BEACH: return AudioCollision_GetRatio(b, 0.f, 10.f, 10.f);
	case SURFACE_WOOD_CRATES: return AudioCollision_GetRatio(b, 1.f, 4.f, 3.f);
	case SURFACE_WOOD_BENCH: return AudioCollision_GetRatio(b, 0.1f, 5.f, 4.9f);
	case SURFACE_WOOD_SOLID: return AudioCollision_GetRatio(b, 0.1f, 40.f, 39.9f);
	case SURFACE_PLASTIC: return AudioCollision_GetRatio(b, 0.1f, 4.f, 3.9f);
	case SURFACE_HEDGE: return AudioCollision_GetRatio(b, 0.f, 0.5f, 0.5f);
	case SURFACE_CONTAINER: return AudioCollision_GetRatio(b, 4.f, 40.f, 36.f);
	case SURFACE_NEWS_VENDOR: return AudioCollision_GetRatio(b, 0.f, 5.f, 5.f);
	default: break;
	}

	return 0.f;
}

float
AudioCollision_GetLoopingRatio(uint32_t a, uint32_t b, float c)
{
	// Preserve the original collision ratio calculation and thresholds
	return AudioCollision_GetRatio(c, 0.0f, 0.02f, 0.02f);
}

float
AudioCollision_GetRatio(float a, float b, float c, float d)
{
	// Preserve the original collision ratio calculation and thresholds
	float e;
	e = a;
	if(a <= b) return 0.0f;
	if(c <= a) e = c;
	return (e - b) / d;
}
//- rouz edit (ChatGPT)
