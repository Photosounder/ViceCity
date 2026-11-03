//+ rouz edit (ChatGPT)
#pragma once

class CEntity;
class CVector;

#include "ExplosionTypes.h"
#include "Explosion_access.h"
#include <stdint.h> // rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#ifdef SIMPLER_MISSIONS
bool Explosion_AddExplosion(CEntity *explodingEntity, CEntity *culprit, eExplosionType type, const CVector &pos, uint32 lifetime, bool makeSound = true, float radius = -1.0f);
#else
bool Explosion_AddExplosion(CEntity* explodingEntity, CEntity* culprit, eExplosionType type, const CVector& pos, uint32 lifetime, bool makeSound = true);
#endif
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
