//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CVehicle;class CCopPed;
extern "C" {
#else
typedef struct CVehicle CVehicle;typedef struct CCopPed CCopPed;
#endif
enum eSetPieceType {SETPIECE_NONE=0,SETPIECE_TWOCOPCARSINALLEY,SETPIECE_CARBLOCKINGPLAYERFROMSIDE,SETPIECE_CARRAMMINGPLAYERFROMSIDE,SETPIECE_CREATECOPPERONFOOT,SETPIECE_CREATETWOCOPPERSONFOOT,SETPIECE_TWOCARSBLOCKINGPLAYERFROMSIDE,SETPIECE_TWOCARSRAMMINGPLAYERFROMSIDE};
typedef struct SetPiecePoint {float x,y;} SetPiecePoint;
typedef struct SetPieceVector {float x,y,z;} SetPieceVector;
typedef struct SetPiece {
    uint8_t m_nType;
    uint32_t m_nLastTimeCreated;
    SetPiecePoint m_vTriggerInf,m_vTriggerSup,m_vSpawn1,m_vSpawn2,m_vTarget1,m_vTarget2;
} SetPiece;
extern bool SetPieces_bDebug;
extern uint32_t SetPieces_NumSetPieces;
extern SetPiece SetPieces_aSetPieces[NUM_SETPIECES];
void SetPieces_Init(void);
void SetPieces_AddOne(uint8_t type,SetPiecePoint triggerInf,SetPiecePoint triggerSup,SetPiecePoint spawn1,SetPiecePoint target1,SetPiecePoint spawn2,SetPiecePoint target2);
void SetPieces_Update(void);
void SetPieces_Save(uint8_t *buffer,uint32_t *size);
void SetPieces_Load(uint8_t *buffer,uint32_t size);
void SetPiece_Update(SetPiece *record);
CVehicle *SetPiece_TryToGenerateCopCar(SetPiece *record,SetPiecePoint spawn,SetPiecePoint target);
CCopPed *SetPiece_TryToGenerateCopPed(SetPiece *record,SetPiecePoint spawn);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
