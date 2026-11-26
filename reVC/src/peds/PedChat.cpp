//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "Camera.h"
#include "DMAudio.h"
#include "General.h"
#include "Ped.h"
#include "PedChat.h"

bool
CPed::ServiceTalkingWhenDead(void)
{
	// Delegate to the C implementation
	return PedChat_ServiceTalkingWhenDead(this);
}

void
CPed::ServiceTalking(void)
{
	// Play the sound if the C code decided it is time
	uint16 sound = PedChat_ServiceTalking(this, Game_germanGame, IsPlayer(), (*Timer_GetTimeInMilliseconds()), General_GetRandomNumberInRangeInt);
	if (sound != SOUND_NO_SOUND)
		DMAudio_PlayOneShot(m_audioEntityId, sound, 1.0f);
}

void
CPed::Say(uint16 audio)
{
	// Delegate to the C implementation
	PedChat_Say(this, audio, TheCamera.GetPosition().z, GetPosition().z, TheCamera.m_CameraAverageSpeed, (*Timer_GetTimeInMilliseconds()), General_GetRandomNumberInRangeInt);
}
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
