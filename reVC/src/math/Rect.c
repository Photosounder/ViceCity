//+ rouz edit (ChatGPT)
#include "Rect.h"
void Rect_Init(CRect *self)
{
    // Preserve the empty-bound constructor and its field-write order
    self->left=1000000.0f;
    self->top=1000000.0f;
    self->right=-1000000.0f;
    self->bottom=-1000000.0f;
}
CRect Rect_Make(float left,float top,float right,float bottom)
{
    // Construct the original left, top, right, bottom argument order
    CRect value;
    value.left=left;value.top=top;value.right=right;value.bottom=bottom;
    return value;
}
CRect Rect_CopyValue(CRect value)
{
    // Preserve explicit copy-construction sites as plain C value copies
    return value;
}
void Rect_ContainPoint(CRect *self,const float *x,const float *y)
{
    // Preserve each comparison and its live coordinate read before the next field update
    if(*x<self->left) self->left=*x;
    if(*x>self->right) self->right=*x;
    if(*y<self->top) self->top=*y;
    if(*y>self->bottom) self->bottom=*y;
}
void Rect_ContainRect(CRect *self,const CRect *other)
{
    // Retain ordered source reads even when the source and destination are the same record
    if(other->left<self->left) self->left=other->left;
    if(other->right>self->right) self->right=other->right;
    if(other->top<self->top) self->top=other->top;
    if(other->bottom>self->bottom) self->bottom=other->bottom;
}
bool Rect_IsPointInside(const CRect *self,const float *x,const float *y)
{
    // Preserve inclusive edges and short-circuit coordinate reads
    return *x>=self->left&&*x<=self->right&&*y>=self->top&&*y<=self->bottom;
}
bool Rect_IsPointInsideRadius(const CRect *self,const float *x,const float *y,float extraRadius)
{
    // Preserve the original radius arithmetic at each inclusive edge comparison
    return *x>=self->left-extraRadius&&*x<=self->right+extraRadius&&*y>=self->top-extraRadius&&*y<=self->bottom+extraRadius;
}
void Rect_Translate(CRect *self,float x,float y)
{
    // Keep the original horizontal then vertical update order
    self->left+=x;self->right+=x;self->bottom+=y;self->top+=y;
}
void Rect_Grow(CRect *self,float radius)
{
    // Apply one radius in the original field order
    self->left-=radius;self->right+=radius;self->top-=radius;self->bottom+=radius;
}
void Rect_GrowSides(CRect *self,float left,float right)
{
    // Apply the original left/top and right/bottom radius pairing
    self->left-=left;self->top-=left;self->right+=right;self->bottom+=right;
}
void Rect_GrowEdges(CRect *self,float left,float right,float top,float bottom)
{
    // Keep all four independently supplied edge radii
    self->left-=left;self->top-=top;self->right+=right;self->bottom+=bottom;
}
float Rect_GetWidth(const CRect *self)
{
    // Return the original signed horizontal extent
    return self->right-self->left;
}
float Rect_GetHeight(const CRect *self)
{
    // Return the original signed vertical extent
    return self->bottom-self->top;
}
//- rouz edit (ChatGPT)
