//+ rouz edit (ChatGPT)
#include "IniFile.h"
#include "PopulationLimits.h"
#include "SharedText.h"
#include "FileMgr.h"
#include <stdio.h>
#define INIFILE_MIN(a,b) ((a) < (b) ? (a) : (b))
#define INIFILE_MAX(a,b) ((a) > (b) ? (a) : (b))

float IniFile_PedNumberMultiplier = 0.6f;
float IniFile_CarNumberMultiplier = 0.6f;

void IniFile_LoadIniFile(void)
{
	// Restore the original file-manager directory before opening the INI file
	FileMgr_SetDir("");
	int f = FileMgr_OpenFile("gta3.ini", "r");
	if (f){
		// Read and parse into the original shared buffer without resetting stale contents
		char *string = gString;
		FileMgr_ReadLine(f, string, 200);
		sscanf(string, "%f", &IniFile_PedNumberMultiplier);
		IniFile_PedNumberMultiplier = INIFILE_MIN(3.0f, INIFILE_MAX(0.5f, IniFile_PedNumberMultiplier));
		FileMgr_ReadLine(f, string, 200);
		sscanf(string, "%f", &IniFile_CarNumberMultiplier);
		IniFile_CarNumberMultiplier = INIFILE_MIN(3.0f, INIFILE_MAX(0.5f, IniFile_CarNumberMultiplier));
		FileMgr_CloseFile(f);
	}
	// Apply the original float-to-integer limits in their original write order
	PopulationLimits_MaxPeds = DEFAULT_MAX_NUMBER_OF_PEDS * IniFile_PedNumberMultiplier;
	PopulationLimits_MaxInteriorPeds = DEFAULT_MAX_NUMBER_OF_PEDS_INTERIOR * IniFile_PedNumberMultiplier;
	PopulationLimits_MaxCars = DEFAULT_MAX_NUMBER_OF_CARS * IniFile_CarNumberMultiplier;
}
//- rouz edit (ChatGPT)
