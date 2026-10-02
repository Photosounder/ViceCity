#include "common.h"
#include "Pools.h"
#include "Lists.h"

//+ rouz edit (ChatGPT)
CPtrNode*
CPtrList::InsertItem(void *item)
{
	// Allocate a pointer-list node from its pool without invoking C++ new.
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPtrNode *node = ((CPtrNode*)CPool_New(CPools::GetPtrNodePool()));
	//- rouz edit (ChatGPT)
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
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPool_Delete(CPools::GetPtrNodePool(), node); // rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
}

//+ rouz edit (ChatGPT)
CEntryInfoNode*
CEntryInfoList::InsertItem(CPtrList *list, CPtrNode *listnode, CSector *sect)
{
	// Allocate an entry-info node from its pool without invoking C++ new.
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CEntryInfoNode *node = ((CEntryInfoNode*)CPool_New(CPools::GetEntryInfoNodePool()));
	//- rouz edit (ChatGPT)
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
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	CPool_Delete(CPools::GetEntryInfoNodePool(), node); // rouz edit (ChatGPT)
	//- rouz edit (ChatGPT)
}
