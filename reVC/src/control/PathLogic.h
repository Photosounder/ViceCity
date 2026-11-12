//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "PathNodeData.h"
#include "PedPathNode.h"

#ifdef __cplusplus
extern "C" {
#endif
// Offset from a path node for a wandering ped, a random number picks one of 16 by 16 spots across the node width
void PathLogic_WanderOffset(float pedNodeWidth, uint16_t random, float *dx, float *dy);
// Same for a place between two nodes, the narrower node counts and widthToPedNodeWidth converts the width units
void PathLogic_WidthOffsetBetweenNodes(uint8_t width1, uint8_t width2, float widthToPedNodeWidth, uint16_t random, float *dx, float *dy);
// How much road there is around a position, lane kilometres of the car path links of the nodes within 80 units
float PathLogic_CalcRoadDensity(const CPathNodeData *nodes, int numNodes, const uint16_t *connections, const int16_t *carPathConnections, const CCarPathLinkData *links, float x, float y);
// Heading in degrees a car placed on a node should face, zero for nodes without links
float PathLogic_FindNodeOrientationForCarPlacement(const CPathNodeData *nodes, const uint16_t *connections, int nodeId);
// Position of a node for scripts, moved aside when the road has a divider in the middle
void PathLogic_FindNodeCoorsForScript(const CPathNodeData *nodes, const uint16_t *connections, int id, float *out);
// Heading in degrees for a car on a node facing the linked node that points most towards a destination, or most away from it when towards is false
float PathLogic_FindNodeOrientationForCarPlacementFacingDestination(const CPathNodeData *nodes, const uint16_t *connections, int nodeId, float x, float y, bool towards);
// Take a node out of the cost list it is in, the list ends in a sentinel so there is always a next node
void PedPath_RemoveNodeFromList(CPedPathNode *node);
// Put a node in front of the sentinel of a cost list and remember the cost as its id
void PedPath_AddNodeToList(CPedPathNode *node, int16_t index, CPedPathNode *list);
// Whether the road link from a path node to one of its neighbours has traffic lights, false when the nodes are not linked
bool PathLogic_LinkHasTrafficLight(const CPathNodeData *nodes, const uint16_t *connections, const int16_t *carPathConnections, const CCarPathLinkData *links, int nodeIndex, int neighbour);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
