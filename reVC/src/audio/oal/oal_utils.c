//+ rouz edit (ChatGPT)
#include <math.h>
#include "oal_utils.h"

#ifdef AUDIO_OAL

/* Prefixed C symbols avoid collisions with static OpenAL extension exports */

LPALGENEFFECTS AudioEFX_alGenEffects;
LPALDELETEEFFECTS AudioEFX_alDeleteEffects;
LPALISEFFECT AudioEFX_alIsEffect;
LPALEFFECTI AudioEFX_alEffecti;
LPALEFFECTIV AudioEFX_alEffectiv;
LPALEFFECTF AudioEFX_alEffectf;
LPALEFFECTFV AudioEFX_alEffectfv;
LPALGETEFFECTI AudioEFX_alGetEffecti;
LPALGETEFFECTIV AudioEFX_alGetEffectiv;
LPALGETEFFECTF AudioEFX_alGetEffectf;
LPALGETEFFECTFV AudioEFX_alGetEffectfv;
LPALGENAUXILIARYEFFECTSLOTS AudioEFX_alGenAuxiliaryEffectSlots;
LPALDELETEAUXILIARYEFFECTSLOTS AudioEFX_alDeleteAuxiliaryEffectSlots;
LPALISAUXILIARYEFFECTSLOT AudioEFX_alIsAuxiliaryEffectSlot;
LPALAUXILIARYEFFECTSLOTI AudioEFX_alAuxiliaryEffectSloti;
LPALAUXILIARYEFFECTSLOTIV AudioEFX_alAuxiliaryEffectSlotiv;
LPALAUXILIARYEFFECTSLOTF AudioEFX_alAuxiliaryEffectSlotf;
LPALAUXILIARYEFFECTSLOTFV AudioEFX_alAuxiliaryEffectSlotfv;
LPALGETAUXILIARYEFFECTSLOTI AudioEFX_alGetAuxiliaryEffectSloti;
LPALGETAUXILIARYEFFECTSLOTIV AudioEFX_alGetAuxiliaryEffectSlotiv;
LPALGETAUXILIARYEFFECTSLOTF AudioEFX_alGetAuxiliaryEffectSlotf;
LPALGETAUXILIARYEFFECTSLOTFV AudioEFX_alGetAuxiliaryEffectSlotfv;
LPALGENFILTERS AudioEFX_alGenFilters;
LPALDELETEFILTERS AudioEFX_alDeleteFilters;
LPALISFILTER AudioEFX_alIsFilter;
LPALFILTERI AudioEFX_alFilteri;
LPALFILTERIV AudioEFX_alFilteriv;
LPALFILTERF AudioEFX_alFilterf;
LPALFILTERFV AudioEFX_alFilterfv;
LPALGETFILTERI AudioEFX_alGetFilteri;
LPALGETFILTERIV AudioEFX_alGetFilteriv;
LPALGETFILTERF AudioEFX_alGetFilterf;
LPALGETFILTERFV AudioEFX_alGetFilterfv;


void EFXInit(void)
{
    // Load OpenAL procedures by their original extension names
	/* Define a macro to help load the function pointers. */
#define LOAD_PROC(T, x)  ((AudioEFX_##x) = (T)alGetProcAddress(#x))
	LOAD_PROC(LPALGENEFFECTS, alGenEffects);
	LOAD_PROC(LPALDELETEEFFECTS, alDeleteEffects);
	LOAD_PROC(LPALISEFFECT, alIsEffect);
	LOAD_PROC(LPALEFFECTI, alEffecti);
	LOAD_PROC(LPALEFFECTIV, alEffectiv);
	LOAD_PROC(LPALEFFECTF, alEffectf);
	LOAD_PROC(LPALEFFECTFV, alEffectfv);
	LOAD_PROC(LPALGETEFFECTI, alGetEffecti);
	LOAD_PROC(LPALGETEFFECTIV, alGetEffectiv);
	LOAD_PROC(LPALGETEFFECTF, alGetEffectf);
	LOAD_PROC(LPALGETEFFECTFV, alGetEffectfv);
	
	LOAD_PROC(LPALGENFILTERS, alGenFilters);
	LOAD_PROC(LPALDELETEFILTERS, alDeleteFilters);
	LOAD_PROC(LPALISFILTER, alIsFilter);
	LOAD_PROC(LPALFILTERI, alFilteri);
	LOAD_PROC(LPALFILTERIV, alFilteriv);
	LOAD_PROC(LPALFILTERF, alFilterf);
	LOAD_PROC(LPALFILTERFV, alFilterfv);
	LOAD_PROC(LPALGETFILTERI, alGetFilteri);
	LOAD_PROC(LPALGETFILTERIV, alGetFilteriv);
	LOAD_PROC(LPALGETFILTERF, alGetFilterf);
	LOAD_PROC(LPALGETFILTERFV, alGetFilterfv);
	
	LOAD_PROC(LPALGENAUXILIARYEFFECTSLOTS, alGenAuxiliaryEffectSlots);
	LOAD_PROC(LPALDELETEAUXILIARYEFFECTSLOTS, alDeleteAuxiliaryEffectSlots);
	LOAD_PROC(LPALISAUXILIARYEFFECTSLOT, alIsAuxiliaryEffectSlot);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTI, alAuxiliaryEffectSloti);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTIV, alAuxiliaryEffectSlotiv);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTF, alAuxiliaryEffectSlotf);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTFV, alAuxiliaryEffectSlotfv);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTI, alGetAuxiliaryEffectSloti);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTIV, alGetAuxiliaryEffectSlotiv);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTF, alGetAuxiliaryEffectSlotf);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTFV, alGetAuxiliaryEffectSlotfv);
#undef LOAD_PROC
}

void SetEffectsLevel(ALuint uiFilter, float level)
{
    // Set the low-pass filter gains through prefixed C procedure pointers
	AudioEFX_alFilteri(uiFilter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	AudioEFX_alFilterf(uiFilter, AL_LOWPASS_GAIN, 1.0f);
	AudioEFX_alFilterf(uiFilter, AL_LOWPASS_GAINHF, level);
}

static inline float gain_to_mB(float gain)
{
    // Convert linear gain to the existing millibel scale
    return (gain > 1e-5f) ? (float)(log10f(gain) * 2000.0f) : -10000l;
}

static inline float mB_to_gain(float millibels)
{
    // Convert the existing millibel scale back to linear gain
    return (millibels > -10000.0f) ? powf(10.0f, millibels/2000.0f) : 0.0f;
}

static inline float clampF(float val, float minval, float maxval)
{
    // Clamp effect parameters to their supported ranges
    if(val >= maxval) return maxval;
    if(val <= minval) return minval;
    return val;
}

void EAX3_Set(ALuint effect, const EAXLISTENERPROPERTIES *props)
{
    // Apply EAX listener parameters through the C effect interface
	AudioEFX_alEffecti (effect, AL_EFFECT_TYPE,                     AL_EFFECT_EAXREVERB);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_DENSITY,               clampF(powf(props->flEnvironmentSize, 3.0f) / 16.0f, 0.0f, 1.0f));
	AudioEFX_alEffectf (effect, AL_EAXREVERB_DIFFUSION,             props->flEnvironmentDiffusion);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_GAIN,                  mB_to_gain((float)props->lRoom));
	AudioEFX_alEffectf (effect, AL_EAXREVERB_GAINHF,                mB_to_gain((float)props->lRoomHF));
	AudioEFX_alEffectf (effect, AL_EAXREVERB_GAINLF,                mB_to_gain((float)props->lRoomLF));
	AudioEFX_alEffectf (effect, AL_EAXREVERB_DECAY_TIME,            props->flDecayTime);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_DECAY_HFRATIO,         props->flDecayHFRatio);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_DECAY_LFRATIO,         props->flDecayLFRatio);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_REFLECTIONS_GAIN,      clampF(mB_to_gain((float)props->lReflections), AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN));
	AudioEFX_alEffectf (effect, AL_EAXREVERB_REFLECTIONS_DELAY,     props->flReflectionsDelay);
	AudioEFX_alEffectfv(effect, AL_EAXREVERB_REFLECTIONS_PAN,       &props->vReflectionsPan.x);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_LATE_REVERB_GAIN,      clampF(mB_to_gain((float)props->lReverb), AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN));
	AudioEFX_alEffectf (effect, AL_EAXREVERB_LATE_REVERB_DELAY,     props->flReverbDelay);
	AudioEFX_alEffectfv(effect, AL_EAXREVERB_LATE_REVERB_PAN,       &props->vReverbPan.x);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_ECHO_TIME,             props->flEchoTime);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_ECHO_DEPTH,            props->flEchoDepth);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_MODULATION_TIME,       props->flModulationTime);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_MODULATION_DEPTH,      props->flModulationDepth);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, clampF(mB_to_gain(props->flAirAbsorptionHF), AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF));
	AudioEFX_alEffectf (effect, AL_EAXREVERB_HFREFERENCE,           props->flHFReference);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_LFREFERENCE,           props->flLFReference);
	AudioEFX_alEffectf (effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR,   props->flRoomRolloffFactor);
	AudioEFX_alEffecti (effect, AL_EAXREVERB_DECAY_HFLIMIT,         (props->ulFlags&EAXLISTENERFLAGS_DECAYHFLIMIT) ? AL_TRUE : AL_FALSE);
}

void EFX_Set(ALuint effect, const EAXLISTENERPROPERTIES *props)
{
    // Apply standard reverb parameters through the C effect interface
	AudioEFX_alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
	
	AudioEFX_alEffectf(effect, AL_REVERB_DENSITY,               clampF(powf(props->flEnvironmentSize, 3.0f) / 16.0f, 0.0f, 1.0f));
	AudioEFX_alEffectf(effect, AL_REVERB_DIFFUSION,             props->flEnvironmentDiffusion);
	AudioEFX_alEffectf(effect, AL_REVERB_GAIN,                  mB_to_gain((float)props->lRoom));
	AudioEFX_alEffectf(effect, AL_REVERB_GAINHF,                mB_to_gain((float)props->lRoomHF));
	AudioEFX_alEffectf(effect, AL_REVERB_DECAY_TIME,            props->flDecayTime);
	AudioEFX_alEffectf(effect, AL_REVERB_DECAY_HFRATIO,         props->flDecayHFRatio);
	AudioEFX_alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN,      clampF(mB_to_gain((float)props->lReflections), AL_EAXREVERB_MIN_REFLECTIONS_GAIN, AL_EAXREVERB_MAX_REFLECTIONS_GAIN));
	AudioEFX_alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY,     props->flReflectionsDelay);
	AudioEFX_alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN,      clampF(mB_to_gain((float)props->lReverb), AL_EAXREVERB_MIN_LATE_REVERB_GAIN, AL_EAXREVERB_MAX_LATE_REVERB_GAIN));
	AudioEFX_alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY,     props->flReverbDelay);
	AudioEFX_alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, clampF(mB_to_gain(props->flAirAbsorptionHF), AL_EAXREVERB_MIN_AIR_ABSORPTION_GAINHF, AL_EAXREVERB_MAX_AIR_ABSORPTION_GAINHF));
	AudioEFX_alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR,   props->flRoomRolloffFactor);
	AudioEFX_alEffecti(effect, AL_REVERB_DECAY_HFLIMIT,         (props->ulFlags&EAXLISTENERFLAGS_DECAYHFLIMIT) ? AL_TRUE : AL_FALSE);
}

void EAX3_SetReverbMix(ALuint filter, float mix)
{
    // Preserve the existing integer millibel conversion for reverb mixing
	//long vol=(long)linear_to_dB(mix);
	//DSPROPERTY_EAXBUFFER_ROOMHF,
	//DSPROPERTY_EAXBUFFER_ROOM,
	//DSPROPERTY_EAXBUFFER_REVERBMIX,
	
	long mbvol = gain_to_mB(mix);
	float mb   = mbvol;
	float mbhf = mbvol;
	
	AudioEFX_alFilteri(filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
	AudioEFX_alFilterf(filter, AL_LOWPASS_GAIN,   mB_to_gain((mb < 0.0f ? mb : 0.0f)));
	AudioEFX_alFilterf(filter, AL_LOWPASS_GAINHF, mB_to_gain(mbhf));
}

#endif//- rouz edit (ChatGPT)
