//+ rouz edit (Claude)
#include <math.h>
#include "VectorMath.h"

void VectorMath_Normalise(VectorMath3 *v)
{
	// Scale to unit length, falling back to the x axis for a null vector
	float sq = v->x*v->x + v->y*v->y + v->z*v->z;
	if (sq > 0.0f) {
		float invsqrt = 1.0f / sqrtf(sq);
		v->x *= invsqrt;
		v->y *= invsqrt;
		v->z *= invsqrt;
	} else
		v->x = 1.0f;
}

VectorMath3 VectorMath_Cross(VectorMath3 a, VectorMath3 b)
{
	// Standard right-handed cross product
	VectorMath3 r = {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
	return r;
}

VectorMath3 VectorMath_MatrixTimesVector3x3(const float m[4][4], VectorMath3 v)
{
	// Multiply by the rotation part using the matrix rows as columns
	VectorMath3 r = {
		m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
		m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
		m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z};
	return r;
}

VectorMath3 VectorMath_VectorTimesMatrix3x3(VectorMath3 v, const float m[4][4])
{
	// Multiply the vector by the rotation part using the matrix rows directly
	VectorMath3 r = {
		m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z,
		m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z,
		m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z};
	return r;
}

VectorMath3 VectorMath_MatrixTimesVector(const float m[4][4], VectorMath3 v)
{
	// Rotate then add the position row
	VectorMath3 r = VectorMath_MatrixTimesVector3x3(m, v);
	r.x += m[3][0];
	r.y += m[3][1];
	r.z += m[3][2];
	return r;
}

void VectorMath_TransformPoints(float *out, int n, const float m[4][4], const void *in, int in_stride)
{
	// Transform each point by the full matrix, stepping the input by bytes and the output by one 16-byte slot
	while (n-- > 0) {
		const float *p = (const float *)in;
		VectorMath3 v = {p[0], p[1], p[2]};
		v = VectorMath_MatrixTimesVector(m, v);
		out[0] = v.x;
		out[1] = v.y;
		out[2] = v.z;
		in = (const char *)in + in_stride;
		out += 4;
	}
}
//- rouz edit (Claude)
