//+ rouz edit (ChatGPT)
#include "common.h"

#include "Pools.h"
#include "World.h"
#include "Dummy.h"
#include "DummyLogic.h"

void
CDummy::Add(void)
{
	// Delegate to the C implementation
	Dummy_Add(this, this, this, World_GetSector(0, 0), &m_entryInfoList);
}

void
CDummy::Remove(void)
{
	// Delegate to the C implementation
	Dummy_Remove(&m_entryInfoList);
}

bool
IsDummyPointerValid(CDummy* pDummy)
{
	if (!pDummy)
		return false;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int index = CPool_GetJustIndex_NoFreeAssert(Pools_GetDummyPool(), pDummy);
	//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	if (index < 0 || index >= CPool_GetSize(Pools_GetDummyPool()))
	//- rouz edit (ChatGPT)
#else
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	if (index < 0 || index > CPool_GetSize(Pools_GetDummyPool()))
	//- rouz edit (ChatGPT)
#endif
		return false;
	return pDummy->m_entryInfoList.first;
}
//- rouz edit (ChatGPT)
