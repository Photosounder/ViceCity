//+ rouz edit (ChatGPT)
#include "common.h"
#include "Vehicle.h"
#include "Door.h"
#include "Door_backend.h"

DoorVector Door_BackendSpeed(CVehicle *vehicle,const DoorVector *offset)
{
    // Read the original physical-point velocity through the existing vehicle operation
    CVector speed=vehicle->GetSpeed(CVector(offset->x,offset->y,offset->z));
    DoorVector result={speed.x,speed.y,speed.z};
    return result;
}

DoorVector Door_BackendToLocal(CVehicle *vehicle,const DoorVector *difference)
{
    // Retain the original vector-first matrix multiply and its coordinate convention
    CVector local=Multiply3x3(CVector(difference->x,difference->y,difference->z),vehicle->GetMatrix());
    DoorVector result={local.x,local.y,local.z};
    return result;
}
//- rouz edit (ChatGPT)
