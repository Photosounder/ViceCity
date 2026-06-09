#include "common.h"

#include "ModelInfo.h"
#include "AnimManager.h"
#include "VisibilityPlugins.h"

void
CWeaponModelInfo::SetAnimFile(const char *file)
{
	if(strcasecmp(file, "null") == 0)
		return;

	m_animFileName = (char*)malloc(strlen(file)+1); // rouz edit (ChatGPT)
	strcpy(m_animFileName, file);
}

void
CWeaponModelInfo::ConvertAnimFileIndex(void)
{
	if(m_animFileIndex != -1){
		// we have a string pointer in that union
		int32 index = CAnimManager::GetAnimationBlockIndex(m_animFileName);
		free(m_animFileName); // rouz edit (ChatGPT)
		m_animFileIndex = index;
	}
}

void
CWeaponModelInfo::Init(void)
{
	CSimpleModelInfo::Init();
	SetWeaponInfo(0);
}

void
CWeaponModelInfo::SetWeaponInfo(int32 weaponId)
{
	m_atomics[2] = (RpAtomic*)weaponId;
}

eWeaponType
CWeaponModelInfo::GetWeaponInfo(void)
{
	return (eWeaponType)(uintptr)m_atomics[2];
}

void
CWeaponModelInfo::SetAtomic(int n, RpAtomic *atomic)
{
	CSimpleModelInfo::SetAtomic(n, atomic);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, CVisibilityPlugins::RenderWeaponCB);
}
