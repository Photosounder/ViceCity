//+ rouz edit (Claude)
#include <stdio.h>
#include <stddef.h>
#include "config.h"
#include "FileMgr.h"
#include "WorkBuffer.h"
#include "ModelRegistry.h"
#include "ObjectData.h"

void re3_debug(const char *format, ...);
void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#ifndef MASTER
#define ObjectData_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define ObjectData_Assert(expression) (expression)
#endif

#define GRAVITY (0.008f)

static CObjectInfo ms_aObjectInfo[NUMOBJECTINFO];

const CObjectInfo *ObjectData_Info(int id)
{
	// Give read access to one object info entry
	return &ms_aObjectInfo[id];
}

// Another ugly file reader
void
ObjectData_Initialise(const char *filename)
{
	char *p, *lp;
	char line[1024], name[256];
	int id;
	float percentSubmerged;
	int damageEffect, responseCase, camAvoid;
	void *mi;
	
	ms_aObjectInfo[0].m_fMass = 99999.0f;
	ms_aObjectInfo[0].m_fTurnMass = 99999.0f;
	ms_aObjectInfo[0].m_fAirResistance = 0.99f;
	ms_aObjectInfo[0].m_fElasticity = 0.1f;
	ms_aObjectInfo[0].m_fBuoyancy = GRAVITY * ms_aObjectInfo[0].m_fMass * 2.0f;
	ms_aObjectInfo[0].m_fUprootLimit = 0.0f;
	ms_aObjectInfo[0].m_fCollisionDamageMultiplier = 1.0f;
	ms_aObjectInfo[0].m_nCollisionDamageEffect = 0;
	ms_aObjectInfo[0].m_nSpecialCollisionResponseCases = 0;
	ms_aObjectInfo[0].m_bCameraToAvoidThisObject = false;

	ms_aObjectInfo[1].m_fMass = 99999.0f;
	ms_aObjectInfo[1].m_fTurnMass = 99999.0f;
	ms_aObjectInfo[1].m_fAirResistance = 0.99f;
	ms_aObjectInfo[1].m_fElasticity = 0.1f;
	ms_aObjectInfo[1].m_fBuoyancy = ms_aObjectInfo[0].m_fBuoyancy;
	ms_aObjectInfo[1].m_fUprootLimit = 0.0f;
	ms_aObjectInfo[1].m_fCollisionDamageMultiplier = 1.0f;
	ms_aObjectInfo[1].m_nCollisionDamageEffect = 0;
	ms_aObjectInfo[1].m_nSpecialCollisionResponseCases = 0;
	ms_aObjectInfo[1].m_bCameraToAvoidThisObject = true;

	ms_aObjectInfo[2].m_fMass = 99999.0f;
	ms_aObjectInfo[2].m_fTurnMass = 99999.0f;
	ms_aObjectInfo[2].m_fAirResistance = 0.99f;
	ms_aObjectInfo[2].m_fElasticity = 0.1f;
	ms_aObjectInfo[2].m_fBuoyancy = ms_aObjectInfo[0].m_fBuoyancy;
	ms_aObjectInfo[2].m_fUprootLimit = 0.0f;
	ms_aObjectInfo[2].m_fCollisionDamageMultiplier = 1.0f;
	ms_aObjectInfo[2].m_nCollisionDamageEffect = 0;
	ms_aObjectInfo[2].m_bCameraToAvoidThisObject = false;
	ms_aObjectInfo[2].m_nSpecialCollisionResponseCases = 4;

	ms_aObjectInfo[3].m_fMass = 99999.0f;
	ms_aObjectInfo[3].m_fTurnMass = 99999.0f;
	ms_aObjectInfo[3].m_fAirResistance = 0.99f;
	ms_aObjectInfo[3].m_fElasticity = 0.1f;
	ms_aObjectInfo[3].m_fBuoyancy = ms_aObjectInfo[0].m_fBuoyancy;
	ms_aObjectInfo[3].m_fUprootLimit = 0.0f;
	ms_aObjectInfo[3].m_fCollisionDamageMultiplier = 1.0f;
	ms_aObjectInfo[3].m_nCollisionDamageEffect = 0;
	ms_aObjectInfo[3].m_nSpecialCollisionResponseCases = 4;
	ms_aObjectInfo[3].m_bCameraToAvoidThisObject = true;

	FileMgr_SetDir("");
	FileMgr_LoadFile(filename, work_buff, sizeof(work_buff), "r");

	id = 4;
	p = (char*)work_buff;
	while(*p != '*'){
		// skip over white space and comments
		while(*p == ' ' || *p == '\n' || *p == '\r' || *p == ';')
			if(*p == ';')
				while(*p != '\n' && *p != '*')
					p++;
			else
				p++;

		if(*p == '*')
			break;

		// read one line
		lp = line;
		while(*p != '\n' && *p != '*'){
			*lp++ = *p == ',' ? ' ' : *p;
			p++;
		}
		if(*p == '\n')
			p++;
#ifdef FIX_BUGS
		*lp = '\0';	// FIX: game wrote '\n' here
#else
		*lp = '\n';
#endif

		ObjectData_Assert(id < NUMOBJECTINFO);
		sscanf(line, "%s %f %f %f %f %f %f %f %d %d %d", name,
			&ms_aObjectInfo[id].m_fMass,
			&ms_aObjectInfo[id].m_fTurnMass,
			&ms_aObjectInfo[id].m_fAirResistance,
			&ms_aObjectInfo[id].m_fElasticity,
			&percentSubmerged,
			&ms_aObjectInfo[id].m_fUprootLimit,
			&ms_aObjectInfo[id].m_fCollisionDamageMultiplier,
			&damageEffect, &responseCase, &camAvoid);

		ms_aObjectInfo[id].m_fBuoyancy = 100.0f/percentSubmerged * GRAVITY *ms_aObjectInfo[id].m_fMass;
		ms_aObjectInfo[id].m_nCollisionDamageEffect = damageEffect;
		ms_aObjectInfo[id].m_nSpecialCollisionResponseCases = responseCase;
		ms_aObjectInfo[id].m_bCameraToAvoidThisObject = camAvoid;

		mi = ModelRegistry_Find(name, 0, MODELINFOSIZE-1, NULL);
		if (mi) {
			if (ms_aObjectInfo[0].m_fMass != ms_aObjectInfo[id].m_fMass
				|| ms_aObjectInfo[0].m_fCollisionDamageMultiplier != ms_aObjectInfo[id].m_fCollisionDamageMultiplier
				|| ms_aObjectInfo[0].m_nCollisionDamageEffect != ms_aObjectInfo[id].m_nCollisionDamageEffect
				|| ((ms_aObjectInfo[0].m_nSpecialCollisionResponseCases != ms_aObjectInfo[id].m_nSpecialCollisionResponseCases)
					&& (ms_aObjectInfo[2].m_nSpecialCollisionResponseCases != ms_aObjectInfo[id].m_nSpecialCollisionResponseCases))) {
				ObjectData_BackendSetModelObjectID(mi, id++);
			} else if (ms_aObjectInfo[0].m_nSpecialCollisionResponseCases == ms_aObjectInfo[id].m_nSpecialCollisionResponseCases) {
				if (ms_aObjectInfo[0].m_bCameraToAvoidThisObject == ms_aObjectInfo[id].m_bCameraToAvoidThisObject)
					ObjectData_BackendSetModelObjectID(mi, 0);
				else
					ObjectData_BackendSetModelObjectID(mi, 1);
			} else if (ms_aObjectInfo[2].m_bCameraToAvoidThisObject == ms_aObjectInfo[id].m_bCameraToAvoidThisObject)
				ObjectData_BackendSetModelObjectID(mi, 2);
			else 
				ObjectData_BackendSetModelObjectID(mi, 3);
		} else
			re3_debug("CObjectData: Cannot find object %s\n", name);
	}
}
//- rouz edit (Claude)
