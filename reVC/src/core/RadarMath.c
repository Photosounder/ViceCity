//+ rouz edit (Claude)
#include <math.h>
#include "RadarMath.h"
#include "RadarTrace.h"

uint8_t RadarMath_CalculateBlipAlpha(bool menuMapActive, float dist)
{
	// The menu map shows everything fully
	if (menuMapActive)
		return 255;

	if (dist <= 1.0f)
		return 255;

	// Fade to half opacity between 1 and 10 units
	if (dist <= 10.0f)
		return (128.0f * ((dist - 1.0f) / 9.0f)) + ((1.0f - (dist - 1.0f) / 9.0f) * 255.0f);

	return 128;
}

// Clip a polygon against the half plane a*x + b*y + c >= 0
static int ClipPolyPlane(const float *in, int nin, float *out, float a, float b, float c)
{
	int j;
	int nout = 0;

	for (j = 0; j < nin; j++) {
		int x1 = j;
		int x2 = (j+1) % nin;
		float d1 = a*in[x1*2] + b*in[x1*2+1] + c;
		float d2 = a*in[x2*2] + b*in[x2*2+1] + c;

		// Add the crossing point
		if (d1*d2 < 0.0f) {
			float t = d1/(d1 - d2);
			out[nout*2] = in[x1*2]*(1.0f-t) + in[x2*2]*t;
			out[nout*2+1] = in[x1*2+1]*(1.0f-t) + in[x2*2+1]*t;
			nout++;
		}

		// And the end point if it is inside
		if (d2 >= 0.0f) {
			out[nout*2] = in[x2*2];
			out[nout*2+1] = in[x2*2+1];
			nout++;
		}
	}
	return nout;
}

int RadarMath_ClipRadarPoly(float *poly, const float *rect)
{
	float tmp[16];
	int n;

	// Clip against the four sides of the radar square in turn
	if (n = ClipPolyPlane(rect, 4, tmp, -1.0f, 0.0f, 1.0f), n == 0) return 0;
	if (n = ClipPolyPlane(tmp, n, poly, 1.0f, 0.0f, 1.0f), n == 0) return 0;
	if (n = ClipPolyPlane(poly, n, tmp, 0.0f, -1.0f, 1.0f), n == 0) return 0;
	if (n = ClipPolyPlane(tmp, n, poly, 0.0f, 1.0f, 1.0f), n == 0) return 0;
	return n;
}

uint32_t RadarMath_GetRadarTraceColour(uint32_t color, bool bright)
{
	// Dark and bright variant of each named colour
	switch (color) {
	case RADAR_TRACE_RED: return bright ? 0x712B49FF : 0x7F0000FF;
	case RADAR_TRACE_GREEN: return bright ? 0x5FA06AFF : 0x007F00FF;
	case RADAR_TRACE_LIGHT_BLUE: return bright ? 0x80A7F3FF : 0x00007FFF;
	case RADAR_TRACE_GRAY: return bright ? 0xE1E1E1FF : 0x7F7F7FFF;
	case RADAR_TRACE_YELLOW: return bright ? 0xFFFF00FF : 0x7F7F00FF;
	case RADAR_TRACE_MAGENTA: return bright ? 0xFF00FFFF : 0x7F007FFF;
	case RADAR_TRACE_CYAN: return bright ? 0x00FFFFFF : 0x007F7FFF;
	default: return color;
	}
}

float RadarMath_LimitRadarPoint(float *point, bool menuMapActive)
{
	float dist = sqrtf(point[0]*point[0] + point[1]*point[1]);
	float invdist;

	// The menu map is not limited
	if (menuMapActive)
		return dist;

	// Scale back to the circle edge
	if (dist > 1.0f) {
		invdist = 1.0f / dist;
		point[0] *= invdist;
		point[1] *= invdist;
	}
	return dist;
}

void RadarMath_TransformRealWorldPointToRadarSpace(float *out, const float *in, float s, float c, const float *origin, float radarRange)
{
	// Scale to radar units then rotate
	float x = (in[0] - origin[0]) * (1.0f / radarRange);
	float y = (in[1] - origin[1]) * (1.0f / radarRange);

	out[0] = s * y + c * x;
	out[1] = c * y - s * x;
}

void RadarMath_TransformRadarPointToRealWorldSpace(float *out, const float *in, float s, float c, const float *origin, float radarRange)
{
	// Rotate back, scale up and move to the origin
	float ns = -s;
	float x = ns * in[1] + c * in[0];
	float y = c * in[1] - ns * in[0];

	out[0] = x * radarRange + origin[0];
	out[1] = y * radarRange + origin[1];
}
int32_t RadarMath_GetActualBlipArrayIndex(const sRadarTraceData *traces, int32_t handle)
{
	// -1 and handles whose counter no longer matches are invalid
	if (handle == -1)
		return -1;
	else if ((handle & 0xFFFF0000) >> 16 != traces[(uint16_t)handle].m_BlipIndex)
		return -1;
	else
		return (uint16_t)handle;
}

int32_t RadarMath_GetNewUniqueBlipIndex(sRadarTraceData *traces, int32_t index)
{
	// Count up, skipping zero and the largest value on wrap-around
	if (traces[index].m_BlipIndex >= UINT16_MAX - 1)
		traces[index].m_BlipIndex = 1;
	else
		traces[index].m_BlipIndex++;
	return index | (traces[index].m_BlipIndex << 16);
}
int Radar_BlipHeightMode(float blipZ, float playerZ)
{
	int mode = 0;
	if (blipZ - playerZ <= 2.0f) {
		if (blipZ - playerZ < -4.0f) mode = 1;
		else mode = 2;
	}
	return mode;
}

float Radar_DebugRadiusStep(float radius)
{
	radius = radius - 0.1f;
	if (radius < 1.0f)
		radius = 5.0f;
	return radius;
}

void Radar_MarkerCorner(int corner, float x, float y, float sizeX, float sizeY, float correctedAngle, float *outX, float *outY)
{
	const float pi = (float)3.14159265358979323846;
	const float halfpi = pi / 2;
	const float cornerAngle = corner * halfpi + correctedAngle;

	*outX = x + (0.0f * cosf(cornerAngle) + 1.0f * sinf(cornerAngle)) * sizeX;
	*outY = y - (0.0f * sinf(cornerAngle) - 1.0f * cosf(cornerAngle)) * sizeY;
}

void Radar_QuarterCirclePoint(float cornerX, float cornerY, int step, float *outX, float *outY)
{
	const float pi = (float)3.14159265358979323846;

	*outX = cornerX * cosf(step * (pi / 2.0f / 6.0f));
	*outY = cornerY * sinf(step * (pi / 2.0f / 6.0f));
}
//- rouz edit (Claude)
