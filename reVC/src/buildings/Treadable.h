#pragma once

#include "Building.h"

class CTreadable : public CBuilding
{
public:
	bool GetIsATreadable(void) { return true; }
};
