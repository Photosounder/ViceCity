//+ rouz edit (ChatGPT)
#include "General.h"
#include "Random.h"
#include <ctype.h>
#include <math.h>
#define GENERAL_PI ((float)3.14159265358979323846)
#define GENERAL_TWOPI (GENERAL_PI * 2)
#define GENERAL_DEGTORAD(x) ((x) * GENERAL_PI / 180.0f)
#define GENERAL_RADTODEG(x) ((x) * 180.0f / GENERAL_PI)
#define GENERAL_CLAMP(v,low,high) ((v) < (low) ? (low) : (v) > (high) ? (high) : (v))
#ifdef USE_PS2_RAND
#define GENERAL_RAND_MAX 65535
#else
#define GENERAL_RAND_MAX 32767
#endif

float General_GetATanOfXY(float x, float y)
{
	// Compute the original quadrant angle using float ratios
	if(x == 0.0f && y == 0.0f)
		return 0.0f;
	float xabs = fabsf(x);
	float yabs = fabsf(y);
	
	if(xabs < yabs){
		if(y > 0.0f){
			if(x > 0.0f)
				return 0.5f*GENERAL_PI - atan2f(x / y, 1.0f);
			else
				return 0.5f*GENERAL_PI + atan2f(-x / y, 1.0f);
		}else{
			if(x > 0.0f)
				return 1.5f*GENERAL_PI + atan2f(x / -y, 1.0f);
			else
				return 1.5f*GENERAL_PI - atan2f(-x / -y, 1.0f);
		}
	}else{
		if(y > 0.0f){
			if(x > 0.0f)
				return atan2f(y / x, 1.0f);
			else
				return GENERAL_PI - atan2f(y / -x, 1.0f);
		}else{
			if(x > 0.0f)
				return 2.0f*GENERAL_PI - atan2f(-y / x, 1.0f);
			else
				return GENERAL_PI + atan2f(-y / -x, 1.0f);
		}
	}
}

float General_LimitAngle(float angle)
{
	// Wrap degrees into the original half-open range
	float result = angle;
	
	while (result >= 180.0f) {
		result -= 2 * 180.0f;
	}
	
	while (result < -180.0f) {
		result += 2 * 180.0f;
	}
	
	return result;
}

float General_LimitRadianAngle(float angle)
{
	// Clamp and wrap radians at the original float boundaries
	float result = GENERAL_CLAMP(angle, -25.0f, 25.0f);
	
	while (result >= GENERAL_PI) {
		result -= 2 * GENERAL_PI;
	}
	
	while (result < -GENERAL_PI) {
		result += 2 * GENERAL_PI;
	}
	
	return result;
}

float General_GetRadianAngleBetweenPoints(float x1, float y1, float x2, float y2)
{
	// Compute point-to-point radians with the original zero-y adjustment
	float x = x2 - x1;
	float y = y2 - y1;
	
	if (y == 0.0f)
		y = 0.0001f;
	
	if (x > 0.0f) {
		if (y > 0.0f)
			return GENERAL_PI - atan2f(x / y, 1.0f);
		else
			return -atan2f(x / y, 1.0f);
	} else {
		if (y > 0.0f)
			return -(GENERAL_PI + atan2f(x / y, 1.0f));
		else
			return -atan2f(x / y, 1.0f);
	}
}

float General_GetAngleBetweenPoints(float x1, float y1, float x2, float y2)
{
	// Convert the original point-to-point angle into degrees
	return GENERAL_RADTODEG(General_GetRadianAngleBetweenPoints(x1, y1, x2, y2));
}

int General_GetNodeHeadingFromVector(float x, float y)
{
	// Map the vector angle to the original eight heading sectors
	float angle = General_GetRadianAngleBetweenPoints(x, y, 0.0f, 0.0f);
	if (angle < 0.0f)
		angle += GENERAL_TWOPI;
	
	angle = GENERAL_DEGTORAD(22.5f) + GENERAL_TWOPI - angle;
	
	if (angle >= GENERAL_TWOPI)
		angle -= GENERAL_TWOPI;
	
	return (int)floorf(angle / GENERAL_DEGTORAD(45.0f));
}

bool General_faststrcmp(const char *str1, const char *str2)
{
	// Report string differences including unequal lengths
	for (; *str1; str1++, str2++) {
		if (*str1 != *str2)
			return true;
	}
	return *str2 != '\0';
}

bool General_faststrncmp(const char *str1, const char *str2, uint32_t count)
{
	// Compare the first string prefix up to the original count
	for(uint32_t i = 0; *str1 && i < count; str1++, str2++, i++) {
		if (*str1 != *str2)
			return true;
	}
	return false;
}

bool General_faststricmp(const char *str1, const char *str2)
{
	// Compare case-folded characters using the configured implementation
	for (; *str1; str1++, str2++) {
#ifndef ASCII_STRCMP
				if (toupper(*str1) != toupper(*str2))
#else
				if (__ascii_toupper(*str1) != __ascii_toupper(*str2))
#endif
					return true;
			}
			return *str2 != '\0';
}

bool General_SolveQuadratic(float a, float b, float c, float *root1, float *root2)
{
	// Leave failed outputs untouched and write the second root before the first
	float discriminant = b * b - 4.f * a * c;
	if (discriminant < 0.f)
		return false;
	
	float discriminantSqrt = sqrtf(discriminant);
	*root2 = (-b + discriminantSqrt) / (2.f * a);
	*root1 = (-b - discriminantSqrt) / (2.f * a);
	return true;
}

uint16_t General_GetRandomNumber(void)
{
	// Consume one random draw and apply the configured mask
	return myrand() & GENERAL_RAND_MAX;
}

bool General_GetRandomTrueFalse(void)
{
	// Compare one random draw against the original Boolean threshold
	return General_GetRandomNumber() < GENERAL_RAND_MAX / 2;
}

float General_GetRandomNumberInRangeFloat(float low, float high)
{
	// Consume one draw and retain the original floating-point range formula
	return low + (high - low)*(General_GetRandomNumber()/(float)(GENERAL_RAND_MAX + 1));
}

int32_t General_GetRandomNumberInRangeInt(int32_t low, int32_t high)
{
	// Consume one draw and retain the original floating-point range formula
	return low + (high - low)*(General_GetRandomNumber()/(float)(GENERAL_RAND_MAX + 1));
}

void General_SetRandomSeed(int32_t seed)
{
	// Pass the original signed seed through the random provider conversion
	mysrand(seed);
}

//- rouz edit (ChatGPT)
