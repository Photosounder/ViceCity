"""Replace both intrusive list owners and their nodes with C records and explicit lifetime operations."""
import re
import object_names_migration as objects
RENAMES={'src/core/Lists.cpp':'src/core/Lists.c'}

def header():
    # Preserve all four pointer-only records and the opaque sector identity
    source='''//+ rouz edit (ChatGPT)
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
'''
    for prefix,record,node in [('PtrList','CPtrList','CPtrNode'),('EntryInfoList','CEntryInfoList','CEntryInfoNode')]:
        for operation,result,args in [('Init','void',''),('Destroy','void',''),('InsertNode',node+' *',node+' *node'),('RemoveNode','void',node+' *node'),('DeleteNode','void',node+' *node'),('Flush','void','')]:
            source+=result+' '+prefix+'_'+operation+'('+record+' *self'+(', '+args if args else '')+');\n'
    source+='''CPtrNode *PtrList_FindItem(CPtrList *self,void *item);
CPtrNode *PtrList_InsertItem(CPtrList *self,void *item);
void PtrList_RemoveItem(CPtrList *self,void *item);
CEntryInfoNode *EntryInfoList_InsertItem(CEntryInfoList *self,CPtrList *list,CPtrNode *listnode,CSector *sector);
/* The world registers its three static blocks in their original definition order */
int PtrList_InitStaticBlock(void *base,size_t records,size_t stride,size_t offset,size_t listsPerRecord);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return source

def kernel():
    # Keep the original assertion provider and pool algorithms in a strict C11 translation unit
    source='''//+ rouz edit (ChatGPT)
#include "config.h"
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "Lists.h"
#include "Pools.h"

'''
    for prefix,record,node,pool in [('PtrList','CPtrList','CPtrNode','PtrNode'),('EntryInfoList','CEntryInfoList','CEntryInfoNode','EntryInfoNode')]:
        bodies=[('Init','void','', 'Initialize the list head without touching any node storage','self->first = NULL;'),
                ('Destroy','void','', 'Release the nodes at the former member-destructor boundary',prefix+'_Flush(self);'),
                ('InsertNode',node+' *',node+' *node','Prepend the supplied node and update both neighboring links','node->prev = NULL;\nnode->next = self->first;\nif(self->first)\n    self->first->prev = node;\nself->first = node;\nreturn node;'),
                ('RemoveNode','void',node+' *node','Unlink the node while preserving its original stale links','if(node == self->first)\n    self->first = node->next;\nif(node->prev)\n    node->prev->next = node->next;\nif(node->next)\n    node->next->prev = node->prev;'),
                ('DeleteNode','void',node+' *node','Unlink the node before returning its slot to the original pool',prefix+'_RemoveNode(self,node);\nCPool_Delete(CPools_Get'+pool+'Pool(),node);'),
                ('Flush','void','','Cache the successor before releasing each node',node+' *node,*next;\nfor(node=self->first;node;node=next){\n    // Preserve traversal through the node released by this iteration\n    next=node->next;\n    '+prefix+'_DeleteNode(self,node);\n}')]
        for operation,result,args,comment,body in bodies:
            source+=result+'\n'+prefix+'_'+operation+'('+record+' *self'+(', '+args if args else '')+')\n{\n    // '+comment+'\n    '+body.replace('\n','\n    ')+'\n}\n\n'
    source+='''CPtrNode *
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
'''
    return source

def lists_calls(source):
    # Replace whole owners first and then project only known typed callers and lifecycle sites
    if 'class CPtrNode\n{' in source and 'class CEntryInfoList\n{' in source:return header()
    if 'CPtrList::InsertItem(void *item)' in source:return kernel()
    original=source
    for name in ['CPtrList','CPtrNode','CEntryInfoList','CEntryInfoNode']:
        source=source.replace('class '+name+';','struct '+name+';')
    source=source.replace('class CSector\n{\npublic:\n','struct CSector\n{\n')
    source=re.sub(r'm_entryInfoList\.(InsertItem|DeleteNode|Flush)\(',r'EntryInfoList_\1(&m_entryInfoList, ',source)
    source=source.replace('EntryInfoList_Flush(&m_entryInfoList, )','EntryInfoList_Flush(&m_entryInfoList)')
    # The listed receivers are all CPtrList objects or pointers in the immutable source audit
    source=re.sub(r'((?:node|next)->list|list)->(InsertItem|RemoveItem|RemoveNode|InsertNode|DeleteNode)\(',r'PtrList_\2(\1, ',source)
    source=re.sub(r'(CWorld::GetMovingEntityList\(\)|ms_bigBuildingsList\[[^\]]+\]|ms_listMovingEntityPtrs|pSector->m_lists\[[^\]]+\])\.(InsertItem|RemoveItem|DeleteNode|Flush)\(',r'PtrList_\2(&\1, ',source)
    source=re.sub(r'(PtrList_Flush\([^\n]+), \)',r'\1)',source)
    if 'CPhysical::CPhysical(void)\n{' in source and 'EntryInfoList_Init(&m_entryInfoList);' not in source:
        source=source.replace('CPhysical::CPhysical(void)\n{','CPhysical::CPhysical(void)\n{\n\t// Initialize sector membership at the former list constructor boundary\n\tEntryInfoList_Init(&m_entryInfoList);')
    source=source.replace('CPhysical::~CPhysical(void)\n{\n\tEntryInfoList_Flush(&m_entryInfoList);\n}', 'CPhysical::~CPhysical(void)\n{\n\t// Preserve the explicit flush followed by the former member destructor\n\tEntryInfoList_Flush(&m_entryInfoList);\n\tEntryInfoList_Destroy(&m_entryInfoList);\n}')
    source=source.replace('CDummy(void) { m_type = ENTITY_TYPE_DUMMY; }','''CDummy(void) {
        // Initialize sector membership before assigning the dummy type
        EntryInfoList_Init(&m_entryInfoList);
        m_type = ENTITY_TYPE_DUMMY;
    }
    ~CDummy(void) {
        // Release membership nodes before the entity base destructor
        EntryInfoList_Destroy(&m_entryInfoList);
    }''')
    if 'static CPtrList ms_bigBuildingsList[NUM_LEVELS];' in source and 'static int ms_bigBuildingsListLifetime,' not in source:
        source=source.replace('static uint16 ms_nCurrentScanCode;','static int ms_bigBuildingsListLifetime,ms_movingListLifetime,ms_sectorListsLifetime;\n\tstatic uint16 ms_nCurrentScanCode;')
    for declaration,definition in [
        ('CPtrList CWorld::ms_bigBuildingsList[NUM_LEVELS];','int CWorld::ms_bigBuildingsListLifetime=PtrList_InitStaticBlock(&CWorld::ms_bigBuildingsList,NUM_LEVELS,sizeof(CPtrList),0,1);'),
        ('CPtrList CWorld::ms_listMovingEntityPtrs;','int CWorld::ms_movingListLifetime=PtrList_InitStaticBlock(&CWorld::ms_listMovingEntityPtrs,1,sizeof(CPtrList),0,1);'),
        ('CSector CWorld::ms_aSectors[NUMSECTORS_Y][NUMSECTORS_X];','int CWorld::ms_sectorListsLifetime=PtrList_InitStaticBlock(&CWorld::ms_aSectors,NUMSECTORS_Y*NUMSECTORS_X,sizeof(CSector),offsetof(CSector,m_lists),NUMSECTORENTITYLISTS);')]:
        if declaration in source and definition not in source:source=source.replace(declaration,declaration+'\n'+definition)
    if 'CPool *CPools_GetAudioScriptObjectPool(void);' in source and 'CPool *CPools_GetPtrNodePool(void);' not in source:
        source=source.replace('CPool *CPools_GetAudioScriptObjectPool(void);','CPool *CPools_GetAudioScriptObjectPool(void);\nCPool *CPools_GetPtrNodePool(void);\nCPool *CPools_GetEntryInfoNodePool(void);')
    if 'CPools_GetAudioScriptObjectPool(void)\n{' in source and 'CPools_GetPtrNodePool(void)\n{' not in source:
        source+='''
CPool *CPools_GetPtrNodePool(void)
{
    // Expose the existing pointer-node pool to the C list owner
    return CPools::GetPtrNodePool();
}

CPool *CPools_GetEntryInfoNodePool(void)
{
    // Expose the existing entry-info pool to the C list owner
    return CPools::GetEntryInfoNodePool();
}
'''
    if source==original:return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n' if not (source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)')) else source

def lists_project(source):
    # Retain the explicit historical rename while current generated projects discover Lists.c directly
    source=source.replace('..\\src\\core\\Lists.cpp','..\\src\\core\\Lists.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\core\\Lists\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain unrelated per-file options and mark the renamed producer as C11
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="..\\src\\core\\Lists.c">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Preserve complete list ownership before replacing the streaming-info record class
    import streaming_info_migration as streaming_info
    return streaming_info.calls(lists_calls(source))

def project(source):
    # Retain previous registrations before adding the independent C streaming-info producer
    import streaming_info_migration as streaming_info
    return streaming_info.project(lists_project(source))
