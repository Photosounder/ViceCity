//+ rouz edit (ChatGPT)
#include "common.h"
#include "Ped.h"
#include "PedPlacement.h"
#include "PedPlacement_backend.h"
#include "World.h"

bool PedPlacement_BackendVerticalLine(float x,float y,float z,float endZ,float *hitZ)
{
    // Query the native world with the original collision flags
    CColPoint foundCol;
    CEntity *foundEnt;
    CVector point(x,y,z);
    if (!CWorld::ProcessVerticalLine(point,endZ,foundCol,foundEnt,true,false,false,false,true,false,nil)) return false;
    *hitZ=foundCol.point.z;
    return true;
}

int16_t PedPlacement_BackendFindObjects(float x,float y,float z,float radius,int total,void *entities)
{
    // Query native nearby entities with the original collision flags
    int16 count;
    CVector point(x,y,z);
    CWorld::FindObjectsKindaColliding(point,radius,true,&count,total,(CEntity**)entities,false,true,true,false,false);
    return count;
}

void *PedPlacement_BackendSphere(float x,float y,float z,float radius)
{
    // Preserve the native sphere query and its original flags
    CVector point(x,y,z);
    return CWorld::TestSphereAgainstWorld(point,radius,nil,true,true,false,false,false,false);
}

//- rouz edit (ChatGPT)
