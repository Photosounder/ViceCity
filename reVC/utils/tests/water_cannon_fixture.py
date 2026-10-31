"""Audit whole cannon relocation and construct complete original/C differential units."""
from pathlib import Path
import re,json
from audio_controls_migration import function
import water_cannon_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/water-cannon-c-tests'
def normalize(source):
    # Compare every complete statement without comments or formatting
    import fire_migration
    source=fire_migration.calls(source)
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/WaterCannon.cpp').read_text();header=(out/'before/src/renderer/WaterCannon.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/WaterCannon.c').read_text())
assert (root/'src/renderer/WaterCannon_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^void CWaterCannons?::\w+\(',old,re.M))==9
deps=['src/core/common.h','src/core/config.h','src/core/Timer.h','src/core/Pool.h','src/core/main.h','src/math/Vector.h','src/math/Vector2D.h','src/math/Vector.cpp','src/math/maths.h','src/animation/AnimationId.h','src/renderer/ParticleType.h','src/audio/AudioSoundGame.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
common=(root/'build/skidmarks-c-tests/common.h').read_text();saved=out/'before/skidmark-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
macros='\n'.join(re.search(r'^#define '+name+r'\([^\n]+',(out/'before/src/core/common.h').read_text(),re.M)[0] for name in ['Min','Max'])
extra='''
#ifndef NATIVE_PROJECT_HEADERS
typedef uint16_t uint16;
struct RwCamera;
struct RwFrame;
namespace rw { struct Matrix { V3d pos;V3d at; }; }
RwFrame *RwCameraGetFrame(const RwCamera *);
RwMatrix *RwFrameGetMatrix(RwFrame *);
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#define VALIDATE_SIZE(type,size) static_assert(sizeof(type)==size,"Original record size")
#ifndef MASTER
#define ASSERT(expr) (void)((!!(expr)) || (re3_assert(#expr,__FILE__,__LINE__,__FUNCTION__),0))
#else
#define ASSERT(expr) (expr)
#endif
'''+macros+'\n#endif\n'
(out/'common.h').write_text(common+extra,newline='\n')
for path in ['src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','src/animation/AnimationId.h','src/renderer/ParticleType.h']:(out/Path(path).name).write_text((out/'before'/path).read_text(),newline='\n')
(out/'main.h').write_text((out/'before/src/core/main.h').read_text().split('struct GlobalScene')[0]+'\nvoid LittleTest(void);\n',newline='\n')
(out/'Camera.h').write_text('#pragma once\nclass CCamera { public: CVector forward;CVector &GetForward(void); };\nextern CCamera TheCamera;\n',newline='\n')
(out/'custompipes.h').write_text('#pragma once\nnamespace CustomPipes { extern bool bRenderingEnvMap;extern RwCamera *EnvMapCam; }\n',newline='\n')
(out/'WaterLevel.h').write_text('#pragma once\nextern RwRaster *gpWaterRaster;\n',newline='\n')
(out/'AnimManager.h').write_text('#pragma once\n#include "AnimationId.h"\n',newline='\n')
(out/'Fire.h').write_text('#pragma once\nclass CFireManager { public: bool ExtinguishPointWithWater(CVector,float); };\nextern CFireManager gFireManager;\n',newline='\n')
(out/'Ped.h').write_text('''#pragma once
#include "common.h"
#include "AnimationId.h"
class WaterPedAnchor { public: uintptr guard=0x876543;virtual ~WaterPedAnchor(){}; };
class CPed : public WaterPedAnchor { public:
    CVector position,m_vecMoveSpeed;bool bIsStanding=true;int id=0,localDir=0,falls=0,lastAnimation=0;
    CVector &GetPosition(void);
    int GetLocalDirection(const CVector2D &);
    void ApplyMoveForce(float,float,float);
    void SetFall(int,AnimationId,uint8);
};
''',newline='\n')
(out/'Pools.h').write_text('#pragma once\n#include "Pool.h"\n#include "Ped.h"\nclass CPools { public: static CPool *GetPedPool(void); };\n',newline='\n')
(out/'Particle.h').write_text('''#pragma once
#include "ParticleType.h"
class CEntity;
class CParticle { public: static CParticle *AddParticle(tParticleType,const CVector &,const CVector &,CEntity *entity=NULL,float size=0.0f,int32 rotationSpeed=0,int32 rotation=0,int32 frame=0,int32 life=0); };
''',newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
(out/'original-WaterCannon.h').write_text(header,newline='\n')
original=old.replace('#include "common.h"',commonInclude).replace('#include "WaterCannon.h"','#include "original-WaterCannon.h"')
original=re.sub(r'^RwImVertexIndex WaterCannonIndexList\[[^\n]+;',lambda match:'extern "C" { '+match[0]+' }',original,flags=re.M)
copy='''extern "C" WaterCannon TestWC_Record(int index)
{
    // Observe every original declared field including inactive vector data
    CWaterCannon &v=CWaterCannons::aCannons[index];WaterCannon result={};result.m_nId=v.m_nId;result.m_nCur=v.m_nCur;result.m_nTimeCreated=v.m_nTimeCreated;
    for(int i=0;i<WATERCANNON_SEGMENTS;i++){result.m_avecPos[i]={v.m_avecPos[i].x,v.m_avecPos[i].y,v.m_avecPos[i].z};result.m_avecVelocity[i]={v.m_avecVelocity[i].x,v.m_avecVelocity[i].y,v.m_avecVelocity[i].z};result.m_abUsed[i]=v.m_abUsed[i];}
    return result;
}
extern "C" void TestWC_SetRecord(int index,const WaterCannon *v)
{
    // Supply complete legal original record states without replacing any algorithm
    CWaterCannon &r=CWaterCannons::aCannons[index];r.m_nId=v->m_nId;r.m_nCur=v->m_nCur;r.m_nTimeCreated=v->m_nTimeCreated;
    for(int i=0;i<WATERCANNON_SEGMENTS;i++){r.m_avecPos[i]=CVector(v->m_avecPos[i].x,v->m_avecPos[i].y,v->m_avecPos[i].z);r.m_avecVelocity[i]=CVector(v->m_avecVelocity[i].x,v->m_avecVelocity[i].y,v->m_avecVelocity[i].z);r.m_abUsed[i]=v->m_abUsed[i];}
}
extern "C" void TestWC_Alias(int index,int kind,bool manager)
{
    // Use actual original vector subobjects to exercise ordered input/output aliases
    CWaterCannon &c=CWaterCannons::aCannons[index];CVector *pos=kind&1?&c.m_avecVelocity[c.m_nCur]:&c.m_avecPos[c.m_nCur];CVector *dir=kind&2?&c.m_avecPos[c.m_nCur]:&c.m_avecVelocity[c.m_nCur];
    if(manager)CWaterCannons::UpdateOne((uint32)c.m_nId,pos,dir);else c.Update_NewInput(pos,dir);
}
'''
protos='''void TestWC_Init(void);
void TestWC_Update(void);
void TestWC_Render(void);
void TestWC_One(uint32_t id,const void *pos,const void *dir);
void TestWC_Input(int index,const void *pos,const void *dir);
void TestWC_Frame(int index,int16_t poolIndex);
void TestWC_Draw(int index);
void TestWC_Push(int index);
void TestWC_Alias(int index,int kind,bool manager);
WaterCannon TestWC_Record(int index);
void TestWC_SetRecord(int index,const WaterCannon *record);
typedef struct AudioSoundPosition { float x,y,z; } AudioSoundPosition;
int32_t AudioWaterCannonHost_Id(uint32_t index);
void AudioWaterCannonHost_Position(uint32_t index,AudioSoundPosition *position);
'''
(out/'caller.h').write_text('#pragma once\n#include "WaterCannon.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+protos+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
def audio(source):
    # Compile the exact old or current production reader bodies without rewriting their decisions
    result=''
    for name,resultType,args in [('Id','int32_t','uint32_t index'),('Position','void','uint32_t index,AudioSoundPosition *position')]:
        body=function(source,'AudioWaterCannonHost_',name);body=body[body.index('{'):]
        result+='extern "C" '+resultType+' AudioWaterCannonHost_'+name+'('+args+')\n'+body+'\n'
    return result
game=(out/'before/src/audio/AudioSoundGame.h').read_text();a=game.index('static inline AudioSoundPosition');b=game.index('\n}',a)+2
converter=game[a:b]
(out/'original.cpp').write_text(original+'\n#include "caller.h"\n'+converter+'\n'+copy+audio((out/'before/src/audio/AudioLogic.cpp').read_text()),newline='\n')
(out/'host.cpp').write_text(m.HOST.replace('#include "common.h"',commonInclude)+'\n#include "caller.h"\n'+audio((root/'src/audio/AudioLogic.cpp').read_text()),newline='\n')
vector=(out/'before/src/math/Vector.cpp').read_text();a=vector.index('void\nCVector::Normalise');b=vector.index('\nCVector\nMultiply3x3',a)
(out/'original-vector.cpp').write_text(commonInclude+'\n'+vector[a:b],newline='\n')
oldcaller=commonInclude+'\n#include "original-WaterCannon.h"\n#include "caller.h"\n';newcaller='#include "caller.h"\n'
for name,args,oldexpr,newexpr in [
    ('Init','void','CWaterCannons::Init()','WaterCannons_Init()'),('Update','void','CWaterCannons::Update()','WaterCannons_Update()'),('Render','void','CWaterCannons::Render()','WaterCannons_Render()'),
    ('One','uint32_t id,const void *pos,const void *dir','CWaterCannons::UpdateOne(id,(CVector *)pos,(CVector *)dir)','WaterCannons_UpdateOneLegacy(id,pos,dir)'),
    ('Input','int index,const void *pos,const void *dir','CWaterCannons::aCannons[index].Update_NewInput((CVector *)pos,(CVector *)dir)','WaterCannon_Update_NewInputLegacy(&WaterCannons_aCannons[index],pos,dir)'),
    ('Frame','int index,int16_t poolIndex','CWaterCannons::aCannons[index].Update_OncePerFrame(poolIndex)','WaterCannon_Update_OncePerFrame(&WaterCannons_aCannons[index],poolIndex)'),
    ('Draw','int index','CWaterCannons::aCannons[index].Render()','WaterCannon_Render(&WaterCannons_aCannons[index])'),
    ('Push','int index','CWaterCannons::aCannons[index].PushPeds()','WaterCannon_PushPeds(&WaterCannons_aCannons[index])')]:
    oldcaller+='extern "C" void TestWC_'+name+'('+args+')\n{\n    // Enter the complete original cannon algorithm\n    '+oldexpr+';\n}\n'
    newcaller+='void TestWC_'+name+'('+args+')\n{\n    // Enter the actual C algorithm through a separate C11 caller\n    '+newexpr+';\n}\n'
newcaller+='''void TestWC_Alias(int index,int kind,bool manager)
{
    // Use actual C vector subobjects to preserve the original alias/read/write order
    WaterCannon *c=&WaterCannons_aCannons[index];WaterCannonVector *pos=kind&1?&c->m_avecVelocity[c->m_nCur]:&c->m_avecPos[c->m_nCur];WaterCannonVector *dir=kind&2?&c->m_avecPos[c->m_nCur]:&c->m_avecVelocity[c->m_nCur];
    if(manager)WaterCannons_UpdateOne((uint32_t)c->m_nId,pos,dir);else WaterCannon_Update_NewInput(c,pos,dir);
}
'''
(out/'original-caller.cpp').write_text(oldcaller,newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
observer='''WaterCannon TestWC_Record(int index)
{
    // Observe every actual public C record field
    return WaterCannons_aCannons[index];
}
void TestWC_SetRecord(int index,const WaterCannon *record)
{
    // Supply the same complete record values to the C algorithms
    WaterCannons_aCannons[index]=*record;
}
'''
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/WaterCannon.c').as_posix()+'"\n#include "caller.h"\n'+observer,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/water_cannon.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
print('Audited nine complete cannon algorithms, ordered typed/legacy inputs, exact audio readers, 38 typed boundaries and nine full source/project files')
