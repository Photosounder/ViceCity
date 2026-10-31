//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct tGear
{
	float fMaxVelocity;
	float fShiftUpVelocity;
	float fShiftDownVelocity;
} tGear;
typedef struct cTransmission {
	// Gear 0 is reverse, 1-5 are forward
	tGear Gears[6];
	char nDriveType;
	char nEngineType;
	int8_t nNumberOfGears;
	uint8_t Flags;
	float fEngineAcceleration;
	float fMaxVelocity;
	float fMaxCruiseVelocity;
	float fMaxReverseVelocity;
	float fCurVelocity;

} cTransmission;
#ifdef __cplusplus
extern "C" {
#endif
void Transmission_InitGearRatios(cTransmission *self);
void Transmission_CalculateGearForSimpleCar(cTransmission *self,float speed,uint8_t *gear);
float Transmission_CalculateDriveAcceleration(cTransmission *self,const float *gasPedal,uint8_t *gear,float *time,const float *velocity,bool cheat);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
