"""Audit whole rope algorithms and generate exact original/C differential inputs."""
from pathlib import Path
import re,json
import ropes_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/ropes-c-tests'
def normalize(source):
    # Compare complete statements while ignoring comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/core/Ropes.cpp').read_text();header=(out/'before/src/core/Ropes.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/core/Ropes.c').read_text())
assert (root/'src/core/Ropes_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^(?:CRopes|CRope)::\w+\(',old,re.M))==9
deps=['src/math/Vector.h','src/math/maths.h','src/core/config.h','src/core/common.h','src/core/Timer.h','src/core/main.h','src/renderer/RenderBuffer.h','src/fakerw/rwcore.h','src/fakerw/rwplcore.h','vendor/librw/src/rwbase.h','vendor/librw/src/rwrender.h','vendor/librw/src/d3d/rwd3d.h','vendor/librw/src/gl/rwgl3.h','src/modelinfo/ModelIds.h','src/peds/PedType.h','src/peds/CopPed.h','src/animation/AnimManager.h','src/animation/AnimationId.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    # Follow the separately audited shared-buffer header relocation
    if path=='src/renderer/RenderBuffer.h':
        import render_buffer_migration
        assert render_buffer_migration.header(saved.read_text())==(root/path).read_text(),path
    elif path=='src/peds/PedType.h':
        import ped_data_migration
        ped_data_migration.verify_dependency(saved,root/path)
    elif path=='src/animation/AnimManager.h':
        import weapon_info_migration
        weapon_info_migration.verify_dependency(saved,root/path)
    else:assert saved.read_bytes()==(root/path).read_bytes(),path
def saved(path):
    # Read the immutable dependency snapshot for the original implementation
    return (out/'before'/path).read_text()
def enum(source,name):
    # Preserve the complete original enum declaration and its exact values
    return re.search(r'enum '+name+r'\n\{.*?\n\};',source,re.S)[0]
common=(root/'build/antennas-c-tests/common.h').read_text()
# Preserve the exact previously audited vector/vertex declarations as a complete dependency snapshot
shared=out/'before/antenna-common.h'
if not shared.exists():shared.write_text(common,newline='\n')
assert shared.read_text()==common
extra='\n#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))\n'+re.search(r'^#define Clamp[^\n]+',saved('src/core/common.h'),re.M)[0]+'\n'
for backend,path in [('d3d','vendor/librw/src/d3d/rwd3d.h'),('gl3','vendor/librw/src/gl/rwgl3.h')]:
    extra+='namespace rw { namespace '+backend+' {\n'+re.search(r'struct Im2DVertex\n\{.*?\n\};',saved(path),re.S)[0]+'\n} }\n'
extra+='#ifdef TEST_GL3\ntypedef rw::gl3::Im2DVertex RwIm2DVertex;\n#else\ntypedef rw::d3d::Im2DVertex RwIm2DVertex;\n#endif\n'
extra+='namespace rw {\n'+enum(saved('vendor/librw/src/rwbase.h'),'PrimitiveType')+'\nnamespace im3d {\n'+enum(saved('vendor/librw/src/rwrender.h'),'TransformFlags')+'\n}\n}\n'
extra+=enum(saved('src/fakerw/rwplcore.h'),'RwPrimitiveType')+'\n'+enum(saved('src/fakerw/rwcore.h'),'RwIm3DTransformFlags')+'\ntypedef uint16_t RwImVertexIndex;\nstruct RpClump;\n'
extra+='RwBool RwIm3DRenderIndexedPrimitive(RwPrimitiveType,RwImVertexIndex *,RwInt32);\n'
(out/'common.h').write_text('#pragma once\n#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n'+common+extra+'#endif\n',newline='\n')
for path in ['src/math/Vector.h','src/math/maths.h']:(out/Path(path).name).write_text(saved(path),newline='\n')
(out/'main.h').write_text(saved('src/core/main.h').split('struct GlobalScene')[0],newline='\n')
(out/'ModelIndices.h').write_text(saved('src/modelinfo/ModelIds.h'),newline='\n')
owners='''#pragma once
#include "common.h"
'''+enum(saved('src/peds/PedType.h'),'ePedType')+'\n'+enum(saved('src/peds/CopPed.h'),'eCopType')+'\n'+enum(saved('src/animation/AnimManager.h'),'AssocGroupId')+'\n'+enum(saved('src/animation/AnimationId.h'),'AnimationId')+'''
class CEntity { public: bool bUsesCollision=true;virtual ~CEntity(){}; };
class RopeAnchor { public: uintptr guard=0x12345678;virtual ~RopeAnchor(){}; };
class CPed : public RopeAnchor,public CEntity {};
class CCopPed : public CPed { public:
    CEntity *m_pRopeEntity=nullptr;
    uintptr m_nRopeID=0;
    RpClump *GetClump(void);
};
class CAnimManager { public: static void *BlendAnimation(RpClump *,AssocGroupId,AnimationId,float); };
'''
(out/'CopPed.h').write_text(owners,newline='\n')
(out/'Population.h').write_text('#pragma once\n#include "CopPed.h"\nclass CPopulation { public: static CPed *AddPed(ePedType,uint32,const CVector &,int32 modifier=0); };\n',newline='\n')
(out/'Streaming.h').write_text('#pragma once\nclass CStreaming { public: static bool HasModelLoaded(int32 id); };\n',newline='\n')
(out/'Camera.h').write_text('#pragma once\nclass CCamera { public: bool IsSphereVisible(const CVector &,float); };\nextern CCamera TheCamera;\n',newline='\n')
(out/'custompipes.h').write_text('#pragma once\nnamespace CustomPipes { extern bool bRenderingEnvMap; }\n',newline='\n')
render=saved('src/renderer/RenderBuffer.h');render=render[render.index('#define TEMPBUFFERVERTSIZE'):]
(out/'RenderBuffer.h').write_text('#pragma once\n'+render,newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
seed='''#ifdef TEST_ROPE_WRAP
#define TEST_ROPE_SEED (UINT32_MAX-2u)
#else
#define TEST_ROPE_SEED 0
#endif
'''
assert old.count('static uint32 ropeId = 0;')==1
original=old.replace('#include "common.h"',commonInclude).replace('#include "Ropes.h"','#include "original-Ropes.h"').replace('static uint32 ropeId = 0;','static uint32 ropeId = TEST_ROPE_SEED;')
original=original.replace('RwImVertexIndex RopeIndices[64]', 'extern "C" RwImVertexIndex RopeIndices[64]')
(out/'original-Ropes.h').write_text(header.replace('class CRopes\n{','class CRopes\n{\npublic:'),newline='\n')
copy='''static Rope TestR_Copy(const CRope &value)
{
    // Observe every original declared field without copying C++ object storage
    Rope result={};result.m_bActive=value.m_bActive;result.m_bWasRegistered=value.m_bWasRegistered;result.m_unk=value.m_unk;result.m_id=value.m_id;result.m_updateTimer=value.m_updateTimer;
    for(int i=0;i<32;i++){
        // Copy the original position and speed values through their typed members
        result.m_pos[i]={value.m_pos[i].x,value.m_pos[i].y,value.m_pos[i].z};result.m_speed[i]={value.m_speed[i].x,value.m_speed[i].y,value.m_speed[i].z};
    }
    return result;
}
static CRope TestR_Original(const Rope &value)
{
    // Construct a complete original record for direct member comparisons
    CRope result;result.m_bActive=value.m_bActive;result.m_bWasRegistered=value.m_bWasRegistered;result.m_unk=value.m_unk;result.m_id=value.m_id;result.m_updateTimer=value.m_updateTimer;
    for(int i=0;i<32;i++){
        // Reconstruct each original CVector through its actual value constructor
        result.m_pos[i]=CVector(value.m_pos[i].x,value.m_pos[i].y,value.m_pos[i].z);result.m_speed[i]=CVector(value.m_speed[i].x,value.m_speed[i].y,value.m_speed[i].z);
    }
    return result;
}
extern "C" Rope TestR_Record(int index)
{
    // Observe every original private record including inactive values
    return TestR_Copy(CRopes::aRopes[index]);
}
extern "C" void TestR_DirectUpdate(Rope *value)
{
    // Execute the complete original member physics on supplied record values
    CRope record=TestR_Original(*value);record.Update();*value=TestR_Copy(record);
}
extern "C" void TestR_DirectRender(Rope *value)
{
    // Execute the complete original member render path with supplied values
    CRope record=TestR_Original(*value);record.Render();*value=TestR_Copy(record);
}
'''
(out/'original.cpp').write_text(seed+original+'\n#include "caller.h"\n'+copy,newline='\n')
kernel=(root/'src/core/Ropes.c').read_text();assert kernel.count('static uint32_t ropeId = 0;')==1
kernel=kernel.replace('static uint32_t ropeId = 0;','static uint32_t ropeId = TEST_ROPE_SEED;')
(out/'observed-kernel.c').write_text(seed+kernel+'\n#include "caller.h"\nRope TestR_Record(int index)\n{\n    // Observe the actual C private array in its implementation translation unit\n    return aRopes[index];\n}\n',newline='\n')
protos='''void TestR_Init(void);
void TestR_Update(void);
void TestR_Render(void);
bool TestR_Register(uintptr_t id,RopesVector pos,bool timer);
void TestR_Speed(uintptr_t id,RopesVector speed);
bool TestR_Find(uintptr_t id,float t,RopesVector *out);
bool TestR_LegacyFind(uintptr_t id,float t,void *out);
bool TestR_Swat(RopesVector pos);
Rope TestR_Record(int index);
void TestR_DirectUpdate(Rope *record);
void TestR_DirectRender(Rope *record);
bool TestR_LegacyRegister(uintptr_t id,const void *pos,bool timer);
'''
(out/'caller.h').write_text('#pragma once\n#include "Ropes.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+protos+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
oldcaller=commonInclude+'\n#include "original-Ropes.h"\n#include "caller.h"\n';newcaller='#include "caller.h"\n'
for name,result,args,oldexpr,newexpr in [
    ('Init','void','void','CRopes::Init()','Ropes_Init()'),('Update','void','void','CRopes::Update()','Ropes_Update()'),('Render','void','void','CRopes::Render()','Ropes_Render()'),
    ('Register','bool','uintptr_t id,RopesVector pos,bool timer','CRopes::RegisterRope(id,CVector(pos.x,pos.y,pos.z),timer)','Ropes_RegisterRope(id,pos,timer)'),
    ('Speed','void','uintptr_t id,RopesVector speed','CRopes::SetSpeedOfTopNode(id,CVector(speed.x,speed.y,speed.z))','Ropes_SetSpeedOfTopNode(id,speed)'),
    ('Swat','bool','RopesVector pos','CRopes::CreateRopeWithSwatComingDown(CVector(pos.x,pos.y,pos.z))','Ropes_CreateRopeWithSwatComingDown(pos)')]:
    for orig,expr in [(True,oldexpr),(False,newexpr)]:
        body=('extern "C" ' if orig else '')+result+' TestR_'+name+'('+args+')\n{\n    // Enter the complete '+('original' if orig else 'C11')+' rope algorithm\n    '+('return ' if result!='void' else '')+expr+';\n}\n'
        if orig:oldcaller+=body
        else:newcaller+=body
oldcaller+='''extern "C" bool TestR_Find(uintptr_t id,float t,RopesVector *out)
{
    // Retain original failure output contents while providing a typed C value comparison
    CVector value;
    if(!CRopes::FindCoorsAlongRope(id,t,&value))return false;
    *out={value.x,value.y,value.z};return true;
}
extern "C" bool TestR_LegacyFind(uintptr_t id,float t,void *out)
{
    // Exercise the original real CVector output object directly
    return CRopes::FindCoorsAlongRope(id,t,(CVector *)out);
}
'''
for name,args,expr in [('Find','uintptr_t id,float t,RopesVector *out','Ropes_FindCoorsAlongRope(id,t,out)'),('LegacyFind','uintptr_t id,float t,void *out','Ropes_FindCoorsAlongRopeToVector(id,t,out)')]:
    newcaller+='bool TestR_'+name+'('+args+')\n{\n    // Exercise the C lookup or typed output adapter from a separate C11 caller\n    return '+expr+';\n}\n'
newcaller+='void TestR_DirectUpdate(Rope *record)\n{\n    // Exercise the complete C member physics\n    Rope_Update(record);\n}\nvoid TestR_DirectRender(Rope *record)\n{\n    // Exercise the complete C member rendering\n    Rope_Render(record);\n}\n'
(out/'original-caller.cpp').write_text(oldcaller,newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
legacy=commonInclude+'\n#include "caller.h"\n#ifdef ORIGINAL_ROPES\n#include "original-Ropes.h"\n#else\n#include "Ropes_backend.h"\n#endif\n'
legacy+='''extern "C" bool TestR_LegacyRegister(uintptr_t id,const void *pos,bool timer)
{
    // Preserve the original typed value construction and pointer-width ID
#ifdef ORIGINAL_ROPES
    return CRopes::RegisterRope(id,*(const CVector *)pos,timer);
#else
    return Ropes_RegisterRope(id,Ropes_BackendCopyVector(*(const CVector *)pos),timer);
#endif
}
'''
(out/'legacy.cpp').write_text(legacy,newline='\n')
(out/'host.cpp').write_text(m.HOST.replace('#include "common.h"',commonInclude),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/ropes.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
(out/'no-fix.h').write_text('#pragma once\n#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all nine rope methods, complete visibility decision, C output adapter, typed owner boundaries and ten full source/project files')
