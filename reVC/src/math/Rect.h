//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
typedef struct CRect {
    float left;
    float bottom;
    float right;
    float top;
} CRect;
#ifdef __cplusplus
extern "C" {
#endif
void Rect_Init(CRect *self);
CRect Rect_Make(float left,float top,float right,float bottom);
CRect Rect_CopyValue(CRect value);
void Rect_ContainPoint(CRect *self,const float *x,const float *y);
void Rect_ContainRect(CRect *self,const CRect *other);
bool Rect_IsPointInside(const CRect *self,const float *x,const float *y);
bool Rect_IsPointInsideRadius(const CRect *self,const float *x,const float *y,float extraRadius);
void Rect_Translate(CRect *self,float x,float y);
void Rect_Grow(CRect *self,float radius);
void Rect_GrowSides(CRect *self,float left,float right);
void Rect_GrowEdges(CRect *self,float left,float right,float top,float bottom);
float Rect_GetWidth(const CRect *self);
float Rect_GetHeight(const CRect *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
