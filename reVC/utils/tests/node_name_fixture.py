"""Compare both whole frame-name owners through actual registered SDK callback signatures."""
from pathlib import Path
import re
import node_name_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/node-name-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
sdk='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int32_t int32;
typedef int32_t RwInt32;
typedef uint32_t RwUInt32;
struct RwFrame {unsigned char bytes[128];};
struct RwStream {unsigned char bytes[128];uint32_t limit,requested;bool failure;};
typedef void *(*RwPluginObjectConstructor)(void*,RwInt32,RwInt32);
typedef void *(*RwPluginObjectDestructor)(void*,RwInt32,RwInt32);
typedef void *(*RwPluginObjectCopy)(void*,const void*,RwInt32,RwInt32);
typedef RwStream *(*RwPluginDataChunkReadCallBack)(RwStream*,RwInt32,void*,RwInt32,RwInt32);
typedef RwStream *(*RwPluginDataChunkWriteCallBack)(RwStream*,RwInt32,const void*,RwInt32,RwInt32);
typedef RwInt32 (*RwPluginDataChunkGetSizeCallBack)(const void*,RwInt32,RwInt32);
RwInt32 RwFrameRegisterPlugin(RwInt32,RwUInt32,RwPluginObjectConstructor,RwPluginObjectDestructor,RwPluginObjectCopy);
RwInt32 RwFrameRegisterPluginStream(RwUInt32,RwPluginDataChunkReadCallBack,RwPluginDataChunkWriteCallBack,RwPluginDataChunkGetSizeCallBack);
RwUInt32 RwStreamRead(RwStream*,void*,RwUInt32);
RwStream *RwStreamWrite(RwStream*,const void*,RwUInt32);
#define nil NULL
#define MAKECHUNKID(v,i) (((v)<<8)|(i))
#define rwVENDORID_ROCKSTAR 0x0253F2
#define RWPLUGINOFFSET(t,b,o) ((t*)((unsigned char*)(b)+(o)))
#define rwstrlen strlen
'''
entry='''#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool TestAttach(void);
char *TestName(void *frame);
#ifdef __cplusplus
}
#endif
'''
caller='''#include "NodeName.h"
#include "entry.h"
bool TestAttach(void)
{
    // Exercise the actual public attach operation through an independent caller
    return NodeNamePluginAttach();
}
char *TestName(void *frame)
{
    // Preserve pointer identity through the real public name lookup
    return GetFrameNodeName(frame);
}
'''
original=re.sub(r'^#include[^\n]*\n','',(out/'before/src/rw/NodeName.cpp').read_text(),flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
before=caller.replace('#include "NodeName.h"','#include "sdk.h"\n#include "original-NodeName.h"').replace('GetFrameNodeName(frame)','GetFrameNodeName(static_cast<RwFrame*>(frame))')
for name,value in [('sdk.h',sdk),('entry.h',entry),('original-NodeName.h',(out/'before/src/rw/NodeName.h').read_text()),('original.cpp','#include "sdk.h"\n'+original),('native.cpp','#include "sdk.h"\n#include "NodeName_backend.h"\n'+native),('before-caller.cpp',before),('after-caller.c',caller),('fixture.cpp',(root/'utils/tests/node_name_fixture.cpp.in').read_text())]:
    # Materialize complete original/C producers and SDK-only fixtures in an isolated immutable runtime directory
    (probe/name).write_text(value,newline='\n')
print('Prepared complete original/C frame-name plugin owners with actual registered callback signatures')
