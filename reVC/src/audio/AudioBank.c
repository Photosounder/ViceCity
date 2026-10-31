//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "sampman.h"

#ifdef GTA_PS2
void
AudioBank_LoadIfNecessary(uint8_t bank)
{
	// Preserve the original loaded-bank gate and captured byte argument
	if(!SampleManager_IsSampleBankLoaded(&SampleManager, bank))
		SampleManager_LoadSampleBank(&SampleManager, bank);
}
#endif
//- rouz edit (ChatGPT)
