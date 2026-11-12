//+ rouz edit (Claude)
#pragma once
#include "EntityLayout.h"
#include "BikeData.h"

// Complete view of the bike for C code, the layout is checked against the C++ class in EntityLayoutChecks.cpp
typedef struct CBikeStruct {
	CVehicleStruct base;
	CBikeData bike;
} CBikeStruct;
//- rouz edit (Claude)
