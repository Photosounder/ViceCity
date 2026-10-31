"""Audit every banner statement and compare unchanged original and real C records."""
from pathlib import Path
import re,json
import plane_banners_migration as m
import timecycle_migration as cycle
root=Path(__file__).resolve().parents[2];out=root/'build/plane-banners-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('PlaneBanners.h',m.HEADER),('PlaneBanners_backend.h',m.BACKEND_HEADER),('PlaneBanners.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
assert len(re.findall(r'\nCPlaneBanners?::\w+\(',m.section(original)))==8
for path in ['src/core/Camera.cpp','src/renderer/RenderBuffer.h','src/renderer/Rubbish_backend.h','src/math/Vector.h','src/math/maths.h','src/core/common.h']:cycle.verify_dependency(out/'before'/path,root/path)
portable=(root/'build/plane-trails-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(portable,newline='\n')
assert saved.read_text()==portable
(out/'common.h').write_text(portable,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['PlaneBanners.h','PlaneBanners_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
owners='''#pragma once
#include "Timer.h"
#include "RenderBuffer.h"
#include "Rubbish_backend.h"
class CCamera { public:CVector position;CVector &GetPosition(void);bool IsSphereVisible(const CVector &,float); };
extern CCamera TheCamera;
namespace CustomPipes { extern bool bRenderingEnvMap;extern RwCamera *EnvMapCam; }
'''
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''PlaneBanner TestPb_Get(int32_t index);void TestPb_Set(int32_t index,const PlaneBanner *record);
void TestPb_Init(void);void TestPb_Update(void);void TestPb_Render(void);
void TestPb_InitOne(int32_t index);void TestPb_UpdateOne(int32_t index);void TestPb_RenderOne(int32_t index);
void TestPb_Register(PlaneBannerVector pos,uint32_t id);void TestPb_RegisterVector(const void *pos,uint32_t id);
void TestPb_RegisterAlias(int32_t index,int32_t slot,uint32_t id);
PlaneBannerVector TestPb_Camera(void);
'''
(out/'caller.h').write_text('#pragma once\n#include "PlaneBanners.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
classes=m.classes(header).replace('class CPlaneBanner\n{','class CPlaneBanner\n{\npublic:').replace('class CPlaneBanners\n{','class CPlaneBanners\n{\npublic:')
(out/'original-classes.h').write_text(classes,newline='\n')
helpers=m.function(original,'','IsEffectSphereVisible')+'\n'+m.function(original,'','GetEffectCameraPosition')
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\n'+helpers+'\n'+m.section(original)+'\nextern "C" {\n'
after='#include "'+(root/'src/renderer/PlaneBanners.c').as_posix()+'"\n#include "caller.h"\n'
for old in [True,False]:
    text=before if old else after;array='CPlaneBanners::aArray' if old else 'aArray';kind='CPlaneBanner' if old else 'PlaneBanner'
    text+='PlaneBanner TestPb_Get(int32_t index)\n{\n    // Observe all eight actual declared position vectors component by component\n    '+kind+' *v=&'+array+'[index];PlaneBanner result;for(int i=0;i<8;i++){\n        // Copy named fields without assuming a CVector representation\n        result.m_pos[i].x=v->m_pos[i].x;result.m_pos[i].y=v->m_pos[i].y;result.m_pos[i].z=v->m_pos[i].z;\n    }return result;\n}\n'
    text+='void TestPb_Set(int32_t index,const PlaneBanner *record)\n{\n    // Seed each real position field while retaining the record identity\n    '+kind+' *v=&'+array+'[index];for(int i=0;i<8;i++){\n        // Update each declared field through its own actual type\n        v->m_pos[i].x=record->m_pos[i].x;v->m_pos[i].y=record->m_pos[i].y;v->m_pos[i].z=record->m_pos[i].z;\n    }\n}\n'
    for name in ['Init','Update','Render']:
        text+='void TestPb_'+name+'(void)\n{\n    // Enter the complete original or C controller algorithm\n    '+('CPlaneBanners::' if old else 'PlaneBanners_')+name+'();\n}\n'
        text+='void TestPb_'+name+'One(int32_t index)\n{\n    // Enter the complete actual record operation independently of controller traversal\n    '+(array+'[index].'+name+'()' if old else 'PlaneBanner_'+name+'(&aArray[index])')+';\n}\n'
    text+='void TestPb_Register(PlaneBannerVector pos,uint32_t id)\n{\n    // Retain the original independent by-value input before destination writes\n    '+('CPlaneBanners::RegisterPoint(CVector(pos.x,pos.y,pos.z),id)' if old else 'PlaneBanners_RegisterPoint(pos,id)')+';\n}\n'
    text+='void TestPb_RegisterVector(const void *pos,uint32_t id)\n{\n    // Exercise the actual typed application entry from a separate C translation unit\n    '+('CPlaneBanners::RegisterPoint(*(const CVector *)pos,id)' if old else 'PlaneBanners_RegisterPointVector(pos,id)')+';\n}\n'
    text+='void TestPb_RegisterAlias(int32_t index,int32_t slot,uint32_t id)\n{\n    // Capture a value borrowed from an actual destination record before assignment\n    '+('CPlaneBanners::RegisterPoint('+array+'[index].m_pos[slot],id)' if old else 'PlaneBanners_RegisterPoint(aArray[index].m_pos[slot],id)')+';\n}\n'
    text+='PlaneBannerVector TestPb_Camera(void)\n{\n    // Enter the complete original or C camera helper including all fallback gates\n    '+('CVector v=GetEffectCameraPosition();PlaneBannerVector result={v.x,v.y,v.z};return result;' if old else 'return PlaneBanners_GetCameraPosition();')+'\n}\n'
    if old:text+='}\n'
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
observed=(out/'observed-kernel.c').read_text();caller='#include "caller.h"\n'
for name in ['Init','Update','Render','Register','RegisterVector']:
    text=m.function(observed,'TestPb_',name);observed=observed.replace(text,'');caller+=text+'\n'
(out/'observed-kernel.c').write_text(observed+'\n_Static_assert(sizeof(PlaneBanner)==96,"Complete original record size");\n',newline='\n')
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n'+m.HOST,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/plane_banners.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all eight methods, both complete camera helpers, five records and seven entire source/project files')
