//+ rouz edit (ChatGPT)
#pragma once

#include "Object.h"

struct CutsceneShadow;

class CCutsceneObject : public CObject
{
public:
	CutsceneShadow *m_pShadow;
	void     *m_pAttachTo;
	CObject  *m_pAttachmentObject;
  
	CCutsceneObject(void);
	~CCutsceneObject(void);

	void SetModelIndex(uint32 id);
	void CreateShadow(void);
	void ProcessControl(void);
	void PreRender(void);
	void Render(void);
	bool SetupLighting(void);
	void RemoveLighting(bool reset);
};
//- rouz edit (ChatGPT)
