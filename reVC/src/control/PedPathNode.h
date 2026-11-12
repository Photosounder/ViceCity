//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

// A node of the grid used to search routes for peds, kept in lists sorted by cost
typedef struct CPedPathNode
{
	bool bBlockade;
	uint8_t nodeIdX;
	uint8_t nodeIdY;
	int16_t id;
	struct CPedPathNode* prev;
	struct CPedPathNode* next;
} CPedPathNode;
//- rouz edit (Claude)
