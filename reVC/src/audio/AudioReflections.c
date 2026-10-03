//+ rouz edit (ChatGPT)
#include "AudioManager.h"
#include <math.h>
#ifdef AUDIO_REFLECTIONS
static float AudioReflections_Distance(const AudioSoundPosition *start, const AudioSoundPosition *hit)
{
    // Retain the original vector difference and unclamped magnitude formula
    float x = hit->x - start->x;
    float y = hit->y - start->y;
    float z = hit->z - start->z;
    return sqrtf(x*x + y*y + z*z);
}

void
AudioReflections_Update(cAudioManager *manager)
{
	// Preserve the original version schedule, endpoint writes and probe result timing
	AudioSoundPosition camPos;
	AudioSoundPosition hit;
	float hitZ;

#if GTA_VERSION < GTAVC_PC_10
	if (manager->m_FrameCounter % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[0] = camPos;
		manager->m_avecReflectionsPos[0].y += 50.0f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[0], &hit))
			manager->m_afReflectionsDistances[0] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[0] = 50.0f;
	} else if ((manager->m_FrameCounter + 1) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[1] = camPos;
		manager->m_avecReflectionsPos[1].y -= 50.0f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[1], &hit))
			manager->m_afReflectionsDistances[1] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[1] = 50.0f;
	} else if ((manager->m_FrameCounter + 2) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[2] = camPos;
		manager->m_avecReflectionsPos[2].x -= 50.0f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[2], &hit))
			manager->m_afReflectionsDistances[2] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[2] = 50.0f;
	} else if ((manager->m_FrameCounter + 3) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[3] = camPos;
		manager->m_avecReflectionsPos[3].x += 50.0f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[3], &hit))
			manager->m_afReflectionsDistances[3] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[3] = 50.0f;
	} else if ((manager->m_FrameCounter + 4) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[4] = camPos;
		manager->m_avecReflectionsPos[4].z += 50.0f;
		if (AudioReflectionsHost_Vertical(&camPos, manager->m_avecReflectionsPos[4].z, &hitZ))
			manager->m_afReflectionsDistances[4] = hitZ - camPos.z;
		else
			manager->m_afReflectionsDistances[4] = 50.0f;
	}
#else
	if (manager->m_FrameCounter % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[0] = camPos;
		manager->m_avecReflectionsPos[0].y += 100.f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[0], &hit))
			manager->m_afReflectionsDistances[0] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[0] = 100.0f;
	} else if ((manager->m_FrameCounter + 1) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[1] = camPos;
		manager->m_avecReflectionsPos[1].y -= 100.0f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[1], &hit))
			manager->m_afReflectionsDistances[1] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[1] = 100.0f;
	} else if ((manager->m_FrameCounter + 2) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[2] = camPos;
		manager->m_avecReflectionsPos[2].x -= 100.0f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[2], &hit))
			manager->m_afReflectionsDistances[2] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[2] = 100.0f;
	} else if ((manager->m_FrameCounter + 3) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[3] = camPos;
		manager->m_avecReflectionsPos[3].x += 100.0f;
		if (AudioReflectionsHost_Line(&camPos, &manager->m_avecReflectionsPos[3], &hit))
			manager->m_afReflectionsDistances[3] = AudioReflections_Distance(&camPos, &hit);
		else
			manager->m_afReflectionsDistances[3] = 100.0f;
	} else if ((manager->m_FrameCounter + 4) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		camPos.y += 1.0f;
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[4] = camPos;
		manager->m_avecReflectionsPos[4].z += 100.0f;
		if (AudioReflectionsHost_Vertical(&camPos, manager->m_avecReflectionsPos[4].z, &hitZ))
			manager->m_afReflectionsDistances[4] = hitZ - camPos.z;
		else
			manager->m_afReflectionsDistances[4] = 100.0f;
	} else if ((manager->m_FrameCounter + 5) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		camPos.y -= 1.0f;
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[5] = camPos;
		manager->m_avecReflectionsPos[5].z += 100.0f;
		if (AudioReflectionsHost_Vertical(&camPos, manager->m_avecReflectionsPos[5].z, &hitZ))
			manager->m_afReflectionsDistances[5] = hitZ - camPos.z;
		else
			manager->m_afReflectionsDistances[5] = 100.0f;
	} else if ((manager->m_FrameCounter + 6) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		camPos.x -= 1.0f;
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[6] = camPos;
		manager->m_avecReflectionsPos[6].z += 100.0f;
		if (AudioReflectionsHost_Vertical(&camPos, manager->m_avecReflectionsPos[6].z, &hitZ))
			manager->m_afReflectionsDistances[6] = hitZ - camPos.z;
		else
			manager->m_afReflectionsDistances[6] = 100.0f;
	} else if ((manager->m_FrameCounter + 7) % 8 == 0) {
		AudioGeometryHost_CameraPosition(&camPos);
		camPos.x += 1.0f;
		// Copy the camera coordinates into plain reflection position storage
		manager->m_avecReflectionsPos[7] = camPos;
		manager->m_avecReflectionsPos[7].z += 100.0f;
		if (AudioReflectionsHost_Vertical(&camPos, manager->m_avecReflectionsPos[7].z, &hitZ))
			manager->m_afReflectionsDistances[7] = hitZ - camPos.z;
		else
			manager->m_afReflectionsDistances[7] = 100.0f;
	}
#endif
}
#endif
//- rouz edit (ChatGPT)
