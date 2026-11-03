//+ rouz edit (ChatGPT)
#include "Explosion_backend.h"

ExplosionRecord gaExplosion[NUM_EXPLOSIONS];

void Explosion_HostResetRefs(uint32_t index,ExplosionResetRefs *refs)
{
    // Borrow the C record fields and writable native position components
    ExplosionRecord *record=&gaExplosion[index];
    ExplosionMutablePosition position=Explosion_HostMutablePosition(index);
    refs->type=&record->m_ExplosionType;
    refs->x=position.x;refs->y=position.y;refs->z=position.z;
    refs->radius=&record->m_fRadius;refs->power=&record->m_fPower;refs->propagation=&record->m_fPropagationRate;refs->zshift=&record->m_fZshift;
    refs->creator=&record->m_pCreatorEntity;refs->victim=&record->m_pVictimEntity;
    refs->stopTime=&record->m_fStopTime;refs->iteration=&record->m_nIteration;refs->activeCounter=&record->m_nActiveCounter;
    refs->particlesExpireTime=&record->m_nParticlesExpireTime;refs->startTime=&record->m_fStartTime;
    refs->isBoat=&record->m_bIsBoat;refs->makeSound=&record->m_bMakeSound;
}
CollisionPointRef Explosion_HostPosition(uint32_t index)
{
    // Expose read-only references to the live native position components
    ExplosionMutablePosition position=Explosion_HostMutablePosition(index);
    return CollisionPoint_Ref(position.x,position.y,position.z);
}
uint8_t *Explosion_HostIteration(uint32_t index)
{
    // Borrow the live iteration field
    return &gaExplosion[index].m_nIteration;
}
uint8_t *Explosion_HostCounter(uint8_t id)
{
    // Borrow the live active counter
    return &gaExplosion[id].m_nActiveCounter;
}
enum eExplosionType *Explosion_HostType(uint8_t id)
{
    // Borrow the live explosion type
    return &gaExplosion[id].m_ExplosionType;
}
bool *Explosion_HostMakeSound(uint8_t id)
{
    // Borrow the live sound flag
    return &gaExplosion[id].m_bMakeSound;
}
//- rouz edit (ChatGPT)
