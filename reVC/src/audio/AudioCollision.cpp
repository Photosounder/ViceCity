//+ rouz edit (ChatGPT)
#include "common.h"

#include "DMAudio.h"
#include "Entity.h"
#include "AudioCollision.h"
#include "AudioManager.h"
#include "AudioSoundGame.h"
#include "AudioSamples.h"
#include "SurfaceTable.h"
#include "sampman.h"






//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
uint8_t
AudioCollisionReportHost_IsBuilding(CEntity *entity)
{
    // Query each original building test at its short-circuit point
    return entity->IsBuilding();
}

void
AudioCollisionReportHost_Position(CEntity *entity, AudioSoundPosition *position)
{
    // Copy the current game position into plain C coordinates
    *position = AudioSoundPosition_FromVector(entity->GetPosition());
}
//- rouz edit (ChatGPT)
