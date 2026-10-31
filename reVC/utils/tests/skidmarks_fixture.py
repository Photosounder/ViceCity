"""Audit complete skidmark relocation and generate original/C observation drivers."""
from pathlib import Path
import re,json
import skidmarks_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/skidmarks-c-tests'
def normalize(source):
    # Compare every original statement while allowing comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text();expected=m.project(original) if path.startswith('build/') else m.calls(original)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Skidmarks.cpp').read_text();header=(out/'before/src/renderer/Skidmarks.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/Skidmarks.c').read_text())
assert (root/'src/renderer/Skidmarks_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^CSkidmarks::\w+\(',old,re.M))==7
deps=['src/core/common.h','src/core/config.h','src/core/Timer.h','src/core/main.h','src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
common=(root/'build/ropes-c-tests/common.h').read_text();shared=out/'before/rope-common.h'
if not shared.exists():shared.write_text(common,newline='\n')
assert shared.read_text()==common
rgba=(out/'before/src/core/common.h').read_text();rgba=rgba[rgba.index('class CRGBA\n'):rgba.index('\n#if (defined(_MSC_VER))',rgba.index('class CRGBA\n'))]
extra='''
#ifndef NATIVE_PROJECT_HEADERS
typedef uint8_t uint8;
typedef int16_t int16;
#include "Vector2D.h"
'''+rgba+'''
struct RwTexture;
struct RwRaster;
RwTexture *RwTextureRead(const char *,const char *);
RwBool RwTextureDestroy(RwTexture *);
RwRaster *RwTextureGetRaster(const RwTexture *);
void RwIm3DVertexSetU(RwIm3DVertex *,RwReal);
void RwIm3DVertexSetV(RwIm3DVertex *,RwReal);
#endif
'''
(out/'common.h').write_text(common+extra,newline='\n')
for path in ['src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h']:(out/Path(path).name).write_text((out/'before'/path).read_text(),newline='\n')
(out/'main.h').write_text((out/'before/src/core/main.h').read_text().split('struct GlobalScene')[0]+'\nvoid LittleTest(void);\n',newline='\n')
(out/'TxdStore.h').write_text('''#pragma once
class CTxdStore { public:
    static void PushCurrentTxd(void);
    static int FindTxdSlot(const char *);
    static void SetCurrentTxd(int);
    static void PopCurrentTxd(void);
};
''',newline='\n')
(out/'Replay.h').write_text('#pragma once\nclass CReplay { public: static bool IsPlayingBack(void); };\n',newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
def rename(source):
    # Keep the original complete enum definitions independent of the new C enum
    return re.sub(r'\b(SKIDMARK_LENGTH|SKIDMARK_NORMAL|SKIDMARK_MUDDY|SKIDMARK_SANDY|SKIDMARK_BLOODY|eSkidmarkType)\b',r'ORIGINAL_\1',source)
(out/'original-Skidmarks.h').write_text(rename(header).replace('class CSkidmarks\n{','class CSkidmarks\n{\npublic:'),newline='\n')
original=rename(old).replace('#include "common.h"',commonInclude).replace('#include "Skidmarks.h"','#include "original-Skidmarks.h"')
original=re.sub(r'^RwImVertexIndex SkidmarkIndexList\[[^\n]+;',lambda match:'extern "C" { '+match[0]+' }',original,flags=re.M)
observer='''static Skidmark TestS_Copy(const CSkidmark &value)
{
    // Observe every original declared field without copying C++ object storage
    Skidmark result={};result.m_id=value.m_id;result.m_lastUpdate=value.m_lastUpdate;result.m_fadeStart=value.m_fadeStart;result.m_fadeEnd=value.m_fadeEnd;result.m_type=value.m_type;result.m_last=value.m_last;result.m_state=value.m_state;result.m_wasUpdated=value.m_wasUpdated;
    for(int i=0;i<SKIDMARK_LENGTH;i++){
        // Preserve all position and side-array values including inactive data
        result.m_pos[i]={value.m_pos[i].x,value.m_pos[i].y,value.m_pos[i].z};result.m_sideX[i]=value.m_sideX[i];result.m_sideY[i]=value.m_sideY[i];
    }
    return result;
}
extern "C" Skidmark TestS_Record(int index)
{
    // Observe actual original private storage in its implementation translation unit
    return TestS_Copy(CSkidmarks::aSkidmarks[index]);
}
extern "C" void TestS_SetRecord(int index,const Skidmark *value)
{
    // Supply complete original record states without changing the algorithms
    CSkidmark &result=CSkidmarks::aSkidmarks[index];result.m_id=value->m_id;result.m_lastUpdate=value->m_lastUpdate;result.m_fadeStart=value->m_fadeStart;result.m_fadeEnd=value->m_fadeEnd;result.m_type=value->m_type;result.m_last=value->m_last;result.m_state=value->m_state;result.m_wasUpdated=value->m_wasUpdated;
    for(int i=0;i<SKIDMARK_LENGTH;i++){
        // Reconstruct actual original CVector values and scalar side arrays
        result.m_pos[i]=CVector(value->m_pos[i].x,value->m_pos[i].y,value->m_pos[i].z);result.m_sideX[i]=value->m_sideX[i];result.m_sideY[i]=value->m_sideY[i];
    }
}
extern "C" bool *TestS_Updated(int index)
{
    // Expose the actual original bool field for legal output-pointer alias tests
    return &CSkidmarks::aSkidmarks[index].m_wasUpdated;
}
extern "C" void *TestS_Texture(void)
{
    // Observe publication and clearing of the actual original resource handle
    return gpSkidTex;
}
'''
(out/'original.cpp').write_text(original+'\n#include "caller.h"\n'+observer,newline='\n')
observeC='''Skidmark TestS_Record(int index)
{
    // Observe all actual C private record fields
    return aSkidmarks[index];
}
void TestS_SetRecord(int index,const Skidmark *value)
{
    // Supply the same complete states to the C implementation
    aSkidmarks[index]=*value;
}
bool *TestS_Updated(int index)
{
    // Expose the same C bool field for output alias tests
    return &aSkidmarks[index].m_wasUpdated;
}
void *TestS_Texture(void)
{
    // Observe the actual C resource handle at the original publication points
    return gpSkidTex;
}
'''
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/Skidmarks.c').as_posix()+'"\n#include "caller.h"\n'+observeC,newline='\n')
protos='''void TestS_Init(void);
void TestS_Shutdown(void);
void TestS_Clear(void);
void TestS_Update(void);
void TestS_Render(void);
void TestS_Type(uintptr_t id,const void *pos,float x,float y,int type,bool *blood);
void TestS_Flags(uintptr_t id,const void *pos,float x,float y,bool *mud,bool *blood);
void TestS_Temporary(uintptr_t id,float x,float y,float z,int type,bool *blood);
Skidmark TestS_Record(int index);
void TestS_SetRecord(int index,const Skidmark *record);
bool *TestS_Updated(int index);
void *TestS_Texture(void);
'''
(out/'caller.h').write_text('#pragma once\n#include "Skidmarks.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+protos+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
oldcaller=commonInclude+'\n#include "original-Skidmarks.h"\n#include "caller.h"\n';newcaller='#include "caller.h"\n'
for name in ['Init','Shutdown','Clear','Update','Render']:
    oldcaller+='extern "C" void TestS_'+name+'(void)\n{\n    // Enter the complete original skidmark algorithm\n    CSkidmarks::'+name+'();\n}\n'
    newcaller+='void TestS_'+name+'(void)\n{\n    // Enter the complete C algorithm from a separate C11 caller\n    Skidmarks_'+name+'();\n}\n'
oldcaller+='''extern "C" void TestS_Type(uintptr_t id,const void *pos,float x,float y,int type,bool *blood)
{
    // Select the original explicit-type overload with its original borrowed object
    CSkidmarks::RegisterOne(id,*(const CVector *)pos,x,y,(ORIGINAL_eSkidmarkType)type,blood);
}
extern "C" void TestS_Flags(uintptr_t id,const void *pos,float x,float y,bool *mud,bool *blood)
{
    // Select the original bool-pointer dispatcher without copying its position
    CSkidmarks::RegisterOne(id,*(const CVector *)pos,x,y,mud,blood);
}
extern "C" void TestS_Temporary(uintptr_t id,float x,float y,float z,int type,bool *blood)
{
    // Keep the original temporary value alive through the entire registration call
    CSkidmarks::RegisterOne(id,CVector(x,y,z)+CVector(0.125f,-0.25f,0.5f),1.0f,0.0f,(ORIGINAL_eSkidmarkType)type,blood);
}
'''
newcaller+='''void TestS_Type(uintptr_t id,const void *pos,float x,float y,int type,bool *blood)
{
    // Pass the borrowed legacy object through C without eagerly inspecting its coordinates
    Skidmarks_RegisterOneType(id,pos,x,y,(eSkidmarkType)type,blood);
}
void TestS_Flags(uintptr_t id,const void *pos,float x,float y,bool *mud,bool *blood)
{
    // Exercise the independent C dispatcher and pointer aliases
    Skidmarks_RegisterOneFlags(id,pos,x,y,mud,blood);
}
'''
(out/'original-caller.cpp').write_text(oldcaller,newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
legacy=commonInclude+'''
#include "caller.h"
#include "Skidmarks_backend.h"
extern "C" void TestS_Temporary(uintptr_t id,float x,float y,float z,int type,bool *blood)
{
    // Borrow the original temporary until the outer C registration call completes
    Skidmarks_RegisterOneType(id,Skidmarks_BackendPosition(CVector(x,y,z)+CVector(0.125f,-0.25f,0.5f)),1.0f,0.0f,(eSkidmarkType)type,blood);
}
'''
(out/'legacy.cpp').write_text(legacy,newline='\n');(out/'host.cpp').write_text(m.HOST.replace('#include "common.h"',commonInclude),newline='\n')
absolute=Path('C:/msys/home/rouziclib/math/ieee754.c');saved=out/'before/external/rouziclib/math/ieee754.c'
if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(absolute.read_bytes())
assert saved.read_bytes()==absolute.read_bytes()
source=saved.read_text();a=source.index('float fast_absf(float x)');b=source.index('\n}',a)+2
(out/'fast-absf.c').write_text('#include <stdint.h>\n'+source[a:b].replace('{','{\n    // Execute the complete original sign-bit operation',1)+'\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/skidmarks.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
print('Audited seven complete algorithms, independent enums, eight typed registration calls, lazy borrowed positions and ten full source/project files')
