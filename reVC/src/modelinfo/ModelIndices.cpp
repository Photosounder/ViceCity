//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelInfo.h"
#include "ModelIndices_backend.h"
ModelIndexModel *ModelIndices_BackendModel(int16_t id)
{
    // Capture the same actual model entry with the original typed native downcast
    return (CSimpleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(id)); // rouz edit (ChatGPT)
}
bool ModelIndices_BackendBuilding(ModelIndexModel *model)
{
    // Read the original native building predicate without copying model state
    return model->IsBuilding();
}
bool ModelIndices_BackendCodeGlass(const ModelIndexModel *model)
{
    // Read the same captured model's original code-glass flag
    return model->m_isCodeGlass;
}
bool ModelIndices_BackendArtistGlass(const ModelIndexModel *model)
{
    // Read the same captured model's original artist-glass flag
    return model->m_isArtistGlass;
}
//- rouz edit (ChatGPT)
