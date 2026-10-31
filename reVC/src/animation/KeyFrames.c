//+ rouz edit (ChatGPT)
#include "KeyFrames.h"
void KeyFrameCompressed_GetRotation(const KeyFrameCompressed *self,CQuaternion *value)
{
    // Decode signed rotation components with the original reciprocal multiplication
    const float scale=1.0f/4096.0f;
    value->x=self->rot[0]*scale;value->y=self->rot[1]*scale;value->z=self->rot[2]*scale;value->w=self->rot[3]*scale;
}
void KeyFrameCompressed_SetRotation(KeyFrameCompressed *self,const CQuaternion *value)
{
    // Retain the original component multiplication and signed narrowing at each store
    self->rot[0]=value->x*4096.0f;self->rot[1]=value->y*4096.0f;self->rot[2]=value->z*4096.0f;self->rot[3]=value->w*4096.0f;
}
float KeyFrameCompressed_GetDeltaTime(const KeyFrameCompressed *self)
{
    // Keep division by sixty instead of changing its rounding to reciprocal multiplication
    return self->deltaTime/60.0f;
}
void KeyFrameCompressed_SetTime(KeyFrameCompressed *self,float value)
{
    // Retain the original positive half bias before signed truncation
    self->deltaTime=value*60.0f+0.5f;
}
void KeyFrameTransCompressed_GetTranslation(const KeyFrameTransCompressed *self,KeyFrameVectorOut value)
{
    // Decode signed translation coordinates into their original destination fields in order
    const float scale=1.0f/1024.0f;
    *value.x=self->trans[0]*scale;*value.y=self->trans[1]*scale;*value.z=self->trans[2]*scale;
}
void KeyFrameTransCompressed_SetTranslation(KeyFrameTransCompressed *self,KeyFrameVectorRef value)
{
    // Keep live coordinate references and the original ordered signed narrowing stores
    self->trans[0]=*value.x*1024.0f;self->trans[1]=*value.y*1024.0f;self->trans[2]=*value.z*1024.0f;
}
//- rouz edit (ChatGPT)
