"""Audit whole bright-light relocation and compile unchanged original algorithms against C11."""
from pathlib import Path
import re,json
import bright_lights_migration as m
import traffic_lights_migration as traffic
root=Path(__file__).resolve().parents[2];out=root/'build/bright-lights-c-tests'

def normalize(source):
    # Compare complete statements independently of edit comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    if path=='src/control/TrafficLights.cpp':traffic.verify_legacy(old,m.calls,root)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text()
for name,value in [('BrightLights.c',m.kernel(old)),('BrightLights.cpp',m.HOST),('BrightLights.h',m.HEADER),('BrightLights_backend.h',m.BACKEND_HEADER)]:assert (root/'src/renderer'/name).read_text()==value
assert len(re.findall(r'\nCBrightLights::\w+\(',m.section(old)))==4
for path in ['src/core/common.h','src/core/config.h','src/math/Vector.h','src/math/Vector.cpp','src/math/Vector2D.h','src/math/maths.h','src/renderer/RenderBuffer.h','src/renderer/RenderBuffer.cpp','src/renderer/RenderBuffer_backend.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
    if path.startswith('src/math/') and path.endswith('.h'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/shiny-texts-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text(common+'\n#ifndef NATIVE_PROJECT_HEADERS\nRwFrame *RwCameraGetFrame(const RwCamera *);\nRwMatrix *RwFrameGetMatrix(RwFrame *);\n#endif\n',newline='\n')
owners='''#pragma once
#include "BrightLights.h"
#include "RenderBuffer.h"
#include "General.h"
class CCamera { public:CVector position,right,up;CVector &GetPosition(void);CVector &GetRight(void);CVector &GetUp(void); };
extern CCamera TheCamera;
namespace CustomPipes { extern bool bRenderingEnvMap;extern RwCamera *EnvMapCam; }
void LittleTest(void);
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Camera.h','custompipes.h','main.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
oldh=m.record_header(header);enum=oldh[:oldh.index('class CBrightLight\n')]
probes='namespace OriginalBrightEnum {\n'+enum+'}\n'
for name in re.findall(r'\b(BRIGHTLIGHT_\w+)\b',m.ENUM):probes+='static_assert((int)OriginalBrightEnum::'+name+'==(int)'+name+',"Original '+name+' value");\n'
(out/'original-BrightLights.h').write_text('#pragma once\n#include "BrightLights.h"\n'+probes+oldh[oldh.index('class CBrightLight\n'):],newline='\n')
prototypes='''BrightLight TestBL_Record(bool env,int index);
void TestBL_SetRecord(bool env,int index,const BrightLight *value);
float TestBL_Flicker(bool env,int index);
void TestBL_SetFlicker(bool env,int index,float value);
void TestBL_Counts(int *mainCount,int *envCount);
void TestBL_SetCounts(int mainCount,int envCount);
void TestBL_Init(void);
void TestBL_Render(bool reuseFlicker);
void TestBL_Flush(void);
void TestBL_Register(const BrightLight *value);
void TestBL_RegisterLegacy(const BrightLight *value,bool defaults);
void TestBL_Alias(int index,bool legacy);
'''
(out/'caller.h').write_text('#pragma once\n#include "BrightLights.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
fields=['m_pos','m_up','m_side','m_front']
oldobs='#include "caller.h"\n';newobs='#include "caller.h"\n'
newobs+='_Static_assert(sizeof(BrightLight)==56 && _Alignof(BrightLight)==4,"Original record size and alignment");\n_Static_assert(offsetof(BrightLight,m_pos)==0 && offsetof(BrightLight,m_up)==12 && offsetof(BrightLight,m_side)==24 && offsetof(BrightLight,m_front)==36 && offsetof(BrightLight,m_camDist)==48 && offsetof(BrightLight,m_type)==52 && offsetof(BrightLight,m_red)==53 && offsetof(BrightLight,m_green)==54 && offsetof(BrightLight,m_blue)==55,"Original field offsets");\n'
oldobs+='extern "C" BrightLight TestBL_Record(bool env,int index)\n{\n    // Observe actual declared fields without reading padding or assuming vector layout\n    const CBrightLight &v=env?BrightLightsEnvMap[index]:CBrightLights::aBrightLights[index];BrightLight result={'+','.join('{v.'+f+'.x,v.'+f+'.y,v.'+f+'.z}' for f in fields)+',v.m_camDist,v.m_type,v.m_red,v.m_green,v.m_blue};return result;\n}\n'
oldobs+='extern "C" void TestBL_SetRecord(bool env,int index,const BrightLight *value)\n{\n    // Seed every original field through its actual typed owner\n    CBrightLight &v=env?BrightLightsEnvMap[index]:CBrightLights::aBrightLights[index];'+''.join('v.'+f+'=CVector(value->'+f+'.x,value->'+f+'.y,value->'+f+'.z);' for f in fields)+'v.m_camDist=value->m_camDist;v.m_type=value->m_type;v.m_red=value->m_red;v.m_green=value->m_green;v.m_blue=value->m_blue;\n}\n'
newobs+='BrightLight TestBL_Record(bool env,int index)\n{\n    // Observe every C-owned field through legal same-unit access\n    return env?BrightLightsEnvMap[index]:aBrightLights[index];\n}\nvoid TestBL_SetRecord(bool env,int index,const BrightLight *value)\n{\n    // Seed the same complete legal main or reflection record\n    if(env)BrightLightsEnvMap[index]=*value;else aBrightLights[index]=*value;\n}\n'
for variant in ['old','new']:
    text='';linkage='extern "C" ' if variant=='old' else '';main='CBrightLights::NumBrightLights' if variant=='old' else 'NumBrightLights'
    for name,result,args,body in [('Flicker','float','bool env,int index','return env?BrightLightsEnvMapFlicker[index]:BrightLightsFlicker[index];'),('SetFlicker','void','bool env,int index,float value','if(env)BrightLightsEnvMapFlicker[index]=value;else BrightLightsFlicker[index]=value;'),('Counts','void','int *mainCount,int *envCount','*mainCount='+main+';*envCount=BrightLightsEnvMapCount;'),('SetCounts','void','int mainCount,int envCount',main+'=mainCount;BrightLightsEnvMapCount=envCount;')]:text+=linkage+result+' TestBL_'+name+'('+args+')\n{\n    // Observe or seed the original private list and flicker state\n    '+body+'\n}\n'
    if variant=='old':oldobs+=text
    else:newobs+=text
oldargs=','.join('CVector(v->'+f+'.x,v->'+f+'.y,v->'+f+'.z)' for f in fields)+',v->m_type,v->m_red,v->m_green,v->m_blue'
oldobs+='extern "C" void TestBL_Register(const BrightLight *v)\n{\n    // Supply equivalent by-value vectors to the complete original registration\n    CBrightLights::RegisterOne('+oldargs+');\n}\n'
oldobs+='extern "C" void TestBL_RegisterLegacy(const BrightLight *v,bool defaults)\n{\n    // Exercise original typed arguments and the actual declared color defaults\n    if(defaults)CBrightLights::RegisterOne('+oldargs.rsplit(',v->m_red',1)[0]+');else CBrightLights::RegisterOne('+oldargs+');\n}\n'
oldobs+='extern "C" void TestBL_Alias(int index,bool)\n{\n    // Capture original pool subobjects before a camera callback changes their owner\n    CBrightLight &v=CBrightLights::aBrightLights[index];CBrightLights::RegisterOne(v.m_pos,v.m_up,v.m_side,v.m_front,v.m_type,v.m_red,v.m_green,v.m_blue);\n}\n'
for name,args,method in [('Init','void','Init()'),('Render','bool reuseFlicker','Render(reuseFlicker)'),('Flush','void','RenderOutGeometryBuffer()')]:oldobs+='extern "C" void TestBL_'+name+'('+args+')\n{\n    // Enter the whole original method\n    CBrightLights::'+method+';\n}\n'
newobs+='''void TestBL_Alias(int index,bool legacy)
{
    // Preserve the same actual C pool capture before either public input boundary
    BrightLight value=aBrightLights[index];
    if(legacy)TestBL_RegisterLegacy(&value,false);else BrightLights_RegisterOne(value.m_pos,value.m_up,value.m_side,value.m_front,value.m_type,value.m_red,value.m_green,value.m_blue);
}
'''
(out/'original.cpp').write_text('#include "common.h"\n#include "owners.h"\n#include "original-BrightLights.h"\n'+m.section(old)+'\n'+oldobs,newline='\n')
(out/'observed-kernel.c').write_text('#include <stddef.h>\n#include "'+(root/'src/renderer/BrightLights.c').as_posix()+'"\n'+newobs,newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'buffer-host.cpp').write_text((out/'before/src/renderer/RenderBuffer.cpp').read_text(),newline='\n')
vector=(out/'before/src/math/Vector.cpp').read_text();a=vector.index('void\nCVector::Normalise');b=vector.index('\nCVector\nCrossProduct',a)
(out/'original-vector.cpp').write_text('#include "common.h"\n'+vector[a:b]+'\n',newline='\n')
caller='#include "caller.h"\n'
for name,args,invoke in [('Init','void','BrightLights_Init()'),('Render','bool reuseFlicker','BrightLights_Render(reuseFlicker)'),('Flush','void','BrightLights_RenderOutGeometryBuffer()'),('Register','const BrightLight *v','BrightLights_RegisterOne(v->m_pos,v->m_up,v->m_side,v->m_front,v->m_type,v->m_red,v->m_green,v->m_blue)')]:caller+='void TestBL_'+name+'('+args+')\n{\n    // Enter the actual C11 algorithm from a separate C caller\n    '+invoke+';\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
legacy='#include "common.h"\n#include "caller.h"\nextern "C" void TestBL_RegisterLegacy(const BrightLight *v,bool defaults)\n{\n    // Enter the real typed compatibility boundary and its original defaults\n    if(defaults)BrightLights_RegisterOneLegacy('+oldargs.rsplit(',v->m_red',1)[0]+');else BrightLights_RegisterOneLegacy('+oldargs+');\n}\n'
(out/'legacy-caller.cpp').write_text(legacy,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/bright_lights.cpp.in').read_text(),newline='\n')
(out/'no-pipeline.h').write_text('#include "config.h"\n#undef EXTENDED_PIPELINES\n',newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited four complete methods, camera helper, all geometry/flicker tables, both class removals and ten entire source/project files')
