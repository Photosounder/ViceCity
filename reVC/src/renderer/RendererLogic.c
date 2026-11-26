//+ rouz edit (Claude)
#include <math.h>
#include "RendererLogic.h"
#include "Vector.h" // rouz edit (Claude)

static float calcNewDelta(const RendererV2 *a, const RendererV2 *b)
{
	return (b->x - a->x) / (b->y - a->y);
}

// The fixed build rounds down, the original cut off the fraction
#define TOINT(x) ((int)floorf(x))

void RendererLogic_ScanSectorPoly(const RendererV2 *poly, int32_t numVertices, int32_t numSectorsX, int32_t numSectorsY, void (*scan)(int32_t x, int32_t y, void *context), void *context)
{
	float miny, maxy;
	int y, yend;
	int x, xstart, xend;
	int i;
	int a1, a2, b1, b2;
	float deltaA, deltaB;
	float xA, xB;

	miny = poly[0].y;
	maxy = poly[0].y;
	a2 = 0;
	xstart = 9999;
	xend = -9999;

	for(i = 1; i < numVertices; i++){
		if(poly[i].y > maxy)
			maxy = poly[i].y;
		if(poly[i].y < miny){
			miny = poly[i].y;
			a2 = i;
		}
	}
	y = TOINT(miny);
	yend = TOINT(maxy);

	// Go left in poly to find first edge b
	b2 = a2;
	for(i = 0; i < numVertices; i++){
		b1 = b2--;
		if(b2 < 0) b2 = numVertices-1;
		if(poly[b1].x < xstart)
			xstart = TOINT(poly[b1].x);
		if(TOINT(poly[b1].y) != TOINT(poly[b2].y))
			break;
	}
	// Go right to find first edge a
	for(i = 0; i < numVertices; i++){
		a1 = a2++;
		if(a2 == numVertices) a2 = 0;
		if(poly[a1].x > xend)
			xend = TOINT(poly[a1].x);
		if(TOINT(poly[a1].y) != TOINT(poly[a2].y))
			break;
	}

	// prestep x1 and x2 to next integer y
	deltaA = calcNewDelta(&poly[a1], &poly[a2]);
	xA = deltaA * (ceilf(poly[a1].y) - poly[a1].y) + poly[a1].x;
	deltaB = calcNewDelta(&poly[b1], &poly[b2]);
	xB = deltaB * (ceilf(poly[b1].y) - poly[b1].y) + poly[b1].x;

	if(y != yend){
		if(deltaB < 0.0f && TOINT(xB) < xstart)
			xstart = TOINT(xB);
		if(deltaA >= 0.0f && TOINT(xA) > xend)
			xend = TOINT(xA);
	}

	while(y <= yend && y < numSectorsY){
		// scan one x-line
		if(y >= 0 && xstart < numSectorsX)
			for(x = xstart; x <= xend && x != numSectorsX; x++)
				if(x >= 0)
					scan(x, y, context);

		// advance one scan line
		y++;
		xA += deltaA;
		xB += deltaB;

		// update left side
		if(y == TOINT(poly[b2].y)){
			// reached end of edge
			if(y == yend){
				if(deltaB < 0.0f){
					do{
						xstart = TOINT(poly[b2--].x);
						if(b2 < 0) b2 = numVertices-1;
					}while(xstart > TOINT(poly[b2].x));
				}else
					xstart = TOINT(xB - deltaB);
			}else{
				// switch edges
				if(deltaB < 0.0f)
					xstart = TOINT(poly[b2].x);
				else
					xstart = TOINT(xB - deltaB);
				do{
					b1 = b2--;
					if(b2 < 0) b2 = numVertices-1;
					if(TOINT(poly[b1].x) < xstart)
						xstart = TOINT(poly[b1].x);
				}while(y == TOINT(poly[b2].y));
				deltaB = calcNewDelta(&poly[b1], &poly[b2]);
				xB = deltaB * (ceilf(poly[b1].y) - poly[b1].y) + poly[b1].x;
				if(deltaB < 0.0f && TOINT(xB) < xstart)
					xstart = TOINT(xB);
			}
		}else{
			if(deltaB < 0.0f)
				xstart = TOINT(xB);
			else
				xstart = TOINT(xB - deltaB);
		}

		// update right side
		if(y == TOINT(poly[a2].y)){
			// reached end of edge
			if(y == yend){
				if(deltaA < 0.0f)
					xend = TOINT(xA - deltaA);
				else{
					do{
						xend = TOINT(poly[a2++].x);
						if(a2 == numVertices) a2 = 0;
					}while(xend < TOINT(poly[a2].x));
				}
			}else{
				// switch edges
				if(deltaA < 0.0f)
					xend = TOINT(xA - deltaA);
				else
					xend = TOINT(poly[a2].x);
				do{
					a1 = a2++;
					if(a2 == numVertices) a2 = 0;
					if(TOINT(poly[a1].x) > xend)
						xend = TOINT(poly[a1].x);
				}while(y == TOINT(poly[a2].y));
				deltaA = calcNewDelta(&poly[a1], &poly[a2]);
				xA = deltaA * (ceilf(poly[a1].y) - poly[a1].y) + poly[a1].x;
				if(deltaA >= 0.0f && TOINT(xA) > xend)
					xend = TOINT(xA);
			}
		}else{
			if(deltaA < 0.0f)
				xend = TOINT(xA - deltaA);
			else
				xend = TOINT(xA);
		}
	}
}
uint32_t RendererLogic_DistanceFadeAlpha(uint8_t alpha, float largestDistance, float camDist, float fadeDistance)
{
	float fadefactor = (largestDistance - (camDist - fadeDistance))/fadeDistance;
	if(fadefactor > 1.0f)
		fadefactor = 1.0f;
	return alpha * fadefactor;
}

bool RendererLogic_WithinStreamDistance(float dist, float largestDistance, float streamDistance)
{
	return dist - streamDistance < largestDistance;
}

void Occluder_BuildBox(const float pos[3], const float cam[3], float length, float width, float height, float angle, float corners[8][3], bool frontFace[6])
{
	float vec[3][3];
	int i, k;
	// Signs of the three axes for each corner
	static const int signs[8][3] = {
		{ 1, 1, 1 }, { -1, 1, 1 }, { 1, -1, 1 }, { -1, -1, 1 },
		{ 1, 1, -1 }, { -1, 1, -1 }, { 1, -1, -1 }, { -1, -1, -1 }
	};

	vec[0][0] = length / 2.0f * sinf(angle);
	vec[0][1] = -length / 2.0f * cosf(angle);
	vec[0][2] = 0.0f;
	vec[1][0] = width / 2.0f * cosf(angle);
	vec[1][1] = width / 2.0f * sinf(angle);
	vec[1][2] = 0.0f;
	vec[2][0] = 0.0f;
	vec[2][1] = 0.0f;
	vec[2][2] = height / 2.0f;

	// Figure out if the front or the back of a face is seen
	for (i = 0; i < 3; i++) {
		float dotPlus = (pos[0] + vec[i][0] - cam[0]) * vec[i][0] + (pos[1] + vec[i][1] - cam[1]) * vec[i][1] + (pos[2] + vec[i][2] - cam[2]) * vec[i][2];
		float dotMinus = (pos[0] - vec[i][0] - cam[0]) * -vec[i][0] + (pos[1] - vec[i][1] - cam[1]) * -vec[i][1] + (pos[2] - vec[i][2] - cam[2]) * -vec[i][2];

		frontFace[i * 2 + 0] = dotPlus < 0.0f;
		frontFace[i * 2 + 1] = dotMinus < 0.0f;
	}

	// Corners of the box
	for (i = 0; i < 8; i++) {
		for (k = 0; k < 3; k++) {
			float v = pos[k];

			v = signs[i][0] > 0 ? v + vec[0][k] : v - vec[0][k];
			v = signs[i][1] > 0 ? v + vec[1][k] : v - vec[1][k];
			v = signs[i][2] > 0 ? v + vec[2][k] : v - vec[2][k];
			corners[i][k] = v;
		}
	}
}
bool Occlusion_CrossesLine(float p1X, float p1Y, float p2X, float p2Y, float lineX, float lineY, float lineDX, float lineDY)
{
	float side1 = (p1X - lineX) * lineDY - (p1Y - lineY) * lineDX;
	float side2 = (p2X - lineX) * lineDY - (p2Y - lineY) * lineDX;

	return side1 * side2 < 0.0f;
}

bool Occlusion_LineTouchesScreen(float lineX, float lineY, float lineDX, float lineDY, float screenWidth, float screenHeight)
{
	if (lineX > 0.0f && lineY > 0.0f && screenWidth > lineX && screenHeight > lineY)
		return true;

	return Occlusion_CrossesLine(0.0f, 0.0f, screenWidth, 0.0f, lineX, lineY, lineDX, lineDY) ||
		Occlusion_CrossesLine(0.0f, 0.0f, 0.0f, screenHeight, lineX, lineY, lineDX, lineDY) ||
		Occlusion_CrossesLine(screenWidth, 0.0f, screenWidth, screenHeight, lineX, lineY, lineDX, lineDY) ||
		Occlusion_CrossesLine(0.0f, screenHeight, screenWidth, screenHeight, lineX, lineY, lineDX, lineDY);
}

bool Occlusion_IsPointInsideLine(float lineX, float lineY, float lineDX, float lineDY, float pX, float pY, float area)
{
	return (pX - lineX) * lineDY - (pY - lineY) * lineDX >= area;
}
float Occlusion_BoxSide(float width, float length, float height)
{
	return sqrtf(width * width + length * length + height * height) / 4.0f;
}

float Occlusion_TestArea(float screenW, float screenH, float side)
{
	return (screenW > screenH ? screenW : screenH) * side;
}

bool Occlusion_OccluderInFront(float depth, float side, float radius)
{
	return depth - (side * 0.85f) > radius;
}
bool Visibility_CompCullCheckNeeded(float distToCameraSq, float cullCompsDist, bool noCull, float pitchToCamera)
{
	return distToCameraSq > cullCompsDist && !noCull && pitchToCamera < 0.2f;
}

bool Visibility_CompIsHidden(float dot, bool angleCull, float distToCameraSq)
{
	return dot > 0.0f && (angleCull || 0.1f * distToCameraSq < dot * dot);
}
float Visibility_DotWithCameraVector(const CVector *atomicPos, const CVector *cameraPos, const CVector *clumpForward, const CVector *clumpRight, unsigned int flags)
{
	/* Flags from ATOMIC_FLAG_* */
	const unsigned int LEFT = 0x4, RIGHT = 0x8, FRONT = 0x10, REAR = 0x20, REARDOOR = 0x200, FRONTDOOR = 0x400;
	float dist[3], dot, dotdoor;

	dist[0] = atomicPos->x - cameraPos->x;
	dist[1] = atomicPos->y - cameraPos->y;
	dist[2] = atomicPos->z - cameraPos->z;
	/* Forward/backward facing */
	if (flags & (FRONT | REAR))
		dot = dist[0] * clumpForward->x + dist[1] * clumpForward->y + dist[2] * clumpForward->z;
	/* Left/right facing */
	else if (flags & (LEFT | RIGHT))
		dot = dist[0] * clumpRight->x + dist[1] * clumpRight->y + dist[2] * clumpRight->z;
	else
		dot = 0.0f;
	if (flags & (LEFT | REAR))
		dot = -dot;

	if (flags & (REARDOOR | FRONTDOOR)) {
		if (flags & REARDOOR)
			dotdoor = -(dist[0] * clumpForward->x + dist[1] * clumpForward->y + dist[2] * clumpForward->z);
		else if (flags & FRONTDOOR)
			dotdoor = dist[0] * clumpForward->x + dist[1] * clumpForward->y + dist[2] * clumpForward->z;
		else
			dotdoor = 0.0f;

		if (dot < 0.0f && dotdoor < 0.0f)
			dot += dotdoor;
		if (dot > 0.0f && dotdoor > 0.0f)
			dot += dotdoor;
	}
	return dot;
}
float Visibility_PitchToCamera(float dx, float dy, float dz)
{
	float dist2d = sqrtf(dx * dx + dy * dy);

	return atan2f(dz, dist2d);
}
unsigned int Visibility_FadeAlpha(float largestDistance, float camDist, float fadeDistance, unsigned int modelAlpha)
{
	float fadefactor = (largestDistance - (camDist - fadeDistance)) / fadeDistance;

	if (fadefactor > 1.0f)
		fadefactor = 1.0f;
	return (unsigned int)(modelAlpha * fadefactor);
}
void Occlusion_ExtendBounds(float *minX, float *maxX, float *minY, float *maxY, float p1x, float p1y, float p2x, float p2y)
{
	float t;

	t = *minX < p1x ? *minX : p1x;
	*minX = t < p2x ? t : p2x;
	t = *maxX > p1x ? *maxX : p1x;
	*maxX = t > p2x ? t : p2x;
	t = *minY < p1y ? *minY : p1y;
	*minY = t < p2y ? t : p2y;
	t = *maxY > p1y ? *maxY : p1y;
	*maxY = t > p2y ? t : p2y;
}
float Occlusion_LargerSide(float a, float b)
{
	return a > b ? a : b;
}
float Visibility_HighAltitudeLodDistance(float largestLodDist)
{
	float v = largestLodDist - 0.1f;

	return 0.0f > v ? 0.0f : v;
}
//- rouz edit (Claude)
