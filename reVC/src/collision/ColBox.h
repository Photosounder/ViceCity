//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct CBox {
    CollisionVector3 min,max;
} CBox;
typedef struct CColBox {
    CBox bounds;
    uint8_t surface,piece;
} CColBox;
#ifdef __cplusplus
extern "C" {
#endif
void Box_Set(CBox *self,CollisionPointRef min,CollisionPointRef max);
CollisionVector3 Box_GetSize(const CBox *self);
void ColBox_SetBounds(CColBox *self,CollisionPointRef min,CollisionPointRef max);
void ColBox_Set(CColBox *self,CollisionPointRef min,CollisionPointRef max,uint8_t surface,uint8_t piece);
CColBox *ColBox_Assign(CColBox *self,const CColBox *other);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
