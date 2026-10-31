//+ rouz edit (ChatGPT)
#include "ColSphere.h"
#include "General.h"
#include <string.h>
static void Sphere_WriteCenter(CollisionVector3 *self,CollisionPointRef center)
{
    // Capture the complete source before assigning its destination coordinates
    CollisionVector3 value;
    memcpy(&value.x,center.x,sizeof(float));memcpy(&value.y,center.y,sizeof(float));memcpy(&value.z,center.z,sizeof(float));
    memcpy(self,&value,sizeof(value));
}
void Sphere_Set(CSphere *self,float radius,CollisionPointRef center)
{
    // Keep the base setter's original center-before-radius ordering
    Sphere_WriteCenter(&self->center,center);self->radius=radius;
}
void ColSphere_SetBounds(CColSphere *self,float radius,CollisionPointRef center)
{
    // Preserve the inherited base setter without touching collision metadata or padding
    Sphere_Set(&self->bounds,radius,center);
}
void ColSphere_Set(CColSphere *self,float radius,CollisionPointRef center,uint8_t surface,uint8_t piece)
{
    // Keep the collision setter's original radius-before-center and metadata ordering
    self->bounds.radius=radius;Sphere_WriteCenter(&self->bounds.center,center);
    self->surface=surface;self->piece=piece;
}
static void Sphere_WritePoint(CollisionPointOut destination,CollisionVector3 value)
{
    // Assign one fully evaluated point before the next ray expression is read
    memcpy(destination.x,&value.x,sizeof(float));memcpy(destination.y,&value.y,sizeof(float));memcpy(destination.z,&value.z,sizeof(float));
}
bool ColSphere_IntersectRay(CColSphere *self,CollisionPointRef from,CollisionPointRef dir,CollisionPointOut entry,CollisionPointOut exit)
{
    // Retain the original vector subtraction, squared-distance grouping and unit quadratic coefficient
    const CollisionVector3 delta={*from.x-self->bounds.center.x,*from.y-self->bounds.center.y,*from.z-self->bounds.center.z};
    const float touch=delta.x*delta.x+delta.y*delta.y+delta.z*delta.z-self->bounds.radius*self->bounds.radius;
    float root1,root2;
    if(!General_SolveQuadratic(1.0f,(delta.x**dir.x+delta.y**dir.y+delta.z**dir.z)*2.f,touch,&root1,&root2))return false;
    // Evaluate the complete entry point before writing any of its coordinates
    const CollisionVector3 first={*from.x+*dir.x*root1,*from.y+*dir.y*root1,*from.z+*dir.z*root1};
    Sphere_WritePoint(entry,first);
    // Reread live inputs after entry writes so valid input/output aliases retain their original behavior
    const CollisionVector3 second={*from.x+*dir.x*root2,*from.y+*dir.y*root2,*from.z+*dir.z*root2};
    Sphere_WritePoint(exit,second);return true;
}
CSphere Sphere_FromRaw16(const void *bytes)
{
    // Transfer only the actual center/radius quadword into a complete C geometry record
    CSphere value;memcpy(&value,bytes,sizeof(value));return value;
}
CColSphere ColSphere_FromRaw16(const void *bytes)
{
    // Supply the scratch geometry without reading nonexistent metadata beyond its original quadword
    CColSphere value;memset(&value,0,sizeof(value));memcpy(&value.bounds,bytes,sizeof(value.bounds));return value;
}
//- rouz edit (ChatGPT)
