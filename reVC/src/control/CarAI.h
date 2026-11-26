#pragma once

#include "AutoPilot.h"

class CVehicle;

//+ rouz edit (Claude)
// Formerly the static class CCarAI, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
float CarAI_FindSwitchDistanceClose(CVehicle*);
float CarAI_FindSwitchDistanceFarNormalVehicle(CVehicle*);
float CarAI_FindSwitchDistanceFar(CVehicle*);
void CarAI_BackToCruisingIfNoWantedLevel(CVehicle*);
void CarAI_UpdateCarAI(CVehicle*);
void CarAI_CarHasReasonToStop(CVehicle*);
float CarAI_GetCarToGoToCoors(CVehicle*, CVector*);
float CarAI_GetCarToParkAtCoors(CVehicle*, CVector*);
void CarAI_AddPoliceCarOccupants(CVehicle*);
void CarAI_AddAmbulanceOccupants(CVehicle*);
void CarAI_AddFiretruckOccupants(CVehicle*);
void CarAI_TellOccupantsToLeaveCar(CVehicle*);
void CarAI_TellOccupantsToFleeCar(CVehicle*);
void CarAI_TellCarToRamOtherCar(CVehicle*, CVehicle*);
void CarAI_TellCarToBlockOtherCar(CVehicle*, CVehicle*);
uint8 CarAI_FindPoliceCarMissionForWantedLevel();
uint8 CarAI_FindPoliceBoatMissionForWantedLevel();
int32 CarAI_FindPoliceCarSpeedForWantedLevel(CVehicle*);
void CarAI_MellowOutChaseSpeed(CVehicle*);
void CarAI_MellowOutChaseSpeedBoat(CVehicle*);
void CarAI_MakeWayForCarWithSiren(CVehicle *veh);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
#endif
//- rouz edit (Claude)
