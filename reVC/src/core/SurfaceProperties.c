//+ rouz edit (ChatGPT)
#include "SurfaceTable_backend.h"
#include "FileMgr.h"
#include <stdio.h>
static float SurfaceTable_AdhesiveLimits[NUMADHESIVEGROUPS][NUMADHESIVEGROUPS];
void
SurfaceTable_Initialise(const char *filename)
{
    // Preserve the complete original surface parser or lookup decision
	int lineno, fieldno;
	char *line;
	char surfname[256];
	float adhesiveLimit;

	FileMgr_SetDir("");
	FileMgr_LoadFile(filename, SurfaceTable_BackendWorkBuffer(), SURFACE_TABLE_WORK_BUFFER_SIZE, "r");

	line = (char*)SurfaceTable_BackendWorkBuffer();
	for(lineno = 0; lineno < NUMADHESIVEGROUPS; lineno++){
		// Preserve the original scan, branch and assignment order
		// skip white space and comments
		while(*line == ' ' || *line == '\t' || *line == '\n' || *line == '\r' || *line == ';'){
			// Preserve the original scan, branch and assignment order
			if(*line == ';'){
				// Preserve the original scan, branch and assignment order
				while(*line != '\n' && *line != '\r')
					line++;
			}else
				line++;
		}

		sscanf(line, "%s", surfname);
		// skip what we just read
		while(!(*line == ' ' || *line == '\t' || *line == ','))
			line++;

		for(fieldno = 0; fieldno <= lineno; fieldno++){
			// Preserve the original scan, branch and assignment order
			// skip white space
			while(*line == ' ' || *line == '\t' || *line == ',')
				line++;
			adhesiveLimit = 0.0f;
			if(*line != '-')
				sscanf(line, "%f", &adhesiveLimit);
			// skip what we just read
			while(!(*line == ' ' || *line == '\t' || *line == ',' || *line == '\n'))
				line++;

			SurfaceTable_AdhesiveLimits[lineno][fieldno] = adhesiveLimit;
			SurfaceTable_AdhesiveLimits[fieldno][lineno] = adhesiveLimit;
		}
	}
}

int
SurfaceTable_GetAdhesionGroup(uint8_t surfaceType)
{
    // Preserve the complete original surface parser or lookup decision
	switch(surfaceType){
		// Preserve the original scan, branch and assignment order
	case SURFACE_DEFAULT:		return ADHESIVE_ROAD;
	case SURFACE_TARMAC:		return ADHESIVE_ROAD;
	case SURFACE_GRASS:		return ADHESIVE_LOOSE;
	case SURFACE_GRAVEL:		return ADHESIVE_LOOSE;
	case SURFACE_MUD_DRY:		return ADHESIVE_HARD;
	case SURFACE_PAVEMENT:		return ADHESIVE_ROAD;
	case SURFACE_CAR:		return ADHESIVE_HARD;
	case SURFACE_GLASS:		return ADHESIVE_HARD;
	case SURFACE_TRANSPARENT_CLOTH:	return ADHESIVE_HARD;
	case SURFACE_GARAGE_DOOR:	return ADHESIVE_HARD;
	case SURFACE_CAR_PANEL:		return ADHESIVE_HARD;
	case SURFACE_THICK_METAL_PLATE:	return ADHESIVE_HARD;
	case SURFACE_SCAFFOLD_POLE:	return ADHESIVE_HARD;
	case SURFACE_LAMP_POST:		return ADHESIVE_HARD;
	case SURFACE_FIRE_HYDRANT:	return ADHESIVE_HARD;
	case SURFACE_GIRDER:		return ADHESIVE_HARD;
	case SURFACE_METAL_CHAIN_FENCE:	return ADHESIVE_HARD;
	case SURFACE_PED:		return ADHESIVE_RUBBER;
	case SURFACE_SAND:		return ADHESIVE_SAND;
	case SURFACE_WATER:		return ADHESIVE_WET;
	case SURFACE_WOOD_CRATES:	return ADHESIVE_ROAD;
	case SURFACE_WOOD_BENCH:	return ADHESIVE_ROAD;
	case SURFACE_WOOD_SOLID:	return ADHESIVE_ROAD;
	case SURFACE_RUBBER:		return ADHESIVE_RUBBER;
	case SURFACE_PLASTIC:		return ADHESIVE_HARD;
	case SURFACE_HEDGE:		return ADHESIVE_LOOSE;
	case SURFACE_STEEP_CLIFF:	return ADHESIVE_LOOSE;
	case SURFACE_CONTAINER:		return ADHESIVE_HARD;
	case SURFACE_NEWS_VENDOR:	return ADHESIVE_HARD;
	case SURFACE_WHEELBASE:		return ADHESIVE_RUBBER;
	case SURFACE_CARDBOARDBOX:	return ADHESIVE_LOOSE;
	case SURFACE_TRANSPARENT_STONE:	return ADHESIVE_HARD;
	case SURFACE_METAL_GATE:	return ADHESIVE_HARD;
	case SURFACE_SAND_BEACH:	return ADHESIVE_SAND;
	case SURFACE_CONCRETE_BEACH:	return ADHESIVE_ROAD;
	default:			return ADHESIVE_ROAD;
	}
}

float
SurfaceTable_GetWetMultiplier(uint8_t surfaceType)
{
    // Preserve the complete original surface parser or lookup decision
	switch(surfaceType){
		// Preserve the original scan, branch and assignment order
	case SURFACE_DEFAULT:
	case SURFACE_TARMAC:
	case SURFACE_MUD_DRY:
	case SURFACE_PAVEMENT:
	case SURFACE_TRANSPARENT_CLOTH:
	case SURFACE_WOOD_CRATES:
	case SURFACE_WOOD_BENCH:
	case SURFACE_WOOD_SOLID:
	case SURFACE_HEDGE:
	case SURFACE_CARDBOARDBOX:
	case SURFACE_TRANSPARENT_STONE:
	case SURFACE_CONCRETE_BEACH:
		return 1.0f - SurfaceTable_BackendWetRoads()*0.25f;

	case SURFACE_GRASS:
	case SURFACE_CAR:
	case SURFACE_GLASS:
	case SURFACE_GARAGE_DOOR:
	case SURFACE_CAR_PANEL:
	case SURFACE_THICK_METAL_PLATE:
	case SURFACE_SCAFFOLD_POLE:
	case SURFACE_LAMP_POST:
	case SURFACE_FIRE_HYDRANT:
	case SURFACE_GIRDER:
	case SURFACE_METAL_CHAIN_FENCE:
	case SURFACE_PED:
	case SURFACE_RUBBER:
	case SURFACE_PLASTIC:
	case SURFACE_STEEP_CLIFF:
	case SURFACE_CONTAINER:
	case SURFACE_NEWS_VENDOR:
	case SURFACE_WHEELBASE:
	case SURFACE_METAL_GATE:
		return 1.0f - SurfaceTable_BackendWetRoads()*0.4f;

	case SURFACE_SAND:
	case SURFACE_SAND_BEACH:
		return 1.0f + SurfaceTable_BackendWetRoads()*0.5f;

	default:
		return 1.0f;
	}
}

float
SurfaceTable_GetAdhesiveLimit(const CColPoint *colpoint)
{
    // Preserve the complete original surface parser or lookup decision
	return SurfaceTable_AdhesiveLimits[SurfaceTable_GetAdhesionGroup(SurfaceTable_BackendSurfaceB(colpoint))][SurfaceTable_GetAdhesionGroup(SurfaceTable_BackendSurfaceA(colpoint))];
}

bool
SurfaceTable_IsSoftLanding(uint8_t surf)
{
    // Preserve the complete original surface parser or lookup decision
	return surf == SURFACE_GRASS || surf == SURFACE_SAND || surf == SURFACE_SAND_BEACH;
}

bool
IsSeeThrough(uint8_t surfType)
{
    // Preserve the complete original surface parser or lookup decision
	switch(surfType)
	case SURFACE_GLASS:
	case SURFACE_TRANSPARENT_CLOTH:
	case SURFACE_METAL_CHAIN_FENCE:
	case SURFACE_TRANSPARENT_STONE:
	case SURFACE_SCAFFOLD_POLE:
		return true;
	return false;
}

// I think the necessity of this function is really a bug
bool
IsSeeThroughVertical(uint8_t surfType)
{
    // Preserve the complete original surface parser or lookup decision
	switch(surfType)
	case SURFACE_GLASS:
	case SURFACE_TRANSPARENT_CLOTH:
		return true;
	return false;
}

bool
IsShootThrough(uint8_t surfType)
{
    // Preserve the complete original surface parser or lookup decision
	switch(surfType)
	case SURFACE_TRANSPARENT_CLOTH:
	case SURFACE_METAL_CHAIN_FENCE:
	case SURFACE_TRANSPARENT_STONE:
	case SURFACE_SCAFFOLD_POLE:
		return true;
	return false;
}
//- rouz edit (ChatGPT)
