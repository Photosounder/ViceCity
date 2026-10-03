#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef LINK_PREDEFINED_ASSERT
#undef assert
#define assert(condition) ((condition) ? (void)0 : abort())
#endif

#include "../../src/core/LinkList.h"

static void check_pointer_partition(const CLinkList *list, const CLink *storage, int capacity)
{
	// Check that every allocated node belongs to exactly one correctly linked chain
	unsigned char seen[32] = {0};
	const CLink *heads[2] = {&list->head, &list->freeHead};
	const CLink *tails[2] = {&list->tail, &list->freeTail};
	int visited = 0;
	int chain;
	assert(capacity <= 32);
	for(chain = 0; chain < 2; chain++) {
		// Verify links in both directions and reject repeated or foreign nodes
		const CLink *previous = heads[chain];
		const CLink *node;
		for(node = previous->next; node != tails[chain]; node = node->next) {
			// Track this node's index without reading its object payload
			ptrdiff_t index = node - storage;
			assert(index >= 0 && index < capacity);
			assert(!seen[index]);
			seen[index] = 1;
			assert(node->prev == previous);
			previous = node;
			visited++;
		}
		assert(tails[chain]->prev == previous);
	}
	assert(visited == capacity);
}

static void test_pointer_list(void)
{
	// Allocate nodes directly and verify their initial order and exhaustion behavior
	CLinkList list;
	CLink *storage = (CLink*)calloc(4, sizeof(CLink));
	int objects[5] = {0};
	int i;
	memset(&list, 0, sizeof(list));
	assert(storage != NULL);
	CLinkList_Init(&list, storage, 4);
	for(i = 0; i < 4; i++) {
		// Each insertion consumes the lowest remaining array index and prepends it
		CLink *node = CLinkList_Insert(&list, &objects[i]);
		assert(node == &storage[i]);
		assert(node->item == &objects[i]);
		assert(list.head.next == node);
		check_pointer_partition(&list, storage, 4);
	}
	assert(CLinkList_Count(&list) == 4);
	assert(CLinkList_Insert(&list, &objects[4]) == NULL);
	assert(list.tail.prev == &storage[0]);
	// Move an existing cache entry to the front without consuming a free node
	CLink_Remove(&storage[0]);
	CLink_Insert(&list.head, &storage[0]);
	assert(list.head.next == &storage[0]);
	assert(CLinkList_Count(&list) == 4);
	check_pointer_partition(&list, storage, 4);
	// Reuse a removed node first while keeping all other node addresses stable
	CLinkList_Remove(&list, &storage[2]);
	assert(CLinkList_Insert(&list, &objects[4]) == &storage[2]);
	assert(storage[2].item == &objects[4]);
	check_pointer_partition(&list, storage, 4);
	// Clearing reverses active order into free order and retains old payloads
	CLinkList_Clear(&list);
	assert(CLinkList_Count(&list) == 0);
	assert(list.freeHead.next == &storage[1]);
	assert(storage[2].item == &objects[4]);
	assert(CLinkList_Insert(&list, NULL) == &storage[1]);
	assert(storage[1].item == NULL);
	check_pointer_partition(&list, storage, 4);
	// Free caller-owned storage and leave a valid empty reusable manager
	free(storage);
	CLinkList_Init(&list, NULL, 0);
	CLinkList_Clear(&list);
	assert(CLinkList_Count(&list) == 0);
	assert(CLinkList_Insert(&list, &objects[0]) == NULL);
}

static void test_sorted_list(void)
{
	// Cover equal keys, signed zeros, infinities and NaNs with distinct objects
	CSortedLinkList list;
	CSortedLink *storage = (CSortedLink*)calloc(8, sizeof(CSortedLink));
	int objects[9] = {0};
	CSortedLinkItem items[9] = {
		{&objects[0], 2.0f}, {&objects[1], 1.0f}, {&objects[2], 2.0f},
		{&objects[3], -INFINITY}, {&objects[4], INFINITY}, {&objects[5], NAN},
		{&objects[6], 0.0f}, {&objects[7], -0.0f}, {&objects[8], 3.0f}
	};
	const int expected[8] = {3, 7, 6, 1, 2, 0, 4, 5};
	CSortedLink *node;
	int i;
	memset(&list, 0, sizeof(list));
	assert(storage != NULL);
	CSortedLinkList_Init(&list, storage, 8);
	for(i = 0; i < 8; i++) {
		// Consume each slot once while inserting before equal keys and appending NaNs
		assert(CSortedLinkList_InsertSorted(&list, &items[i]) == &storage[i]);
	}
	assert(CSortedLinkList_InsertSorted(&list, &items[8]) == NULL);
	assert(CSortedLinkList_Count(&list) == 8);
	for(node = list.head.next, i = 0; node != &list.tail; node = node->next, i++) {
		// Observe forward and backward ordering with no sentinel key assumptions
		assert(i < 8);
		assert(node->item.object == &objects[expected[i]]);
		assert(node->next->prev == node);
		assert(node->prev->next == node);
	}
	assert(i == 8);
	assert(list.tail.prev->item.object == &objects[5]);
	// Verify sorted removal, unsorted prepend and clear reuse preserve payloads
	CSortedLinkList_Remove(&list, &storage[2]);
	assert(CSortedLinkList_Insert(&list, &items[8]) == &storage[2]);
	assert(list.head.next == &storage[2]);
	CSortedLinkList_Clear(&list);
	assert(CSortedLinkList_Count(&list) == 0);
	assert(list.freeHead.next == &storage[5]);
	assert(storage[2].item.object == &objects[8]);
	assert(CSortedLinkList_InsertSorted(&list, &items[0]) == &storage[5]);
	// Release the array without deleting any referenced objects
	free(storage);
	CSortedLinkList_Init(&list, NULL, 0);
	CSortedLinkList_Clear(&list);
	assert(CSortedLinkList_Count(&list) == 0);
	assert(CSortedLinkList_InsertSorted(&list, &items[0]) == NULL);
}

int main(void)
{
	// Exercise both list types through the same standalone C and C++ header
	test_pointer_list();
	test_sorted_list();
	return 0;
}
