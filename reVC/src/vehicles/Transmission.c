//+ rouz edit (ChatGPT)
#include "Transmission.h"
#include "HandlingFlags.h"
#include "Timer.h"
#include <string.h>
#include <math.h>
#include <stddef.h>
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
static inline float sq(float value)
{
    // Preserve the original float square helper's expression and return type
    return value*value;
}
#define Abs(value) fabsf(value)
//+ rouz edit (ChatGPT)


void
Transmission_InitGearRatios(cTransmission *self)
{
	// Preserve the complete original transmission operation and its ordered live accesses
	static tGear *pGearRatio0 = NULL;
	static tGear *pGearRatio1 = NULL;
	int i;
	float velocityDiff;

	memset(self->Gears, 0, sizeof(self->Gears));

	for(i = 1; i <= self->nNumberOfGears; i++){
		pGearRatio0 = &self->Gears[i-1];
		pGearRatio1 = &self->Gears[i];

		pGearRatio1->fMaxVelocity = (float)i / self->nNumberOfGears * self->fMaxVelocity;

		velocityDiff = pGearRatio1->fMaxVelocity - pGearRatio0->fMaxVelocity;

		if(i >= self->nNumberOfGears){
			pGearRatio1->fShiftUpVelocity = self->fMaxVelocity;
		}else{
			self->Gears[i+1].fShiftDownVelocity = velocityDiff*0.42f + pGearRatio0->fMaxVelocity;
			pGearRatio1->fShiftUpVelocity = velocityDiff*0.6667f + pGearRatio0->fMaxVelocity;
		}
	}

	// Reverse gear
	self->Gears[0].fMaxVelocity = self->fMaxReverseVelocity;
	self->Gears[0].fShiftUpVelocity = -0.01f;
	self->Gears[0].fShiftDownVelocity = self->fMaxReverseVelocity;

	self->Gears[1].fShiftDownVelocity = -0.01f;
}

void
Transmission_CalculateGearForSimpleCar(cTransmission *self,float speed,uint8_t *gear)
{
	// Preserve the complete original transmission operation and its ordered live accesses
	static tGear *pGearRatio;

	pGearRatio = &self->Gears[(*gear)];
	self->fCurVelocity = speed;
	if(speed > pGearRatio->fShiftUpVelocity)
		(*gear)++;
	else if(speed < pGearRatio->fShiftDownVelocity){
		if((*gear) - 1 < 0)
			(*gear) = 0;
		else
			(*gear)--;
	}
}

float
Transmission_CalculateDriveAcceleration(cTransmission *self,const float *gasPedal,uint8_t *gear,float *time,const float *velocity,bool cheat)
{
	// Retain the unused time reference without reading or modifying its storage
	(void)time;
	// Preserve the complete original transmission operation and its ordered live accesses
	static float fAcceleration = 0.0f;
	static float fVelocity;
	static float fCheat;
	static tGear *pGearRatio;

	fVelocity = (*velocity);
	if(fVelocity < self->fMaxReverseVelocity){
		fVelocity = self->fMaxReverseVelocity;
		return 0.0f;
	}
	if(fVelocity > self->fMaxVelocity){
		fVelocity = self->fMaxVelocity;
		return 0.0f;
	}
	self->fCurVelocity = fVelocity;

	assert((*gear) <= self->nNumberOfGears);

	pGearRatio = &self->Gears[(*gear)];
	if(fVelocity > pGearRatio->fShiftUpVelocity){
		if((*gear) != 0 || (*gasPedal) > 0.0f){
			(*gear)++;
			return Transmission_CalculateDriveAcceleration(self,gasPedal,gear,time,&fVelocity,false);
		}
	}else if(fVelocity < pGearRatio->fShiftDownVelocity && (*gear) != 0){
		if((*gear) != 1 || (*gasPedal) < 0.0f){
			(*gear)--;
			return Transmission_CalculateDriveAcceleration(self,gasPedal,gear,time,&fVelocity,false);
		}
	}

	float speedMul, accelMul;

	if((*gear) < 1){
		// going reverse
		accelMul = (self->Flags & HANDLING_2G_BOOST) ? 2.0f : 1.0f;
		speedMul = -1.0f;
	}else if(self->nNumberOfGears == 1){
		accelMul = 1.0f;
		speedMul = 1.0f;
	}else{
		// BUG or not? this is 1.0 normally but 0.0 in the highest (*gear)
		float f = 1.0f - ((*gear)-1)/(self->nNumberOfGears-1);
		speedMul = 3.0f*sq(f) + 1.0f;
		// This is pretty ugly, could be written more clearly
		if(self->Flags & HANDLING_2G_BOOST){
			if((*gear) == 1)
				accelMul = (self->Flags & HANDLING_1G_BOOST) ? 2.0f : 1.6f;
			else if((*gear) == 2)
				accelMul = 1.3f;
			else
				accelMul = 1.0f;
		}else if(self->Flags & HANDLING_1G_BOOST && (*gear) == 1){
			accelMul = 2.0f;
		}else
			accelMul = 1.0f;
	}

	if(cheat)
		fCheat = 1.2f;
	else
		fCheat = 1.0f;
	float targetVelocity = self->Gears[(*gear)].fMaxVelocity*speedMul*fCheat;
//+ rouz edit (ChatGPT)
//targetVelocity = 110.f  * 1000.0f / (60.0f * 60.0f * 50.0f);
//- rouz edit (ChatGPT)
	float accel = (targetVelocity - fVelocity) * (self->fEngineAcceleration*accelMul) / Abs(targetVelocity);
	if(Abs(fVelocity) < Abs(self->Gears[(*gear)].fMaxVelocity*fCheat))
		fAcceleration = (*gasPedal) * accel * (*Timer_GetTimeStep());
	else
		fAcceleration = 0.0f;
	return fAcceleration;
}

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
