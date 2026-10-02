#pragma once

//+ rouz edit (ChatGPT)
#include "Store.h"
#include "Pool.h"
//- rouz edit (ChatGPT)

template<typename T>
class CLink
{
public:
	T item;
	CLink<T> *prev;
	CLink<T> *next;

	void Insert(CLink<T> *link){
		link->next = this->next;
		this->next->prev = link;
		link->prev = this;
		this->next = link;
	}
	void Remove(void){
		this->prev->next = this->next;
		this->next->prev = this->prev;
	}
};

template<typename T>
class CLinkList
{
public:
	CLink<T> head, tail;
	CLink<T> freeHead, freeTail;
	CLink<T> *links;

	void Init(int n){
//+ rouz edit (ChatGPT)
		// Allocate and construct links without invoking C++ array new.
		links = (CLink<T>*)malloc(sizeof(CLink<T>)*n);
		for(int i = 0; i < n; i++)
			std::allocator<CLink<T> >().construct(&links[i]);
//- rouz edit (ChatGPT)
		head.next = &tail;
		tail.prev = &head;
		freeHead.next = &freeTail;
		freeTail.prev = &freeHead;
		while(n--)
			freeHead.Insert(&links[n]);
	}
	void Shutdown(void){
//+ rouz edit (ChatGPT)
		// Destroy and release links without invoking C++ array delete.
		int count = 0;
		for(CLink<T> *link = freeHead.next; link != &freeTail; link = link->next)
			count++;
		for(CLink<T> *link = head.next; link != &tail; link = link->next)
			count++;
		for(int i = 0; i < count; i++)
			std::allocator<CLink<T> >().destroy(&links[i]);
		free(links);
//- rouz edit (ChatGPT)
		links = nil;
	}
	void Clear(void){
		while(head.next != &tail)
			Remove(head.next);
	}
	CLink<T> *Insert(T const &item){
		CLink<T> *node = freeHead.next;
		if(node == &freeTail)
			return nil;
		node->item = item;
		node->Remove();		// remove from free list
		head.Insert(node);
		return node;
	}
	CLink<T> *InsertSorted(T const &item){
		CLink<T> *sort;
		for(sort = head.next; sort != &tail; sort = sort->next)
			if(sort->item.sort >= item.sort)
				break;
		CLink<T> *node = freeHead.next;
		if(node == &freeTail)
			return nil;
		node->item = item;
		node->Remove();		// remove from free list
		sort->prev->Insert(node);
		return node;
	}
	void Remove(CLink<T> *link){
		link->Remove();		// remove from list
		freeHead.Insert(link);	// insert into free list
	}
	int32 Count(void){
		int n = 0;
		CLink<T> *lnk;
		for(lnk = head.next; lnk != &tail; lnk = lnk->next)
			n++;
		return n;
	}
};
