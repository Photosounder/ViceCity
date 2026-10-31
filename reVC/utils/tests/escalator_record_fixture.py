"""Compare complete original/C record algorithms with actual pool and native matrix implementations."""
from pathlib import Path
import json,re
import model_indices_migration as models
from audio_controls_migration import function
import escalator_record_migration as m
import timecycle_migration as cycle
root=Path(__file__).resolve().parents[2];out=root/'build/escalator-record-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('Escalator.h',m.HEADER),('Escalator_backend.h',m.BACKEND_HEADER),('Escalator.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
for path in ['src/core/common.h','src/core/Camera.h','src/core/Camera.cpp','src/core/Pool.h','src/core/Pools.h','src/core/Placeable.h','src/core/World.h','src/math/Vector.h','src/math/Vector.cpp','src/math/Matrix.h','src/math/Matrix.cpp','src/math/maths.h','src/objects/Object.h','src/objects/Object.cpp','src/modelinfo/ModelInfo.h','src/modelinfo/ModelIndices.h','src/audio/AudioSoundGame.h','src/core/config.h']:
    if path=='src/modelinfo/ModelIndices.h':models.verify_header((out/'before'/path).read_text(),root)
    else:cycle.verify_dependency(out/'before'/path,root/path)
normalize=lambda source:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',source))
assert normalize(m.method(original,'CEscalator'))==normalize('CEscalator::CEscalator(){m_bIsActive=false;for(int i=0;i<24;i++){m_pSteps[i]=nil;}}')
matrix=(out/'before/src/math/Matrix.cpp').read_text();vector=(out/'before/src/math/Vector.h').read_text()
assert normalize(function(matrix,'CMatrix::','CMatrix'))==normalize('CMatrix::CMatrix(void){m_attachment=nil;m_hasRwMatrix=false;}')
assert 'CVector(void) {}' in vector
common=(root/'build/escalators-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['Escalator.h','Escalator_backend.h','Escalators.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
owners='''#pragma once
#include <memory>
#include "Escalators.h"
#include "Pool.h"
typedef CPool CObjectPool;
#include "Timer.h"
#include "AudioSoundGame.h"
enum { UNKNOWN_OBJECT=0,GAME_OBJECT=1,TEMP_OBJECT=3,CONTROLLED_SUB_OBJECT=5 };
struct TestOffset { uint64_t offset; };
class CEntity { public:int32_t baseTag; };
class CObject:public TestOffset,public CEntity { public:
CMatrix matrix;CVector m_vecMoveSpeed;int8_t ObjectCreatedBy;int32_t model;
CObject(int32_t,bool);~CObject();
const CVector &GetPosition(void){return matrix.GetPosition();}
void SetPosition(const CVector &value){matrix.GetPosition()=value;}
CMatrix &GetMatrix(void){return matrix;}
void UpdateRwFrame(void);
};
class CPools { public:static CPool *GetObjectPool(void); };
class CWorld { public:static void Add(CEntity *);static void Remove(CEntity *); };
class CCamera { public:CVector position;const CVector &GetPosition(void){return position;}bool IsSphereVisible(const CVector &,float); };
extern CCamera TheCamera;
struct TestBox { CVector max; };struct CColModel { TestBox boundingBox; };
class CModelInfo { public:static CColModel *GetColModel(int); };
extern int16 MI_ESCALATORSTEP;
'''
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''Escalator TestEr_Get(int32_t index);void TestEr_Set(int32_t index,const Escalator *value);
void *TestEr_Matrix(int32_t index);
void TestEr_Add(int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool down);
void TestEr_Update(int32_t index);void TestEr_Off(int32_t index);
bool TestEr_Active(int32_t index);EscalatorVector TestEr_Position(int32_t index);
void TestEr_RunAdd(int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool down);
void TestEr_RunUpdate(int32_t index);void TestEr_RunOff(int32_t index);
int32_t AudioWorldLoopsHost_EscalatorCount(void);uint8_t AudioWorldLoopsHost_EscalatorActive(int32_t index);void AudioWorldLoopsHost_EscalatorPosition(int32_t index,AudioSoundPosition *position);
'''
(out/'caller.h').write_text('#pragma once\n#include "Escalators.h"\n#include "AudioSound.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
classes=m.record(header).replace('class CEscalator\n{','class CEscalator\n{\npublic:');(out/'original-classes.h').write_text(classes,newline='\n')
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\nstatic CEscalator OriginalEscalators[NUM_ESCALATORS];\nbool deletingEscalator;\n#define Min(a,b) ((a)<(b)?(a):(b))\n#define Max(a,b) ((a)>(b)?(a):(b))\n'+'\n\n'.join(m.method(original,name) for name in ['CEscalator','AddThisOne','Update','SwitchOff'])+'\nextern "C" {\n'
before=before.replace('#define Min(a,b) ((a)<(b)?(a):(b))\n','').replace('#define Max(a,b) ((a)>(b)?(a):(b))\n','')
after='#include "'+(root/'src/renderer/Escalator.c').as_posix()+'"\n#include "caller.h"\n'
for old in [True,False]:
    text=before if old else after;kind='CEscalator' if old else 'Escalator';array='OriginalEscalators' if old else 'Escalators_aEscalators'
    text+='Escalator TestEr_Get(int32_t index)\n{\n    // Observe all original named fields without assuming the private physical record layout\n    '+kind+' *v=&'+array+'[index];Escalator result;'
    for field in m.FIELDS:
        if field.startswith('m_pos') or field=='m_midPoint':text+=''.join('result.'+field+'.'+axis+'=v->'+field+'.'+axis+';' for axis in 'xyz')
        elif field=='m_pSteps':text+='for(int i=0;i<24;i++)result.m_pSteps[i]=v->m_pSteps[i];'
        else:text+='result.'+field+'=v->'+field+';'
    text+='return result;\n}\nvoid TestEr_Set(int32_t index,const Escalator *value)\n{\n    // Seed each declared field through its own real C or original native type\n    '+kind+' *v=&'+array+'[index];'
    for field in m.FIELDS:
        if field.startswith('m_pos') or field=='m_midPoint':text+=''.join('v->'+field+'.'+axis+'=value->'+field+'.'+axis+';' for axis in 'xyz')
        elif field=='m_pSteps':text+='for(int i=0;i<24;i++)v->m_pSteps[i]=value->m_pSteps[i];'
        else:text+='v->'+field+'=value->'+field+';'
    text+='\n}\n'
    for name,target in [('Update','Update'),('Off','SwitchOff')]:text+='void TestEr_'+name+'(int32_t index)\n{\n    // Enter the complete actual original or C record operation\n    '+(array+'[index].'+target+'()' if old else 'Escalator_'+target+'(index)')+';\n}\n'
    text+='void TestEr_Add(int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool down)\n{\n    // Preserve the original four independently captured by-value inputs\n    '+(array+'[index].AddThisOne(CVector(first.x,first.y,first.z),CVector(lower.x,lower.y,lower.z),CVector(upper.x,upper.y,upper.z),CVector(last.x,last.y,last.z),down)' if old else 'Escalator_AddThisOne(index,first,lower,upper,last,down)')+';\n}\n'
    text+='bool TestEr_Active(int32_t index)\n{\n    // Observe the same live activity field through the actual public query\n    return '+(array+'[index].IsActive()' if old else 'Escalator_IsActive(index)')+';\n}\nEscalatorVector TestEr_Position(int32_t index)\n{\n    // Observe all three actual live midpoint components through the actual query\n    '+('const CVector &v='+array+'[index].GetPosition();EscalatorVector result={v.x,v.y,v.z};return result;' if old else 'return Escalator_GetPosition(index);')+'\n}\n'
    if old:text+='void *TestEr_Matrix(int32_t index)\n{\n    // Borrow the actual original embedded native matrix without copying its ownership metadata\n    return &OriginalEscalators[index].m_matrix;\n}\n}\nconst CEscalator &Escalators_GetEscalatorLegacy(int index)\n{\n    // Retain the original audio adapter dependency for its unchanged source body\n    return OriginalEscalators[index];\n}\n'
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
host='#include "common.h"\n#include "owners.h"\n#include "caller.h"\nstatic CMatrix EscalatorMatrices[NUM_ESCALATORS];\n'+m.HOST+'''extern "C" void *TestEr_Matrix(int32_t index)
{
    // Borrow the corresponding actual native matrix in the parallel owner array
    return &EscalatorMatrices[index];
}
'''
for old in [True,False]:
    source=(out/'before/src/audio/AudioLogic.cpp').read_text() if old else (root/'src/audio/AudioLogic.cpp').read_text();bodies=[]
    for name in ['EscalatorCount','EscalatorActive','EscalatorPosition']:
        match=re.search(r'extern "C" (?:int32_t|uint8_t|void)\nAudioWorldLoopsHost_'+name+r'\([^\n]*\)\n\{',source);assert match,name
        a=source.index('{',match.start());b=a+1;depth=1
        while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
        bodies.append(source[match.start():b])
    if old:(out/'original.cpp').write_text((out/'original.cpp').read_text()+'\n'+'\n'.join(bodies),newline='\n')
    else:host+='\n'+'\n'.join(bodies)
(out/'host.cpp').write_text(host,newline='\n')
caller='#include "caller.h"\n'
for name in ['Update','Off']:caller+='void TestEr_Run'+name+'(int32_t index)\n{\n    // Enter the complete original or C operation from a separate C11 translation unit\n    TestEr_'+name+'(index);\n}\n'
caller+='void TestEr_RunAdd(int32_t index,EscalatorVector first,EscalatorVector lower,EscalatorVector upper,EscalatorVector last,bool down)\n{\n    // Exercise the actual by-value C endpoint and boolean ABI\n    TestEr_Add(index,first,lower,upper,last,down);\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
math='#include "common.h"\n'+function((out/'before/src/math/Vector.cpp').read_text(),'CVector::','Normalise')+'\n'
for name in ['CMatrix','~CMatrix','operator=','UpdateRW']:math+=function(matrix,'CMatrix::',name)+'\n'
(out/'original-math.cpp').write_text(math,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/escalator_record.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'fast-absf.c').write_text((root/'build/escalators-c-tests/fast-absf.c').read_text(),newline='\n')
print('Audited all record algorithms, static-zero constructor equivalence, native matrix/object/pool sources and actual original/current audio adapters')
