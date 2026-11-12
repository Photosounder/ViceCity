//+ rouz edit (Claude)
#pragma once
#include "PlaceableData.h"
#include "EntityData.h"
#include "PhysicalData.h"

// Complete views of the entity and physical objects for C code, the layout is checked against the C++ classes in EntityLayoutChecks.cpp
// The first member is the virtual function table pointer the C++ classes start with
typedef struct CEntityStruct {
	void *vfptr;
	CPlaceableData placeable;
	CEntityData entity;
} CEntityStruct;

typedef struct CPhysicalStruct {
	CEntityStruct base;
	CPhysicalData physical;
} CPhysicalStruct;
//- rouz edit (Claude)
