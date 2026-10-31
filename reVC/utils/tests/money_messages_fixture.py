"""Audit complete money-message relocation and compare actual original methods with strict C11."""
from pathlib import Path
import re,json
import money_messages_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/money-messages-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text()
assert (root/'src/renderer/MoneyMessages.c').read_text()==m.kernel(old,(root/'src/core/common.h').read_text())
for name,value in [('MoneyMessages.h',m.HEADER),('MoneyMessages_backend.h',m.BACKEND_HEADER),('MoneyMessages.cpp',m.HOST)]:assert (root/'src/renderer'/name).read_text()==value
assert len(re.findall(r'\nCMoneyMessages?::\w+\(',m.section(old)))==4
for path in ['src/core/common.h','src/core/config.h','src/math/Vector.h','src/math/Vector2D.h','src/math/Matrix.h','src/math/Rect.h','src/math/maths.h','src/renderer/Font.cpp','src/renderer/Draw.c']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
    if path.startswith('src/math/'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/special-fx-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
(out/'common.h').write_text(common,newline='\n')
(out/'original-MoneyMessages.h').write_text('#pragma once\n'+m.record_header(header),newline='\n')
owners='''#pragma once
#include "MoneyMessages.h"
#include "Timer.h"
#include "FontStyles.h"
class CSprite { public:static bool CalcScreenCoors(const RwV3d &,RwV3d *,float *,float *,bool); };
class CFont { public:
    static void SetPropOn(void);
    static void SetBackgroundOff(void);
    static void SetCentreOn(void);
    static void SetJustifyOff(void);
    static void SetBackGroundOnlyTextOff(void);
    static void SetScale(float,float);
    static void SetCentreSize(float);
    static void SetColor(CRGBA);
    static void PrintString(float,float,wchar *);
    static bool IsJapanese(void);
};
#ifdef MORE_LANGUAGES
#define FONT_LOCALE(style) (CFont::IsJapanese()?FONT_JAPANESE:(style))
#else
#define FONT_LOCALE(style) (style)
#endif
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Font.h','Sprite.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
prototypes='''MoneyMessage TestMM_Record(int index);
void *TestMM_Text(int index);
void TestMM_SetRecord(int index,const MoneyMessage *value);
void TestMM_Init(void);
void TestMM_Render(void);
void TestMM_Member(int index);
void TestMM_RegisterC(MoneyMessageVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity);
void TestMM_RegisterLegacy(const void *position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity);
void TestMM_Alias(int index,const char *text,bool legacy);
'''
(out/'caller.h').write_text('#pragma once\n#include "MoneyMessages.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
prefix='#include "common.h"\n#include "owners.h"\n'
oldobserver='''#include "caller.h"
extern "C" void *TestMM_Text(int index)
{
    // Expose only the original legal text subobject identity for owner checks
    return CMoneyMessages::aMoneyMessages[index].m_aText;
}
extern "C" MoneyMessage TestMM_Record(int index)
{
    // Observe all declared fields without copying C++ layout or padding
    CMoneyMessage &v=CMoneyMessages::aMoneyMessages[index];
    MoneyMessage result={v.m_nTimeRegistered,{v.m_vecPosition.x,v.m_vecPosition.y,v.m_vecPosition.z},{0},{v.m_Colour.r,v.m_Colour.g,v.m_Colour.b,v.m_Colour.a},v.m_fSize,v.m_fOpacity};memcpy(result.m_aText,v.m_aText,sizeof(result.m_aText));return result;
}
extern "C" void TestMM_SetRecord(int index,const MoneyMessage *value)
{
    // Seed every actual original field including the normally retained alpha and text tail
    CMoneyMessage &v=CMoneyMessages::aMoneyMessages[index];v.m_nTimeRegistered=value->m_nTimeRegistered;v.m_vecPosition=CVector(value->m_vecPosition.x,value->m_vecPosition.y,value->m_vecPosition.z);memcpy(v.m_aText,value->m_aText,sizeof(v.m_aText));v.m_Colour=CRGBA(value->m_Colour.r,value->m_Colour.g,value->m_Colour.b,value->m_Colour.a);v.m_fSize=value->m_fSize;v.m_fOpacity=value->m_fOpacity;
}
extern "C" void TestMM_Init(void)
{
    // Enter the complete original manager initializer
    CMoneyMessages::Init();
}
extern "C" void TestMM_Render(void)
{
    // Enter the complete original manager renderer
    CMoneyMessages::Render();
}
extern "C" void TestMM_Member(int index)
{
    // Enter the complete original record method including an inactive timestamp
    CMoneyMessages::aMoneyMessages[index].Render();
}
extern "C" void TestMM_RegisterC(MoneyMessageVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity)
{
    // Supply equivalent copied values to the complete original registration
    CMoneyMessages::RegisterOne(CVector(position.x,position.y,position.z),text,r,g,b,size,opacity);
}
extern "C" void TestMM_RegisterLegacy(const void *position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity)
{
    // Retain the actual original by-value CVector boundary
    CMoneyMessages::RegisterOne(*(const CVector *)position,text,r,g,b,size,opacity);
}
extern "C" void TestMM_Alias(int index,const char *text,bool)
{
    // Capture a real original pool subobject before converter callbacks can overwrite it
    CMoneyMessages::RegisterOne(CMoneyMessages::aMoneyMessages[index].m_vecPosition,text,5,17,239,.8f,.5f);
}
'''
(out/'original.cpp').write_text(prefix+'#include "original-MoneyMessages.h"\n'+m.section(old)+'\n'+oldobserver,newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
observer='''#include "caller.h"
void *TestMM_Text(int index)
{
    // Expose the corresponding legal C text subobject identity
    return aMoneyMessages[index].m_aText;
}
_Static_assert(sizeof(MoneyMessage)==60 && _Alignof(MoneyMessage)==4,"Original record size and alignment");
_Static_assert(offsetof(MoneyMessage,m_nTimeRegistered)==0 && offsetof(MoneyMessage,m_vecPosition)==4 && offsetof(MoneyMessage,m_aText)==16 && offsetof(MoneyMessage,m_Colour)==48 && offsetof(MoneyMessage,m_fSize)==52 && offsetof(MoneyMessage,m_fOpacity)==56,"Original field offsets");
MoneyMessage TestMM_Record(int index)
{
    // Observe the complete C-owned record through legal same-unit access
    return aMoneyMessages[index];
}
void TestMM_SetRecord(int index,const MoneyMessage *value)
{
    // Seed all C fields without reaching through a guessed C++ representation
    aMoneyMessages[index]=*value;
}
void TestMM_Alias(int index,const char *text,bool legacy)
{
    // Capture the same original pool position before entering either public input boundary
    MoneyMessageVector position=aMoneyMessages[index].m_vecPosition;
    if(legacy){
        // Use the test adapter for the real typed legacy entry
        TestMM_RegisterAliasLegacy(position,text);
    }else{
        // Exercise the native C record input directly
        MoneyMessages_RegisterOne(position,text,5,17,239,.8f,.5f);
    }
}
'''
(out/'observed-kernel.c').write_text('#include <stddef.h>\n#include "'+(root/'src/renderer/MoneyMessages.c').as_posix()+'"\nvoid TestMM_RegisterAliasLegacy(MoneyMessageVector position,const char *text);\n'+observer,newline='\n')
caller='''#include "caller.h"
void TestMM_Init(void)
{
    // Enter the actual C initializer from a separate C unit
    MoneyMessages_Init();
}
void TestMM_Render(void)
{
    // Enter the actual C manager renderer from a separate C unit
    MoneyMessages_Render();
}
void TestMM_Member(int index)
{
    // Enter the actual C record renderer for an observed legal pool record
    extern void TestMM_MemberInput(int index);TestMM_MemberInput(index);
}
void TestMM_RegisterC(MoneyMessageVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity)
{
    // Exercise actual native C by-value position inputs
    MoneyMessages_RegisterOne(position,text,r,g,b,size,opacity);
}
'''
(out/'caller.c').write_text(caller,newline='\n')
with (out/'observed-kernel.c').open('a',newline='\n') as f:f.write('void TestMM_MemberInput(int index)\n{\n    // Observe the original member entry on the actual C pool\n    MoneyMessage_Render(&aMoneyMessages[index]);\n}\n')
(out/'legacy-caller.cpp').write_text(prefix+'''#include "caller.h"
extern "C" void TestMM_RegisterLegacy(const void *position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity)
{
    // Enter the actual by-value typed compatibility boundary
    MoneyMessages_RegisterOneLegacy(*(const CVector *)position,text,r,g,b,size,opacity);
}
extern "C" void TestMM_RegisterAliasLegacy(MoneyMessageVector position,const char *text)
{
    // Construct a legal typed value for the same captured pool position
    MoneyMessages_RegisterOneLegacy(CVector(position.x,position.y,position.z),text,5,17,239,.8f,.5f);
}
''',newline='\n')
scale=m.function((out/'before/src/renderer/Draw.c').read_text(),'Draw_','ScaleY')
convert=m.function((out/'before/src/renderer/Font.cpp').read_text(),'','AsciiToUnicode')
(out/'test.cpp').write_text((root/'utils/tests/money_messages.cpp.in').read_text().replace('@SCALE@',scale).replace('@CONVERT@',convert[convert.index('{')+1:convert.rindex('}')]),newline='\n')
(out/'pc-scaling.h').write_text('#include "config.h"\n#define FORCE_PC_SCALING\n',newline='\n')
(out/'no-languages.h').write_text('#include "config.h"\n#undef MORE_LANGUAGES\n',newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited four complete algorithms, both removed owners, exact typed backend and eight complete source/project files; generated original/C comparison')
