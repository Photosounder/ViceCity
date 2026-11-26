//+ rouz edit (Claude)
#pragma once
#include "VectorFwd.h" // rouz edit (Claude)
#include <stdbool.h>
#include "PlaneNodes.h"

#ifdef __cplusplus
extern "C" {
#endif
// Hermite interpolation along a flight path between two nodes, f is 0 at node and 1 at next, the result goes to out[3]
void PlaneMath_InterpolatePathPosition(const CPlaneNode *nodes, int numNodes, float totalLength, bool loop, int node, int next, float f, float *out);
// Finds the segment of a flight path a position along the path is on, starting the search from node curNode which gets moved forward, nextNode gets the node after it and f how far along the segment the position is
void PlaneMath_LocatePathNode(const CPlaneNode *nodes, int numNodes, float totalLength, int *curNode, float pathPosition, int *nextNode, float *f);
// Pitch of the airtrain from where it is on its path: it goes up shortly before and after the take off and down shortly before and after the landing
float PlaneMath_AirtrainPitch(float pathPosition, float takeOffPoint, float landingPoint);
// Position along its path and speed of a jumbo jet that is time (in milliseconds) into the loop of the flight, from the keyframes of the flight
void PlaneMath_JumboPath(uint32_t time, float totalDuration, const CPlaneInterpolationLine *bits, float *position, float *speed);
// Positions along the path and speeds of numPlanes planes that fly at the same speed and are evenly spread over a loop of the flight lasting totalDuration
void PlaneMath_CruisePath(uint32_t time, float totalDuration, float cruiseSpeed, int numPlanes, float *positions, float *speeds);
// Distance along the path of every node of a flight path, for a path that is no loop the last node ends it, totalLength gets the length of the path
void PlaneMath_ComputePathLengths(CPlaneNode *nodes, int numNodes, bool loop, float *totalLength);
// Marks the nodes of the path of the jumbo jets that are on the ground, which are put at a height of 14, and finds the point they land and the point they take off at
void PlaneMath_FindGroundPoints(CPlaneNode *nodes, int numNodes, float *landingPoint, float *takeOffPoint);
// Keyframes of the flight of the jumbo jets: taxi, take off, cruise, landing, brake and taxi back, the last keyframe only gets its time, totalDuration is the time the flight lasts
void PlaneMath_BuildFlightTimeline(CPlaneInterpolationLine *bits, float takeOffPoint, float landingPoint, float totalLength, float cruiseSpeed, float taxiSpeed, float *totalDuration);
// Orientation of a plane following its path from the points of the rear and the front of the path and a second front point: the forward direction (tilted by the pitch when it is not zero), the right direction (with the roll added when asked) and the up direction
void PlaneMath_PathOrientation(const CVector *front, const CVector *rear, const CVector *front2, float pitch, bool addRoll, CVector *fwdOut, CVector *rightOut, CVector *upOut);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
