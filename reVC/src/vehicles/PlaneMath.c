//+ rouz edit (Claude)
#include <math.h>
#include "PlaneMath.h"
#include "VectorMath.h"

static float SegmentLength(const CPlaneNode *nodes, float totalLength, int start, int end)
{
	float length;

	// Distance along the path, wrapping once around a loop
	if (start == end)
		return 0.0f;
	length = nodes[end].t - nodes[start].t;
	if (length <= 0.0f)
		length += totalLength;
	return length;
}

static void Tangent(const CPlaneNode *nodes, int numNodes, float totalLength, bool loop, int node, float segmentLength, float *t)
{
	int prev = node > 0 ? node - 1 : (loop ? numNodes - 1 : node);
	int next = node + 1 < numNodes ? node + 1 : (loop ? 0 : node);
	float prevLength = SegmentLength(nodes, totalLength, prev, node);
	float nextLength = SegmentLength(nodes, totalLength, node, next);
	float tangentLength = prevLength + nextLength;

	// Path ends use the single neighbour
	if (prev == node) {
		t[0] = nodes[next].p.x - nodes[node].p.x;
		t[1] = nodes[next].p.y - nodes[node].p.y;
		t[2] = nodes[next].p.z - nodes[node].p.z;
		return;
	}
	if (next == node) {
		t[0] = nodes[node].p.x - nodes[prev].p.x;
		t[1] = nodes[node].p.y - nodes[prev].p.y;
		t[2] = nodes[node].p.z - nodes[prev].p.z;
		return;
	}
	if (tangentLength <= 0.0f) {
		t[0] = t[1] = t[2] = 0.0f;
		return;
	}

	// Central difference scaled to the segment length
	{
		float s = segmentLength / tangentLength;
		t[0] = (nodes[next].p.x - nodes[prev].p.x) * s;
		t[1] = (nodes[next].p.y - nodes[prev].p.y) * s;
		t[2] = (nodes[next].p.z - nodes[prev].p.z) * s;
	}
}

void PlaneMath_InterpolatePathPosition(const CPlaneNode *nodes, int numNodes, float totalLength, bool loop, int node, int next, float f, float *out)
{
	float f2 = f*f;
	float f3 = f2*f;
	float segmentLength = SegmentLength(nodes, totalLength, node, next);
	float t0[3], t1[3];
	float a = 2.0f*f3 - 3.0f*f2 + 1.0f;
	float b = f3 - 2.0f*f2 + f;
	float c = -2.0f*f3 + 3.0f*f2;
	float d = f3 - f2;

	// Hermite basis on both end points and tangents
	Tangent(nodes, numNodes, totalLength, loop, node, segmentLength, t0);
	Tangent(nodes, numNodes, totalLength, loop, next, segmentLength, t1);
	out[0] = nodes[node].p.x*a + t0[0]*b + nodes[next].p.x*c + t1[0]*d;
	out[1] = nodes[node].p.y*a + t0[1]*b + nodes[next].p.y*c + t1[1]*d;
	out[2] = nodes[node].p.z*a + t0[2]*b + nodes[next].p.z*c + t1[2]*d;
}
void PlaneMath_LocatePathNode(const CPlaneNode *nodes, int numNodes, float totalLength, int *curNode, float pathPosition, int *nextNode, float *f)
{
	float pos1, pos2;
	int next = *curNode + 1;

	// Advance the current node to the position
	pos1 = nodes[*curNode].t;
	if (next < numNodes)
		pos2 = nodes[next].t;
	else {
		next = 0;
		pos2 = totalLength;
	}
	while (pathPosition < pos1 || pathPosition > pos2) {
		*curNode = (*curNode + 1) % numNodes;
		next = *curNode + 1;
		pos1 = nodes[*curNode].t;
		if (next < numNodes)
			pos2 = nodes[next].t;
		else {
			next = 0;
			pos2 = totalLength;
		}
	}
	float dist = nodes[next].t - nodes[*curNode].t;
	if (dist < 0.0f)
		dist += totalLength;
	*f = (pathPosition - nodes[*curNode].t) / dist;
	*nextNode = next;
}
float PlaneMath_AirtrainPitch(float pathPosition, float takeOffPoint, float landingPoint)
{
	float pitch = 0.0f;
	float distSinceTakeOff = pathPosition - takeOffPoint;
	if (distSinceTakeOff <= 0.0f && distSinceTakeOff > -70.0f) {
		// Shortly before take off
		pitch = 1.0f - distSinceTakeOff / -70.0f;
	} else if (distSinceTakeOff >= 0.0f && distSinceTakeOff < 100.0f) {
		// Shortly after take off
		pitch = 1.0f - distSinceTakeOff / 100.0f;
	}

	float distSinceLanding = pathPosition - landingPoint;
	if (distSinceLanding <= 0.0f && distSinceLanding > -200.0f) {
		// Shortly before landing
		pitch = 1.0f - distSinceLanding / -200.0f;
	} else if (distSinceLanding >= 0.0f && distSinceLanding < 70.0f) {
		// Shortly after landing
		pitch = 1.0f - distSinceLanding / 70.0f;
	}
	return pitch;
}
void PlaneMath_JumboPath(uint32_t time, float totalDuration, const CPlaneInterpolationLine *bits, float *position, float *speed)
{
	int j;
	float t = totalDuration * (float)(time & 0x7FFFF) / 0x80000;

	// Find the current keyframe
	for (j = 0; t > bits[j + 1].time; j++);

	float deltaT = t - bits[j].time;
	switch (bits[j].type) {
	case 0:	// Standing still
		*position = bits[j].position;
		*speed = 0.0f;
		break;
	case 1:	// Moving with constant speed
		*position = bits[j].position + bits[j].speed * deltaT;
		*speed = (totalDuration * 1000.0f / 0x80000) * bits[j].speed;
		break;
	case 2:	// Accelerating or braking
		*position = bits[j].position + (bits[j].speed + bits[j].acceleration * deltaT) * deltaT;
		*speed = (totalDuration * 1000.0f / 0x80000) * bits[j].speed + 2.0f * bits[j].acceleration * deltaT;
		break;
	}
}

void PlaneMath_CruisePath(uint32_t time, float totalDuration, float cruiseSpeed, int numPlanes, float *positions, float *speeds)
{
	int k;
	float t = totalDuration / 0x80000;

	// Planes are spread evenly over the loop
	for (k = 0; k < numPlanes; k++)
		positions[k] = cruiseSpeed * ((time + (0x80000 / numPlanes) * k) & 0x7FFFF) * t;
	for (k = 0; k < numPlanes; k++)
		speeds[k] = cruiseSpeed * t;
}
void PlaneMath_ComputePathLengths(CPlaneNode *nodes, int numNodes, bool loop, float *totalLength)
{
	int i;

	*totalLength = 0.0f;
	for (i = 0; i < numNodes; i++) {
		nodes[i].t = *totalLength;
		float dx = nodes[(i + 1) % numNodes].p.x - nodes[i].p.x;
		float dy = nodes[(i + 1) % numNodes].p.y - nodes[i].p.y;
		float l = sqrtf(dx * dx + dy * dy);
		if (!loop && i == numNodes - 1)
			l = 0.0f;
		*totalLength += l;
	}
}

void PlaneMath_FindGroundPoints(CPlaneNode *nodes, int numNodes, float *landingPoint, float *takeOffPoint)
{
	int i;

	// Figure out which nodes are on the ground
	for (i = 0; i < numNodes; i++) {
		if (nodes[i].p.z < 14.0f) {
			nodes[i].p.z = 14.0f;
			nodes[i].bOnGround = true;
		} else
			nodes[i].bOnGround = false;
	}

	// Find the landing and the take off points
	*landingPoint = -1.0f;
	*takeOffPoint = -1.0f;
	bool lastOnGround = nodes[numNodes - 1].bOnGround;
	for (i = 0; i < numNodes; i++) {
		if (nodes[i].bOnGround && !lastOnGround)
			*landingPoint = nodes[i].t;
		else if (!nodes[i].bOnGround && lastOnGround)
			*takeOffPoint = nodes[i].t;
		lastOnGround = nodes[i].bOnGround;
	}
}

void PlaneMath_BuildFlightTimeline(CPlaneInterpolationLine *bits, float takeOffPoint, float landingPoint, float totalLength, float cruiseSpeed, float taxiSpeed, float *totalDuration)
{
	float time = 0.0f;
	float position = 0.0f;

	// Start on the ground with slow speed
	bits[0].type = 1;
	bits[0].time = time;
	bits[0].position = position;
	bits[0].speed = taxiSpeed;
	bits[0].acceleration = 0.0f;
	float dist = (takeOffPoint - 500.0f) - position;
	time += dist / taxiSpeed;
	position += dist;

	// Accelerate to take off
	bits[1].type = 2;
	bits[1].time = time;
	bits[1].position = position;
	bits[1].speed = taxiSpeed;
	bits[1].acceleration = 618.75f / 500.0f;
	time += 500.0f / ((cruiseSpeed + taxiSpeed) / 2.0f);
	position += 500.0f;

	// Fly at cruise speed
	bits[2].type = 1;
	bits[2].time = time;
	bits[2].position = position;
	bits[2].speed = cruiseSpeed;
	bits[2].acceleration = 0.0f;
	dist = landingPoint - takeOffPoint;
	time += dist / cruiseSpeed;
	position += dist;

	// Brake after landing
	bits[3].type = 2;
	bits[3].time = time;
	bits[3].position = position;
	bits[3].speed = cruiseSpeed;
	bits[3].acceleration = -618.75f / 500.0f;
	time += 500.0f / ((cruiseSpeed + taxiSpeed) / 2.0f);
	position += 500.0f;

	// Taxi
	bits[4].type = 1;
	bits[4].time = time;
	bits[4].position = position;
	bits[4].speed = taxiSpeed;
	bits[4].acceleration = 0.0f;
	time += (totalLength - position) / taxiSpeed;

	// The end
	bits[5].time = time;
	*totalDuration = time;
}
void PlaneMath_PathOrientation(const float front[3], const float rear[3], const float front2[3], float pitch, bool addRoll, float fwdOut[3], float rightOut[3], float upOut[3])
{
	VectorMath3 fwd = { front[0] - rear[0], front[1] - rear[1], front[2] - rear[2] };
	VectorMath3 fwd2 = { front2[0] - rear[0], front2[1] - rear[1], front2[2] - rear[2] };
	VectorMath3 up3 = { 0.0f, 0.0f, 1.0f };
	VectorMath3 roll, right, up;

	// Forward direction, tilted by the pitch when there is one
	VectorMath_Normalise(&fwd);
	if (pitch != 0.0f) {
		fwd.z += 0.4f * pitch;
		VectorMath_Normalise(&fwd);
	}

	// Right direction from the forward direction and the roll
	VectorMath_Normalise(&fwd2);
	roll = VectorMath_Cross(fwd, fwd2);
	right = VectorMath_Cross(fwd, up3);
	if (addRoll)
		right.z += 3.0f * roll.z;
	VectorMath_Normalise(&right);
	up = VectorMath_Cross(right, fwd);
	fwdOut[0] = fwd.x; fwdOut[1] = fwd.y; fwdOut[2] = fwd.z;
	rightOut[0] = right.x; rightOut[1] = right.y; rightOut[2] = right.z;
	upOut[0] = up.x; upOut[1] = up.y; upOut[2] = up.z;
}
//- rouz edit (Claude)
