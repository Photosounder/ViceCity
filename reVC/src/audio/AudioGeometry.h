//+ rouz edit (ChatGPT)
#pragma once
#include "AudioSound.h"
typedef struct AudioCameraMatrix {
    float rx, ry, rz;
    float fx, fy, fz;
    float ux, uy, uz;
    float px, py, pz;
} AudioCameraMatrix;
#ifdef __cplusplus
extern "C" {
#endif
float AudioGeometry_DistanceSquared(const AudioSoundPosition *position);
void AudioGeometry_Translate(const AudioSoundPosition *position, AudioSoundPosition *translated);
void AudioGeometryHost_CameraPosition(AudioSoundPosition *position);
void AudioGeometryHost_CameraMatrix(AudioCameraMatrix *matrix);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
