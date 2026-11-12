//+ rouz edit (Claude)
#pragma once
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif
// Move an element from one doubly linked list to the front of another, the lists are given by their head pointers and the links of an element sit at the given byte offsets from it
void DList_MoveToList(void **from, void **to, void *obj, size_t prevOffset, size_t nextOffset);
// Take an element out of the list that starts at head
void DList_Remove(void **head, void *obj, size_t prevOffset, size_t nextOffset);
// Put an element at the front of the list that starts at head
void DList_PushFront(void **head, void *obj, size_t prevOffset, size_t nextOffset);
// Put a whole chain of singly linked elements in front of a list, the chain is left empty, next is the offset of the link in an element
void SList_MoveChainToFront(void **chain, void **list, size_t nextOffset);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
