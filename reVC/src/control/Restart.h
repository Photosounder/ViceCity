//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "GameLevels.h"
#define NUM_RESTART_POINTS 8
typedef struct RestartPoint {float x,y,z;} RestartPoint;
#ifdef __cplusplus
class CVector;
typedef CVector RestartPosition;
extern "C" {
#else
typedef struct CVector RestartPosition;
#endif
extern uint8_t Restart_OverrideHospitalLevel;
extern uint8_t Restart_OverridePoliceStationLevel;
extern bool Restart_bFadeInAfterNextArrest;
extern bool Restart_bFadeInAfterNextDeath;
extern bool Restart_bOverrideRestart;
extern RestartPoint Restart_OverridePosition;
extern float Restart_OverrideHeading;
extern uint16_t Restart_NumberOfHospitalRestarts;
extern uint16_t Restart_NumberOfPoliceRestarts;
extern RestartPoint Restart_HospitalRestartPoints[NUM_RESTART_POINTS];
extern float Restart_HospitalRestartHeadings[NUM_RESTART_POINTS];
extern RestartPoint Restart_PoliceRestartPoints[NUM_RESTART_POINTS];
extern float Restart_PoliceRestartHeadings[NUM_RESTART_POINTS];
void Restart_Initialise(void);
void Restart_AddHospitalRestartPoint(const RestartPosition *position,float heading);
void Restart_AddPoliceRestartPoint(const RestartPosition *position,float heading);
void Restart_OverrideNextRestart(const RestartPosition *position,float heading);
void Restart_CancelOverrideRestart(void);
void Restart_FindClosestHospitalRestartPoint(const RestartPosition *position,RestartPosition *outPosition,float *outHeading);
void Restart_FindClosestPoliceRestartPoint(const RestartPosition *position,RestartPosition *outPosition,float *outHeading);
void Restart_LoadAllRestartPoints(uint8_t *buffer,uint32_t size);
void Restart_SaveAllRestartPoints(uint8_t *buffer,uint32_t *size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
