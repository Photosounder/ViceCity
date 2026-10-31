//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;
#else
typedef struct CEntity CEntity;
#endif
#define NUMMOVINGTHINGS 48
typedef struct MovingThingVector { float x,y,z; } MovingThingVector;
typedef struct MovingThing {
    struct MovingThing *m_pNext,*m_pPrev;
    int16_t m_nType,m_farAway;
    MovingThingVector m_vecPosn;
    CEntity *m_pEntity;
} MovingThing;
#ifdef __cplusplus
extern "C" {
#endif
extern MovingThing MovingThings_StartCloseList,MovingThings_EndCloseList;
extern MovingThing MovingThings_aMovingThings[NUMMOVINGTHINGS];
extern int16_t MovingThings_Num;
void MovingThings_Init(void);
void MovingThings_Shutdown(void);
void MovingThings_Update(void);
void MovingThings_Render(void);
#ifdef REVC_SOFTWARE_POLYGONS
void MovingThings_RenderForEnvMap(void *camera);
#endif
void MovingThings_RegisterOne(CEntity *entity,uint16_t type);
void MovingThings_PossiblyAddThisEntity(CEntity *entity);
void MovingThing_Update(MovingThing *thing);
void MovingThing_AddToList(MovingThing *thing,MovingThing *after);
void MovingThing_RemoveFromList(MovingThing *thing);
int16_t MovingThing_SizeList(MovingThing *thing);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
