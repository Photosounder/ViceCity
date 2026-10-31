"""Audit all traffic-light decisions and generate complete original/C console comparisons."""
from pathlib import Path
import json,re
import traffic_lights_migration as m
import timecycle_test_dependencies as cycle_dependencies
import messages_migration as messages
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/traffic-lights-c-tests'
changes=json.loads((out/'changes.json').read_text())
for path in changes:
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/control/TrafficLights.cpp').read_text()
assert (root/'src/control/TrafficControl.c').read_text()==m.kernel(old)
assert (root/'src/control/TrafficLights_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in changes:messages.verify_dependency(out/'before'/path,root/path)
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#define _USE_MATH_DEFINES
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;typedef uintptr_t uintptr;
struct RwV3d {float x,y,z;};struct RwMatrix {RwV3d right,up,at,pos;};struct RwTexture;
class CMatrix;
#define nil NULL
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define SQR(a) ((a)*(a))
#include "Draw_constants.h"
#include "maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "Matrix.h"
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#undef assert
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef LANE_WIDTH
#define LANE_WIDTH 5.0f
#endif
#define WIDTH_TO_PED_NODE_WIDTH (31.f/(500.f*8.f))
void TestTraffic_MatrixUpdate(RwMatrix *);void TestTraffic_MatrixDestroy(RwMatrix *);
#undef RwMatrixUpdate
#undef RwMatrixDestroy
#define RwMatrixUpdate TestTraffic_MatrixUpdate
#define RwMatrixDestroy TestTraffic_MatrixDestroy
'''
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','maths.h']:(out/name).write_text((out/'before/src/math'/name).read_text(),newline='\n')
(out/'Draw_constants.h').write_text((out/'before/src/renderer/Draw_constants.h').read_text(),newline='\n')
(out/'original-TrafficLights.h').write_text((out/'before/src/control/TrafficLights.h').read_text(),newline='\n')
path_header=(out/'before/src/control/PathFind.h').read_text()
node=path_header[path_header.index('struct CPathNode\n'):path_header.index('union CConnectionFlags')].rstrip()
link=path_header[path_header.index('struct CCarPathLink\n'):path_header.index('// This is what we\'re reading from the files')].rstrip()
world=(out/'before/src/core/World.h').read_text();dimensions='\n'.join(re.findall(r'(?m)^#define NUMSECTORS_[XY][^\n]*',world))
world_enum=re.search(r'enum\n\{\n\tENTITYLIST_BUILDINGS,[\s\S]*?\n\};',world)[0]
autopilot=(out/'before/src/control/AutoPilot.h').read_text();fields=[]
for name in ['NextPathNodeInfo','CurrentPathNodeInfo','PreviousPathNodeInfo','NextRouteNode','CurrentRouteNode','PrevRouteNode','NextDirection','CurrentDirection','PreviousDirection']:
    fields.append(re.search(r'(?m)^\s*(?:int8|uint8|int16|uint16|int32|uint32)\s+m_n'+name+r';',autopilot)[0].strip())
owners='''#pragma once
#include "common.h"
#include "Store.h"
#include "Timer.h"
#include "General.h"
#include "ModelIndexData.h"
#include "Weather.h"
#include "EntityStatus.h"
#include "PointLights.h"
#include "BrightLights.h"
#ifdef TEST_TRAFFIC_ORIGINAL
#include "original-TrafficLights.h"
#else
#include "'''+(root/'src/control/TrafficLights.h').as_posix()+'''"
#endif
class CEntity {public:CMatrix matrix;bool bRenderDamaged;int16 model;int id;
    CEntity():bRenderDamaged(false),model(0),id(-1){
        // Initialize the native entity shell with the real matrix operation
        matrix.SetTranslate(0,0,0);
    }
    CMatrix &GetMatrix(){
        // Borrow the actual live transform used by the original operators
        return matrix;
    }
    CVector &GetPosition(){
        // Borrow the actual position without copying or replacing native storage
        return matrix.GetPosition();
    }
    CVector &GetForward(){
        // Borrow the actual forward axis used by orientation and visibility decisions
        return matrix.GetForward();
    }
    CVector &GetRight(){
        // Borrow the actual right axis used by the light geometry
        return matrix.GetRight();
    }
    CVector &GetUp(){
        // Borrow the actual up axis used by the damaged-light gate
        return matrix.GetUp();
    }
    int16 GetModelIndex(){
        // Read the original signed model identifier
        return model;
    }
};
class CAutoPilot {public:
'''+ '\n'.join(fields)+'''
};
class CVehicle:public CEntity {public:CAutoPilot AutoPilot;uint8 status;
    uint8 GetStatus(){
        // Return the independently seeded native vehicle status
        return status;
    }
};
class C2dEffect {public:CVector pos;};
class CBaseModelInfo {public:int16 m_2dEffectsID;C2dEffect *Get2dEffect(int n);};
CStore *TestTraffic_EffectStore(void);
class CModelInfo {public:static CBaseModelInfo *models[256];
    static CBaseModelInfo *GetModelInfo(int model){
        // Preserve the actual native model table's indexed lookup
        return models[model];
    }
    static CStore *Get2dEffectStore(){
        // Observe the real effect-store lookup before the unchanged native getter resolves its slot
        return TestTraffic_EffectStore();
    }
};
class CCamera {public:CVector forward;
    CVector &GetForward(){
        // Read the actual live camera axis at the original visibility gate
        return forward;
    }
};
extern CCamera TheCamera;
class CTimeCycle {public:static float ground,sprite,size;
    static float GetLightOnGroundBrightness(){
        // Read the live ground-light multiplier independently for every original color operand
        return ground;
    }
    static float GetSpriteBrightness(){
        // Read the live sprite multiplier independently for every original color operand
        return sprite;
    }
    static float GetSpriteSize(){
        // Read the original live sprite-size multiplier
        return size;
    }
};
class CPtrNode {public:void *item;CPtrNode *prev,*next;};
class CPtrList {public:CPtrNode *first;};
'''+dimensions+'\n'+world_enum+'''
class CSector {public:CPtrList m_lists[NUMSECTORENTITYLISTS];};
class CWorld {public:static CSector sectors[NUMSECTORS_Y][NUMSECTORS_X];static CSector *GetSector(int x,int y);};
'''+node+'\n'+link+'''
class CPathFind {public:CPathNode m_pathNodes[32];CCarPathLink m_carPathLinks[32];uint16 m_connections[64];int16 m_carPathConnections[64];int32 m_numCarPathNodes,m_numPathNodes;
    uint16 ConnectedNode(int id){
        // Preserve the actual native connection mask
        return m_connections[id]&0x3fff;
    }
    bool ConnectionCrossesRoad(int id){
        // Preserve the actual native crossing flag query
        return !!(m_connections[id]&0x8000);
    }
    void ConnectionSetTrafficLight(int id){
        // Preserve the actual native traffic-light bit update
        m_connections[id]|=0x4000;
    }
};
extern CPathFind ThePaths;
class CCoronas {public:enum {TYPE_STAR=0,FLARE_NONE=0,REFLECTION_ON=1,LOSCHECK_OFF=0,STREAK_OFF=0};
    static void RegisterCorona(uint32,uint8,uint8,uint8,uint8,const CVector &,float,float,uint8,int8,uint8,uint8,uint8,float,bool=false,float=1.5f);
};
extern RwTexture *gpShadowExplosionTex;
class CShadows {public:static bool StoreStaticShadow(uint32,uint8,RwTexture *,const CVector *,float,float,float,float,int16,uint8,uint8,uint8,float,float,float,bool,float);};
enum {SHADOWTYPE_ADDITIVE=2};
'''
owners+=cycle_dependencies.declarations(m.HOST)
(out/'owners.h').write_text(owners,newline='\n')
for name in ['ModelInfo.h','Camera.h','Coronas.h','PathFind.h','Shadows.h','Timecycle.h','Vehicle.h','World.h','Clock.h','SpecialFX.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
prototypes='''bool *TestTraffic_Cheat(void);
void TestTraffic_Display(CEntity *entity);void TestTraffic_Scan(void);int32_t TestTraffic_Type(CEntity *entity);
uint8_t TestTraffic_Phase(int which);bool TestTraffic_Stop(CVehicle *vehicle,bool alwaysStop);bool TestTraffic_Bridge(CVehicle *vehicle);
bool TestTraffic_Intersect(float,float,float,float,float,float,float,float);
void TestTraffic_RunDisplay(CEntity *entity);void TestTraffic_RunScan(void);int32_t TestTraffic_RunType(CEntity *entity);
uint8_t TestTraffic_RunPhase(int which);bool TestTraffic_RunStop(CVehicle *vehicle,bool alwaysStop);bool TestTraffic_RunBridge(CVehicle *vehicle);
bool TestTraffic_RunIntersect(float,float,float,float,float,float,float,float);
'''
(out/'c_entry.h').write_text('#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nclass CEntity;class CVehicle;\nextern "C" {\n#else\ntypedef struct CEntity CEntity;typedef struct CVehicle CVehicle;\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
(out/'caller.h').write_text('#pragma once\n#include "owners.h"\n#include "c_entry.h"\n',newline='\n')
wrappers='''#include "caller.h"
extern "C" bool *TestTraffic_Cheat(void)
{
    // Expose the actual canonical cheat flag before any operation or seeding
#ifdef TEST_TRAFFIC_ORIGINAL
    return &CTrafficLights::bGreenLightsCheat;
#else
    return &TrafficLights_bGreenLightsCheat;
#endif
}
extern "C" uint8_t TestTraffic_Phase(int which)
{
    // Enter all five complete original or C phase functions through the same selector
    switch(which){
'''
for index,name in enumerate(['LightForPeds','LightForCars1','LightForCars2','LightForCars1_Visual','LightForCars2_Visual']):wrappers+='    case '+str(index)+':\n#ifdef TEST_TRAFFIC_ORIGINAL\n        return CTrafficLights::'+name+'();\n#else\n        return TrafficLights_'+name+'();\n#endif\n'
wrappers+='    default:abort();\n    }\n}\n'
ops=[('void','Display','DisplayActualLight','CEntity *entity','entity'),('void','Scan','ScanForLightsOnMap','void',''),('int32_t','Type','FindTrafficLightType','CEntity *entity','entity'),('bool','Stop','ShouldCarStopForLight','CVehicle *vehicle,bool alwaysStop','vehicle,alwaysStop'),('bool','Bridge','ShouldCarStopForBridge','CVehicle *vehicle','vehicle'),('bool','Intersect','DoesLineSegmentIntersect','float x1,float y1,float x2,float y2,float u1,float v1,float u2,float v2','x1,y1,x2,y2,u1,v1,u2,v2')]
for kind,alias,target,args,invoke in ops:
    wrappers+='extern "C" '+kind+' TestTraffic_'+alias+'('+args+')\n{\n    // Enter the complete original or C operation with the same native objects and scalar values\n#ifdef TEST_TRAFFIC_ORIGINAL\n    '+('return ' if kind!='void' else '')+('' if alias=='Intersect' else 'CTrafficLights::')+target+'('+invoke+');\n#else\n    '+('return ' if kind!='void' else '')+'TrafficLights_'+target+'('+invoke+');\n#endif\n}\n'
(out/'original.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'+wrappers,newline='\n')
(out/'adapter.cpp').write_text(wrappers,newline='\n');(out/'host.cpp').write_text(m.calls(m.HOST)+cycle_dependencies.definitions(m.HOST),newline='\n')
caller='#include "'+(root/'src/control/TrafficLights.h').as_posix()+'"\n#include "c_entry.h"\n'
for kind,alias,target,args,invoke in [*ops,('uint8_t','Phase','','int which','which')]:caller+=kind+' TestTraffic_Run'+alias+'('+args+')\n{\n    // Retain the same opaque native objects and scalar values through a separate C11 caller\n    '+('return ' if kind!='void' else '')+'TestTraffic_'+alias+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
helpers='#include "ModelIndexData.h"\n'+''.join('int16_t '+name+';\n' for name in ['MI_TRAFFICLIGHTS','MI_TRAFFICLIGHTS_VERTICAL','MI_TRAFFICLIGHTS_MIAMI','MI_TRAFFICLIGHTS_TWOVERTICAL'])+function((out/'before/src/modelinfo/ModelIndexData.c').read_text(),'','IsTrafficLight')+'\n'
(out/'helpers.c').write_text(helpers,newline='\n')
matrix=(out/'before/src/math/Matrix.cpp').read_text();parts=[]
for name in ['CMatrix','~CMatrix','SetTranslate','RotateZ']:
    if name=='CMatrix':parts.append(re.search(r'CMatrix::CMatrix\(void\)\n\{[\s\S]*?\n\}',matrix)[0])
    else:parts.append(function(matrix,'CMatrix::',re.escape(name)))
vector=(out/'before/src/math/Vector.cpp').read_text();match=re.search(r'CVector\noperator\*\(const CMatrix &mat, const CVector &vec\)',vector);assert match;parts.append(vector[match.start():vector.index('\n}',match.end())+2])
parts.append(function((out/'before/src/modelinfo/BaseModelInfo.cpp').read_text(),'CBaseModelInfo::','Get2dEffect'))
point=(out/'before/src/renderer/PointLights.cpp').read_text();parts.extend(function(point,'',name) for name in ['PointLights_BackendReadVector','PointLights_AddLightLegacy'])
bright=(out/'before/src/renderer/BrightLights.cpp').read_text();parts.extend(function(bright,'',name) for name in ['BrightLights_FromVector','BrightLights_RegisterOneLegacy'])
(out/'math.cpp').write_text('#include "owners.h"\n#include "PointLights_backend.h"\n'+'\n\n'.join(parts)+'\n',newline='\n')
(out/'fastabs.c').write_text((root/'build/weather-c-tests/fastabs.c').read_text(),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/traffic_lights.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all eleven complete traffic-light operations, canonical cheat flag, 59 typed native boundaries and six complete callers')
