//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioPoliceGame.h"
#include "DMAudio.h"
#include <stddef.h>

CVehicle *
AudioGame_FindVehicle(void)
{
	// Preserve player and attachment lookup order without owner state
	CVehicle* vehicle = AudioPoliceHost_PlayerVehicle();
	CPlayerPed* ped = AudioPoliceHost_FindPlayer();
	if (vehicle == NULL && ped != NULL) {
		CEntity *attachedTo = AudioPoliceHost_AttachedTo(ped);
		if (attachedTo && AudioPoliceHost_IsVehicle(attachedTo))
			vehicle = AudioPoliceHost_AsVehicle(attachedTo);
	}
	return vehicle;
}

void
AudioPolice_ReportCrime(cAudioManager *manager, enum eCrimeType type, const CVector *position)
{
	// Apply the existing game eligibility and cooldown checks before updating C state
	if (manager->m_bIsInitialised && AudioPoliceHost_MusicMode() != MUSICMODE_CUTSCENE && AudioPoliceHost_WantedLevel(AudioPoliceHost_FindPlayer()) > 0 &&
		(type > CRIME_NONE || type < NUM_CRIME_TYPES) && manager->m_FrameCounter >= gMinTimeToNextReport[type]) {
		// Preserve duplicate, slot-selection, and cooldown behavior through the C API
		AudioCrimePosition plain;
		AudioPoliceHost_CrimePosition(position, &plain);
		AudioCrimes_Report(manager->m_aCrimes, gMinTimeToNextReport, type, manager->m_FrameCounter, plain.x, plain.y, plain.z);
	}
}

void
DMAudio_ReportCrime(enum eCrimeType crime, const CVector *position)
{
	// Apply the original crime operation to the live C audio owner
	AudioPolice_ReportCrime(&AudioManager, crime, position);
}
//- rouz edit (ChatGPT)
