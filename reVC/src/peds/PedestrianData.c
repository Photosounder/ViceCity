//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "PedType.h"
#include "PedData_backend.h"
#include "General.h"
#include "FileMgr.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *, const char *, unsigned int, const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression, __FILE__, __LINE__, __func__), 0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
#include <stdio.h>
#include <string.h>
_Static_assert(sizeof(PedType) == 0x20, "Original pedestrian type record layout");
_Static_assert(sizeof(PedStats) == 0x34, "Original pedestrian statistics record layout");
PedType *PedType_records[NUM_PEDTYPES];
PedStats *PedStats_records[NUM_PEDSTATS];

void
PedType_Initialise(void)
{
    // Execute the complete original pedestrian type Initialise operation
	int i;

	PedData_BackendDebug("Initialising CPedType...\n");
	for(i = 0; i < NUM_PEDTYPES; i++){

		// Allocate each record through the existing native provider
		PedType_records[i] = (PedType*)PedData_BackendAllocate(sizeof(PedType));
		// Preserve value initialization, including the previously implicit zero fields
		memset(PedType_records[i], 0, sizeof(*PedType_records[i]));

		PedType_records[i]->m_flag = PED_FLAG_PLAYER1;
		PedType_records[i]->unknown1 = 0.0f;
		PedType_records[i]->unknown2 = 0.0f;
		// Unknown3 retains the original implicit value initialization
		PedType_records[i]->unknown4 = 0.0f;
		PedType_records[i]->unknown5 = 0.0f;
		PedType_records[i]->m_threats = 0;
		PedType_records[i]->m_avoid = 0;
	}
	PedData_BackendDebug("Loading ped data...\n");
	PedType_LoadPedData();
	PedData_BackendDebug("CPedType ready\n");
}

void
PedType_Shutdown(void)
{
    // Execute the complete original pedestrian type Shutdown operation
	int i;
	PedData_BackendDebug("Shutting down CPedType...\n");
	for(i = 0; i < NUM_PEDTYPES; i++)

		// Release each trivially destructible record through the native provider
	{
		PedData_BackendFree(PedType_records[i]);
	}

	PedData_BackendDebug("CPedType shut down\n");
}

void
PedType_LoadPedData(void)
{
    // Execute the complete original pedestrian type LoadPedData operation
	char *buf;
	char line[256];
	char word[32];
	FileMgrSize bp, buflen;
	int lp, linelen;
	int type;
	uint32_t flags;
	float f1, f2, f3, f4, f5;

	type = NUM_PEDTYPES;
	buf = (char*)PedData_BackendAllocate(16 * 1024);

	FileMgr_SetDir("DATA");
	buflen = FileMgr_LoadFile("PED.DAT", (uint8_t*)buf, 16 * 1024, "r");
	FileMgr_SetDir("");

	for(bp = 0; bp < buflen; ){
		// read file line by line
		for(linelen = 0; buf[bp] != '\n' && bp < buflen; bp++){
			if(buf[bp] == '\r' || buf[bp] == ',' || buf[bp] == '\t')
				line[linelen++] = ' ';
			else
				line[linelen++] = buf[bp];
		}
		bp++;
		line[linelen] = '\0';

		// skip white space
		for(lp = 0; line[lp] <= ' '; lp++);

		if(lp >= linelen ||		// FIX: game uses == here, but this is safer if we have empty lines
		   line[lp] == '#')
			continue;

		// Game uses just "line" here since sscanf already trims whitespace, but this is safer
		sscanf(&line[lp], "%s", word);

		if(strcmp(word, "Threat") == 0){
			flags = 0;
			lp += 7;
			while(sscanf(&line[lp], "%s", word) == 1 && lp <= linelen){
				flags |= PedType_FindPedFlag(word);
				// skip word
				while(line[lp] != ' ' && line[lp] != '\n' && line[lp] != '\0')
					lp++;
				// skip white space
				while(line[lp] == ' ')
					lp++;
			}
			PedType_records[type]->m_threats = flags;
		}else if(strcmp(word, "Avoid") == 0){
			flags = 0;
			lp += 6;
			while(sscanf(&line[lp], "%s", word) == 1 && lp <= linelen){
				flags |= PedType_FindPedFlag(word);
				// skip word
				while(line[lp] != ' ' && line[lp] != '\n' && line[lp] != '\0')
					lp++;
				// skip white space
				while(line[lp] == ' ')
					lp++;
			}
			PedType_records[type]->m_avoid = flags;
		}else{
			sscanf(line, "%s %f %f %f %f %f", word, &f1, &f2, &f3, &f4, &f5);
			type = PedType_FindPedType(word);
			PedType_records[type]->m_flag = PedType_FindPedFlag(word);
			// unknown values
			PedType_records[type]->unknown1 = f1 / 50.0f;
			PedType_records[type]->unknown2 = f2 / 50.0f;
			PedType_records[type]->unknown3 = f3 / 50.0f;
			PedType_records[type]->unknown4 = f4;
			PedType_records[type]->unknown5 = f5;

		}
	}

	PedData_BackendFree(buf);
}

ePedType
PedType_FindPedType(char *type)
{
    // Execute the complete original pedestrian type FindPedType operation
	if(strcmp(type, "PLAYER1") == 0) return PEDTYPE_PLAYER1;
	if(strcmp(type, "PLAYER2") == 0) return PEDTYPE_PLAYER2;
	if(strcmp(type, "PLAYER3") == 0) return PEDTYPE_PLAYER3;
	if(strcmp(type, "PLAYER4") == 0) return PEDTYPE_PLAYER4;
	if(strcmp(type, "CIVMALE") == 0) return PEDTYPE_CIVMALE;
	if(strcmp(type, "CIVFEMALE") == 0) return PEDTYPE_CIVFEMALE;
	if(strcmp(type, "COP") == 0) return PEDTYPE_COP;
	if(strcmp(type, "GANG1") == 0) return PEDTYPE_GANG1;
	if(strcmp(type, "GANG2") == 0) return PEDTYPE_GANG2;
	if(strcmp(type, "GANG3") == 0) return PEDTYPE_GANG3;
	if(strcmp(type, "GANG4") == 0) return PEDTYPE_GANG4;
	if(strcmp(type, "GANG5") == 0) return PEDTYPE_GANG5;
	if(strcmp(type, "GANG6") == 0) return PEDTYPE_GANG6;
	if(strcmp(type, "GANG7") == 0) return PEDTYPE_GANG7;
	if(strcmp(type, "GANG8") == 0) return PEDTYPE_GANG8;
	if(strcmp(type, "GANG9") == 0) return PEDTYPE_GANG9;
	if(strcmp(type, "EMERGENCY") == 0) return PEDTYPE_EMERGENCY;
	if(strcmp(type, "FIREMAN") == 0) return PEDTYPE_FIREMAN;
	if(strcmp(type, "CRIMINAL") == 0) return PEDTYPE_CRIMINAL;
	if(strcmp(type, "SPECIAL") == 0) return PEDTYPE_SPECIAL;
	if(strcmp(type, "PROSTITUTE") == 0) return PEDTYPE_PROSTITUTE;
	PedData_BackendUnknownType();
	return NUM_PEDTYPES;
}

uint32_t
PedType_FindPedFlag(char *type)
{
    // Execute the complete original pedestrian type FindPedFlag operation
	if(strcmp(type, "PLAYER1") == 0) return PED_FLAG_PLAYER1;
	if(strcmp(type, "PLAYER2") == 0) return PED_FLAG_PLAYER2;
	if(strcmp(type, "PLAYER3") == 0) return PED_FLAG_PLAYER3;
	if(strcmp(type, "PLAYER4") == 0) return PED_FLAG_PLAYER4;
	if(strcmp(type, "CIVMALE") == 0) return PED_FLAG_CIVMALE;
	if(strcmp(type, "CIVFEMALE") == 0) return PED_FLAG_CIVFEMALE;
	if(strcmp(type, "COP") == 0) return PED_FLAG_COP;
	if(strcmp(type, "GANG1") == 0) return PED_FLAG_GANG1;
	if(strcmp(type, "GANG2") == 0) return PED_FLAG_GANG2;
	if(strcmp(type, "GANG3") == 0) return PED_FLAG_GANG3;
	if(strcmp(type, "GANG4") == 0) return PED_FLAG_GANG4;
	if(strcmp(type, "GANG5") == 0) return PED_FLAG_GANG5;
	if(strcmp(type, "GANG6") == 0) return PED_FLAG_GANG6;
	if(strcmp(type, "GANG7") == 0) return PED_FLAG_GANG7;
	if(strcmp(type, "GANG8") == 0) return PED_FLAG_GANG8;
	if(strcmp(type, "GANG9") == 0) return PED_FLAG_GANG9;
	if(strcmp(type, "EMERGENCY") == 0) return PED_FLAG_EMERGENCY;
	if(strcmp(type, "FIREMAN") == 0) return PED_FLAG_FIREMAN;
	if(strcmp(type, "CRIMINAL") == 0) return PED_FLAG_CRIMINAL;
	if(strcmp(type, "SPECIAL") == 0) return PED_FLAG_SPECIAL;
	if(strcmp(type, "GUN") == 0) return PED_FLAG_GUN;
	if(strcmp(type, "COP_CAR") == 0) return PED_FLAG_COP_CAR;
	if(strcmp(type, "FAST_CAR") == 0) return PED_FLAG_FAST_CAR;
	if(strcmp(type, "EXPLOSION") == 0) return PED_FLAG_EXPLOSION;
	if(strcmp(type, "PROSTITUTE") == 0) return PED_FLAG_PROSTITUTE;
	if(strcmp(type, "DEADPEDS") == 0) return PED_FLAG_DEADPEDS;
	return 0;
}

void
PedType_Save(uint8_t *buf, uint32_t *size)
{
    // Execute the complete original pedestrian type Save operation
	*size = sizeof(PedType) * NUM_PEDTYPES + SAVE_HEADER_SIZE;
INITSAVEBUF

	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveHeader(&buf, 'P', 'T', 'P', '\0', *size - SAVE_HEADER_SIZE);

	for(int i = 0; i < NUM_PEDTYPES; i++)

		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &(*PedType_records[i]), sizeof(*PedType_records[i]));

VALIDATESAVEBUF(*size)
}

void
PedType_Load(uint8_t *buf, uint32_t size)
{
    // Execute the complete original pedestrian type Load operation
INITSAVEBUF
	// original: SkipSaveBuf(buf, SAVE_HEADER_SIZE);

	// Transfer save data through the C buffer API with explicit sizes
	CheckSaveHeader(&buf, 'P', 'T', 'P', '\0', size - SAVE_HEADER_SIZE);


	for(int i = 0; i < NUM_PEDTYPES; i++)

		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(PedType_records[i], &buf, sizeof(*PedType_records[i]));

VALIDATESAVEBUF(size)
}

void
PedStats_Initialise(void)
{
    // Execute the complete original pedestrian statistics Initialise operation
	int i;

	PedData_BackendDebug("Initialising CPedStats...\n");
	for(i = 0; i < NUM_PEDSTATS; i++){

		// Allocate each record through the existing native provider
		PedStats_records[i] = (PedStats*)PedData_BackendAllocate(sizeof(PedStats));
		// Preserve value initialization, including the previously implicit zero fields
		memset(PedStats_records[i], 0, sizeof(*PedStats_records[i]));

		PedStats_records[i]->m_type = PEDSTAT_PLAYER;
		PedStats_records[i]->m_name[8] = 'R';	// WHAT?
		PedStats_records[i]->m_fleeDistance = 20.0f;
		PedStats_records[i]->m_headingChangeRate = 15.0f;
		PedStats_records[i]->m_fear = 50;
		PedStats_records[i]->m_temper = 50;
		PedStats_records[i]->m_lawfulness = 50;
		PedStats_records[i]->m_sexiness = 50;
		PedStats_records[i]->m_attackStrength = 1.0f;
		PedStats_records[i]->m_defendWeakness = 1.0f;
		PedStats_records[i]->m_flags = 0;
	}
	PedData_BackendDebug("Loading pedstats data...\n");
	PedStats_LoadPedStats();
	PedData_BackendDebug("CPedStats ready\n");
}

void
PedStats_Shutdown(void)
{
    // Execute the complete original pedestrian statistics Shutdown operation
	int i;
	PedData_BackendDebug("Shutting down CPedStats...\n");
	for(i = 0; i < NUM_PEDSTATS; i++)

		// Release each trivially destructible record through the native provider
	{
		PedData_BackendFree(PedStats_records[i]);
	}

	PedData_BackendDebug("CPedStats shut down\n");
}

void
PedStats_LoadPedStats(void)
{
    // Execute the complete original pedestrian statistics LoadPedStats operation
	char *buf;
	char line[256];
	char name[32];
	FileMgrSize bp, buflen;
	int lp, linelen;
	int type;
	float fleeDist, headingChangeRate, attackStrength, defendWeakness;
	int fear, temper, lawfullness, sexiness, flags;

	type = 0;
	buf = (char*)PedData_BackendAllocate(16 * 1024);

	FileMgr_SetDir("DATA");
	buflen = FileMgr_LoadFile("PEDSTATS.DAT", (uint8_t*)buf, 16 * 1024, "r");
	FileMgr_SetDir("");

	for(bp = 0; bp < buflen; ){
		// read file line by line
		for(linelen = 0; buf[bp] != '\n' && bp < buflen; bp++){
			if(buf[bp] == '\r' || buf[bp] == ',' || buf[bp] == '\t')
				line[linelen++] = ' ';
			else
				line[linelen++] = buf[bp];
		}
		bp++;
		line[linelen] = '\0';

		// skip white space
		for(lp = 0; line[lp] <= ' '; lp++);

		if(lp >= linelen ||		// FIX: game uses == here, but this is safer if we have empty lines
		   line[lp] == '#')
			continue;

		sscanf(&line[lp], "%s %f %f %d %d %d %d %f %f %d",
			name,
			&fleeDist,
			&headingChangeRate,
			&fear,
			&temper,
			&lawfullness,
			&sexiness,
			&attackStrength,
			&defendWeakness,
			&flags);
		PedStats_records[type]->m_type = (ePedStats)type;
		strncpy(PedStats_records[type]->m_name, name, 24);	// FIX: game uses strcpy
		PedStats_records[type]->m_fleeDistance = fleeDist;
		PedStats_records[type]->m_headingChangeRate = headingChangeRate;
		PedStats_records[type]->m_fear = fear;
		PedStats_records[type]->m_temper = temper;
		PedStats_records[type]->m_lawfulness = lawfullness;
		PedStats_records[type]->m_sexiness = sexiness;
		PedStats_records[type]->m_attackStrength = attackStrength;
		PedStats_records[type]->m_defendWeakness = defendWeakness;
		PedStats_records[type]->m_flags = flags;
		type++;
	}

	PedData_BackendFree(buf);
}

ePedStats
PedStats_GetPedStatType(char *name)
{
    // Execute the complete original pedestrian statistics GetPedStatType operation
	for(uint16_t type = 0; type < NUM_PEDSTATS; type++)
		if(!General_faststrcmp(PedStats_records[type]->m_name, name))
			return (ePedStats) type;

	return NUM_PEDSTATS;
}

uint32_t PedType_GetFlag(int type)
{
    // Preserve the original live record access and bit-mask conversion
    return PedType_records[type]->m_flag;
}

uint32_t PedType_GetAvoid(int type)
{
    // Preserve the original live record access and bit-mask conversion
    return PedType_records[type]->m_avoid;
}

uint32_t PedType_GetThreats(int type)
{
    // Preserve the original live record access and bit-mask conversion
    return PedType_records[type]->m_threats;
}

void PedType_SetThreats(int type, uint32_t threat)
{
    // Preserve the original live record access and bit-mask conversion
    PedType_records[type]->m_threats = threat;
}

void PedType_AddThreat(int type, int threat)
{
    // Preserve the original live record access and bit-mask conversion
    PedType_records[type]->m_threats |= threat;
}

void PedType_RemoveThreat(int type, int threat)
{
    // Preserve the original live record access and bit-mask conversion
    PedType_records[type]->m_threats &= ~threat;
}

bool PedType_IsThreat(int type, int threat)
{
    // Preserve the original live record access and bit-mask conversion
    return PedType_records[type]->m_threats & threat;
}
//- rouz edit (ChatGPT)
