//+ rouz edit (Claude)
#pragma once
#include "PhysicalLayout.h"
#include "VehicleData.h"
#include "PedData.h"
#include "ObjectLayout.h"

// Complete views of the vehicle and ped objects for C code, the layout is checked against the C++ classes in EntityLayoutChecks.cpp
typedef struct CVehicleStruct {
	CPhysicalStruct base;
	CVehicleData vehicle;
} CVehicleStruct;

typedef struct CPedStruct {
	CPhysicalStruct base;
	CPedData ped;
} CPedStruct;
//- rouz edit (Claude)
