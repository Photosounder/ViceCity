//+ rouz edit (ChatGPT)
#include "ModelRegistry.h"
#include "ModelRegistry_backend.h"
#include "config.h"

static void *modelInfoPtrs[MODELINFOSIZE];

void *ModelRegistry_Get(int id)
{
    // Return the original unchecked model-table entry
    return modelInfoPtrs[id];
}

void ModelRegistry_Set(int id,void *model)
{
    // Publish the supplied base-model pointer in the original table slot
    modelInfoPtrs[id]=model;
}
//- rouz edit (ChatGPT)
