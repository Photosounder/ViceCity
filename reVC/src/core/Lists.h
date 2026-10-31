//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
typedef struct CPtrNode {void *item;struct CPtrNode *prev,*next;} CPtrNode;
typedef struct CPtrList {CPtrNode *first;} CPtrList;
typedef struct CSector CSector;
typedef struct CEntryInfoNode {
    CPtrList *list;
    CPtrNode *listnode;
    CSector *sector;
    struct CEntryInfoNode *prev,*next;
} CEntryInfoNode;
typedef struct CEntryInfoList {CEntryInfoNode *first;} CEntryInfoList;
#ifdef __cplusplus
extern "C" {
#endif
void PtrList_Init(CPtrList *self);
void PtrList_Destroy(CPtrList *self);
CPtrNode * PtrList_InsertNode(CPtrList *self, CPtrNode *node);
void PtrList_RemoveNode(CPtrList *self, CPtrNode *node);
void PtrList_DeleteNode(CPtrList *self, CPtrNode *node);
void PtrList_Flush(CPtrList *self);
void EntryInfoList_Init(CEntryInfoList *self);
void EntryInfoList_Destroy(CEntryInfoList *self);
CEntryInfoNode * EntryInfoList_InsertNode(CEntryInfoList *self, CEntryInfoNode *node);
void EntryInfoList_RemoveNode(CEntryInfoList *self, CEntryInfoNode *node);
void EntryInfoList_DeleteNode(CEntryInfoList *self, CEntryInfoNode *node);
void EntryInfoList_Flush(CEntryInfoList *self);
CPtrNode *PtrList_FindItem(CPtrList *self,void *item);
CPtrNode *PtrList_InsertItem(CPtrList *self,void *item);
void PtrList_RemoveItem(CPtrList *self,void *item);
CEntryInfoNode *EntryInfoList_InsertItem(CEntryInfoList *self,CPtrList *list,CPtrNode *listnode,CSector *sector);
/* The world registers its three static blocks in their original definition order */
int PtrList_InitStaticBlock(void *base,size_t records,size_t stride,size_t offset,size_t listsPerRecord);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
