//+ rouz edit (Claude)
#include "GarageLogic.h"

bool Garage_IsPointInside(const CGarageData *g, float x, float y, float z)
{
	// Same as the margin test with a zero margin
	float tx, ty, dp;
	if (z < g->m_fInfZ)
		return false;
	if (z > g->m_fSupZ)
		return false;
	tx = x - g->m_vecCorner1.x;
	ty = y - g->m_vecCorner1.y;
	dp = g->m_vDir1.x * tx + g->m_vDir1.y * ty;
	if (dp < 0.0f)
		return false;
	if (g->m_fDir1Len < dp)
		return false;
	dp = g->m_vDir2.x * tx + g->m_vDir2.y * ty;
	if (dp < 0.0f)
		return false;
	if (g->m_fDir2Len < dp)
		return false;
	return true;
}

bool Garage_IsPointInsideWithMargin(const CGarageData *g, float x, float y, float z, float margin)
{
	float tx, ty, dp;
	if (z < g->m_fInfZ - margin)
		return false;
	if (z > g->m_fSupZ + margin)
		return false;
	tx = x - g->m_vecCorner1.x;
	ty = y - g->m_vecCorner1.y;
	dp = g->m_vDir1.x * tx + g->m_vDir1.y * ty;
	if (dp < -margin)
		return false;
	if (g->m_fDir1Len + margin < dp)
		return false;
	dp = g->m_vDir2.x * tx + g->m_vDir2.y * ty;
	if (dp < -margin)
		return false;
	if (g->m_fDir2Len + margin < dp)
		return false;
	return true;
}

float Garage_CalcDistToRectangleSquared(const CGarageData *g, float X, float Y)
{
	// Squared distance from the point to the garage rectangle, zero inside
	float distX, distY;
	if (X < g->m_fInfX)
		distX = g->m_fInfX - X;
	else if (X > g->m_fSupX)
		distX = X - g->m_fSupX;
	else
		distX = 0.0f;
	if (Y < g->m_fInfY)
		distY = g->m_fInfY - Y;
	else if (Y > g->m_fSupY)
		distY = Y - g->m_fSupY;
	else
		distY = 0.0f;
	return distX * distX + distY * distY;
}

float Garage_CalcSmallestDistToDoorSquared(const CGarageData *g, float X, float Y)
{
	// Squared distance to the closer of the two doors that exist
	float dist1 = 10000000.0f;
	float dist2 = 10000000.0f;
	if (g->m_pDoor1)
		dist1 = (g->m_fDoor1X - X) * (g->m_fDoor1X - X) + (g->m_fDoor1Y - Y) * (g->m_fDoor1Y - Y);
	if (g->m_pDoor2)
		dist2 = (g->m_fDoor2X - X) * (g->m_fDoor2X - X) + (g->m_fDoor2Y - Y) * (g->m_fDoor2Y - Y);
	return dist1 < dist2 ? dist1 : dist2;
}

void Garage_OpenThis(CGarageData *g)
{
	// Only closed or closing garages start opening
	if (g->m_eGarageState == GS_FULLYCLOSED || g->m_eGarageState == GS_CLOSING || g->m_eGarageState == GS_CLOSEDCONTAINSCAR)
		g->m_eGarageState = GS_OPENING;
}

void Garage_CloseThis(CGarageData *g)
{
	// Only open or opening garages start closing
	if (g->m_eGarageState == GS_OPENED || g->m_eGarageState == GS_OPENING)
		g->m_eGarageState = GS_CLOSING;
}
//- rouz edit (Claude)
