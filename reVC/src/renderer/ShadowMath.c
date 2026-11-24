//+ rouz edit (Claude)
#include <math.h>
#include "ShadowMath.h"

void ShadowMath_CalcPedShadowValues(float lightX, float lightY, float *frontX, float *frontY, float *sideX, float *sideY, float *displacementX, float *displacementY)
{
	float dist, mult;

	// The front points away from the light and is stretched by one unit
	*frontX = -lightX;
	*frontY = -lightY;

	dist = sqrtf(*frontY * *frontY + *frontX * *frontX);
	mult = (dist + 1.0f) / dist;

	*frontX *= mult;
	*frontY *= mult;

	// The side is perpendicular to the light
	*sideX = -lightY / dist;
	*sideY = lightX / dist;

	*displacementX = -lightX;
	*displacementY = -lightY;

	// Everything is half the size
	*frontX /= 2;
	*frontY /= 2;

	*sideX /= 2;
	*sideY /= 2;

	*displacementX /= 2;
	*displacementY /= 2;
}

static float cross2d(float ax, float ay, float bx, float by)
{
	return ax*by - ay*bx;
}

#define absf fabsf

void ShadowMath_ProjectQuad(float frontX, float frontY, float sideX, float sideY, float rightX, float rightY, float forwardX, float forwardY, float relX, float relY,
	VectorMath3 points[4], float *minX, float *maxX, float *minY, float *maxY)
{
	float fFrontRight    = frontX*rightX + frontY*rightY;
	float fFrontForward  = frontX*forwardX + frontY*forwardY;
	float fSideRight     = sideX*rightX + sideY*rightY;
	float fSideForward   = sideX*forwardX + sideY*forwardY;
	float fLengthRight   = relX*rightX + relY*rightY;
	float fLengthForward = relX*forwardX + relY*forwardY;

	points[0].x = (fLengthRight   + fFrontRight  )  - fSideRight;
	points[0].y = (fLengthForward + fFrontForward)  - fSideForward;

	points[1].x = fSideRight      + (fLengthRight   + fFrontRight);
	points[1].y = fSideForward    + (fLengthForward + fFrontForward);

	points[2].x = fSideRight      + (fLengthRight   - fFrontRight);
	points[2].y = fSideForward    + (fLengthForward - fFrontForward);

	points[3].x = (fLengthRight   - fFrontRight)    - fSideRight;
	points[3].y = (fLengthForward - fFrontForward)  - fSideForward;

	// Box around the four corners
#define SM_MIN(a,b) ((a) < (b) ? (a) : (b))
#define SM_MAX(a,b) ((a) > (b) ? (a) : (b))
	*minX = SM_MIN(SM_MIN(points[0].x, points[1].x), SM_MIN(points[2].x, points[3].x));
	*maxX = SM_MAX(SM_MAX(points[0].x, points[1].x), SM_MAX(points[2].x, points[3].x));

	*minY = SM_MIN(SM_MIN(points[0].y, points[1].y), SM_MIN(points[2].y, points[3].y));
	*maxY = SM_MAX(SM_MAX(points[0].y, points[1].y), SM_MAX(points[2].y, points[3].y));
#undef SM_MIN
#undef SM_MAX
}

// Clips the polygon of numIn vertices at inBase to the side of the line through start with direction dist that has a positive cross product, the result goes to outBase
static int32_t clipToEdge(VectorMath3 *List, VectorMath3 *Texture, int32_t inBase, int32_t numIn, int32_t outBase, float startX, float startY, float distX, float distY)
{
	int32_t numOut = 0;
	int16_t vertType = 0;

	for ( int32_t j = 0; j < numIn; j++ )
	{
		int32_t in = inBase + j;

		float cp = cross2d(List[in].x - startX, List[in].y - startY, distX, distY);

		if ( cp > 0.0f )
		{
			switch ( vertType )
			{
				case 0:
				case 1:
				{
					int32_t out = numOut++ + outBase;

					Texture[out].x = Texture[in].x;
					Texture[out].y = Texture[in].y;
					List[out].x    = List[in].x;
					List[out].y    = List[in].y;

					break;
				}

				case 2:
				{
					float prevcp = cross2d(List[in-1].x - startX, List[in-1].y - startY, distX, distY);

					float Scale = absf(prevcp) / (absf(prevcp) + absf(cp));
					float Compl = 1.0f - Scale;

					int32_t out1 = numOut++ + outBase;
					int32_t out2 = numOut++ + outBase;

					Texture[out1].x = Compl*Texture[in-1].x + Scale*Texture[in].x;
					Texture[out1].y = Compl*Texture[in-1].y + Scale*Texture[in].y;
					List[out1].x    = Compl*List[in-1].x    + Scale*List[in].x;
					List[out1].y    = Compl*List[in-1].y    + Scale*List[in].y;

					Texture[out2].x = Texture[in].x;
					Texture[out2].y = Texture[in].y;
					List[out2].x    = List[in].x;
					List[out2].y    = List[in].y;

					break;
				}
			}

			vertType = 1;
		}
		else
		{
			switch ( vertType )
			{
				case 1:
				{
					float prevcp = cross2d(List[in-1].x - startX, List[in-1].y - startY, distX, distY);

					float Scale = absf(prevcp) / (absf(prevcp) + absf(cp));
					float Compl = 1.0f - Scale;

					int32_t out = numOut++ + outBase;

					Texture[out].x = Compl*Texture[in-1].x + Scale*Texture[in].x;
					Texture[out].y = Compl*Texture[in-1].y + Scale*Texture[in].y;
					List[out].x    = Compl*List[in-1].x    + Scale*List[in].x;
					List[out].y    = Compl*List[in-1].y    + Scale*List[in].y;

					break;
				}
			}

			vertType = 2;
		}
	}

	// The edge between the last and the first vertex
	float cp1 = cross2d(List[inBase].x - startX, List[inBase].y - startY, distX, distY);
	if ( (cp1 > 0.0f && vertType == 2) || (cp1 <= 0.0f && vertType == 1) )
	{
		int32_t in = inBase + numIn;

		float cp2 = cross2d(List[in-1].x - startX, List[in-1].y - startY, distX, distY);

		float Scale = absf(cp2) / (absf(cp2) + absf(cp1));
		float Compl = 1.0f - Scale;

		int32_t out = numOut++ + outBase;

		Texture[out].x = Compl*Texture[in-1].x + Scale*Texture[inBase].x;
		Texture[out].y = Compl*Texture[in-1].y + Scale*Texture[inBase].y;
		List[out].x    = Compl*List[in-1].x    + Scale*List[inBase].x;
		List[out].y    = Compl*List[in-1].y    + Scale*List[inBase].y;
	}

	return numOut;
}

int32_t ShadowMath_ClipQuadToTriangle(VectorMath3 *list, VectorMath3 *texture, const VectorMath3 *points, VectorMath3 pointA, VectorMath3 pointB, VectorMath3 pointC)
{
	// The quad with its texture coordinates
	list[0].x = points[0].x;
	list[0].y = points[0].y;

	list[1].x = points[1].x;
	list[1].y = points[1].y;

	list[2].x = points[2].x;
	list[2].y = points[2].y;

	list[3].x = points[3].x;
	list[3].y = points[3].y;

	texture[0].x = 0.0f;
	texture[0].y = 0.0f;

	texture[1].x = 1.0f;
	texture[1].y = 0.0f;

	texture[2].x = 1.0f;
	texture[2].y = 1.0f;

	texture[3].x = 0.0f;
	texture[3].y = 1.0f;

	// Clip to the three edges of the triangle, alternating between the two halves of the lists
	int32_t numVerts1 = clipToEdge(list, texture, 0, 4, 10, pointA.x, pointA.y, pointB.x - pointA.x, pointB.y - pointA.y);
	int32_t numVerts2 = clipToEdge(list, texture, 10, numVerts1, 0, pointB.x, pointB.y, pointC.x - pointB.x, pointC.y - pointB.y);
	return clipToEdge(list, texture, 0, numVerts2, 10, pointC.x, pointC.y, pointA.x - pointC.x, pointA.y - pointC.y);
}

void ShadowMath_ClippedToWorld(VectorMath3 *list, int32_t numVerts, VectorMath3 normal, VectorMath3 pointA, float forwardX, float forwardY, float rightX, float rightY, VectorMath3 entityPos)
{
	float dot = normal.x*pointA.x + normal.y*pointA.y + normal.z*pointA.z;

	// Height of each vertex on the plane of the triangle
	for ( int32_t j = 0; j < numVerts; j++ )
	{
		int32_t idx = j + 10;

		list[idx].z = -((normal.x*list[idx].x + normal.y*list[idx].y) - dot) / normal.z;
	}

	// From the space of the entity to the world
	for ( int32_t j = 0; j < numVerts; j++ )
	{
		int32_t idx = j + 10;

		VectorMath3 p = list[idx];

		list[idx].x = p.y * forwardX + p.x * rightX + entityPos.x;
		list[idx].y = p.y * forwardY + p.x * rightY + entityPos.y;
		list[idx].z = p.z + entityPos.z;
	}
}
int32_t ShadowMath_FadedStrength(float distToCam, float drawDistance, int32_t shadowStrength)
{
	float fMult = 1.0f - (4.0f / drawDistance) * (distToCam - (drawDistance * (1.0f / 4.0f)));

	if (distToCam >= (drawDistance * (1.0f / 4.0f)))
		return (int32_t)(shadowStrength * fMult);
	return shadowStrength;
}

int32_t ShadowMath_CarFadedStrength(float distToCam, float drawDistance, int32_t shadowStrength)
{
	float fMult = 1.0f - (distToCam - (drawDistance * (1.0f - (1.0f / 4.0f)))) / (drawDistance * (1.0f / 4.0f));

	if (distToCam >= (drawDistance * (1.0f - (1.0f / 4.0f))))
		return (int32_t)(fMult * shadowStrength);
	return shadowStrength;
}
void ShadowMath_ShadowExtent(float posX, float posY, float frontX, float frontY, float sideX, float sideY, float *startX, float *endX, float *startY, float *endY)
{
	float fWidth = fabsf(frontX) + fabsf(sideX);
	float fHeight = fabsf(frontY) + fabsf(sideY);

	*startX = posX - fWidth;
	*endX = posX + fWidth;
	*startY = posY - fHeight;
	*endY = posY + fHeight;
}
float ShadowMath_BikeWidthMultiplier(float leanAngle, bool isPhysics, float rightZ)
{
	float wheelZ = fabsf(leanAngle);
	float mul = 5.092958f * wheelZ + 1.0f;

	if (isPhysics) {
		float z = rightZ;
		if (z > 0.6f)
			mul += 4.0f * z;
	}
	return mul;
}
void ShadowMath_VehicleAxes(bool isBike, bool isHeli, const float forward[3], const float right[3], const float up[3], float front[2], float side[2])
{
	front[0] = forward[0];
	front[1] = forward[1];
	side[0] = right[0];
	side[1] = right[1];
	if (isBike) {
		if (fabsf(right[2]) > 0.6f) {
			side[0] = up[0];
			side[1] = up[1];
		}
	} else if (isHeli) {
		if (fabsf(right[2]) > 0.57f) {
			side[0] = up[0];
			side[1] = up[1];
		}
		if (fabsf(forward[2]) > 0.57f) {
			front[0] = up[0];
			front[1] = up[1];
		}
	}
}
float ShadowMath_VehicleDrawDistance(bool isFlyer)
{
	return isFlyer ? 144.0f : 18.0f;
}

void ShadowMath_ModelAdjust(int kind, float *height, float *width, float *size)
{
	switch (kind) {
	case SHADOWMODEL_MOPED:
		*height *= 1.2f;
		*size = 0.05f;
		break;
	case SHADOWMODEL_BIKE:
		*height *= 1.5f;
		*size *= 0.03f;
		break;
	case SHADOWMODEL_HELI:
		*width *= 3.0f;
		*height *= 1.4f;
		*size *= 0.5f;
		break;
	case SHADOWMODEL_RCCOPTER:
		*height *= 1.5f;
		*width *= 2.0f;
		*size *= 0.2f;
		break;
	case SHADOWMODEL_DODO:
		*height *= 0.9f;
		*width *= 0.4f;
		break;
	}
}
void ShadowMath_FadeStaticShadow(float distToCamSqr, float drawDistance, int32_t *intensity, int32_t *red, int32_t *green, int32_t *blue)
{
	if (drawDistance != 0.0f) {
		float distToCam = sqrtf(distToCamSqr);

		if (distToCam >= (drawDistance * (1.0f - (1.0f / 4.0f)))) {
			float fMult = 1.0f - (4.0f / drawDistance) * (distToCam - (drawDistance * (1.0f - (1.0f / 4.0f))));

			*intensity = (int32_t)(*intensity * fMult);
			*red = (int32_t)(*red * fMult);
			*green = (int32_t)(*green * fMult);
			*blue = (int32_t)(*blue * fMult);
		}
	}
}
void ShadowMath_FadeHeadlightShadow(float distToCam, float drawDistance, int32_t *red, int32_t *green, int32_t *blue)
{
	float fMult = 1.0f - (3.0f / drawDistance) * (distToCam - (drawDistance * (1.0f - (1.0f / 3.0f))));

	*red = (int32_t)(*red * fMult);
	*green = (int32_t)(*green * fMult);
	*blue = (int32_t)(*blue * fMult);
}
bool ShadowMath_StaticShadowIsNear(float posX, float posY, float storedX, float storedY, float upDistance)
{
	return fabsf(posX - storedX) < upDistance && fabsf(posY - storedY) < upDistance;
}

bool ShadowMath_StaticShadowIsSamePlace(float posX, float posY, float posZ, float storedX, float storedY, float storedZ)
{
	return fabsf(posX - storedX) < 0.05f && fabsf(posY - storedY) < 0.05f && fabsf(posZ - storedZ) < 2.0f;
}
bool ShadowMath_SameGeometry(const float a[7], const float b[7])
{
	int i;

	for (i = 0; i < 7; i++)
		if (!(fabsf(a[i] - b[i]) < 0.01f))
			return false;
	return true;
}
//- rouz edit (Claude)
