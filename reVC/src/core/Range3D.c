//+ rouz edit (ChatGPT)
#include "Range3D.h"
#include "General.h"
#include <math.h>
void Range3D_Init(Range3D *range,Range3DPoint min,Range3DPoint max)
{
    // Copy both original captured endpoint values in their constructor order
    range->min=min;range->max=max;
}
bool
Range3D_IsInRange(const Range3D *range,Range3DPoint vec)
{
    // Preserve the original strict component comparisons and short-circuit order
	return range->min.x < vec.x && range->max.x > vec.x && range->min.y < vec.y && range->max.y > vec.y && range->min.z < vec.z && range->max.z > vec.z;
}

void
Range3D_DebugShowRange(const Range3D *range,float value,int mode)
{
    // Preserve the original empty debug operation for every input
    (void)range;(void)value;(void)mode;
}

Range3DPoint
Range3D_GetRandomPointInRange(const Range3D *range)
{
    // Capture every original integer width before drawing coordinates from live minima
	int distX = fabsf(range->max.x - range->min.x);
	int distY = fabsf(range->max.y - range->min.y);
	int distZ = fabsf(range->max.z - range->min.z);

	float outX = General_GetRandomNumber() % distX + range->min.x;
	float outY = General_GetRandomNumber() % distY + range->min.y;
	float outZ = General_GetRandomNumber() % distZ + range->min.z;

	Range3DPoint result={outX, outY, outZ};
	return result;
}
//- rouz edit (ChatGPT)
