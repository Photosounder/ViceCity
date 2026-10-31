//+ rouz edit (ChatGPT)
#include "TrafficLights_backend.h"
#include "Timer.h"
#include "General.h"
#include "Weather.h"
#include "ModelIndexData.h"
#include "PointLights.h"
#include "BrightLights.h"
#include "EntityStatus.h"
#include <math.h>
#define TRAFFICLIGHTS_PI ((float)3.14159265358979323846)
#define TRAFFICLIGHTS_RADTODEG(x) ((x)*180.0f/TRAFFICLIGHTS_PI)
#define SQR(x) ((x)*(x))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
bool TrafficLights_bGreenLightsCheat;
static TrafficLightVector TrafficLights_Make(float x,float y,float z)
{
    // Materialize the original three-component value
    TrafficLightVector value={x,y,z};return value;
}
static TrafficLightVector TrafficLights_Sub(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original component-wise subtraction
    return TrafficLights_Make(a.x-b.x,a.y-b.y,a.z-b.z);
}
static TrafficLightVector TrafficLights_Add(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original component-wise addition
    return TrafficLights_Make(a.x+b.x,a.y+b.y,a.z+b.z);
}
static TrafficLightVector TrafficLights_Divide(TrafficLightVector value,float divisor)
{
    // Preserve each original component division rather than introducing reciprocal multiplication
    return TrafficLights_Make(value.x/divisor,value.y/divisor,value.z/divisor);
}
static TrafficLightVector TrafficLights_Negate(TrafficLightVector value)
{
    // Preserve the opposite side vector used for the second traffic-light face
    return TrafficLights_Make(-value.x,-value.y,-value.z);
}
static float TrafficLights_Dot(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original ordered three-component dot product
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float TrafficLights_Dot2D(TrafficLightVector a,TrafficLightVector b)
{
    // Preserve the original two-component stopping distance
    return a.x*b.x+a.y*b.y;
}
static float TrafficLights_MagnitudeSqr(TrafficLightVector value)
{
    // Preserve the original three-term squared scan distance
    return value.x*value.x+value.y*value.y+value.z*value.z;
}
static PointLightsVector TrafficLights_PointVector(TrafficLightVector value)
{
    // Enter the existing C point-light registration with the same coordinates
    PointLightsVector result={value.x,value.y,value.z};return result;
}
static BrightLightVector TrafficLights_BrightVector(TrafficLightVector value)
{
    // Enter the existing C bright-light registration with the same coordinates
    BrightLightVector result={value.x,value.y,value.z};return result;
}
bool TrafficLights_DoesLineSegmentIntersect(float l1x1, float l1y1, float l1x2, float l1y2, float l2x1, float l2y1, float l2x2, float l2y2)
{
    // Preserve the complete original DoesLineSegmentIntersect operation and ordered live accesses
	return ((l2y2 - l1y1) * (l1x2 - l1x1) + (l1x1 - l2x2) * (l1y2 - l1y1)) *
		((l2y1 - l1y1) * (l1x2 - l1x1) + (l1x1 - l2x1) * (l1y2 - l1y1)) <= 0.0f &&
		((l1y2 - l2y1) * (l2x2 - l2x1) + (l2y2 - l2y1) * (l2x1 - l1x2)) *
		((l1y1 - l2y1) * (l2x2 - l2x1) + (l2y2 - l2y1) * (l2x1 - l1x1)) <= 0.0f;
}

void
TrafficLights_DisplayActualLight(CEntity *ent)
{
    // Preserve the complete original DisplayActualLight operation and ordered live accesses
	if(TrafficLights_BackendUpZ(ent) < 0.96f || TrafficLights_BackendDamaged(ent))
		return;

	int phase;
	if(TrafficLights_FindTrafficLightType(ent) == 1)
		phase = TrafficLights_LightForCars1_Visual();
	else
		phase = TrafficLights_LightForCars2_Visual();

	int i, m = TrafficLights_BackendModel(ent);
	if (MI_TRAFFICLIGHTS == m) {
		TrafficLightModel *mi = TrafficLights_BackendModelInfo(TrafficLights_BackendModel(ent));
		float x = TrafficLights_BackendEffectX(mi,0);
		float yMin = TrafficLights_BackendEffectY(mi,0);
		float yMax = TrafficLights_BackendEffectY(mi,0);
		float zMin = TrafficLights_BackendEffectZ(mi,0);
		float zMax = TrafficLights_BackendEffectZ(mi,0);
		for (i = 1; i < 6; i++) {
			assert(TrafficLights_BackendHasEffect(mi,i));
			yMin = Min(yMin, TrafficLights_BackendEffectY(mi,i));
			yMax = Max(yMax, TrafficLights_BackendEffectY(mi,i));
			zMin = Min(zMin, TrafficLights_BackendEffectZ(mi,i));
			zMax = Max(zMax, TrafficLights_BackendEffectZ(mi,i));
		}

		TrafficLightVector pos1, pos2;
		uint8_t r, g;
		int id;
		switch (phase) {
		case CAR_LIGHTS_GREEN:
			r = 0;
			g = 255;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMax,zMin));
			pos2 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMin,zMin));
			id = 0;
			break;
		case CAR_LIGHTS_YELLOW:
			r = 255;
			g = 128;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMax,(zMin + zMax) / 2.0f));
			pos2 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMin,(zMin + zMax) / 2.0f));
			id = 1;
			break;
		case CAR_LIGHTS_RED:
			r = 255;
			g = 0;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMax,zMax));
			pos2 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMin,zMax));
			id = 2;
			break;
		default:
			r = 0;
			g = 0;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMax,(zMin + zMax) / 2.0f));
			pos2 = TrafficLights_BackendTransform(ent,TrafficLights_Make(x,yMin,(zMin + zMax) / 2.0f));
			id = -1;
			break;
		}

		if (Weather_TrafficLightBrightness > 0.5f)
			// Enter the C point-light algorithm with the original owner expressions
			PointLights_AddLight(POINTLIGHTS_LIGHT_POINT,TrafficLights_PointVector(pos1),TrafficLights_PointVector(TrafficLights_Make(0.0f,0.0f,0.0f)),8.0f,r / 255.0f,g / 255.0f,0 / 255.0f,POINTLIGHTS_FOG_NORMAL,true);

		if (Weather_TrafficLightBrightness > 0.05f)
			TrafficLights_BackendShadow((uintptr_t)ent,pos1,r * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f,g * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f,0 * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f);

		if (TrafficLights_Dot(TrafficLights_BackendCameraForward(), TrafficLights_BackendForward(ent)) < 0.0f)
			TrafficLights_BackendCorona((uintptr_t)ent + id,r * TrafficLights_BackendSpriteBrightness() * 0.7f,g * TrafficLights_BackendSpriteBrightness() * 0.7f,0 * TrafficLights_BackendSpriteBrightness() * 0.7f,pos1,1.75f * TrafficLights_BackendSpriteSize());
		else
			TrafficLights_BackendCorona((uintptr_t)ent + id + 3,r * TrafficLights_BackendSpriteBrightness() * 0.7f,g * TrafficLights_BackendSpriteBrightness() * 0.7f,0 * TrafficLights_BackendSpriteBrightness() * 0.7f,pos2,1.75f * TrafficLights_BackendSpriteSize());

		// Enter the C bright-light algorithm with the original arguments
		BrightLights_RegisterOne(TrafficLights_BrightVector(pos1),TrafficLights_BrightVector(TrafficLights_BackendUp(ent)),TrafficLights_BrightVector(TrafficLights_BackendRight(ent)),TrafficLights_BrightVector(TrafficLights_Make(0.0f,0.0f,0.0f)),id + BRIGHTLIGHT_TRAFFIC_GREEN,0,0,0);
		// Enter the C bright-light algorithm with the original arguments
		BrightLights_RegisterOne(TrafficLights_BrightVector(pos2),TrafficLights_BrightVector(TrafficLights_BackendUp(ent)),TrafficLights_BrightVector(TrafficLights_Negate(TrafficLights_BackendRight(ent))),TrafficLights_BrightVector(TrafficLights_Make(0.0f,0.0f,0.0f)),id + BRIGHTLIGHT_TRAFFIC_GREEN,0,0,0);
	}
	else if (MI_TRAFFICLIGHTS_VERTICAL == m) {
		TrafficLightModel *mi = TrafficLights_BackendModelInfo(TrafficLights_BackendModel(ent));
		float x = TrafficLights_BackendEffectX(mi,0);
		float yMin = TrafficLights_BackendEffectY(mi,0);
		float yMax = TrafficLights_BackendEffectY(mi,0);
		float zMin = TrafficLights_BackendEffectZ(mi,0);
		float zMax = TrafficLights_BackendEffectZ(mi,0);
		for (i = 1; i < 6; i++) {
			assert(TrafficLights_BackendHasEffect(mi,i));
			yMin = Min(yMin, TrafficLights_BackendEffectY(mi,i));
			yMax = Max(yMax, TrafficLights_BackendEffectY(mi,i));
			zMin = Min(zMin, TrafficLights_BackendEffectZ(mi,i));
			zMax = Max(zMax, TrafficLights_BackendEffectZ(mi,i));
		}

		TrafficLightVector pos1;
		uint8_t r, g;
		int id;
		switch (phase) {
		case CAR_LIGHTS_GREEN:
			r = 0;
			g = 255;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,2));
			id = 0;
			break;
		case CAR_LIGHTS_YELLOW:
			r = 255;
			g = 128;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,1));
			id = 1;
			break;
		case CAR_LIGHTS_RED:
			r = 255;
			g = 0;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,0));
			id = 2;
			break;
		default:
			r = 0;
			g = 0;
			pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,1));
			id = -1;
			break;
		}

		// Enter the C bright-light algorithm with the original arguments
		BrightLights_RegisterOne(TrafficLights_BrightVector(pos1),TrafficLights_BrightVector(TrafficLights_BackendUp(ent)),TrafficLights_BrightVector(TrafficLights_BackendRight(ent)),TrafficLights_BrightVector(TrafficLights_Make(0.0f,0.0f,0.0f)),id + BRIGHTLIGHT_TRAFFIC_GREEN,0,0,0);

		if (Weather_TrafficLightBrightness > 0.5f)
			// Enter the C point-light algorithm with the original owner expressions
			PointLights_AddLight(POINTLIGHTS_LIGHT_POINT,TrafficLights_PointVector(pos1),TrafficLights_PointVector(TrafficLights_Make(0.0f,0.0f,0.0f)),8.0f,r / 255.0f,g / 255.0f,0 / 255.0f,POINTLIGHTS_FOG_NORMAL,true);

		if (Weather_TrafficLightBrightness > 0.05f)
			TrafficLights_BackendShadow((uintptr_t)ent,pos1,r * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f,g * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f,0 * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f);

		if (TrafficLights_Dot(TrafficLights_BackendCameraForward(), TrafficLights_BackendForward(ent)) < 0.0f)
			TrafficLights_BackendCorona((uintptr_t)ent + id,r * TrafficLights_BackendSpriteBrightness() * 0.7f,g * TrafficLights_BackendSpriteBrightness() * 0.7f,0 * TrafficLights_BackendSpriteBrightness() * 0.7f,pos1,1.75f * TrafficLights_BackendSpriteSize());
	}
	else if (MI_TRAFFICLIGHTS_MIAMI == m || MI_TRAFFICLIGHTS_TWOVERTICAL == m) {
		TrafficLightModel *mi = TrafficLights_BackendModelInfo(TrafficLights_BackendModel(ent));
		TrafficLightVector pos1, pos2;
		uint8_t r, g;
		int id;
		if (MI_TRAFFICLIGHTS_MIAMI == m) {
			switch (phase) {
			case CAR_LIGHTS_GREEN:
				r = 0;
				g = 255;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,4));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,5));
				id = 0;
				break;
			case CAR_LIGHTS_YELLOW:
				r = 255;
				g = 128;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,2));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,3));
				id = 1;
				break;
			case CAR_LIGHTS_RED:
				r = 255;
				g = 0;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,0));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,1));
				id = 2;
				break;
			default:
				r = 0;
				g = 0;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,2));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,3));
				id = -1;
				break;
			}
		}
		else {
			switch (phase) {
			case CAR_LIGHTS_GREEN:
				r = 0;
				g = 255;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,2));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,5));
				id = 0;
				break;
			case CAR_LIGHTS_YELLOW:
				r = 255;
				g = 128;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,1));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,4));
				id = 1;
				break;
			case CAR_LIGHTS_RED:
				r = 255;
				g = 0;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,0));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,3));
				id = 2;
				break;
			default:
				r = 0;
				g = 0;
				pos1 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,1));
				pos2 = TrafficLights_BackendTransform(ent,TrafficLights_BackendEffectPosition(mi,4));
				id = -1;
				break;
			}
		}

		TrafficLightVector pos = TrafficLights_Divide(TrafficLights_Add(pos1,pos2),2);
		if (id >= 0) {
			// Enter the C bright-light algorithm with the original arguments
			BrightLights_RegisterOne(TrafficLights_BrightVector(pos1),TrafficLights_BrightVector(TrafficLights_BackendUp(ent)),TrafficLights_BrightVector(TrafficLights_BackendRight(ent)),TrafficLights_BrightVector(TrafficLights_Make(0.0f,0.0f,0.0f)),id + BRIGHTLIGHT_TRAFFIC_GREEN,0,0,0);
			// Enter the C bright-light algorithm with the original arguments
			BrightLights_RegisterOne(TrafficLights_BrightVector(pos2),TrafficLights_BrightVector(TrafficLights_BackendUp(ent)),TrafficLights_BrightVector(TrafficLights_BackendRight(ent)),TrafficLights_BrightVector(TrafficLights_Make(0.0f,0.0f,0.0f)),id + BRIGHTLIGHT_TRAFFIC_GREEN,0,0,0);
		}

		if (Weather_TrafficLightBrightness > 0.5f)
			// Enter the C point-light algorithm with the original owner expressions
			PointLights_AddLight(POINTLIGHTS_LIGHT_POINT,TrafficLights_PointVector(pos),TrafficLights_PointVector(TrafficLights_Make(0.0f,0.0f,0.0f)),8.0f,r / 255.0f,g / 255.0f,0 / 255.0f,POINTLIGHTS_FOG_NORMAL,true);

		if (Weather_TrafficLightBrightness > 0.05f)
			TrafficLights_BackendShadow((uintptr_t)ent,pos,r * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f,g * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f,0 * TrafficLights_BackendGroundBrightness() * Weather_TrafficLightBrightness / 8.0f);

		if (id >= 0) {
			if (TrafficLights_Dot(TrafficLights_BackendCameraForward(), TrafficLights_BackendForward(ent)) > 0.0f)
				TrafficLights_BackendCorona((uintptr_t)ent + id,r * TrafficLights_BackendSpriteBrightness() * 0.7f,g * TrafficLights_BackendSpriteBrightness() * 0.7f,0 * TrafficLights_BackendSpriteBrightness() * 0.7f,pos1,1.75f * TrafficLights_BackendSpriteSize());
			else
				TrafficLights_BackendCorona((uintptr_t)ent + id,r * TrafficLights_BackendSpriteBrightness() * 0.7f,g * TrafficLights_BackendSpriteBrightness() * 0.7f,0 * TrafficLights_BackendSpriteBrightness() * 0.7f,pos2,1.75f * TrafficLights_BackendSpriteSize());
		}
	}
}

void
TrafficLights_ScanForLightsOnMap(void)
{
    // Preserve the complete original ScanForLightsOnMap operation and ordered live accesses
	int x, y;
	int i, j, k, l;
	TrafficLightNode *node;

	for(x = 0; x < TrafficLights_BackendNumSectorsX(); x++)
	for(y = 0; y < TrafficLights_BackendNumSectorsY(); y++){
		TrafficLightList *list = TrafficLights_BackendSectorList(x,y);
		for(node = TrafficLights_BackendFirst(list); node; node = TrafficLights_BackendNext(node)){
			CEntity *light = TrafficLights_BackendNodeEntity(node);
			if (!IsTrafficLight(TrafficLights_BackendModel(light)))
				continue;

			TrafficLightVector pos1 = TrafficLights_BackendTransform(light,TrafficLights_Make(17.0f,0.0f,0.0f));
			TrafficLightVector pos2 = TrafficLights_BackendTransform(light,TrafficLights_Make(-15.0f,0.0f,0.0f));

			// Check cars
			for(i = 0; i < TrafficLights_BackendCarNodeCount(); i++){
				if (TrafficLights_MagnitudeSqr(TrafficLights_Sub(TrafficLights_BackendPathPosition(i),pos1)) >= SQR(100.0f))
					continue;
				for (j = 0; j < TrafficLights_BackendNumLinks(i); j++){
					int con = TrafficLights_BackendConnectedNode(TrafficLights_BackendFirstLink(i) + j);
					if (i < con) {
						TrafficLightVector i_pos = TrafficLights_BackendPathPosition(i);
						TrafficLightVector con_pos = TrafficLights_BackendPathPosition(con);
						if (fabsf(pos1.z - (i_pos.z + con_pos.z) / 2) < 10.0f &&
							TrafficLights_DoesLineSegmentIntersect(pos1.x, pos1.y, pos2.x, pos2.y, i_pos.x, i_pos.y, con_pos.x, con_pos.y)) {
							//debug("Setting up light: nodes %f %f %f - %f %f %f, light %f %f %f - %f %f %f\n", i_pos.x, i_pos.y, i_pos.z, con_pos.x, con_pos.y, con_pos.z, pos1.x, pos1.y, pos1.z, pos2.x, pos2.y, pos2.z);
							int link = TrafficLights_BackendCarConnection(TrafficLights_BackendFirstLink(i) + j);
							TrafficLights_BackendSetLightType(link,TrafficLights_FindTrafficLightType(light));
							if (TrafficLights_BackendNumLinks(i) > TrafficLights_BackendNumLinks(con))
								con = i;
							if (TrafficLights_BackendLinkNode(link) != con)
								TrafficLights_BackendSetLightDirection(link,true);
						}
					}
				}
			}

			// Check peds
			for(i = TrafficLights_BackendCarNodeCount(); i < TrafficLights_BackendNodeCount(); i++){
				float dist1, dist2;
				dist1 = fabsf(TrafficLights_BackendPathX(i) - TrafficLights_BackendPositionX(light)) +
					fabsf(TrafficLights_BackendPathY(i) - TrafficLights_BackendPositionY(light));
				if(dist1 < 50.0f){
					for(l = 0; l < TrafficLights_BackendNumLinks(i); l++){
						j = TrafficLights_BackendFirstLink(i) + l;
						if(TrafficLights_BackendCrossesRoad(j)){
							k = TrafficLights_BackendConnectedNode(j);
							dist2 = fabsf(TrafficLights_BackendPathX(k) - TrafficLights_BackendPositionX(light)) +
								fabsf(TrafficLights_BackendPathY(k) - TrafficLights_BackendPositionY(light));
							if(dist1 < 15.0f || dist2 < 15.0f)
								TrafficLights_BackendSetConnectionLight(j);
						}
					}
				}
			}
		}
	}
}

int32_t
TrafficLights_FindTrafficLightType(CEntity *light)
{
    // Preserve the complete original FindTrafficLightType operation and ordered live accesses
	float orientation = TRAFFICLIGHTS_RADTODEG(General_GetATanOfXY(TrafficLights_BackendForwardX(light), TrafficLights_BackendForwardY(light)));
	if((orientation > 60.0f && orientation < 60.0f + 90.0f) ||
	   (orientation > 240.0f && orientation < 240.0f + 90.0f))
		return 1;
	return 2;
}

uint8_t
TrafficLights_LightForPeds(void)
{
    // Preserve the complete original LightForPeds operation and ordered live accesses
	uint32_t period = (*Timer_GetTimeInMilliseconds()) % 16384;

	if(period < 12000)
		return PED_LIGHTS_DONT_WALK;
	else if(period < 16384 - 1000)
		return PED_LIGHTS_WALK;
	else
		return PED_LIGHTS_WALK_BLINK;
}

uint8_t
TrafficLights_LightForCars1(void)
{
    // Preserve the complete original LightForCars1 operation and ordered live accesses
	if (Weather_Wind > 1.1f)
		return CAR_LIGHTS_GREEN;

	if (TrafficLights_bGreenLightsCheat)
		return CAR_LIGHTS_GREEN;

	uint32_t period = (*Timer_GetTimeInMilliseconds()) % 16384;

	if(period < 5000)
		return CAR_LIGHTS_GREEN;
	else if(period < 5000 + 1000)
		return CAR_LIGHTS_YELLOW;
	else
		return CAR_LIGHTS_RED;
}

uint8_t
TrafficLights_LightForCars2(void)
{
    // Preserve the complete original LightForCars2 operation and ordered live accesses
	if (Weather_Wind > 1.1f)
		return CAR_LIGHTS_GREEN;

	if (TrafficLights_bGreenLightsCheat)
		return CAR_LIGHTS_GREEN;

	uint32_t period = (*Timer_GetTimeInMilliseconds()) % 16384;

	if(period < 6000)
		return CAR_LIGHTS_RED;
	else if(period < 12000 - 1000)
		return CAR_LIGHTS_GREEN;
	else if(period < 12000)
		return CAR_LIGHTS_YELLOW;
	else
		return CAR_LIGHTS_RED;
}

uint8_t
TrafficLights_LightForCars1_Visual(void)
{
    // Preserve the complete original LightForCars1_Visual operation and ordered live accesses
	if (Weather_Wind <= 1.1f)
		return TrafficLights_LightForCars1();
	return ((*Timer_GetTimeInMilliseconds()) & 0x400 ? CAR_LIGHTS_NONE : CAR_LIGHTS_YELLOW);
}

uint8_t
TrafficLights_LightForCars2_Visual(void)
{
    // Preserve the complete original LightForCars2_Visual operation and ordered live accesses
	if (Weather_Wind <= 1.1f)
		return TrafficLights_LightForCars2();
	return ((*Timer_GetTimeInMilliseconds()) & 0x400 ? CAR_LIGHTS_NONE : CAR_LIGHTS_YELLOW);
}

bool
TrafficLights_ShouldCarStopForLight(CVehicle *vehicle, bool alwaysStop)
{
    // Preserve the complete original ShouldCarStopForLight operation and ordered live accesses
	int node, type;
	bool direction;

	node = TrafficLights_BackendNextPathNodeInfo(vehicle);
	type = TrafficLights_BackendLightType(node);
	direction = TrafficLights_BackendLightDirection(node);

	if(type){
		if((direction || TrafficLights_BackendLinkNode(node) == TrafficLights_BackendNextRouteNode(vehicle)) &&
		   (!direction || TrafficLights_BackendLinkNode(node) != TrafficLights_BackendNextRouteNode(vehicle)))
			if(alwaysStop ||
			   type == 1 && TrafficLights_LightForCars1() != CAR_LIGHTS_GREEN ||
			   type == 2 && TrafficLights_LightForCars2() != CAR_LIGHTS_GREEN){
				float dist = TrafficLights_Dot2D(TrafficLights_Sub(TrafficLights_BackendVehiclePosition(vehicle),TrafficLights_BackendLinkPosition(node)),
						TrafficLights_BackendLinkDirection(node));
				if(TrafficLights_BackendNextDirection(vehicle) == -1){
					if(dist > 0.0f && dist < 8.0f)
						return true;
				}else{
					if(dist < 0.0f && dist > -8.0f)
						return true;
				}
			}
	}

	node = TrafficLights_BackendCurrentPathNodeInfo(vehicle);
	type = TrafficLights_BackendLightType(node);
	direction = TrafficLights_BackendLightDirection(node);
	if(type){
		if((direction || TrafficLights_BackendLinkNode(node) == TrafficLights_BackendCurrentRouteNode(vehicle)) &&
		   (!direction || TrafficLights_BackendLinkNode(node) != TrafficLights_BackendCurrentRouteNode(vehicle)))
			if(alwaysStop ||
			   type == 1 && TrafficLights_LightForCars1() != CAR_LIGHTS_GREEN ||
			   type == 2 && TrafficLights_LightForCars2() != CAR_LIGHTS_GREEN){
				float dist = TrafficLights_Dot2D(TrafficLights_Sub(TrafficLights_BackendVehiclePosition(vehicle),TrafficLights_BackendLinkPosition(node)),
						TrafficLights_BackendLinkDirection(node));
				if(TrafficLights_BackendCurrentDirection(vehicle) == -1){
					if(dist > 0.0f && dist < 8.0f)
						return true;
				}else{
					if(dist < 0.0f && dist > -8.0f)
						return true;
				}
			}
	}

	if(TrafficLights_BackendVehicleStatus(vehicle) == STATUS_PHYSICS){
		node = TrafficLights_BackendPreviousPathNodeInfo(vehicle);
		type = TrafficLights_BackendLightType(node);
		direction = TrafficLights_BackendLightDirection(node);
		if(type){
			if((direction || TrafficLights_BackendLinkNode(node) == TrafficLights_BackendPrevRouteNode(vehicle)) &&
			   (!direction || TrafficLights_BackendLinkNode(node) != TrafficLights_BackendPrevRouteNode(vehicle)))
				if(alwaysStop ||
				   type == 1 && TrafficLights_LightForCars1() != CAR_LIGHTS_GREEN ||
				   type == 2 && TrafficLights_LightForCars2() != CAR_LIGHTS_GREEN){
					float dist = TrafficLights_Dot2D(TrafficLights_Sub(TrafficLights_BackendVehiclePosition(vehicle),TrafficLights_BackendLinkPosition(node)),
							TrafficLights_BackendLinkDirection(node));
					if(TrafficLights_BackendPreviousDirection(vehicle) == -1){
						if(dist > 0.0f && dist < 6.0f)
							return true;
					}else{
						if(dist < 0.0f && dist > -6.0f)
							return true;
					}
				}
		}
	}

	return false;
}

bool
TrafficLights_ShouldCarStopForBridge(CVehicle *vehicle)
{
    // Preserve the complete original ShouldCarStopForBridge operation and ordered live accesses
#ifdef GTA_BRIDGE
	return TrafficLights_BackendBridgeLight(TrafficLights_BackendNextPathNodeInfo(vehicle)) &&
		!TrafficLights_BackendBridgeLight(TrafficLights_BackendCurrentPathNodeInfo(vehicle));
#else
	return false;
#endif
}
//- rouz edit (ChatGPT)
