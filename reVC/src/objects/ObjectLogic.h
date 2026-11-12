//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include "ObjectLayout.h"
#include "Pool.h"

#ifdef __cplusplus
extern "C" {
#endif
// Field initialisation done by the default object constructor
void Object_InitFields(CObjectStruct *self);
// Whether the object may be deleted by the cleanup code
bool Object_CanBeDeleted(const CObjectState *self);
// Objects of the pool created in a given way, in pool order
int Object_SelectByCreation(const CPool *objectPool, int creationMode, void **out, int maxOut);
// Objects the cleanup code may delete, walking the pool backwards
int Object_SelectDeletable(const CPool *objectPool, void **out, int maxOut);
// Temporary objects within a radius of a point, in pool order
int Object_SelectTempInArea(const CPool *objectPool, float x, float y, float z, float radius, void **out, int maxOut);

#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
