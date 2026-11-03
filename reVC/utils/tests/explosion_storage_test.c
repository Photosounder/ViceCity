#include "Explosion_backend.h"
#include <assert.h>

static float x,y,z;
extern CollisionPointRef Explosion_HostPosition(uint32_t index);
extern uint8_t *Explosion_HostIteration(uint32_t index);
extern uint8_t *Explosion_HostCounter(uint8_t id);
extern enum eExplosionType *Explosion_HostType(uint8_t id);
extern bool *Explosion_HostMakeSound(uint8_t id);
ExplosionMutablePosition Explosion_HostMutablePosition(uint32_t index)
{
    // Supply writable position components to the production storage adapter
    assert(index==2);ExplosionMutablePosition result={&x,&y,&z};return result;
}
int main(void)
{
    // Check all borrowed fields refer to the selected production C record
    ExplosionResetRefs refs;Explosion_HostResetRefs(2,&refs);
    *refs.x=1;*refs.y=2;*refs.z=3;*refs.type=EXPLOSION_ROCKET;
    *refs.radius=7;*refs.iteration=4;*refs.activeCounter=5;*refs.makeSound=true;
    assert(x==1 && y==2 && z==3 && gaExplosion[2].m_fRadius==7);
    CollisionPointRef point=Explosion_HostPosition(2);x=9;
    assert(*point.x==9 && *point.y==2 && *point.z==3);
    assert(*Explosion_HostType(2)==EXPLOSION_ROCKET && *Explosion_HostIteration(2)==4);
    assert(*Explosion_HostCounter(2)==5 && *Explosion_HostMakeSound(2));
    assert(gaExplosion[1].m_nIteration==0 && gaExplosion[1].m_fRadius==0);
    return 0;
}
