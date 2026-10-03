#pragma once

//+ rouz edit (ChatGPT)
#ifndef assert
#include <assert.h>
#endif
#include <stddef.h>
#include <stdint.h>

typedef struct CLink {
	void *item;
	struct CLink *prev;
	struct CLink *next;
} CLink;

typedef struct CLinkList {
	CLink head, tail;
	CLink freeHead, freeTail;
	CLink *links;
} CLinkList;

typedef struct CSortedLinkItem {
	void *object;
	float sort;
} CSortedLinkItem;

typedef struct CSortedLink {
	CSortedLinkItem item;
	struct CSortedLink *prev;
	struct CSortedLink *next;
} CSortedLink;

typedef struct CSortedLinkList {
	CSortedLink head, tail;
	CSortedLink freeHead, freeTail;
	CSortedLink *links;
} CSortedLinkList;

static inline void
CLink_Insert(CLink *after, CLink *link)
{
	// Splice the node immediately after the supplied link
	link->next = after->next;
	after->next->prev = link;
	link->prev = after;
	after->next = link;
}

static inline void
CLink_Remove(CLink *link)
{
	// Unlink the node without changing its payload
	link->prev->next = link->next;
	link->next->prev = link->prev;
}

static inline void
CLinkList_Init(CLinkList *list, CLink *links, int32_t count)
{
	// Connect the sentinels to caller-owned storage without allocating or constructing entries
	assert(count >= 0 && (count == 0 || links != NULL));
	list->links = links;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
	list->freeHead.next = &list->freeTail;
	list->freeTail.prev = &list->freeHead;
	// Make the lowest array index the first available node
	while(count-- > 0)
		CLink_Insert(&list->freeHead, &links[count]);
}

static inline void
CLinkList_Remove(CLinkList *list, CLink *link)
{
	// Return the active node to the front of the free list
	CLink_Remove(link);
	CLink_Insert(&list->freeHead, link);
}

static inline void
CLinkList_Clear(CLinkList *list)
{
	// Recycle active nodes in their existing order without clearing payloads
	while(list->head.next != &list->tail)
		CLinkList_Remove(list, list->head.next);
}

static inline CLink *
CLinkList_Insert(CLinkList *list, void *item)
{
	// Reserve the first free node and report exhaustion without changing either list
	CLink *node = list->freeHead.next;
	if(node == &list->freeTail)
		return NULL;
	// Copy the payload and move the node to the active list front
	node->item = item;
	CLink_Remove(node);
	CLink_Insert(&list->head, node);
	return node;
}

static inline int32_t
CLinkList_Count(const CLinkList *list)
{
	// Count active nodes without inspecting sentinel payloads
	int32_t count = 0;
	const CLink *node;
	for(node = list->head.next; node != &list->tail; node = node->next)
		count++;
	return count;
}

static inline void
CSortedLink_Insert(CSortedLink *after, CSortedLink *link)
{
	// Splice the node immediately after the supplied link
	link->next = after->next;
	after->next->prev = link;
	link->prev = after;
	after->next = link;
}

static inline void
CSortedLink_Remove(CSortedLink *link)
{
	// Unlink the node without changing its payload
	link->prev->next = link->next;
	link->next->prev = link->prev;
}

static inline void
CSortedLinkList_Init(CSortedLinkList *list, CSortedLink *links, int32_t count)
{
	// Connect the sentinels to caller-owned storage without allocating or constructing entries
	assert(count >= 0 && (count == 0 || links != NULL));
	list->links = links;
	list->head.next = &list->tail;
	list->tail.prev = &list->head;
	list->freeHead.next = &list->freeTail;
	list->freeTail.prev = &list->freeHead;
	// Make the lowest array index the first available node
	while(count-- > 0)
		CSortedLink_Insert(&list->freeHead, &links[count]);
}

static inline void
CSortedLinkList_Remove(CSortedLinkList *list, CSortedLink *link)
{
	// Return the active node to the front of the free list
	CSortedLink_Remove(link);
	CSortedLink_Insert(&list->freeHead, link);
}

static inline void
CSortedLinkList_Clear(CSortedLinkList *list)
{
	// Recycle active nodes in their existing order without clearing payloads
	while(list->head.next != &list->tail)
		CSortedLinkList_Remove(list, list->head.next);
}

static inline CSortedLink *
CSortedLinkList_Insert(CSortedLinkList *list, const CSortedLinkItem *item)
{
	// Reserve the first free node and report exhaustion without changing either list
	CSortedLink *node = list->freeHead.next;
	if(node == &list->freeTail)
		return NULL;
	// Copy the payload and move the node to the active list front
	node->item = *item;
	CSortedLink_Remove(node);
	CSortedLink_Insert(&list->head, node);
	return node;
}

static inline int32_t
CSortedLinkList_Count(const CSortedLinkList *list)
{
	// Count active nodes without inspecting sentinel payloads
	int32_t count = 0;
	const CSortedLink *node;
	for(node = list->head.next; node != &list->tail; node = node->next)
		count++;
	return count;
}

static inline CSortedLink *
CSortedLinkList_InsertSorted(CSortedLinkList *list, const CSortedLinkItem *item)
{
	// Find the first greater or equal key using the original floating-point comparison
	CSortedLink *position;
	for(position = list->head.next; position != &list->tail; position = position->next)
		if(position->item.sort >= item->sort)
			break;
	// Leave the list intact if its caller-owned node array is exhausted
	CSortedLink *node = list->freeHead.next;
	if(node == &list->freeTail)
		return NULL;
	// Insert before equal keys and append NaN keys just as the original list did
	node->item = *item;
	CSortedLink_Remove(node);
	CSortedLink_Insert(position->prev, node);
	return node;
}
//- rouz edit (ChatGPT)
