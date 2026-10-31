//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct CloudsVector { float x,y,z; } CloudsVector;
typedef struct CloudsColor { uint8_t r,g,b,a; } CloudsColor;
#ifdef __cplusplus
extern "C" {
#endif
extern float Clouds_CloudRotation;
extern uint32_t Clouds_IndividualRotation;
extern float Clouds_ms_cameraRoll;
extern float Clouds_ms_horizonZ;
extern float Clouds_ms_HorizonTilt;
void Clouds_Init(void);
void Clouds_Shutdown(void);
void Clouds_Update(void);
void Clouds_Render(void);
#ifdef REVC_SOFTWARE_POLYGONS
void Clouds_RenderForEnvMap(void *camera);
#endif
void Clouds_RenderBackground(int16_t topred,int16_t topgreen,int16_t topblue,int16_t botred,int16_t botgreen,int16_t botblue,int16_t alpha);
void Clouds_RenderHorizon(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
