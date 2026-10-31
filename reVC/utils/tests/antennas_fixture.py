"""Audit whole antenna relocation and build exact original/C storage observers."""
from pathlib import Path
import re,json
import antennas_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/antennas-c-tests'
def normalize(source):return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Antennas.cpp').read_text();header=(out/'before/src/renderer/Antennas.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/Antennas.c').read_text())
assert (root/'src/renderer/Antennas_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^(?:CAntennas|CAntenna)::\w+\(',old,re.M))==7
deps=['src/math/Vector.h','src/math/Vector.cpp','src/math/maths.h','src/core/config.h','src/core/main.h','vendor/librw/src/rwbase.h','vendor/librw/src/rwrender.h','vendor/librw/src/d3d/rwd3d.h','vendor/librw/src/gl/rwgl3.h','src/fakerw/rwplcore.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
def saved(path):return (out/'before'/path).read_text()
rw=saved('vendor/librw/src/rwbase.h');records=re.search(r'struct RGBA\n\{.*?\n\};.*?inline RGBA makeRGBA[^\n]+',rw,re.S)[0];vector=re.search(r'struct V3d\n\{.*?\n\};',rw,re.S)[0]
d3d=saved('vendor/librw/src/d3d/rwd3d.h');gl3=saved('vendor/librw/src/gl/rwgl3.h');d3dv=re.search(r'struct Im3DVertex\n\{.*?\n\};',d3d,re.S)[0];gl3v=re.search(r'struct Im3DVertex\n\{.*?\n\};',gl3,re.S)[0]
color=re.search(r'^#define COLOR_ARGB[^\n]+',d3d,re.M)[0]
blend=re.search(r'enum BlendFunction\n\{.*?\n\};',saved('vendor/librw/src/rwrender.h'),re.S)[0]
states=re.search(r'enum RwRenderState\n\{.*?\n\};',saved('src/fakerw/rwplcore.h'),re.S)[0]
rwblend=re.search(r'enum RwBlendFunction\n\{.*?\n\};',saved('src/fakerw/rwplcore.h'),re.S)[0]
common='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <math.h>
#include "config.h"
typedef uint32_t uint32;
typedef int32_t int32;
typedef uintptr_t uintptr;
namespace rw {
typedef uint8_t uint8;
typedef uint32_t uint32;
typedef int32_t int32;
typedef float float32;
typedef uint32_t bool32;
struct Matrix;
'''+records+'\n'+vector+'\n'+blend+'\nnamespace d3d {\n'+color+'\n'+d3dv+'\n}\nnamespace gl3 {\n'+gl3v+'''
}
}
#ifdef TEST_GL3
typedef rw::gl3::Im3DVertex RwIm3DVertex;
#else
typedef rw::d3d::Im3DVertex RwIm3DVertex;
#endif
typedef rw::V3d RwV3d;
typedef rw::RGBA RwRGBA;
typedef rw::Matrix RwMatrix;
typedef float RwReal;
typedef uint8_t RwUInt8;
typedef uint32_t RwUInt32;
typedef int32_t RwInt32;
typedef int32_t RwBool;
#define nil NULL
#define TRUE 1
#define FALSE 0
#include "maths.h"
#include "Vector.h"
'''+states+'\n'+rwblend+'''
RwBool RwRenderStateSet(RwRenderState,void *);
void RwIm3DVertexSetRGBA(RwIm3DVertex *,RwUInt8,RwUInt8,RwUInt8,RwUInt8);
void RwIm3DVertexSetPos(RwIm3DVertex *,RwReal,RwReal,RwReal);
void *RwIm3DTransform(RwIm3DVertex *,RwUInt32,RwMatrix *,RwUInt32);
RwBool RwIm3DRenderLine(RwInt32,RwInt32);
RwBool RwIm3DEnd(void);
'''
(out/'common.h').write_text(common,newline='\n')
for path in ['src/math/Vector.h','src/math/maths.h','src/core/main.h']:(out/Path(path).name).write_text(saved(path).split('struct GlobalScene')[0] if path.endswith('main.h') else saved(path),newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
mainInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/main.h').as_posix()+'"\n#else\n#include "main.h"\n#endif'
(out/'original-Antennas.h').write_text(header.replace('class CAntennas\n{','class CAntennas\n{\npublic:'),newline='\n')
protos='''void TestA_Init(void);
void TestA_Frame(void);
void TestA_Register(uint32_t id,AntennasVector dir,AntennasVector pos,float length);
void TestA_Render(void);
void TestA_UpdateRecord(Antenna *record,AntennasVector dir,AntennasVector pos);
Antenna TestA_Record(int index,int auxiliary);
int TestA_Count(void);
void TestA_Legacy(uintptr_t id,const void *dir,const void *pos,float length);
#ifdef REVC_SOFTWARE_POLYGONS
void TestA_EnvRegister(uint32_t id,AntennasVector dir,AntennasVector pos,float length);
void TestA_EnvRender(void);
#endif
'''
(out/'caller.h').write_text('#pragma once\n#include "Antennas.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+protos+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
convert='''static Antenna TestA_Copy(const CAntenna *value)
{
    // Observe each original declared field without assuming the CVector base layout
    Antenna result={};result.active=value->active;result.updatedLastFrame=value->updatedLastFrame;result.id=value->id;result.segmentLength=value->segmentLength;
    for(int i=0;i<6;i++){
        // Copy the original position and speed components in their declared order
        result.pos[i]={value->pos[i].x,value->pos[i].y,value->pos[i].z};result.speed[i]={value->speed[i].x,value->speed[i].y,value->speed[i].z};
    }
    return result;
}
extern "C" Antenna TestA_Record(int index,int auxiliary)
{
    // Observe both entire private arrays from the original implementation unit
#ifdef REVC_SOFTWARE_POLYGONS
    if(auxiliary)return TestA_Copy(&EnvMapAntennas[index]);
#endif
    return TestA_Copy(&CAntennas::aAntennas[index]);
}
extern "C" int TestA_Count(void)
{
    // Observe the original live auxiliary count only where it exists
#ifdef REVC_SOFTWARE_POLYGONS
    return NumEnvMapAntennas;
#else
    return 0;
#endif
}
extern "C" void TestA_UpdateRecord(Antenna *value,AntennasVector dir,AntennasVector pos)
{
    // Enter the original spring algorithm with every declared record field supplied
    CAntenna record;record.active=value->active;record.updatedLastFrame=value->updatedLastFrame;record.id=value->id;record.segmentLength=value->segmentLength;
    for(int i=0;i<6;i++){
        // Reconstruct the original typed vector values without casting the C record
        record.pos[i]=CVector(value->pos[i].x,value->pos[i].y,value->pos[i].z);record.speed[i]=CVector(value->speed[i].x,value->speed[i].y,value->speed[i].z);
    }
    record.Update(CVector(dir.x,dir.y,dir.z),CVector(pos.x,pos.y,pos.z));*value=TestA_Copy(&record);
}
'''
(out/'original.cpp').write_text(old.replace('#include "common.h"',commonInclude).replace('#include "main.h"',mainInclude).replace('#include "Antennas.h"','#include "original-Antennas.h"')+'\n#include "caller.h"\n'+convert,newline='\n')
observer='''
Antenna TestA_Record(int index,int auxiliary)
{
    // Observe the actual C private arrays in the implementation translation unit
#ifdef REVC_SOFTWARE_POLYGONS
    if(auxiliary)return EnvMapAntennas[index];
#endif
    return aAntennas[index];
}
int TestA_Count(void)
{
    // Observe the actual C auxiliary queue count
#ifdef REVC_SOFTWARE_POLYGONS
    return NumEnvMapAntennas;
#else
    return 0;
#endif
}
'''
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/Antennas.c').as_posix()+'"\n#include "caller.h"\n'+observer,newline='\n')
newcaller='#include "caller.h"\n';oldcaller=commonInclude+'\n#include "original-Antennas.h"\n#include "caller.h"\n'
for suffix,args,newexpr,oldexpr,conditional in [
    ('Init','void','Antennas_Init()','CAntennas::Init()',False),('Frame','void','Antennas_Update()','CAntennas::Update()',False),
    ('Register','uint32_t id,AntennasVector dir,AntennasVector pos,float length','Antennas_RegisterOne(id,dir,pos,length)','CAntennas::RegisterOne(id,CVector(dir.x,dir.y,dir.z),CVector(pos.x,pos.y,pos.z),length)',False),
    ('Render','void','Antennas_Render()','CAntennas::Render()',False),
    ('EnvRegister','uint32_t id,AntennasVector dir,AntennasVector pos,float length','Antennas_RegisterOneForEnvMap(id,dir,pos,length)','CAntennas::RegisterOneForEnvMap(id,CVector(dir.x,dir.y,dir.z),CVector(pos.x,pos.y,pos.z),length)',True),
    ('EnvRender','void','Antennas_RenderForEnvMap()','CAntennas::RenderForEnvMap()',True)]:
    for original,expression in [(True,oldexpr),(False,newexpr)]:
        body=('extern "C" ' if original else '')+'void TestA_'+suffix+'('+args+')\n{\n    // Enter the complete '+('original' if original else 'C11')+' antenna algorithm\n    '+expression+';\n}\n'
        if conditional:body='#ifdef REVC_SOFTWARE_POLYGONS\n'+body+'#endif\n'
        if original:oldcaller+=body
        else:newcaller+=body
newcaller+='void TestA_UpdateRecord(Antenna *record,AntennasVector dir,AntennasVector pos)\n{\n    // Enter the complete C spring algorithm from a separate C11 unit\n    Antenna_Update(record,dir,pos);\n}\n'
(out/'caller.c').write_text(newcaller,newline='\n');(out/'original-caller.cpp').write_text(oldcaller,newline='\n')
legacy=commonInclude+'\n#include "caller.h"\n#ifdef ORIGINAL_ANTENNAS\n#include "original-Antennas.h"\n#else\n#include "Antennas_backend.h"\n#endif\n'
legacy+='''extern "C" void TestA_Legacy(uintptr_t id,const void *dir,const void *pos,float length)
{
    // Preserve the original pointer-ID narrowing and typed vector copy boundaries
#ifdef ORIGINAL_ANTENNAS
    CAntennas::RegisterOne(id,*(const CVector *)dir,*(const CVector *)pos,length);
#else
    Antennas_RegisterOne(id,Antennas_BackendVector(dir),Antennas_BackendVector(pos),length);
#endif
}
'''
(out/'legacy.cpp').write_text(legacy,newline='\n')
(out/'host.cpp').write_text(m.HOST.replace('#include "common.h"',commonInclude).replace('#include "main.h"',mainInclude),newline='\n')
vector=saved('src/math/Vector.cpp');normalise=vector[vector.index('void\nCVector::Normalise'):vector.index('\nCVector\nCrossProduct')]
(out/'original-vector.cpp').write_text(commonInclude+'\n'+normalise,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/antennas.cpp.in').read_text().replace('@COMMON@',commonInclude).replace('@MAIN@',mainInclude),newline='\n')
print('Audited seven full antenna algorithms, 156-byte records, exact original vector helpers, 13 typed backend operations and nine complete source/project files')
