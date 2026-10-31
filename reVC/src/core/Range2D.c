//+ rouz edit (ChatGPT)
#include "Range2D.h"
#include "General.h"
#include <math.h>
void Range2D_Init(Range2D *range,Range2DPoint min,Range2DPoint max)
{
    // Copy both original captured endpoint values in their constructor order
    range->min=min;range->max=max;
}
bool
Range2D_IsInRange(const Range2D *range,Range2DPoint vec)
{
    // Preserve the original strict component comparisons and short-circuit order
	return range->min.x < vec.x && range->max.x > vec.x && range->min.y < vec.y && range->max.y > vec.y;
}

void
Range2D_DebugShowRange(const Range2D *range,float value,int mode)
{
    // Preserve the original empty debug operation for every input
    (void)range;(void)value;(void)mode;
}

Range2DPoint
Range2D_GetRandomPointInRange(const Range2D *range)
{
    // Capture every original integer width before drawing coordinates from live minima
	int distX = fabsf(range->max.x - range->min.x);
	int distY = fabsf(range->max.y - range->min.y);

	float outX = General_GetRandomNumber() % distX + range->min.x;
	float outY = General_GetRandomNumber() % distY + range->min.y;

	Range2DPoint result={outX, outY};
	return result;
}
//- rouz edit (ChatGPT)
