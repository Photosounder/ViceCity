//+ rouz edit (ChatGPT)
#include "AudioManager.h"

void
AudioOwner_Destroy(cAudioManager *manager)
{
	// Terminate initialized audio before resetting retained script state
	if (manager->m_bIsInitialised)
		AudioLifecycle_Terminate(manager);
	// Preserve the former embedded script-object manager destructor reset
	AudioScriptObjectManager_Reset(&manager->m_sAudioScriptObjectManager);
}

//- rouz edit (ChatGPT)
