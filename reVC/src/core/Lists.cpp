#include "common.h"
#include "Pools.h"
#include "Lists.h"

//+ rouz edit (ChatGPT)
CPtrNode*
CPtrList::InsertItem(void *item)
{
	// Allocate a pointer-list node from its pool without invoking C++ new.
	CPtrNode *node = CPools::GetPtrNodePool()->New();
	assert(node);
	node->item = item;
	InsertNode(node);
//- rouz edit (ChatGPT)
	return node;
}

void
//+ rouz edit (ChatGPT)
CPtrList::DeleteNode(CPtrNode *node)
{
	RemoveNode(node);
//- rouz edit (ChatGPT)
	CPools::GetPtrNodePool()->Delete(node); // rouz edit (ChatGPT)
}

//+ rouz edit (ChatGPT)
CEntryInfoNode*
CEntryInfoList::InsertItem(CPtrList *list, CPtrNode *listnode, CSector *sect)
{
	// Allocate an entry-info node from its pool without invoking C++ new.
	CEntryInfoNode *node = CPools::GetEntryInfoNodePool()->New();
	assert(node);
	node->list = list;
	node->listnode = listnode;
	node->sector = sect;
	InsertNode(node);
//- rouz edit (ChatGPT)
	return node;
}

void
//+ rouz edit (ChatGPT)
CEntryInfoList::DeleteNode(CEntryInfoNode *node)
{
	RemoveNode(node);
//- rouz edit (ChatGPT)
	CPools::GetEntryInfoNodePool()->Delete(node); // rouz edit (ChatGPT)
}
