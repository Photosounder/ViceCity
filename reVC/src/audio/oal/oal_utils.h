//+ rouz edit (ChatGPT)
#pragma once

#ifdef AUDIO_OAL
#include "eax.h"
#include "AL/efx.h"


#ifdef __cplusplus
extern "C" {
#endif
void EFXInit(void);
void EAX3_Set(ALuint effect, const EAXLISTENERPROPERTIES *props);
void EFX_Set(ALuint effect, const EAXLISTENERPROPERTIES *props);
void EAX3_SetReverbMix(ALuint filter, float mix);
void SetEffectsLevel(ALuint uiFilter, float level);

extern LPALGENEFFECTS AudioEFX_alGenEffects;
extern LPALDELETEEFFECTS AudioEFX_alDeleteEffects;
extern LPALISEFFECT AudioEFX_alIsEffect;
extern LPALEFFECTI AudioEFX_alEffecti;
extern LPALEFFECTIV AudioEFX_alEffectiv;
extern LPALEFFECTF AudioEFX_alEffectf;
extern LPALEFFECTFV AudioEFX_alEffectfv;
extern LPALGETEFFECTI AudioEFX_alGetEffecti;
extern LPALGETEFFECTIV AudioEFX_alGetEffectiv;
extern LPALGETEFFECTF AudioEFX_alGetEffectf;
extern LPALGETEFFECTFV AudioEFX_alGetEffectfv;
extern LPALGENAUXILIARYEFFECTSLOTS AudioEFX_alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS AudioEFX_alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT AudioEFX_alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI AudioEFX_alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV AudioEFX_alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF AudioEFX_alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV AudioEFX_alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI AudioEFX_alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV AudioEFX_alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF AudioEFX_alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV AudioEFX_alGetAuxiliaryEffectSlotfv;
extern LPALGENFILTERS AudioEFX_alGenFilters;
extern LPALDELETEFILTERS AudioEFX_alDeleteFilters;
extern LPALISFILTER AudioEFX_alIsFilter;
extern LPALFILTERI AudioEFX_alFilteri;
extern LPALFILTERIV AudioEFX_alFilteriv;
extern LPALFILTERF AudioEFX_alFilterf;
extern LPALFILTERFV AudioEFX_alFilterfv;
extern LPALGETFILTERI AudioEFX_alGetFilteri;
extern LPALGETFILTERIV AudioEFX_alGetFilteriv;
extern LPALGETFILTERF AudioEFX_alGetFilterf;
extern LPALGETFILTERFV AudioEFX_alGetFilterfv;

#ifdef __cplusplus
}
#endif

#endif
//- rouz edit (ChatGPT)
