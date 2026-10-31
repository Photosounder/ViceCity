"""Audit the complete credits conversion and compare all original methods."""
from pathlib import Path
import re,json
import credits_migration as m
import text_migration as text
root=Path(__file__).resolve().parents[2];out=root/'build/credits-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Credits.cpp').read_text();header=(out/'before/src/renderer/Credits.h').read_text()
for path in ['src/core/common.h','src/core/config.h','src/core/Pad.h','src/core/Frontend.h','src/renderer/Font.h','src/renderer/Draw.c','src/text/Text.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    text.verify_dependency(saved,root/path)
common=(out/'before/src/core/common.h').read_text()
for name,value in [('Credits.c',m.kernel(old,common)),('Credits.cpp',m.calls(m.HOST)),('Credits.h',m.HEADER),('Credits_backend.h',m.BACKEND_HEADER)]:assert (root/'src/renderer'/name).read_text()==value
assert len(re.findall(r'CCredits::\w+\(',m.section(old)))==7
originalkeys=re.findall(r'TheText.Get\("([^"]+)"\)',m.section(old));assert originalkeys==re.findall(r'Credits_BackendText\("([^"]+)"\)',(root/'src/renderer/Credits.c').read_text())
portable=(root/'build/special-fx-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(portable,newline='\n')
assert saved.read_text()==portable
(out/'common.h').write_text(portable,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
frontend=(out/'before/src/core/Frontend.h').read_text();a=frontend.index('\tenum LANGUAGE\n');b=frontend.index('\n\t};',a)+4;language=frontend[a:b]
pad=(out/'before/src/core/Pad.h').read_text();cross=next(line.strip() for line in pad.splitlines() if 'bool GetCrossJustDown()' in line)
owners='''#pragma once
#include "Timer.h"
#include "FontStyles.h"
class CMenuManager { public:
'''+language+'''
bool m_bMenuActive;int32 m_PrefsLanguage;static bool m_PrefsCutsceneBorders;
};
extern CMenuManager FrontEndMenuManager;
class CCamera { public:bool m_WideScreenOn;void DrawBordersForWideScreen(void); };
extern CCamera TheCamera;
class CPad { public:struct State { int16 Cross; } NewState,OldState;
static void UpdatePads(void);static CPad *GetPad(int32);
'''+cross+'''
};
class CText { public:wchar *Get(const char *); };
extern CText TheText;
extern "C" uint16_t *Text_Get(CText *self,const char *key);
class CFont { public:
static void SetScale(float,float);static void SetColor(CRGBA);static void PrintString(float,float,wchar *);
static void SetCentreSize(float);static void SetFontStyle(int16);static void DrawFonts(void);
static void SetJustifyOff(void);static void SetBackgroundOff(void);static void SetCentreOn(void);static void SetPropOn(void);
};
void DefinedState(void);
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Font.h','Frontend.h','RwHelper.h','Camera.h','Text.h','Pad.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
(out/'original-Credits.h').write_text(header.replace('class CCredits\n{','class CCredits\n{\npublic:'),newline='\n')
prototypes='''bool TestCr_Going(void);uint32_t TestCr_StartTime(void);void TestCr_SetState(bool going,uint32_t time);
void TestCr_Init(void);void TestCr_Start(void);void TestCr_Stop(void);bool TestCr_Done(void);void TestCr_Render(void);
void TestCr_Space(float space,uint32_t *line);
void TestCr_Text(float x,float y,uint16_t *text,uint32_t *line,float scroll);
'''
(out/'caller.h').write_text('#pragma once\n#include "Credits.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
observers=''
for result,name,args,body in [('bool','Going','void','return OWNERbCreditsGoing;'),('uint32_t','StartTime','void','return OWNERCreditsStartTime;'),('void','SetState','bool going,uint32_t time','OWNERbCreditsGoing=going;OWNERCreditsStartTime=time;')]:observers+=result+' TestCr_'+name+'('+args+')\n{\n    // Observe or seed the actual original or C-owned private scalar state\n    '+body+'\n}\n'
wrappers=''
for result,name,args,invoke in [('void','Init','void','Init()'),('void','Start','void','Start()'),('void','Stop','void','Stop()'),('bool','Done','void','AreCreditsDone()'),('void','Render','void','Render()'),('void','Space','float space,uint32_t *line','PrintCreditSpace(space,REFline)'),('void','Text','float x,float y,uint16_t *text,uint32_t *line,float scroll','PrintCreditText(x,y,text,REFline,scroll)')]:wrappers+=result+' TestCr_'+name+'('+args+')\n{\n    // Enter the complete method through its actual original or C11 public API\n    '+('return ' if result!='void' else '')+'OWNER'+invoke+';\n}\n'
original='#include "common.h"\n#include "owners.h"\n#include "original-Credits.h"\n#include "caller.h"\n'+m.section(old)+'\nextern "C" {\n'+(observers+wrappers).replace('OWNER','CCredits::').replace('REFline','*line')+'}\n'
(out/'original.cpp').write_text(original,newline='\n')
(out/'observed-kernel.c').write_text('#include "'+(root/'src/renderer/Credits.c').as_posix()+'"\n#include "caller.h"\n'+observers.replace('OWNER',''),newline='\n')
(out/'caller.c').write_text('#include "caller.h"\n'+wrappers.replace('OWNER','Credits_').replace('REFline','line'),newline='\n')
(out/'host.cpp').write_text(m.calls(m.HOST),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/credits.cpp.in').read_text()+'''\nextern "C" uint16_t *Text_Get(CText *self,const char *key)
{
    // Preserve the unchanged observed text dependency across the production adapter's C boundary
    return self->Get(key);
}
''',newline='\n')
(out/'pc-scaling.h').write_text('#include "config.h"\n#define FORCE_PC_SCALING\n',newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'no-borders-switch.h').write_text('#include "config.h"\n#undef CUTSCENE_BORDERS_SWITCH\n',newline='\n')
(out/'no-aspect.h').write_text('#include "config.h"\n#undef ASPECT_RATIO_SCALE\n',newline='\n')
print('Audited seven complete methods,',len(originalkeys),'ordered text lookups and nine complete source/project files')
