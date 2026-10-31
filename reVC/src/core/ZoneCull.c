//+ rouz edit (ChatGPT)
#include "ZoneCull.h"
#include "Timer.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *expression, const char *filename, unsigned int line, const char *function);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define assert(expression) (expression)
#endif
#define CULL_ZONES_MAX(a,b) ((a) > (b) ? (a) : (b))

int32_t     CullZones_numAttributeZones;
CAttributeZone CullZones_attributeZones[NUMATTRIBZONES];

int32_t CullZones_playerWantedLevelDrop;
int32_t CullZones_cameraFlags;
int32_t CullZones_playerFlags;
bool CullZones_subwayInvisible;
bool CullZones_atBeachForAudio;

void
CullZones_Init(void)
{
	// Preserve the original zone operation and live state access order
	CullZones_numAttributeZones = 0;
	CullZones_playerWantedLevelDrop = 0;
	CullZones_cameraFlags = 0;
	CullZones_playerFlags = 0;
	CullZones_subwayInvisible = false;
}

void
CullZones_Update(void)
{
	// Preserve the original zone operation and live state access order
	bool invisible;

	switch((*Timer_GetFrameCounter()) & 7){
	case 0:
	case 4:
		CullZones_UpdateAtBeachForAudio();
		break;

	case 2:
		/* Update camera attributes */
		CullZones_cameraFlags = CullZones_FindAttributesForCoors(CullZonesHost_GameCamera(), NULL);
		invisible = (CullZones_cameraFlags & ATTRZONE_SUBWAYVISIBLE) == 0;
		if(invisible != CullZones_subwayInvisible){
			CullZones_MarkSubwayAsInvisible(!invisible);
			CullZones_subwayInvisible = invisible;
		}
		break;

	case 6:
		/* Update player attributes */
		CullZones_playerFlags = CullZones_FindAttributesForCoors(CullZonesHost_Player(),
			&CullZones_playerWantedLevelDrop);
		break;
	}
}

// TODO? put somewhere else?
bool
IsPointWithinArbitraryArea(float px, float py, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4)
{
	// Preserve the original zone operation and live state access order
	if((px-x1)*(x2-x1) - (py-y1)*(y2-y1) < 0.0f) return false;
	if((px-x2)*(x3-x2) - (py-y2)*(y3-y2) < 0.0f) return false;
	if((px-x3)*(x4-x3) - (py-y3)*(y4-y3) < 0.0f) return false;
	if((px-x4)*(x1-x4) - (py-y4)*(y1-y4) < 0.0f) return false;
	return true;
}

void
CullZones_UpdateAtBeachForAudio(void)
{
	// Preserve the original zone operation and live state access order
	CullZones_atBeachForAudio = IsPointWithinArbitraryArea(CullZonesHost_CameraX(), CullZonesHost_CameraY(),
		400.0f, -1644.4f,
		751.9f, 1267.8f,
		971.9f, 1216.2f,
		840.0f, -1744.0f);
}

void
CullZones_ForceCullZoneCoors(CullZonesPosition coors)
{
	// Preserve the original zone operation and live state access order
}

int32_t
CullZones_FindAttributesForCoors(CullZonesPosition coors, int32_t *wantedLevel)
{
	// Preserve the original zone operation and live state access order
	int i;
	int32_t attribs;

	if (wantedLevel)
		*wantedLevel = 0;
	attribs = 0;
	for(i = 0; i < CullZones_numAttributeZones; i++)
		if(coors.x >= CullZones_attributeZones[i].minx && coors.x <= CullZones_attributeZones[i].maxx &&
		   coors.y >= CullZones_attributeZones[i].miny && coors.y <= CullZones_attributeZones[i].maxy &&
		   coors.z >= CullZones_attributeZones[i].minz && coors.z <= CullZones_attributeZones[i].maxz){
			attribs |= CullZones_attributeZones[i].attributes;
			if(wantedLevel)
				*wantedLevel = CULL_ZONES_MAX(*wantedLevel, CullZones_attributeZones[i].wantedLevel);
		}
	return attribs;
}

CAttributeZone*
CullZones_FindZoneWithStairsAttributeForPlayer(void)
{
	// Preserve the original zone operation and live state access order
	int i;
	CullZonesPosition coors;

	coors = CullZonesHost_Player();
	for(i = 0; i < CullZones_numAttributeZones; i++)
		if(CullZones_attributeZones[i].attributes & ATTRZONE_STAIRS &&
		   coors.x >= CullZones_attributeZones[i].minx && coors.x <= CullZones_attributeZones[i].maxx &&
		   coors.y >= CullZones_attributeZones[i].miny && coors.y <= CullZones_attributeZones[i].maxy &&
		   coors.z >= CullZones_attributeZones[i].minz && coors.z <= CullZones_attributeZones[i].maxz)
			return &CullZones_attributeZones[i];
	return NULL;
}

void
CullZones_MarkSubwayAsInvisible(bool visible)
{
	// Preserve the original zone operation and live state access order
	int i, n;
	CEntity *e;
	CVehicle *v;

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	n = CPool_GetSize(CullZonesHost_BuildingPool())-1;
	//- rouz edit (ChatGPT)
	for(i = n; i >= 0; i--){
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		e = CullZonesHost_Building(CPool_GetSlot(CullZonesHost_BuildingPool(), i));
		//- rouz edit (ChatGPT)
		if(e && CullZonesHost_IsSubway(e))
			CullZonesHost_SetEntityVisible(e, visible);
	}

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	n = CPool_GetSize(CullZonesHost_TreadablePool())-1;
	//- rouz edit (ChatGPT)
	for(i = n; i >= 0; i--){
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		e = CullZonesHost_Treadable(CPool_GetSlot(CullZonesHost_TreadablePool(), i));
		//- rouz edit (ChatGPT)
		if(e && CullZonesHost_IsSubway(e))
			CullZonesHost_SetEntityVisible(e, visible);
	}

	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	n = CPool_GetSize(CullZonesHost_VehiclePool())-1;
	//- rouz edit (ChatGPT)
	for(i = n; i >= 0; i--){
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		v = CullZonesHost_Vehicle(CPool_GetSlot(CullZonesHost_VehiclePool(), i));
		//- rouz edit (ChatGPT)
		if(v && CullZonesHost_IsTrain(v) && CullZonesHost_Track(v) != 0)
			CullZonesHost_SetVehicleVisible(v, visible);
	}
}

void
CullZones_AddCullZone(const void *position,
                        float minx, float maxx,
                        float miny, float maxy,
                        float minz, float maxz,
                        uint16_t flag, int16_t wantedLevel)
{
	// Preserve the original zone operation and live state access order
	CAttributeZone *attrib;

	assert(CullZones_numAttributeZones < NUMATTRIBZONES);
	attrib = &CullZones_attributeZones[CullZones_numAttributeZones++];
	attrib->minx = minx;
	attrib->maxx = maxx;
	attrib->miny = miny;
	attrib->maxy = maxy;
	attrib->minz = minz;
	attrib->maxz = maxz;
	attrib->attributes = flag;
	attrib->wantedLevel = wantedLevel;
}

//- rouz edit (ChatGPT)
