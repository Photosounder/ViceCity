//+ rouz edit (ChatGPT)
#include "2dEffect.h"
#include "ModelRegistry.h"
#include "Store.h"
#include "config.h"

static C2dEffect effectEntries[TWODFXSIZE];
static CStore effectStore={0,effectEntries,sizeof(C2dEffect),TWODFXSIZE};

CStore *ModelRegistry_EffectsStore(void)
{
    // Return the statically allocated original effect store
    return &effectStore;
}
//- rouz edit (ChatGPT)
