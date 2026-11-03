//+ rouz edit (ChatGPT)
#include "PedPlacement_backend.h"
#include "PedConstants.h"

bool PedPlacement_CFindZ(float x,float y,float *z)
{
    // Sample the original two vertical lines and retain their highest hit
    float endZ=*z-100.0f;
    float lineZ=*z+1.0f;
    float first=-100.0f,second=-100.0f;
    float hit;
    if (PedPlacement_BackendVerticalLine(x,y,lineZ,endZ,&hit)) first=hit;
    x+=0.1f;
    y+=0.1f;
    if (PedPlacement_BackendVerticalLine(x,y,lineZ,endZ,&hit)) second=hit;
    float highest=first>second ? first : second;
    // Update the position only when a valid ground height was found
    if (highest>-99.0f) {
        *z=FEET_OFFSET+highest;
        return true;
    }
    return false;
}

bool PedPlacement_CIsClear(float x,float y,float z,float radius,int total,void *entities)
{
    // Resolve the original sentinel defaults before checking for collisions
    if (radius==-1.0f) radius=0.75f;
    if (total==-1) total=2;
    return PedPlacement_BackendFindObjects(x,y,z,radius,total,entities)==0;
}
void *PedPlacement_CFindCar(float x,float y,float z)
{
    // Check the original quarter-unit sphere for nearby cars
    return PedPlacement_BackendSphere(x,y,z,0.25f);
}
//- rouz edit (ChatGPT)
