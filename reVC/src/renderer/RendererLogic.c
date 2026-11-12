//+ rouz edit (Claude)
#include <math.h>
#include "RendererLogic.h"

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

//- rouz edit (Claude)
