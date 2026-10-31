//+ rouz edit (ChatGPT)
#pragma once
#include "ColSphere.h"
#include "CollisionVector_native.h"
#include "VuVector.h"
static inline CVuVector Sphere_ToVu(const CSphere &sphere)
{
    // Transfer the complete original quadword without an unrelated struct view
    CVuVector value;memcpy(&value,&sphere,sizeof(sphere));return value;
}
//- rouz edit (ChatGPT)
