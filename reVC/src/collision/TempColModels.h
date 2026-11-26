#pragma once

#include "ColModel.h"

//+ rouz edit (Claude)
// Formerly the static class CTempColModels, now C globals and functions shared with C
#ifdef __cplusplus
extern "C" {
#endif
void TempColModels_Initialise(void);
#ifdef __cplusplus
}
// Members that C cannot declare yet (references, default arguments, overloads, C++ class values), with C++ linkage
extern CColModel TempColModels_ms_colModelPed1;
extern CColModel TempColModels_ms_colModelPed2;
extern CColModel TempColModels_ms_colModelBBox;
extern CColModel TempColModels_ms_colModelBumper1;
extern CColModel TempColModels_ms_colModelWheel1;
extern CColModel TempColModels_ms_colModelPanel1;
extern CColModel TempColModels_ms_colModelBodyPart2;
extern CColModel TempColModels_ms_colModelBodyPart1;
extern CColModel TempColModels_ms_colModelCutObj[5];
extern CColModel TempColModels_ms_colModelPedGroundHit;
extern CColModel TempColModels_ms_colModelBoot1;
extern CColModel TempColModels_ms_colModelDoor1;
extern CColModel TempColModels_ms_colModelBonnet1;
extern CColModel TempColModels_ms_colModelWeapon;
#endif
//- rouz edit (Claude)
