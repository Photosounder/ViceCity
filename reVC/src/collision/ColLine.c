//+ rouz edit (ChatGPT)
#include "ColLine.h"
#include <string.h>
static void ColLine_Write(CColLine *self,CollisionPointRef p0,CollisionPointRef p1)
{
    // Capture the complete first vector before writing its destination
    CollisionVector3 first;
    memcpy(&first.x,p0.x,sizeof(float));memcpy(&first.y,p0.y,sizeof(float));memcpy(&first.z,p0.z,sizeof(float));
    memcpy(&self->p0,&first,sizeof(first));
    // Read the second vector only after the first destination has been updated
    CollisionVector3 second;
    memcpy(&second.x,p1.x,sizeof(float));memcpy(&second.y,p1.y,sizeof(float));memcpy(&second.z,p1.z,sizeof(float));
    memcpy(&self->p1,&second,sizeof(second));
}
void ColLine_Init(CColLine *self,CollisionPointRef p0,CollisionPointRef p1)
{
    // Preserve parameterized construction without writing either padding word
    ColLine_Write(self,p0,p1);
}
void ColLine_Set(CColLine *self,CollisionPointRef p0,CollisionPointRef p1)
{
    // Preserve the original ordered member assignment and valid endpoint aliases
    ColLine_Write(self,p0,p1);
}
CColLine ColLine_Make(CollisionPointRef p0,CollisionPointRef p1)
{
    // Construct a temporary line while retaining the original unspecified padding-word values
    CColLine value;ColLine_Write(&value,p0,p1);return value;
}
CColLine ColLine_FromRaw32(const void *bytes)
{
    // Transfer both complete quadwords into an actual C record instead of creating a typed view
    CColLine value;memcpy(&value,bytes,sizeof(value));return value;
}
//- rouz edit (ChatGPT)
