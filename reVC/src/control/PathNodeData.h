//+ rouz edit (Claude)
#pragma once
#include <stdint.h>

// The plain data of a road/pedestrian path node, usable from C
typedef struct CPathNodeData {
	int16_t prevIndex;
	int16_t nextIndex;
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t distance;		// in path search
	int16_t firstLink;
	uint8_t width;
	int8_t group;

	uint8_t numLinks : 4;
	uint8_t bDeadEnd : 1;
	uint8_t bDisabled : 1;
	uint8_t bBetweenLevels : 1;
	uint8_t bUseInRoadBlock : 1;

	uint8_t bWaterPath : 1;
	uint8_t bOnlySmallBoats : 1;
	uint8_t bSelected : 1;
	uint8_t speedLimit : 2;
	//uint8_t flagB20 : 1;
	//uint8_t flagB40 : 1;
	//uint8_t flagB80 : 1;

	uint8_t spawnRate : 4;
	uint8_t flagsC : 4;
} CPathNodeData;

// The plain data of a road link used by cars, usable from C
typedef struct CCarPathLinkData {
	int16_t x;
	int16_t y;
	int16_t pathNodeIndex;
	int8_t dirX;
	int8_t dirY;
	int8_t numLeftLanes : 3;
	int8_t numRightLanes : 3;
	uint8_t trafficLightDirection : 1;
	uint8_t trafficLightType : 2;
	uint8_t bBridgeLights : 1;	// at least in LCS...
	uint8_t width;
} CCarPathLinkData;
//- rouz edit (Claude)
