//+ rouz edit (ChatGPT)
#include "AudioGeometry.h"

static float AudioGeometry_Square(float value)
{
    // Retain the original float square operation before adding each axis
    return value * value;
}

float AudioGeometry_DistanceSquared(const AudioSoundPosition *position)
{
    // Read the camera position and retain the original vertical distance weighting
    AudioSoundPosition camera;
    AudioGeometryHost_CameraPosition(&camera);
    return AudioGeometry_Square(position->x - camera.x) + AudioGeometry_Square(position->y - camera.y)
        + AudioGeometry_Square((position->z - camera.z) * 0.2f);
}

void AudioGeometry_Translate(const AudioSoundPosition *position, AudioSoundPosition *translated)
{
    // Subtract camera translation before applying the original three basis dot products
    AudioCameraMatrix matrix;
    AudioSoundPosition relative, result;
    AudioGeometryHost_CameraMatrix(&matrix);
    relative.x = position->x - matrix.px;
    relative.y = position->y - matrix.py;
    relative.z = position->z - matrix.pz;
    result.x = matrix.rx * relative.x + matrix.ry * relative.y + matrix.rz * relative.z;
    result.y = matrix.fx * relative.x + matrix.fy * relative.y + matrix.fz * relative.z;
    result.z = matrix.ux * relative.x + matrix.uy * relative.y + matrix.uz * relative.z;
    *translated = result;
}
//- rouz edit (ChatGPT)
