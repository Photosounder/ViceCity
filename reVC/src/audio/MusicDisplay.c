//+ rouz edit (ChatGPT)
#include "MusicManager.h"
#include "AudioManager.h"
#include "DMAudio.h"
#include "sampman.h"
#include "soundlist.h"
#include <stddef.h>

void
MusicDisplay_StationName(cMusicManager *manager)
{
	// Preserve the shared station text, timer and original display operation order
	uint8_t gStreamedSound;
	static uint16_t *pCurrentStation = NULL;
	static uint8_t cDisplay = 0;

	if(!MusicDisplayHost_Paused() && !MusicDisplayHost_Widescreen() && MusicRadio_PlayerInCar(manager) &&
	   !AudioEnvironmentHost_Replay()) {
		CVehicle *vehicle = AudioGame_FindVehicle();

		if (vehicle)
		{
			// Prev scroll needs it to be signed, and manager->m_nFrontendTrack can be NO_TRACK thus FIX_BUGS
#if defined RADIO_SCROLL_TO_PREV_STATION || defined FIX_BUGS
			int track;
#else
			uint8_t track;
#endif
			gStreamedSound = (*MusicRadioHost_Station(vehicle));
			if (gStreamedSound >= STREAMED_SOUND_CITY_AMBIENT && gStreamedSound <= STREAMED_SOUND_AMBSIL_AMBIENT)
				gStreamedSound = RADIO_OFF;
			if (gNumRetunePresses != 0)
			{
				track = gNumRetunePresses + gStreamedSound;
#ifdef RADIO_SCROLL_TO_PREV_STATION
				while (track < 0) track += NUM_RADIOS + 1;
#endif
				while (track >= NUM_RADIOS + 1) track -= NUM_RADIOS + 1;

				// On scrolling back we handle this condition on key press. No need to change this.
				if (!DMAudio_IsMP3RadioChannelAvailable() && track == USERTRACK)
					gNumRetunePresses++;
			}
			else
#ifdef RADIO_OFF_TEXT
				track = MusicRadio_GetCarTuning(manager); // gStreamedSound or veh->m_nRadioStation would also work, but these don't cover police/taxi radios
#else
				track = manager->m_nFrontendTrack;
#endif
			uint16_t* string = NULL;
			switch (track) {
			case WILDSTYLE: string = MusicDisplayHost_Text("FEA_FM0"); break;
			case FLASH_FM: string = MusicDisplayHost_Text("FEA_FM1"); break;
			case KCHAT: string = MusicDisplayHost_Text("FEA_FM2"); break;
			case FEVER: string = MusicDisplayHost_Text("FEA_FM3"); break;
			case V_ROCK: string = MusicDisplayHost_Text("FEA_FM4"); break;
			case VCPR: string = MusicDisplayHost_Text("FEA_FM5"); break;
			case RADIO_ESPANTOSO: string = MusicDisplayHost_Text("FEA_FM6"); break;
			case EMOTION: string = MusicDisplayHost_Text("FEA_FM7"); break;
			case WAVE: string = MusicDisplayHost_Text("FEA_FM8"); break;
			case USERTRACK:
				if (!SampleManager_IsMP3RadioChannelAvailable(&SampleManager))
					return;
				string = MusicDisplayHost_Text("FEA_MP3"); break;
#ifdef RADIO_OFF_TEXT
			case RADIO_OFF: {
				// Otherwise RADIO OFF will be seen after pausing-resuming game and Mission Complete text
				if (!manager->m_bRadioStreamReady || !manager->m_bGameplayAllowsRadio)
					return;

				

				string = MusicDisplayHost_Text("FEA_NON");
				if (string == MusicDisplayHost_ErrorText()) {
					pCurrentStation = NULL;
					return;
				}
				break;
			}
#endif
			default: return;
			};

			if (pCurrentStation != string) {
				pCurrentStation = string;
				cDisplay = 60;
			}
			else {
				if (cDisplay == 0) return;
#ifdef FIX_BUGS
				cDisplay -= AudioPoliceHost_LogicalFramesPassed();
#else
				cDisplay--;
#endif
			}

			MusicDisplayFont_JustifyOff();
			MusicDisplayFont_BackgroundOff();
			MusicDisplayFont_Scale(MusicDisplayHost_ScaleX(0.8f), MusicDisplayHost_ScaleY(1.35f));
			MusicDisplayFont_PropOn();
			MusicDisplayFont_Standard();
			MusicDisplayFont_CentreOn();
			MusicDisplayFont_CentreSize(MusicDisplayHost_CentreSize());
			MusicDisplayFont_Color(0, 0, 0, 255);
			MusicDisplayFont_Print(MusicDisplayHost_Width() / 2 + MusicDisplayHost_ScaleX(2.0f), MusicDisplayHost_ScaleY(22.0f) + MusicDisplayHost_ScaleY(2.0f), pCurrentStation);

			if (gNumRetunePresses)
				MusicDisplayFont_Color(102, 133, 143, 255);
			else
				MusicDisplayFont_Color(147, 196, 211, 255);

			MusicDisplayFont_Print(MusicDisplayHost_Width() / 2, MusicDisplayHost_ScaleY(22.0f), pCurrentStation);
			MusicDisplayFont_Draw();
		}
	}
	// Always show station text after entering car. Same behaviour as III and SA.
#ifdef FIX_BUGS
	else
		pCurrentStation = NULL;
#endif
}
//- rouz edit (ChatGPT)
