"""Move complete texture dictionary ownership and loading policy into C."""
import re
from rect_migration import mask,closing
from memory_heap_migration import body

RENAMES={'src/rw/TxdStore.cpp':'src/rw/TxdStore.c'}
OPERATIONS=[
    ('void','Initialise','void','Initialise(void)'),
    ('void','Shutdown','void','Shutdown(void)'),
    ('void','GameShutdown','void','GameShutdown(void)'),
    ('int','AddTxdSlot','const char *name','AddTxdSlot(const char *name)'),
    ('void','RemoveTxdSlot','int slot','RemoveTxdSlot(int slot)'),
    ('int','FindTxdSlot','const char *name','FindTxdSlot(const char *name)'),
    ('char *','GetTxdName','int slot','GetTxdName(int slot)'),
    ('void','PushCurrentTxd','void','PushCurrentTxd(void)'),
    ('void','PopCurrentTxd','void','PopCurrentTxd(void)'),
    ('void','SetCurrentTxd','int slot','SetCurrentTxd(int slot)'),
    ('void','Create','int slot','Create(int slot)'),
    ('int','GetNumRefs','int slot','GetNumRefs(int slot)'),
    ('void','AddRef','int slot','AddRef(int slot)'),
    ('void','RemoveRef','int slot','RemoveRef(int slot)'),
    ('void','RemoveRefWithoutDelete','int slot','RemoveRefWithoutDelete(int slot)'),
    ('bool','LoadStream','int slot,void *stream','LoadTxd(int slot, RwStream *stream)'),
    ('bool','LoadFile','int slot,const char *filename','LoadTxd(int slot, const char *filename)'),
    ('bool','StartLoadTxd','int slot,void *stream','StartLoadTxd(int slot, RwStream *stream)'),
    ('bool','FinishLoadTxd','int slot,void *stream','FinishLoadTxd(int slot, RwStream *stream)'),
    ('void','RemoveTxd','int slot','RemoveTxd(int slot)'),
]
HOST=[
    ('void','DestroyDictionary','void *dictionary','RwTexDictionaryDestroy((RwTexDictionary*)dictionary)'),
    ('void *','GetCurrent','void','RwTexDictionaryGetCurrent()'),
    ('void','SetCurrent','void *dictionary','RwTexDictionarySetCurrent((RwTexDictionary*)dictionary)'),
    ('void *','CreateDictionary','void','RwTexDictionaryCreate()'),
    ('void','RemoveStreamingTxd','int slot','CStreaming::RemoveTxd(slot)'),
    ('bool','FindDictionaryChunk','void *stream','RwStreamFindChunk((RwStream*)stream,rwID_TEXDICTIONARY,NULL,NULL)!=NULL'),
    ('void *','ReadDictionary','void *stream','RwTexDictionaryGtaStreamRead((RwStream*)stream)'),
    ('void *','ReadDictionary1','void *stream','RwTexDictionaryGtaStreamRead1((RwStream*)stream)'),
    ('void *','ReadDictionary2','void *stream,void *dictionary','RwTexDictionaryGtaStreamRead2((RwStream*)stream,(RwTexDictionary*)dictionary)'),
    ('void','EnableRasterConversion','bool enabled','_rwD3D8TexDictionaryEnableRasterFormatConversion(enabled)'),
    ('void *','OpenFileStream','const char *filename','RwStreamOpen(rwSTREAMFILENAME,rwSTREAMREAD,filename)'),
    ('void','CloseStream','void *stream','RwStreamClose((RwStream*)stream,NULL)'),
]

def header():
    # Expose flat slot records and SDK-independent operations with separate names for the former overloads
    value='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
typedef struct TxdDef {
    void *texDict;
    int refCount;
    char name[20];
} TxdDef;
#ifdef __cplusplus
extern "C" {
#endif
'''
    value+=''.join(result+' TxdStore_'+name+'('+args+');\n' for result,name,args,_ in OPERATIONS)
    value+='TxdDef *TxdStore_GetSlot(int slot);\nbool TxdStore_isTxdLoaded(int slot);\n'
    return value+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def backend_header():
    # Keep only external SDK and streaming primitives outside the C owner
    value='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    value+=''.join(result+' TxdStore_Host'+name+'('+args+');\n' for result,name,args,_ in HOST)
    return value+'void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def transform(value):
    # Preserve each complete original body and replace only explicit owner and SDK boundaries
    mapping={
        'RwStreamFindChunk(stream, rwID_TEXDICTIONARY, nil, nil)':'TxdStore_HostFindDictionaryChunk(stream)',
        'RwStreamOpen(rwSTREAMFILENAME, rwSTREAMREAD, filename)':'TxdStore_HostOpenFileStream(filename)',
        'RwStreamClose(stream, nil)':'TxdStore_HostCloseStream(stream)',
        'RwTexDictionaryDestroy':'TxdStore_HostDestroyDictionary',
        'RwTexDictionaryGetCurrent':'TxdStore_HostGetCurrent',
        'RwTexDictionarySetCurrent':'TxdStore_HostSetCurrent',
        'RwTexDictionaryCreate':'TxdStore_HostCreateDictionary',
        'CStreaming::RemoveTxd':'TxdStore_HostRemoveStreamingTxd',
        'RwTexDictionaryGtaStreamRead2':'TxdStore_HostReadDictionary2',
        'RwTexDictionaryGtaStreamRead1':'TxdStore_HostReadDictionary1',
        'RwTexDictionaryGtaStreamRead':'TxdStore_HostReadDictionary',
        '_rwD3D8TexDictionaryEnableRasterFormatConversion':'TxdStore_HostEnableRasterConversion',
        'ms_pTxdPool':'TxdStore_pool','ms_pStoredTxd':'TxdStore_stored',
        'RwStream *stream':'void *stream',
    }
    for old,new in mapping.items():value=value.replace(old,new)
    names=[name for _,name,_,_ in OPERATIONS if name not in ['LoadFile','LoadStream']]+['GetSlot']
    value=re.sub(r'(?<![\w:])\b('+'|'.join(names)+r')\(',lambda match:'TxdStore_'+match[1]+'(',value)
    value=value.replace('LoadTxd(slot, stream)','TxdStore_LoadStream(slot, stream)')
    return re.sub(r'\bnil\b','NULL',value)

def kernel(original,original_header):
    # Keep the real C pool and original allocator macros without a C++ facade or synthetic pool algorithm
    value='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "TxdStore.h"
#include "TxdStore_backend.h"
#include "General.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#include "Pool.h"
static CPool *TxdStore_pool;
static void *TxdStore_stored;
'''
    value+='TxdDef *TxdStore_GetSlot(int slot)\n{\n    // Preserve original slot assertions and live pool lookup\n'+transform(body(original_header,'static TxdDef *GetSlot(int slot)'))+'\n}\n'
    for result,name,args,signature in OPERATIONS:
        # Move every complete original operation and retain its mutation and callback ordering
        value+=result+' TxdStore_'+name+'('+args+')\n{\n    // Preserve the complete original '+name+' operation and live mutation order\n'+transform(body(original,'CTxdStore::'+signature))+'\n}\n'
    return value+'//- rouz edit (ChatGPT)\n'

def native():
    # Bind SDK primitives without moving dictionary storage, retry loops or reference policy into C++
    value='//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "Streaming.h"\n#include "RwHelper.h"\n#include "TxdStore_backend.h"\n'
    for result,name,args,expression in HOST:
        # Forward exactly one original external operation at its original boundary
        value+=result+' TxdStore_Host'+name+'('+args+')\n{\n    // Invoke the original external owner primitive with the same borrowed arguments\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
    return value+'//- rouz edit (ChatGPT)\n'

BINDING_PATHS=['src/renderer/BulletTraces.cpp','src/renderer/Clouds.cpp','src/renderer/Markers3D.cpp','src/renderer/PlayerSkin.cpp','src/renderer/Rubbish.cpp','src/renderer/Skidmarks.cpp']

def forwarder_definitions(source):
    # Select only whole public forwarding functions containing one original TXD operation
    code=mask(source);result=[]
    for match in re.finditer(r'(?m)^[\w *]+\b(\w+_Backend\w+)\([^\n]*\)\n\{',code):
        end=match.end();depth=1
        while depth:
            if code[end]=='{':depth+=1
            elif code[end]=='}':depth-=1
            end+=1
        if re.fullmatch(r'\s*(?:return\s+)?CTxdStore::\w+\([^;]*\);\s*',code[match.end():end-1]):
            result.append((match.start(),end,source[match.start():end],match[1]))
    return result

def consumer_calls(source):
    # Give former overloads explicit names based on the complete inventoried stream and filename arguments
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bCTxdStore::LoadTxd\(',code))):
        end=closing(code,match.end()-1);arguments=code[match.end():end-1];depth=0;commas=[]
        for index,char in enumerate(arguments):
            if char in '([{':depth+=1
            elif char in ')]}':depth-=1
            elif char==',' and depth==0:commas.append(index)
        assert len(commas)==1,source[match.start():end]
        argument=source[match.end()+commas[0]+1:end-1].strip()
        assert argument=='stream' or argument.startswith('"') or argument in ['str','filename','txdPath','controllerTypesPaths[type]'],argument
        name='TxdStore_LoadStream' if argument=='stream' else 'TxdStore_LoadFile'
        source=source[:match.start()]+name+source[match.end()-1:]
    source=re.sub(r'\bCTxdStore::(\w+)',lambda match:'TxdStore_'+match[1],source)
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bTxdStore_GetSlot\(',code))):
        end=closing(code,match.end()-1)
        field=re.match(r'\s*->texDict\b',code[end:])
        if field:
            end+=field.end()
            source=source[:match.start()]+'((RwTexDictionary*)'+source[match.start():end]+')'+source[end:]
    return source

def bindings_header(originals):
    # Retain all existing caller ABIs with standalone C declarations
    definitions=[definition for original in originals for _,_,definition,_ in forwarder_definitions(original)]
    assert len(definitions)==27,len(definitions)
    value='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    value+=''.join(definition[:definition.index('\n{')]+';\n' for definition in definitions)
    return value+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def bindings(originals):
    # Move all original forwarding definitions to C without routing C policy through C++
    value='//+ rouz edit (ChatGPT)\n#include "TxdStore.h"\n#include "TxdStore_bindings.h"\n'
    definitions=[consumer_calls(definition) for original in originals for _,_,definition,_ in forwarder_definitions(original)]
    assert len(definitions)==27,len(definitions)
    return value+'\n'.join(definitions)+'\n//- rouz edit (ChatGPT)\n'

def txd_store_calls(source):
    # Move the complete owner and every whole caller onto explicit C operations
    if 'CTxdStore::Initialise(void)' in source:
        from txd_store_original_header import ORIGINAL_HEADER
        return kernel(source,ORIGINAL_HEADER)
    if 'class CTxdStore\n' in source:return header()
    original=source
    for start,end,_,_ in reversed(forwarder_definitions(source)):
        source=source[:start]+source[end:]
    if 'CTxdStore::' in mask(source):
        source=consumer_calls(source)
        lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True)
        for index in reversed(range(len(lines))):
            if re.search(r'\bTxdStore_\w+\(',code[index]):
                indent=re.match(r'\s*',lines[index])[0]
                lines.insert(index,indent+'// Use the complete C texture dictionary owner with the original live arguments\n')
        source=''.join(lines)
    if source!=original:return '//+ rouz edit (ChatGPT)\n'+source+'//- rouz edit (ChatGPT)\n'
    return source

def txd_store_project(source):
    # Compile both TXD C units independently while keeping the native SDK leaves in their own unit
    import object_names_migration as objects
    source=source.replace('src\\rw\\TxdStore.cpp','src\\rw\\TxdStore.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\TxdStore\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing metadata and add explicit C11 settings for both complete C units
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>'
            prefix=match[1][:-len('TxdStore.c')]
            for name in ['TxdStore_native.cpp','TxdStore_bindings.c']:
                if '<ClCompile Include="'+prefix+name+'"' not in source:
                    result+='\n    <ClCompile Include="'+prefix+name+'"'
                    result+=('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if name.endswith('.c') else ' />')
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\rw\\TxdStore\.h)"\s*/>',source)
    if anchor:
        for name in ['TxdStore_backend.h','TxdStore_bindings.h']:
            entry='<ClInclude Include="'+anchor[1][:-len('TxdStore.h')]+name+'" />'
            if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)


def calls(source):
    # Retain complete dictionary-store ownership before applying the complete C texture successor
    import tex_read_migration as texture
    return texture.calls(txd_store_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting both complete C texture units
    import tex_read_migration as texture
    return texture.project(txd_store_project(source))
