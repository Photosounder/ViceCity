//+ rouz edit (Claude)
#include "PlaceableMath.h"

static void OrderPair(float *a, float *b)
{
	// Swap the bounds so the first is not greater than the second
	if (*a > *b) {
		float tmp = *a;
		*a = *b;
		*b = tmp;
	}
}

bool PlaceableMath_IsWithinArea2D(float px, float py, float x1, float y1, float x2, float y2)
{
	// Normalise the rectangle then test the point against it inclusively
	OrderPair(&x1, &x2);
	OrderPair(&y1, &y2);

	return x1 <= px && px <= x2 &&
	       y1 <= py && py <= y2;
}

bool PlaceableMath_IsWithinArea3D(float px, float py, float pz, float x1, float y1, float z1, float x2, float y2, float z2)
{
	// Normalise the box then test the point against it inclusively
	OrderPair(&x1, &x2);
	OrderPair(&y1, &y2);
	OrderPair(&z1, &z2);

	return x1 <= px && px <= x2 &&
	       y1 <= py && py <= y2 &&
	       z1 <= pz && pz <= z2;
}
//- rouz edit (Claude)
