//+ rouz edit (ChatGPT)
#include "common.h"

#include "ModelInfo.h"
#include "AnimManager.h"
#include "VisibilityPlugins.h"

void
CWeaponModelInfo::SetAnimFile(const char *file)
{
    // Apply the shared C animation-file policy
    ModelAnimFile_Set(&m_animFile,file);
}

void
CWeaponModelInfo::ConvertAnimFileIndex(void)
{
    // Resolve the shared C animation-file state
    ModelAnimFile_Convert(&m_animFile);
}

void
CWeaponModelInfo::Init(void)
{
	CSimpleModelInfo::Init();
	// Initialize the encoded weapon identifier after the base model
	m_atomics[2] = (RpAtomic*)0;
}

void
CWeaponModelInfo::SetAtomic(int n, RpAtomic *atomic)
{
	CSimpleModelInfo::SetAtomic(n, atomic);
	CVisibilityPlugins::SetAtomicRenderCallback(atomic, CVisibilityPlugins::RenderWeaponCB);
}
//- rouz edit (ChatGPT)
