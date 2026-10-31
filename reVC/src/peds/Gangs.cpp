//+ rouz edit (ChatGPT)
#include "common.h"
#include "ModelIndices.h"
#include "Streaming.h"
#include "Gangs_backend.h"

bool Gangs_BackendHasModelLoaded(int32_t model)
{
    // Query the original streaming table at each conditional lookup in the C controller
    return CStreaming::HasModelLoaded(model);
}
//- rouz edit (ChatGPT)
