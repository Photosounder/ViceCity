#pragma once
#include "ModelAnimFile.h" // rouz edit (ChatGPT)

#include "SimpleModelInfo.h"
#include "WeaponType.h"

class CWeaponModelInfo : public CSimpleModelInfo
{
	ModelAnimFile m_animFile; // rouz edit (ChatGPT)
public:
	CWeaponModelInfo(void) : CSimpleModelInfo(MITYPE_WEAPON) { m_animFile.index = -1; } // rouz edit (ChatGPT)

	virtual void SetAnimFile(const char *file);
	virtual void ConvertAnimFileIndex(void);
	virtual int GetAnimFileIndex(void) { return m_animFile.index; } // rouz edit (ChatGPT)
	virtual void SetAtomic(int n, RpAtomic *atomic);
	
	void Init(void);
};
