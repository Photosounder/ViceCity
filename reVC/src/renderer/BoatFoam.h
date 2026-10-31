//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void BoatFoam_CreateFoamAroundObject(void *matrix,float innerFw,float innerRg,float innerUp,int32_t particles);
void BoatFoam_StartBoatFoamAnimation(void);
void BoatFoam_UpdateBoatFoamAnimation(void *matrix);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
