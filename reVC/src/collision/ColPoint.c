//+ rouz edit (ChatGPT)
#include "ColPoint.h"
#include <string.h>
#include <math.h>
static void ColPoint_CopyCoordinates(CollisionVector3 *self,const CollisionVector3 *other)
{
    // Capture one complete vector before assigning its destination
    CollisionVector3 value;memcpy(&value,other,sizeof(value));memcpy(self,&value,sizeof(value));
}
const CollisionVector3 *ColPoint_GetNormal(const CColPoint *self)
{
    // Borrow the actual normal so later contact updates remain visible through the query result
    return &self->normal;
}
float ColPoint_GetDepth(const CColPoint *self)
{
    // Return the original stored depth without interpretation
    return self->depth;
}
void ColPoint_SetSurfaces(CColPoint *self,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB)
{
    // Retain the original surface and piece write order without changing depth or geometry
    self->surfaceA=surfaceA;self->pieceA=pieceA;self->surfaceB=surfaceB;self->pieceB=pieceB;
}
void ColPoint_Set(CColPoint *self,float depth,uint8_t surfaceA,uint8_t pieceA,uint8_t surfaceB,uint8_t pieceB)
{
    // Write depth before the original four metadata fields
    self->depth=depth;ColPoint_SetSurfaces(self,surfaceA,pieceA,surfaceB,pieceB);
}
CColPoint *ColPoint_Assign(CColPoint *self,const CColPoint *other)
{
    // Copy point, normal and metadata while leaving both gap words and depth untouched
    ColPoint_CopyCoordinates(&self->point,&other->point);ColPoint_CopyCoordinates(&self->normal,&other->normal);
    self->surfaceA=other->surfaceA;self->pieceA=other->pieceA;self->surfaceB=other->surfaceB;self->pieceB=other->pieceB;
    return self;
}
CColPoint ColPoint_CopyValue(CColPoint value)
{
    // Preserve full copy construction including depth and both explicit gap words
    return value;
}
void ColPoint_NormaliseNormal(CColPoint *self)
{
    // Retain the original native squared-length and reciprocal-square-root operation grouping
    const float sq=self->normal.x*self->normal.x+self->normal.y*self->normal.y+self->normal.z*self->normal.z;
    if(sq>0.0f){
        // Scale all three components in their original order for a positive squared length
        const float inverse=1.0f/sqrtf(sq);self->normal.x*=inverse;self->normal.y*=inverse;self->normal.z*=inverse;
    }else{
        // Preserve the original fallback that changes only X even for a nonfinite squared length
        self->normal.x=1.0f;
    }
}
void ColPoint_ReverseNormal(CColPoint *self)
{
    // Retain the original ordered multiplication by negative one before suspension forces are applied
    self->normal.x*=-1.0f;self->normal.y*=-1.0f;self->normal.z*=-1.0f;
}
//- rouz edit (ChatGPT)
