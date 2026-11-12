//+ rouz edit (Claude)
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"
#include "config.h"
#include "PathLogic.h"

void PathLogic_WanderOffset(float pedNodeWidth, uint16_t random, float *dx, float *dy)
{
	// The low and high nibble of the random number pick the spot, 7 is the middle
	*dx = pedNodeWidth * ((random % 16) - 7);
	*dy = pedNodeWidth * (((random / 16) % 16) - 7);
}

void PathLogic_WidthOffsetBetweenNodes(uint8_t width1, uint8_t width2, float widthToPedNodeWidth, uint16_t random, float *dx, float *dy)
{
	// The narrower of the two nodes limits the spread
	uint8_t narrow = width1 < width2 ? width1 : width2;
	*dx = narrow * widthToPedNodeWidth * ((random % 16) - 7);
	*dy = narrow * widthToPedNodeWidth * (((random / 16) % 16) - 7);
}
float PathLogic_CalcRoadDensity(const CPathNodeData *nodes, int numNodes, const uint16_t *connections, const int16_t *carPathConnections, const CCarPathLinkData *links, float x, float y)
{
	int i, j;
	float density = 0.0f;

	// Add up the lanes times the length of every link of the nodes near the position
	for (i = 0; i < numNodes; i++) {
		if (fabsf(nodes[i].x/8.0f - x) < 80.0f &&
		    fabsf(nodes[i].y/8.0f - y) < 80.0f &&
		    nodes[i].numLinks > 0) {
			for (j = 0; j < nodes[i].numLinks; j++) {
				int next = connections[nodes[i].firstLink + j] & 0x3FFF;
				float dx = nodes[i].x/8.0f - nodes[next].x/8.0f;
				float dy = nodes[i].y/8.0f - nodes[next].y/8.0f;
				float dist = sqrtf(dx*dx + dy*dy);
				next = carPathConnections[nodes[i].firstLink + j];
				density += links[next].numLeftLanes * dist;
				density += links[next].numRightLanes * dist;
			}
		}
	}
	return density/2500.0f;
}
float PathLogic_FindNodeOrientationForCarPlacement(const CPathNodeData *nodes, const uint16_t *connections, int nodeId)
{
	const CPathNodeData *n = &nodes[nodeId];
	const CPathNodeData *next;
	float dx, dy, sq;

	if (n->numLinks == 0)
		return 0.0f;

	// Direction to the first connected node on the ground plane
	next = &nodes[connections[n->firstLink] & 0x3FFF];
	dx = next->x/8.0f - n->x/8.0f;
	dy = next->y/8.0f - n->y/8.0f;
	sq = dx*dx + dy*dy + 0.0f*0.0f;
	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		dx *= invsqrt;
		dy *= invsqrt;
	} else
		dx = 1.0f;
	return RADTODEG(atan2f(-dx, dy));
}

void PathLogic_FindNodeCoorsForScript(const CPathNodeData *nodes, const uint16_t *connections, int id, float *out)
{
	const CPathNodeData *n = &nodes[id];
	const CPathNodeData *next;
	float dx, dy, sq, offset;

	// Nodes without a divider or links are used as they are
	if (n->width == 0 || n->numLinks == 0) {
		out[0] = n->x/8.0f;
		out[1] = n->y/8.0f;
		out[2] = n->z/8.0f;
		return;
	}

	// Direction along the road, flipped to point towards positive x
	next = &nodes[connections[n->firstLink] & 0x3FFF];
	dx = next->x/8.0f - n->x/8.0f;
	dy = next->y/8.0f - n->y/8.0f;
	sq = dx*dx + dy*dy;
	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		dx *= invsqrt;
		dy *= invsqrt;
	} else
		dx = 1.0f;
	if (dx < 0) {
		dx = -dx;
		dy = -dy;
	}

	// Move sideways by half a lane and the divider width
	offset = 5.0f / 2 + n->width/(2*8.0f);
	out[0] = n->x/8.0f + (-dy)*offset;
	out[1] = n->y/8.0f + dx*offset;
	out[2] = n->z/8.0f + 0.0f*offset;
}
// Unit direction on the ground plane from node a to node b
static void GroundDirection(const CPathNodeData *a, const CPathNodeData *b, float *dx, float *dy)
{
	float sq;
	*dx = b->x/8.0f - a->x/8.0f;
	*dy = b->y/8.0f - a->y/8.0f;
	sq = *dx * *dx + *dy * *dy;
	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		*dx *= invsqrt;
		*dy *= invsqrt;
	} else
		*dx = 1.0f;
}

float PathLogic_FindNodeOrientationForCarPlacementFacingDestination(const CPathNodeData *nodes, const uint16_t *connections, int nodeId, float x, float y, bool towards)
{
	const CPathNodeData *n = &nodes[nodeId];
	float tx = x - n->x/8.0f;
	float ty = y - n->y/8.0f;
	float sq = tx*tx + ty*ty;
	int i, bestNode;
	float bestDot, dx, dy;

	// Unit direction to the destination
	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		tx *= invsqrt;
		ty *= invsqrt;
	} else
		tx = 1.0f;

	if (n->numLinks == 0)
		return 0.0f;

	// Pick the linked node whose direction fits the destination best
	bestNode = connections[n->firstLink] & 0x3FFF;
#ifdef FIX_BUGS
	bestDot = towards ? -2.0f : 2.0f;
#else
	bestDot = towards ? -2 : 2;
#endif
	for (i = 0; i < n->numLinks; i++) {
		float angle;
		GroundDirection(n, &nodes[connections[n->firstLink + i] & 0x3FFF], &dx, &dy);
		angle = dx*tx + dy*ty;
		if (towards) {
			if (angle > bestDot) {
				bestDot = angle;
				bestNode = connections[n->firstLink + i] & 0x3FFF;
			}
		} else {
			if (angle < bestDot) {
				bestDot = angle;
				bestNode = connections[n->firstLink + i] & 0x3FFF;
			}
		}
	}

	GroundDirection(n, &nodes[bestNode], &dx, &dy);
	return RADTODEG(atan2f(-dx, dy));
}
void PedPath_RemoveNodeFromList(CPedPathNode *node)
{
	// Link the neighbours to each other
	node->next->prev = node->prev;
	if (node->prev)
		node->prev->next = node->next;
}

void PedPath_AddNodeToList(CPedPathNode *node, int16_t index, CPedPathNode *list)
{
	// Insert before the sentinel of the list
	node->prev = list[index].prev;
	node->next = &list[index];
	if (list[index].prev)
		list[index].prev->next = node;
	list[index].prev = node;
	node->id = index;
}
bool PathLogic_LinkHasTrafficLight(const CPathNodeData *nodes, const uint16_t *connections, const int16_t *carPathConnections, const CCarPathLinkData *links, int nodeIndex, int neighbour)
{
	const CPathNodeData *cur = &nodes[nodeIndex];
	int i;

	// Find the link that leads to the neighbour
	for (i = 0; i < cur->numLinks; i++)
		if ((connections[cur->firstLink + i] & 0x3FFF) == neighbour)
			break;

	// It has lights when its traffic light type is set
	return i < cur->numLinks && (links[carPathConnections[cur->firstLink + i]].trafficLightType & 3) != 0;
}
//- rouz edit (Claude)
