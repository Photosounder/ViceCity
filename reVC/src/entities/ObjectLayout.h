//+ rouz edit (Claude)
#pragma once
#include "PhysicalLayout.h"
#include "ObjectState.h"

// Complete view of the object for C code, the layout is checked against the C++ class in EntityLayoutChecks.cpp
typedef struct CObjectStruct {
	CPhysicalStruct base;
	CObjectState object;
} CObjectStruct;
//- rouz edit (Claude)
