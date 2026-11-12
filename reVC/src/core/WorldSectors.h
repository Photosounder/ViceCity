//+ rouz edit (Claude)
#pragma once
#include "Lists.h"

/* Sectors span from -2400 to 1600 in x and -2000 to 2000 y.
 * With 80x80 sectors, each is 50x50 units. */

#define SECTOR_SIZE_X (50.0f)
#define SECTOR_SIZE_Y (50.0f)

#define NUMSECTORS_X (80*2)		// rouz edit, the *2s fix a -1 sector id crash
#define NUMSECTORS_Y (80*2)

#define WORLD_SIZE_X (NUMSECTORS_X * SECTOR_SIZE_X)
#define WORLD_SIZE_Y (NUMSECTORS_Y * SECTOR_SIZE_Y)

#define WORLD_MIN_X (-2400.0f*2.f)
#define WORLD_MIN_Y (-2000.0f*2.f)

#define WORLD_MAX_X (WORLD_MIN_X + WORLD_SIZE_X)
#define WORLD_MAX_Y (WORLD_MIN_Y + WORLD_SIZE_Y)

#define MAP_Z_LOW_LIMIT -100.0f

enum
{
	ENTITYLIST_BUILDINGS,
	ENTITYLIST_BUILDINGS_OVERLAP,
	ENTITYLIST_OBJECTS,
	ENTITYLIST_OBJECTS_OVERLAP,
	ENTITYLIST_VEHICLES,
	ENTITYLIST_VEHICLES_OVERLAP,
	ENTITYLIST_PEDS,
	ENTITYLIST_PEDS_OVERLAP,
	ENTITYLIST_DUMMIES,
	ENTITYLIST_DUMMIES_OVERLAP,

	NUMSECTORENTITYLISTS
};

struct CSector
{
	CPtrList m_lists[NUMSECTORENTITYLISTS];
};

//- rouz edit (Claude)
