//+ rouz edit (ChatGPT)
#include "PedRoutes.h"

CRouteNode gaRoutes[NUMPEDROUTES];

void
PedRoutes_Initialise(void)
{
	// Preserve the original route operation and its ordered table updates
	for (int i = 0; i < NUMPEDROUTES; i++) {
		gaRoutes[i].m_route = -1;
		gaRoutes[i].m_pos = (PedRouteVector){0.0f, 0.0f, 0.0f};
	}
}

int16_t
PedRoutes_GetRouteThisPointIsOn(int16_t point)
{
	// Preserve the original route operation and its ordered table updates
	return gaRoutes[point].m_route;
}

// Actually GetFirstPointOfRoute
int16_t
PedRoutes_GetRouteStart(int16_t route)
{
	// Preserve the original route operation and its ordered table updates
	for (int i = 0; i < NUMPEDROUTES; i++) {
		if (route == gaRoutes[i].m_route)
			return i;
	}
	return -1;
}

PedRouteVector
PedRoutes_GetPointPosition(int16_t point)
{
	// Preserve the original route operation and its ordered table updates
	return gaRoutes[point].m_pos;
}

void
PedRoutes_AddRoutePoint(int16_t route, PedRouteVector pos)
{
	// Preserve the original route operation and its ordered table updates
	uint16_t point;
	for (point = 0; point < NUMPEDROUTES; point++) {
		if (gaRoutes[point].m_route == -1)
			break;
	}
#ifdef FIX_BUGS
	if (point == NUMPEDROUTES)
		return;
#endif
	gaRoutes[point].m_route = route;
	gaRoutes[point].m_pos = pos;
}

void
PedRoutes_RemoveRoute(int16_t route)
{
	// Preserve the original route operation and its ordered table updates
	uint16_t first_point, last_point, i;
	for (first_point = 0; first_point < NUMPEDROUTES; first_point++) {
		if (gaRoutes[first_point].m_route == route)
			break;
	}
	if (first_point == NUMPEDROUTES)
		return;
	for (last_point = first_point; last_point < NUMPEDROUTES; last_point++)
		if (gaRoutes[last_point].m_route != route)
			break;
	uint16_t diff = last_point - first_point;
#ifdef FIX_BUGS	
	for (i = first_point; i < NUMPEDROUTES - diff; i++)
		gaRoutes[i] = gaRoutes[i + diff];
#else
	for (i = 0; i < diff; i++)
		gaRoutes[first_point + i] = gaRoutes[last_point + i];
#endif
	for (i = NUMPEDROUTES - diff; i < NUMPEDROUTES; i++)
		gaRoutes[i].m_route = -1;
}
//- rouz edit (ChatGPT)
