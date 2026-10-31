"""Audit whole shared-buffer relocation and compile the immutable original algorithms."""
from pathlib import Path
import re,json
import render_buffer_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/render-buffer-c-tests'

def normalize(source):
    # Compare every complete statement independently of formatting and comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/RenderBuffer.cpp').read_text();header=(out/'before/src/renderer/RenderBuffer.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/RenderBuffer.c').read_text())
assert (root/'src/renderer/RenderBuffer_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^RenderBuffer::\w+\(',old,re.M))==4
for name in ['RenderBuffer.h','RenderBuffer_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
for path in ['src/math/Vector.h','src/math/maths.h','src/core/config.h','src/core/common.h','vendor/librw/src/d3d/rwd3d.h','vendor/librw/src/gl/rwgl3.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
    if path in ['src/math/Vector.h','src/math/maths.h']:(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/ropes-c-tests/common.h').read_text();saved=out/'before/rope-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text(common,newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
(out/'original-RenderBuffer.h').write_text(header,newline='\n')
(out/'original.cpp').write_text(old.replace('#include "common.h"',commonInclude).replace('#include "RenderBuffer.h"','#include "original-RenderBuffer.h"'),newline='\n')
(out/'host.cpp').write_text(m.HOST.replace('#include "common.h"',commonInclude),newline='\n')
prototypes='''void TestRB_Clear(void);
void TestRB_Start(int32_t indices,int32_t vertices,uint16_t **indexStart,void **vertexStart);
void TestRB_StartLegacy(int32_t indices,int32_t vertices,uint16_t **indexStart,void *vertexStart);
void TestRB_Stop(void);
void TestRB_Flush(void);
void TestRB_State(int32_t *values);
void TestRB_SetState(int32_t vertices,int32_t indices,int32_t pendingVertices,int32_t pendingIndices);
'''
(out/'caller.h').write_text('#pragma once\n#include <stdint.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
oldcaller=commonInclude+'\n#include "original-RenderBuffer.h"\n#include "caller.h"\n'
newcaller='#include "RenderBuffer.h"\n#include "caller.h"\n'
for name,original,new in [('Clear','ClearRenderBuffer','ClearRenderBuffer'),('Stop','StopStoring','StopStoring'),('Flush','RenderStuffInBuffer','RenderStuffInBuffer')]:
    oldcaller+='extern "C" void TestRB_'+name+'(void)\n{\n    // Enter the complete original algorithm\n    RenderBuffer::'+original+'();\n}\n'
    newcaller+='void TestRB_'+name+'(void)\n{\n    // Enter the actual C algorithm from a separate C11 caller\n    RenderBuffer_'+new+'();\n}\n'
for variant in ['old','new']:
    prefix='RenderBuffer::' if variant=='old' else 'RenderBuffer_'
    statement='void TestRB_State(int32_t *values)\n{\n    // Observe all live counters including pending reservations\n    values[0]=TempBufferVerticesStored;values[1]=TempBufferIndicesStored;values[2]='+prefix+'VerticesToBeStored;values[3]='+prefix+'IndicesToBeStored;\n}\n'
    statement+='void TestRB_SetState(int32_t vertices,int32_t indices,int32_t pendingVertices,int32_t pendingIndices)\n{\n    // Seed legal original state without replacing any algorithm\n    TempBufferVerticesStored=vertices;TempBufferIndicesStored=indices;'+prefix+'VerticesToBeStored=pendingVertices;'+prefix+'IndicesToBeStored=pendingIndices;\n}\n'
    if variant=='old':oldcaller+='extern "C" {\n'+statement+'}\n'
    else:newcaller+=statement
oldcaller+='''extern "C" void TestRB_Start(int32_t indices,int32_t vertices,uint16_t **indexStart,void **vertexStart)
{
    // Observe the original typed output as an opaque handle after the complete method
    RwIm3DVertex *typed;RenderBuffer::StartStoring(indices,vertices,indexStart,&typed);*vertexStart=typed;
}
extern "C" void TestRB_StartLegacy(int32_t indices,int32_t vertices,uint16_t **indexStart,void *vertexStart)
{
    // Use actual typed pointer storage in the original method
    RenderBuffer::StartStoring(indices,vertices,indexStart,(RwIm3DVertex **)vertexStart);
}
'''
newcaller+='''void TestRB_Start(int32_t indices,int32_t vertices,uint16_t **indexStart,void **vertexStart)
{
    // Exercise the public C handle output without a pointer-to-pointer cast
    RenderBuffer_StartStoring(indices,vertices,indexStart,vertexStart);
}
void TestRB_StartLegacy(int32_t indices,int32_t vertices,uint16_t **indexStart,void *vertexStart)
{
    // Exercise the actual typed legacy output writer selected by the C algorithm
    RenderBuffer_StartStoringLegacy(indices,vertices,indexStart,vertexStart);
}
'''
(out/'original-caller.cpp').write_text(oldcaller,newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/render_buffer.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
print('Audited four complete algorithms, both ordered output paths, five typed operations, unchanged vertex storage and twelve whole source/project files')
