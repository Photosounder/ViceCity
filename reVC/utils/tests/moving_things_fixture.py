"""Audit complete scenery relocation and generate the unchanged original/C comparison."""
from pathlib import Path
import re,json
import model_indices_migration as models
from audio_controls_migration import function
import moving_things_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/moving-things-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('MovingThings.h',m.HEADER),('MovingThings_backend.h',m.BACKEND_HEADER),('MovingThings.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
for path in ['src/core/common.h','src/core/World.h','src/core/Lists.h','src/core/GameLevels.h','src/core/Camera.h','src/core/Placeable.h','src/math/Matrix.h','src/math/Vector.h','src/math/maths.h','src/entities/Entity.h','src/modelinfo/ModelIndices.h','src/renderer/Coronas.h']:
    if path=='src/modelinfo/ModelIndices.h':models.verify_header((out/'before'/path).read_text(),root)
    else:assert (out/'before'/path).read_bytes()==(root/path).read_bytes(),path
assert len(m.METHODS)==11
portable=(root/'build/smoke-trails-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(portable,newline='\n')
assert saved.read_text()==portable
(out/'common.h').write_text(portable,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['MovingThings.h','MovingThings_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
world=(out/'before/src/core/World.h').read_text();dimensions='\n'.join(line for line in world.splitlines() if line.startswith(('#define NUMSECTORS_X','#define NUMSECTORS_Y')))
owners='''#pragma once
#include "Timer.h"
#include "Clock.h"
#include "GameLevels.h"
#include "PlaneTrails.h"
#include "PlaneBanners.h"
#include "SmokeTrails.h"
#include "PointLights.h"
'''+dimensions+'''
class CEntity;
struct TestMatrix { CEntity *owner;void UpdateRW(void); };
class CEntity { public:
int identity;int16 model;CVector position,right,forward,up;TestMatrix matrix;
const CVector &GetPosition(void){return position;}
CVector &GetRight(void){return right;}CVector &GetForward(void){return forward;}CVector &GetUp(void){return up;}
void SetPosition(const CVector &value){position=value;}
TestMatrix &GetMatrix(void){return matrix;}
void UpdateRwFrame(void);int16 GetModelIndex(void)const{return model;}
};
class CCamera { public:CVector position;const CVector &GetPosition(void){return position;} };
extern CCamera TheCamera;
struct CPtrNode { void *item;CPtrNode *prev,*next; };
struct CPtrList { CPtrNode *first; };
enum { ENTITYLIST_BUILDINGS=0 };
struct CSector { CPtrList m_lists[1]; };
class CWorld { public:static CSector *GetSector(int,int);static CPtrList &GetBigBuildingList(eLevelName); };
enum eScrollBarTypes { SCROLL_ARENA_STRING };
class CScrollBar { public:
bool visible;int updates,renders;CVector first,last;uint8 type,r,g,b;float scale;
void Init(CVector,CVector,uint8,uint8,uint8,uint8,float);void SetVisibility(bool);
bool IsVisible(void);void Update(void);void Render(void);
void RenderForEnvMap(RwCamera *);
};
extern CScrollBar aScrollBars[1];
class CEscalators { public:static void Init(void);static void Shutdown(void);static void Update(void); };
class CCoronas { public:
enum { TYPE_STAR=0,REFLECTION_OFF=0,LOSCHECK_OFF=0,STREAK_OFF=0 };
static void RegisterCorona(uint32,uint8,uint8,uint8,uint8,const CVector &,float,float,uint8,int8,uint8,uint8,uint8,float,bool useNearDist=false,float nearDist=1.5f);
};
'''+''.join('extern int16 MI_'+name+';\n' for name in m.MODELS)
owners=owners.replace('#include "PointLights.h"','#include "PointLights.h"\n#include "ScrollBar.h"\n#include "Escalators.h"').replace('extern CScrollBar aScrollBars[1];','extern CScrollBar TestMovingRows[1];')
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''typedef struct TestMovingView { int32_t next,prev;int16_t type,distant;MovingThingVector position;void *entity; } TestMovingView;
TestMovingView TestMo_Get(int32_t index);void TestMo_Set(int32_t index,const TestMovingView *value);
int16_t TestMo_Num(void);void TestMo_SetNum(int16_t count);
void TestMo_Distances(float *out);void TestMo_SetDistance(int32_t index,float value);
void TestMo_Init(void);void TestMo_Shutdown(void);void TestMo_Update(void);void TestMo_Render(void);void TestMo_Env(void *camera);
void TestMo_Register(void *entity,uint16_t type);void TestMo_Possible(void *entity);
void TestMo_UpdateOne(int32_t index);void TestMo_Add(int32_t index,int32_t after);void TestMo_Remove(int32_t index);int16_t TestMo_Size(int32_t index);
'''
(out/'caller.h').write_text('#pragma once\n#include "MovingThings.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
(out/'original-classes.h').write_text(m.classes(header),newline='\n')
methods=[]
for owner,name in m.METHODS:
    text=m.method(original,owner,name)
    if name=='RenderForEnvMap':text='#ifdef REVC_SOFTWARE_POLYGONS\n'+text+'\n#endif'
    methods.append(text)
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\n#ifndef SQR\n#define SQR(a) ((a)*(a))\n#endif\n'+m.storage(original)+'\n'+m.distances(original)+'\n'+'\n\n'.join(methods)+'\nextern "C" {\n'
after='#include "'+(root/'src/renderer/MovingThings.c').as_posix()+'"\n#include "caller.h"\n'
for old in [True,False]:
    text=before if old else after;kind='CMovingThing' if old else 'MovingThing';prefix='CMovingThings::' if old else 'MovingThings_'
    text+='static '+kind+' *Object(int32_t index)\n{\n    // Normalize pool and sentinel identities without assuming their physical placement\n    if(index<0)return NULL;if(index==48)return &'+prefix+'StartCloseList;if(index==49)return &'+prefix+'EndCloseList;return &'+prefix+'aMovingThings[index];\n}\n'
    text+='static int32_t Identity('+kind+' *value)\n{\n    // Compare actual pointer relationships through stable logical record identities\n    if(!value)return -1;for(int32_t i=0;i<50;i++)if(value==Object(i))return i;return -2;\n}\n'
    text+='TestMovingView TestMo_Get(int32_t index)\n{\n    // Observe every declared record field independently of CVector or pointer layout\n    '+kind+' *v=Object(index);TestMovingView result;result.next=Identity(v->m_pNext);result.prev=Identity(v->m_pPrev);result.type=v->m_nType;result.distant=v->m_farAway;result.position.x=v->m_vecPosn.x;result.position.y=v->m_vecPosn.y;result.position.z=v->m_vecPosn.z;result.entity=v->m_pEntity;return result;\n}\n'
    text+='void TestMo_Set(int32_t index,const TestMovingView *value)\n{\n    // Seed actual fields and actual logical links through their declared types\n    '+kind+' *v=Object(index);v->m_pNext=Object(value->next);v->m_pPrev=Object(value->prev);v->m_nType=value->type;v->m_farAway=value->distant;v->m_vecPosn.x=value->position.x;v->m_vecPosn.y=value->position.y;v->m_vecPosn.z=value->position.z;v->m_pEntity=(CEntity *)value->entity;\n}\n'
    text+='int16_t TestMo_Num(void)\n{\n    // Read the original live signed record count\n    return '+prefix+'Num;\n}\nvoid TestMo_SetNum(int16_t count)\n{\n    // Modify the real count at a defined owner callback\n    '+prefix+'Num=count;\n}\n'
    text+='void TestMo_Distances(float *out)\n{\n    // Observe all five actual mutable distance entries\n    for(int i=0;i<5;i++)out[i]=maxUpdateDists[i];\n}\nvoid TestMo_SetDistance(int32_t index,float value)\n{\n    // Preserve the real mutable distance table across controller initialization\n    maxUpdateDists[index]=value;\n}\n'
    for name in ['Init','Shutdown','Update','Render']:text+='void TestMo_'+name+'(void)\n{\n    // Enter the complete original or C controller algorithm\n    '+prefix+name+'();\n}\n'
    text+='void TestMo_Env(void *camera)\n{\n    // Enter the optional original or C environment-camera pass with the same opaque owner\n#ifdef REVC_SOFTWARE_POLYGONS\n    '+prefix+'RenderForEnvMap('+('(RwCamera *)camera' if old else 'camera')+');\n#else\n    (void)camera;\n#endif\n}\n'
    for name,target,args in [('Register','RegisterOne','void *entity,uint16_t type'),('Possible','PossiblyAddThisEntity','void *entity')]:text+='void TestMo_'+name+'('+args+')\n{\n    // Retain the actual typed entity and original public argument widths\n    '+prefix+target+'((CEntity *)entity'+(',type' if name=='Register' else '')+');\n}\n'
    text+='void TestMo_UpdateOne(int32_t index)\n{\n    // Exercise the complete real per-record animation and distance/list decisions\n    '+('Object(index)->Update()' if old else 'MovingThing_Update(Object(index))')+';\n}\n'
    text+='void TestMo_Add(int32_t index,int32_t after)\n{\n    // Link actual records in the original intrusive-list order\n    '+('Object(index)->AddToList(Object(after))' if old else 'MovingThing_AddToList(Object(index),Object(after))')+';\n}\nvoid TestMo_Remove(int32_t index)\n{\n    // Preserve retained links after removing the actual record\n    '+('Object(index)->RemoveFromList()' if old else 'MovingThing_RemoveFromList(Object(index))')+';\n}\nint16_t TestMo_Size(int32_t index)\n{\n    // Retain the original signed count including the terminal sentinel\n    return '+('Object(index)->SizeList()' if old else 'MovingThing_SizeList(Object(index))')+';\n}\n'
    if old:text+='}\n'
    if old:text=text.replace('aScrollBars','TestMovingRows')
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
observed=(out/'observed-kernel.c').read_text();caller='#include "caller.h"\n'
for name in ['Init','Shutdown','Update','Render','Env','Register','Possible']:
    match=re.search(r'void TestMo_'+name+r'\([^\n]*\)\n\{',observed);assert match,name
    a=observed.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(observed[b]=='{')-(observed[b]=='}');b+=1
    text=observed[match.start():b];observed=observed.replace(text,'');caller+=text+'\n'
(out/'observed-kernel.c').write_text(observed,newline='\n');(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n'+m.HOST,newline='\n')
test=(root/'utils/tests/moving_things.cpp.in').read_text().replace('aScrollBars','TestMovingRows')
test+='''
extern "C" {
ScrollBar aScrollBars[NUMSCROLLBARS];
void ScrollBar_Init(ScrollBar *bar,ScrollBarVector first,ScrollBarVector last,uint8_t type,uint8_t r,uint8_t g,uint8_t b,float scale)
{
    // Keep the controller comparison attached to the same external sign simulator
    TestMovingRows[bar-aScrollBars].Init(CVector(first.x,first.y,first.z),CVector(last.x,last.y,last.z),type,r,g,b,scale);
}
void ScrollBar_SetVisibility(ScrollBar *bar,bool visible)
{
    // Compare the direct C call against the original sign-owner visibility operation
    TestMovingRows[bar-aScrollBars].SetVisibility(visible);
}
bool ScrollBar_IsVisible(ScrollBar *bar)
{
    // Read the same simulated external component through the new public C boundary
    return TestMovingRows[bar-aScrollBars].IsVisible();
}
void ScrollBar_Update(ScrollBar *bar)
{
    // Retain all existing sign-update callbacks and controller reentry probes
    TestMovingRows[bar-aScrollBars].Update();
}
void ScrollBar_Render(ScrollBar *bar)
{
    // Retain all existing sign-render callbacks and scheduling observations
    TestMovingRows[bar-aScrollBars].Render();
}
#ifdef REVC_SOFTWARE_POLYGONS
void ScrollBar_RenderForEnvMap(ScrollBar *bar,void *camera)
{
    // Forward the exact optional camera to the existing sign simulator
    TestMovingRows[bar-aScrollBars].RenderForEnvMap((RwCamera *)camera);
}
#endif
}
'''
test+='''
extern "C" void Escalators_Init(void)
{
    // Keep the controller comparison attached to the same external escalator simulator
    CEscalators::Init();
}
extern "C" void Escalators_Update(void)
{
    // Preserve the existing escalator callback timing before moving-scenery scheduling
    CEscalators::Update();
}
extern "C" void Escalators_Shutdown(void)
{
    // Preserve the existing child shutdown observation through the direct C API
    CEscalators::Shutdown();
}
'''
(out/'test.cpp').write_text(test,newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
absolute=(root/'build/cloud-background-c-tests/fast-absf.c').read_text();actual=Path('C:/msys/home/rouziclib/math/ieee754.c').read_text()
normalize=lambda text:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',text))
assert normalize(function(absolute,'','fast_absf'))==normalize(function(actual,'','fast_absf'))
(out/'fast-absf.c').write_text(absolute,newline='\n')
print('Audited all eleven complete methods, retained list links, exact typed boundary and seven complete source/project files')
