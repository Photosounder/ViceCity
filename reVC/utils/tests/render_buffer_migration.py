"""Move all shared-buffer algorithms and counters into C without changing typed storage."""
import re
import object_names_migration as objects
import motion_blur_streaks_migration as motion_blur_streaks

PUBLIC='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#define TEMPBUFFERVERTSIZE 512
#define TEMPBUFFERINDEXSIZE 1024
#ifdef __cplusplus
extern "C" {
#endif
extern int32_t TempBufferVerticesStored;
extern int32_t TempBufferIndicesStored;
extern int32_t RenderBuffer_VerticesToBeStored;
extern int32_t RenderBuffer_IndicesToBeStored;
extern uint16_t TempBufferRenderIndexList[TEMPBUFFERINDEXSIZE];
void RenderBuffer_ClearRenderBuffer(void);
void RenderBuffer_StartStoring(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void **vertexStart);
void RenderBuffer_StartStoringLegacy(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void *vertexStart);
void RenderBuffer_StopStoring(void);
void RenderBuffer_RenderStuffInBuffer(void);
#ifdef __cplusplus
}
'''
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "RenderBuffer.h"
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void *RenderBuffer_BackendVertex(int32_t index);
void RenderBuffer_BackendWriteVertex(void *output,int32_t index);
bool RenderBuffer_BackendTransform(int32_t vertices);
void RenderBuffer_BackendRenderIndexed(int32_t indices);
void RenderBuffer_BackendEnd(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "RenderBuffer_backend.h"
VertexBufferUnion TempVertexBuffer;
void *RenderBuffer_BackendVertex(int32_t index)
{
    // Address the actual typed vertex array without guessing its stride
    return &TempBufferRenderVertices[index];
}
void RenderBuffer_BackendWriteVertex(void *output,int32_t index)
{
    // Publish the original typed pointer at the C algorithm's ordered write point
    *(RwIm3DVertex **)output=&TempBufferRenderVertices[index];
}
bool RenderBuffer_BackendTransform(int32_t vertices)
{
    // Preserve the original typed buffer, count, null matrix and UV flags
    return RwIm3DTransform(TempBufferRenderVertices,vertices,nil,rwIM3D_VERTEXUV)!=nil;
}
void RenderBuffer_BackendRenderIndexed(int32_t indices)
{
    // Render the original shared index array using the live C-selected count
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,TempBufferRenderIndexList,indices);
}
void RenderBuffer_BackendEnd(void)
{
    // Complete the original successful immediate draw
    RwIm3DEnd();
}
//- rouz edit (ChatGPT)
'''
HELPERS='''typedef void (*RenderBufferWriter)(void *,int32_t);
static void RenderBuffer_WriteC(void *output,int32_t index)
{
    // Publish an opaque C vertex handle before updating pending reservation counts
    *(void **)output=RenderBuffer_BackendVertex(index);
}
'''
WRAPPERS='''void RenderBuffer_StartStoring(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void **vertexStart)
{
    // Enter the complete reservation algorithm with a legal C pointer output
    RenderBuffer_Reserve(numIndices,numVertices,indexStart,vertexStart,RenderBuffer_WriteC);
}
void RenderBuffer_StartStoringLegacy(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void *vertexStart)
{
    // Write actual legacy pointer storage through its typed owner operation
    RenderBuffer_Reserve(numIndices,numVertices,indexStart,vertexStart,RenderBuffer_BackendWriteVertex);
}
'''

def header(source):
    # Retain both original typed arrays and their original independent struct storage
    tail=source[source.index('struct VertexBufferUnion'):]
    tail=re.sub(r'^extern int32 TempBuffer(?:Vertices|Indices)Stored;\n','',tail,flags=re.M)
    tail=re.sub(r'^extern RwImVertexIndex TempBufferRenderIndexList\[TEMPBUFFERINDEXSIZE\];\n?','',tail,flags=re.M)
    return PUBLIC+tail.rstrip()+'\n#endif\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Rename actual class references and preserve all legacy output expressions
    if 'class RenderBuffer\n' in source:return header(source)
    if 'int RenderBuffer::VerticesToBeStored;' in source:return HOST
    # Preserve the helper's exact formula when its optional declaration is unavailable
    if 'TEXTURE_ADDU' in source:source=source.replace('float timeStep = Timer_GetTimeStepFix();','// Preserve the original normalized time step without the optional helper declaration\n\t\tfloat timeStep = (*Timer_GetTimeStep()) / (50.0f / 30.0f);')
    changed='RenderBuffer::' in source
    source=source.replace('RenderBuffer::StartStoring(','RenderBuffer_StartStoringLegacy(').replace('RenderBuffer::','RenderBuffer_')
    if changed:
        lines=[]
        for line in source.splitlines(keepends=True):
            if re.search(r'\bRenderBuffer_(?:ClearRenderBuffer|StartStoringLegacy|StopStoring|RenderStuffInBuffer)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the C shared-buffer algorithm with the original arguments\n')
            lines.append(line)
        source=''.join(lines)
    return motion_blur_streaks.calls(source)

def kernel(source):
    # Relocate every complete algorithm while keeping typed array arithmetic in the backend
    source=source[source.index('int32 TempBufferVerticesStored;'):]
    source=source.replace('VertexBufferUnion TempVertexBuffer;\n','').replace('RwImVertexIndex TempBufferRenderIndexList','uint16_t TempBufferRenderIndexList')
    source=source.replace('int RenderBuffer::VerticesToBeStored;','int32_t RenderBuffer_VerticesToBeStored;').replace('int RenderBuffer::IndicesToBeStored;','int32_t RenderBuffer_IndicesToBeStored;')
    source=source.replace('void\nRenderBuffer::StartStoring(int numIndices, int numVertices, RwImVertexIndex **indexStart, RwIm3DVertex **vertexStart)','static void\nRenderBuffer_Reserve(int32_t numIndices,int32_t numVertices,uint16_t **indexStart,void *vertexStart,RenderBufferWriter write)')
    source=source.replace('RenderBuffer::','RenderBuffer_')
    source=source.replace('*vertexStart = &TempBufferRenderVertices[TempBufferVerticesStored];','write(vertexStart,TempBufferVerticesStored);')
    source=re.sub(r'(?<![\w])IndicesToBeStored\b','RenderBuffer_IndicesToBeStored',source)
    source=re.sub(r'(?<![\w])VerticesToBeStored\b','RenderBuffer_VerticesToBeStored',source)
    source=re.sub(r'(?<![\w])RenderStuffInBuffer\(','RenderBuffer_RenderStuffInBuffer(',source)
    source=re.sub(r'(?<![\w])ClearRenderBuffer\(','RenderBuffer_ClearRenderBuffer(',source)
    source=source.replace('RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXUV)','RenderBuffer_BackendTransform(TempBufferVerticesStored)').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored)','RenderBuffer_BackendRenderIndexed(TempBufferIndicesStored)').replace('RwIm3DEnd()','RenderBuffer_BackendEnd()')
    source=re.sub(r'\bint32\b','int32_t',source)
    source=source.replace('\n{\n','\n{\n\t// Preserve original live counters, short circuits and write order\n')
    source=source.replace(')){\n', ')){\n\t\t// Render and end only after the original transform succeeds\n')
    assert 'RenderBuffer::' not in source and 'RwIm' not in source
    return '//+ rouz edit (ChatGPT)\n#include "RenderBuffer_backend.h"\n\n'+HELPERS+source.rstrip()+'\n\n'+WRAPPERS+'//- rouz edit (ChatGPT)\n'

def project(source):
    # Register the C11 unit and preserve unique C/C++ object paths
    if '..\\src\\renderer\\RenderBuffer.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\RenderBuffer.cpp" />';addition='<ClCompile Include="..\\src\\renderer\\RenderBuffer.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\renderer\\RenderBuffer.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('RenderBuffer.cpp','RenderBuffer.c'))
    if '..\\src\\renderer\\RenderBuffer_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\RenderBuffer.h" />','<ClInclude Include="..\\src\\renderer\\RenderBuffer.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('RenderBuffer.h','RenderBuffer_backend.h'))
    return motion_blur_streaks.project(objects.project(source))
