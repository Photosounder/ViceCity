//+ rouz edit (ChatGPT)
#pragma once
#include "ModelIndexData.h"
#ifdef __cplusplus
class CSimpleModelInfo;
typedef CSimpleModelInfo ModelIndexModel;
extern "C" {
#else
typedef struct ModelIndexModel ModelIndexModel;
#endif
ModelIndexModel *ModelIndices_BackendModel(int16_t id);
bool ModelIndices_BackendBuilding(ModelIndexModel *model);
bool ModelIndices_BackendCodeGlass(const ModelIndexModel *model);
bool ModelIndices_BackendArtistGlass(const ModelIndexModel *model);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
