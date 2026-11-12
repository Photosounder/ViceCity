//+ rouz edit (Claude)
#include <math.h>
#include "MatrixMath.h"
#include "VectorMath.h"

static VectorMath3 GetRow(const float m[4][4], int row)
{
	// Read a row's first three components
	VectorMath3 v = {m[row][0], m[row][1], m[row][2]};
	return v;
}

static void SetRow(float m[4][4], int row, VectorMath3 v)
{
	// Write a row's first three components
	m[row][0] = v.x;
	m[row][1] = v.y;
	m[row][2] = v.z;
}

void MatrixMath_ResetOrientation(float m[4][4])
{
	// Set the rotation part to identity
	m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
}

void MatrixMath_SetUnity(float m[4][4])
{
	// Identity rotation and zero position
	MatrixMath_ResetOrientation(m);
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
}

void MatrixMath_SetScale(float m[4][4], float s)
{
	// Uniform scale on the diagonal and zero position
	m[0][0] = s;    m[0][1] = 0.0f; m[0][2] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = s;    m[1][2] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = s;
	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
}

void MatrixMath_SetTranslate(float m[4][4], float x, float y, float z)
{
	// Identity rotation with the given position
	MatrixMath_ResetOrientation(m);
	m[3][0] = x;
	m[3][1] = y;
	m[3][2] = z;
}

void MatrixMath_SetRotateXOnly(float m[4][4], float angle)
{
	// Rotation around X leaving the position untouched
	float c = cosf(angle);
	float s = sinf(angle);

	m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = c;    m[1][2] = s;
	m[2][0] = 0.0f; m[2][1] = -s;   m[2][2] = c;
}

void MatrixMath_SetRotateYOnly(float m[4][4], float angle)
{
	// Rotation around Y leaving the position untouched
	float c = cosf(angle);
	float s = sinf(angle);

	m[0][0] = c;    m[0][1] = 0.0f; m[0][2] = -s;
	m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
	m[2][0] = s;    m[2][1] = 0.0f; m[2][2] = c;
}

void MatrixMath_SetRotateZOnly(float m[4][4], float angle)
{
	// Rotation around Z leaving the position untouched
	float c = cosf(angle);
	float s = sinf(angle);

	m[0][0] = c;    m[0][1] = s;    m[0][2] = 0.0f;
	m[1][0] = -s;   m[1][1] = c;    m[1][2] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
}

void MatrixMath_SetRotate(float m[4][4], float xAngle, float yAngle, float zAngle)
{
	// Combined rotation with zero position
	float cX = cosf(xAngle);
	float sX = sinf(xAngle);
	float cY = cosf(yAngle);
	float sY = sinf(yAngle);
	float cZ = cosf(zAngle);
	float sZ = sinf(zAngle);

	m[0][0] = cZ * cY - (sZ * sX) * sY;
	m[0][1] = (cZ * sX) * sY + sZ * cY;
	m[0][2] = -cX * sY;

	m[1][0] = -sZ * cX;
	m[1][1] = cZ * cX;
	m[1][2] = sX;

	m[2][0] = (sZ * sX) * cY + cZ * sY;
	m[2][1] = sZ * sY - (cZ * sX) * cY;
	m[2][2] = cX * cY;

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
}

void MatrixMath_RotateX(float m[4][4], float x)
{
	// Rotate all four rows around X in place
	float c = cosf(x);
	float s = sinf(x);

	for (int i = 0; i < 4; i++) {
		float y = m[i][1];
		float z = m[i][2];
		m[i][1] = c * y - s * z;
		m[i][2] = c * z + s * y;
	}
}

void MatrixMath_RotateY(float m[4][4], float y)
{
	// Rotate all four rows around Y in place
	float c = cosf(y);
	float s = sinf(y);

	for (int i = 0; i < 4; i++) {
		float x = m[i][0];
		float z = m[i][2];
		m[i][0] = c * x + s * z;
		m[i][2] = c * z - s * x;
	}
}

void MatrixMath_RotateZ(float m[4][4], float z)
{
	// Rotate all four rows around Z in place
	float c = cosf(z);
	float s = sinf(z);

	for (int i = 0; i < 4; i++) {
		float x = m[i][0];
		float y = m[i][1];
		m[i][0] = c * x - s * y;
		m[i][1] = c * y + s * x;
	}
}

void MatrixMath_Rotate(float m[4][4], float x, float y, float z)
{
	// Apply a combined rotation to all four rows
	float cX = cosf(x);
	float sX = sinf(x);
	float cY = cosf(y);
	float sY = sinf(y);
	float cZ = cosf(z);
	float sZ = sinf(z);

	float x1 = cZ * cY - (sZ * sX) * sY;
	float x2 = (cZ * sX) * sY + sZ * cY;
	float x3 = -cX * sY;
	float y1 = -sZ * cX;
	float y2 = cZ * cX;
	float y3 = sX;
	float z1 = (sZ * sX) * cY + cZ * sY;
	float z2 = sZ * sY - (cZ * sX) * cY;
	float z3 = cX * cY;

	for (int i = 0; i < 4; i++) {
		float a = m[i][0];
		float b = m[i][1];
		float c = m[i][2];
		m[i][0] = x1 * a + y1 * b + z1 * c;
		m[i][1] = x2 * a + y2 * b + z2 * c;
		m[i][2] = x3 * a + y3 * b + z3 * c;
	}
}

void MatrixMath_Reorthogonalise(float m[4][4])
{
	// Rebuild up, right and forward as mutually perpendicular unit vectors
	VectorMath3 r = GetRow(m, 0);
	VectorMath3 f = GetRow(m, 1);
	VectorMath3 u = VectorMath_Cross(r, f);
	VectorMath_Normalise(&u);
	r = VectorMath_Cross(f, u);
	VectorMath_Normalise(&r);
	f = VectorMath_Cross(u, r);
	SetRow(m, 0, r);
	SetRow(m, 1, f);
	SetRow(m, 2, u);
}

void MatrixMath_Multiply(float out[4][4], const float m1[4][4], const float m2[4][4])
{
	// Compose into a temporary so the output may alias an input
	float t[4][4];
	for (int i = 0; i < 4; i++) {
		t[i][0] = m1[0][0] * m2[i][0] + m1[1][0] * m2[i][1] + m1[2][0] * m2[i][2];
		t[i][1] = m1[0][1] * m2[i][0] + m1[1][1] * m2[i][1] + m1[2][1] * m2[i][2];
		t[i][2] = m1[0][2] * m2[i][0] + m1[1][2] * m2[i][1] + m1[2][2] * m2[i][2];
	}
	t[3][0] += m1[3][0];
	t[3][1] += m1[3][1];
	t[3][2] += m1[3][2];
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 3; j++)
			out[i][j] = t[i][j];
}

void MatrixMath_Invert(float dst[4][4], const float src[4][4])
{
	// Transpose the rotation and rotate the negated position, keeping the original accumulation order
	dst[3][0] = dst[3][1] = dst[3][2] = 0.0f;

	dst[0][0] = src[0][0];
	dst[0][1] = src[1][0];
	dst[0][2] = src[2][0];

	dst[1][0] = src[0][1];
	dst[1][1] = src[1][1];
	dst[1][2] = src[2][1];

	dst[2][0] = src[0][2];
	dst[2][1] = src[1][2];
	dst[2][2] = src[2][2];

	dst[3][0] += dst[0][0] * src[3][0];
	dst[3][1] += dst[0][1] * src[3][0];
	dst[3][2] += dst[0][2] * src[3][0];

	dst[3][0] += dst[1][0] * src[3][1];
	dst[3][1] += dst[1][1] * src[3][1];
	dst[3][2] += dst[1][2] * src[3][1];

	dst[3][0] += dst[2][0] * src[3][2];
	dst[3][1] += dst[2][1] * src[3][2];
	dst[3][2] += dst[2][2] * src[3][2];

	dst[3][0] = -dst[3][0];
	dst[3][1] = -dst[3][1];
	dst[3][2] = -dst[3][2];
}
//- rouz edit (Claude)
