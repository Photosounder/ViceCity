"""Stage complete collision-model algorithms before adapting native callers and lifetime owners."""
import re
import object_names_migration as objects
from anim_association_migration import original_body
from rect_migration import mask
RENAMES={'src/collision/ColModel.cpp':'src/collision/ColModel.c'}
DECLARATIONS=[('void','Init','CColModel *self'),('void','Destroy','CColModel *self'),('void','RemoveCollisionVolumes','CColModel *self'),('void','CalculateTrianglePlanes','CColModel *self'),('void','RemoveTrianglePlanes','CColModel *self'),('void','SetLinkPtr','CColModel *self,CLink *lptr'),('CLink *','GetLinkPtr','CColModel *self'),('CollisionVector3','GetTrianglePoint','const CColModel *self,int32_t i'),('CColModel *','Assign','CColModel *self,const CColModel *other')]
SIGNATURES=['CColModel::CColModel(void)','CColModel::~CColModel(void)','CColModel::RemoveCollisionVolumes(void)','CColModel::CalculateTrianglePlanes(void)','CColModel::RemoveTrianglePlanes(void)','CColModel::SetLinkPtr(CLink *lptr)','CColModel::GetLinkPtr(void)','CColModel::GetTrianglePoint(CVector &v, int i) const','CColModel::operator=(const CColModel &other)']
FIELDS=['boundingSphere','boundingBox','numSpheres','numBoxes','numTriangles','numLines','level','ownsCollisionVolumes','spheres','lines','boxes','vertices','triangles','trianglePlanes']

def header():
    # Retain the complete collision-model layout with explicit C operations and no automatic C++ lifetime
    source='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ColBox.h"
#include "ColSphere.h"
#include "ColLine.h"
#include "ColPoint.h"
#include "ColTriangle.h"
#include "LinkList.h"
typedef struct CColModel {
    CSphere boundingSphere;
    CBox boundingBox;
    int16_t numSpheres,numBoxes,numTriangles;
    int8_t numLines;
    uint8_t level;
    bool ownsCollisionVolumes;
    CColSphere *spheres;
    CColLine *lines;
    CColBox *boxes;
    CompressedVector *vertices;
    CColTriangle *triangles;
    CColTrianglePlane *trianglePlanes;
} CColModel;
#ifdef __cplusplus
extern "C" {
#endif
'''
    source+=''.join(result+' ColModel_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)
    source+='CColModel ColModel_InitStatic(CColModel *self,void (*cleanup)(void));\nvoid ColModel_DestroyArray(CColModel *models,int32_t count);\n'
    return source+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def backend_header():
    # Expose external allocator primitives and the separate collision-cache owner without copying model state
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "ColModel.h"
#ifdef __cplusplus
extern "C" {
#endif
void *ColModel_BackendAllocate(size_t size);
void ColModel_BackendFree(void *memory);
void ColModel_BackendPush(int32_t id);
void ColModel_BackendPop(void);
void ColModel_BackendRegister(void *location);
void ColModel_BackendRemoveCachedPlanes(CColModel *model);
void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Project every complete original model method and keep its live allocation and mutation ordering
    source='''//+ rouz edit (ChatGPT)
#include "ColModel.h"
#include "ColModel_backend.h"
#include "GameLevels.h"
#include "config.h"
#include <stdint.h>
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#define ALIGNPTR(p) (void*)((((uintptr_t)(void*)(p))+sizeof(void*)-1)&~(sizeof(void*)-1))
enum {COLMODEL_MEMID_COLLISION=13};
'''
    comments=['Initialize only the fields written by the original constructor','Release volumes through the complete original ownership policy','Preserve ownership checks, free ordering and the separate collision-cache boundary','Allocate the original extra plane element and calculate all live triangle planes','Release the current plane allocation before clearing its pointer','Store the cache link at the original aligned extra-element address','Read the cache link from the original aligned extra-element address','Decode the complete compressed vertex before returning its coordinates','Retain deep-copy allocation, reuse and source traversal in the original member order']
    fieldPattern=r'(?<![\w.>])\b(?:'+'|'.join(FIELDS)+r')\b'
    for (result,name,args),signature,comment in zip(DECLARATIONS,SIGNATURES,comments):
        # Extract complete definitions before replacing fields and external owner boundaries
        body=original_body(original,signature)
        for match in reversed(list(re.finditer(fieldPattern,mask(body)))):body=body[:match.start()]+'self->'+body[match.start():]
        body=body.replace('other.','other->').replace('this','self')
        body=re.sub(r'\bnil\b','NULL',body).replace('RwMalloc(','ColModel_BackendAllocate(').replace('RwFree(','ColModel_BackendFree(')
        body=body.replace('CCollision::RemoveTrianglePlanes(self)','ColModel_BackendRemoveCachedPlanes(self)')
        body=body.replace('PUSH_MEMID(MEMID_COLLISION)','ColModel_BackendPush(COLMODEL_MEMID_COLLISION)').replace('POP_MEMID()','ColModel_BackendPop()').replace('REGISTER_MEMPTR(','ColModel_BackendRegister(')
        if name=='Destroy':body=body.replace('RemoveCollisionVolumes();','ColModel_RemoveCollisionVolumes(self);')
        if name=='GetTrianglePoint':body=body.replace('v = CVector(collisionVertexValue0_0.x,collisionVertexValue0_0.y,collisionVertexValue0_0.z);','return collisionVertexValue0_0;')
        if name=='Assign':body=body.replace('return *self;','return self;')
        source+=result+' ColModel_'+name+'('+args+')\n{\n    // '+comment+'\n'+body+'}\n'
    source+='''CColModel ColModel_InitStatic(CColModel *self,void (*cleanup)(void))
{
    // Initialize original static storage and register its native storage-address callback at the same lifetime boundary
    ColModel_Init(self);
    if(cleanup)atexit(cleanup);
    return *self;
}
void ColModel_DestroyArray(CColModel *models,int32_t count)
{
    // Preserve the original reverse element destruction order for native static collision arrays
    for(int32_t i=count;i>0;--i)ColModel_Destroy(&models[i-1]);
}
'''
    return source+'//- rouz edit (ChatGPT)\n'

def native():
    # Keep only the actual external allocation, memory-category and collision-cache operations native
    return '''//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColModel_backend.h"
#include "Collision.h"
#include "MemoryHeap.h"
static_assert(MEMID_COLLISION==13,"Original collision allocation category");
void *ColModel_BackendAllocate(size_t size)
{
    // Forward the original byte allocation through the existing SDK memory functions
    return RwMalloc(size);
}
void ColModel_BackendFree(void *memory)
{
    // Release the exact original pointer through the existing SDK memory functions
    RwFree(memory);
}
void ColModel_BackendPush(int32_t id)
{
    // Preserve the original conditional custom-heap category operation
    PUSH_MEMID(id);
}
void ColModel_BackendPop(void)
{
    // Restore the original custom-heap category when that allocator is enabled
    POP_MEMID();
}
void ColModel_BackendRegister(void *location)
{
    // Register the actual live pointer slot with the original custom allocator
    REGISTER_MEMPTR(location);
}
void ColModel_BackendRemoveCachedPlanes(CColModel *model)
{
    // Enter the separate collision-cache owner at the original removal boundary
    CCollision::RemoveTrianglePlanes(model);
}
//- rouz edit (ChatGPT)
'''

def col_model_calls(source):
    # Move the complete model owner and adapt diagnosed native operation and lifetime consumers
    if 'CColModel::CColModel(void)' in source and 'CColModel::operator=' in source:return kernel(source)
    if 'struct CColModel' in source and 'CColModel& operator=(const CColModel& other);' in source:return header()
    from col_model_consumers import calls as consumers,player_methods
    from col_model_schema import PLAYER_FIELD_SCHEMA
    if 'CPlayerInfo::Clear(void)' in source and 'CPlayerInfo::CPlayerInfo(void)' not in source:
        assert source.count('CVector lastPlayerPos;')==1
        source=source.replace('CVector lastPlayerPos;',player_methods(PLAYER_FIELD_SCHEMA)+'\nCVector lastPlayerPos;',1)
    return consumers(source)


def col_model_project(source):
    # Select the complete C collision-model producer and add its distinct native primitive unit
    source=source.replace('src\\collision\\ColModel.cpp','src\\collision\\ColModel.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\collision\\ColModel\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain all other metadata while explicitly selecting strict C11 compilation
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';native=match[1][:-len('ColModel.c')]+'ColModel_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\collision\\ColModel\.h)"\s*/>',source)
    if anchor:
        # Keep the native boundary declarations beside the self-contained public C API
        entry='<ClInclude Include="'+anchor[1][:-len('ColModel.h')]+'ColModel_backend.h" />'
        if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)



def calls(source):
    # Preserve complete C memory ownership before moving complete memory allocation policy into C
    import memory_mgr_migration as memory
    return memory.calls(col_model_calls(source))


def project(source):
    # Retain C collision-model metadata before selecting the C memory owner and native SDK table
    import memory_mgr_migration as memory
    return memory.project(col_model_project(source))
