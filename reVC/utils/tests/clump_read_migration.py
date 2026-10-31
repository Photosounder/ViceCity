"""Move complete two-stage clump and geometry loading policy into C."""
import re
from rect_migration import mask

RENAMES={'src/rw/ClumpRead.cpp':'src/rw/ClumpRead.c'}
OPERATIONS=['GeometryListStreamRead1','GeometryListStreamRead2','GeometryListDeinitialize','ClumpAtomicStreamRead','RpClumpGtaStreamRead1','RpClumpGtaStreamRead2','RpClumpGtaCancelStream']
NAMES=dict(zip(OPERATIONS,['GeometryListStreamRead1','GeometryListStreamRead2','GeometryListDeinitialize','AtomicStreamRead','Start','Finish','Cancel']))
HOST=[
    ('void *','Allocate','size_t size','RwMalloc(size)'),
    ('void','Free','void *memory','RwFree(memory)'),
    ('uint32_t','Read','void *stream,void *output,uint32_t size','RwStreamRead((RwStream*)stream,output,size)'),
    ('void *','ReadGeometry','void *stream','RpGeometryStreamRead((RwStream*)stream)'),
    ('void','DestroyGeometry','void *geometry','RpGeometryDestroy((RpGeometry*)geometry)'),
    ('void *','CreateAtomic','void','RpAtomicCreate()'),
    ('void','DestroyAtomic','void *atomic','RpAtomicDestroy((RpAtomic*)atomic)'),
    ('void','AtomicFlags','void *atomic,int32_t flags','RpAtomicSetFlags((RpAtomic*)atomic,flags)'),
    ('void','AtomicFrame','void *atomic,void *frame','RpAtomicSetFrame((RpAtomic*)atomic,(RwFrame*)frame)'),
    ('void','AtomicGeometry','void *atomic,void *geometry,uint32_t flags','RpAtomicSetGeometry((RpAtomic*)atomic,(RpGeometry*)geometry,flags)'),
    ('void *','Frames','void','&nativeFrames'),
    ('int32_t','FrameCount','void *frames','((rwFrameList*)frames)->numFrames'),
    ('void *','FrameAt','void *frames,int32_t index','((rwFrameList*)frames)->frames[index]'),
    ('void','SetFrameCount','void *frames,int32_t count','((rwFrameList*)frames)->numFrames=count'),
    ('void *','ReadFrames','void *stream,void *frames','rwFrameListStreamRead((RwStream*)stream,(rwFrameList*)frames)'),
    ('void','DeinitializeFrames','void *frames','rwFrameListDeinitialize((rwFrameList*)frames)'),
    ('ClumpReadStreamPosition','Position','void *stream','STREAMPOS((RwStream*)stream)'),
    ('void','Skip','void *stream,uint32_t offset','RwStreamSkip((RwStream*)stream,offset)'),
    ('void *','CreateClump','void','RpClumpCreate()'),
    ('void','DestroyClump','void *clump','RpClumpDestroy((RpClump*)clump)'),
    ('void','ClumpFrame','void *clump,void *frame','RpClumpSetFrame((RpClump*)clump,(RwFrame*)frame)'),
    ('void','AddAtomic','void *clump,void *atomic','RpClumpAddAtomic((RpClump*)clump,(RpAtomic*)atomic)'),
]

def header():
    # Expose complete C operations and the original application-owned record fields without SDK types
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct ClumpReadGeometryList {void **geometries;int32_t numGeoms;} ClumpReadGeometryList;
typedef struct ClumpReadAtomicBinary {int32_t frameIndex,geomIndex,flags,unused;} ClumpReadAtomicBinary;
typedef struct ClumpReadChunkInfo {int32_t numAtomics,numLights,numCameras;} ClumpReadChunkInfo;
#ifdef __cplusplus
extern "C" {
#endif
ClumpReadGeometryList *ClumpRead_GeometryListStreamRead1(void *stream,ClumpReadGeometryList *geomlist);
ClumpReadGeometryList *ClumpRead_GeometryListStreamRead2(void *stream,ClumpReadGeometryList *geomlist);
void ClumpRead_GeometryListDeinitialize(ClumpReadGeometryList *geomlist);
void *ClumpRead_AtomicStreamRead(void *stream,void *frames,ClumpReadGeometryList *geometries);
bool ClumpRead_Start(void *stream);
void *ClumpRead_Finish(void *stream);
void ClumpRead_Cancel(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Borrow the actual SDK frame-list object and keep its live mutations visible through native metadata primitives
    value='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stddef.h>\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    value+='enum {CLUMPREAD_STRUCT,CLUMPREAD_GEOMETRY,CLUMPREAD_FRAMELIST,CLUMPREAD_GEOMETRYLIST,CLUMPREAD_ATOMIC};\n'
    value+='#ifdef LIBRW\ntypedef int32_t ClumpReadStreamPosition;\n#else\ntypedef uint32_t ClumpReadStreamPosition;\n#endif\n'
    value+='bool ClumpRead_HostFindChunk(void *stream,uint32_t kind,uint32_t *size,uint32_t *version);\n'
    value+=''.join(result+' ClumpRead_Host'+name+'('+args+');\n' for result,name,args,_ in HOST)
    return value+'void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def kernel(original):
    # Preserve every complete original body while keeping only SDK object storage and external operations native
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=re.sub(r'struct rpGeometryList\n\{[\s\S]*?\n\};\n','',source)
    source=re.sub(r'struct rpAtomicBinary\n\{[\s\S]*?\n\};\n','',source)
    source=source.replace('static rwFrameList gFrameList;\n','')
    source=source.replace('gFrameList.frames[0]','ClumpRead_HostFrameAt(ClumpRead_HostFrames(),0)')
    source=source.replace('gFrameList.numFrames = 0','ClumpRead_HostSetFrameCount(ClumpRead_HostFrames(),0)')
    source=source.replace('&gFrameList','ClumpRead_HostFrames()')
    source=source.replace('frmList->frames[a.frameIndex]','ClumpRead_HostFrameAt(frmList,a.frameIndex)')
    source=source.replace('frmList->numFrames','ClumpRead_HostFrameCount(frmList)')
    for old,new in [('rwID_STRUCT','CLUMPREAD_STRUCT'),('rwID_GEOMETRYLIST','CLUMPREAD_GEOMETRYLIST'),('rwID_GEOMETRY','CLUMPREAD_GEOMETRY'),('rwID_FRAMELIST','CLUMPREAD_FRAMELIST'),('rwID_ATOMIC','CLUMPREAD_ATOMIC')]:
        # Convert only SDK chunk-kind identities at the native boundary without inventing serialized values
        source=re.sub(r'\b'+old+r'\b',new,source)
    mapping={'RwStreamFindChunk':'ClumpRead_HostFindChunk','RwStreamRead':'ClumpRead_HostRead','RwMalloc':'ClumpRead_HostAllocate','RwFree':'ClumpRead_HostFree','RpGeometryStreamRead':'ClumpRead_HostReadGeometry','RpGeometryDestroy':'ClumpRead_HostDestroyGeometry','RpAtomicCreate':'ClumpRead_HostCreateAtomic','RpAtomicDestroy':'ClumpRead_HostDestroyAtomic','RpAtomicSetFlags':'ClumpRead_HostAtomicFlags','RpAtomicSetFrame':'ClumpRead_HostAtomicFrame','RpAtomicSetGeometry':'ClumpRead_HostAtomicGeometry','rwFrameListStreamRead':'ClumpRead_HostReadFrames','rwFrameListDeinitialize':'ClumpRead_HostDeinitializeFrames','STREAMPOS':'ClumpRead_HostPosition','RwStreamSkip':'ClumpRead_HostSkip','RpClumpCreate':'ClumpRead_HostCreateClump','RpClumpDestroy':'ClumpRead_HostDestroyClump','RpClumpSetFrame':'ClumpRead_HostClumpFrame','RpClumpAddAtomic':'ClumpRead_HostAddAtomic'}
    for old,new in mapping.items():source=re.sub(r'\b'+old+r'\b',new,source)
    for old,new in [('rpGeometryList','ClumpReadGeometryList'),('rpAtomicBinary','ClumpReadAtomicBinary'),('RpClumpChunkInfo','ClumpReadChunkInfo'),('rwFrameList','void'),('RwStream','void'),('RpGeometry','void'),('RpAtomic','void'),('RpClump','void'),('RwUInt32','uint32_t'),('RwInt32','int32_t'),('int32','int32_t'),('nil','NULL')]:
        # Retain primitive widths and opaque SDK pointer representations
        source=re.sub(r'\b'+old+r'\b',new,source)
    for old,new in NAMES.items():source=re.sub(r'\b'+old+r'\b','ClumpRead_'+new,source)
    for name in NAMES.values():
        # Describe each whole converted operation before its original control flow
        source=re.sub(r'(?m)^(ClumpRead_'+name+r'\([^\n]*\)\n\{\n)',lambda match:match[1]+'    // Preserve the complete original '+name+' operation and live mutation order\n',source)
    prefix='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "ClumpRead.h"
#include "ClumpRead_backend.h"
#include <string.h>
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
'''
    return prefix+source+'\n//- rouz edit (ChatGPT)\n'

def native():
    # Keep the same actual SDK frame-list instance rather than copying a partially mutated record across callbacks
    value='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "ClumpRead_backend.h"
static rwFrameList nativeFrames;
bool ClumpRead_HostFindChunk(void *stream,uint32_t kind,uint32_t *size,uint32_t *version)
{
    // Translate the reader's chunk identity to the original SDK constant for one native lookup
    static const RwUInt32 ids[]={rwID_STRUCT,rwID_GEOMETRY,rwID_FRAMELIST,rwID_GEOMETRYLIST,rwID_ATOMIC};
    return RwStreamFindChunk((RwStream*)stream,ids[kind],size,version)!=0;
}
'''
    for result,name,args,expression in HOST:
        # Forward one SDK operation or live record access without moving clump loading decisions out of C
        value+=result+' ClumpRead_Host'+name+'('+args+')\n{\n    // Preserve the original external SDK primitive and its live borrowed arguments\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
    return value+'//- rouz edit (ChatGPT)\n'

def clump_read_calls(source):
    # Move the complete owner and all native clients onto the explicit C API
    if 'GeometryListStreamRead1(RwStream *stream, rpGeometryList *geomlist)' in source:return kernel(source)
    if 'bool RpClumpGtaStreamRead1(RwStream *stream);' in source:
        start=source.index('bool RpClumpGtaStreamRead1(RwStream *stream);');end=source.index('void RpClumpGtaCancelStream(void);',start)+len('void RpClumpGtaCancelStream(void);')
        return '//+ rouz edit (ChatGPT)\n'+source[:start]+'#include "ClumpRead.h"'+source[end:]+'//- rouz edit (ChatGPT)\n'
    original=source
    source=source.replace('RpClumpGtaStreamRead1(','ClumpRead_Start(').replace('RpClumpGtaCancelStream(','ClumpRead_Cancel(')
    code=mask(source)
    from rect_migration import closing
    for match in reversed(list(re.finditer(r'\bRpClumpGtaStreamRead2\(',code))):
        # Keep each caller's original SDK pointer type without retaining a C++ algorithm wrapper
        end=closing(code,match.end()-1);value='((RpClump*)ClumpRead_Finish'+source[match.end()-1:end]+')'
        source=source[:match.start()]+value+source[end:]
    if source!=original:
        lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True)
        for index in reversed(range(len(lines))):
            if re.search(r'\bClumpRead_(?:Start|Finish|Cancel)\(',code[index]):
                lines.insert(index,re.match(r'\s*',lines[index])[0]+'// Use the complete C clump reader with the original live stream state\n')
        return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'
    return source

def clump_read_project(source):
    # Compile complete clump policy as C11 and keep SDK data and primitives in their separate native unit
    import object_names_migration as objects
    source=source.replace('src\\rw\\ClumpRead.cpp','src\\rw\\ClumpRead.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\ClumpRead\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing per-file metadata while selecting strict C11 for the complete owner
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>'
            native=match[1][:-len('ClumpRead.c')]+'ClumpRead_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\rw\\RwHelper\.h)"\s*/>',source)
    if anchor:
        for name in ['ClumpRead.h','ClumpRead_backend.h']:
            entry='<ClInclude Include="'+anchor[1][:-len('RwHelper.h')]+name+'" />'
            if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)


def calls(source):
    # Retain complete clump ownership before applying the complete C lighting successor
    import lights_migration as lights
    return lights.calls(clump_read_calls(source))

def project(source):
    # Retain preceding C metadata before selecting C lighting and its native SDK unit
    import lights_migration as lights
    return lights.project(clump_read_project(source))
