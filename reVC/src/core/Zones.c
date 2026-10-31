//+ rouz edit (ChatGPT)
#include "Zones.h"
#include "Clock.h"
#include "Timer.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#undef assert
#ifndef MASTER
void re3_assert(const char *expression, const char *filename, unsigned int line, const char *function);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define assert(expression) (expression)
#endif
#include "../save/SaveBuf.h"
#define ZONES_ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))

eLevelName Zones_currentLevel;
int16_t Zones_findIndex;

uint16_t Zones_audioCount;
int16_t Zones_audioIndices[NUMAUDIOZONES];
uint16_t Zones_mapCount;
uint16_t Zones_infoCount;
uint16_t Zones_navigationCount;
CZone Zones_info[NUMINFOZONES];
CZone Zones_map[NUMMAPZONES];
CZone Zones_navigation[NUMNAVIGZONES];
uint16_t Zones_zoneInfoCount;
CZoneInfo Zones_zoneInfo[2*NUMINFOZONES];


#define SWAPF(a, b) { float t; t = a; a = b; b = t; }

uint16_t*
Zone_GetTranslatedName(CZone *zone)
{
	// Preserve the original zone operation and live state access order
	return ZonesHost_Text(zone->name);
}

void
Zones_Init(void)
{
	// Preserve the original zone operation and live state access order
	int i, j;
	for(i = 0; i < NUMAUDIOZONES; i++)
		Zones_audioIndices[i] = -1;
	Zones_audioCount = 0;

	for(i = 0; i < NUMNAVIGZONES; i++)
		memset(&Zones_navigation[i], 0, sizeof(CZone));

	for(i = 0; i < NUMINFOZONES; i++)
		memset(&Zones_info[i], 0, sizeof(CZone));

	int x = 1000/9;
	for(i = 0; i < 2*NUMINFOZONES; i++){
		// Cars

		Zones_zoneInfo[i].carDensity = 10;
		Zones_zoneInfo[i].carThreshold[0] = x;
		Zones_zoneInfo[i].carThreshold[1] = Zones_zoneInfo[i].carThreshold[0] + x;
		Zones_zoneInfo[i].carThreshold[2] = Zones_zoneInfo[i].carThreshold[1] + x;
		Zones_zoneInfo[i].carThreshold[3] = Zones_zoneInfo[i].carThreshold[2] + x;
		Zones_zoneInfo[i].carThreshold[4] = Zones_zoneInfo[i].carThreshold[3] + x;
		Zones_zoneInfo[i].carThreshold[5] = Zones_zoneInfo[i].carThreshold[4] + x;
		Zones_zoneInfo[i].carThreshold[6] = Zones_zoneInfo[i].carThreshold[5] + x;
		Zones_zoneInfo[i].carThreshold[7] = Zones_zoneInfo[i].carThreshold[6] + x;
		Zones_zoneInfo[i].carThreshold[8] = 1000;

		Zones_zoneInfo[i].boatThreshold[0] = 500;
		Zones_zoneInfo[i].boatThreshold[1] = 1000;

		// What's going on here? this looks more like density
		Zones_zoneInfo[i].copThreshold = 50;
		for(j = 0; j < NUM_GANGS; j++)
			Zones_zoneInfo[i].gangThreshold[j] = Zones_zoneInfo[i].copThreshold;

		// Peds

		Zones_zoneInfo[i].pedDensity = 12;

		// What's going on here? this looks more like density
		Zones_zoneInfo[i].copPedThreshold = 50;
		for(j = 0; j < NUM_GANGS; j++)
			Zones_zoneInfo[i].gangPedThreshold[j] = Zones_zoneInfo[i].copPedThreshold;

		Zones_zoneInfo[i].pedGroup = 0;
	}

	Zones_zoneInfoCount = 1;	// why 1?
	Zones_navigationCount = 1;
	Zones_infoCount = 1;

	strcpy(Zones_info[0].name, "CITYINF");
	Zones_info[0].minx = -2400.0f;
	Zones_info[0].miny = -2000.0f;
	Zones_info[0].minz = -500.0f;
	Zones_info[0].maxx =  1600.0f;
	Zones_info[0].maxy =  2000.0f;
	Zones_info[0].maxz =  500.0f;
	Zones_info[0].level = LEVEL_GENERIC;
	Zones_info[0].type = ZONE_INFO;

	strcpy(Zones_navigation[0].name, "VICE_C");
	Zones_navigation[0].minx = -2400.0f;
	Zones_navigation[0].miny = -2000.0f;
	Zones_navigation[0].minz = -500.0f;
	Zones_navigation[0].maxx =  1600.0f;
	Zones_navigation[0].maxy =  2000.0f;
	Zones_navigation[0].maxz =  500.0f;
	Zones_navigation[0].level = LEVEL_GENERIC;
	Zones_navigation[0].type = ZONE_DEFAULT;

	Zones_currentLevel = LEVEL_GENERIC;

	for(i = 0; i < NUMMAPZONES; i++){
		memset(&Zones_map[i], 0, sizeof(CZone));
		Zones_map[i].type = ZONE_MAPZONE;
	}
	Zones_mapCount = 1;
	strcpy(Zones_map[0].name, "THEMAP");
	Zones_map[0].minx = -2400.0f;
	Zones_map[0].miny = -2000.0f;
	Zones_map[0].minz = -500.0f;
	Zones_map[0].maxx =  1600.0f;
	Zones_map[0].maxy =  2000.0f;
	Zones_map[0].maxz =  500.0f;
	Zones_map[0].level = LEVEL_GENERIC;
}

void
Zones_Update(void)
{
	// Preserve the original zone operation and live state access order
#ifdef SQUEEZE_PERFORMANCE
	if ((*Timer_GetFrameCounter()) % 5 != 0)
		return;
#endif
	ZonesPosition pos;
	pos = ZonesHost_Player();
	Zones_currentLevel = Zones_GetLevelFromPosition(pos);
}

void
Zones_CreateZone(char *name, eZoneType type,
	              float minx, float miny, float minz,
	              float maxx, float maxy, float maxz,
	              eLevelName level)
{
	// Preserve the original zone operation and live state access order
	char tmpname[24];
	char *p;

	if(minx > maxx) SWAPF(minx, maxx);
	if(miny > maxy) SWAPF(miny, maxy);
	if(minz > maxz) SWAPF(minz, maxz);

	// make upper case
	for(p = name; *p; p++) if(islower(*p)) *p = toupper(*p);

	strncpy(tmpname, name, 7);
	tmpname[7] = '\0';

	// add zone
	switch(type){
	case ZONE_DEFAULT:
	case ZONE_NAVIG:
		assert(Zones_navigationCount < NUMNAVIGZONES);
		strcpy(Zones_navigation[Zones_navigationCount].name, tmpname);
		Zones_navigation[Zones_navigationCount].type = type;
		Zones_navigation[Zones_navigationCount].minx = minx;
		Zones_navigation[Zones_navigationCount].miny = miny;
		Zones_navigation[Zones_navigationCount].minz = minz;
		Zones_navigation[Zones_navigationCount].maxx = maxx;
		Zones_navigation[Zones_navigationCount].maxy = maxy;
		Zones_navigation[Zones_navigationCount].maxz = maxz;
		Zones_navigation[Zones_navigationCount].level = level;
		Zones_navigationCount++;
		break;
	case ZONE_INFO:
		assert(Zones_infoCount < NUMINFOZONES);
		strcpy(Zones_info[Zones_infoCount].name, tmpname);
		Zones_info[Zones_infoCount].type = type;
		Zones_info[Zones_infoCount].minx = minx;
		Zones_info[Zones_infoCount].miny = miny;
		Zones_info[Zones_infoCount].minz = minz;
		Zones_info[Zones_infoCount].maxx = maxx;
		Zones_info[Zones_infoCount].maxy = maxy;
		Zones_info[Zones_infoCount].maxz = maxz;
		Zones_info[Zones_infoCount].level = level;
		Zones_info[Zones_infoCount].zoneinfoDay = Zones_zoneInfoCount++;
		Zones_info[Zones_infoCount].zoneinfoNight = Zones_zoneInfoCount++;
		Zones_infoCount++;
		break;
	case ZONE_MAPZONE:
		assert(Zones_mapCount < NUMMAPZONES);
		strcpy(Zones_map[Zones_mapCount].name, tmpname);
		Zones_map[Zones_mapCount].type = type;
		Zones_map[Zones_mapCount].minx = minx;
		Zones_map[Zones_mapCount].miny = miny;
		Zones_map[Zones_mapCount].minz = minz;
		Zones_map[Zones_mapCount].maxx = maxx;
		Zones_map[Zones_mapCount].maxy = maxy;
		Zones_map[Zones_mapCount].maxz = maxz;
		Zones_map[Zones_mapCount].level = level;
		Zones_mapCount++;
		break;
	}
}

void
Zones_PostZoneCreation(void)
{
	// Preserve the original zone operation and live state access order
	int i;
	for(i = 1; i < Zones_navigationCount; i++)
		Zones_InsertZoneIntoZoneHierarchy(&Zones_navigation[i]);
	Zones_InitialiseAudioZoneArray();
#ifndef MASTER
	Zones_CheckZonesForOverlap();
#endif
}

void
Zones_CheckZonesForOverlap(void)
{
	// Preserve the original zone operation and live state access order
	int i, j;
	char str[116];

	for(i = 1; i < Zones_infoCount; i++){
		Zones_ZoneIsEntirelyContainedWithinOtherZone(&Zones_info[i], &Zones_info[0]);
		
		for(j = 1; j < Zones_infoCount; j++)
			if(i != j && Zones_ZoneIsEntirelyContainedWithinOtherZone(&Zones_info[i], &Zones_info[j]))
				sprintf(str, "Info zone %s contains %s\n", Zones_info[j].name, Zones_info[i].name);
	}
}

void
Zones_InsertZoneIntoZoneHierarchy(CZone *zone)
{
	// Preserve the original zone operation and live state access order
	zone->child = NULL;
	zone->parent = NULL;
	zone->next = NULL;
	Zones_InsertZoneIntoZoneHierRecursive(zone, &Zones_navigation[0]);
}

bool
Zones_InsertZoneIntoZoneHierRecursive(CZone *inner, CZone *outer)
{
	// Preserve the original zone operation and live state access order
	int n;
	CZone *child, *next, *insert;

	// return false if inner was not inserted into outer
	if(outer == NULL ||
	   !Zones_ZoneIsEntirelyContainedWithinOtherZone(inner, outer))
		return false;

	// try to insert inner into children of outer
	for(child = outer->child; child; child = child->next)
		if(Zones_InsertZoneIntoZoneHierRecursive(inner, child))
			return true;

	// insert inner as child of outer
	// count number of outer's children contained within inner
	n = 0;
	for(child = outer->child; child; child = child->next)
		if(Zones_ZoneIsEntirelyContainedWithinOtherZone(child, inner))
			n++;
	inner->next = outer->child;
	inner->parent = outer;
	outer->child = inner;
	// move children from outer to inner
	if(n){
		insert = inner;
		for(child = inner->next; child; child = next){
			next = child->next;
			if(Zones_ZoneIsEntirelyContainedWithinOtherZone(child,inner)){
				insert->next = child->next;
				child->parent = inner;
				child->next = inner->child;
				inner->child = child;
			}else
				insert = child;
		}
	}

	return true;
}

bool
Zones_ZoneIsEntirelyContainedWithinOtherZone(CZone *inner, CZone *outer)
{
	// Preserve the original zone operation and live state access order
	char tmp[100];

	if(inner->minx < outer->minx ||
	   inner->maxx > outer->maxx ||
	   inner->miny < outer->miny ||
	   inner->maxy > outer->maxy ||
	   inner->minz < outer->minz ||
	   inner->maxz > outer->maxz){
		ZonesPosition vmin = {inner->minx, inner->miny, inner->minz};
		if(Zones_PointLiesWithinZone(vmin, outer))
			sprintf(tmp, "Overlapping zones %s and %s\n",
			      inner->name, outer->name);
		ZonesPosition vmax = {inner->maxx, inner->maxy, inner->maxz};
		if(Zones_PointLiesWithinZone(vmax, outer))
			sprintf(tmp, "Overlapping zones %s and %s\n",
			      inner->name, outer->name);
		return false;
	}
	return true;
}

bool
Zones_PointLiesWithinZone(ZonesPosition v, CZone *zone)
{
	// Preserve the original zone operation and live state access order
	return zone->minx <= v.x && v.x <= zone->maxx &&
	       zone->miny <= v.y && v.y <= zone->maxy &&
	       zone->minz <= v.z && v.z <= zone->maxz;
}

eLevelName
Zones_GetLevelFromPosition(ZonesPosition v)
{
	// Preserve the original zone operation and live state access order
	int i;
//	char tmp[116];
//	if(!Zones_PointLiesWithinZone(v, &Zones_map[0]))
//		sprintf(tmp, "x = %.3f y= %.3f z = %.3f\n", v.x, v.y, v.z);
	for(i = 1; i < Zones_mapCount; i++)
		if(Zones_PointLiesWithinZone(v, &Zones_map[i]))
			return Zones_map[i].level;
	return Zones_map[0].level;
}

CZone*
Zones_FindInformationZoneForPosition(ZonesPosition v)
{
	// Preserve the original zone operation and live state access order
	int i;
//	char tmp[116];
//	if(!Zones_PointLiesWithinZone(v, &Zones_info[0]))
//		sprintf(tmp, "x = %.3f y= %.3f z = %.3f\n", v.x, v.y, v.z);
	for(i = 1; i < Zones_infoCount; i++)
		if(Zones_PointLiesWithinZone(v, &Zones_info[i]))
			return &Zones_info[i];
	return &Zones_info[0];
}

CZone*
Zones_FindSmallestNavigationZoneForPosition(ZonesPosition v, bool findDefault, bool findNavig)
{
	// Preserve the original zone operation and live state access order
	CZone *best = NULL;
	if(findDefault && Zones_navigation[0].type == ZONE_DEFAULT ||
	   findNavig && Zones_navigation[0].type == ZONE_NAVIG)
		best = &Zones_navigation[0];
	// zone to test next
	CZone *zone = Zones_navigation[0].child;
	while(zone)
		// if in zone, descent into children
		if(Zones_PointLiesWithinZone(v, zone)){
			if(findDefault && zone->type == ZONE_DEFAULT ||
			   findNavig && zone->type == ZONE_NAVIG)
				best = zone;
			zone = zone->child;
		// otherwise try next zone
		}else
			zone = zone->next;
	return best;
}

int16_t
Zones_FindZoneByLabelAndReturnIndex(char *name, eZoneType type)
{
	// Preserve the original zone operation and live state access order
	char str[8];
	memset(str, 0, 8);
	strncpy(str, name, 8);
	switch(type){
	case ZONE_DEFAULT:
	case ZONE_NAVIG:
		for(Zones_findIndex = 0; Zones_findIndex < Zones_navigationCount; Zones_findIndex++)
			if(strcmp(Zones_GetNavigationZone(Zones_findIndex)->name, name) == 0)
				return Zones_findIndex;
		break;

	case ZONE_INFO:
		for(Zones_findIndex = 0; Zones_findIndex < Zones_infoCount; Zones_findIndex++)
			if(strcmp(Zones_GetInfoZone(Zones_findIndex)->name, name) == 0)
				return Zones_findIndex;
		break;

	case ZONE_MAPZONE:
		for(Zones_findIndex = 0; Zones_findIndex < Zones_mapCount; Zones_findIndex++)
			if(strcmp(Zones_GetMapZone(Zones_findIndex)->name, name) == 0)
				return Zones_findIndex;
		break;
	}
	return -1;
}

int16_t
Zones_FindNextZoneByLabelAndReturnIndex(char *name, eZoneType type)
{
	// Preserve the original zone operation and live state access order
	char str[8];
	++Zones_findIndex;
	memset(str, 0, 8);
	strncpy(str, name, 8);
	switch(type){
	case ZONE_DEFAULT:
	case ZONE_NAVIG:
		for(; Zones_findIndex < Zones_navigationCount; Zones_findIndex++)
			if(strcmp(Zones_GetNavigationZone(Zones_findIndex)->name, name) == 0)
				return Zones_findIndex;
		break;

	case ZONE_INFO:
		for(; Zones_findIndex < Zones_infoCount; Zones_findIndex++)
			if(strcmp(Zones_GetInfoZone(Zones_findIndex)->name, name) == 0)
				return Zones_findIndex;
		break;

	case ZONE_MAPZONE:
		for(; Zones_findIndex < Zones_mapCount; Zones_findIndex++)
			if(strcmp(Zones_GetMapZone(Zones_findIndex)->name, name) == 0)
				return Zones_findIndex;
		break;
	}
	return -1;
}

CZoneInfo*
Zones_GetZoneInfo(ZonesPosition v, uint8_t day)
{
	// Preserve the original zone operation and live state access order
	CZone *zone;
	zone = Zones_FindInformationZoneForPosition(v);
	if(zone == NULL)
		return &Zones_zoneInfo[0];
	return &Zones_zoneInfo[day ? zone->zoneinfoDay : zone->zoneinfoNight];
}

void
Zones_GetZoneInfoForTimeOfDay(ZonesPosition pos, CZoneInfo *info)
{
	// Preserve the original zone operation and live state access order
	CZoneInfo *day, *night;
	float d, n;
	int i;

	day = Zones_GetZoneInfo(pos, 1);
	night = Zones_GetZoneInfo(pos, 0);

	if(Clock_GetIsTimeInRange(8, 19))
		*info = *day;
	else if(Clock_GetIsTimeInRange(22, 5))
		*info = *night;
	else{
		if(Clock_GetIsTimeInRange(19, 22)){
			n = (Clock_GetHours() - 19) / 3.0f;
			assert(n >= 0.0f && n <= 1.0f);
			d = 1.0f - n;
		}else{
			d = (Clock_GetHours() - 5) / 3.0f;
			assert(d >= 0.0f && d <= 1.0f);
			n = 1.0f - d;
		}
#ifdef FIX_BUGS
		info->carDensity = day->carDensity * d + night->carDensity * n;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->carThreshold); i++)
			info->carThreshold[i] = day->carThreshold[i] * d + night->carThreshold[i] * n;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->boatThreshold); i++)
			info->boatThreshold[i] = day->boatThreshold[i] * d + night->boatThreshold[i] * n;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->gangThreshold); i++)
			info->gangThreshold[i] = day->gangThreshold[i] * d + night->gangThreshold[i] * n;

		info->copThreshold = day->copThreshold * d + night->copThreshold * n;
		info->pedDensity = day->pedDensity * d + night->pedDensity * n;
		info->copPedThreshold = day->copPedThreshold * d + night->copPedThreshold * n;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->gangPedThreshold); i++)
			info->gangPedThreshold[i] = day->gangPedThreshold[i] * d + night->gangPedThreshold[i] * n;
#else
		// This is a complete mess.
		info->carDensity = day->carDensity * n + night->carDensity * d;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->carThreshold); i++)
			info->carThreshold[i] = night->carThreshold[i] * d + night->carThreshold[i] * n;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->boatThreshold); i++)
			info->boatThreshold[i] = night->boatThreshold[i] * d + night->boatThreshold[i] * n;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->gangThreshold); i++)
			info->gangThreshold[i] = night->gangThreshold[i] * d + night->gangThreshold[i] * n;

		info->copThreshold = night->copThreshold * d + night->copThreshold * n;
		info->pedDensity = night->pedDensity * d + night->pedDensity * n;
		info->copPedThreshold = night->copPedThreshold * d + night->copPedThreshold * n;
		for(i = 0; i < ZONES_ARRAY_SIZE(info->gangPedThreshold); i++)
			info->gangPedThreshold[i] = night->gangPedThreshold[i] * d + night->gangPedThreshold[i] * n;
#endif
	}
	if(Clock_GetIsTimeInRange(5, 19))
		info->pedGroup = day->pedGroup;
	else
		info->pedGroup = night->pedGroup;
}

void
Zones_SetZoneCarInfo(uint16_t zoneid, uint8_t day, int16_t carDensity,
	int16_t copCarDensity, const int16_t *gangCarDensities)
{
	// Preserve the original zone operation and live state access order
	CZone *zone;
	CZoneInfo *info;
	zone = Zones_GetInfoZone(zoneid);
	info = &Zones_zoneInfo[day ? zone->zoneinfoDay : zone->zoneinfoNight];

	info->carDensity = carDensity;
	info->copThreshold = copCarDensity;
	info->gangThreshold[0] = gangCarDensities[0] + copCarDensity;
	info->gangThreshold[1] = gangCarDensities[1] + info->gangThreshold[0];
	info->gangThreshold[2] = gangCarDensities[2] + info->gangThreshold[1];
	info->gangThreshold[3] = gangCarDensities[3] + info->gangThreshold[2];
	info->gangThreshold[4] = gangCarDensities[4] + info->gangThreshold[3];
	info->gangThreshold[5] = gangCarDensities[5] + info->gangThreshold[4];
	info->gangThreshold[6] = gangCarDensities[6] + info->gangThreshold[5];
	info->gangThreshold[7] = gangCarDensities[7] + info->gangThreshold[6];
	info->gangThreshold[8] = gangCarDensities[8] + info->gangThreshold[7];
}

void Zones_SetZoneCivilianCarInfo(uint16_t zoneid, uint8_t day,
	const int16_t* carDensities, const int16_t* boatDensities)
{
	// Preserve the original zone operation and live state access order
	CZone* zone;
	CZoneInfo* info;
	zone = Zones_GetInfoZone(zoneid);
	info = &Zones_zoneInfo[day ? zone->zoneinfoDay : zone->zoneinfoNight];
	info->carThreshold[0] = carDensities[0];
	for (int i = 1; i < CARCLASS_NUM_CAR_CLASSES; i++)
		info->carThreshold[i] = carDensities[i] + info->carThreshold[i-1];
	info->boatThreshold[0] = boatDensities[0];
	for (int i = 1; i < CARCLASS_NUM_BOAT_CLASSES; i++)
		info->boatThreshold[i] = boatDensities[i] + info->boatThreshold[i - 1];
}

void
Zones_SetZonePedInfo(uint16_t zoneid, uint8_t day, int16_t pedDensity,
	int16_t gang0Density, int16_t gang1Density, int16_t gang2Density, int16_t gang3Density,
	int16_t gang4Density, int16_t gang5Density, int16_t gang6Density, int16_t gang7Density,
	int16_t gang8Density, int16_t copDensity)
{
	// Preserve the original zone operation and live state access order
	CZone *zone;
	CZoneInfo *info;
	zone = Zones_GetInfoZone(zoneid);
	info = &Zones_zoneInfo[day ? zone->zoneinfoDay : zone->zoneinfoNight];
	info->pedDensity = pedDensity;
	info->copPedThreshold = copDensity;
	info->gangPedThreshold[0] = gang0Density;
	info->gangPedThreshold[1] = gang1Density;
	info->gangPedThreshold[2] = gang2Density;
	info->gangPedThreshold[3] = gang3Density;
	info->gangPedThreshold[4] = gang4Density;
	info->gangPedThreshold[5] = gang5Density;
	info->gangPedThreshold[6] = gang6Density;
	info->gangPedThreshold[7] = gang7Density;
	info->gangPedThreshold[8] = gang8Density;

	info->gangPedThreshold[0] += info->copPedThreshold;
	info->gangPedThreshold[1] += info->gangPedThreshold[0];
	info->gangPedThreshold[2] += info->gangPedThreshold[1];
	info->gangPedThreshold[3] += info->gangPedThreshold[2];
	info->gangPedThreshold[4] += info->gangPedThreshold[3];
	info->gangPedThreshold[5] += info->gangPedThreshold[4];
	info->gangPedThreshold[6] += info->gangPedThreshold[5];
	info->gangPedThreshold[7] += info->gangPedThreshold[6];
	info->gangPedThreshold[8] += info->gangPedThreshold[7];
}

// unused
void
Zones_SetCarDensity(uint16_t zoneid, uint8_t day, uint16_t cardensity)
{
	// Preserve the original zone operation and live state access order
	CZone *zone;
	zone = Zones_GetInfoZone(zoneid);
	Zones_zoneInfo[day ? zone->zoneinfoDay : zone->zoneinfoNight].carDensity = cardensity;
}

// unused
void
Zones_SetPedDensity(uint16_t zoneid, uint8_t day, uint16_t peddensity)
{
	// Preserve the original zone operation and live state access order
	CZone *zone;
	zone = Zones_GetInfoZone(zoneid);
	Zones_zoneInfo[day ? zone->zoneinfoDay : zone->zoneinfoNight].pedDensity = peddensity;
}

void
Zones_SetPedGroup(uint16_t zoneid, uint8_t day, uint16_t pedgroup)
{
	// Preserve the original zone operation and live state access order
	CZone *zone;
	zone = Zones_GetInfoZone(zoneid);
	Zones_zoneInfo[day ? zone->zoneinfoDay : zone->zoneinfoNight].pedGroup = pedgroup;
}

int16_t
Zones_FindAudioZone(ZonesPosition pos)
{
	// Preserve the original zone operation and live state access order
	int i;

	for(i = 0; i < Zones_audioCount; i++)
		if(Zones_PointLiesWithinZone(pos, Zones_GetAudioZone(i)))
			return i;
	return -1;
}

void
Zones_AddZoneToAudioZoneArray(CZone *zone)
{
	// Preserve the original zone operation and live state access order
	int i, z;

	if(zone->type != ZONE_DEFAULT)
		return;

	/* This is a bit stupid */
	z = -1;
	for(i = 0; i < NUMNAVIGZONES; i++)
		if(&Zones_navigation[i] == zone)
			z = i;
	assert(Zones_audioCount < NUMAUDIOZONES);
	Zones_audioIndices[Zones_audioCount++] = z;
}

void
Zones_InitialiseAudioZoneArray(void)
{
	// Preserve the original zone operation and live state access order
	bool gonext;
	CZone *zone;

	gonext = false;
	zone = &Zones_navigation[0];
	// Go deep first,
	// set gonext when backing up a level to visit the next child
	while(zone)
		if(gonext){
			Zones_AddZoneToAudioZoneArray(zone);
			if(zone->next){
				gonext = false;
				zone = zone->next;
			}else
				zone = zone->parent;
		}else if(zone->child)
			zone = zone->child;
		else{
			Zones_AddZoneToAudioZoneArray(zone);
			if(zone->next)
				zone = zone->next;
			else{
				gonext = true;
				zone = zone->parent;
			}
		}
}

void
Zones_SaveAllZones(uint8_t *buffer, uint32_t *size)
{
	// Preserve the original zone operation and live state access order
	INITSAVEBUF
	int i;

#define CZONE_SAVE_SIZE (sizeof(char)*8+sizeof(float)+sizeof(float)+sizeof(float)+sizeof(float)+sizeof(float)+sizeof(float)+sizeof(eZoneType)+sizeof(eLevelName)+sizeof(int16_t)+sizeof(int16_t)+sizeof(int32_t)+sizeof(int32_t)+sizeof(int32_t))

	*size = SAVE_HEADER_SIZE
		+ sizeof(Zones_currentLevel) + sizeof(Zones_findIndex)
		+ sizeof(int16_t) // padding
		+ CZONE_SAVE_SIZE * ZONES_ARRAY_SIZE(Zones_navigation) + CZONE_SAVE_SIZE * ZONES_ARRAY_SIZE(Zones_info) + sizeof(Zones_zoneInfo)
		+ sizeof(Zones_navigationCount) + sizeof(Zones_infoCount) + sizeof(Zones_zoneInfoCount)
		+ sizeof(int16_t) // padding
		+ CZONE_SAVE_SIZE * ZONES_ARRAY_SIZE(Zones_map) + sizeof(Zones_audioIndices)
		+ sizeof(Zones_mapCount) + sizeof(Zones_audioCount);
#undef CZONE_SAVE_SIZE

	uint32_t length = 0;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveHeaderWithLength(&buffer, &length, 'Z', 'N', 'S', '\0', *size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(&buffer, &length, &Zones_currentLevel, sizeof(Zones_currentLevel));
	WriteSaveBufWithLength(&buffer, &length, &Zones_findIndex, sizeof(Zones_findIndex));
	{
		// Materialize the saved value with its original serialized type
		int16_t saveValue = (int16_t)0;
		WriteSaveBufWithLength(&buffer, &length, &saveValue, sizeof(saveValue)); // padding
	}
	//- rouz edit (ChatGPT)

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_navigation); i++)
		Zones_SaveOneZone(&Zones_navigation[i], &buffer, &length, ZONE_NAVIG);

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_info); i++)
		Zones_SaveOneZone(&Zones_info[i], &buffer, &length, ZONE_INFO);

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_zoneInfo); i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBufWithLength(&buffer, &length, &Zones_zoneInfo[i], sizeof(Zones_zoneInfo[i]));
		//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(&buffer, &length, &Zones_navigationCount, sizeof(Zones_navigationCount));
	WriteSaveBufWithLength(&buffer, &length, &Zones_infoCount, sizeof(Zones_infoCount));
	WriteSaveBufWithLength(&buffer, &length, &Zones_zoneInfoCount, sizeof(Zones_zoneInfoCount));
	{
		// Materialize the saved value with its original serialized type
		int16_t saveValue = (int16_t)0;
		WriteSaveBufWithLength(&buffer, &length, &saveValue, sizeof(saveValue)); // padding
	}
	//- rouz edit (ChatGPT)

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_map); i++)
		Zones_SaveOneZone(&Zones_map[i], &buffer, &length, ZONE_MAPZONE);

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_audioIndices); i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBufWithLength(&buffer, &length, &Zones_audioIndices[i], sizeof(Zones_audioIndices[i]));
		//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(&buffer, &length, &Zones_mapCount, sizeof(Zones_mapCount));
	WriteSaveBufWithLength(&buffer, &length, &Zones_audioCount, sizeof(Zones_audioCount));
	//- rouz edit (ChatGPT)

	VALIDATESAVEBUF(*size)
}

void
Zones_SaveOneZone(CZone *zone, uint8_t **buffer, uint32_t *length, eZoneType zoneType)
{
	// Preserve the original zone operation and live state access order
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(buffer, length, &(*(uint32_t*)&zone->name[0]), sizeof(*(uint32_t*)&zone->name[0]));
	WriteSaveBufWithLength(buffer, length, &(*(uint32_t*)&zone->name[4]), sizeof(*(uint32_t*)&zone->name[4]));
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(buffer, length, &zone->minx, sizeof(zone->minx));
	WriteSaveBufWithLength(buffer, length, &zone->miny, sizeof(zone->miny));
	WriteSaveBufWithLength(buffer, length, &zone->minz, sizeof(zone->minz));
	WriteSaveBufWithLength(buffer, length, &zone->maxx, sizeof(zone->maxx));
	WriteSaveBufWithLength(buffer, length, &zone->maxy, sizeof(zone->maxy));
	WriteSaveBufWithLength(buffer, length, &zone->maxz, sizeof(zone->maxz));
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(buffer, length, &zone->type, sizeof(zone->type));
	WriteSaveBufWithLength(buffer, length, &zone->level, sizeof(zone->level));
	WriteSaveBufWithLength(buffer, length, &zone->zoneinfoDay, sizeof(zone->zoneinfoDay));
	WriteSaveBufWithLength(buffer, length, &zone->zoneinfoNight, sizeof(zone->zoneinfoNight));
	//- rouz edit (ChatGPT)

	int32_t zoneId;
	zoneId = Zones_GetIndexForNavigationZonePointer(zone->child);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(buffer, length, &zoneId, sizeof(zoneId));
	//- rouz edit (ChatGPT)
	zoneId = Zones_GetIndexForNavigationZonePointer(zone->parent);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(buffer, length, &zoneId, sizeof(zoneId));
	//- rouz edit (ChatGPT)
	zoneId = Zones_GetIndexForNavigationZonePointer(zone->next);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBufWithLength(buffer, length, &zoneId, sizeof(zoneId));
	//- rouz edit (ChatGPT)
}

void
Zones_LoadAllZones(uint8_t *buffer, uint32_t size)
{
	// Preserve the original zone operation and live state access order
	INITSAVEBUF
	int i;

	uint32_t length = 0;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	CheckSaveHeaderWithLength(&buffer, &length, 'Z', 'N', 'S', '\0', size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Zones_currentLevel, &buffer, sizeof(Zones_currentLevel));
	ReadSaveBuf(&Zones_findIndex, &buffer, sizeof(Zones_findIndex));
	SkipSaveBuf(&buffer, 2);
	//- rouz edit (ChatGPT)

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_navigation); i++)
		Zones_LoadOneZone(&Zones_navigation[i], &buffer, &length, ZONE_NAVIG);

	for (i = 0; i < ZONES_ARRAY_SIZE(Zones_info); i++)
		Zones_LoadOneZone(&Zones_info[i], &buffer, &length, ZONE_INFO);

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_zoneInfo); i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Zones_zoneInfo[i], &buffer, sizeof(Zones_zoneInfo[i]));
		//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Zones_navigationCount, &buffer, sizeof(Zones_navigationCount));
	ReadSaveBuf(&Zones_infoCount, &buffer, sizeof(Zones_infoCount));
	ReadSaveBuf(&Zones_zoneInfoCount, &buffer, sizeof(Zones_zoneInfoCount));
	SkipSaveBuf(&buffer, 2);
	//- rouz edit (ChatGPT)

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_map); i++)
		Zones_LoadOneZone(&Zones_map[i], &buffer, &length, ZONE_MAPZONE);

	for(i = 0; i < ZONES_ARRAY_SIZE(Zones_audioIndices); i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Zones_audioIndices[i], &buffer, sizeof(Zones_audioIndices[i]));
		//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Zones_mapCount, &buffer, sizeof(Zones_mapCount));
	ReadSaveBuf(&Zones_audioCount, &buffer, sizeof(Zones_audioCount));
	//- rouz edit (ChatGPT)

	VALIDATESAVEBUF(size)
}

void
Zones_LoadOneZone(CZone *zone, uint8_t **buffer, uint32_t *length, eZoneType zoneType)
{
	// Preserve the original zone operation and live state access order
#ifdef THIS_IS_STUPID
	uint32_t part1, part2;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBufWithLength(&part1, buffer, length, sizeof(part1));
	ReadSaveBufWithLength(&part2, buffer, length, sizeof(part2));
	//- rouz edit (ChatGPT)

	*(uint64_t *)&zone->name[0] = (uint64_t)part2;
	*(uint64_t *)&zone->name[0] <<= 32;
	*(uint64_t *)&zone->name[0] |= (uint64_t)part1;
#else
	for(int i = 0; i < sizeof(zone->name); i++)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBufWithLength(&zone->name[i], buffer, length, sizeof(zone->name[i]));
		//- rouz edit (ChatGPT)
#endif

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBufWithLength(&zone->minx, buffer, length, sizeof(zone->minx));
	ReadSaveBufWithLength(&zone->miny, buffer, length, sizeof(zone->miny));
	ReadSaveBufWithLength(&zone->minz, buffer, length, sizeof(zone->minz));
	ReadSaveBufWithLength(&zone->maxx, buffer, length, sizeof(zone->maxx));
	ReadSaveBufWithLength(&zone->maxy, buffer, length, sizeof(zone->maxy));
	ReadSaveBufWithLength(&zone->maxz, buffer, length, sizeof(zone->maxz));
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBufWithLength(&zone->type, buffer, length, sizeof(zone->type));
	ReadSaveBufWithLength(&zone->level, buffer, length, sizeof(zone->level));
	ReadSaveBufWithLength(&zone->zoneinfoDay, buffer, length, sizeof(zone->zoneinfoDay));
	ReadSaveBufWithLength(&zone->zoneinfoNight, buffer, length, sizeof(zone->zoneinfoNight));
	//- rouz edit (ChatGPT)

	int32_t zoneId;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBufWithLength(&zoneId, buffer, length, sizeof(zoneId));
	//- rouz edit (ChatGPT)
	zone->child = Zones_GetPointerForNavigationZoneIndex(zoneId);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBufWithLength(&zoneId, buffer, length, sizeof(zoneId));
	//- rouz edit (ChatGPT)
	zone->parent = Zones_GetPointerForNavigationZoneIndex(zoneId);
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBufWithLength(&zoneId, buffer, length, sizeof(zoneId));
	//- rouz edit (ChatGPT)
	zone->next = Zones_GetPointerForNavigationZoneIndex(zoneId);
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
