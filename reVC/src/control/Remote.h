//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Remote_GivePlayerRemoteControlledCar(float x,float y,float z,float rotation,uint16_t model);
void Remote_TakeRemoteControlledCarFromPlayer(bool blowUp);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
