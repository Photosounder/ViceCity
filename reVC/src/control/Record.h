//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "RecordGame.h"
#include "SharedText.h"
#ifdef __cplusplus
class CAutomobile;
class CVehicle;
class CMatrix;
extern "C" {
#else
typedef struct CAutomobile CAutomobile;
typedef struct CVehicle CVehicle;
typedef struct CMatrix CMatrix;
#endif

typedef struct RecordChaseVector { float x, y, z; } RecordChaseVector;
typedef struct CCarStateEachFrame {
    int16_t velX, velY, velZ;
    int8_t rightX, rightY, rightZ;
    int8_t forwardX, forwardY, forwardZ;
    int8_t wheel, gas, brake;
    bool handbrake;
    RecordChaseVector pos;
} CCarStateEachFrame;

bool RecordChase_IsRecording(void);
void RecordChase_Init(void);
void RecordChase_SaveOrRetrieveDataForThisFrame(void);
void RecordChase_SaveOrRetrieveCarPositions(void);
void RecordChase_StoreInfoForCar(CAutomobile *car, CCarStateEachFrame *state);
void RecordChase_RestoreInfoForMatrix(CMatrix *matrix, CCarStateEachFrame *state);
void RecordChase_RestoreInfoForCar(CAutomobile *car, CCarStateEachFrame *state, bool stop);
void RecordChase_ProcessControlCars(void);
bool RecordChase_ShouldThisPadBeLeftAlone(uint8_t pad);
void RecordChase_GiveUsACar(int32_t model, RecordChaseVector pos, float angle, CAutomobile **car, uint8_t colour1, uint8_t colour2);
void RecordChase_StartChaseScene(float startTime);
void RecordChase_CleanUpChaseScene(void);
void RecordChase_SetUpCarsForChaseScene(void);
void RecordChase_CleanUpCarsForChaseScene(void);
void RecordChase_RemoveCarFromChase(int32_t index);
CVehicle *RecordChase_TurnChaseCarIntoScriptCar(int32_t index);
void RemoveUnusedCollision(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
