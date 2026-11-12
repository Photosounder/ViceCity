//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "Rect.h"

// One slot of the collision store, a collision file with the area and the model range it covers (name made up)
typedef struct ColDef {
	int32_t unused;
	bool isLoaded;
	CRect bounds;
	char name[20];
	int16_t minIndex;
	int16_t maxIndex;
} ColDef;
//- rouz edit (Claude)
