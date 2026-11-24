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
// Compass direction (0 north, 1 north east, 2 east, 3 south east, 4 south, 5 south west, 6 west, 7 north west) a ped that walks along a link goes in, from the normalised direction of the link
uint8_t PathLogic_WanderDirection(float dirX, float dirY);
// Unit vector of a compass direction in eighths of a turn (0 is north), x is the sine and y the cosine
void PathLogic_CompassVector(uint8_t direction, float *x, float *y);
// Axes of the oriented box the nodes of a road are switched on or off in: along the line from the first point to the second one (unit vector and length) and across it (unit vector, towards the side of the angle), from the two points, the angle of the box and its width; axes receives the first axis x, y, its length and the second axis x, y
void PathLogic_SwitchBoxAxes(float x1, float y1, float x2, float y2, float angle, float width, float axes[5]);
// Whether a node is inside an oriented box, from its position relative to the first point of the box, the axes from the function above and the length of the second axis
bool PathLogic_NodeInSwitchBox(float relX, float relY, const float axes[5], float width);
// Direction (scaled by 100, not yet stored as an integer) of the link made of two external nodes that were collapsed, from the difference of the positions of the nodes they connect to
void PathLogic_CollapsedDirection(float dx, float dy, float *dirX, float *dirY);
// Middle and direction (unit vector) of a link between two internal nodes, from their positions and whether the first one has the lower index (the direction is reversed then): out receives posX, posY, dirX and dirY
void PathLogic_InternalLinkGeometry(float x1, float y1, float x2, float y2, bool firstIsLower, float out[4]);
// Distance between a path node and a point used to find the closest node: the sum of the distances on x and y plus three times the distance on z
float PathLogic_NodeDistance(float dx, float dy, float dz);
// Distance of a node from the spawn distance: water nodes count the spawn distance 1.5 times
float PathLogic_SpawnDelta(float dist, float spawnDist, bool isWaterPath);
// Position between two nodes where the spawn distance is crossed, from their distances from the spawn distance (of opposite signs)
float PathLogic_CrossFraction(float d1, float d2);
// Heading in degrees of a direction on the ground (0 along y, positive to the left)
float PathLogic_HeadingDegrees(float dirX, float dirY);
// Whether the distance between the start and the end of a ped route is larger than the limit on any axis
bool PathLogic_RouteTooFar(float dx, float dy, float dz, float limit);
// Width of a path tile stored in the node info: 8 times the width limited to a maximum
float PathLogic_TileWidth(float width, float maxWidth);
// Distance between two nodes measured as the larger of the distances on x and y
float PathLogic_LargestAxisDistance(float dx, float dy);
// Whether a spawn position is too far above or below the ground (more than 3 units)
bool PathLogic_TooFarFromGround(float groundZ, float posZ);
// Distance stored for a connection between two nodes: the distance limited to 255 and truncated to a byte
uint8_t PathLogic_LinkDistance(float dist);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
