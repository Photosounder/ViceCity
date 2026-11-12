//+ rouz edit (Claude)
#include <math.h>
#include "CopLogic.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"

// Scale a ground plane vector to unit length, the x axis for a null vector
static void Normalise2D(float *x, float *y)
{
	float sq = *x * *x + *y * *y;
	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		*x *= invsqrt;
		*y *= invsqrt;
	} else
		*x = 1.0f;
}

bool CopLogic_ShouldDeployStinger(float vehDistX, float vehDistY, float dirVehGoingX, float dirVehGoingY, float forwardHeading, float *angle)
{
	float diffX, diffY;

	// Only within 30 units and when the vehicle moves
	if (!(vehDistX * vehDistX + vehDistY * vehDistY < 30.0f * 30.0f))
		return false;
	if (!(dirVehGoingX * dirVehGoingX + dirVehGoingY * dirVehGoingY > 0.0f))
		return false;

	Normalise2D(&vehDistX, &vehDistY);
	Normalise2D(&dirVehGoingX, &dirVehGoingY);

	// The vehicle has to come towards the cop
	if (!(vehDistX * dirVehGoingX + vehDistY * dirVehGoingY > 0.8f))
		return false;

	// Face to the side the vehicle is heading
	diffX = dirVehGoingX - vehDistX;
	diffY = dirVehGoingY - vehDistY;
	*angle = (vehDistX * diffY - vehDistY * diffX < 0.0f ?
		forwardHeading - HALFPI :
		HALFPI + forwardHeading);
	return true;
}
//- rouz edit (Claude)
