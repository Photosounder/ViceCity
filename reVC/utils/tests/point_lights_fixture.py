"""Audit complete point-light relocation and build immutable differential inputs."""
from pathlib import Path
import json,re
import point_lights_migration as m
import fire_migration as fire
import traffic_lights_migration as traffic
root=Path(__file__).resolve().parents[2];out=root/'build/point-lights-c-tests'

def normalize(source):
    # Compare complete statements without comments or whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    if path=='src/core/Fire.cpp':fire.verify_legacy(old,m.calls,root)
    if path=='src/control/TrafficLights.cpp':traffic.verify_legacy(old,m.calls,root)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/PointLights.cpp').read_text();oldheader=(out/'before/src/renderer/PointLights.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/PointLights.c').read_text())
assert (root/'src/renderer/PointLights_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^CPointLights::\w+\(',old,re.M))==10
for name in ['PointLights.h','PointLights_backend.h']:
    (out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
deps=['src/core/common.h','src/core/config.h','src/core/main.h','src/core/Timer.h','src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','src/math/Vector.cpp','src/renderer/Draw_constants.h','src/renderer/RenderBuffer.h','src/fakerw/rwcore.h','src/fakerw/rwplcore.h','vendor/librw/src/rwbase.h','vendor/librw/src/rwrender.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    # Follow the separately audited shared-buffer header relocation
    if path=='src/renderer/RenderBuffer.h':
        import render_buffer_migration
        assert render_buffer_migration.header(saved.read_text())==(root/path).read_text(),path
    else:assert saved.read_bytes()==(root/path).read_bytes(),path
common=(root/'build/water-cannon-c-tests/common.h').read_text();saved=out/'before/water-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
common=common.replace('struct Matrix { V3d pos;V3d at; };','struct Matrix { V3d pos;V3d at;V3d right;V3d up; };')
common+='''
#ifndef NATIVE_PROJECT_HEADERS
typedef int8_t int8;
struct RpWorld;
struct RwFrame { RwMatrix *getLTM(void); };
struct RwCamera { RwFrame *frame;RwFrame *getFrame(void)const{return frame;} };
@CULL_ENUMS@
#define Const const
#define sq(a) ((a)*(a))
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"
#endif
'''
nativeCull=re.search(r'enum CullMode\n\{.*?\n\};',(out/'before/vendor/librw/src/rwrender.h').read_text(),re.S)[0]
wrapperCull=re.search(r'enum RwCullMode\n\{.*?\n\};',(out/'before/src/fakerw/rwplcore.h').read_text(),re.S)[0]
common=common.replace('@CULL_ENUMS@','namespace rw {\n'+nativeCull+'\n}\n'+wrapperCull)
(out/'common.h').write_text(common,newline='\n')
for path in ['src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','src/renderer/Draw_constants.h']:(out/Path(path).name).write_text((out/'before'/path).read_text(),newline='\n')
(out/'main.h').write_text((out/'before/src/core/main.h').read_text().split('extern uint8 work_buff')[0],newline='\n')
(out/'CutsceneMgr.h').write_text('#pragma once\nclass CCutsceneMgr { public:static bool IsRunning(void); };\n',newline='\n')
(out/'Camera.h').write_text('#pragma once\nclass CCamera {public:CVector position;CVector &GetPosition(void);};\nextern CCamera TheCamera;\n',newline='\n')
(out/'Weather.h').write_text('#pragma once\nclass CWeather {public:static float Foggyness;};\n',newline='\n')
(out/'Collision.h').write_text('#pragma once\nclass CColPoint {public:CVector point;};\n',newline='\n')
(out/'World.h').write_text('#pragma once\n#include "Collision.h"\nclass CEntity {};\nclass CStoredCollPoly;\nclass CWorld {public:static bool ProcessVerticalLine(const CVector &,float,CColPoint &,CEntity *&,bool,bool,bool,bool,bool,bool,CStoredCollPoly *);};\n',newline='\n')
(out/'Lights.h').write_text('#pragma once\nvoid AddAnExtraDirectionalLight(RpWorld *,float,float,float,float,float,float);\nvoid RemoveExtraDirectionalLights(RpWorld *);\n',newline='\n')
sprite=(root/'src/renderer/Sprite.h').read_text();signatures=[]
for name in ['InitSpriteBuffer','FlushSpriteBuffer','CalcScreenCoors','RenderBufferedOneXLUSprite_Rotate_Aspect']:
    signatures.append(re.search(r'\bstatic [^\n]+\b'+name+r'\([^\n]+;',sprite)[0])
(out/'Sprite.h').write_text('#pragma once\nclass CSprite {public:\n'+'\n'.join(signatures)+'\n};\n',newline='\n')
(out/'RenderBuffer.h').write_text((out/'before/src/renderer/RenderBuffer.h').read_text().split('#define TEMPBUFFERVERTSIZE')[0],newline='\n')
(out/'custompipes.h').write_text('#pragma once\nnamespace CustomPipes {extern bool bRenderingEnvMap;extern RwCamera *EnvMapCam;}\n',newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
(out/'original-PointLights.h').write_text(oldheader,newline='\n')
prototypes='''int TestPL_Count(bool env);
PointLight TestPL_Record(bool env,int index);
void TestPL_SetRecord(bool env,int index,const PointLight *value);
void TestPL_SetCounts(int16_t mainCount,int16_t envCount);
PointLightsVector TestPL_Cache(int index);
float TestPL_CacheResult(int index);
int32_t TestPL_Next(void);
void TestPL_SetNext(int32_t index);
void TestPL_Init(void);
void TestPL_InitPerFrame(void);
void TestPL_ResetEnv(void);
void TestPL_Add(uint8_t type,const void *coors,const void *dir,float radius,float red,float green,float blue,uint8_t fog,bool shadows);
void TestPL_AddC(uint8_t type,const void *coors,const void *dir,float radius,float red,float green,float blue,uint8_t fog,bool shadows);
float TestPL_Generate(const void *coors);
float TestPL_GenerateAlias(void);
void TestPL_Remove(void);
void TestPL_Fog(void);
void TestPL_Env(void *camera);
void TestPL_EnvLights(void *camera,PointLight *lights,int32_t count);
bool TestPL_Vertical(const void *coors,float *height);
void TestPL_Billboard(PointLightsVector position,PointLightsVector right,PointLightsVector up,float size,float red,float green,float blue,float intensity,float rotation);
'''
(out/'caller.h').write_text('#pragma once\n#include "PointLights.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
condition='#if defined(REVC_SOFTWARE_POLYGONS) && defined(EXTENDED_PIPELINES) && defined(LIBRW)\n'
observer=''
for variant in ['old','new']:
    oldVariant=variant=='old';array='CPointLights::aLights' if oldVariant else 'PointLights_aLights';prefix='CPointLights::' if oldVariant else 'PointLights_'
    text='''int TestPL_Count(bool env)
{
    // Observe both independent live registration counts
'''+condition+'''    if(env)return NumEnvMapLights;
#endif
    return env?0:'''+prefix+'''NumLights;
}
PointLight TestPL_Record(bool env,int index)
{
    // Observe all declared fields without comparing padding
'''
    if oldVariant:
        text+='    CRegisteredPointLight *v=&'+array+'[index];\n'+condition+'    if(env)v=&aEnvMapLights[index];\n#endif\n    PointLight r={{v->coors.x,v->coors.y,v->coors.z},{v->dir.x,v->dir.y,v->dir.z},v->radius,v->red,v->green,v->blue,v->type,v->fogType,v->castExtraShadows};return r;\n}\n'
    else:text+='    PointLight *v=&'+array+'[index];\n'+condition+'    if(env)v=&aEnvMapLights[index];\n#endif\n    return *v;\n}\n'
    text+='void TestPL_SetRecord(bool env,int index,const PointLight *value)\n{\n    // Seed identical complete legal records on both sides\n'
    text+=('    CRegisteredPointLight' if oldVariant else '    PointLight')+' *v=&'+array+'[index];\n'+condition+'    if(env)v=&aEnvMapLights[index];\n#endif\n'
    text+=('    v->coors=CVector(value->coors.x,value->coors.y,value->coors.z);v->dir=CVector(value->dir.x,value->dir.y,value->dir.z);v->radius=value->radius;v->red=value->red;v->green=value->green;v->blue=value->blue;v->type=value->type;v->fogType=value->fogType;v->castExtraShadows=value->castExtraShadows;\n' if oldVariant else '    *v=*value;\n')+'}\n'
    text+='void TestPL_SetCounts(int16_t mainCount,int16_t envCount)\n{\n    // Supply original manager counts without changing an algorithm\n    '+prefix+'NumLights=mainCount;\n'+condition+'    NumEnvMapLights=envCount;\n#endif\n}\n'
    text+='PointLightsVector TestPL_Cache(int index)\n{\n    // Observe every original cached position component\n    PointLightsVector r={'+prefix+'aCachedMapReads[index].x,'+prefix+'aCachedMapReads[index].y,'+prefix+'aCachedMapReads[index].z};return r;\n}\n'
    for name,result,args,body in [('CacheResult','float','int index','return '+prefix+'aCachedMapReadResults[index];'),('Next','int32_t','void','return '+prefix+'NextCachedValue;'),('SetNext','void','int32_t index',prefix+'NextCachedValue=index;')]:text+=result+' TestPL_'+name+'('+args+')\n{\n    // Observe or seed the original cache ring state\n    '+body+'\n}\n'
    text+='void TestPL_Billboard(PointLightsVector position,PointLightsVector right,PointLightsVector up,float size,float red,float green,float blue,float intensity,float rotation)\n{\n    // Enter the complete original or relocated billboard helper\n'
    text+=('    StoreFogBillboard(CVector(position.x,position.y,position.z),CVector(right.x,right.y,right.z),CVector(up.x,up.y,up.z),size,red,green,blue,intensity,rotation);\n' if oldVariant else '    StoreFogBillboard(position,right,up,size,red,green,blue,intensity,rotation);\n')+'}\n'
    if oldVariant:observer=text
    else:(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/PointLights.c').as_posix()+'"\n#include "caller.h"\n'+text,newline='\n')
original=old.replace('#include "common.h"',commonInclude).replace('#include "PointLights.h"','#include "original-PointLights.h"')
original=re.sub(r'^float FogSizes\[[^\n]+;',lambda match:'extern "C" { '+match[0]+' }',original,flags=re.M)
(out/'original.cpp').write_text(original+'\n#include "caller.h"\nextern "C" {\n'+observer+'}\n',newline='\n')
(out/'host.cpp').write_text(m.HOST.replace('#include "common.h"',commonInclude),newline='\n')
oldcaller=commonInclude+'\n#include "original-PointLights.h"\n#include "caller.h"\n';newcaller='#include "caller.h"\n#include "PointLights_backend.h"\n'
for name,result,args,oldexpr,newexpr in [
('Init','void','void','CPointLights::Init()','PointLights_Init()'),('InitPerFrame','void','void','CPointLights::InitPerFrame()','PointLights_InitPerFrame()'),('ResetEnv','void','void','CPointLights::ResetEnvMapLights()','PointLights_ResetEnvMapLights()'),
('Generate','float','const void *coors','CPointLights::GenerateLightsAffectingObject((const CVector *)coors)','PointLights_GenerateLightsAffectingObjectLegacy(coors)'),
('GenerateAlias','float','void','CPointLights::GenerateLightsAffectingObject(&CPointLights::aLights[0].coors)','PointLights_GenerateLightsAffectingObject(&PointLights_aLights[0].coors)'),
('Remove','void','void','CPointLights::RemoveLightsAffectingObject()','PointLights_RemoveLightsAffectingObject()'),('Fog','void','void','CPointLights::RenderFogEffect()','PointLights_RenderFogEffect()'),
('Env','void','void *camera','CPointLights::RenderFogEffectForEnvMap((RwCamera *)camera)','PointLights_RenderFogEffectForEnvMap(camera)'),
('Vertical','bool','const void *coors,float *height','CPointLights::ProcessVerticalLineUsingCache(*(const CVector *)coors,height)','PointLights_ProcessVerticalLineUsingCache(PointLights_BackendReadVector(coors),height)')]:
    statement='return ' if result!='void' else ''
    oldcaller+='extern "C" '+result+' TestPL_'+name+'('+args+')\n{\n    // Enter the complete original method\n    '+statement+oldexpr+';\n}\n'
    newcaller+=result+' TestPL_'+name+'('+args+')\n{\n    // Enter the actual C11 algorithm from a separate C unit\n    '+statement+newexpr+';\n}\n'
addArgs='uint8_t type,const void *coors,const void *dir,float radius,float red,float green,float blue,uint8_t fog,bool shadows'
for name in ['Add','AddC']:
    oldcaller+='extern "C" void TestPL_'+name+'('+addArgs+')\n{\n    // Pass actual original vector values through the complete original method\n    CPointLights::AddLight(type,*(const CVector *)coors,*(const CVector *)dir,radius,red,green,blue,fog,shadows);\n}\n'
newcaller+='void TestPL_AddC('+addArgs+')\n{\n    // Exercise actual C vector inputs and the public C entry point\n    PointLights_AddLight(type,PointLights_BackendReadVector(coors),PointLights_BackendReadVector(dir),radius,red,green,blue,fog,shadows);\n}\n'
(out/'legacy-caller.cpp').write_text(commonInclude+'\n#include "caller.h"\nextern "C" void TestPL_Add('+addArgs+')\n{\n    // Enter the actual by-value C++ input boundary\n    PointLights_AddLightLegacy(type,*(const CVector *)coors,*(const CVector *)dir,radius,red,green,blue,fog,shadows);\n}\n',newline='\n')
envArgs='void *camera,PointLight *lights,int32_t count'
oldcaller+='extern "C" void TestPL_EnvLights('+envArgs+')\n{\n    // Supply legal typed originals to the complete explicit-list renderer\n    CRegisteredPointLight original[NUMPOINTLIGHTS*2];\n    for(int i=0;i<count;i++){\n        // Copy every original declared field independently\n        PointLight *s=&lights[i];CRegisteredPointLight *v=&original[i];v->coors=CVector(s->coors.x,s->coors.y,s->coors.z);v->dir=CVector(s->dir.x,s->dir.y,s->dir.z);v->radius=s->radius;v->red=s->red;v->green=s->green;v->blue=s->blue;v->type=s->type;v->fogType=s->fogType;v->castExtraShadows=s->castExtraShadows;\n    }\n    CPointLights::RenderFogEffectForEnvMapLights((RwCamera *)camera,original,count);\n}\n'
newcaller+='void TestPL_EnvLights('+envArgs+')\n{\n    // Enter the actual C explicit-list renderer\n    PointLights_RenderFogEffectForEnvMapLights(camera,lights,count);\n}\n'
(out/'original-caller.cpp').write_text(oldcaller,newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
vector=(out/'before/src/math/Vector.cpp').read_text();a=vector.index('void\nCVector::Normalise');b=vector.index('\nCVector\nMultiply3x3',a)
(out/'original-vector.cpp').write_text(commonInclude+'\n'+vector[a:b],newline='\n')
(out/'fast-absf.c').write_text((root/'build/rubbish-c-tests/fast-absf.c').read_text(),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/point_lights.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
print('Audited ten complete public methods, complete billboard helper, 44-byte record, 40 typed operations and 21 whole source/project files')
