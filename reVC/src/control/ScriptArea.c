//+ rouz edit (Claude)
#include <math.h>
#include "ScriptArea.h"
#include "General.h"

bool ScriptArea_InCenteredBox2D(float X, float Y, float dX, float dY, float posX, float posY)
{
	return X - dX <= posX &&
		X + dX >= posX &&
		Y - dY <= posY &&
		Y + dY >= posY;
}

bool ScriptArea_InCenteredBox3D(float X, float Y, float Z, float dX, float dY, float dZ, float posX, float posY, float posZ)
{
	return X - dX <= posX &&
		X + dX >= posX &&
		Y - dY <= posY &&
		Y + dY >= posY &&
		Z - dZ <= posZ &&
		Z + dZ >= posZ;
}

bool ScriptArea_InMinMaxBox2D(float infX, float infY, float supX, float supY, float posX, float posY)
{
	return infX <= posX &&
		supX >= posX &&
		infY <= posY &&
		supY >= posY;
}

bool ScriptArea_InMinMaxBox3D(float infX, float infY, float infZ, float supX, float supY, float supZ, float posX, float posY, float posZ)
{
	return infX <= posX &&
		supX >= posX &&
		infY <= posY &&
		supY >= posY &&
		infZ <= posZ &&
		supZ >= posZ;
}
void ScriptArea_AngledGeometry(float infX, float infY, float supX, float supY, float side2length, ScriptAngledArea *area)
{
	const float pi = (float)3.14159265358979323846;

	// Direction of the second side, a quarter turn from the first
	float initAngle = General_GetRadianAngleBetweenPoints(infX, infY, supX, supY) + pi/2;
	while (initAngle < 0.0f)
		initAngle += pi*2;
	while (initAngle > pi*2)
		initAngle -= pi*2;

	// It looks like the idea is to use a rectangle using the diagonal of the rectangle as
	// the side of new rectangle, with "length" being the length of second side
	area->infX = infX;
	area->infY = infY;
	area->rotatedSupX = supX + side2length * sinf(initAngle);
	area->rotatedSupY = supY - side2length * cosf(initAngle);
	area->rotatedInfX = infX + side2length * sinf(initAngle);
	area->rotatedInfY = infY - side2length * cosf(initAngle);
	area->side1X = supX - infX;
	area->side1Y = supY - infY;
	area->side1Length = sqrtf(area->side1X*area->side1X + area->side1Y*area->side1Y);
	area->side2X = area->rotatedInfX - infX;
	area->side2Y = area->rotatedInfY - infY;
	area->side2Length = sqrtf(area->side2X*area->side2X + area->side2Y*area->side2Y);
}

bool ScriptArea_InAngledArea(const ScriptAngledArea *area, float posX, float posY, float posZ, float infZ, float supZ, bool is3D)
{
	float X = posX - area->infX;
	float Y = posY - area->infY;
	float positionAlongSide1 = X * area->side1X / area->side1Length + Y * area->side1Y / area->side1Length;
	bool in_area = false;
	if (positionAlongSide1 >= 0.0f && positionAlongSide1 <= area->side1Length) {
		float positionAlongSide2 = X * area->side2X / area->side2Length + Y * area->side2Y / area->side2Length;
		if (positionAlongSide2 >= 0.0f && positionAlongSide2 <= area->side2Length) {
			in_area = !is3D || (posZ >= infZ && posZ <= supZ);
		}
	}
	return in_area;
}

void ScriptArea_SortPair(float *inf, float *sup)
{
	if (*inf > *sup) {
		float tmp = *inf;
		*inf = *sup;
		*sup = tmp;
	}
}

static float ScriptArea_LimitAngleOnCircle(float angle)
{
	return angle < 0.0f ? angle + 360.0f : angle;
}

float ScriptArea_RotateObjectHeading(float heading, float headingTarget, float rotateBy)
{
	float angleClockwise = ScriptArea_LimitAngleOnCircle(headingTarget - heading);
	float angleCounterclockwise = ScriptArea_LimitAngleOnCircle(heading - headingTarget);

	if (angleClockwise < angleCounterclockwise)
		return rotateBy < angleClockwise ? heading + rotateBy : headingTarget;
	return rotateBy < angleCounterclockwise ? heading - rotateBy : headingTarget;
}
float ScriptArea_SlideAxis(float pos, float posTarget, float slideBy)
{
	float posDiff = pos - posTarget;

	if (posDiff < 0)
		return -posDiff < slideBy ? posTarget : pos + slideBy;
	return posDiff < slideBy ? posTarget : pos - slideBy;
}
float ScriptArea_HeadingDegrees(float radians)
{
	const float pi = (float)3.14159265358979323846;
	float angle = radians * 180.0f / pi;

	if (angle < 0.0f)
		angle += 360.0f;
	if (angle > 360.0f)
		angle -= 360.0f;
	return angle;
}
float ScriptArea_Min4(float a, float b, float c, float d)
{
	float m = c < d ? c : d;

	m = b < m ? b : m;
	return a < m ? a : m;
}

float ScriptArea_Max4(float a, float b, float c, float d)
{
	float m = c > d ? c : d;

	m = b > m ? b : m;
	return a > m ? a : m;
}

float ScriptArea_Radius(float centerX, float centerY, float infX, float infY)
{
	float dx = centerX - infX;
	float dy = centerY - infY;

	return dx > dy ? dx : dy;
}
void ScriptArea_AngledBounds(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float *infX, float *supX, float *infY, float *supY)
{
	float X, Y;

	X = (x1 + x2) / 2;
	Y = (y1 + y2) / 2;
	*supX = *infX = X;
	*supY = *infY = Y;
	X = (x2 + x3) / 2;
	Y = (y2 + y3) / 2;
	*infX = *infX < X ? *infX : X;
	*supX = *supX > X ? *supX : X;
	*infY = *infY < Y ? *infY : Y;
	*supY = *supY > Y ? *supY : Y;
	X = (x3 + x4) / 2;
	Y = (y3 + y4) / 2;
	*infX = *infX < X ? *infX : X;
	*supX = *supX > X ? *supX : X;
	*infY = *infY < Y ? *infY : Y;
	*supY = *supY > Y ? *supY : Y;
	X = (x4 + x1) / 2;
	Y = (y4 + y1) / 2;
	*infX = *infX < X ? *infX : X;
	*supX = *supX > X ? *supX : X;
	*infY = *infY < Y ? *infY : Y;
	*supY = *supY > Y ? *supY : Y;
}
void ScriptArea_OrderedBounds(float x1, float y1, float x2, float y2, float *infX, float *supX, float *infY, float *supY)
{
	if (x1 < x2) {
		*infX = x1;
		*supX = x2;
	} else {
		*infX = x2;
		*supX = x1;
	}
	if (y1 < y2) {
		*infY = y1;
		*supY = y2;
	} else {
		*infY = y2;
		*supY = y1;
	}
}
float ScriptMath_SinDegrees(float degrees)
{
	const float pi = (float)3.14159265358979323846;

	return sinf(degrees * pi / 180.0f);
}

float ScriptMath_CosDegrees(float degrees)
{
	const float pi = (float)3.14159265358979323846;

	return cosf(degrees * pi / 180.0f);
}

float ScriptMath_Sqrt(float value)
{
	return sqrtf(value);
}

float ScriptMath_GroundDirection(float component, float forwardX, float forwardY)
{
	return component / sqrtf(forwardX * forwardX + forwardY * forwardY);
}
float ScriptMath_ArmourAfterAdd(float armour, int32_t add, float maxArmour)
{
	float v = armour + add;

	return v < 0.0f ? 0.0f : v > maxArmour ? maxArmour : v;
}

float ScriptMath_Distance2D(float ax, float ay, float bx, float by)
{
	float dx = ax - bx;
	float dy = ay - by;

	return sqrtf(dx * dx + dy * dy);
}

float ScriptMath_Distance3D(float ax, float ay, float az, float bx, float by, float bz)
{
	float dx = ax - bx;
	float dy = ay - by;
	float dz = az - bz;

	return sqrtf(dx * dx + dy * dy + dz * dz);
}
bool ScriptMath_IsUpsideDown(float upZ, float moveX, float moveY, float moveZ, float turnX, float turnY, float turnZ, float upThreshold, float moveThreshold, float turnThreshold)
{
	return upZ <= upThreshold &&
		sqrtf(moveX * moveX + moveY * moveY + moveZ * moveZ) < moveThreshold &&
		sqrtf(turnX * turnX + turnY * turnY + turnZ * turnZ) < turnThreshold;
}
//- rouz edit (Claude)
