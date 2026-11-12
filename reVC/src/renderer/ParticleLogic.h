//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "VectorMath.h"

#ifdef __cplusplus
extern "C" {
#endif
// Movement of one water drop on the screen for a frame, step is the movement and count the running counter shared by all drops, the result tells that the drop lives 1250 ms longer
bool Particle_WaterDropStep(uint8_t *count, VectorMath3 *step, float cameraAverageSpeed, float posZ);
// Index in the sine and cosine tables of an angle in degrees, the table covers a full turn
int32_t Particle_SinCosIndex(int16_t rotationDegrees, int32_t tableSize);
// Fading of a ship at the horizon: it appears slowly up to an alpha of 96 and starts fading away 10 seconds before it is destroyed
void Particle_ShipFade(uint32_t now, uint32_t timeWhenWillBeDestroyed, uint8_t *alpha, int16_t *fadeAlphaTimer);
// Colour of a particle fading from the colour of its system to the destination colour, the channels are red, green, blue and alpha, nothing happens when no change is due
void Particle_UpdateColor(uint32_t now, uint32_t timeWhenColorWillBeChanged, uint32_t colorFadeTime, const uint8_t render[4], const uint8_t dest[4], uint8_t color[4]);
// Size of a particle for the next frame, the result is false when it shrank away, bigSplash is a car or boat splash that gets smaller instead of bigger and then slows its fall (dropVelZ)
bool Particle_NextSize(float size, float expansionRate, float windMag, float moveMag, bool fumes, bool splash, bool waterDrop, float moveX, float moveZ, float *newSize, bool *dropVelZ);
// Intensity of a particle fading to black, the amount is taken off each frame down to zero
uint8_t Particle_FadeToBlack(uint8_t intensity, int16_t amount);
// Alpha of a particle fading away, the result is false once it is fully transparent
bool Particle_FadeAlpha(uint8_t *alpha, int16_t amount);
// Timer driven turning of the z rotation angle: every changeTime frames the angle changes by amount
void Particle_StepZRotation(uint16_t *timer, uint16_t changeTime, int16_t *angle, int16_t amount);
// Timer driven change of the z radius: every changeTime frames the radius changes by amount
void Particle_StepZRadius(uint16_t *timer, uint16_t changeTime, float *radius, float amount);
// Frame animation of a particle: the frame advances every speed frames, it cycles back to the start frame or stays on the last frame
void Particle_StepAnimation(int16_t *timer, uint16_t speed, uint8_t *frame, uint8_t startFrame, uint8_t finalFrame, bool cycle);
// Screen z of a point drawn on top of the 2D screen from its world z, the clip planes and the screen z range of the sprite buffer
float Particle_ScreenZ(float posZ, float nearClip, float farClip, float nearScreenZ, float farScreenZ);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
