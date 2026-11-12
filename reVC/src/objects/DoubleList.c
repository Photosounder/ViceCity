//+ rouz edit (Claude)
#include "DoubleList.h"

#define LINK(o, off) (*(void**)((char*)(o) + (off)))

void DList_MoveToList(void **from, void **to, void *obj, size_t prevOffset, size_t nextOffset)
{
	// Take the element out of its list, the head moves on when it was the first one
	if (LINK(obj, prevOffset) == NULL) {
		*from = LINK(obj, nextOffset);
		if (*from)
			LINK(*from, prevOffset) = NULL;
	} else {
		if (LINK(obj, nextOffset) == NULL)
			LINK(LINK(obj, prevOffset), nextOffset) = NULL;
		else {
			LINK(LINK(obj, nextOffset), prevOffset) = LINK(obj, prevOffset);
			LINK(LINK(obj, prevOffset), nextOffset) = LINK(obj, nextOffset);
		}
	}

	// Put it in front of the other list
	LINK(obj, nextOffset) = *to;
	LINK(obj, prevOffset) = NULL;
	*to = obj;

	if (LINK(obj, nextOffset))
		LINK(LINK(obj, nextOffset), prevOffset) = obj;
}
void DList_Remove(void **head, void *obj, size_t prevOffset, size_t nextOffset)
{
	// Link the neighbours to each other, the head moves on when it was the first one
	if (LINK(obj, prevOffset))
		LINK(LINK(obj, prevOffset), nextOffset) = LINK(obj, nextOffset);
	else
		*head = LINK(obj, nextOffset);
	if (LINK(obj, nextOffset))
		LINK(LINK(obj, nextOffset), prevOffset) = LINK(obj, prevOffset);
}

void DList_PushFront(void **head, void *obj, size_t prevOffset, size_t nextOffset)
{
	// The old first element follows the new one
	LINK(obj, nextOffset) = *head;
	LINK(obj, prevOffset) = NULL;
	if (*head)
		LINK(*head, prevOffset) = obj;
	*head = obj;
}
void SList_MoveChainToFront(void **chain, void **list, size_t nextOffset)
{
	// Nothing to move for an empty chain
	if (*chain != NULL) {
		void *oldFront = *list;
		void *last = *chain;

		// The list follows the chain and the chain becomes the front
		*list = *chain;
		while (LINK(last, nextOffset) != NULL)
			last = LINK(last, nextOffset);
		LINK(last, nextOffset) = oldFront;
	}
	*chain = NULL;
}
//- rouz edit (Claude)
