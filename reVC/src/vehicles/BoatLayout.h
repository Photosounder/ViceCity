//+ rouz edit (Claude)
#pragma once
#include "EntityLayout.h"
#include "BoatData.h"

// Complete view of the boat for C code, the layout is checked against the C++ class in EntityLayoutChecks.cpp
typedef struct CBoatStruct {
	CVehicleStruct base;
	CBoatData boat;
} CBoatStruct;
//- rouz edit (Claude)
