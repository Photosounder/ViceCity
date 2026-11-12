//+ rouz edit (Claude)
#pragma once
#include "EntityLayout.h"
#include "AutomobileData.h"

// Complete view of the automobile for C code, the layout is checked against the C++ class in EntityLayoutChecks.cpp
typedef struct CAutomobileStruct {
	CVehicleStruct base;
	CAutomobileData automobile;
} CAutomobileStruct;
//- rouz edit (Claude)
