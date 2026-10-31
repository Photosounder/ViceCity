//+ rouz edit (ChatGPT)
#include "config.h"
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "Lists.h"
#include "Pools.h"

void
PtrList_Init(CPtrList *self)
{
    // Initialize the list head without touching any node storage
    self->first = NULL;
}

void
PtrList_Destroy(CPtrList *self)
{
    // Release the nodes at the former member-destructor boundary
    PtrList_Flush(self);
}

CPtrNode *
PtrList_InsertNode(CPtrList *self, CPtrNode *node)
{
    // Prepend the supplied node and update both neighboring links
    node->prev = NULL;
    node->next = self->first;
    if(self->first)
        self->first->prev = node;
    self->first = node;
    return node;
}

void
PtrList_RemoveNode(CPtrList *self, CPtrNode *node)
{
    // Unlink the node while preserving its original stale links
    if(node == self->first)
        self->first = node->next;
    if(node->prev)
        node->prev->next = node->next;
    if(node->next)
        node->next->prev = node->prev;
}

void
PtrList_DeleteNode(CPtrList *self, CPtrNode *node)
{
    // Unlink the node before returning its slot to the original pool
    PtrList_RemoveNode(self,node);
    CPool_Delete(CPools_GetPtrNodePool(),node);
}

void
PtrList_Flush(CPtrList *self)
{
    // Cache the successor before releasing each node
    CPtrNode *node,*next;
    for(node=self->first;node;node=next){
        // Preserve traversal through the node released by this iteration
        next=node->next;
        PtrList_DeleteNode(self,node);
    }
}

void
EntryInfoList_Init(CEntryInfoList *self)
{
    // Initialize the list head without touching any node storage
    self->first = NULL;
}

void
EntryInfoList_Destroy(CEntryInfoList *self)
{
    // Release the nodes at the former member-destructor boundary
    EntryInfoList_Flush(self);
}

CEntryInfoNode *
EntryInfoList_InsertNode(CEntryInfoList *self, CEntryInfoNode *node)
{
    // Prepend the supplied node and update both neighboring links
    node->prev = NULL;
    node->next = self->first;
    if(self->first)
        self->first->prev = node;
    self->first = node;
    return node;
}

void
EntryInfoList_RemoveNode(CEntryInfoList *self, CEntryInfoNode *node)
{
    // Unlink the node while preserving its original stale links
    if(node == self->first)
        self->first = node->next;
    if(node->prev)
        node->prev->next = node->next;
    if(node->next)
        node->next->prev = node->prev;
}

void
EntryInfoList_DeleteNode(CEntryInfoList *self, CEntryInfoNode *node)
{
    // Unlink the node before returning its slot to the original pool
    EntryInfoList_RemoveNode(self,node);
    CPool_Delete(CPools_GetEntryInfoNodePool(),node);
}

void
EntryInfoList_Flush(CEntryInfoList *self)
{
    // Cache the successor before releasing each node
    CEntryInfoNode *node,*next;
    for(node=self->first;node;node=next){
        // Preserve traversal through the node released by this iteration
        next=node->next;
        EntryInfoList_DeleteNode(self,node);
    }
}

CPtrNode *
PtrList_FindItem(CPtrList *self,void *item)
{
    // Return the first matching item without changing the list
    CPtrNode *node;
    for(node=self->first;node;node=node->next)
        if(node->item==item)
            return node;
    return NULL;
}

CPtrNode *
PtrList_InsertItem(CPtrList *self,void *item)
{
    // Allocate the same pool slot and prepend its item
    CPtrNode *node=(CPtrNode*)CPool_New(CPools_GetPtrNodePool());
    assert(node);
    node->item=item;
    PtrList_InsertNode(self,node);
    return node;
}

void
PtrList_RemoveItem(CPtrList *self,void *item)
{
    // Remove every match while retaining the cached-next traversal
    CPtrNode *node,*next;
    for(node=self->first;node;node=next){
        // Keep the successor before the pool releases this slot
        next=node->next;
        if(node->item==item)
            PtrList_DeleteNode(self,node);
    }
}

CEntryInfoNode *
EntryInfoList_InsertItem(CEntryInfoList *self,CPtrList *list,CPtrNode *listnode,CSector *sector)
{
    // Record the same sector membership in an entry-info pool slot
    CEntryInfoNode *node=(CEntryInfoNode*)CPool_New(CPools_GetEntryInfoNodePool());
    assert(node);
    node->list=list;
    node->listnode=listnode;
    node->sector=sector;
    EntryInfoList_InsertNode(self,node);
    return node;
}

typedef struct PtrListStaticBlock {
    unsigned char *base;
    size_t records,stride,offset,listsPerRecord;
} PtrListStaticBlock;
static PtrListStaticBlock staticBlocks[3];
static size_t staticBlockCount;

static void
PtrList_DestroyStaticBlock(void)
{
    // Mirror reverse destruction of the last registered array and its list members
    const PtrListStaticBlock *block=&staticBlocks[--staticBlockCount];
    size_t record,list;
    for(record=block->records;record>0;--record){
        // Destroy each record's list members in reverse declaration order
        for(list=block->listsPerRecord;list>0;--list){
            // Address each list through the complete array object's byte representation
            CPtrList *value=(CPtrList*)(block->base+(record-1)*block->stride+block->offset+(list-1)*sizeof(CPtrList));
            PtrList_Destroy(value);
        }
    }
}

int
PtrList_InitStaticBlock(void *base,size_t records,size_t stride,size_t offset,size_t listsPerRecord)
{
    // Register the world's three global list groups at their former constructor boundaries
    PtrListStaticBlock *block;
    size_t record,list;
    int result;
    assert(staticBlockCount<sizeof(staticBlocks)/sizeof(staticBlocks[0]));
    block=&staticBlocks[staticBlockCount++];
    block->base=(unsigned char*)base;
    block->records=records;
    block->stride=stride;
    block->offset=offset;
    block->listsPerRecord=listsPerRecord;
    for(record=0;record<records;++record){
        // Initialize each record's list members in forward declaration order
        for(list=0;list<listsPerRecord;++list){
            // Use the same complete-array addressing for initialization and cleanup
            CPtrList *value=(CPtrList*)(block->base+record*stride+offset+list*sizeof(CPtrList));
            PtrList_Init(value);
        }
    }
    // Keep exit registration outside the assertion so all build modes register cleanup
    result=atexit(PtrList_DestroyStaticBlock);
    assert(result==0);
    return result;
}
//- rouz edit (ChatGPT)
