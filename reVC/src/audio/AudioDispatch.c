//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include "../entities/EntityTypes.h"
#include "GameAreas.h"

#ifdef AUDIO_REVERB
#define AUDIO_DISPATCH_REVERB(b) manager->m_sQueueSample.m_bReverb = b
#else
#define AUDIO_DISPATCH_REVERB(b)
#endif

void
AudioDispatch_Entity(cAudioManager *manager, int32_t id)
{
	// Preserve entity routing gates and live owner state reads
	if (manager->m_asAudioEntities[id].m_bStatus) {
		manager->m_sQueueSample.m_nEntityIndex = id;
		switch (manager->m_asAudioEntities[id].m_nType) {
		case AUDIOTYPE_PHYSICAL:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatch_Physical(manager, id);
			}
			break;
		case AUDIOTYPE_EXPLOSION:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessExplosions(manager, id);
			}
			break;
		case AUDIOTYPE_FIRE:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessFires(manager, id);
			}
			break;
		case AUDIOTYPE_WEATHER:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				if(AudioDispatchHost_Area() == AREA_MAIN_MAP || AudioDispatchHost_Area() == AREA_EVERYWHERE)
					AudioDispatchHost_ProcessWeather(manager, id);
			}
			break;
/*		case AUDIOTYPE_CRANE:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				ProcessCrane();
			}
			break;*/
		case AUDIOTYPE_SCRIPTOBJECT:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessScriptObject(manager, id);
			}
			break;
#ifdef GTA_BRIDGE
		case AUDIOTYPE_BRIDGE:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessBridge(manager);
			}
			break;
#endif
		case AUDIOTYPE_FRONTEND:
			AUDIO_DISPATCH_REVERB(0);
			AudioDispatchHost_ProcessFrontEnd(manager);
			break;
		case AUDIOTYPE_PROJECTILE:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessProjectiles(manager);
			}
			break;
		case AUDIOTYPE_GARAGE:
			if (!manager->m_bIsPaused)
				AudioDispatchHost_ProcessGarages(manager);
			break;
		case AUDIOTYPE_FIREHYDRANT:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessFireHydrant(manager);
			}
			break;
		case AUDIOTYPE_WATERCANNON:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessWaterCannon(manager, id);
			}
			break;
		case AUDIOTYPE_ESCALATOR:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessEscalators(manager);
			}
			break;
		case AUDIOTYPE_EXTRA_SOUNDS:
			if (!manager->m_bIsPaused) {
				AUDIO_DISPATCH_REVERB(1);
				AudioDispatchHost_ProcessExtraSounds(manager);
			}
			break;
		default:
			return;
		}
	}
}

void
AudioDispatch_Physical(cAudioManager *manager, int32_t id)
{
	// Preserve entity routing gates and live owner state reads
	void *entity = manager->m_asAudioEntities[id].m_pEntity;
	if (entity) {
		switch (AudioDispatchHost_PhysicalType(entity)) {
		case ENTITY_TYPE_VEHICLE:
			AudioDispatchHost_Vehicle(manager, manager->m_asAudioEntities[id].m_pEntity);
			break;
		case ENTITY_TYPE_PED:
			AudioDispatchHost_Ped(manager, manager->m_asAudioEntities[id].m_pEntity);
			break;
		default:
			return;
		}
	}
}

#undef AUDIO_DISPATCH_REVERB
//- rouz edit (ChatGPT)
