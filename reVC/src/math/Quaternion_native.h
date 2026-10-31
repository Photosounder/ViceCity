//+ rouz edit (ChatGPT)
#pragma once
#include "Quaternion.h"
static inline QuaternionVectorRef Quaternion_VectorRef(const RwV3d &value)
{
    // Borrow the original native coordinates for ordered C reads
    const QuaternionVectorRef result={&value.x,&value.y,&value.z};return result;
}
static inline QuaternionVectorOut Quaternion_VectorOut(RwV3d &value)
{
    // Preserve the actual destination coordinates and their alias relationships
    const QuaternionVectorOut result={&value.x,&value.y,&value.z};return result;
}
static inline QuaternionMatrixRef Quaternion_MatrixRef(const RwMatrix &value)
{
    // Borrow only the original matrix axes while leaving flags and position outside the interface
    const QuaternionMatrixRef result={Quaternion_VectorRef(value.right),Quaternion_VectorRef(value.up),Quaternion_VectorRef(value.at)};return result;
}
static inline QuaternionMatrixOut Quaternion_MatrixOut(RwMatrix &value)
{
    // Preserve the original nine matrix writes through actual native field addresses
    const QuaternionMatrixOut result={Quaternion_VectorOut(value.right),Quaternion_VectorOut(value.up),Quaternion_VectorOut(value.at)};return result;
}
//- rouz edit (ChatGPT)
