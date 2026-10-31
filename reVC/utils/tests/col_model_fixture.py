"""Prepare whole original/C collision-model owners with real C geometry and cache removal."""
from pathlib import Path
import re
import col_model_migration as m
from anim_association_migration import original_body
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/col-model-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
#include <malloc.h>
#include "ColBox.h"
#include "ColSphere.h"
#include "ColLine.h"
#include "ColPoint.h"
#include "ColTriangle.h"
#include "LinkList.h"
#include "GameLevels.h"
typedef int16_t int16;
typedef int8_t int8;
typedef uint8_t uint8;
typedef uintptr_t uintptr;
struct CVector {float x,y,z;CVector(){} CVector(float a,float b,float c):x(a),y(b),z(c){}};
#ifdef ORIGINAL
#include "original-ColModel.h"
#else
#include "ColModel.h"
#endif
class CCollision {public:static CLinkList ms_colModelCache;static void RemoveTrianglePlanes(CColModel *model);};
void *RwMalloc(size_t size);
void RwFree(void *memory);
class ModelHeap {public:void PushMemId(int32_t id);void PopMemId();void RegisterMemPointer(void *location);};
extern ModelHeap gMainHeap;
#ifdef USE_CUSTOM_ALLOCATOR
#define PUSH_MEMID(id) gMainHeap.PushMemId(id)
#define POP_MEMID() gMainHeap.PopMemId()
#define REGISTER_MEMPTR(p) gMainHeap.RegisterMemPointer(p)
#else
#define PUSH_MEMID(id)
#define POP_MEMID()
#define REGISTER_MEMPTR(p)
#endif
enum {MEMID_COLLISION=13};
#define nil NULL
#define ALIGNPTR(p) (void*)((((uintptr)(void*)(p))+sizeof(void*)-1)&~(sizeof(void*)-1))
extern "C" void re3_assert(const char*,const char*,unsigned int,const char*);
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
'''
entry='''#pragma once
#include <stdint.h>
#include "CollisionVector.h"
typedef struct ColModelResult {void *pointer;CollisionVector3 point;} ColModelResult;
#ifdef __cplusplus
extern "C" {
#endif
ColModelResult TestModel(unsigned operation,void *self,const void *other,int32_t index,void *link);
#ifdef __cplusplus
}
#endif
'''
before='''#include "owners.h"
#include "entry.h"
ColModelResult TestModel(unsigned operation,void *value,const void *other,int32_t index,void *link)
{
    // Invoke every complete original owner operation through the independent comparison boundary
    CColModel *self=(CColModel*)value;ColModelResult result={};switch(operation){
    case 0:new(self) CColModel;break;
    case 1:self->~CColModel();break;
    case 2:self->RemoveCollisionVolumes();break;
    case 3:self->CalculateTrianglePlanes();break;
    case 4:self->RemoveTrianglePlanes();break;
    case 5:self->SetLinkPtr((CLink*)link);break;
    case 6:result.pointer=self->GetLinkPtr();break;
    case 7:{CVector point;self->GetTrianglePoint(point,index);result.point={point.x,point.y,point.z};break;}
    case 8:result.pointer=&(*self=*(const CColModel*)other);break;
    }return result;
}
'''
after='''#include "ColModel.h"
#include "entry.h"
ColModelResult TestModel(unsigned operation,void *value,const void *other,int32_t index,void *link)
{
    // Invoke all complete C operations from a separately compiled strict C11 consumer
    CColModel *self=(CColModel*)value;ColModelResult result={0};switch(operation){
    case 0:ColModel_Init(self);break;
    case 1:ColModel_Destroy(self);break;
    case 2:ColModel_RemoveCollisionVolumes(self);break;
    case 3:ColModel_CalculateTrianglePlanes(self);break;
    case 4:ColModel_RemoveTrianglePlanes(self);break;
    case 5:ColModel_SetLinkPtr(self,(CLink*)link);break;
    case 6:result.pointer=ColModel_GetLinkPtr(self);break;
    case 7:result.point=ColModel_GetTrianglePoint(self,index);break;
    case 8:result.pointer=ColModel_Assign(self,(const CColModel*)other);break;
    }return result;
}
'''
original=re.sub(r'^#include[^\n]*\n','',(out/'before/src/collision/ColModel.cpp').read_text(),flags=re.M)
originalHeader=re.sub(r'^#include[^\n]*\n','',(out/'before/src/collision/ColModel.h').read_text(),flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
cache=original_body((out/'before/src/collision/Collision.cpp').read_text(),'CCollision::RemoveTrianglePlanes(CColModel *model)')
cacheAfter=cache.replace('model->GetLinkPtr()','ColModel_GetLinkPtr(model)').replace('model->RemoveTrianglePlanes()','ColModel_RemoveTrianglePlanes(model)')
cacheSource='#include "owners.h"\nCLinkList CCollision::ms_colModelCache;\nvoid CCollision::RemoveTrianglePlanes(CColModel *model)\n{\n#ifdef ORIGINAL\n'+cache+'#else\n'+cacheAfter+'#endif\n}\n'
for name,value in [('owners.h',owners),('entry.h',entry),('original-ColModel.h',originalHeader),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "ColModel_backend.h"\n'+native),('before-caller.cpp',before),('after-caller.c',after),('cache.cpp',cacheSource),('fixture.cpp',(root/'utils/tests/col_model_fixture.cpp.in').read_text())]:
    # Keep all generated comparison sources isolated from the coherent staged production candidate
    (probe/name).write_text(value,newline='\n')
print('Prepared nine complete original/C model methods with genuine C geometry and actual cache removal')
