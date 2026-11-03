#include <assert.h>
#include <math.h>
#include "PedPlacement_backend.h"

static int calls;
static bool hits[2];
static float heights[2];
static float expectedZ,expectedEnd;
static int16_t objectCount;
static float actualRadius;
static int actualTotal;
static void *actualEntities;

bool PedPlacement_BackendVerticalLine(float x,float y,float z,float endZ,float *hitZ)
{
    // Verify both original sample coordinates and record the requested hit
    int i=calls++;
    assert(i<2 && fabsf(x-(10.0f+0.1f*i))<0.0001f && fabsf(y-(20.0f+0.1f*i))<0.0001f);
    assert(z==expectedZ && endZ==expectedEnd);
    *hitZ=heights[i];
    return hits[i];
}
void *PedPlacement_BackendSphere(float x,float y,float z,float radius)
{
    // Verify the original sphere coordinates and radius
    assert(x==1.0f && y==2.0f && z==3.0f && radius==0.25f);
    return &objectCount;
}
int16_t PedPlacement_BackendFindObjects(float x,float y,float z,float radius,int total,void *entities)
{
    // Capture the collision query arguments for the clearance checks
    assert(x==1.0f && y==2.0f && z==3.0f);
    actualRadius=radius;actualTotal=total;actualEntities=entities;
    return objectCount;
}
int main(void)
{
    // Check highest-hit selection, missing hits, and the original threshold
    float z=50.0f;expectedZ=51.0f;expectedEnd=-50.0f;
    hits[0]=hits[1]=true;heights[0]=4.0f;heights[1]=7.0f;
    assert(PedPlacement_CFindZ(10.0f,20.0f,&z) && fabsf(z-8.04f)<0.0001f && calls==2);
    z=50.0f;calls=0;hits[0]=hits[1]=false;
    assert(!PedPlacement_CFindZ(10.0f,20.0f,&z) && z==50.0f && calls==2);
    calls=0;hits[0]=true;heights[0]=-99.0f;
    assert(!PedPlacement_CFindZ(10.0f,20.0f,&z) && z==50.0f);
    calls=0;hits[0]=false;hits[1]=true;heights[1]=0.0f;
    assert(PedPlacement_CFindZ(10.0f,20.0f,&z) && z==1.04f);
    // Check the car query returns the world result unchanged
    assert(PedPlacement_CFindCar(1,2,3)==&objectCount);
    // Check default and explicit clearance arguments and collision results
    objectCount=0;
    assert(PedPlacement_CIsClear(1,2,3,-1.0f,-1,0));
    assert(actualRadius==0.75f && actualTotal==2 && actualEntities==0);
    objectCount=1;
    assert(!PedPlacement_CIsClear(1,2,3,3.0f,5,&objectCount));
    assert(actualRadius==3.0f && actualTotal==5 && actualEntities==&objectCount);
    return 0;
}
