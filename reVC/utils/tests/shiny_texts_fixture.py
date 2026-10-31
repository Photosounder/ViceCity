"""Audit complete shiny-text migration and generate whole-original/C11 comparison units."""
from pathlib import Path
import re,json
import shiny_texts_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/shiny-texts-c-tests'

def normalize(source):
    # Compare complete statements independently of formatting and edit comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text()
for name,value in [('ShinyTexts.c',m.kernel(old)),('ShinyTexts.cpp',m.HOST),('ShinyTexts.h',m.HEADER),('ShinyTexts_backend.h',m.BACKEND_HEADER)]:assert (root/'src/renderer'/name).read_text()==value
assert len(re.findall(r'\nCShinyTexts::\w+\(',m.section(old)))==5
assert 'enum\n{\n\tSHINYTEXT_WALK = 1,\n\tSHINYTEXT_FLAT\n};' in header
for path in ['src/core/common.h','src/core/config.h','src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','src/renderer/RenderBuffer.h','src/renderer/RenderBuffer.cpp','src/renderer/RenderBuffer_backend.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
    if path.startswith('src/math/'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
portable=(root/'build/render-buffer-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(portable,newline='\n')
assert saved.read_text()==portable
(out/'common.h').write_text(portable+'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef NATIVE_PROJECT_HEADERS
typedef uint8_t uint8;
#include "Vector2D.h"
namespace rw { struct Matrix { V3d right,up,at,pos; }; }
struct RwFrame { RwMatrix *getLTM(void); };
struct RwCamera { RwFrame *frame;RwFrame *getFrame(void)const{return frame;} };
struct RwTexture;
struct RwRaster;
RwRaster *RwTextureGetRaster(const RwTexture *);
void RwIm3DVertexSetU(RwIm3DVertex *,RwReal);
void RwIm3DVertexSetV(RwIm3DVertex *,RwReal);
#endif
''',newline='\n')
owners='''#pragma once
#include "ShinyTexts.h"
#include "RenderBuffer.h"
class CCamera { public:CVector position;CVector &GetPosition(void); };
extern CCamera TheCamera;
namespace CustomPipes { extern bool bRenderingEnvMap;extern RwCamera *EnvMapCam; }
extern RwTexture *gpWalkDontTex;
void LittleTest(void);
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Camera.h','custompipes.h','Shadows.h','main.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
oldheader=m.record_header(header);oldheader=oldheader[oldheader.index('class CShinyText\n'):]
(out/'original-ShinyTexts.h').write_text('#pragma once\n#include "ShinyTexts.h"\n'+oldheader,newline='\n')
prototypes='''ShinyText TestST_Record(bool env,int index);
void TestST_SetRecord(bool env,int index,const ShinyText *value);
void TestST_Counts(int *mainCount,int *envCount);
void TestST_SetCounts(int mainCount,int envCount);
void TestST_Init(void);
void TestST_Render(void);
void TestST_Env(void);
void TestST_Flush(void);
void TestST_Register(const ShinyText *value,float maxDist);
void TestST_Alias(int index,float maxDist);
'''
(out/'caller.h').write_text('#pragma once\n#include "ShinyTexts.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
oldobs='''#include "caller.h"
extern "C" ShinyText TestST_Record(bool env,int index)
{
    // Observe original declared fields without reading padding or assuming vector layout
    const CShinyText &v=env?EnvMapShinyTexts[index]:CShinyTexts::aShinyTexts[index];ShinyText result;
    for(int i=0;i<4;i++){
        // Copy the actual original coordinate and UV components
        result.m_verts[i]={v.m_verts[i].x,v.m_verts[i].y,v.m_verts[i].z};result.m_texCoords[i]={v.m_texCoords[i].x,v.m_texCoords[i].y};
    }
    result.m_camDist=v.m_camDist;result.m_type=v.m_type;result.m_red=v.m_red;result.m_green=v.m_green;result.m_blue=v.m_blue;return result;
}
extern "C" void TestST_SetRecord(bool env,int index,const ShinyText *value)
{
    // Seed every original field through its actual typed record
    CShinyText &v=env?EnvMapShinyTexts[index]:CShinyTexts::aShinyTexts[index];
    for(int i=0;i<4;i++){
        // Retain legal typed vector and UV assignments
        v.m_verts[i]=CVector(value->m_verts[i].x,value->m_verts[i].y,value->m_verts[i].z);v.m_texCoords[i].x=value->m_texCoords[i].x;v.m_texCoords[i].y=value->m_texCoords[i].y;
    }
    v.m_camDist=value->m_camDist;v.m_type=value->m_type;v.m_red=value->m_red;v.m_green=value->m_green;v.m_blue=value->m_blue;
}
extern "C" void TestST_Counts(int *mainCount,int *envCount)
{
    // Observe both independent original list lifetimes
    *mainCount=CShinyTexts::NumShinyTexts;*envCount=EnvMapShinyTextCount;
}
extern "C" void TestST_SetCounts(int mainCount,int envCount)
{
    // Seed original legal counts without changing an algorithm
    CShinyTexts::NumShinyTexts=mainCount;EnvMapShinyTextCount=envCount;
}
extern "C" void TestST_Register(const ShinyText *v,float maxDist)
{
    // Supply equivalent value inputs to the whole unchanged original registration
    CShinyTexts::RegisterOne(CVector(v->m_verts[0].x,v->m_verts[0].y,v->m_verts[0].z),CVector(v->m_verts[1].x,v->m_verts[1].y,v->m_verts[1].z),CVector(v->m_verts[2].x,v->m_verts[2].y,v->m_verts[2].z),CVector(v->m_verts[3].x,v->m_verts[3].y,v->m_verts[3].z),v->m_texCoords[0].x,v->m_texCoords[0].y,v->m_texCoords[1].x,v->m_texCoords[1].y,v->m_texCoords[2].x,v->m_texCoords[2].y,v->m_texCoords[3].x,v->m_texCoords[3].y,v->m_type,v->m_red,v->m_green,v->m_blue,maxDist);
}
extern "C" void TestST_Alias(int index,float maxDist)
{
    // Capture actual original pool subobjects before the camera query can mutate the pool
    CShinyText &v=CShinyTexts::aShinyTexts[index];CShinyTexts::RegisterOne(v.m_verts[0],v.m_verts[1],v.m_verts[2],v.m_verts[3],v.m_texCoords[0].x,v.m_texCoords[0].y,v.m_texCoords[1].x,v.m_texCoords[1].y,v.m_texCoords[2].x,v.m_texCoords[2].y,v.m_texCoords[3].x,v.m_texCoords[3].y,v.m_type,v.m_red,v.m_green,v.m_blue,maxDist);
}
'''
for name,original in [('Init','Init'),('Render','Render'),('Env','RenderForEnvMap'),('Flush','RenderOutGeometryBuffer')]:oldobs+='extern "C" void TestST_'+name+'(void)\n{\n    // Enter the complete original method\n    CShinyTexts::'+original+'();\n}\n'
(out/'original.cpp').write_text('#include "common.h"\n#include "owners.h"\n#include "original-ShinyTexts.h"\n'+m.section(old)+'\n'+oldobs,newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'buffer-host.cpp').write_text((out/'before/src/renderer/RenderBuffer.cpp').read_text(),newline='\n')
observer='''#include "caller.h"
_Static_assert(sizeof(ShinyText)==88 && _Alignof(ShinyText)==4,"Original record size and alignment");
_Static_assert(offsetof(ShinyText,m_verts)==0 && offsetof(ShinyText,m_texCoords)==48 && offsetof(ShinyText,m_camDist)==80 && offsetof(ShinyText,m_type)==84 && offsetof(ShinyText,m_red)==85 && offsetof(ShinyText,m_green)==86 && offsetof(ShinyText,m_blue)==87,"Original field offsets");
ShinyText TestST_Record(bool env,int index)
{
    // Observe all fields through legal same-unit C storage access
    return env?EnvMapShinyTexts[index]:aShinyTexts[index];
}
void TestST_SetRecord(bool env,int index,const ShinyText *value)
{
    // Seed identical main or retained reflection records
    if(env)EnvMapShinyTexts[index]=*value;else aShinyTexts[index]=*value;
}
void TestST_Counts(int *mainCount,int *envCount)
{
    // Observe both C-owned live counts
    *mainCount=NumShinyTexts;*envCount=EnvMapShinyTextCount;
}
void TestST_SetCounts(int mainCount,int envCount)
{
    // Seed identical legal list lengths
    NumShinyTexts=mainCount;EnvMapShinyTextCount=envCount;
}
void TestST_Alias(int index,float maxDist)
{
    // Capture real C pool fields through the same public by-value argument boundary
    ShinyText *v=&aShinyTexts[index];ShinyTexts_RegisterOne(v->m_verts[0],v->m_verts[1],v->m_verts[2],v->m_verts[3],v->m_texCoords[0].x,v->m_texCoords[0].y,v->m_texCoords[1].x,v->m_texCoords[1].y,v->m_texCoords[2].x,v->m_texCoords[2].y,v->m_texCoords[3].x,v->m_texCoords[3].y,v->m_type,v->m_red,v->m_green,v->m_blue,maxDist);
}
'''
(out/'observed-kernel.c').write_text('#include <stddef.h>\n#include "'+(root/'src/renderer/ShinyTexts.c').as_posix()+'"\n'+observer,newline='\n')
caller='#include "caller.h"\n'
for name,method in [('Init','Init'),('Render','Render'),('Env','RenderForEnvMap'),('Flush','RenderOutGeometryBuffer')]:caller+='void TestST_'+name+'(void)\n{\n    // Enter the actual C algorithm from a separate C11 caller\n    ShinyTexts_'+method+'();\n}\n'
caller+='''void TestST_Register(const ShinyText *v,float maxDist)
{
    // Capture the exact same complete public C argument list
    ShinyTexts_RegisterOne(v->m_verts[0],v->m_verts[1],v->m_verts[2],v->m_verts[3],v->m_texCoords[0].x,v->m_texCoords[0].y,v->m_texCoords[1].x,v->m_texCoords[1].y,v->m_texCoords[2].x,v->m_texCoords[2].y,v->m_texCoords[3].x,v->m_texCoords[3].y,v->m_type,v->m_red,v->m_green,v->m_blue,maxDist);
}
'''
(out/'caller.c').write_text(caller,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/shiny_texts.cpp.in').read_text(),newline='\n')
(out/'no-pipeline.h').write_text('#include "config.h"\n#undef EXTENDED_PIPELINES\n',newline='\n')
print('Audited five complete algorithms, exact 88-byte record/backend, shared-buffer reuse and seven complete source/project files')
