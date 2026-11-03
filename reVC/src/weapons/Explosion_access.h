//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ExplosionTypes.h"
#ifdef __cplusplus
class CEntity;
#else
typedef struct CEntity CEntity;
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct ExplosionPosition { float x,y,z; } ExplosionPosition;
ExplosionPosition Explosion_GetExplosionPosition(uint8_t id);
void Explosion_AddAtPosition(CEntity *source,enum eExplosionType type,ExplosionPosition position);
bool Explosion_TestForExplosionInArea(enum eExplosionType type,float x1,float x2,float y1,float y2,float z1,float z2);
void Explosion_RemoveAllExplosionsInArea(ExplosionPosition position,float radius);
void Explosion_Initialise(void);
void Explosion_Update(void);
void Explosion_ClearAllExplosions(void);
void Explosion_Shutdown(void);
int8_t Explosion_GetExplosionActiveCounter(uint8_t id);
void Explosion_ResetExplosionActiveCounter(uint8_t id);
uint8_t Explosion_GetExplosionType(uint8_t id);
bool Explosion_DoesExplosionMakeSound(uint8_t id);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
