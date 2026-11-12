#include "common.h"
#include "Placeable.h"


CPlaceable::CPlaceable(void)
{
	// Delegate to the C implementation
	Placeable_Init(this);
}

void
CPlaceable::SetHeading(float angle)
{
	// Delegate to the C implementation
	Placeable_SetHeading(this, angle);
}

bool
CPlaceable::IsWithinArea(float x1, float y1, float x2, float y2)
{
	// Delegate to the C implementation
	return Placeable_IsWithinArea2D(this, x1, y1, x2, y2);
}

bool
CPlaceable::IsWithinArea(float x1, float y1, float z1, float x2, float y2, float z2)
{
	// Delegate to the C implementation
	return Placeable_IsWithinArea3D(this, x1, y1, z1, x2, y2, z2);
}
