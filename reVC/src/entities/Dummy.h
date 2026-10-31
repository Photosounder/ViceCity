//+ rouz edit (ChatGPT)
#pragma once

#include "Lists.h"
#include "Entity.h"

class CDummy : public CEntity
{
public:
	CEntryInfoList m_entryInfoList;

	CDummy(void) {
        // Initialize sector membership before assigning the dummy type
        EntryInfoList_Init(&m_entryInfoList);
        m_type = ENTITY_TYPE_DUMMY;
    }
    ~CDummy(void) {
        // Release membership nodes before the entity base destructor
        EntryInfoList_Destroy(&m_entryInfoList);
    }
	void Add(void);
	void Remove(void);
};

bool IsDummyPointerValid(CDummy* pDummy);
//- rouz edit (ChatGPT)
