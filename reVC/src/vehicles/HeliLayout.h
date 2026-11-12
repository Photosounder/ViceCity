//+ rouz edit (Claude)
#pragma once
#include "EntityLayout.h"
#include "HeliData.h"

// Complete view of the helicopter for C code, the layout is checked against the C++ class in EntityLayoutChecks.cpp
typedef struct CHeliStruct {
	CVehicleStruct base;
	CHeliData heli;
} CHeliStruct;
//- rouz edit (Claude)
