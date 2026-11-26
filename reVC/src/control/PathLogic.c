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
void PathLogic_RemoveNodeFromList(CPedPathNode *node)
{
	// Link the neighbours to each other
	node->next->prev = node->prev;
	if (node->prev)
		node->prev->next = node->next;
}

void PathLogic_AddNodeToList(CPedPathNode *node, int16_t index, CPedPathNode *list)
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
uint8_t PathLogic_WanderDirection(float dirX, float dirY)
{
	// North, east, south and west or the diagonals in between
	if (dirX < 0.0f) {
		if (2.0f * fabsf(dirY) < -dirX)
			return 6;
		if (-2.0f * dirX < dirY)
			return 0;
		if (2.0f * dirX > dirY)
			return 4;
		if (dirY > 0.0f)
			return 7;
		return 5;
	}
	if (2.0f * fabsf(dirY) < dirX)
		return 2;
	if (2.0f * dirX < dirY)
		return 0;
	if (-2.0f * dirX > dirY)
		return 4;
	if (dirY > 0.0f)
		return 1;
	return 3;
}

void PathLogic_CompassVector(uint8_t direction, float *x, float *y)
{
	*x = sinf(direction * PI / 4.0f);
	*y = cosf(direction * PI / 4.0f);
}
void PathLogic_SwitchBoxAxes(float x1, float y1, float x2, float y2, float angle, float width, float axes[5])
{
	float v12x = x2 - x1;
	float v12y = y2 - y1;
	float len12 = sqrtf(v12x * v12x + v12y * v12y);
	float v23x, v23y, mag;

	v12x /= len12;
	v12y /= len12;
	v23x = sinf(angle) * width;
	v23y = -(cosf(angle) * width);
	mag = sqrtf(v23x * v23x + v23y * v23y);
	v23x /= mag;
	v23y /= mag;
	axes[0] = v12x;
	axes[1] = v12y;
	axes[2] = len12;
	axes[3] = v23x;
	axes[4] = v23y;
}

bool PathLogic_NodeInSwitchBox(float relX, float relY, const float axes[5], float width)
{
	float dot = relX * axes[0] + relY * axes[1];

	if (dot < 0.0f || dot > axes[2])
		return false;
	dot = relX * axes[3] + relY * axes[4];
	if (dot < 0.0f || dot > width)
		return false;
	return true;
}
void PathLogic_CollapsedDirection(float dx, float dy, float *dirX, float *dirY)
{
	float mag = sqrtf(dx * dx + dy * dy);

	*dirX = dx / mag * 100;
	*dirY = dy / mag * 100;
}

void PathLogic_InternalLinkGeometry(float x1, float y1, float x2, float y2, bool firstIsLower, float out[4])
{
	float dx = x2 - x1;
	float dy = y2 - y1;
	float mag = sqrtf(dx * dx + dy * dy);

	out[0] = (x1 + x2) * 0.5f;
	out[1] = (y1 + y2) * 0.5f;
	dx /= mag;
	dy /= mag;
	if (firstIsLower) {
		dx = -dx;
		dy = -dy;
	}
	out[2] = dx;
	out[3] = dy;
}
float PathLogic_NodeDistance(float dx, float dy, float dz)
{
	return fabsf(dx) + fabsf(dy) + 3.0f * fabsf(dz);
}
float PathLogic_SpawnDelta(float dist, float spawnDist, bool isWaterPath)
{
	return isWaterPath ? (dist - spawnDist * 1.5f) : (dist - spawnDist);
}

float PathLogic_CrossFraction(float d1, float d2)
{
	return fabsf(d1) / (fabsf(d1) + fabsf(d2));
}

float PathLogic_HeadingDegrees(float dirX, float dirY)
{
	const float pi = (float)3.14159265358979323846;

	return atan2f(-dirX, dirY) * 180.0f / pi;
}
bool PathLogic_RouteTooFar(float dx, float dy, float dz, float limit)
{
	return fabsf(dx) > limit || fabsf(dy) > limit || fabsf(dz) > limit;
}

float PathLogic_TileWidth(float width, float maxWidth)
{
	return 8.0f * (width < maxWidth ? width : maxWidth);
}
float PathLogic_LargestAxisDistance(float dx, float dy)
{
	float ax = fabsf(dx);
	float ay = fabsf(dy);

	return ax > ay ? ax : ay;
}

bool PathLogic_TooFarFromGround(float groundZ, float posZ)
{
	return fabsf(groundZ - posZ) > 3.0f;
}
uint8_t PathLogic_LinkDistance(float dist)
{
	return (uint8_t)(dist < 255 ? dist : 255);
}
//- rouz edit (Claude)
