//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"

typedef struct PedRouteVector {
    float x, y, z;
} PedRouteVector;

typedef struct CRouteNode {
    int16_t m_route;
    PedRouteVector m_pos;
} CRouteNode;

#ifdef __cplusplus
extern "C" {
#endif
extern CRouteNode gaRoutes[NUMPEDROUTES];
void PedRoutes_Initialise(void);
int16_t PedRoutes_GetRouteThisPointIsOn(int16_t point);
int16_t PedRoutes_GetRouteStart(int16_t route);
PedRouteVector PedRoutes_GetPointPosition(int16_t point);
void PedRoutes_AddRoutePoint(int16_t route, PedRouteVector pos);
void PedRoutes_RemoveRoute(int16_t route);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
