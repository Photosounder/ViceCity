"""Audit complete trace relocation and generate immutable original/C comparison units."""
from pathlib import Path
import re,json
import bullet_traces_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/bullet-traces-c-tests'
def normalize(source):
    # Compare every complete statement without comments or formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/BulletTraces.c').read_text())
for name,value in [('BulletTraces.cpp',m.HOST),('BulletTraces.h',m.HEADER),('BulletTraces_backend.h',m.BACKEND_HEADER)]:assert (root/'src/renderer'/name).read_text()==value
assert (root/'src/core/CameraModes.h').read_text()==m.camera_modes((out/'before/src/core/Camera.h').read_text())
(out/'CameraModes.h').write_text('#pragma once\n#include "'+(root/'src/core/CameraModes.h').as_posix()+'"\n',newline='\n')
assert len(re.findall(r'^void CBulletTraces?::\w+\(',m.section(old),re.M))==6
deps=['src/core/common.h','src/core/config.h','src/math/Vector.h','src/math/Vector2D.h','src/math/Vector.cpp','src/math/maths.h','src/weapons/WeaponType.h','src/control/Replay.h','src/audio/DMAudio.h','src/audio/soundlist.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
    if path.startswith('src/math/') and not path.endswith('.cpp'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/water-cannon-c-tests/common.h').read_text();saved=out/'before/water-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text(common+'\n#ifndef SQR\n#define SQR(a) ((a)*(a))\n#endif\n',newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
owners='''#pragma once
class CEntity { public:int tag=0; };
class TraceAnchor { public:uint64_t guard=UINT64_C(0x8123456789abcdef); };
class CPhysical : public TraceAnchor,public CEntity { public:CVector m_vecMoveSpeed; };
class CPlayerPed : public CPhysical {};
class CVehicle : public CPhysical {};
CPlayerPed *FindPlayerPed(void);
CVehicle *FindPlayerVehicle(void);
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['World.h','PlayerPed.h','Vehicle.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
enum=m.camera_enum((out/'before/src/core/Camera.h').read_text())
(out/'Camera.h').write_text('#pragma once\nclass CCam { public:\n'+enum+'\nint16 Mode;};\nclass CCamera { public:CVector position,forward,up,right;CCam Cams[2];uint8 ActiveCam;CVector &GetPosition(void);CVector &GetForward(void);CVector &GetUp(void);CVector &GetRight(void);};\nextern CCamera TheCamera;\n',newline='\n')
(out/'TxdStore.h').write_text('#pragma once\nclass CTxdStore {public:static void PushCurrentTxd(void);static int FindTxdSlot(const char *);static void SetCurrentTxd(int);static void PopCurrentTxd(void);};\n',newline='\n')
(out/'custompipes.h').write_text('#pragma once\nnamespace CustomPipes {extern bool bRenderingEnvMap;extern RwCamera *EnvMapCam;}\n',newline='\n')
a=header.index('struct CBulletTrace\n');b=header.index('\nenum\n',a)
(out/'original-Traces.h').write_text('#pragma once\n'+header[a:b],newline='\n')
packet=re.search(r'\tstruct tBulletTracePacket\n\t\{.*?\n\t\};',(out/'before/src/control/Replay.h').read_text(),re.S)[0].replace('tBulletTracePacket','TestReplayPacket')
(out/'replay-packet.h').write_text('#pragma once\n'+packet+'\nstatic_assert(sizeof(TestReplayPacket)==28,"Original packet size");\n',newline='\n')
prototypes='''void TestBT_Init(void);
void TestBT_Geometry(void);
void TestBT_InitTexture(void);
void TestBT_ShutdownTexture(void);
void TestBT_Update(void);
void TestBT_MemberUpdate(int index);
void TestBT_Render(void);
void TestBT_Add(const void *start,const void *end,float thickness,uint32_t life,uint8_t visibility);
void TestBT_Weapon(const void *start,const void *end,int32_t weapon,void *shooter);
void TestBT_Alias(int kind,bool weapon);
BulletTrace TestBT_Record(int index);
void TestBT_SetRecord(int index,const BulletTrace *value);
uint16_t TestBT_Index(int index);
void *TestBT_Texture(void);
void *TestBT_Vertices(void);
void TestBT_ReadPacket(uint8_t i,void *packet);
void TestBT_ApplyPacket(const void *packet);
'''
(out/'caller.h').write_text('#pragma once\n#include "BulletTraces.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
original=commonInclude+'\n#include "original-Traces.h"\n#include "Camera.h"\n#include "owners.h"\n#include "TxdStore.h"\n#include "Timer.h"\n#include "DMAudio.h"\n#include "WeaponType.h"\n#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n#include "custompipes.h"\n#endif\nRwIm3DVertex TraceVertices[10];\n'+re.search(r'static RwImVertexIndex TraceIndexList\[48\] = \{.*?\};',old,re.S)[0]+'\nstatic RwTexture *gpSmokeTrailTexture;\n'+m.section(old)
for name,body in [('Geometry',m.geometry(old)),('InitTexture',m.resource_init(old)),('ShutdownTexture',m.resource_shutdown(old))]:original+='\nextern "C" void TestBT_'+name+'(void)\n{\n    // Execute the exact original shared-effect setup or teardown statements\n'+body+'\n}\n'
original+='''
#include "caller.h"
extern "C" BulletTrace TestBT_Record(int index)
{
    // Observe every original declared field without comparing padding
    CBulletTrace &s=CBulletTraces::aTraces[index];BulletTrace r={{s.m_vecStartPos.x,s.m_vecStartPos.y,s.m_vecStartPos.z},{s.m_vecEndPos.x,s.m_vecEndPos.y,s.m_vecEndPos.z},s.m_bInUse,s.m_nCreationTime,s.m_nLifeTime,s.m_fThickness,s.m_fVisibility};return r;
}
extern "C" void TestBT_SetRecord(int index,const BulletTrace *s)
{
    // Seed actual C++ objects component by component without layout casts
    CBulletTrace &r=CBulletTraces::aTraces[index];r.m_vecStartPos=CVector(s->m_vecStartPos.x,s->m_vecStartPos.y,s->m_vecStartPos.z);r.m_vecEndPos=CVector(s->m_vecEndPos.x,s->m_vecEndPos.y,s->m_vecEndPos.z);r.m_bInUse=s->m_bInUse;r.m_nCreationTime=s->m_nCreationTime;r.m_nLifeTime=s->m_nLifeTime;r.m_fThickness=s->m_fThickness;r.m_fVisibility=s->m_fVisibility;
}
extern "C" void TestBT_Alias(int kind,bool weapon)
{
    // Exercise real original subobject aliases including writes that change later input reads
    CBulletTrace &r=CBulletTraces::aTraces[0];CVector *start=kind&1?&r.m_vecEndPos:&r.m_vecStartPos;CVector *end=kind&2?&r.m_vecStartPos:&r.m_vecEndPos;
    if(weapon)CBulletTraces::AddTrace(start,end,WEAPONTYPE_COLT45,NULL);else CBulletTraces::AddTrace(start,end,0.4f,750,150);
}
extern "C" uint16_t TestBT_Index(int index)
{
    // Observe every original initialized triangle index
    return TraceIndexList[index];
}
extern "C" void *TestBT_Texture(void)
{
    // Observe original texture publication and destruction timing
    return gpSmokeTrailTexture;
}
extern "C" void *TestBT_Vertices(void)
{
    // Expose actual original shared vertex storage only to the observer
    return TraceVertices;
}
'''
observer='''#include "caller.h"
BulletTrace TestBT_Record(int index)
{
    // Observe the actual public C record
    return BulletTraces_aTraces[index];
}
void TestBT_SetRecord(int index,const BulletTrace *value)
{
    // Seed the identical declared values in the C pool
    BulletTraces_aTraces[index]=*value;
}
void TestBT_Alias(int kind,bool weapon)
{
    // Exercise real C subobject aliases through both typed C entry points
    BulletTrace *r=&BulletTraces_aTraces[0];BulletTraceVector *start=kind&1?&r->m_vecEndPos:&r->m_vecStartPos;BulletTraceVector *end=kind&2?&r->m_vecStartPos:&r->m_vecEndPos;
    if(weapon)BulletTraces_AddWeapon(start,end,WEAPONTYPE_COLT45,NULL);else BulletTraces_Add(start,end,0.4f,750,150);
}
uint16_t TestBT_Index(int index)
{
    // Observe all C-owned initialized indices
    return TraceIndexList[index];
}
void *TestBT_Texture(void)
{
    // Observe the actual C-owned resource handle
    return gpSmokeTrailTexture;
}
'''
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/BulletTraces.c').as_posix()+'"\n'+observer,newline='\n')
current=m.HOST.replace('#include "common.h"',commonInclude)+'\nextern "C" void *TestBT_Vertices(void)\n{\n    // Expose the actual typed backend vertex allocation for full comparison\n    return TraceVertices;\n}\n'
for variant,source in [('old',(out/'before/src/control/Replay.cpp').read_text()),('new',(root/'src/control/Replay.cpp').read_text())]:
    reads='\n'.join(line.strip() for line in source.splitlines() if re.search(r'bt->(?:inf|sup) = ',line))
    a=source.index('case REPLAYPACKET_BULLET_TRACES:',source.index('case REPLAYPACKET_BULLET_TRACES:') if 'case REPLAYPACKET_BULLET_TRACES:' in source else 0)
    tail=source[a:];writes='\n'.join(line.strip() for line in tail[:tail.index('buffer->m_nOffset')].splitlines() if 'aTraces[pb->index].' in line)
    assert len(reads.splitlines())==2 and len(writes.splitlines())==3
    wrappers='\n#include "replay-packet.h"\nextern "C" void TestBT_ReadPacket(uint8_t i,void *packet)\n{\n    // Compile the exact production replay output assignments\n    TestReplayPacket *bt=(TestReplayPacket *)packet;\n'+reads+'\n}\nextern "C" void TestBT_ApplyPacket(const void *packet)\n{\n    // Compile the exact production partial replay record writes\n    const TestReplayPacket *pb=(const TestReplayPacket *)packet;\n'+writes+'\n}\n'
    if variant=='old':original+=wrappers
    else:current+=wrappers
(out/'original.cpp').write_text(original,newline='\n');(out/'host.cpp').write_text(current,newline='\n')
oldcaller=commonInclude+'\n#include "original-Traces.h"\n#include "caller.h"\n';newcaller='#include "caller.h"\n'
for name,args,before,after in [('Init','void','CBulletTraces::Init()','BulletTraces_Init()'),('Update','void','CBulletTraces::Update()','BulletTraces_Update()'),('MemberUpdate','int index','CBulletTraces::aTraces[index].Update()','BulletTrace_Update(&BulletTraces_aTraces[index])'),('Render','void','CBulletTraces::Render()','BulletTraces_Render()'),('Add','const void *start,const void *end,float thickness,uint32_t life,uint8_t visibility','CBulletTraces::AddTrace((CVector *)start,(CVector *)end,thickness,life,visibility)','BulletTraces_AddLegacy(start,end,thickness,life,visibility)'),('Weapon','const void *start,const void *end,int32_t weapon,void *shooter','CBulletTraces::AddTrace((CVector *)start,(CVector *)end,weapon,(CEntity *)shooter)','BulletTraces_AddWeaponLegacy(start,end,weapon,shooter)')]:
    oldcaller+='extern "C" void TestBT_'+name+'('+args+')\n{\n    // Enter the complete original method with its actual parameter types\n    '+before+';\n}\n'
    newcaller+='void TestBT_'+name+'('+args+')\n{\n    // Enter actual C logic from a separate C11 caller\n    '+after+';\n}\n'
for name in ['Geometry','InitTexture','ShutdownTexture']:newcaller+='void TestBT_'+name+'(void)\n{\n    // Enter the complete C shared-effect setup or teardown operation\n    BulletTraces_'+('InitGeometry' if name=='Geometry' else name)+'();\n}\n'
(out/'original-caller.cpp').write_text(oldcaller.replace('#include "caller.h"','#include "caller.h"\n#include "owners.h"'),newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
vector=(out/'before/src/math/Vector.cpp').read_text();a=vector.index('void\nCVector::Normalise');b=vector.index('\nCVector\nMultiply3x3',a)
(out/'original-vector.cpp').write_text(commonInclude+'\n'+vector[a:b],newline='\n')
(out/'fast-absf.c').write_text((root/'build/rubbish-c-tests/fast-absf.c').read_text(),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/bullet_traces.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
print('Audited six complete methods, exact geometry/resources, replay packet assignments, shared camera constants, 36 typed operations and eight full source/project files')
