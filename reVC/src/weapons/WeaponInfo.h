//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimGroups.h"
#include "WeaponType.h"
#ifdef __cplusplus
#include "AnimManager.h"
#include "AnimationId.h"
#endif
enum
{
	WEAPONFLAG_USE_GRAVITY = 1,
	WEAPONFLAG_SLOWS_DOWN = 1 << 1,
	WEAPONFLAG_DISSIPATES = 1 << 2,
	WEAPONFLAG_RAND_SPEED = 1 << 3,
	WEAPONFLAG_EXPANDS = 1 << 4,
	WEAPONFLAG_EXPLODES = 1 << 5,
	WEAPONFLAG_CANAIM = 1 << 6,
	WEAPONFLAG_CANAIM_WITHARM = 1 << 7,
	WEAPONFLAG_1ST_PERSON = 1 << 8,
	WEAPONFLAG_HEAVY = 1 << 9,
	WEAPONFLAG_THROW = 1 << 10,
	WEAPONFLAG_RELOAD_LOOP2START = 1 << 11,
	WEAPONFLAG_USE_2ND = 1 << 12,
	WEAPONFLAG_GROUND_2ND = 1 << 13,
	WEAPONFLAG_FINISH_3RD = 1 << 14,
	WEAPONFLAG_RELOAD = 1 << 15,
	WEAPONFLAG_FIGHTMODE = 1 << 16,
	WEAPONFLAG_CROUCHFIRE = 1 << 17,
	WEAPONFLAG_COP3_RD = 1 << 18,
	WEAPONFLAG_GROUND_3RD = 1 << 19,
	WEAPONFLAG_PARTIALATTACK = 1 << 20,
	WEAPONFLAG_ANIMDETONATE = 1 << 21,
};
typedef struct WeaponInfoVector {float x,y,z;} WeaponInfoVector;
typedef struct CWeaponInfo {
	enum eWeaponFire m_eWeaponFire;
	float m_fRange;
	uint32_t m_nFiringRate;
	uint32_t m_nReload;
	int32_t m_nAmountofAmmunition;
	uint32_t m_nDamage;
	float m_fSpeed;
	float m_fRadius;
	float m_fLifespan;
	float m_fSpread;
	WeaponInfoVector m_vecFireOffset;
	enum AssocGroupId m_AnimToPlay;
	float m_fAnimLoopStart;
	float m_fAnimLoopEnd;
	float m_fAnimFrameFire;
	float m_fAnim2LoopStart;
	float m_fAnim2LoopEnd;
	float m_fAnim2FrameFire;
	float m_fAnimBreakout;
	int32_t m_nModelId;
	int32_t m_nModel2Id;
	uint32_t m_Flags;

	uint32_t m_nWeaponSlot;

} CWeaponInfo;
#ifdef __cplusplus
extern "C" {
#endif
extern uint16_t WeaponInfo_ms_aReloadSampleTime[WEAPONTYPE_TOTALWEAPONS];
extern int32_t WeaponInfo_ms_aMaxAmmoForWeapon[WEAPONTYPE_TOTALWEAPONS];
void WeaponInfo_Initialise(void);
void WeaponInfo_LoadWeaponData(void);
CWeaponInfo *WeaponInfo_GetWeaponInfo(enum eWeaponType weapon);
enum eWeaponFire WeaponInfo_FindWeaponFireType(char *name);
enum eWeaponType WeaponInfo_FindWeaponType(char *name);
void WeaponInfo_Shutdown(void);
bool WeaponInfo_IsWeaponSlotAmmoMergeable(uint32_t slot);
bool WeaponInfo_IsFlagSet(const CWeaponInfo *info,uint32_t flag);
#ifdef __cplusplus
}
class CVector;
CVector WeaponInfo_VectorToNative(WeaponInfoVector value);
#endif
//- rouz edit (ChatGPT)
