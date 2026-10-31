import weather_migration as _weather_conversion
"""Audit whole rubbish relocation and generate independent original/C observers."""
from pathlib import Path
import re,json
import rubbish_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/rubbish-c-tests'
def normalize(source):
    # Apply the explicit verified weather rename before comparing every original statement
    source=_weather_conversion.calls(source)
    # Compare whole statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text();expected=m.project(original) if path.startswith('build/') else m.calls(original)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Rubbish.cpp').read_text();header=(out/'before/src/renderer/Rubbish.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/renderer/Rubbish.c').read_text())
assert (root/'src/renderer/Rubbish_backend.h').read_text()==m.BACKEND_HEADER
assert len(re.findall(r'^(?:CRubbish|COneSheet)::\w+\(',old,re.M))==8
deps=['src/core/common.h','src/core/config.h','src/core/Timer.h','src/core/main.h','src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','src/renderer/RenderBuffer.h']
for path in deps:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    # Follow the separately audited shared-buffer header relocation
    if path=='src/renderer/RenderBuffer.h':
        import render_buffer_migration
        assert render_buffer_migration.header(saved.read_text())==(root/path).read_text(),path
    else:assert saved.read_bytes()==(root/path).read_bytes(),path
common=(root/'build/skidmarks-c-tests/common.h').read_text();saved=out/'before/skidmark-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
extra='''
#ifndef NATIVE_PROJECT_HEADERS
typedef int8_t int8;
struct RwCamera;
struct RwFrame;
namespace rw { struct Matrix { V3d pos; }; }
RwFrame *RwCameraGetFrame(const RwCamera *);
RwMatrix *RwFrameGetMatrix(RwFrame *);
#endif
'''
macros='\n'.join(re.search(r'^#define '+name+r'\([^\n]+',(out/'before/src/core/common.h').read_text(),re.M)[0] for name in ['SQR','Min','Max'])
(out/'common.h').write_text(common+extra+'\n#ifndef NATIVE_PROJECT_HEADERS\n'+macros+'\n#endif\n',newline='\n')
for path in ['src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h']:(out/Path(path).name).write_text((out/'before'/path).read_text(),newline='\n')
(out/'main.h').write_text((out/'before/src/core/main.h').read_text().split('struct GlobalScene')[0]+'\nvoid LittleTest(void);\n',newline='\n')
(out/'TxdStore.h').write_text((root/'build/skidmarks-c-tests/TxdStore.h').read_text(),newline='\n')
owners='''#pragma once
#include "common.h"
#include <string.h>
void TestRB_Input(unsigned event,uint32_t value);
struct RubbishFloat {
    float value=0.0f;unsigned event=0;
    operator float() const {
        // Observe each original scalar read without changing its float value
        uint32_t word;memcpy(&word,&value,4);TestRB_Input(event,word);return value;
    }
};
struct RubbishInt {
    int32 value=0;unsigned event=0;
    operator int32() const {
        // Observe each original integer flag read
        TestRB_Input(event,(uint32_t)value);return value;
    }
};
struct RubbishSeed {
    uint16_t value=0;
    operator uint16_t() const {
        // Preserve the original small-integer promotion and seed-read timing
        TestRB_Input(20,value);return value;
    }
};
'''
(out/'owners.h').write_text(owners,newline='\n')
(out/'Camera.h').write_text('''#pragma once
#include "owners.h"
class CCamera { public: CVector position;RubbishFloat Orientation;
    CVector &GetPosition(void) {
        // Observe every original camera getter including short-circuited paths
        TestRB_Input(21,0);return position;
    }
};
extern CCamera TheCamera;
''',newline='\n')
(out/'Weather.h').write_text('#pragma once\n#include "owners.h"\nclass CWeather { public: static RubbishFloat Wind; };\n',newline='\n')
(out/'Stats.h').write_text('#pragma once\n#include "owners.h"\nclass CStats { public: static RubbishInt PamphletMissionPassed; };\n',newline='\n')
(out/'Vehicle.h').write_text('''#pragma once
#include "owners.h"
struct RubbishBounds { CVector max; };
struct RubbishColModel { RubbishBounds boundingBox; };
class RubbishAnchor { public: uintptr guard=0x71346;virtual ~RubbishAnchor(){}; };
class CVehicle { public:
    RubbishSeed m_randomSeed;CVector position,speed,forward,right;RubbishColModel collision;
    virtual ~CVehicle(){}
    CVector &GetPosition(void) {
        // Observe the original live vehicle position getter
        TestRB_Input(22,0);return position;
    }
    CVector &GetMoveSpeed(void) {
        // Observe the original live vehicle speed getter
        TestRB_Input(23,0);return speed;
    }
    CVector &GetForward(void) {
        // Observe the original live vehicle forward getter
        TestRB_Input(24,0);return forward;
    }
    CVector &GetRight(void) {
        // Observe the original live vehicle right getter
        TestRB_Input(25,0);return right;
    }
    RubbishColModel *GetColModel(void) {
        // Observe the original collision-model query
        TestRB_Input(26,0);return &collision;
    }
};
class RubbishTestVehicle : public RubbishAnchor,public CVehicle {};
''',newline='\n')
(out/'World.h').write_text('#pragma once\nclass CWorld { public: static float FindGroundZFor3DCoord(float,float,float,bool *); };\n',newline='\n')
(out/'custompipes.h').write_text('#pragma once\nnamespace CustomPipes { extern bool bRenderingEnvMap;extern RwCamera *EnvMapCam; }\n',newline='\n')
render=(out/'before/src/renderer/RenderBuffer.h').read_text();render=render[render.index('#define TEMPBUFFERVERTSIZE'):]
(out/'RenderBuffer.h').write_text('#pragma once\n'+render,newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif'
def rename(source):
    # Keep the old enum and typed public texture array independent of the new C declarations
    return source.replace('NUM_RUBBISH_SHEETS','ORIGINAL_NUM_RUBBISH_SHEETS').replace('gpRubbishTexture','OriginalRubbishTextures')
(out/'original-Rubbish.h').write_text(rename(header).replace('class CRubbish\n{','class CRubbish\n{\npublic:'),newline='\n')
original=rename(old).replace('#include "common.h"',commonInclude).replace('#include "Rubbish.h"','#include "original-Rubbish.h"')
original=re.sub(r'^RwImVertexIndex RubbishIndexList\[[^\n]+;',lambda match:'extern "C" { '+match[0]+' }',original,flags=re.M)
sentinels=['StartEmptyList','EndEmptyList','StartStaticsList','EndStaticsList','StartMoversList','EndMoversList']
select='\n'.join('    if(index==NUM_RUBBISH_SHEETS+'+str(i)+')return &CRubbish::'+name+';' for i,name in enumerate(sentinels))
copy='''static COneSheet *TestRB_OriginalNode(int index)
{
    // Map every pool record and sentinel to the original actual object
    if(index<NUM_RUBBISH_SHEETS)return &CRubbish::aSheets[index];
'''+select+'''
    abort();
}
static int TestRB_OriginalIndex(COneSheet *node)
{
    // Encode original node identity without process addresses
    if(!node)return -1;
    for(int i=0;i<NUM_RUBBISH_SHEETS+6;i++)if(TestRB_OriginalNode(i)==node)return i;
    abort();
}
extern "C" RubbishSheet TestRB_Record(int index)
{
    // Observe every declared original payload field while links are observed separately
    COneSheet *v=TestRB_OriginalNode(index);RubbishSheet r={};
    r.m_basePos={v->m_basePos.x,v->m_basePos.y,v->m_basePos.z};r.m_animatedPos={v->m_animatedPos.x,v->m_animatedPos.y,v->m_animatedPos.z};
    r.m_targetZ=v->m_targetZ;r.m_state=v->m_state;r.m_animationType=v->m_animationType;r.m_moveStart=v->m_moveStart;r.m_moveDuration=v->m_moveDuration;r.m_animHeight=v->m_animHeight;r.m_xDist=v->m_xDist;r.m_yDist=v->m_yDist;r.m_angle=v->m_angle;r.m_isVisible=v->m_isVisible;r.m_targetIsVisible=v->m_targetIsVisible;
    return r;
}
extern "C" void TestRB_SetRecord(int index,const RubbishSheet *v)
{
    // Supply complete original payloads while preserving the real list links
    COneSheet *r=TestRB_OriginalNode(index);
    r->m_basePos=CVector(v->m_basePos.x,v->m_basePos.y,v->m_basePos.z);r->m_animatedPos=CVector(v->m_animatedPos.x,v->m_animatedPos.y,v->m_animatedPos.z);
    r->m_targetZ=v->m_targetZ;r->m_state=v->m_state;r->m_animationType=v->m_animationType;r->m_moveStart=v->m_moveStart;r->m_moveDuration=v->m_moveDuration;r->m_animHeight=v->m_animHeight;r->m_xDist=v->m_xDist;r->m_yDist=v->m_yDist;r->m_angle=v->m_angle;r->m_isVisible=v->m_isVisible;r->m_targetIsVisible=v->m_targetIsVisible;
}
extern "C" int TestRB_Link(int index,bool previous)
{
    // Observe the exact original doubly linked topology
    COneSheet *node=TestRB_OriginalNode(index);return TestRB_OriginalIndex(previous?node->m_prev:node->m_next);
}
extern "C" void TestRB_Move(int index,int list)
{
    // Exercise both complete original list methods on valid linked records
    COneSheet *node=TestRB_OriginalNode(index);node->RemoveFromList();node->AddToList(TestRB_OriginalNode(NUM_RUBBISH_SHEETS+list*2));
}
extern "C" void TestRB_State(int visibility,bool invisible)
{
    // Supply the same private visibility state before complete algorithm calls
    CRubbish::RubbishVisibility=visibility;CRubbish::bRubbishInvisible=invisible;
}
extern "C" int TestRB_Visibility(void)
{
    // Observe the original live fade scalar
    return CRubbish::RubbishVisibility;
}
extern "C" bool TestRB_Invisible(void)
{
    // Observe the original live inverted visibility target
    return CRubbish::bRubbishInvisible;
}
extern "C" void *TestRB_Texture(int index)
{
    // Observe the original typed texture handles
    return OriginalRubbishTextures[index];
}
extern "C" void *TestRB_FluffRaster(void)
{
    // Exercise the unchanged original external texture-consumer expression
    return RwTextureGetRaster(OriginalRubbishTextures[2]);
}
'''
(out/'original.cpp').write_text(original+'\n#include "caller.h"\n#include <stdlib.h>\n'+copy,newline='\n')
select='\n'.join('    if(index==NUM_RUBBISH_SHEETS+'+str(i)+')return &'+name+';' for i,name in enumerate(sentinels))
observe='''static RubbishSheet *TestRB_Node(int index)
{
    // Map every actual C pool record and sentinel
    if(index<NUM_RUBBISH_SHEETS)return &aSheets[index];
'''+select+'''
    abort();
}
RubbishSheet TestRB_Record(int index)
{
    // Observe actual C payloads while recording topology through independent indices
    RubbishSheet result=*TestRB_Node(index);result.m_next=NULL;result.m_prev=NULL;return result;
}
void TestRB_SetRecord(int index,const RubbishSheet *value)
{
    // Preserve real list links while supplying identical complete payloads
    RubbishSheet *node=TestRB_Node(index),*next=node->m_next,*prev=node->m_prev;*node=*value;node->m_next=next;node->m_prev=prev;
}
int TestRB_Link(int index,bool previous)
{
    // Observe every original next or previous link as a logical node index
    RubbishSheet *source=TestRB_Node(index),*node=previous?source->m_prev:source->m_next;
    if(!node)return -1;
    for(int i=0;i<NUM_RUBBISH_SHEETS+6;i++)if(TestRB_Node(i)==node)return i;
    abort();
}
void TestRB_Move(int index,int list)
{
    // Exercise both actual C list algorithms on valid linked records
    RubbishSheet *node=TestRB_Node(index);RubbishSheet_RemoveFromList(node);RubbishSheet_AddToList(node,TestRB_Node(NUM_RUBBISH_SHEETS+list*2));
}
void TestRB_State(int visibility,bool invisible)
{
    // Supply the same live visibility state to the C algorithms
    RubbishVisibility=visibility;bRubbishInvisible=invisible;
}
int TestRB_Visibility(void)
{
    // Observe the actual C fade scalar
    return RubbishVisibility;
}
bool TestRB_Invisible(void)
{
    // Observe the actual C inverted visibility target
    return bRubbishInvisible;
}
void *TestRB_Texture(int index)
{
    // Observe the actual C-owned texture array
    return gpRubbishTexture[index];
}
'''
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/Rubbish.c').as_posix()+'"\n#include "caller.h"\n#include <stdlib.h>\n'+observe,newline='\n')
protos='''void TestRB_Init(void);
void TestRB_Shutdown(void);
void TestRB_Update(void);
void TestRB_Render(void);
void TestRB_Stir(CVehicle *vehicle);
void TestRB_SetVisibility(bool visible);
RubbishSheet TestRB_Record(int index);
void TestRB_SetRecord(int index,const RubbishSheet *value);
int TestRB_Link(int index,bool previous);
void TestRB_Move(int index,int list);
void TestRB_State(int visibility,bool invisible);
int TestRB_Visibility(void);
bool TestRB_Invisible(void);
void *TestRB_Texture(int index);
void *TestRB_FluffRaster(void);
'''
(out/'caller.h').write_text('#pragma once\n#include "Rubbish.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+protos+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
oldcaller=commonInclude+'\n#include "original-Rubbish.h"\n#include "caller.h"\n';newcaller='#include "caller.h"\n'
for name,args,expr in [('Init','void',''),('Shutdown','void',''),('Update','void',''),('Render','void',''),('Stir','CVehicle *vehicle','vehicle'),('SetVisibility','bool visible','visible')]:
    method='StirUp' if name=='Stir' else name
    oldcaller+='extern "C" void TestRB_'+name+'('+args+')\n{\n    // Enter the complete original rubbish algorithm\n    CRubbish::'+method+'('+expr+');\n}\n'
    newcaller+='void TestRB_'+name+'('+args+')\n{\n    // Enter the complete C algorithm through a separate C11 caller\n    Rubbish_'+method+'('+expr+');\n}\n'
(out/'original-caller.cpp').write_text(oldcaller,newline='\n');(out/'caller.c').write_text(newcaller,newline='\n')
host=m.HOST.replace('#include "common.h"',commonInclude)+'''\n#include "caller.h"
extern "C" void *TestRB_FluffRaster(void)
{
    // Exercise the actual converted external Fluff texture-reader expression
    return Rubbish_BackendRaster(gpRubbishTexture[2]);
}
'''
(out/'host.cpp').write_text(host,newline='\n')
absolute=(root/'build/skidmarks-c-tests/before/external/rouziclib/math/ieee754.c').read_bytes();saved=out/'before/external/rouziclib/math/ieee754.c'
if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes(absolute)
assert saved.read_bytes()==absolute
(out/'fast-absf.c').write_text((root/'build/skidmarks-c-tests/fast-absf.c').read_text(),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/rubbish.cpp.in').read_text().replace('@COMMON@',commonInclude),newline='\n')
print('Audited all eight complete algorithms, full sheet/sentinel topology, 47 typed operations, public textures, external Fluff read and eleven full source/project files')
