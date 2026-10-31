//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "DoorTypes.h"
typedef struct DoorVector {float x,y,z;} DoorVector;
typedef struct CDoor {
	float m_fMaxAngle;
	float m_fMinAngle;
	// direction of rotation for air resistance
	int8_t m_nDirn;
	// axis in which this door rotates
	int8_t m_nAxis;
	int8_t m_nDoorState;
	float m_fAngle;
	float m_fPrevAngle;
	float m_fAngVel;
	DoorVector m_vecSpeed;

} CDoor;
typedef struct CTrainDoor {
	float m_fClosedPosn;
	float m_fOpenPosn;
	int8_t m_nDirn;
	int8_t m_nDoorState;	// same enum as above?
	int8_t m_nAxis;
	float m_fPosn;
	float m_fPrevPosn;
	int field_14;	// unused?

} CTrainDoor;
#ifdef __cplusplus
class CVehicle;
extern "C" {
#else
typedef struct CVehicle CVehicle;
#endif
void Door_Init(CDoor *self);
void Door_SetLimits(CDoor *self,float minAngle,float maxAngle,int8_t dir,int8_t axis);
void Door_Open(CDoor *self,float ratio);
void Door_Process(CDoor *self,CVehicle *vehicle);
float Door_RetAngleWhenClosed(CDoor *self);
float Door_RetAngleWhenOpen(CDoor *self);
float Door_GetAngleOpenRatio(CDoor *self);
bool Door_IsFullyOpen(CDoor *self);
bool Door_IsClosed(CDoor *self);
void TrainDoor_Init(CTrainDoor *self);
void TrainDoor_SetLimits(CTrainDoor *self,float open,float closed,int8_t dir,int8_t axis);
void TrainDoor_Open(CTrainDoor *self,float ratio);
float TrainDoor_RetTranslationWhenClosed(CTrainDoor *self);
float TrainDoor_RetTranslationWhenOpen(CTrainDoor *self);
bool TrainDoor_IsFullyOpen(CTrainDoor *self);
bool TrainDoor_IsClosed(CTrainDoor *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
