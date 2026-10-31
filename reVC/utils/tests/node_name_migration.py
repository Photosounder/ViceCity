"""Move the complete frame-name plugin owner into C with actual SDK primitives kept native."""
import re
import object_names_migration as objects
RENAMES={'src/rw/NodeName.cpp':'src/rw/NodeName.c'}
OPERATIONS=['NodeNameConstructor','NodeNameDestructor','NodeNameCopy','NodeNameStreamRead','NodeNameStreamWrite','NodeNameStreamGetSize','NodeNamePluginAttach','GetFrameNodeName']

def header():
    # Expose the existing public operations without requiring C++ or SDK declarations
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool NodeNamePluginAttach(void);
char *GetFrameNodeName(void *frame);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Declare only the native SDK boundaries and correctly typed C lifecycle callbacks
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
int32_t NodeName_HostRegister(int32_t size,uint32_t id);
void NodeName_HostRegisterStream(uint32_t id);
void NodeName_HostRead(void *stream,void *buffer,uint32_t length);
void NodeName_HostWrite(void *stream,const void *buffer,uint32_t length);
void *NodeNameConstructor(void *object,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameDestructor(void *object,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameCopy(void *dstObject,const void *srcObject,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameStreamRead(void *stream,int32_t binaryLength,void *object,int32_t offsetInObject,int32_t sizeInObject);
void *NodeNameStreamWrite(void *stream,int32_t binaryLength,const void *object,int32_t offsetInObject,int32_t sizeInObject);
int32_t NodeNameStreamGetSize(const void *object,int32_t offsetInObject,int32_t sizeInObject);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Retain every complete plugin algorithm and its original global offset lifetime
    source=source.replace('#include "common.h"','#include "NodeName_backend.h"\n#include <stddef.h>\n#include <string.h>')
    source=source.replace('MAKECHUNKID(rwVENDORID_ROCKSTAR, 0xFE)','0x0253F2FE')
    source=source.replace('(RWPLUGINOFFSET(char, o, gPluginOffset))','((char *)(o) + gPluginOffset)')
    source=source.replace('RwInt32','int32_t').replace('int32 gPluginOffset','int32_t gPluginOffset').replace('RwStream*','void*').replace('RwStream *stream','void *stream').replace('RwFrame *frame','void *frame')
    source=re.sub(r'\bnil\b','NULL',source).replace('rwstrlen(name)','strlen(name)')
    source=source.replace('RwStreamRead(stream,','NodeName_HostRead(stream,').replace('RwStreamWrite(stream,','NodeName_HostWrite(stream,')
    source=source.replace('RwFrameRegisterPlugin(24, ID_NODENAME,\n                                NodeNameConstructor,\n                                NodeNameDestructor,\n                                NodeNameCopy)','NodeName_HostRegister(24, ID_NODENAME)')
    source=source.replace('RwFrameRegisterPluginStream(ID_NODENAME,\n\t\tNodeNameStreamRead,\n\t\tNodeNameStreamWrite,\n\t\tNodeNameStreamGetSize)','NodeName_HostRegisterStream(ID_NODENAME)')
    comments={
        'NodeNameConstructor':'Clear the registered slot only when the original global offset is positive',
        'NodeNameDestructor':'Retain the original no-op plugin destruction and object identity',
        'NodeNameCopy':'Copy exactly twenty-three bytes with the original padding and null return',
        'NodeNameStreamRead':'Read the requested bytes before terminating at the current registered offset',
        'NodeNameStreamWrite':'Write the requested bytes and return the caller stream independently of SDK success',
        'NodeNameStreamGetSize':'Measure the current registered name without changing the original callback arguments',
        'NodeNamePluginAttach':'Register object callbacks before stream callbacks and preserve the original failure result',
        'GetFrameNodeName':'Borrow the registered name slot unless plugin registration produced a negative offset'}
    for name,comment in comments.items():
        # Describe each complete converted operation inside its actual function body
        pattern=r'(?m)^('+name+r'\([^\n]*\)\n\{\n)';source,count=re.subn(pattern,lambda match:match[1]+'\t// '+comment+'\n',source);assert count==1,name
    return '//+ rouz edit (ChatGPT)\n'+source+'//- rouz edit (ChatGPT)\n'

def native():
    # Keep only actual SDK registration, typed stream callback transport and raw byte I/O native
    return '''//+ rouz edit (ChatGPT)
#include "common.h"
#include "NodeName_backend.h"
static_assert(MAKECHUNKID(rwVENDORID_ROCKSTAR,0xFE)==0x0253F2FE,"Node-name plugin identity");
static RwStream *NodeNameReadNative(RwStream *stream,RwInt32 length,void *object,RwInt32 offset,RwInt32 size)
{
    // Transport the actual SDK stream handle into the complete C read callback
    return static_cast<RwStream*>(NodeNameStreamRead(stream,length,object,offset,size));
}
static RwStream *NodeNameWriteNative(RwStream *stream,RwInt32 length,const void *object,RwInt32 offset,RwInt32 size)
{
    // Transport the actual SDK stream handle into the complete C write callback
    return static_cast<RwStream*>(NodeNameStreamWrite(stream,length,object,offset,size));
}
int32_t NodeName_HostRegister(int32_t size,uint32_t id)
{
    // Bind the complete C lifecycle callbacks at the real RenderWare registration boundary
    return RwFrameRegisterPlugin(size,id,NodeNameConstructor,NodeNameDestructor,NodeNameCopy);
}
void NodeName_HostRegisterStream(uint32_t id)
{
    // Bind typed SDK stream trampolines and the unchanged C size callback
    RwFrameRegisterPluginStream(id,NodeNameReadNative,NodeNameWriteNative,NodeNameStreamGetSize);
}
void NodeName_HostRead(void *stream,void *buffer,uint32_t length)
{
    // Read bytes from the original live SDK stream into caller-owned plugin storage
    RwStreamRead(static_cast<RwStream*>(stream),buffer,length);
}
void NodeName_HostWrite(void *stream,const void *buffer,uint32_t length)
{
    // Write caller-owned plugin bytes to the original live SDK stream
    RwStreamWrite(static_cast<RwStream*>(stream),buffer,length);
}
//- rouz edit (ChatGPT)
'''

def node_name_calls(source):
    # Convert only the diagnosed complete producer and its existing two-operation public header
    if 'static int32 gPluginOffset;' in source and 'NodeNameStreamGetSize(' in source:return kernel(source)
    if 'char *GetFrameNodeName(RwFrame *frame);' in source and 'NodeNamePluginAttach(void);' in source:return header()
    return source

def node_name_project(source):
    # Select the complete C plugin producer and add its distinct native primitive unit
    source=source.replace('src\\rw\\NodeName.cpp','src\\rw\\NodeName.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\NodeName\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain all other metadata while explicitly selecting strict C11 compilation
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';native=match[1][:-len('NodeName.c')]+'NodeName_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\rw\\NodeName\.h)"\s*/>',source)
    if anchor:
        # Keep the native boundary declarations beside the self-contained public C API
        entry='<ClInclude Include="'+anchor[1][:-len('NodeName.h')]+'NodeName_backend.h" />'
        if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)



def calls(source):
    # Preserve complete C frame-name ownership before moving complete collision-model lifetime and storage into C
    import col_model_migration as models
    return models.calls(node_name_calls(source))


def project(source):
    # Retain C frame-name metadata before selecting the C collision-model producer and native primitives
    import col_model_migration as models
    return models.project(node_name_project(source))
