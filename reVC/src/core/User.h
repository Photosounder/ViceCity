//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "Pager.h"
#include "OnscreenTimer.h"

typedef struct CZone CZone;
#ifdef __cplusplus
class CVehicle;
#else
typedef struct CVehicle CVehicle;
#endif
typedef struct CPlaceName {
    CZone *m_pZone;
    CZone *m_pZone2;
    int16_t m_nAdditionalTimer;
} CPlaceName;
typedef struct CCurrentVehicle {
    CVehicle *m_pCurrentVehicle;
} CCurrentVehicle;
#ifdef __cplusplus
extern "C" {
#endif
extern CPlaceName PlaceName_state;
extern CCurrentVehicle CurrentVehicle_state;
void PlaceName_Init(CPlaceName *self);
void PlaceName_Process(CPlaceName *self);
void PlaceName_Display(CPlaceName *self);
void PlaceName_ProcessAfterFrontEndShutDown(CPlaceName *self);
void CurrentVehicle_Init(CCurrentVehicle *self);
void CurrentVehicle_Process(CCurrentVehicle *self);
void CurrentVehicle_Display(CCurrentVehicle *self);
void UserDisplay_Init(void);
void UserDisplay_Process(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
