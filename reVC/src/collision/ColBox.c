//+ rouz edit (ChatGPT)
#include "ColBox.h"
#include <string.h>
void Box_Set(CBox *self,CollisionPointRef min,CollisionPointRef max)
{
    // Capture and assign the complete minimum before reading the maximum
    CollisionVector3 value;
    memcpy(&value.x,min.x,sizeof(float));memcpy(&value.y,min.y,sizeof(float));memcpy(&value.z,min.z,sizeof(float));
    memcpy(&self->min,&value,sizeof(value));
    // Retain the second reference's live values after the first destination write
    memcpy(&value.x,max.x,sizeof(float));memcpy(&value.y,max.y,sizeof(float));memcpy(&value.z,max.z,sizeof(float));
    memcpy(&self->max,&value,sizeof(value));
}
CollisionVector3 Box_GetSize(const CBox *self)
{
    // Return the same signed component differences without normalizing the bounds
    const CollisionVector3 value={self->max.x-self->min.x,self->max.y-self->min.y,self->max.z-self->min.z};
    return value;
}
void ColBox_SetBounds(CColBox *self,CollisionPointRef min,CollisionPointRef max)
{
    // Preserve the inherited two-argument operation without changing collision metadata
    Box_Set(&self->bounds,min,max);
}
void ColBox_Set(CColBox *self,CollisionPointRef min,CollisionPointRef max,uint8_t surface,uint8_t piece)
{
    // Keep the original minimum, maximum, surface and piece write order
    Box_Set(&self->bounds,min,max);self->surface=surface;self->piece=piece;
}
CColBox *ColBox_Assign(CColBox *self,const CColBox *other)
{
    // Copy only the original fields and retain both destination tail-padding bytes
    Box_Set(&self->bounds,COLLISION_POINT_REF(other->bounds.min),COLLISION_POINT_REF(other->bounds.max));
    self->surface=other->surface;self->piece=other->piece;return self;
}
//- rouz edit (ChatGPT)
