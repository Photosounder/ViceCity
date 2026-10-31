"""Audit and exercise the whole bridge controller with real pool, matrix and path-link code."""
from pathlib import Path
import json,re
import bridge_migration as m
import traffic_lights_migration as traffic
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/bridge-controller-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/control/BridgeControl.c').read_text()==m.kernel((out/'before/src/control/Bridge.cpp').read_text())
assert (root/'src/control/Bridge_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in json.loads((out/'changes.json').read_text()):traffic.verify_dependency(out/'before'/path,root/path)
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;
struct RwV3d {float x,y,z;};struct RwMatrix {RwV3d right,up,at,pos;};
struct RwObject {};struct RwFrame {};
class CMatrix;
#define nil NULL
#define LANE_WIDTH 5.0f
#include "maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "Matrix.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef LANE_WIDTH
#define LANE_WIDTH 5.0f
#endif
#define MI_BRIDGELIFT 10001
#define MI_BRIDGEROADSEGMENT 10002
#define MI_BRIDGEWEIGHT 10003
void TestBridge_MatrixUpdate(RwMatrix *);void TestBridge_MatrixDestroy(RwMatrix *);
void *TestBridge_ObjectParent(RwObject *);void TestBridge_FrameUpdate(RwFrame *);
#undef RwMatrixUpdate
#undef RwMatrixDestroy
#undef rwObjectGetParent
#undef RwFrameUpdateObjects
#define RwMatrixUpdate TestBridge_MatrixUpdate
#define RwMatrixDestroy TestBridge_MatrixDestroy
#define rwObjectGetParent TestBridge_ObjectParent
#define RwFrameUpdateObjects TestBridge_FrameUpdate
'''
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','maths.h']:(out/name).write_text((out/'before/src/math'/name).read_text(),newline='\n')
(out/'original-Bridge.h').write_text((out/'before/src/control/Bridge.h').read_text(),newline='\n')
path_header=(out/'before/src/control/PathFind.h').read_text();link=path_header[path_header.index('struct CCarPathLink\n'):path_header.index('// This is what we\'re reading from the files')].rstrip()
owners='''#pragma once
#include "common.h"
#include "Pool.h"
#include "Timer.h"
#ifdef TEST_BRIDGE_ORIGINAL
#include "original-Bridge.h"
#else
#include "'''+(root/'src/control/Bridge.h').as_posix()+'''"
#endif
class CEntity;
int TestBridge_Model(CEntity *entity);
class CEntity {
public:
    CMatrix matrix;RwObject *m_rwObject;int id,model;
    CEntity():m_rwObject(nil),id(-1),model(0){
        // Start each native shell with the actual matrix initialization
        matrix.SetTranslate(0,0,0);
    }
    CVector &GetPosition(){
        // Keep borrowed positions in the same native matrix
        return matrix.GetPosition();
    }
    CMatrix &GetMatrix(){
        // Expose the actual native transform and attachment
        return matrix;
    }
    int GetModelIndex(){
        // Observe each separate original model read in the reverse pool scan
        return TestBridge_Model(this);
    }
    void UpdateRwFrame(void);
};
struct TestBridgePrefix {uint64_t padding[3];};
class CBuilding:public TestBridgePrefix,public CEntity {};
class CPools {public:static CPool *GetBuildingPool(void);};
class CStats {public:static int32 CommercialPassed;};
'''+link+'''
class CPathFind {public:CCarPathLink m_carPathLinks[16];int32 m_numCarPathLinks;void SetLinksBridgeLights(float,float,float,float,bool);void ApplyLinks(float,float,float,float,bool);};
extern CPathFind ThePaths;
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Pools.h','ModelIndices.h','PathFind.h','Stats.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
state='typedef struct TestBridgeState {CEntity *road,*lift,*weight;int32_t state,oldState;float partZ,roadZ,weightZ,oldLift;uint32_t operational;} TestBridgeState;\n'
prototypes='''void TestBridge_Read(TestBridgeState *state);void TestBridge_Write(const TestBridgeState *state);
void TestBridge_Init(void);void TestBridge_Update(void);void TestBridge_Find(void);
bool TestBridge_Flashing(void);bool TestBridge_Moving(int32_t model);
void TestBridge_RunInit(void);void TestBridge_RunUpdate(void);void TestBridge_RunFind(void);
bool TestBridge_RunFlashing(void);bool TestBridge_RunMoving(int32_t model);
'''
(out/'caller.h').write_text('#pragma once\n#include "owners.h"\n'+state+'extern "C" {\n'+prototypes+'}\n',newline='\n')
wrappers='''#include "caller.h"
#ifdef TEST_BRIDGE_ORIGINAL
#define BRIDGE_FIELD(name) CBridge::name
#else
#define BRIDGE_FIELD(name) Bridge_##name
#endif
extern "C" void TestBridge_Read(TestBridgeState *state)
{
    // Read each actual canonical original or C field without a shadow controller
'''
fields=[('road','pLiftRoad'),('lift','pLiftPart'),('weight','pWeight'),('state','State'),('oldState','OldState'),('partZ','DefaultZLiftPart'),('roadZ','DefaultZLiftRoad'),('weightZ','DefaultZLiftWeight'),('oldLift','OldLift'),('operational','TimeOfBridgeBecomingOperational')]
wrappers+=''.join('    state->'+field+'=BRIDGE_FIELD('+native+');\n' for field,native in fields)+'}\nextern "C" void TestBridge_Write(const TestBridgeState *state)\n{\n    // Seed every actual canonical original or C field before the next operation\n'+''.join('    BRIDGE_FIELD('+native+')=state->'+field+';\n' for field,native in fields)+'}\n'
operations=[('void','Init','Init','void',''),('void','Update','Update','void',''),('void','Find','FindBridgeEntities','void',''),('bool','Flashing','ShouldLightsBeFlashing','void',''),('bool','Moving','ThisIsABridgeObjectMovingUp','int32_t model','model')]
caller='#include <stdint.h>\n#include <stdbool.h>\n'
for kind,alias,target,args,invoke in operations:
    wrappers+='extern "C" '+kind+' TestBridge_'+alias+'('+args+')\n{\n    // Enter the complete original or C operation with the same scalar arguments\n#ifdef TEST_BRIDGE_ORIGINAL\n    '+('return ' if kind!='void' else '')+'CBridge::'+target+'('+invoke+');\n#else\n    '+('return ' if kind!='void' else '')+'Bridge_'+target+'('+invoke+');\n#endif\n}\n'
    caller+=kind+' TestBridge_'+alias+'('+args+');\n'+kind+' TestBridge_Run'+alias+'('+args+')\n{\n    // Preserve the C scalar call boundary for the complete original and converted operation\n    '+('return ' if kind!='void' else '')+'TestBridge_'+alias+'('+invoke+');\n}\n'
original=(out/'before/src/control/Bridge.cpp').read_text()
(out/'original.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',original,flags=re.M)+'\n'+wrappers,newline='\n')
(out/'adapter.cpp').write_text(wrappers,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'host.cpp').write_text(m.HOST,newline='\n')
matrix=(out/'before/src/math/Matrix.cpp').read_text();parts=[]
for name in ['CMatrix','~CMatrix','SetTranslate','UpdateRW']:
    if name=='CMatrix':parts.append(re.search(r'CMatrix::CMatrix\(void\)\n\{[\s\S]*?\n\}',matrix)[0])
    else:parts.append(function(matrix,'CMatrix::',re.escape(name)))
parts.append(function((out/'before/src/entities/Entity.cpp').read_text(),'CEntity::','UpdateRwFrame'))
parts.append('#ifdef GTA_BRIDGE\n'+function((out/'before/src/control/PathFind.cpp').read_text(),'CPathFind::','SetLinksBridgeLights').replace('CPathFind::SetLinksBridgeLights','CPathFind::ApplyLinks')+'\n#endif')
(out/'math.cpp').write_text('#include "owners.h"\n'+'\n\n'.join(parts)+'\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/bridge.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all five bridge operations, ten canonical fields, twelve typed native boundaries and four complete callers')
