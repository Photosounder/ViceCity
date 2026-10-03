#pragma once
//+ rouz edit (ChatGPT)
#include "common.h"
#include "AudioSound.h"
#include "AudioGeometry.h"

static inline AudioSoundPosition
AudioSoundPosition_FromVector(const CVector &position)
{
    // Copy the game vector coordinates into plain C position storage
    AudioSoundPosition result;
    result.x = position.x;
    result.y = position.y;
    result.z = position.z;
    return result;
}

static inline CVector
AudioSoundPosition_ToVector(const AudioSoundPosition *position)
{
    // Copy the plain C coordinates for the existing game geometry operations
    return CVector(position->x, position->y, position->z);
}

static inline float
AudioGeometry_DistanceVector(const CVector &position)
{
    // Copy the game coordinates before calling the C camera distance calculation
    const AudioSoundPosition plain = AudioSoundPosition_FromVector(position);
    return AudioGeometry_DistanceSquared(&plain);
}

static inline void
AudioGeometry_TranslateVector(const CVector *position, CVector *translated)
{
    // Preserve an input snapshot and complete the C transform before writing the game vector
    const AudioSoundPosition plain = AudioSoundPosition_FromVector(*position);
    AudioSoundPosition result;
    AudioGeometry_Translate(&plain, &result);
    *translated = AudioSoundPosition_ToVector(&result);
}
//- rouz edit (ChatGPT)
