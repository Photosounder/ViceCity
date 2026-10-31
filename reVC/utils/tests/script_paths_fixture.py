import weather_migration as _weather_conversion
"""Audit complete scripted-path relocation and generate original/C runtime oracles."""
from pathlib import Path
import re,json
from audio_controls_migration import function
import script_paths_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/script-paths-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('ScriptPaths.h',m.HEADER),('ScriptPaths_backend.h',m.BACKEND_HEADER),('ScriptPaths.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
for path in ['src/core/common.h','src/core/Pool.h','src/core/Pools.h','src/core/Timer.h','src/core/Placeable.h','src/core/References.h','src/core/References.cpp','src/core/References.c','src/entities/Entity.h','src/entities/Entity.cpp','src/math/Vector.h','src/math/Vector.cpp','src/math/Matrix.h','src/math/Matrix.cpp','src/math/maths.h','src/objects/Object.h','src/objects/Object.cpp','src/vehicles/Plane.h','src/vehicles/Plane.cpp','src/save/SaveBuf.h','src/core/config.h']:_weather_conversion.verify_dependency(out/'before'/path,root/path)
common=(root/'build/escalator-record-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
common+='\n#ifndef NATIVE_PROJECT_HEADERS\n#define PI 3.14159265358979323846\n#define TWOPI (2.0*PI)\n#endif\n'
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['ScriptPaths.h','ScriptPaths_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
plane=(out/'before/src/vehicles/Plane.h').read_text();a=plane.index('struct CPlaneNode\n');b=plane.index('\n};',a)+3;node=plane[a:b]
owners='''#pragma once
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ScriptPaths.h"
#include "Pool.h"
#include "Timer.h"
#include "SaveBuf.h"
'''+node+'''
struct TestOffset { uint64_t extra; };
class CEntity { public:int32_t baseTag;void RegisterReference(CEntity **); };
class CObject:public TestOffset,public CEntity { public:
CMatrix matrix;CVector m_vecMoveSpeed,m_vecTurnSpeed,m_vecMoveFriction,m_vecTurnFriction;bool bIsBIGBuilding,m_phy_flagA08;int32_t identity;
CObject();~CObject();
const CVector &GetPosition(void){return matrix.GetPosition();}void SetPosition(const CVector &v){matrix.GetPosition()=v;}
CVector &GetRight(void){return matrix.GetRight();}CVector &GetUp(void){return matrix.GetUp();}CVector &GetForward(void){return matrix.GetForward();}CMatrix &GetMatrix(void){return matrix;}
void UpdateRwFrame(void);void RemoveAndAdd(void);
};
class CPools { public:static CPool *GetObjectPool(void); };
class CPlane { public:static CPlaneNode *LoadPath(const char *,int32_t &,float &,bool); };
extern "C" void *TestSp_Malloc(size_t);extern "C" void TestSp_Free(void *);
extern "C" void TestSp_MatrixBorn(CMatrix *);extern "C" void TestSp_MatrixDying(CMatrix *);
#ifdef malloc
#undef malloc
#endif
#ifdef free
#undef free
#endif
#define malloc(size) TestSp_Malloc(size)
#define free(memory) TestSp_Free(memory)
'''
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''ScriptPath TestSp_Get(int32_t index);void TestSp_Set(int32_t index,const ScriptPath *value);
void TestSp_Padding(int32_t index,uint8_t value);CObject **TestSp_Slot(int32_t index,int32_t slot);
int32_t *TestSp_CountSlot(int32_t index);float *TestSp_LengthSlot(int32_t index);
void TestSp_Init(void);void TestSp_Shutdown(void);void TestSp_Update(void);bool TestSp_Active(void);
void TestSp_Clear(int32_t index);void TestSp_UpdateOne(int32_t index);void TestSp_Initialise(int32_t index,int32_t file,float length);void TestSp_Control(int32_t index,CObject *object);
void TestSp_Find(int32_t index,float distance,float *x,float *y,float *z);
void TestSp_Save(uint8_t *buffer,uint32_t *size);void TestSp_Load(uint8_t *buffer,uint32_t size);void TestSp_SaveReplay(void);void TestSp_LoadReplay(void);
void TestSp_RunInit(void);void TestSp_RunShutdown(void);void TestSp_RunUpdate(void);void TestSp_RunSave(uint8_t *,uint32_t *);void TestSp_RunLoad(uint8_t *,uint32_t);
void TestSp_RunInitialise(int32_t,int32_t,float);void TestSp_RunControl(int32_t,CObject *);void TestSp_RunFind(int32_t,float,float *,float *,float *);
'''
(out/'caller.h').write_text('#pragma once\n#include "ScriptPaths.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
classes=m.classes(header);classes=classes[classes.index('class CScriptPath\n'):];(out/'original-classes.h').write_text(classes,newline='\n')
macro=re.search(r'#ifdef COMPATIBLE_SAVES\n#define SCRIPTPATHS_SAVE_SIZE 0x9C\n#else\n#define SCRIPTPATHS_SAVE_SIZE sizeof\(aArray\)\n#endif',original)[0]
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\n'+macro+'\nCScriptPath CScriptPaths::aArray[3];\nCObject *g_pScriptPathObjects[18];\n'+'\n\n'.join(m.method(original,owner,name) for owner,name in m.METHODS)+'\nextern "C" {\n'
after='#include "'+(root/'src/renderer/ScriptPaths.c').as_posix()+'"\n#include "caller.h"\n'
for old in [True,False]:
    text=before if old else after;kind='CScriptPath' if old else 'ScriptPath';array='CScriptPaths::aArray' if old else 'ScriptPaths_aArray';prefix='CScriptPaths::' if old else 'ScriptPaths_'
    text+='ScriptPath TestSp_Get(int32_t index)\n{\n    // Observe every declared field without copying unspecified record padding\n    '+kind+' *p=&'+array+'[index];ScriptPath result;'
    for field in m.FIELDS:text+=('for(int i=0;i<6;i++)result.m_pObjects[i]=p->m_pObjects[i];' if field=='m_pObjects' else 'result.'+field+'=p->'+field+';')
    text+='return result;\n}\nvoid TestSp_Set(int32_t index,const ScriptPath *value)\n{\n    // Seed real fields without substituting a shadow record\n    '+kind+' *p=&'+array+'[index];'
    for field in m.FIELDS:text+=('for(int i=0;i<6;i++)p->m_pObjects[i]=value->m_pObjects[i];' if field=='m_pObjects' else 'p->'+field+'=value->'+field+';')
    text+='\n}\nvoid TestSp_Padding(int32_t index,uint8_t value)\n{\n    // Poison only the actual padding gaps checked by the native layout probe\n    uint8_t *p=(uint8_t *)&'+array+'[index];for(size_t i=sizeof(int32_t);i<offsetof('+kind+',m_pNode);i++)p[i]=value;for(size_t i=offsetof('+kind+',m_state)+sizeof(int32_t);i<offsetof('+kind+',m_pObjects);i++)p[i]=value;\n}\n'
    text+='CObject **TestSp_Slot(int32_t index,int32_t slot)\n{\n    // Expose the actual stable pointer slot registered with the native entity interface\n    return &'+array+'[index].m_pObjects[slot];\n}\n'
    text+='int32_t *TestSp_CountSlot(int32_t index)\n{\n    // Expose the real node-count output field borrowed by the native loader\n    return &'+array+'[index].m_numNodes;\n}\nfloat *TestSp_LengthSlot(int32_t index)\n{\n    // Expose the real total-length output field borrowed by the native loader\n    return &'+array+'[index].m_fTotalLength;\n}\n'
    for name in ['Init','Shutdown','Update']:text+='void TestSp_'+name+'(void)\n{\n    // Enter the complete original or C controller operation\n    '+prefix+name+'();\n}\n'
    text+='bool TestSp_Active(void)\n{\n    // Preserve the complete original active-state and speed predicate\n    return '+prefix+'IsOneActive();\n}\n'
    for name,target in [('Clear','Clear'),('UpdateOne','Update')]:text+='void TestSp_'+name+'(int32_t index)\n{\n    // Enter the complete real record operation independently of controller traversal\n    '+(array+'[index].'+target+'()' if old else 'ScriptPath_'+target+'(&'+array+'[index])')+';\n}\n'
    text+='void TestSp_Initialise(int32_t index,int32_t file,float length)\n{\n    // Retain the filename identifier separately from the loader-produced node count\n    '+(array+'[index].InitialiseOne(file,length)' if old else 'ScriptPath_InitialiseOne(&'+array+'[index],file,length)')+';\n}\n'
    text+='void TestSp_Control(int32_t index,CObject *object)\n{\n    // Keep the actual typed object and original canonical pointer-slot address\n    '+(array+'[index].SetObjectToControl(object)' if old else 'ScriptPath_SetObjectToControl(&'+array+'[index],object)')+';\n}\n'
    text+='void TestSp_Find(int32_t index,float distance,float *x,float *y,float *z)\n{\n    // Preserve the original output pointer aliases and write order\n    '+(array+'[index].FindCoorsFromDistanceOnPath(distance,x,y,z)' if old else 'ScriptPath_FindCoorsFromDistanceOnPath(&'+array+'[index],distance,x,y,z)')+';\n}\n'
    for name in ['Save','Load']:text+='void TestSp_'+name+'(uint8_t *buffer,uint32_t '+('*size' if name=='Save' else 'size')+')\n{\n    // Enter the entire actual serialization operation including validation and padding behavior\n    '+prefix+name+'(buffer,size);\n}\n'
    for name in ['Save','Load']:text+='void TestSp_'+name+'Replay(void)\n{\n    // Preserve all eighteen actual replay pointer slots\n    '+prefix+name+'_ForReplay();\n}\n'
    if old:text+='}\n'
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n'+m.HOST,newline='\n')
caller='#include "caller.h"\n'
for name in ['Init','Shutdown','Update']:caller+='void TestSp_Run'+name+'(void)\n{\n    // Reach the original or actual C controller through a separate C11 caller\n    TestSp_'+name+'();\n}\n'
for name,args,invoke in [('Save','uint8_t *buffer,uint32_t *size','buffer,size'),('Load','uint8_t *buffer,uint32_t size','buffer,size'),('Initialise','int32_t index,int32_t file,float length','index,file,length'),('Control','int32_t index,CObject *object','index,object'),('Find','int32_t index,float distance,float *x,float *y,float *z','index,distance,x,y,z')]:caller+='void TestSp_Run'+name+'('+args+')\n{\n    // Retain the actual C parameter widths and borrowed buffer or output pointers\n    TestSp_'+name+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
matrix=(out/'before/src/math/Matrix.cpp').read_text();math='#include "common.h"\n#include "owners.h"\n'
for name in ['Normalise','CrossProduct']:math+=function((out/'before/src/math/Vector.cpp').read_text(),'CVector::' if name=='Normalise' else '',name)+'\n'
math+=function(matrix,'CMatrix::','CMatrix')+'\n';a=matrix.index('CMatrix::CMatrix(CMatrix const &m)');b=matrix.index('\n}',a)+2;copy=matrix[a:b];math+=copy[:-1]+'    // Observe the completed actual copy without mutating any game state\n    TestSp_MatrixBorn(this);\n}\n'
dtor=function(matrix,'CMatrix::','~CMatrix');dtor=dtor.replace('\n{\n','\n{\n    // Observe scope exit before the original native ownership check\n    TestSp_MatrixDying(this);\n',1);math+=dtor+'\n'
for name in ['operator=','UpdateRW']:math+=function(matrix,'CMatrix::',name)+'\n'
(out/'original-math.cpp').write_text(math,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/script_paths.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'raw.h').write_text('#include "config.h"\n#undef COMPATIBLE_SAVES\n#undef FIX_INCOMPATIBLE_SAVES\n',newline='\n')
(out/'fast-absf.c').write_text((root/'build/escalator-record-c-tests/fast-absf.c').read_text(),newline='\n')
print('Audited thirteen complete methods, three records, replay storage, native node/record save layouts and per-object native matrix lifetime')
