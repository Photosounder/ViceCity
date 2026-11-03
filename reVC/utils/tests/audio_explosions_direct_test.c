#include "AudioManager.h"
#include "AudioSamples.h"
#include "AudioGeometry.h"
#include "Explosion_access.h"
#include <assert.h>
#include <string.h>

static uint8_t counters[NUM_EXPLOSIONS];
static int submitted,positionReads,resetCount;
static cAudioManager manager;

bool Explosion_DoesExplosionMakeSound(uint8_t id)
{
    // Leave the third active record muted
    return id!=2;
}
int8_t Explosion_GetExplosionActiveCounter(uint8_t id)
{
    // Return the current live counter
    return (int8_t)counters[id];
}
void Explosion_ResetExplosionActiveCounter(uint8_t id)
{
    // Consume each selected record before its type and position are read
    assert(counters[id]==1);counters[id]=0;++resetCount;
}
uint8_t Explosion_GetExplosionType(uint8_t id)
{
    // Confirm the record has already been consumed
    assert(counters[id]==0);return id==0 ? EXPLOSION_ROCKET : EXPLOSION_MOLOTOV;
}
ExplosionPosition Explosion_GetExplosionPosition(uint8_t id)
{
    // Supply one audible and one distant position after the reset
    assert(counters[id]==0);++positionReads;
    ExplosionPosition position={id==0 ? 3.0f : 300.0f,4.0f,0.0f};return position;
}
float AudioGeometry_DistanceSquared(const AudioSoundPosition *position)
{
    // Measure the coordinates copied into the audio request
    return position->x*position->x+position->y*position->y+position->z*position->z;
}
int32_t AudioMath_RandomDisplacement(const int32_t values[5],uint32_t seed)
{
    // Keep sample frequency deterministic for the fixture
    (void)values;(void)seed;return 0;
}
uint8_t AudioMath_ComputeVolume(uint8_t volume,float maxDistance,float distance)
{
    // Verify only the nearby record reaches the volume calculation
    assert(maxDistance==200 && distance==5);return volume;
}
void AudioRequests_Submit(cAudioManager *owner)
{
    // Verify the selected sample and copied position at the submission boundary
    assert(owner==&manager && owner->m_sQueueSample.m_nSampleIndex==SFX_EXPLOSION_2);
    assert(owner->m_sQueueSample.m_nCounter==0 && owner->m_sQueueSample.m_vecPos.x==3 && owner->m_sQueueSample.m_vecPos.y==4);
    ++submitted;
}
int main(void)
{
    // Verify audible, distant, and muted records retain their original consumption rules
    memset(&manager,0,sizeof(manager));counters[0]=1;counters[1]=1;counters[2]=1;
    AudioExplosions_Process(&manager,0);
    assert(submitted==1 && positionReads==2 && resetCount==2);
    assert(counters[0]==0 && counters[1]==0 && counters[2]==1);
    AudioExplosions_Process(&manager,0);
    assert(submitted==1 && positionReads==2 && resetCount==2);
    return 0;
}
