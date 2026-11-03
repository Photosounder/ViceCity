//+ rouz edit (ChatGPT)
#include "PopulationGroups.h"
#include "FileMgr.h"
#include "ModelRegistry.h"
#include "ModelIds.h"
#include "General.h"
#include "Gangs.h"
#include "Weather.h"
#include <string.h>
PedGroup Population_pedGroups[NUMPEDGROUPS];
void Population_LoadPedGroups(void)
{
    // Open the original data file through the same directory changes
    int fd;
    char line[1024];
    int nextPedGroup = 0;
    char modelName[256];
    FileMgr_ChangeDir("\\DATA\\");
    fd = FileMgr_OpenFile("PEDGRP.DAT", "r");
    FileMgr_ChangeDir("\\");
    while (FileMgr_ReadLine(fd, line, sizeof(line))) {
        // Replace separators until the original newline terminator
        int end;
        int cursor = 0;
        int i;
        for (end = 0; ; end++) {
            // Retain the original separator and newline handling
            if (line[end] == '\n')
                break;
            if (line[end] == ',' || line[end] == '\r')
                line[end] = ' ';
        }
        line[end] = '\0';
        for (i = 0; i < NUMMODELSPERPEDGROUP; i++) {
            // Skip whitespace and stop at comments or the end of the row
            int nextWhitespace;
            while (line[cursor] <= ' ' && line[cursor] != '\0')
                ++cursor;
            if (line[cursor] == '#')
                break;
            for (nextWhitespace = cursor; line[nextWhitespace] > ' '; ++nextWhitespace)
                ;
            if (cursor == nextWhitespace)
                break;
            // Resolve each model directly into its group slot
            strncpy(modelName, &line[cursor], nextWhitespace - cursor);
            modelName[nextWhitespace - cursor] = '\0';
            ModelRegistry_Find(modelName, 0, MODELINFOSIZE - 1, &Population_pedGroups[nextPedGroup].models[i]);
            cursor = nextWhitespace;
        }
        if (i == NUMMODELSPERPEDGROUP)
            nextPedGroup++;
    }
    // Close the data file after all rows have been read
    FileMgr_CloseFile(fd);
}
static uint32_t Population_lastCivilianIndex;

bool
Population_IsSunbather(int32_t model)
{
    // Identify the original set of beach pedestrian models
	switch (model) {
		case MI_HFYBE:
		case MI_HFOBE:
		case MI_HMYBE:
		case MI_HMOBE:
		case MI_BFYBE:
		case MI_BMYBE:
		case MI_BFOBE:
		case MI_BMOBE:
		case MI_WFYBE:
		case MI_WMYBE:
		case MI_WFOBE:
		case MI_WMOBE:
			return true;
		default:
			return false;
	}
}

int32_t Population_ChooseCivilianOccupation(int32_t group)
{
    // Select a random group entry and retry sunbathers in rain
    if (Weather_Rain > 0.1f) {
        // Retain the original eight-attempt limit and last result
        int32_t lastModel;
        int i;
        for (i = 0; i < 8; i++) {
            // Draw and inspect each candidate in the original order
            Population_lastCivilianIndex = General_GetRandomNumberInRangeInt(0, NUMMODELSPERPEDGROUP);
            lastModel = Population_pedGroups[group].models[Population_lastCivilianIndex];
            if (!Population_IsSunbather(lastModel))
                break;
        }
        return lastModel;
    }
    // Keep the dry-weather single random draw
    Population_lastCivilianIndex = General_GetRandomNumberInRangeInt(0, NUMMODELSPERPEDGROUP);
    return Population_pedGroups[group].models[Population_lastCivilianIndex];
}

int32_t Population_ChooseNextCivilianOccupation(int32_t group)
{
    // Advance sequentially and skip sunbathers while it rains
    if (Weather_Rain > 0.1f) {
        // Preserve the full-group attempt limit and repeated model lookup
        int32_t lastModel;
        int i;
        for (i = 0; i < NUMMODELSPERPEDGROUP; i++) {
            // Advance and wrap before inspecting the next candidate
            ++Population_lastCivilianIndex;
            if (Population_lastCivilianIndex >= NUMMODELSPERPEDGROUP)
                Population_lastCivilianIndex = 0;
            lastModel = Population_pedGroups[group].models[Population_lastCivilianIndex];
            if (!Population_IsSunbather(Population_pedGroups[group].models[Population_lastCivilianIndex]))
                break;
        }
        return lastModel;
    }
    // Advance once in dry weather
    ++Population_lastCivilianIndex;
    if (Population_lastCivilianIndex >= NUMMODELSPERPEDGROUP)
        Population_lastCivilianIndex = 0;
    return Population_pedGroups[group].models[Population_lastCivilianIndex];
}
bool
Population_IsMale(int32_t model)
{
	// Classify the original pedestrian model list
	switch (model) {
		case MI_HMYST:
		case MI_HMOST:
		case MI_HMYRI:
		case MI_HMORI:
		case MI_HMYBE:
		case MI_HMOBE:
		case MI_HMOTR:
		case MI_HMYAP:
		case MI_HMOCA:
		case MI_BMODK:
		case MI_BMYKR:
		case MI_BMYST:
		case MI_BMOST:
		case MI_BMYRI:
		case MI_BMYBE:
		case MI_BMOBE:
		case MI_BMYBU:
		case MI_BMOTR:
		case MI_BMYPI:
		case MI_BMYBB:
		case MI_WMYCR:
		case MI_WMYST:
		case MI_WMOST:
		case MI_WMYRI:
		case MI_WMORI:
		case MI_WMYBE:
		case MI_WMOBE:
		case MI_WMYCW:
		case MI_WMYGO:
		case MI_WMOGO:
		case MI_WMYLG:
		case MI_WMYBU:
		case MI_WMOBU:
		case MI_WMOTR:
		case MI_WMYPI:
		case MI_WMOCA:
		case MI_WMYJG:
		case MI_WMYSK:

		// BUG? Why no JMOTO?
			return true;
		default:
			return false;
	}
}

bool
Population_IsFemale(int32_t model)
{
	// Classify the original pedestrian model list
	switch (model) {
		case MI_HFYST:
		case MI_HFOST:
		case MI_HFYRI:
		case MI_HFORI:
		case MI_HFYBE:
		case MI_HFOBE:
		case MI_HFYBU:
		case MI_HFYMD:
		case MI_HFYCG:
		case MI_HFYPR:
		case MI_HFOTR:
		case MI_BFYST:
		case MI_BFOST:
		case MI_BFYRI:
		case MI_BFORI:
		case MI_BFYBE:
		case MI_BFOBE:
		case MI_BFYPR:
		case MI_BFOTR:
		case MI_WFYST:
		case MI_WFOST:
		case MI_WFYRI:
		case MI_WFORI:
		case MI_WFYBE:
		case MI_WFOBE:
		case MI_WFOGO:
		case MI_WFYLG:
		case MI_WFYBU:
		case MI_WFYPR:
		case MI_WFOTR:
		case MI_WFYJG:
		case MI_WFYSK:
		case MI_WFYSH:
		case MI_WFOSH:
		case MI_JFOTO:
			return true;
		default:
			return false;
	}
}

void Population_ChooseCivilianCoupleOccupations(int32_t group, int32_t *man, int32_t *woman)
{
    // Preserve the eight random attempts to find a civilian man
    int i;
    *man = -1;
    *woman = -1;
    for (i = 0; i < 8; i++) {
        // Stop drawing after a suitable man has been found
        int32_t model;
        if (*man > -1)
            break;
        model = Population_pedGroups[group].models[General_GetRandomNumberInRangeInt(0, NUMMODELSPERPEDGROUP)];
        if (*man == -1 && Population_IsMale(model) && PopulationGroups_IsCivilianMale(model))
            *man = model;
    }
    if (*man != -1) {
        // Search women in group order and retain the last entry when no match exists
        int32_t model;
        for (i = 0; i < NUMMODELSPERPEDGROUP; i++) {
            // Apply classification before consulting native model details
            model = Population_pedGroups[group].models[i];
            if (Population_IsFemale(model) && PopulationGroups_FemaleMatches(*man, model))
                break;
        }
        *woman = model;
    }
}


int32_t
Population_ChoosePolicePedOccupation()
{
	// Preserve the original population helper behavior
	General_GetRandomNumber();
	return COP_STREET;
}

int
Population_ChooseGangOccupation(int gangId)
{
	// Preserve the original population helper behavior
	return Gangs_ChooseGangPedModel(gangId);
}

bool
Population_CanJeerAtStripper(int32_t model)
{
	// Preserve the original population helper behavior
	return model == MI_WMOBE || model == MI_WMYBE || model == MI_WMOST || model == MI_BMYBB;
}

int32_t
Population_ComputeRandomisedGangSize(void)
{
	// Preserve the original population helper behavior
	return General_GetRandomNumberInRangeInt(3, 6);
}

bool
Population_CanSolicitPlayerInCar(int32_t model)
{
	// Preserve the original population helper behavior
	return model == MI_HFYPR || model == MI_BFYPR || model == MI_WFYPR;
}

bool
Population_CanSolicitPlayerOnFoot(int32_t model)
{
	// Preserve the original population helper behavior
	return model == MI_HFYMD || model == MI_HFYCG || model == MI_BFOTR || model == MI_BMOTR || model == MI_WFOTR || model == MI_WMOTR;
}

bool
Population_IsSecurityGuard(ePedType pedType)
{
	// Preserve the original population helper behavior
	return pedType == PEDTYPE_GANG5;
}
//- rouz edit (ChatGPT)
