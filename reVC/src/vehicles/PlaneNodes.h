//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
#include "Vector.h"
#define PLANE_VEC3 CVector
#else
typedef struct PlaneVec3 {float x,y,z;} PlaneVec3;
#define PLANE_VEC3 PlaneVec3
#endif

enum ePlaneNodes
{
#ifdef CPLANE_ROTORS
	// for heli
	PLANE_TOPROTOR,
	PLANE_BACKROTOR,
#endif
	PLANE_WHEEL_FRONT = 2,
	PLANE_WHEEL_READ,
	NUM_PLANE_NODES
};


typedef struct CPlaneNode
{
	PLANE_VEC3 p;	// position
	float t;	// xy-distance from start on path
	bool bOnGround;	// i.e. not flying
} CPlaneNode;


typedef struct CPlaneInterpolationLine
{
	uint8_t type;
	float time;	// when does this keyframe start
	// initial values at start of frame
	float position;
	float speed;
	float acceleration;
} CPlaneInterpolationLine;
//- rouz edit (Claude)
