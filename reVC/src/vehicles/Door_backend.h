//+ rouz edit (ChatGPT)
#pragma once
#include "Door.h"
#ifdef __cplusplus
extern "C" {
#endif
DoorVector Door_BackendSpeed(CVehicle *vehicle,const DoorVector *offset);
DoorVector Door_BackendToLocal(CVehicle *vehicle,const DoorVector *difference);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
