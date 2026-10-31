//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ModelIndices.h"
#include "WeaponType.h"
#ifdef __cplusplus
class CVehicle;class CPed;
extern "C" {
#else
typedef struct CVehicle CVehicle;typedef struct CPed CPed;
#endif
enum
{
	KILLFRENZY_NONE,
	KILLFRENZY_ONGOING,
	KILLFRENZY_PASSED,
	KILLFRENZY_FAILED,
};

extern int32_t Darkel_TimeLimit;
extern int32_t Darkel_PreviousTime;
extern int32_t Darkel_TimeOfFrenzyStart;
extern int32_t Darkel_WeaponType;
extern int32_t Darkel_AmmoInterruptedWeapon;
extern int32_t Darkel_KillsNeeded;
extern int32_t Darkel_InterruptedWeaponType;
extern int32_t Darkel_InterruptedWeaponSelected;
extern bool Darkel_bStandardSoundAndMessages;
extern bool Darkel_bNeedHeadShot;
extern bool Darkel_bProperKillFrenzy;
extern uint16_t Darkel_Status;
extern uint16_t Darkel_RegisteredKills[NUM_DEFAULT_MODELS];
extern int32_t Darkel_ModelToKill;
extern int32_t Darkel_ModelToKill2;
extern int32_t Darkel_ModelToKill3;
extern int32_t Darkel_ModelToKill4;
extern uint16_t *Darkel_pStartMessage;
uint8_t Darkel_CalcFade(uint32_t time,uint32_t start,uint32_t end);
void Darkel_DrawMessages(void);
bool Darkel_FrenzyOnGoing(void);
void Darkel_Init(void);
uint16_t Darkel_QueryModelsKilledByPlayer(int32_t modelId);
uint16_t Darkel_ReadStatus(void);
void Darkel_RegisterCarBlownUpByPlayer(CVehicle *vehicle);
void Darkel_RegisterKillByPlayer(CPed *victim,enum eWeaponType weapon,bool headshot);
void Darkel_RegisterKillNotByPlayer(CPed *victim,enum eWeaponType weapontype);
void Darkel_ResetModelsKilledByPlayer(void);
void Darkel_ResetOnPlayerDeath(void);
void Darkel_StartFrenzy(enum eWeaponType weaponType,int32_t time,uint16_t kill,int32_t modelId0,uint16_t *text,int32_t modelId2,int32_t modelId3,int32_t modelId4,bool standardSound,bool needHeadShot);
void Darkel_Update(void);
void Darkel_DealWithWeaponChangeAtEndOfFrenzy(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
