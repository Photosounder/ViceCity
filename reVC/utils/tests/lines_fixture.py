"""Audit whole source files and generate original-method versus mixed C11 fixtures."""
from pathlib import Path
import json,re
import lines_migration as m

root=Path(__file__).resolve().parents[2];out=root/'build/lines-c-tests'

def normalize(source):
    # Compare every statement while allowing only comments and formatting to differ
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text()
    expected=m.HOST if path.endswith('/Lines.cpp') else m.project(original) if path.startswith('build/') else m.calls(original)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Lines.cpp').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/Lines.c').read_text())
assert (root/'src/renderer/Lines_backend.h').read_text()==m.BACKEND_HEADER
assert b'\r' not in (root/'src/renderer/Lines.c').read_bytes()
counts={path:(out/'before'/path).read_text().count('CLines::RenderLineWithClipping(') for path in json.loads((out/'changes.json').read_text()) if path.endswith('.cpp') and not path.endswith('/Lines.cpp')}
assert sum(counts.values())==48,counts
for path,count in counts.items():assert (root/path).read_text().count('Lines_RenderLineWithClipping(')==count
assert 'CLines' not in (root/'src/renderer/Lines.h').read_text()
assert (root/'src/renderer/Lines.cpp').read_text().count('static RwIm3DVertex v[2];')==1

# Snapshot exact librw records and immediate-mode setters used by the original backend
dependencies=['vendor/librw/src/rwbase.h','vendor/librw/src/d3d/rwd3d.h','vendor/librw/src/gl/rwgl3.h','src/fakerw/fake.cpp','src/core/common.h']
for path in dependencies:
    target=out/'before'/path
    if not target.exists():target.parent.mkdir(parents=True,exist_ok=True);target.write_bytes((root/path).read_bytes())
rwbase=(out/'before/vendor/librw/src/rwbase.h').read_text()
records=re.search(r'struct RGBA\n\{.*?\n\};.*?inline RGBA makeRGBA[^\n]+',rwbase,re.S)[0]
vector=re.search(r'struct V3d\n\{.*?\n\};',rwbase,re.S)[0]
d3d=(out/'before/vendor/librw/src/d3d/rwd3d.h').read_text()
gl3=(out/'before/vendor/librw/src/gl/rwgl3.h').read_text()
vertices={name:re.search(r'struct Im3DVertex\n\{.*?\n\};',text,re.S)[0] for name,text in [('d3d',d3d),('gl3',gl3)]}
color=re.search(r'^#define COLOR_ARGB[^\n]+',d3d,re.M)[0]
square=re.search(r'inline float sq\(float x\)[^\n]+',(out/'before/src/core/common.h').read_text())[0]
common='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <math.h>
typedef uint32_t uint32;
namespace rw {
typedef uint8_t uint8;
typedef uint32_t uint32;
typedef int32_t int32;
typedef float float32;
typedef uint32_t bool32;
struct Matrix;
'''+records+'\n'+vector+'''
namespace d3d {
'''+color+'\n'+vertices['d3d']+'''
}
namespace gl3 {
'''+vertices['gl3']+'''
}
}
#ifdef TEST_GL3
typedef rw::gl3::Im3DVertex RwIm3DVertex;
#else
typedef rw::d3d::Im3DVertex RwIm3DVertex;
#endif
typedef rw::RGBA RwRGBA;
typedef rw::Matrix RwMatrix;
typedef float RwReal;
typedef uint8_t RwUInt8;
#define nil NULL
'''+square+'''
// The actual common.h probe resolves sqrt(float) to the double C function
static double LinesOriginalSqrt(double x) { return ::sqrt(x); }
#define sqrt LinesOriginalSqrt
void RwIm3DVertexSetRGBA(RwIm3DVertex *,RwUInt8,RwUInt8,RwUInt8,RwUInt8);
void RwIm3DVertexSetPos(RwIm3DVertex *,RwReal,RwReal,RwReal);
void *RwIm3DTransform(RwIm3DVertex *,uint32,RwMatrix *,uint32);
int RwIm3DRenderLine(int,int);
int RwIm3DEnd(void);
void LittleTest(void);
'''
(out/'common.h').write_text(common,newline='\n');(out/'main.h').write_text('#pragma once\nvoid LittleTest(void);\n',newline='\n')
(out/'original-Lines.h').write_text((out/'before/src/renderer/Lines.h').read_text(),newline='\n')
(out/'original.cpp').write_text(old.replace('#include "Lines.h"','#include "original-Lines.h"'),newline='\n')
(out/'caller.h').write_text('''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
void TestLine(float,float,float,float,float,float,uint32_t,uint32_t);
#ifdef __cplusplus
}
#endif
''',newline='\n')
(out/'caller.c').write_text('''#include "Lines.h"
#include "caller.h"
void TestLine(float x1,float y1,float z1,float x2,float y2,float z2,uint32_t c1,uint32_t c2)
{
    // Exercise the actual line function through a separately compiled C11 caller
    Lines_RenderLineWithClipping(x1,y1,z1,x2,y2,z2,c1,c2);
}
''',newline='\n')
(out/'original-caller.cpp').write_text('''#include "common.h"
#include "original-Lines.h"
#include "caller.h"
void TestLine(float x1,float y1,float z1,float x2,float y2,float z2,uint32_t c1,uint32_t c2)
{
    // Exercise the unmodified original member through the same fixture entry point
    CLines::RenderLineWithClipping(x1,y1,z1,x2,y2,z2,c1,c2);
}
''',newline='\n')
print('Audited full C line algorithm, typed backend, 48 calls in four files and both build manifests')
