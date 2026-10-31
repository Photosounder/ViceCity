//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "User.h"
#include "Zones.h"
#ifdef __cplusplus
extern "C" {
#endif
ZonesPosition UserDisplay_BackendPlayerPosition(void);
void UserDisplay_BackendSetZoneName(uint16_t *name);
uint16_t **UserDisplay_BackendLastZoneName(void);
uint32_t *UserDisplay_BackendZoneState(void);
bool UserDisplay_BackendInVehicle(void);
CVehicle *UserDisplay_BackendVehicle(void);
CVehicle *UserDisplay_BackendShortCutTaxi(void);
int16_t UserDisplay_BackendModelIndex(const CVehicle *vehicle);
char *UserDisplay_BackendGameName(int16_t model);
uint16_t *UserDisplay_BackendText(const char *name);
void UserDisplay_BackendSetVehicleName(uint16_t *name);
bool UserDisplay_BackendPlayingBack(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
