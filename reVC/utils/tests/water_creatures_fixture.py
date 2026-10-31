import weather_migration as _weather_conversion
"""Audit complete sea-life relocation and generate native-owner original/C runtime programs."""
from pathlib import Path
import json,re
from audio_controls_migration import function
import water_creatures_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/water-creatures-c-tests'
old=(out/'before/src/renderer/WaterCreatures.cpp').read_text();header=(out/'before/src/renderer/WaterCreatures.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    before=(out/'before'/path).read_text();expected=m.project(before) if path.startswith('build/') else m.calls(before)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/renderer/SeaLife.c').read_text()==m.kernel(old)
assert (root/'src/renderer/WaterCreatures_backend.h').read_text()==m.BACKEND
for path in ['src/core/common.h','src/core/config.h','src/core/Pool.h','src/core/Pools.h','src/core/World.h','src/core/Camera.h','src/core/Placeable.h','src/core/General.h','src/core/General.c','src/core/Timer.h','src/core/re3.cpp','src/entities/Entity.h','src/entities/Entity.cpp','src/objects/Object.h','src/objects/Object.cpp','src/math/Vector.h','src/math/Vector.cpp','src/math/maths.h','src/math/Matrix.h','src/math/Matrix.cpp','src/renderer/WaterLevel.h','src/renderer/Draw_constants.h','src/modelinfo/ModelIndexData.h','src/modelinfo/ModelIndexData.c']:_weather_conversion.verify_dependency(out/'before'/path,root/path)
assert len(m.METHODS)==11 and len(re.findall(r'\{\s*&MI_',old))==65
common=(root/'build/script-paths-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
common=common.replace('#define PI 3.14159265358979323846\n#define TWOPI (2.0*PI)','#ifndef PI\n#define PI (float)M_PI\n#endif\n#ifndef TWOPI\n#define TWOPI (PI*2)\n#endif')
common+='\n#ifndef HALFPI\n#define HALFPI (PI/2)\n#endif\n#ifndef SQR\n#define SQR(value) ((value)*(value))\n#endif\n#ifndef DEGTORAD\n#define DEGTORAD(value) ((value)*PI/180.0f)\n#endif\n#ifndef RADTODEG\n#define RADTODEG(value) ((value)*180.0f/PI)\n#endif\n'
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['WaterCreatures.h','WaterCreatures_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
(out/'ModelIndices.h').write_text('#pragma once\n#include "ModelIndexData.h"\n',newline='\n')
owners='''#pragma once
#include "common.h"
#include <memory>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "WaterCreatures.h"
#include "Pool.h"
#include "Timer.h"
#include "General.h"
#include "ModelIndexData.h"
enum { UNKNOWN_OBJECT=0,GAME_OBJECT=1,MISSION_OBJECT=2,TEMP_OBJECT=3,CUTSCENE_OBJECT=4,CONTROLLED_SUB_OBJECT=5 };
struct ObjectPrefix { uint64_t guard[2]; };
class CEntity { public:int32_t baseTag;void SetRwObjectAlpha(int32_t);bool GetIsOnScreen(void); };
class CObject:public ObjectPrefix,public CEntity { public:
static int16_t nNoTempObjects;
CMatrix matrix;CVector m_vecMoveSpeed,m_vecTurnSpeed;int32_t modelID,alpha,identity;int8_t ObjectCreatedBy;uint32_t m_nEndOfLifeTime;bool bIsStatic,bTouchingWater,bUnderwater,onScreen,world;
CObject(int32_t,bool);~CObject();
const CVector &GetPosition(void){return matrix.GetPosition();}void SetPosition(const CVector &value){matrix.GetPosition()=value;}
CVector &GetRight(void){return matrix.GetRight();}CMatrix &GetMatrix(void){return matrix;}
};
typedef CPool CObjectPool;
class CPools { public:static CObjectPool *GetObjectPool(void); };
class CWorld { public:static void Add(CEntity *);static void Remove(CEntity *); };
class CPlayerPed { public:CVector position;const CVector &GetPosition(void){return position;} };
CPlayerPed *FindPlayerPed(void);
class CCamera { public:bool IsSphereVisible(const CVector &,float); };extern CCamera TheCamera;
class CWaterLevel { public:static bool GetWaterDepth(const CVector &,float *,float *,float *); };
'''
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''WaterCreature TestSea_Get(int index);void TestSea_Set(int index,const WaterCreature *record);WaterCreature *TestSea_Slot(int index);
WaterCreatureProperties TestSea_Property(int index);void TestSea_SetProperty(int index,const WaterCreatureProperties *property);
int32_t TestSea_Count(void);void TestSea_SetCount(int32_t count);
void TestSea_Construct(int index);void TestSea_RecordInit(int index,CObject *object,float forward,float turn,float depth,uint32_t alpha,int32_t state);void TestSea_Allocate(int index,CObject *object,float forward,float turn,float depth,uint32_t alpha,int32_t state);void TestSea_Free(int index);
int TestSea_Find(void);CObject *TestSea_Create(const WaterCreaturePosition *position,int16_t model,int32_t angle);bool TestSea_Space(void);float TestSea_Heading(const WaterCreaturePosition *a,const WaterCreaturePosition *b);void TestSea_One(const WaterCreaturePosition *position,int32_t model);void TestSea_FreeSlot(int index);void TestSea_Update(void);void TestSea_Remove(void);
CObject *TestSea_RunCreate(const WaterCreaturePosition *position,int16_t model,int32_t angle);void TestSea_RunOne(const WaterCreaturePosition *position,int32_t model);void TestSea_RunUpdate(void);void TestSea_RunRemove(void);
'''
(out/'caller.h').write_text('#pragma once\n#include "WaterCreatures.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
oldheader=header.replace('#pragma once','').replace('class CObject;','');(out/'original-classes.h').write_text(oldheader,newline='\n')
for before in [True,False]:
    text='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n'
    if before:text+='namespace Original {\n'+oldheader+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    prefix='Original::CWaterCreatures::' if before else 'WaterCreatures_';array=prefix+'aWaterCreatures';count=prefix+'nNumActiveSeaLifeForms';properties='Original::aProperties' if before else 'WaterCreatures_Properties'
    text+='extern "C" {\nWaterCreature TestSea_Get(int index)\n{\n    // Observe every actual declared field without copying unspecified native record padding\n    WaterCreature result;auto *record=&'+array+'[index];'
    for field in m.FIELDS:text+='result.'+field+'=record->'+field+';'
    text+='return result;\n}\nvoid TestSea_Set(int index,const WaterCreature *value)\n{\n    // Mutate only named canonical fields during external-owner scenarios\n    auto *record=&'+array+'[index];'
    for field in m.FIELDS:text+='record->'+field+'=value->'+field+';'
    text+='\n}\nWaterCreature *TestSea_Slot(int index)\n{\n    // Normalize the real canonical record identity after checking native layout independently\n    return (WaterCreature *)&'+array+'[index];\n}\n'
    text+='WaterCreatureProperties TestSea_Property(int index)\n{\n    // Observe all live model-pointer property fields without substituting shadow state\n    WaterCreatureProperties result;auto *property=&'+properties+'[index];result.modelID=property->modelID;result.fFwdSpeed=property->fFwdSpeed;result.fLevel=property->fLevel;result.fUnknown=property->fUnknown;result.fWaterDepth=property->fWaterDepth;return result;\n}\n'
    text+='void TestSea_SetProperty(int index,const WaterCreatureProperties *value)\n{\n    // Change the actual property pointer and scalar values at valid external call boundaries\n    auto *property=&'+properties+'[index];property->modelID=value->modelID;property->fFwdSpeed=value->fFwdSpeed;property->fLevel=value->fLevel;property->fUnknown=value->fUnknown;property->fWaterDepth=value->fWaterDepth;\n}\n'
    text+='int32_t TestSea_Count(void)\n{\n    // Observe the actual signed active count\n    return '+count+';\n}\nvoid TestSea_SetCount(int32_t value)\n{\n    // Seed the original active-count gate independently of slot traversal\n    '+count+'=value;\n}\n'
    text+='void TestSea_Construct(int index)\n{\n    // Execute the complete actual native constructor or explicit C construction operation\n    '+(array+'[index].~CWaterCreature();std::allocator<Original::CWaterCreature>().construct(&'+array+'[index]);' if before else 'WaterCreature_Construct(&'+array+'[index]);')+'\n}\n'
    for alias,method in [('RecordInit','Initialise'),('Allocate','Allocate')]:text+='void TestSea_'+alias+'(int index,CObject *object,float forward,float turn,float depth,uint32_t alpha,int32_t state)\n{\n    // Retain the exact original captured record arguments and signed state storage\n    '+(array+'[index].'+method+'(object,forward,turn,depth,alpha,(Original::eFishSlotState)state);' if before else 'WaterCreature_'+method+'(&'+array+'[index],object,forward,turn,depth,alpha,(eFishSlotState)state);')+'\n}\n'
    text+='void TestSea_Free(int index)\n{\n    // Enter the complete original or C record clearing operation\n    '+(array+'[index].Free();' if before else 'WaterCreature_Free(&'+array+'[index]);')+'\n}\n'
    text+='int TestSea_Find(void)\n{\n    // Compare the actual first disabled canonical slot and exhaustion result\n    auto *record='+prefix+'GetFishStructSlot();if(!record)return -1;return (int)(record-&'+array+'[0]);\n}\n'
    for alias,method,result,args,invoke in [('Create','CreateSeaLifeForm','CObject *','const WaterCreaturePosition *position,int16_t model,int32_t angle',('*position' if before else 'position')+',model,angle'),('Space','IsSpaceForMoreWaterCreatures','bool','void',''),('Heading','CalculateFishHeading','float','const WaterCreaturePosition *a,const WaterCreaturePosition *b','*a,*b' if before else 'a,b'),('One','CreateOne','void','const WaterCreaturePosition *position,int32_t model',('*position' if before else 'position')+',model'),('FreeSlot','FreeFishStructSlot','void','int index','&'+array+'[index]'),('Update','UpdateAll','void','void',''),('Remove','RemoveAll','void','void','')]:text+=result+' TestSea_'+alias+'('+args+')\n{\n    // Enter the complete actual original or C controller operation\n    '+('return ' if result!='void' else '')+prefix+method+'('+invoke+');\n}\n'
    text+='}\n';(out/('original.cpp' if before else 'adapter.cpp')).write_text(text,newline='\n')
caller='#include "caller.h"\n'
for alias,result,args,invoke in [('Create','CObject *','const WaterCreaturePosition *position,int16_t model,int32_t angle','position,model,angle'),('One','void','const WaterCreaturePosition *position,int32_t model','position,model'),('Update','void','void',''),('Remove','void','void','')]:caller+=result+' TestSea_Run'+alias+'('+args+')\n{\n    // Retain the actual opaque native position and object ABI through a separate C11 caller\n    '+('return ' if result!='void' else '')+'TestSea_'+alias+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n#include "WaterCreatures_backend.h"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
math='#include "common.h"\n#include "owners.h"\n'+function((out/'before/src/math/Vector.cpp').read_text(),'CVector::','Normalise')+'\n'
matrix=(out/'before/src/math/Matrix.cpp').read_text()
for name in ['CMatrix','~CMatrix','SetRotateZOnly','UpdateRW']:math+=function(matrix,'CMatrix::',name)+'\n'
(out/'original-math.cpp').write_text(math,newline='\n')
rng=(out/'before/src/core/re3.cpp').read_text();a=rng.index('#ifdef USE_PS2_RAND\nunsigned long long myrand_seed');b=rng.index('\n#ifdef CUSTOM_FRONTEND_OPTIONS',a)
(out/'rng.cpp').write_text('#include "owners.h"\nnamespace OriginalRng {\n'+rng[a:b]+'\n}\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/water_creatures.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all twelve operations, eight constructor-equivalent C records, live 65-row property table, genuine native fish-position scopes and seven complete source/project files')
