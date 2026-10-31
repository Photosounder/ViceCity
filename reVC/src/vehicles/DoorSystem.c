//+ rouz edit (ChatGPT)
#include "Door.h"
#include "Door_backend.h"
#include <string.h>
#include <math.h>
#define Abs(value) fabsf(value)
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))

void
Door_Init(CDoor *self)
{
    // Preserve the original full-record zeroing including padding
	memset(self, 0, sizeof(*self));
}

void
Door_SetLimits(CDoor *self,float minAngle,float maxAngle,int8_t dir,int8_t axis)
{
    // Change only the original limit and orientation fields
		self->m_fMinAngle = minAngle;
		self->m_fMaxAngle = maxAngle;
		self->m_nDirn = dir;
		self->m_nAxis = axis;
	}

void
Door_Open(CDoor *self,float ratio)
{
    // Preserve the complete original door operation and its ordered state updates
	float open;

	self->m_fPrevAngle = self->m_fAngle;
	open = Door_RetAngleWhenOpen(self);
	if(ratio < 1.0f){
		self->m_fAngle = open*ratio;
		if(self->m_fAngle == 0.0f)
			self->m_fAngVel = 0.0f;
	}else{
		self->m_nDoorState = DOORST_OPEN;
		self->m_fAngle = open;
	}
}

void
Door_Process(CDoor *self,CVehicle *vehicle)
{
    // Preserve the complete original door operation and its ordered state updates
	static DoorVector vecOffset={1.0f,0.0f,0.0f};
	DoorVector speed = Door_BackendSpeed(vehicle,&vecOffset);
	DoorVector vecSpeedDiff={speed.x-self->m_vecSpeed.x,speed.y-self->m_vecSpeed.y,speed.z-self->m_vecSpeed.z};
	vecSpeedDiff = Door_BackendToLocal(vehicle,&vecSpeedDiff);

	// air resistance
	float fSpeedDiff = 0.0f;	// uninitialized in game
	switch(self->m_nAxis){
	case 0:	// x-axis
		if(self->m_nDirn)
			fSpeedDiff = vecSpeedDiff.y + vecSpeedDiff.z;
		else
			fSpeedDiff = -(vecSpeedDiff.y + vecSpeedDiff.z);
		break;

	// we don't support y axis apparently?

	case 2:	// z-axis
		if(self->m_nDirn)
			fSpeedDiff = -(vecSpeedDiff.y + vecSpeedDiff.x);
		else
			fSpeedDiff = vecSpeedDiff.y - vecSpeedDiff.x;
		break;
	}
	fSpeedDiff = Clamp(fSpeedDiff, -0.2f, 0.2f);
	if(Abs(fSpeedDiff) > 0.002f)
		self->m_fAngVel += fSpeedDiff;
	self->m_fAngVel *= 0.945f;
	self->m_fAngVel = Clamp(self->m_fAngVel, -0.3f, 0.3f);

	self->m_fAngle += self->m_fAngVel;
	self->m_nDoorState = DOORST_SWINGING;
	if(self->m_fAngle > self->m_fMaxAngle){
		self->m_fAngle = self->m_fMaxAngle;
		self->m_fAngVel *= -0.8f;
		self->m_nDoorState = DOORST_OPEN;
	}
	if(self->m_fAngle < self->m_fMinAngle){
		self->m_fAngle = self->m_fMinAngle;
		self->m_fAngVel *= -0.8f;
		self->m_nDoorState = DOORST_CLOSED;
	}
	self->m_vecSpeed = speed;
}

float
Door_RetAngleWhenClosed(CDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	if(Abs(self->m_fMaxAngle) < Abs(self->m_fMinAngle))
		return self->m_fMaxAngle;
	else
		return self->m_fMinAngle;
}

float
Door_RetAngleWhenOpen(CDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	if(Abs(self->m_fMaxAngle) < Abs(self->m_fMinAngle))
		return self->m_fMinAngle;
	else
		return self->m_fMaxAngle;
}

float
Door_GetAngleOpenRatio(CDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	float open = Door_RetAngleWhenOpen(self);
	if(open == 0.0f)
		return 0.0f;
	return self->m_fAngle/open;
}

bool
Door_IsFullyOpen(CDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	// why -0.5? that's around 28 deg less than fully open
	if(Abs(self->m_fAngle) < Abs(Door_RetAngleWhenOpen(self)) - 0.5f)
		return false;
	return true;
}

bool
Door_IsClosed(CDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	return self->m_fAngle == Door_RetAngleWhenClosed(self);
}

void
TrainDoor_Init(CTrainDoor *self)
{
    // Preserve the original full-record zeroing including padding
	memset(self, 0, sizeof(*self));
}

void
TrainDoor_SetLimits(CTrainDoor *self,float open,float closed,int8_t dir,int8_t axis)
{
    // Change only the original limit and orientation fields
		self->m_fOpenPosn = open;
		self->m_fClosedPosn = closed;
		self->m_nDirn = dir;
		self->m_nAxis = axis;
	}

void
TrainDoor_Open(CTrainDoor *self,float ratio)
{
    // Preserve the complete original door operation and its ordered state updates
	float open;

	self->m_fPrevPosn = self->m_fPosn;
	open = TrainDoor_RetTranslationWhenOpen(self);
	if(ratio < 1.0f){
		self->m_fPosn = open*ratio;
	}else{
		self->m_nDoorState = DOORST_OPEN;
		self->m_fPosn = open;
	}
}

float
TrainDoor_RetTranslationWhenClosed(CTrainDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	if(Abs(self->m_fClosedPosn) < Abs(self->m_fOpenPosn))
		return self->m_fClosedPosn;
	else
		return self->m_fOpenPosn;
}

float
TrainDoor_RetTranslationWhenOpen(CTrainDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	if(Abs(self->m_fClosedPosn) < Abs(self->m_fOpenPosn))
		return self->m_fOpenPosn;
	else
		return self->m_fClosedPosn;
}

bool
TrainDoor_IsFullyOpen(CTrainDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	// 0.5f again...
	if(Abs(self->m_fPosn) < Abs(TrainDoor_RetTranslationWhenOpen(self)) - 0.5f)
		return false;
	return true;
}

bool
TrainDoor_IsClosed(CTrainDoor *self)
{
    // Preserve the complete original door operation and its ordered state updates
	return self->m_fPosn == TrainDoor_RetTranslationWhenClosed(self);
}

//- rouz edit (ChatGPT)
