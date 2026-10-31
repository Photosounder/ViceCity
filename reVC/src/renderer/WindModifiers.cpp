//+ rouz edit (ChatGPT)
#include "common.h"
#include "Camera.h"
#include "WindModifiers_backend.h"

WindModifiersVector
WindModifiers_BackendVector(const void *position)
{
	// Copy the original typed position once without assuming a CVector layout
	const CVector *vector = (const CVector *)position;
	WindModifiersVector result = {vector->x, vector->y, vector->z};
	return result;
}

WindModifiersVector
WindModifiers_BackendCamera(void)
{
	// Query the live camera position only after the original capacity check
	return WindModifiers_BackendVector(&TheCamera.GetPosition());
}
//- rouz edit (ChatGPT)
