"""Audit complete streak relocation and generate original/C comparison inputs."""
from pathlib import Path
import re,json
import motion_blur_streaks_migration as m
import timecycle_migration as cycle
import timecycle_test_dependencies as cycle_dependencies
root=Path(__file__).resolve().parents[2];out=root/'build/motion-blur-streaks-c-tests'
def normalize(source):
    # Compare every complete statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    source=(out/'before'/path).read_text();expected=m.project(source) if path.startswith('build/') else m.calls(source)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/MotionBlurStreaks.c').read_text())
assert (root/'src/renderer/MotionBlurStreaks.cpp').read_text()==m.calls(m.HOST)
assert (root/'src/renderer/MotionBlurStreaks.h').read_text()==m.HEADER
assert (root/'src/renderer/MotionBlurStreaks_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^(?:CRegisteredMotionBlurStreak|CMotionBlurStreaks)::\w+\(',m.section(old),re.M))==7
deps=['src/core/config.h','src/core/common.h','src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','src/fakerw/rwplcore.h','src/renderer/Timecycle.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    cycle.verify_dependency(saved,root/path)
    if path.startswith('src/math/'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/water-cannon-c-tests/common.h').read_text();saved=out/'before/water-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
packing=re.search(r'#define RWRGBALONG[^\n]+\n[^\n]+',(out/'before/src/fakerw/rwplcore.h').read_text())[0]
(out/'common.h').write_text(common+'\n#ifndef NATIVE_PROJECT_HEADERS\n'+packing+'\n#endif\n',newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
(out/'main.h').write_text('#pragma once\nvoid LittleTest(void);\n',newline='\n')
(out/'Timecycle.h').write_text('#pragma once\nclass CTimeCycle { public:static int32 GetFogRed(void);static int32 GetFogGreen(void);static int32 GetFogBlue(void);};\n'+cycle_dependencies.declarations(m.HOST),newline='\n')
a=header.index('class CRegisteredMotionBlurStreak\n');b=header.index('struct CBulletTrace',a)
originalHeader=header[a:b].replace('class CMotionBlurStreaks\n{','class CMotionBlurStreaks\n{\npublic:')
(out/'original-Streaks.h').write_text('#pragma once\n'+originalHeader,newline='\n')
prototypes='''void TestMBS_InitGeometry(void);
void TestMBS_Init(void);
void TestMBS_Update(void);
void TestMBS_Render(void);
void TestMBS_MemberUpdate(int index);
void TestMBS_MemberRender(int index);
void TestMBS_LocalRender(const MotionBlurStreak *value);
void TestMBS_RegisterC(uintptr_t id,uint8_t red,uint8_t green,uint8_t blue,MotionBlurStreakVector p1,MotionBlurStreakVector p2);
void TestMBS_RegisterLegacy(uintptr_t id,uint8_t red,uint8_t green,uint8_t blue,const void *p1,const void *p2);
void TestMBS_Alias(int index);
bool TestMBS_IsRegistered(uintptr_t id);
MotionBlurStreak TestMBS_Record(int index);
void TestMBS_SetRecord(int index,const MotionBlurStreak *value);
uint16_t TestMBS_Index(int index);
void *TestMBS_Vertices(void);
'''
(out/'caller.h').write_text('#pragma once\n#include "MotionBlurStreaks.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
observer='''extern "C" MotionBlurStreak TestMBS_Record(int index)
{
    // Observe every original field without comparing padding or changing an algorithm
    CRegisteredMotionBlurStreak &s=CMotionBlurStreaks::aStreaks[index];MotionBlurStreak r={};r.m_id=s.m_id;r.m_red=s.m_red;r.m_green=s.m_green;r.m_blue=s.m_blue;
    for(int i=0;i<3;i++){
        // Copy actual original vector components and validity flags independently
        r.m_pos1[i]={s.m_pos1[i].x,s.m_pos1[i].y,s.m_pos1[i].z};r.m_pos2[i]={s.m_pos2[i].x,s.m_pos2[i].y,s.m_pos2[i].z};r.m_isValid[i]=s.m_isValid[i];
    }
    return r;
}
extern "C" void TestMBS_SetRecord(int index,const MotionBlurStreak *s)
{
    // Supply legal original record fields without aliasing a C record as a C++ object
    CRegisteredMotionBlurStreak &r=CMotionBlurStreaks::aStreaks[index];r.m_id=s->m_id;r.m_red=s->m_red;r.m_green=s->m_green;r.m_blue=s->m_blue;
    for(int i=0;i<3;i++){
        // Supply all original vector fields including inactive samples
        r.m_pos1[i]=CVector(s->m_pos1[i].x,s->m_pos1[i].y,s->m_pos1[i].z);r.m_pos2[i]=CVector(s->m_pos2[i].x,s->m_pos2[i].y,s->m_pos2[i].z);r.m_isValid[i]=s->m_isValid[i];
    }
}
extern "C" uint16_t TestMBS_Index(int index)
{
    // Observe the complete original shared index table
    return StreakIndexList[index];
}
extern "C" void *TestMBS_Vertices(void)
{
    // Return the actual original typed vertex allocation to the common observer
    return StreakVertices;
}
extern "C" void TestMBS_Alias(int index)
{
    // Pass actual original vector subobjects through the original by-value API
    CRegisteredMotionBlurStreak &s=CMotionBlurStreaks::aStreaks[index];CMotionBlurStreaks::RegisterStreak(s.m_id,s.m_red,s.m_green,s.m_blue,s.m_pos1[1],s.m_pos2[1]);
}
extern "C" void TestMBS_LocalRender(const MotionBlurStreak *s)
{
    // Render an independent legal local record as the reflection caller does
    CRegisteredMotionBlurStreak r;r.m_id=s->m_id;r.m_red=s->m_red;r.m_green=s->m_green;r.m_blue=s->m_blue;
    for(int i=0;i<3;i++){
        // Preserve each local vector and validity field without a layout cast
        r.m_pos1[i]=CVector(s->m_pos1[i].x,s->m_pos1[i].y,s->m_pos1[i].z);r.m_pos2[i]=CVector(s->m_pos2[i].x,s->m_pos2[i].y,s->m_pos2[i].z);r.m_isValid[i]=s->m_isValid[i];
    }
    r.Render();
}
'''
original=commonInclude+'\n#include "original-Streaks.h"\n#include "Timecycle.h"\n#include "main.h"\nRwIm3DVertex StreakVertices[4];\nRwImVertexIndex StreakIndexList[12];\n'+m.section(old)
original+='\n#include "caller.h"\nextern "C" void TestMBS_InitGeometry(void)\n{\n    // Execute the exact original geometry setup from CSpecialFX::Init\n'+m.geometry(old)+'}\n'+observer
(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text(m.calls(m.HOST).replace('#include "common.h"',commonInclude)+'\nextern "C" void *TestMBS_Vertices(void)\n{\n    // Expose the actual current typed backend allocation for complete observation\n    return StreakVertices;\n}\n',newline='\n')
newObserver='''#include "caller.h"
MotionBlurStreak TestMBS_Record(int index)
{
    // Observe the actual complete C record
    return aStreaks[index];
}
void TestMBS_SetRecord(int index,const MotionBlurStreak *value)
{
    // Seed the identical declared values into the C pool
    aStreaks[index]=*value;
}
uint16_t TestMBS_Index(int index)
{
    // Observe every C-owned index
    return StreakIndexList[index];
}
void TestMBS_Alias(int index)
{
    // Pass actual C vector subobjects through the same by-value API
    MotionBlurStreak *s=&aStreaks[index];MotionBlurStreaks_RegisterStreak(s->m_id,s->m_red,s->m_green,s->m_blue,s->m_pos1[1],s->m_pos2[1]);
}
void TestMBS_MemberUpdate(int index)
{
    // Enter the actual C member algorithm for this pool record
    MotionBlurStreak_Update(&aStreaks[index]);
}
void TestMBS_MemberRender(int index)
{
    // Enter the actual C draw algorithm for this pool record
    MotionBlurStreak_Render(&aStreaks[index]);
}
void TestMBS_LocalRender(const MotionBlurStreak *value)
{
    // Render the same independent local reflection record
    MotionBlurStreak local=*value;MotionBlurStreak_Render(&local);
}
'''
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/MotionBlurStreaks.c').as_posix()+'"\n'+newObserver,newline='\n')
oldcaller=commonInclude+'\n#include "original-Streaks.h"\n#include "caller.h"\n';newcaller='#include "caller.h"\n'
for name,result,args,before,after in [
('Init','void','void','CMotionBlurStreaks::Init()','MotionBlurStreaks_Init()'),('Update','void','void','CMotionBlurStreaks::Update()','MotionBlurStreaks_Update()'),('Render','void','void','CMotionBlurStreaks::Render()','MotionBlurStreaks_Render()'),('IsRegistered','bool','uintptr_t id','CMotionBlurStreaks::IsRegisteredForCurrentFrame(id)','MotionBlurStreaks_IsRegisteredForCurrentFrame(id)')]:
    statement='return ' if result!='void' else ''
    oldcaller+='extern "C" '+result+' TestMBS_'+name+'('+args+')\n{\n    // Enter the complete original manager algorithm\n    '+statement+before+';\n}\n'
    newcaller+=result+' TestMBS_'+name+'('+args+')\n{\n    // Enter the actual C manager through a separate C11 caller\n    '+statement+after+';\n}\n'
for name in ['Update','Render']:oldcaller+='extern "C" void TestMBS_Member'+name+'(int index)\n{\n    // Enter the complete original member algorithm\n    CMotionBlurStreaks::aStreaks[index].'+name+'();\n}\n'
oldcaller+='''extern "C" void TestMBS_RegisterC(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,MotionBlurStreakVector p1,MotionBlurStreakVector p2)
{
    // Convert observation values into actual original by-value arguments
    CMotionBlurStreaks::RegisterStreak(id,r,g,b,CVector(p1.x,p1.y,p1.z),CVector(p2.x,p2.y,p2.z));
}
extern "C" void TestMBS_RegisterLegacy(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,const void *p1,const void *p2)
{
    // Pass actual typed legacy values through the original method
    CMotionBlurStreaks::RegisterStreak(id,r,g,b,*(const CVector *)p1,*(const CVector *)p2);
}
'''
newcaller+='''void TestMBS_InitGeometry(void)
{
    // Enter the actual C geometry initializer at its separate original call point
    MotionBlurStreaks_InitGeometry();
}
void TestMBS_RegisterC(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,MotionBlurStreakVector p1,MotionBlurStreakVector p2)
{
    // Exercise the actual C API with plain vector values
    MotionBlurStreaks_RegisterStreak(id,r,g,b,p1,p2);
}
'''
(out/'original-caller.cpp').write_text(oldcaller,newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
(out/'legacy-caller.cpp').write_text(commonInclude+'''\n#include "caller.h"
extern "C" void TestMBS_RegisterLegacy(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,const void *p1,const void *p2)
{
    // Exercise the actual production typed input boundary
    MotionBlurStreaks_RegisterStreakLegacy(id,r,g,b,*(const CVector *)p1,*(const CVector *)p2);
}
''',newline='\n')
(out/'host.cpp').write_text((out/'host.cpp').read_text()+cycle_dependencies.definitions(m.HOST),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/motion_blur_streaks.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
print('Audited all seven complete methods, exact geometry setup, eighteen typed operations and seven whole source/project files')
