//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "GameLevels.h"
#include "../peds/GangTypes.h"
#include "../control/CarClasses.h"
typedef enum eZoneType
{
	ZONE_DEFAULT,
	ZONE_NAVIG,
	ZONE_INFO,
	ZONE_MAPZONE,
} eZoneType;

typedef struct CZone
{
	char       name[8];
	float      minx;
	float      miny;
	float      minz;
	float      maxx;
	float      maxy;
	float      maxz;
	eZoneType  type;
	eLevelName level;
	int16_t      zoneinfoDay;
	int16_t      zoneinfoNight;
	struct CZone *child;
	struct CZone *parent;
	struct CZone *next;

} CZone;

typedef struct CZoneInfo
{
	// Car data
	int16_t carDensity;
	int16_t carThreshold[CARCLASS_NUM_CAR_CLASSES];
	int16_t boatThreshold[CARCLASS_NUM_BOAT_CLASSES];
	int16_t gangThreshold[NUM_GANGS];
	int16_t copThreshold;

	// Ped data
	uint16_t pedDensity;
	uint16_t gangPedThreshold[NUM_GANGS];
	uint16_t copPedThreshold;
	uint16_t pedGroup;
} CZoneInfo;



//- rouz edit (ChatGPT)
