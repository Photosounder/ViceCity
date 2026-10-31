//+ rouz edit (ChatGPT)
#include "Gangs.h"
#include "Gangs_backend.h"
#include "General.h"
#include "ModelIndexData.h"
#include "WeaponType.h"
#undef assert
#ifndef MASTER
void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "SaveBuf.h"
_Static_assert(NUM_GANGS==9,"Original gang count for constructor-equivalent static initialization");
_Static_assert(sizeof(GangInfo)==24,"Current original gang record layout");
static GangInfo Gangs_records[NUM_GANGS]={
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED},
    {-1,-1,-1,-1,WEAPONTYPE_UNARMED,WEAPONTYPE_UNARMED}
};
static bool Gangs_attackWithCops[NUM_GANGS];

void Gangs_Initialise(void)
{
    // Restore the original eight gang defaults while retaining unrelated live fields
	Gangs_SetGangPedModels(GANG_CUBAN, MI_CBA, MI_CBB);
	Gangs_SetGangPedModels(GANG_HAITIAN, MI_HNA, MI_HNB);
	Gangs_SetGangPedModels(GANG_STREET, MI_SGA, MI_SGB);
	Gangs_SetGangPedModels(GANG_DIAZ, MI_CLA, MI_CLB);
	Gangs_SetGangPedModels(GANG_SECURITY, MI_GDA, MI_GDB);
	Gangs_SetGangPedModels(GANG_BIKER, MI_BKA, MI_BKB);
	Gangs_SetGangPedModels(GANG_PLAYER, MI_PGA, MI_PGB);
	Gangs_SetGangPedModels(GANG_GOLFER, MI_WFOGO, MI_WMOGO);
	Gangs_SetGangVehicleModel(GANG_CUBAN, MI_CUBAN);
	Gangs_SetGangVehicleModel(GANG_HAITIAN, MI_VOODOO);
	Gangs_SetGangVehicleModel(GANG_STREET, MI_GANGBUR);
	Gangs_SetGangVehicleModel(GANG_DIAZ, -1);
	Gangs_SetGangVehicleModel(GANG_SECURITY, -1);
	Gangs_SetGangVehicleModel(GANG_BIKER, MI_ANGEL);
	Gangs_SetGangVehicleModel(GANG_PLAYER, -1);
	Gangs_SetGangVehicleModel(GANG_GOLFER, MI_CADDY);
	Gangs_SetGangWeapons(GANG_GOLFER, WEAPONTYPE_GOLFCLUB, WEAPONTYPE_GOLFCLUB);
#ifdef FIX_BUGS
	for (int i = 0; i < NUM_GANGS; i++)
		Gangs_SetGangPedModelOverride(i, -1);
#endif
}

bool Gangs_HaveGangModelsLoaded(int16_t gang)
{
    // Query the second live model only when the first streaming query succeeds
	GangInfo* pGangInfo = Gangs_GetGangInfo(gang);
	return Gangs_BackendHasModelLoaded(pGangInfo->m_nPedModel1MI) && Gangs_BackendHasModelLoaded(pGangInfo->m_nPedModel2MI);
}

void Gangs_SetGangPedModels(int16_t gang, int32_t mi1, int32_t mi2)
{
    // Write both original model fields through the same canonical record
	Gangs_GetGangInfo(gang)->m_nPedModel1MI = mi1;
	Gangs_GetGangInfo(gang)->m_nPedModel2MI = mi2;
}

void Gangs_SetWillAttackPlayerWithCops(ePedType type, bool will)
{
    // Change attack policy only for the original nine gang pedestrian types
	if (type >= PEDTYPE_GANG1 && type <= PEDTYPE_GANG9)
		Gangs_attackWithCops[type - PEDTYPE_GANG1] = will;
}

bool Gangs_GetWillAttackPlayerWithCops(ePedType type)
{
    // Read attack policy only for the original valid gang pedestrian range
	if (type >= PEDTYPE_GANG1 && type <= PEDTYPE_GANG9)
		return Gangs_attackWithCops[type - PEDTYPE_GANG1];
	return false;
}

int32_t Gangs_ChooseGangPedModel(int16_t gang)
{
    // Preserve override short-circuiting and the original random draw
	GangInfo* pGangInfo = Gangs_GetGangInfo(gang);
	if (pGangInfo->m_nPedModelOverride != -1 || General_GetRandomTrueFalse())
		return pGangInfo->m_nPedModel1MI;
	else
		return pGangInfo->m_nPedModel2MI;
}

void Gangs_SetGangVehicleModel(int16_t gang, int32_t model)
{
    // Update the live vehicle model without narrowing its signed value
	Gangs_GetGangInfo(gang)->m_nVehicleMI = model;
}

void Gangs_SetGangWeapons(int16_t gang, int32_t weapon1, int32_t weapon2)
{
    // Write both original signed weapon fields in order
	GangInfo *gi = Gangs_GetGangInfo(gang);
	gi->m_Weapon1 = weapon1;
	gi->m_Weapon2 = weapon2;
}

void Gangs_SetGangPedModelOverride(int16_t gang, int8_t ovrd)
{
    // Store the original signed-byte override without interpreting its value
	Gangs_GetGangInfo(gang)->m_nPedModelOverride = ovrd;
}

int8_t Gangs_GetGangPedModelOverride(int16_t gang)
{
    // Read the live signed-byte override unchanged
	return Gangs_GetGangInfo(gang)->m_nPedModelOverride;
}

void Gangs_SaveAllGangData(uint8_t *buf, uint32_t *size)
{
    // Preserve the original complete record assignments and save-header order
INITSAVEBUF

	*size = SAVE_HEADER_SIZE + sizeof(Gangs_records);

	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveHeader(&buf, 'G', 'N', 'G', '\0', *size - SAVE_HEADER_SIZE);

	for (int i = 0; i < NUM_GANGS; i++)

		// Transfer save data through the C buffer API with explicit sizes
		{
			// Preserve record assignment semantics and compiler padding behavior
			*(GangInfo*)buf = Gangs_records[i];
			SkipSaveBuf(&buf, sizeof(Gangs_records[i]));
		}


VALIDATESAVEBUF(*size);
}

void Gangs_LoadAllGangData(uint8_t *buf, uint32_t size)
{
    // Initialize at the original point before validating and assigning saved records
	Gangs_Initialise();

INITSAVEBUF

	// Transfer save data through the C buffer API with explicit sizes
	CheckSaveHeader(&buf, 'G', 'N', 'G', '\0', size - SAVE_HEADER_SIZE);


	for (int i = 0; i < NUM_GANGS; i++)

		// Transfer save data through the C buffer API with explicit sizes
		{
			// Preserve record assignment semantics and compiler padding behavior
			Gangs_records[i] = *(GangInfo*)buf;
			SkipSaveBuf(&buf, sizeof(Gangs_records[i]));
		}

VALIDATESAVEBUF(size);
}

int32_t Gangs_GetGangPedModel1(int16_t gang)
{
    // Access the original canonical gang slot without changing its index contract
    return Gangs_records[gang].m_nPedModel1MI;
}

int32_t Gangs_GetGangPedModel2(int16_t gang)
{
    // Access the original canonical gang slot without changing its index contract
    return Gangs_records[gang].m_nPedModel2MI;
}

int32_t Gangs_GetGangVehicleModel(int16_t gang)
{
    // Access the original canonical gang slot without changing its index contract
    return Gangs_records[gang].m_nVehicleMI;
}

GangInfo * Gangs_GetGangInfo(int16_t gang)
{
    // Access the original canonical gang slot without changing its index contract
    return &Gangs_records[gang];
}
//- rouz edit (ChatGPT)
