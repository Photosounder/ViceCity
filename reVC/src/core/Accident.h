//+ rouz edit (ChatGPT)
#pragma once
#include "EventsShared.h"
typedef struct CAccident {CPed *m_pVictim;uint32_t m_nMedicsAttending;uint32_t m_nMedicsPerformingCPR;} CAccident;
typedef struct CAccidentManager {CAccident m_aAccidents[NUM_ACCIDENTS];} CAccidentManager;
enum {ACCIDENT_MAX_MEDICS_TO_ATTEND=2};
#ifdef __cplusplus
extern "C" {
#endif
extern CAccidentManager gAccidentManager;
void Accident_Init(CAccident *self);
void AccidentManager_Init(CAccidentManager *self);
CAccident *AccidentManager_GetNextFreeAccident(CAccidentManager *self);
void AccidentManager_ReportAccident(CAccidentManager *self,CPed *ped);
void AccidentManager_Update(CAccidentManager *self);
CAccident *AccidentManager_FindNearestAccident(CAccidentManager *self,EventsVector position,float *distance);
uint16_t AccidentManager_CountActiveAccidents(CAccidentManager *self);
bool AccidentManager_WorkToDoForMedics(CAccidentManager *self);
bool AccidentManager_UnattendedAccidents(CAccidentManager *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
