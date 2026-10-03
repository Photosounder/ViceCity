//+ rouz edit (ChatGPT)
#include "AudioSound.h"
#include "AudioMath.h"

#ifdef EXTERNAL_3D_SOUND
void
AudioSoundQueue_AdjustVolume(tSound *requested, const uint8_t *order, uint8_t count)
{
	// Adjust emitting volume only for ordered three-dimensional requested sounds
	for (uint8_t i = 0; i < count; i++) {
		tSound *pSample = &requested[order[i]];

		if (!pSample->m_bIs2D)
			pSample->m_nEmittingVolume = AudioMath_ComputeEmittingVolume(pSample->m_nEmittingVolume, pSample->m_MaxDistance, pSample->m_fDistance);
	}
}
#endif
//- rouz edit (ChatGPT)
