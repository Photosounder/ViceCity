"""Audit the whole smoke conversion and generate an unchanged original/C oracle."""
from pathlib import Path
import re,json
import smoke_trails_migration as m
import timecycle_migration as cycle
root=Path(__file__).resolve().parents[2];out=root/'build/smoke-trails-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('SmokeTrails.h',m.HEADER),('SmokeTrails_backend.h',m.BACKEND_HEADER),('SmokeTrails.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
for path in ['src/core/Camera.cpp','src/core/common.h','src/math/Vector.h','src/math/maths.h','src/renderer/RenderBuffer.h','src/animation/Bones.h','src/modelinfo/PedModelInfo.h','src/rw/RwHelper.h']:cycle.verify_dependency(out/'before'/path,root/path)
assert len(m.METHODS)==6 and m.GUARDED_KANGAROO in (root/'src/core/Pad.cpp').read_text()
portable=(root/'build/plane-banners-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(portable,newline='\n')
assert saved.read_text()==portable
(out/'common.h').write_text(portable,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['SmokeTrails.h','SmokeTrails_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
owners='''#pragma once
#include "Timer.h"
#include "RenderBuffer.h"
#ifndef NATIVE_PROJECT_HEADERS
struct RpHAnimHierarchy;
#endif
class CCamera { public:bool Using1stPersonWeaponMode(void);bool IsSphereVisible(const CVector &,float); };
extern CCamera TheCamera;
namespace CustomPipes { extern bool bRenderingEnvMap; }
class CWeather { public:static float Wind; };
class CPlayerPed { public:int identity;RpClump *GetClump(void); };
class CVehicle;
CPlayerPed *FindPlayerPed(void);CVehicle *FindPlayerVehicle(void);
class CCutsceneMgr { public:static bool IsRunning(void); };
enum { PED_HEAD=2 };
int ConvertPedNode2BoneTag(int);
RpHAnimHierarchy *GetAnimHierarchyFromSkinClump(RpClump *);
RwInt32 RpHAnimIDGetIndex(RpHAnimHierarchy *,RwInt32);
RwMatrix *RpHAnimHierarchyGetMatrixArray(RpHAnimHierarchy *);
RwV3d *RwV3dTransformPoints(RwV3d *,const RwV3d *,RwInt32,const RwMatrix *);
'''
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''SmokeTrail TestSm_Get(int32_t index);void TestSm_Set(int32_t index,const SmokeTrail *record);
bool TestSm_Flag(void);void TestSm_SetFlag(bool value);
void TestSm_Tables(uint16_t *indices,float *random);void TestSm_SetIndex(int32_t index,uint16_t value);void TestSm_SetRandom(int32_t index,float value);
void TestSm_Init(void);void TestSm_Update(void);void TestSm_Render(void);
void TestSm_InitOne(int32_t index,int32_t seed);void TestSm_RenderOne(int32_t index);
void TestSm_Register(int32_t index,SmokeTrailVector pos,float opacity);void TestSm_Alias(int32_t index,int32_t slot,float opacity);
'''
(out/'caller.h').write_text('#pragma once\n#include "SmokeTrails.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
classes=m.classes(header).replace('class CSmokeTrail {','class CSmokeTrail {\npublic:').replace('class CSmokeTrails {','class CSmokeTrails {\npublic:')
(out/'original-classes.h').write_text(classes,newline='\n')
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\n'+m.function(original,'','IsEffectSphereVisible')+'\n'+m.globals_section(original)+'\n'+'\n\n'.join(m.method(original,owner,name) for owner,name in m.METHODS)+'\nextern "C" {\n'
after='#include "'+(root/'src/renderer/SmokeTrails.c').as_posix()+'"\n#include "caller.h"\n'
for old in [True,False]:
    text=before if old else after;array='CSmokeTrails::aSmoke' if old else 'aSmoke';kind='CSmokeTrail' if old else 'SmokeTrail';flag='CSmokeTrails::CigOn' if old else 'SmokeTrails_CigOn'
    text+='SmokeTrail TestSm_Get(int32_t index)\n{\n    // Observe all declared original fields including the retained unused bytes\n    '+kind+' *v=&'+array+'[index];SmokeTrail result;for(int i=0;i<16;i++){\n        // Copy real component fields independently of CVector representation\n        result.m_pos[i].x=v->m_pos[i].x;result.m_pos[i].y=v->m_pos[i].y;result.m_pos[i].z=v->m_pos[i].z;result.m_opacity[i]=v->m_opacity[i];result.m_time[i]=v->m_time[i];\n    }for(int i=0;i<536;i++)result.m_unused[i]=v->m_unused[i];result.m_seed=v->m_seed;return result;\n}\n'
    text+='void TestSm_Set(int32_t index,const SmokeTrail *record)\n{\n    // Seed every actual record field while retaining its actual instance identity\n    '+kind+' *v=&'+array+'[index];for(int i=0;i<16;i++){\n        // Write through the original declared component types\n        v->m_pos[i].x=record->m_pos[i].x;v->m_pos[i].y=record->m_pos[i].y;v->m_pos[i].z=record->m_pos[i].z;v->m_opacity[i]=record->m_opacity[i];v->m_time[i]=record->m_time[i];\n    }for(int i=0;i<536;i++)v->m_unused[i]=record->m_unused[i];v->m_seed=record->m_seed;\n}\n'
    text+='bool TestSm_Flag(void)\n{\n    // Read the actual original or canonical C cigarette flag\n    return '+flag+';\n}\nvoid TestSm_SetFlag(bool value)\n{\n    // Change the actual shared flag without adding new initialization behavior\n    '+flag+'=value;\n}\n'
    text+='void TestSm_Tables(uint16_t *indices,float *random)\n{\n    // Observe complete actual mutable tables including the two zero-initialized random entries\n    for(int i=0;i<32;i++)indices[i]=SmokeTrailIndices[i];for(int i=0;i<16;i++)random[i]=RandomSmoke[i];\n}\nvoid TestSm_SetIndex(int32_t index,uint16_t value)\n{\n    // Mutate the original actually borrowed index entry at a defined draw boundary\n    SmokeTrailIndices[index]=value;\n}\nvoid TestSm_SetRandom(int32_t index,float value)\n{\n    // Change the actual original random table entry for subsequent point reads\n    RandomSmoke[index]=value;\n}\n'
    for name in ['Init','Update','Render']:text+='void TestSm_'+name+'(void)\n{\n    // Enter the complete original or C controller operation\n    '+('CSmokeTrails::' if old else 'SmokeTrails_')+name+'();\n}\n'
    text+='void TestSm_InitOne(int32_t index,int32_t seed)\n{\n    // Retain partial per-record initialization and original seed multiplication\n    '+(array+'[index].Init(seed)' if old else 'SmokeTrail_Init(&aSmoke[index],seed)')+';\n}\nvoid TestSm_RenderOne(int32_t index)\n{\n    // Enter the complete original or C record renderer\n    '+(array+'[index].Render()' if old else 'SmokeTrail_Render(&aSmoke[index])')+';\n}\n'
    text+='void TestSm_Register(int32_t index,SmokeTrailVector pos,float opacity)\n{\n    // Capture the original independent by-value registration input\n    '+(array+'[index].RegisterPoint(CVector(pos.x,pos.y,pos.z),opacity)' if old else 'SmokeTrail_RegisterPoint(&aSmoke[index],pos,opacity)')+';\n}\nvoid TestSm_Alias(int32_t index,int32_t slot,float opacity)\n{\n    // Capture a real destination vector before its overlapping descending shift\n    '+(array+'[index].RegisterPoint('+array+'[index].m_pos[slot],opacity)' if old else 'SmokeTrail_RegisterPoint(&aSmoke[index],aSmoke[index].m_pos[slot],opacity)')+';\n}\n'
    if old:text+='}\n'
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
observed=(out/'observed-kernel.c').read_text();caller='#include "caller.h"\n'
for name in ['Init','Update','Render']:
    text=m.function(observed,'TestSm_',name);observed=observed.replace(text,'');caller+=text+'\n'
(out/'observed-kernel.c').write_text(observed+'\n_Static_assert(sizeof(SmokeTrail)==860,"Complete original record size");\n',newline='\n')
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n'+m.HOST,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/smoke_trails.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all six methods, visibility helper, complete records/tables, typed point lifetime and six complete source/project files')
