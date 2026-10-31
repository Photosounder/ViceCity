//+ rouz edit (ChatGPT)
#include "AudioManager.h"

void
AudioPed_Process(cAudioManager *manager, CPed *ped)
{
    // Initialize the audio context explicitly before processing it
    cPedParams params;
    PedParams_Init(&params);

    // Copy the game position into the C sound record
    AudioReleaseHost_Position(ped, &manager->m_sQueueSample.m_vecPos);

    // Calculate the original squared distance before handing off pedestrian events
    params.m_bDistanceCalculated = 0;
    params.m_pPed = ped;
    params.m_fDistance = AudioGeometry_DistanceSquared(&manager->m_sQueueSample.m_vecPos);
    AudioPedOneShots_Process(manager, &params);
}
//- rouz edit (ChatGPT)
