"""Audit full plane-trail relocation and generate an unchanged original/C oracle."""
from pathlib import Path
import re,json
import plane_trails_migration as m
import timecycle_migration as cycle
import timecycle_test_dependencies as cycle_dependencies
root=Path(__file__).resolve().parents[2];out=root/'build/plane-trails-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('PlaneTrails.h',m.HEADER),('PlaneTrails_backend.h',m.BACKEND_HEADER),('PlaneTrails.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
assert len(re.findall(r'\nCPlaneTrails?::\w+\(',m.section(original)))==7
for path in ['src/core/Camera.cpp','src/renderer/Coronas.h','src/renderer/RenderBuffer.h','src/math/Vector.h','src/math/maths.h','src/core/common.h']:cycle.verify_dependency(out/'before'/path,root/path)
portable=(root/'build/cloud-background-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(portable,newline='\n')
assert saved.read_text()==portable
(out/'common.h').write_text(portable,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['PlaneTrails.h','PlaneTrails_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
owners='''#pragma once
#include "Clock.h"
#include "Timer.h"
#include "RenderBuffer.h"
class CCamera { public:bool IsSphereVisible(const CVector &,float); };
extern CCamera TheCamera;
namespace CustomPipes { extern bool bRenderingEnvMap; }
class CWeather { public:static float Foggyness,CloudCoverage,Rain; };
class CTimeCycle { public:static int32 colors[3];static int32 GetSkyTopRed(void);static int32 GetSkyTopGreen(void);static int32 GetSkyTopBlue(void); };
class CCoronas { public:
enum { TYPE_NORMAL=1,FLARE_NONE=0,REFLECTION_OFF=0,LOSCHECK_OFF=0,STREAK_OFF=0 };
static void RegisterCorona(uint32,uint8,uint8,uint8,uint8,const CVector &,float,float,uint8,int8,uint8,uint8,uint8,float,bool useNearDist=false,float nearDist=1.5f);
static void UpdateCoronaCoors(uint32,const CVector &,float,float);
};
'''
owners+=cycle_dependencies.declarations(m.HOST)
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''PlaneTrail TestPt_Get(int32_t index);void TestPt_Set(int32_t index,const PlaneTrail *record);
void TestPt_Indices(uint16_t *out);void TestPt_SetIndex(int32_t index,uint16_t value);
void TestPt_Init(void);void TestPt_Update(void);void TestPt_Render(void);
void TestPt_RenderOne(int32_t index,float visibility);void TestPt_InitOne(int32_t index);
void TestPt_Register(PlaneTrailVector pos,uint32_t id);void TestPt_RegisterVector(const void *pos,uint32_t id);
void TestPt_RegisterAlias(int32_t index,int32_t slot,uint32_t id);
'''
(out/'caller.h').write_text('#pragma once\n#include "PlaneTrails.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
classes=m.classes(header).replace('class CPlaneTrail\n{','class CPlaneTrail\n{\npublic:').replace('class CPlaneTrails\n{','class CPlaneTrails\n{\npublic:')
(out/'original-classes.h').write_text(classes,newline='\n')
helper=m.function(original,'','IsEffectSphereVisible')
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\nCPlaneTrail CPlaneTrails::aArray[6];\n'+helper+'\n'+m.section(original)+'\nextern "C" {\n'
after='#include "'+(root/'src/renderer/PlaneTrails.c').as_posix()+'"\n#include "caller.h"\n'
for old in [True,False]:
    text=before if old else after;array='CPlaneTrails::aArray' if old else 'aArray';kind='CPlaneTrail' if old else 'PlaneTrail'
    text+='PlaneTrail TestPt_Get(int32_t index)\n{\n    // Observe every actual position component and signed timestamp independently\n    '+kind+' *v=&'+array+'[index];PlaneTrail result;for(int i=0;i<16;i++){\n        // Copy declared fields without assuming CVector storage\n        result.m_pos[i].x=v->m_pos[i].x;result.m_pos[i].y=v->m_pos[i].y;result.m_pos[i].z=v->m_pos[i].z;result.m_time[i]=v->m_time[i];\n    }return result;\n}\n'
    text+='void TestPt_Set(int32_t index,const PlaneTrail *record)\n{\n    // Seed each real field while retaining the actual record identity\n    '+kind+' *v=&'+array+'[index];for(int i=0;i<16;i++){\n        // Update named fields through their declared types\n        v->m_pos[i].x=record->m_pos[i].x;v->m_pos[i].y=record->m_pos[i].y;v->m_pos[i].z=record->m_pos[i].z;v->m_time[i]=record->m_time[i];\n    }\n}\n'
    text+='void TestPt_Indices(uint16_t *out)\n{\n    // Observe all actual index entries including the retained unused tail\n    for(int i=0;i<32;i++)out[i]=TrailIndices[i];\n}\nvoid TestPt_SetIndex(int32_t index,uint16_t value)\n{\n    // Mutate the actual mutable index entry at an original draw boundary\n    TrailIndices[index]=value;\n}\n'
    for name in ['Init','Update','Render']:text+='void TestPt_'+name+'(void)\n{\n    // Enter the complete original or C controller algorithm\n    '+('CPlaneTrails::' if old else 'PlaneTrails_')+name+'();\n}\n'
    text+='void TestPt_RenderOne(int32_t index,float visibility)\n{\n    // Enter the complete actual record renderer independently of controller gating\n    '+(array+'[index].Render(visibility)' if old else 'PlaneTrail_Render(&aArray[index],visibility)')+';\n}\n'
    text+='void TestPt_InitOne(int32_t index)\n{\n    // Exercise the original partial per-record initialization\n    '+(array+'[index].Init()' if old else 'PlaneTrail_Init(&aArray[index])')+';\n}\n'
    text+='void TestPt_Register(PlaneTrailVector pos,uint32_t id)\n{\n    // Preserve the by-value input capture before the original registration decisions\n    '+('CPlaneTrails::RegisterPoint(CVector(pos.x,pos.y,pos.z),id)' if old else 'PlaneTrails_RegisterPoint(pos,id)')+';\n}\n'
    text+='void TestPt_RegisterVector(const void *pos,uint32_t id)\n{\n    // Exercise the actual typed application entry through a separate translation unit\n    '+('CPlaneTrails::RegisterPoint(*(const CVector *)pos,id)' if old else 'PlaneTrails_RegisterPointVector(pos,id)')+';\n}\n'
    text+='void TestPt_RegisterAlias(int32_t index,int32_t slot,uint32_t id)\n{\n    // Capture a value from a real destination record before its overlapping descending shift\n    '+('CPlaneTrails::RegisterPoint('+array+'[index].m_pos[slot],id)' if old else 'PlaneTrails_RegisterPoint(aArray[index].m_pos[slot],id)')+';\n}\n'
    if old:text+='}\n'
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
observed=(out/'observed-kernel.c').read_text();caller='#include "caller.h"\n'
for name in ['Init','Update','Render','Register','RegisterVector']:
    text=m.function(observed,'TestPt_',name);observed=observed.replace(text,'');caller+=text+'\n'
(out/'observed-kernel.c').write_text(observed+'\n_Static_assert(sizeof(PlaneTrail)==256,"Complete record size");\n',newline='\n')
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n'+helper+'\n'+cycle.calls(m.HOST)+cycle_dependencies.definitions(m.HOST),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/plane_trails.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited seven complete methods, six complete records, exact typed boundary and six entire source/project files')
