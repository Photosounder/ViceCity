//+ rouz edit (Claude)
#include <stddef.h>
#include "BikeLogic.h"
#include <math.h>
#include <string.h>
#include "WheelState.h"
#include "MatrixMath.h"
#include "VectorMath.h"

bool Bike_IsComponentPresent(const CBikeData *self, int component)
{
	// The component exists when its frame does
	return self->m_aBikeNodes[component] != NULL;
}

void Bike_ResetSuspension(CBikeData *self)
{
	int i;

	// Stopped wheels
	for (i = 0; i < 2; i++) {
		self->m_aWheelRotation[i] = 0.0f;
		self->m_aWheelState[i] = WHEEL_STATE_NORMAL;
	}

	// Fully extended springs
	for (i = 0; i < 4; i++) {
		self->m_aSuspensionSpringRatio[i] = 1.0f;
		self->m_aWheelTimer[i] = 0.0f;
	}
}

void Bike_ClearNodes(CBikeData *self)
{
	// Forget all the frames
	int i;
	for (i = 0; i < BIKE_NUM_NODES; i++)
		self->m_aBikeNodes[i] = NULL;
}

void Bike_RemoveRefsToVehicle(CBikeData *self, const CEntity *ent)
{
	// Forget the ground entity if it is the removed one
	int i;
	for (i = 0; i < 4; i++)
		if ((const void*)self->m_aGroundPhysical[i] == (const void*)ent)
			self->m_aGroundPhysical[i] = NULL;
}
void Bike_CalculateLeanMatrix(CMatrixData *leanMatrix, float leanAngle, const float *up, float minZ)
{
	float mat[4][4];
	float result[4][4];

	// Roll forwards a little with the lean and lean sideways
	MatrixMath_SetRotateXOnly(mat, -0.05f*fabsf(leanAngle));
	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f;
	mat[3][2] = 0.0f;
	MatrixMath_RotateY(mat, leanAngle);

	// Apply it to the matrix of the bike
	MatrixMath_Multiply(result, (const float (*)[4])leanMatrix->f, (const float (*)[4])mat);
	memcpy(leanMatrix->f, result, sizeof(result));

	// Place the wheel back on the ground
	leanMatrix->f[3][0] += up[0]*(1.0f-cosf(leanAngle))*minZ;
	leanMatrix->f[3][1] += up[1]*(1.0f-cosf(leanAngle))*minZ;
	leanMatrix->f[3][2] += up[2]*(1.0f-cosf(leanAngle))*minZ;
}
void Bike_GetCorrectedWorldDoorPosition(const float *forward, const float *up, const float *position, float maxX, float maxZ, const float *p1, const float *p2, float *out)
{
	VectorMath3 fwd = {forward[0], forward[1], forward[2]};
	VectorMath3 worldUp = {0.0f, 0.0f, 1.0f};
	VectorMath3 rightWorld = VectorMath_Cross(fwd, worldUp);
	VectorMath3 upWorld = VectorMath_Cross(rightWorld, fwd);
	float onSide = up[0]*rightWorld.x + up[1]*rightWorld.y + up[2]*rightWorld.z;
	float diff = maxZ - maxX;
	float x, y, z;

	// A leaning bike moves the point sideways by the difference of the box sizes
	diff = diff > 0.0f ? diff : 0.0f;
	y = p2[1] - p1[1];
	x = onSide*diff + p2[0] + p1[0];
	z = p2[2] - p1[2];
	out[0] = x*rightWorld.x + y*fwd.x + z*upWorld.x + position[0];
	out[1] = x*rightWorld.y + y*fwd.y + z*upWorld.y + position[1];
	out[2] = x*rightWorld.z + y*fwd.z + z*upWorld.z + position[2];
}
//- rouz edit (Claude)
