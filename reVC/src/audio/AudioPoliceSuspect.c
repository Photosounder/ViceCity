//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "AudioPoliceGame.h"
#include "sampman.h"
#include "../modelinfo/ModelIds.h"
#include <stddef.h>
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#ifdef __MWERKS__
void debug(char *format, ...);
#define AUDIO_POLICE_DEBUG(format, ...) debug(format, __VA_ARGS__)
#else
void re3_debug(const char *format, ...);
#define AUDIO_POLICE_DEBUG(format, ...) re3_debug("[DBG]: " format, __VA_ARGS__)
#endif

static const uint32_t gCarColourTable[][3] = {
	{NO_SAMPLE, SFX_POLICE_RADIO_BLACK, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_WHITE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_PURPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_YELLOW, NO_SAMPLE},
	{SFX_POLICE_RADIO_BRIGHT, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, SFX_POLICE_RADIO_BLUE, SFX_POLICE_RADIO_GREY},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_RED, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_ORANGE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_ORANGE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_ORANGE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_ORANGE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_ORANGE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_ORANGE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_YELLOW, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_YELLOW, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_YELLOW, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_YELLOW, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_YELLOW, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_YELLOW, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_GREEN, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_GREEN, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_GREEN, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_GREEN, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_GREEN, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_GREEN, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_BLUE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_PURPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_PURPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_PURPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_PURPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_PURPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_PURPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_SILVER, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_SILVER, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_SILVER, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_SILVER, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_SILVER, NO_SAMPLE},
	{NO_SAMPLE, SFX_POLICE_RADIO_SILVER, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_LIGHT, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE},
	{SFX_POLICE_RADIO_DARK, NO_SAMPLE, NO_SAMPLE}
};

void
AudioPolice_SuspectReport(cAudioManager *manager)
{
	// Preserve the original vehicle and foot report gates and queue word order
	CVehicle *veh;
	uint8_t color1;
	uint32_t main_color;
	uint32_t sample;

	uint32_t color_pre_modifier;
	uint32_t color_post_modifier;

	if (AudioPoliceHost_MusicMode() != MUSICMODE_CUTSCENE) {
		veh = AudioGame_FindVehicle();
		if (veh != NULL) {
			if (POLICE_RADIO_QUEUE_MAX_SAMPLES - manager->m_sPoliceRadioQueue.m_nSamplesInQueue > 9) {
				color1 = AudioPoliceHost_VehicleColor(veh);
				if (color1 >= ARRAY_SIZE(gCarColourTable)) {
					AUDIO_POLICE_DEBUG("\n *** UNKNOWN CAR COLOUR %d *** ", color1);
				} else {
					main_color = gCarColourTable[color1][1];
					color_pre_modifier = gCarColourTable[color1][0];
					color_post_modifier = gCarColourTable[color1][2];
					switch (AudioPoliceHost_VehicleModel(veh)) {
					case MI_LANDSTAL:
					case MI_PATRIOT:
					case MI_RANCHER:
					case MI_FBIRANCH:
					case MI_SANDKING:
						sample = SFX_POLICE_RADIO_OFFROAD;
						break;
					case MI_IDAHO:
					case MI_MANANA:
					case MI_ESPERANT:
					case MI_CUBAN:
					case MI_STALLION:
					case MI_SABRE:
					case MI_SABRETUR:
					case MI_VIRGO:
					case MI_BLISTAC:
							sample = SFX_POLICE_RADIO_TUDOOR;
							break;
					case MI_STINGER:
					case MI_INFERNUS:
					case MI_CHEETAH:
					case MI_BANSHEE:
					case MI_PHEONIX:
					case MI_COMET:
					case MI_DELUXO:
					case MI_HOTRING:
						sample = SFX_POLICE_RADIO_SPORTS_CAR;
						break;
					case MI_LINERUN:
						sample = SFX_POLICE_RADIO_RIG;
						break;
					case MI_PEREN:
					case MI_REGINA:
						sample = SFX_POLICE_RADIO_STATION_WAGON;
						break;
					case MI_SENTINEL:
					case MI_FBICAR:
					case MI_WASHING:
					case MI_SENTXS:
					case MI_ADMIRAL:
					case MI_GLENDALE:
					case MI_OCEANIC:
					case MI_HERMES:
					case MI_GREENWOO:
						sample = SFX_POLICE_RADIO_SEDAN;
						break;
					case MI_RIO:
						sample = SFX_POLICE_RADIO_CRUISER;
						break;
					case MI_FIRETRUCK:
						sample = SFX_POLICE_RADIO_FIRE_TRUCK;
						break;
					case MI_TRASH:
						sample = SFX_POLICE_RADIO_GARBAGE_TRUCK;
						break;
					case MI_STRETCH:
					case MI_LOVEFIST:
						sample = SFX_POLICE_RADIO_STRETCH;
						break;
					case MI_VOODOO:
						sample = SFX_POLICE_RADIO_LOWRIDER;
						break;
					case MI_PONY:
					case MI_MOONBEAM:
					case MI_SECURICA:
					case MI_RUMPO:
					case MI_GANGBUR:
					case MI_YANKEE:
					case MI_TOPFUN:
					case MI_BURRITO:
					case MI_SPAND:
						sample = SFX_POLICE_RADIO_VAN;
						break;
					case MI_MULE:
					case MI_BARRACKS:
					case MI_PACKER:
					case MI_FLATBED:
						sample = SFX_POLICE_RADIO_TRUCK;
						break;
					case MI_AMBULAN:
						sample = SFX_POLICE_RADIO_AMBULANCE;
						break;
					case MI_TAXI:
					case MI_CABBIE:
					case MI_ZEBRA:
					case MI_KAUFMAN:
						sample = SFX_POLICE_RADIO_TAXI;
						break;
					case MI_BOBCAT:
					case MI_WALTON:
						sample = SFX_POLICE_RADIO_PICKUP;
						break;
					case MI_MRWHOOP:
						sample = SFX_POLICE_RADIO_ICE_CREAM_VAN;
						break;
					case MI_BFINJECT:
						sample = SFX_POLICE_RADIO_BUGGY;
						break;
					case MI_HUNTER:
					case MI_CHOPPER:
					case MI_SEASPAR:
					case MI_SPARROW:
					case MI_MAVERICK:
					case MI_VCNMAV:
					case MI_POLMAV:
						sample = SFX_POLICE_RADIO_HELICOPTER;
						break;
					case MI_POLICE:
						sample = SFX_POLICE_RADIO_POLICE_CAR;
						break;
					case MI_ENFORCER:
						sample = SFX_POLICE_RADIO_SWAT_VAN;
						break;
					case MI_PREDATOR:
					case MI_SQUALO:
					case MI_SPEEDER:
						sample = SFX_POLICE_RADIO_SPEEDBOAT;
						break;
					case MI_BUS:
						sample = SFX_POLICE_RADIO_BUS;
						break;
					case MI_RHINO:
						sample = SFX_POLICE_RADIO_TANK;
						break;
					case MI_ANGEL:
					case MI_PCJ600:
					case MI_FREEWAY:
					case MI_SANCHEZ:
						sample = SFX_POLICE_RADIO_MOTOBIKE;
						break;
					case MI_COACH:
						sample = SFX_POLICE_RADIO_COACH;
						break;
					case MI_ROMERO:
						sample = SFX_POLICE_RADIO_HEARSE;
						break;
					case MI_PIZZABOY:
					case MI_FAGGIO:
						sample = SFX_POLICE_RADIO_MOPED;
						break;
					case MI_DEADDODO:
					case MI_SKIMMER:
						sample = SFX_POLICE_RADIO_PLANE;
						break;
					case MI_REEFER:
					case MI_TROPIC:
					case MI_COASTG:
					case MI_MARQUIS:
					case MI_JETMAX:
						sample = SFX_POLICE_RADIO_BOAT;
						break;
					case MI_CADDY:
						sample = SFX_POLICE_RADIO_GOLF_CART;
						break;
					case MI_DINGHY:
						sample = SFX_POLICE_RADIO_DINGHY;
						break;
					default:
						//AUDIO_POLICE_DEBUG("\n *** UNKNOWN CAR MODEL INDEX %d *** ", AudioPoliceHost_VehicleModel(veh));
						return;
					}
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_SUSPECT);
					if (manager->m_anRandomTable[3] % 2) 
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_LAST_SEEN);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_IN_A);
					if (color_pre_modifier != NO_SAMPLE)
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, color_pre_modifier);
					if (main_color != NO_SAMPLE)
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, main_color);
					if (color_post_modifier != NO_SAMPLE)
						PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, color_post_modifier);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, sample);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
					PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, NO_SAMPLE);
				}
			}
		} else if (POLICE_RADIO_QUEUE_MAX_SAMPLES - manager->m_sPoliceRadioQueue.m_nSamplesInQueue > 4) {
			PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
			PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_SUSPECT);
			PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_ON_FOOT);
			PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, SFX_POLICE_RADIO_MESSAGE_NOISE_1);
			PoliceRadioQueue_Add(&manager->m_sPoliceRadioQueue, NO_SAMPLE);
		}
	}
}
//- rouz edit (ChatGPT)
