//+ rouz edit (Claude)
#include "PlaneMath.h"

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
//- rouz edit (Claude)
