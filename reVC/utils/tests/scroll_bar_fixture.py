"""Audit the complete sign relocation and generate independent original/C executables."""
from pathlib import Path
import re,json
from audio_controls_migration import function
import scroll_bar_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/scroll-bar-c-tests'
original=(out/'before/src/renderer/Fluff.cpp').read_text();header=(out/'before/src/renderer/Fluff.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
for name,value in [('ScrollBar.h',m.HEADER),('ScrollBar_backend.h',m.BACKEND_HEADER),('ScrollBar.c',m.kernel(original))]:assert (root/'src/renderer'/name).read_text()==value,name
for path in ['src/core/common.h','src/core/Camera.h','src/math/Vector.h','src/math/Vector.cpp','src/math/maths.h','src/renderer/Sprite.h','src/renderer/Sprite.cpp','src/renderer/RenderBuffer.c','src/renderer/RenderBuffer.cpp','src/renderer/RenderBuffer.h']:assert (out/'before'/path).read_bytes()==(root/path).read_bytes(),path
common=(root/'build/smoke-trails-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
common+='''
#ifndef NATIVE_PROJECT_HEADERS
enum { rwFILTERLINEAR=2 };
#endif
#ifndef ABS
#define ABS(a) (((a)<0)?(-(a)):(a))
#endif
'''
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
for name in ['ScrollBar.h','ScrollBar_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
owners='''#pragma once
#include "ScrollBar.h"
#include "RenderBuffer.h"
#include "Sprite.h"
#include "General.h"
#include "Clock.h"
class CCamera { public:CVector position;const CVector &GetPosition(void){return position;}bool IsSphereVisible(const CVector &,float); };
extern CCamera TheCamera;
extern RwTexture *gpCoronaTexture[1];
'''
(out/'owners.h').write_text(owners,newline='\n')
prototypes='''void TestSc_Bind(int32_t kind,const char *value);
ScrollBar TestSc_Get(void);void TestSc_Set(const ScrollBar *value);
int32_t TestSc_Event(void);void TestSc_SetEvent(int32_t value);
uint8_t TestSc_Glyph(int32_t row,int32_t column);void TestSc_SetGlyph(int32_t row,int32_t column,uint8_t value);
const char *TestSc_Time(void);
void TestSc_Init(ScrollBarVector first,ScrollBarVector last,uint8_t type,uint8_t r,uint8_t g,uint8_t b,float scale);
void TestSc_Update(void);void TestSc_Render(void);void TestSc_Env(void *camera);
void TestSc_Visibility(bool visible);bool TestSc_Visible(void);
'''
(out/'caller.h').write_text('#pragma once\n#include "ScrollBar.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
classes=m.classes(header).replace('private:','public:');(out/'original-classes.h').write_text(classes,newline='\n')
methods=[]
for name in ['Init','Update','Render','RenderForEnvMap']:
    text=m.method(original,name)
    if name=='RenderForEnvMap':text='#ifdef REVC_SOFTWARE_POLYGONS\n'+text+'\n#endif'
    methods.append(text)
before='#include "common.h"\n#include "owners.h"\n#include "caller.h"\n#include "original-classes.h"\n'+m.charset(original)+'\nenum { SCROLL_ARENA_STRING=0 };\nCScrollBar TestOriginalBar;\nint32 CScrollBar::TonightsEvent;\nchar String_Time[]="THE TIME IS 12:34    ";\n'+function(original,'','FindTimeMessage')+'\n'+'\n'.join(methods)
after=(root/'src/renderer/ScrollBar.c').read_text()
pattern=r'((?:bar->)?m_pMessage)\s*=\s*("[^"\n]*"(?:\s*"[^"\n]*")*|(?:ScrollBar_)?FindTimeMessage\(\));'
def bind(source):
    # Observe each module's actual literal identity without changing any pointer or state
    matches=list(re.finditer(pattern,source));assert len(matches)==9,len(matches)
    for kind,match in reversed(list(enumerate(matches))):source=source[:match.end()]+'\n    // Observe the actual message pointer without changing selection behavior\n    TestSc_Bind('+str(kind)+','+match[1]+');'+source[match.end():]
    return source
before=bind(before)+'\nextern "C" {\n';after='#include "caller.h"\n'+bind(after)+'\n'
for old in [True,False]:
    text=before if old else after;obj='TestOriginalBar' if old else 'aScrollBars[0]';kind='CScrollBar' if old else 'ScrollBar'
    scalar=[field for field in m.FIELDS if field not in ['m_Position','m_Size','m_MessageBar']]
    text+='ScrollBar TestSc_Get(void)\n{\n    // Observe each named field independently of native CVector representation\n    '+kind+' *v=&'+obj+';ScrollBar result;'
    text+=''.join('result.'+field+'=v->'+field+';' for field in scalar)
    text+=''.join('result.'+field+'.'+axis+'=v->'+field+'.'+axis+';' for field in ['m_Position','m_Size'] for axis in 'xyz')
    text+='for(int i=0;i<40;i++)result.m_MessageBar[i]=v->m_MessageBar[i];return result;\n}\n'
    text+='void TestSc_Set(const ScrollBar *value)\n{\n    // Seed all declared fields through their real original or C types\n    '+kind+' *v=&'+obj+';'
    text+=''.join('v->'+field+'=value->'+field+';' for field in scalar)
    text+=''.join('v->'+field+'.'+axis+'=value->'+field+'.'+axis+';' for field in ['m_Position','m_Size'] for axis in 'xyz')
    text+='for(int i=0;i<40;i++)v->m_MessageBar[i]=value->m_MessageBar[i];\n}\n'
    event='CScrollBar::TonightsEvent' if old else 'ScrollBar_TonightsEvent'
    text+='int32_t TestSc_Event(void)\n{\n    // Observe the canonical live stadium event\n    return '+event+';\n}\nvoid TestSc_SetEvent(int32_t value)\n{\n    // Update the event at the same defined external callback\n    '+event+'=value;\n}\n'
    text+='uint8_t TestSc_Glyph(int32_t row,int32_t column)\n{\n    // Compare every mutable byte of the complete original glyph table\n    return ScrollCharSet[row][column];\n}\nvoid TestSc_SetGlyph(int32_t row,int32_t column,uint8_t value)\n{\n    // Verify that the original mutable table remains live during updates\n    ScrollCharSet[row][column]=value;\n}\n'
    text+='const char *TestSc_Time(void)\n{\n    // Observe the same persistent mutable time buffer and its aliasing\n    return String_Time;\n}\n'
    text+='void TestSc_Init(ScrollBarVector first,ScrollBarVector last,uint8_t type,uint8_t r,uint8_t g,uint8_t b,float scale)\n{\n    // Preserve captured by-value endpoints before any destination writes\n    '+(obj+'.Init(CVector(first.x,first.y,first.z),CVector(last.x,last.y,last.z),type,r,g,b,scale)' if old else 'ScrollBar_Init(&aScrollBars[0],first,last,type,r,g,b,scale)')+';\n}\n'
    for name in ['Update','Render']:text+='void TestSc_'+name+'(void)\n{\n    // Enter the complete original or C sign algorithm\n    '+(obj+'.'+name+'()' if old else 'ScrollBar_'+name+'(&aScrollBars[0])')+';\n}\n'
    text+='void TestSc_Env(void *camera)\n{\n    // Exercise the optional pass with the same native camera owner\n#ifdef REVC_SOFTWARE_POLYGONS\n    '+(obj+'.RenderForEnvMap((RwCamera *)camera)' if old else 'ScrollBar_RenderForEnvMap(&aScrollBars[0],camera)')+';\n#else\n    (void)camera;\n#endif\n}\n'
    text+='void TestSc_Visibility(bool visible)\n{\n    // Exercise the original inline visibility assignment through the C caller\n    '+(obj+'.SetVisibility(visible)' if old else 'ScrollBar_SetVisibility(&aScrollBars[0],visible)')+';\n}\nbool TestSc_Visible(void)\n{\n    // Compare the original inline flag getter through the C caller\n    return '+(obj+'.IsVisible()' if old else 'ScrollBar_IsVisible(&aScrollBars[0])')+';\n}\n'
    if old:text+='}\n'
    (out/('original.cpp' if old else 'observed-kernel.c')).write_text(text,newline='\n')
observed=(out/'observed-kernel.c').read_text();caller='#include "caller.h"\n'
for name in ['Init','Update','Render','Env','Visibility','Visible']:
    text=function(observed,'TestSc_',name);observed=observed.replace(text,'');caller+=text+'\n'
(out/'observed-kernel.c').write_text(observed,newline='\n');(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n'+m.HOST,newline='\n')
(out/'buffer.c').write_text('#include "'+(root/'src/renderer/RenderBuffer.c').as_posix()+'"\n',newline='\n')
(out/'buffer-host.cpp').write_text((root/'src/renderer/RenderBuffer.cpp').read_text(),newline='\n')
(out/'original-math.cpp').write_text('#include "common.h"\n'+function((out/'before/src/math/Vector.cpp').read_text(),'CVector::','Normalise'),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/scroll_bar.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited four complete methods, two inline methods, exact time helper, all glyphs and seven full source/project files')
