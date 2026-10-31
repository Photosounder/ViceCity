"""Prepare complete original/C clump loaders with controlled external SDK objects."""
from pathlib import Path
import re
import clump_read_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/clump-read-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
typedef int32_t int32,RwInt32;
typedef uint32_t RwUInt32;
typedef int RwBool;
#define nil NULL
extern "C" void re3_assert(const char*,const char*,unsigned int,const char*);
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
struct RwFrame {unsigned id,root;bool alive;};
struct RpGeometry {unsigned id,refs;bool alive;};
struct RpAtomic {unsigned id,flags;bool alive;RwFrame *frame;RpGeometry *geometry;};
struct RpClump {unsigned id,count;bool alive;RwFrame *frame;RpAtomic *atomics[32];};
struct rwFrameList {int32_t numFrames;RwFrame **frames;};
struct RpClumpChunkInfo {int32_t numAtomics,numLights,numCameras;};
struct RwStream {
    unsigned id;int mode,section,position,geometryBase,nextGeometry,nextAtomic;
    int totalGeometries,totalAtomics,totalFrames,failGeometryChunk,failGeometryRead,failAtomicChunk,missingKind,shortSection;
    uint32_t version,atomicSize;bool failFrames,cancelInFrames;
    struct {struct {uint32_t position;} memory;} Type;
    int32_t tell(void);
};
enum {rwID_STRUCT=1,rwID_GEOMETRY=2,rwID_FRAMELIST=3,rwID_GEOMETRYLIST=4,rwID_ATOMIC=5};
void *RwMalloc(size_t);
void RwFree(void*);
RwBool RwStreamFindChunk(RwStream*,uint32_t,uint32_t*,uint32_t*);
uint32_t RwStreamRead(RwStream*,void*,uint32_t);
RpGeometry *RpGeometryStreamRead(RwStream*);
RwBool RpGeometryDestroy(RpGeometry*);
RpAtomic *RpAtomicCreate(void);
RwBool RpAtomicDestroy(RpAtomic*);
RpAtomic *RpAtomicSetFlags(RpAtomic*,uint32_t);
RpAtomic *RpAtomicSetFrame(RpAtomic*,RwFrame*);
RpAtomic *RpAtomicSetGeometry(RpAtomic*,RpGeometry*,uint32_t);
rwFrameList *rwFrameListStreamRead(RwStream*,rwFrameList*);
rwFrameList *rwFrameListDeinitialize(rwFrameList*);
RwStream *RwStreamSkip(RwStream*,uint32_t);
RpClump *RpClumpCreate(void);
RwBool RpClumpDestroy(RpClump*);
RpClump *RpClumpSetFrame(RpClump*,RwFrame*);
RpClump *RpClumpAddAtomic(RpClump*,RpAtomic*);
#ifdef LIBRW
#define STREAMPOS(value) ((value)->tell())
#else
#define STREAMPOS(value) ((value)->Type.memory.position)
#endif
#ifdef ORIGINAL
#include "original-records.h"
typedef rpGeometryList FixtureGeometryList;
rpGeometryList *GeometryListStreamRead1(RwStream*,rpGeometryList*);
rpGeometryList *GeometryListStreamRead2(RwStream*,rpGeometryList*);
void GeometryListDeinitialize(rpGeometryList*);
RpAtomic *ClumpAtomicStreamRead(RwStream*,rwFrameList*,rpGeometryList*);
bool RpClumpGtaStreamRead1(RwStream*);
RpClump *RpClumpGtaStreamRead2(RwStream*);
void RpClumpGtaCancelStream(void);
#else
#include "ClumpRead.h"
typedef ClumpReadGeometryList FixtureGeometryList;
#endif
'''
entry='''#pragma once
typedef struct ClumpResult {void *pointer;int number;} ClumpResult;
enum {OP_Geometry1,OP_Geometry2,OP_Deinitialize,OP_Atomic,OP_Start,OP_Finish,OP_Cancel};
#ifdef __cplusplus
extern "C" {
#endif
ClumpResult TestClump(unsigned operation,void *a,void *b,void *c);
#ifdef __cplusplus
}
#endif
'''
before='''case OP_Geometry1:result.pointer=GeometryListStreamRead1((RwStream*)a,(rpGeometryList*)b);break;
    case OP_Geometry2:result.pointer=GeometryListStreamRead2((RwStream*)a,(rpGeometryList*)b);break;
    case OP_Deinitialize:GeometryListDeinitialize((rpGeometryList*)a);break;
    case OP_Atomic:result.pointer=ClumpAtomicStreamRead((RwStream*)a,(rwFrameList*)b,(rpGeometryList*)c);break;
    case OP_Start:result.number=RpClumpGtaStreamRead1((RwStream*)a);break;
    case OP_Finish:result.pointer=RpClumpGtaStreamRead2((RwStream*)a);break;
    case OP_Cancel:RpClumpGtaCancelStream();break;
'''
after='''case OP_Geometry1:result.pointer=ClumpRead_GeometryListStreamRead1(a,(ClumpReadGeometryList*)b);break;
    case OP_Geometry2:result.pointer=ClumpRead_GeometryListStreamRead2(a,(ClumpReadGeometryList*)b);break;
    case OP_Deinitialize:ClumpRead_GeometryListDeinitialize((ClumpReadGeometryList*)a);break;
    case OP_Atomic:result.pointer=ClumpRead_AtomicStreamRead(a,b,(ClumpReadGeometryList*)c);break;
    case OP_Start:result.number=ClumpRead_Start(a);break;
    case OP_Finish:result.pointer=ClumpRead_Finish(a);break;
    case OP_Cancel:ClumpRead_Cancel();break;
'''
for name,cases,prefix in [('before-caller.cpp',before,'#include "owners.h"\n'),('after-caller.c',after,'#include "ClumpRead.h"\n')]:
    # Enter every complete original or C operation through an independently compiled public caller
    value=prefix+'#include "entry.h"\nClumpResult TestClump(unsigned operation,void *a,void *b,void *c)\n{\n    // Dispatch complete owner algorithms without reproducing their loading or cleanup policy\n    ClumpResult result={0};switch(operation){\n    '+cases+'    }return result;\n}\n'
    (probe/name).write_text(value,newline='\n')
original=(out/'before/src/rw/ClumpRead.cpp').read_text();records=[]
for name in ['rpGeometryList','rpAtomicBinary']:
    # Share exact original declarations while preserving every original function body
    value=re.search(r'struct '+name+r'\n\{[\s\S]*?\n\};',original)[0];records.append(value);original=original.replace(value,'')
original=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
for name,value in [('owners.h',owners),('entry.h',entry),('original-records.h','\n'.join(records)+'\n'),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "ClumpRead_backend.h"\n'+native),('fixture.cpp',(root/'utils/tests/clump_read_fixture.cpp.in').read_text())]:
    # Materialize whole original/C producers and the actual native bridge against controlled external SDK operations
    (probe/name).write_text(value,newline='\n')
print('Prepared all seven complete original/C clump operations and live SDK frame-list storage')
