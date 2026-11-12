//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "PedData.h"
#include "PedAudioData.h"


typedef int32_t (*PedChatRandomFn)(int32_t low, int32_t high);

#ifdef __cplusplus
extern "C" {
#endif
extern PedAudioData CommentWaitTime[56];

// Whether the queued sound of a dead ped is the death cry
bool PedChat_ServiceTalkingWhenDead(const CPedData *self);
// Time the queued sound, returns the sound that has to be played now or SOUND_NO_SOUND
uint16_t PedChat_ServiceTalking(CPedData *self, bool germanGame, bool isPlayer, uint32_t now, PedChatRandomFn random);
// Queue a sound unless the camera is too fast or a better one is already queued
void PedChat_Say(CPedData *self, uint16_t audio, float camZ, float pedZ, float camSpeed, uint32_t now, PedChatRandomFn random);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
