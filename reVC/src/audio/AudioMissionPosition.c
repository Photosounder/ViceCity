//+ rouz edit (ChatGPT)
#include "AudioMissionPosition.h"

void
AudioMissionPosition_Set(AudioSoundPosition *positions, uint8_t *is2D, uint8_t initialized, uint8_t slot, float x, float y, float z)
{
    // Preserve the original initialization and slot checks before updating mission coordinates
    if (initialized && slot < MISSION_AUDIO_SLOTS) {
        // Store the three coordinates and select positional mission playback
        is2D[slot] = 0;
        positions[slot].x = x;
        positions[slot].y = y;
        positions[slot].z = z;
    }
}
//- rouz edit (ChatGPT)
