//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CVehicle;
#else
typedef struct CVehicle CVehicle;
#endif
typedef struct RoadBlockVector {float x,y,z;} RoadBlockVector;
typedef struct CScriptRoadblock {RoadBlockVector m_vInf,m_vSup;bool m_bInUse;} CScriptRoadblock;
#ifdef __cplusplus
extern "C" {
#endif
extern int16_t RoadBlocks_NumRoadBlocks;
extern int16_t RoadBlocks_RoadBlockNodes[NUMROADBLOCKS];
extern bool RoadBlocks_InOrOut[NUMROADBLOCKS];
extern CScriptRoadblock RoadBlocks_aScriptRoadBlocks[NUM_SCRIPT_ROADBLOCKS];
#ifdef SECUROM
extern uint8_t roadBlocksPirateCheck;
#endif
RoadBlockVector ScriptRoadblock_GetPosition(const CScriptRoadblock *block);
void RoadBlocks_Init(void);
void RoadBlocks_GenerateRoadBlockCopsForCar(CVehicle *vehicle,int32_t roadBlockType);
void RoadBlocks_GenerateRoadBlocks(void);
void RoadBlocks_ClearScriptRoadBlocks(void);
void RoadBlocks_RegisterScriptRoadBlock(RoadBlockVector inf,RoadBlockVector sup);
void RoadBlocks_CreateRoadBlockBetween2Points(RoadBlockVector point1,RoadBlockVector point2);
#ifdef __cplusplus
}
class CVector;
RoadBlockVector RoadBlocks_FromVector(CVector value);
#endif
//- rouz edit (ChatGPT)
