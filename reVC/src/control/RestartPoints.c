//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "Restart_backend.h"
#include "Zones.h"
#include <stdio.h>
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
uint8_t Restart_OverrideHospitalLevel;
uint8_t Restart_OverridePoliceStationLevel;
bool Restart_bFadeInAfterNextArrest;
bool Restart_bFadeInAfterNextDeath;
bool Restart_bOverrideRestart;
RestartPoint Restart_OverridePosition;
float Restart_OverrideHeading;
uint16_t Restart_NumberOfHospitalRestarts;
uint16_t Restart_NumberOfPoliceRestarts;
RestartPoint Restart_HospitalRestartPoints[NUM_RESTART_POINTS];
float Restart_HospitalRestartHeadings[NUM_RESTART_POINTS];
RestartPoint Restart_PoliceRestartPoints[NUM_RESTART_POINTS];
float Restart_PoliceRestartHeadings[NUM_RESTART_POINTS];
static ZonesPosition Restart_AsZones(RestartPoint point)
{
    // Copy every captured coordinate into the actual C zone-query argument
    ZonesPosition result={point.x,point.y,point.z};return result;
}
static float Restart_DistanceSqr(RestartPoint a,RestartPoint b)
{
    // Retain the original component subtraction and ordered sum of squared differences
    RestartPoint delta={a.x-b.x,a.y-b.y,a.z-b.z};return delta.x*delta.x+delta.y*delta.y+delta.z*delta.z;
}

void
Restart_Initialise(void)
{
    // Preserve every original state access, branch decision and transfer order
	Restart_OverridePoliceStationLevel = LEVEL_GENERIC;
	Restart_OverrideHospitalLevel = LEVEL_GENERIC;
	Restart_bFadeInAfterNextArrest = true;
	Restart_bFadeInAfterNextDeath = true;
	Restart_OverrideHeading = 0.0f;
	Restart_OverridePosition = (RestartPoint){0.0f,0.0f,0.0f};
	Restart_bOverrideRestart = false;
	Restart_NumberOfPoliceRestarts = 0;
	Restart_NumberOfHospitalRestarts = 0;

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		Restart_HospitalRestartPoints[i] = (RestartPoint){0.0f,0.0f,0.0f};
		Restart_HospitalRestartHeadings[i] = 0.0f;
		Restart_PoliceRestartPoints[i] = (RestartPoint){0.0f,0.0f,0.0f};
		Restart_PoliceRestartHeadings[i] = 0.0f;
	}
}

void
Restart_AddHospitalRestartPoint(const RestartPosition *pos, float heading)
{
    // Preserve every original state access, branch decision and transfer order
	Restart_HospitalRestartPoints[Restart_NumberOfHospitalRestarts] = Restart_BackendPosition(pos);
	Restart_HospitalRestartHeadings[Restart_NumberOfHospitalRestarts++] = heading;
}

void
Restart_AddPoliceRestartPoint(const RestartPosition *pos, float heading)
{
    // Preserve every original state access, branch decision and transfer order
	Restart_PoliceRestartPoints[Restart_NumberOfPoliceRestarts] = Restart_BackendPosition(pos);
	Restart_PoliceRestartHeadings[Restart_NumberOfPoliceRestarts++] = heading;
}

void
Restart_OverrideNextRestart(const RestartPosition *pos, float heading)
{
    // Preserve every original state access, branch decision and transfer order
	Restart_bOverrideRestart = true;
	Restart_OverridePosition = Restart_BackendPosition(pos);
	Restart_OverrideHeading = heading;
}

void
Restart_CancelOverrideRestart(void)
{
    // Preserve every original state access, branch decision and transfer order
	Restart_bOverrideRestart = false;
}

void
Restart_FindClosestHospitalRestartPoint(const RestartPosition *pos, RestartPosition *outPos, float *outHeading)
{
    // Preserve every original state access, branch decision and transfer order
	if (Restart_bOverrideRestart) {
		Restart_BackendSetPosition(outPos,Restart_OverridePosition);
		*outHeading = Restart_OverrideHeading;
		Restart_CancelOverrideRestart();
		return;
	}

	eLevelName curlevel = Zones_GetLevelFromPosition(Restart_AsZones(Restart_BackendPosition(pos)));
	float fMinDist = (4000.0f*4000.0f);
	int closestPoint = NUM_RESTART_POINTS;

	// find closest point on this level
	for (int i = 0; i < Restart_NumberOfHospitalRestarts; i++) {
		if (Zones_GetLevelFromPosition(Restart_AsZones(Restart_HospitalRestartPoints[i])) == (Restart_OverrideHospitalLevel != LEVEL_GENERIC ? Restart_OverrideHospitalLevel : curlevel)) {
			float dist = Restart_DistanceSqr(Restart_BackendPosition(pos),Restart_HospitalRestartPoints[i]);
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we didn't find anything, find closest point on any level
	if (closestPoint == NUM_RESTART_POINTS) {
		for (int i = 0; i < Restart_NumberOfHospitalRestarts; i++) {
			float dist = Restart_DistanceSqr(Restart_BackendPosition(pos),Restart_HospitalRestartPoints[i]);
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we still didn't find anything, find closest path node
	if (closestPoint == NUM_RESTART_POINTS) {
		Restart_BackendSetPosition(outPos,Restart_BackendClosestNode(pos));
		*outHeading = 0.0f;
		RestartPoint diagnosticPosition=Restart_BackendPosition(pos),diagnosticOutput=Restart_BackendPosition(outPos);
		printf("Couldn't find a hospital restart zone near the player %f %f %f->%f %f %f\n", diagnosticPosition.x,diagnosticPosition.y,diagnosticPosition.z,diagnosticOutput.x,diagnosticOutput.y,diagnosticOutput.z);
	} else {
		Restart_BackendSetPosition(outPos,Restart_HospitalRestartPoints[closestPoint]);
		*outHeading = Restart_HospitalRestartHeadings[closestPoint];
	}
}

void
Restart_FindClosestPoliceRestartPoint(const RestartPosition *pos, RestartPosition *outPos, float *outHeading)
{
    // Preserve every original state access, branch decision and transfer order
	if (Restart_bOverrideRestart) {
		Restart_BackendSetPosition(outPos,Restart_OverridePosition);
		*outHeading = Restart_OverrideHeading;
		Restart_CancelOverrideRestart();
		return;
	}

	eLevelName curlevel = Zones_GetLevelFromPosition(Restart_AsZones(Restart_BackendPosition(pos)));
	float fMinDist = (4000.0f*4000.0f);
	int closestPoint = NUM_RESTART_POINTS;

	// find closest point on this level
	for (int i = 0; i < Restart_NumberOfPoliceRestarts; i++) {
		if (Zones_GetLevelFromPosition(Restart_AsZones(Restart_PoliceRestartPoints[i])) == (Restart_OverridePoliceStationLevel != LEVEL_GENERIC ? Restart_OverridePoliceStationLevel : curlevel)) {
			float dist = Restart_DistanceSqr(Restart_BackendPosition(pos),Restart_PoliceRestartPoints[i]);
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we didn't find anything, find closest point on any level
	if (closestPoint == NUM_RESTART_POINTS) {
		for (int i = 0; i < Restart_NumberOfPoliceRestarts; i++) {
			float dist = Restart_DistanceSqr(Restart_BackendPosition(pos),Restart_PoliceRestartPoints[i]);
			if (fMinDist >= dist) {
				fMinDist = dist;
				closestPoint = i;
			}
		}
	}

	// if we still didn't find anything, find closest path node
	if (closestPoint == NUM_RESTART_POINTS) {
		printf("Couldn't find a police restart zone near the player\n");
		Restart_BackendSetPosition(outPos,Restart_BackendClosestNode(pos));
		*outHeading = 0.0f;
	} else {
		Restart_BackendSetPosition(outPos,Restart_PoliceRestartPoints[closestPoint]);
		*outHeading = Restart_PoliceRestartHeadings[closestPoint];
	}
}

void
Restart_LoadAllRestartPoints(uint8_t *buf, uint32_t size)
{
    // Preserve every original state access, branch decision and transfer order
	Restart_Initialise();

INITSAVEBUF
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	CheckSaveHeader(&buf, 'R', 'S', 'T', '\0', size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Restart_HospitalRestartPoints[i], &buf, sizeof(Restart_HospitalRestartPoints[i]));
		ReadSaveBuf(&Restart_HospitalRestartHeadings[i], &buf, sizeof(Restart_HospitalRestartHeadings[i]));
		//- rouz edit (ChatGPT)
	}

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Restart_PoliceRestartPoints[i], &buf, sizeof(Restart_PoliceRestartPoints[i]));
		ReadSaveBuf(&Restart_PoliceRestartHeadings[i], &buf, sizeof(Restart_PoliceRestartHeadings[i]));
		//- rouz edit (ChatGPT)
	}

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Restart_NumberOfHospitalRestarts, &buf, sizeof(Restart_NumberOfHospitalRestarts));
	ReadSaveBuf(&Restart_NumberOfPoliceRestarts, &buf, sizeof(Restart_NumberOfPoliceRestarts));
	ReadSaveBuf(&Restart_bOverrideRestart, &buf, sizeof(Restart_bOverrideRestart));
	//- rouz edit (ChatGPT)

	// skip something unused
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	SkipSaveBuf(&buf, 3);
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Restart_OverridePosition, &buf, sizeof(Restart_OverridePosition));
	ReadSaveBuf(&Restart_OverrideHeading, &buf, sizeof(Restart_OverrideHeading));
	ReadSaveBuf(&Restart_bFadeInAfterNextDeath, &buf, sizeof(Restart_bFadeInAfterNextDeath));
	ReadSaveBuf(&Restart_bFadeInAfterNextArrest, &buf, sizeof(Restart_bFadeInAfterNextArrest));
	ReadSaveBuf(&Restart_OverrideHospitalLevel, &buf, sizeof(Restart_OverrideHospitalLevel));
	ReadSaveBuf(&Restart_OverridePoliceStationLevel, &buf, sizeof(Restart_OverridePoliceStationLevel));
	//- rouz edit (ChatGPT)
VALIDATESAVEBUF(size);
}

void
Restart_SaveAllRestartPoints(uint8_t *buf, uint32_t *size)
{
    // Preserve every original state access, branch decision and transfer order
	*size = SAVE_HEADER_SIZE
		+ sizeof(Restart_HospitalRestartPoints)
		+ sizeof(Restart_HospitalRestartHeadings)
		+ sizeof(Restart_PoliceRestartPoints)
		+ sizeof(Restart_PoliceRestartHeadings)
		+ sizeof(Restart_NumberOfHospitalRestarts)
		+ sizeof(Restart_NumberOfPoliceRestarts)
		+ sizeof(Restart_bOverrideRestart)
		+ sizeof(uint8_t)
		+ sizeof(uint16_t)
		+ sizeof(Restart_OverridePosition)
		+ sizeof(Restart_OverrideHeading)
		+ sizeof(Restart_bFadeInAfterNextDeath)
		+ sizeof(Restart_bFadeInAfterNextArrest)
		+ sizeof(Restart_OverrideHospitalLevel)
		+ sizeof(Restart_OverridePoliceStationLevel); // == 292

INITSAVEBUF
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveHeader(&buf, 'R', 'S', 'T', '\0', *size - SAVE_HEADER_SIZE);
	//- rouz edit (ChatGPT)

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &Restart_HospitalRestartPoints[i], sizeof(Restart_HospitalRestartPoints[i]));
		WriteSaveBuf(&buf, &Restart_HospitalRestartHeadings[i], sizeof(Restart_HospitalRestartHeadings[i]));
		//- rouz edit (ChatGPT)
	}

	for (int i = 0; i < NUM_RESTART_POINTS; i++) {
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &Restart_PoliceRestartPoints[i], sizeof(Restart_PoliceRestartPoints[i]));
		WriteSaveBuf(&buf, &Restart_PoliceRestartHeadings[i], sizeof(Restart_PoliceRestartHeadings[i]));
		//- rouz edit (ChatGPT)
	}

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &Restart_NumberOfHospitalRestarts, sizeof(Restart_NumberOfHospitalRestarts));
	WriteSaveBuf(&buf, &Restart_NumberOfPoliceRestarts, sizeof(Restart_NumberOfPoliceRestarts));
	WriteSaveBuf(&buf, &Restart_bOverrideRestart, sizeof(Restart_bOverrideRestart));
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	{
		// Materialize the saved value with its original serialized type
		uint8_t saveValue = (uint8_t)0;
		WriteSaveBuf(&buf, &saveValue, sizeof(saveValue));
	}
	{
		// Materialize the saved value with its original serialized type
		uint16_t saveValue = (uint16_t)0;
		WriteSaveBuf(&buf, &saveValue, sizeof(saveValue));
	}
	//- rouz edit (ChatGPT)

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &Restart_OverridePosition, sizeof(Restart_OverridePosition));
	WriteSaveBuf(&buf, &Restart_OverrideHeading, sizeof(Restart_OverrideHeading));
	WriteSaveBuf(&buf, &Restart_bFadeInAfterNextDeath, sizeof(Restart_bFadeInAfterNextDeath));
	WriteSaveBuf(&buf, &Restart_bFadeInAfterNextArrest, sizeof(Restart_bFadeInAfterNextArrest));
	WriteSaveBuf(&buf, &Restart_OverrideHospitalLevel, sizeof(Restart_OverrideHospitalLevel));
	WriteSaveBuf(&buf, &Restart_OverridePoliceStationLevel, sizeof(Restart_OverridePoliceStationLevel));
	//- rouz edit (ChatGPT)
VALIDATESAVEBUF(*size);
}
//- rouz edit (ChatGPT)
