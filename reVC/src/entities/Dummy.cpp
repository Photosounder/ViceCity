//+ rouz edit (ChatGPT)
#include "common.h"

#include "Pools.h"
#include "World.h"
#include "Dummy.h"

void
CDummy::Add(void)
{
	int x, xstart, xmid, xend;
	int y, ystart, ymid, yend;
	CSector *s;
	CPtrList *list;

	CRect bounds = GetBoundRect();
	xstart = CWorld::GetSectorIndexX(bounds.left);
	xend   = CWorld::GetSectorIndexX(bounds.right);
	xmid   = CWorld::GetSectorIndexX((bounds.left + bounds.right)/2.0f);
	ystart = CWorld::GetSectorIndexY(bounds.top);
	yend   = CWorld::GetSectorIndexY(bounds.bottom);
	ymid   = CWorld::GetSectorIndexY((bounds.top + bounds.bottom)/2.0f);
	assert(xstart >= 0);
	assert(xend < NUMSECTORS_X);
	assert(ystart >= 0);
	assert(yend < NUMSECTORS_Y);

	for(y = ystart; y <= yend; y++)
		for(x = xstart; x <= xend; x++){
			s = CWorld::GetSector(x, y);
			if(x == xmid && y == ymid)
				list = &s->m_lists[ENTITYLIST_DUMMIES];
			else
				list = &s->m_lists[ENTITYLIST_DUMMIES_OVERLAP];
			CPtrNode *node = PtrList_InsertItem(list, this);
			assert(node);
			EntryInfoList_InsertItem(&m_entryInfoList, list, node, s);
		}
}

void
CDummy::Remove(void)
{
	CEntryInfoNode *node, *next;
	for(node = m_entryInfoList.first; node; node = next){
		next = node->next;
		PtrList_DeleteNode(node->list, node->listnode);
		EntryInfoList_DeleteNode(&m_entryInfoList, node);
	}
}

bool
IsDummyPointerValid(CDummy* pDummy)
{
	if (!pDummy)
		return false;
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	int index = CPool_GetJustIndex_NoFreeAssert(CPools::GetDummyPool(), pDummy);
	//- rouz edit (ChatGPT)
#ifdef FIX_BUGS
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	if (index < 0 || index >= CPool_GetSize(CPools::GetDummyPool()))
	//- rouz edit (ChatGPT)
#else
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	if (index < 0 || index > CPool_GetSize(CPools::GetDummyPool()))
	//- rouz edit (ChatGPT)
#endif
		return false;
	return pDummy->m_entryInfoList.first;
}
//- rouz edit (ChatGPT)
