//+ rouz edit (ChatGPT)
#include "WindModifiers.h"
#include "WindModifiers_backend.h"
#include "General.h"
#include <math.h>
#ifdef _MSC_VER
float fast_absf(float x);
#define WindModifiers_Abs fast_absf
#else
#define WindModifiers_Abs fabsf
#endif

static WindModifiersVector
WindModifiers_Subtract(WindModifiersVector left, WindModifiersVector right)
{
	// Retain all three original component subtractions
	WindModifiersVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}

static WindModifiersVector
WindModifiers_Multiply(WindModifiersVector left, float right)
{
	// Multiply each original vector component before division
	WindModifiersVector result = {left.x*right, left.y*right, left.z*right};
	return result;
}

static WindModifiersVector
WindModifiers_Divide(WindModifiersVector left, float right)
{
	// Divide each original multiplied component without folding the scale factor
	WindModifiersVector result = {left.x/right, left.y/right, left.z/right};
	return result;
}

static float
WindModifiers_Magnitude(WindModifiersVector vector)
{
	// Retain the original float sum and sqrtf wrapper
	return sqrtf(vector.x*vector.x + vector.y*vector.y + vector.z*vector.z);
}

static bool
WindModifiers_NearCamera(WindModifiersVector pos)
{
	// Preserve the single camera query within the original short-circuit operand
	return WindModifiers_Magnitude(WindModifiers_Subtract(pos, WindModifiers_BackendCamera())) < 100.0f;
}

#define MAX_HEIGHT_DIST 40.0f
#define MIN_FADE_DIST 20.0f
#define MAX_FADE_DIST 50.0f

static WindModifiersRecord Array[16];
int32_t WindModifiers_Number;

void
WindModifiers_RegisterOne(WindModifiersVector pos, int32_t type)
{
	// Retain capacity and camera-distance checks before copying the original record
	if (WindModifiers_Number < 16 && WindModifiers_NearCamera(pos)) {
		Array[WindModifiers_Number].m_pos = pos;
		Array[WindModifiers_Number].m_type = type;
		WindModifiers_Number++;
	}
}

bool
WindModifiers_FindWindModifier(WindModifiersVector pos, float *x, float *y)
{
	// Preserve the last matching direction, single random draw and ordered output updates
	bool bWasWindModifierFound = false;
	WindModifiersVector dir;
	for (int i = 0; i < WindModifiers_Number; i++) {
		if (Array[i].m_type == 1) {
			float zDist = WindModifiers_Abs(15.0f + pos.z - Array[i].m_pos.z);

			if (zDist < MAX_HEIGHT_DIST) {
				float dist = WindModifiers_Magnitude(WindModifiers_Subtract(pos, Array[i].m_pos));
				if (dist < MAX_FADE_DIST) {
					float distFade = dist < MIN_FADE_DIST ? 1.0f : 1.0f - (dist - MIN_FADE_DIST) / (MAX_FADE_DIST - MIN_FADE_DIST);
					float heightFade = 1.0f - zDist / MAX_HEIGHT_DIST;
					float fade = distFade * heightFade * 0.5f;
					dir = WindModifiers_Divide(WindModifiers_Multiply(WindModifiers_Subtract(pos, Array[i].m_pos), fade), dist);
					bWasWindModifierFound = true;
				}
			}
		}
	}

	if (bWasWindModifierFound) {
		float directionMult = ((General_GetRandomNumber() & 0x1F) - 16) * 0.0035f + 1.0f;
		*x += dir.x * directionMult;
		*y += dir.y * directionMult;
	}

	return bWasWindModifierFound;
}
//- rouz edit (ChatGPT)
