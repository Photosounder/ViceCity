//+ rouz edit (Claude)
#pragma once
#include "CarGenData.h"

#ifdef __cplusplus
extern "C" {
#endif
// Switching generators on and off, activeCount is the number of generators that can still spawn
void CarGen_SwitchOff(CCarGeneratorData *self, uint32_t *activeCount);
void CarGen_SwitchOn(CCarGeneratorData *self, uint32_t *activeCount, uint32_t now);
uint32_t CarGen_CalcNextGen(uint32_t now);
void CarGen_Setup(CCarGeneratorData *self, float x, float y, float z, float angle, int32_t mi, int16_t color1, int16_t color2, uint8_t force, uint8_t alarm, uint8_t lock, uint16_t minDelay, uint16_t maxDelay, uint32_t now);
// Whether the generator is due to spawn, the range test with the players still has to pass
bool CarGen_IsDueToSpawn(const CCarGeneratorData *self, bool generateEvenIfPlayerIsClose, uint32_t now);
// Called after a car was spawned, uses up one spawn and arms the timer
void CarGen_AfterSpawn(CCarGeneratorData *self, uint32_t *activeCount, uint32_t now);
// A spawned car that the player took over is no longer owned by the generator
void CarGen_ReleaseAfterPlayerTookOver(CCarGeneratorData *self);
// Decide whether the generator is within range of the player, visible tells if a boat in view can be spawned in the distance
typedef bool (*CarGenVisibleFn)(void *context);
bool CarGen_CheckRange(CCarGeneratorData *self, float dirX, float dirY, float speedX, float speedY, float genDistMult, bool isBoat, CarGenVisibleFn visible, void *context, bool generateEvenIfPlayerIsClose);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
