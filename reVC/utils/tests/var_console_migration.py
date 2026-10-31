"""Replace the variable-console class, overloads and entry references with complete C11 operations."""
import re
import restart_migration as restarts
ALIASES={'int8':'int8_t','int16':'int16_t','int32':'int32_t','int64':'int64_t','uint8':'uint8_t','uint16':'uint16_t','uint32':'uint32_t','uint64':'uint64_t','bool8':'uint8_t','bool16':'uint16_t','bool32':'uint32_t','wchar':'uint16_t','CVarConsoleEntry':'VarConsoleEntry'}
SUFFIX={'int8':'Int8','int16':'Int16','int32':'Int32','int64':'Int64','uint8':'UInt8','uint16':'UInt16','uint32':'UInt32','uint64':'UInt64','float':'Float','bool':'Bool','bool8':'Bool8','bool16':'Bool16','bool32':'Bool32'}
FIELDS=['m_nCountEntries','m_bIsOpen','m_nCurEntry','m_nFirstEntryOnPage','m_nCurPage','m_nNumPages','m_aEntries']
PADS=['DPadDownJustDown','AnaloguePadDown','DPadUpJustDown','AnaloguePadUp','Square','Triangle','DPadLeftJustDown','AnaloguePadLeft','DPadRightJustDown','AnaloguePadRight','LeftShoulder2JustDown','RightShoulder2JustDown','RightShoulder1JustDown','LeftShoulder1JustDown']
READERS=[
('void','Color','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetColor(CRGBA(r,g,b,a));'),
('void','Style','void','CFont::SetFontStyle(FONT_STANDARD);'),
('void','Scale','float x,float y','CFont::SetScale(x,y);'),
('float','ScaleX','float value','return SCREEN_SCALE_X(value);'),
('float','ScaleY','float value','return SCREEN_SCALE_Y(value);'),
('void','ShadowPosition','int16_t position','CFont::SetDropShadowPosition(position);'),
('void','ShadowColor','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetDropColor(CRGBA(r,g,b,a));'),
('void','Prop','void','CFont::SetPropOn();'),
('void','Wrap','float x','CFont::SetWrapx(x);'),
('void','RightWrap','float x','CFont::SetRightJustifyWrap(x);'),
('void','Unicode','const char *source,uint16_t *target','AsciiToUnicode(source,target);'),
('void','RightOn','void','CFont::SetRightJustifyOn();'),
('void','RightOff','void','CFont::SetRightJustifyOff();'),
('void','Print','float x,float y,uint16_t *text','CFont::PrintString(x,y,text);'),
('void','BackgroundOn','void','CFont::SetBackgroundOn();'),
('void','BackgroundOff','void','CFont::SetBackgroundOff();'),
('void','BackgroundColor','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetBackgroundColor(CRGBA(r,g,b,a));')]+[('bool',name,'void','return CPad::GetPad(1)->Get'+name+'();') for name in PADS]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "VarConsole.h"
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' VarConsole_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "VarConsole_backend.h"
#include "Font.h"
#include "Pad.h"
'''+''.join(result+' VarConsole_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed native font or second-pad operation\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'

def aliases(source):
    # Preserve every native scalar width while removing dependencies on the C++ common header
    return re.sub(r'\b('+ '|'.join(ALIASES)+r')\b',lambda match:ALIASES[match[0]],source)

def methods(source):
    # Delimit every complete actual overload and control operation in original declaration order
    result=[]
    for match in re.finditer(r'(?m)^(void|bool8)\s*\nCVarConsole::(\w+)\(([^\n]*)\)\n\{',source):
        a=source.index('{',match.start());b=a+1;depth=1
        while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
        name=match[2]
        if name=='Add':
            value=re.search(r'char \*text, (\w+) \*pVal',match[3]);name+='Function' if not value else SUFFIX[value[1]]
        result.append((name,match[1],match[3],source[a:b]))
    assert len(result)==26 and len({name for name,ret,args,body in result})==26
    return result

def header(source,original):
    # Retain all enum encodings and every complete plain entry/controller field declaration
    assert 'CVarConsole(){Initialise();}' in re.sub(r'\s+','',source)
    assert len(re.findall(r'void Add\(',source))==14
    a=source.index('enum eVarConsoleEntryType');b=source.index('\n\nclass CVarConsole')
    entry=source[a:b].replace('struct CVarConsoleEntry\n{','typedef struct VarConsoleEntry\n{');entry=entry[:entry.rfind('};')]+'} VarConsoleEntry;\n'
    a=source.index('\n{',source.index('class CVarConsole'))+2;b=source.index('\npublic:',a)
    state='typedef struct VarConsoleState {\n'+source[a:b]+'\n} VarConsoleState;\n'
    text='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
'''+aliases(entry)+'\n'+aliases(state)+'''#ifdef __cplusplus
extern "C" {
#endif
extern VarConsoleState VarConsole;
void VarConsole_Construct(VarConsoleState *console);
'''
    for name,ret,args,body in methods(original):text+=aliases(ret+' VarConsole_'+name+'(VarConsoleState *console'+(','+args if args else '')+');\n')
    return text+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Move all original numeric operations, callback calls, formatting, input order and canonical state into C
    chunks=[];ordinary=[name for name,ret,args,body in methods(source) if not name.startswith('Add')]
    for name,ret,args,body in methods(source):
        body=body.replace('CVarConsoleEntry &entry = m_aEntries[m_nCurEntry];','VarConsoleEntry *entry = &m_aEntries[m_nCurEntry];').replace('entry.','entry->')
        for field in FIELDS:body=re.sub(r'(?<![\w>])'+field+r'\b','console->'+field,body)
        for method in ordinary:body=re.sub(r'(?<![\w])'+method+r'\(\)', 'VarConsole_'+method+'(console)',body)
        body=re.sub(r'CFont::SetColor\(CRGBA\(([^)]*)\)\)',r'VarConsole_BackendColor(\1)',body)
        body=re.sub(r'CFont::SetDropColor\(CRGBA\(([^)]*)\)\)',r'VarConsole_BackendShadowColor(\1)',body)
        body=re.sub(r'CFont::SetBackgroundColor\(CRGBA\(([^)]*)\)\)',r'VarConsole_BackendBackgroundColor(\1)',body)
        body=body.replace('CFont::SetFontStyle(FONT_STANDARD)','VarConsole_BackendStyle()')
        for native,cname in [('SetScale','Scale'),('SetDropShadowPosition','ShadowPosition'),('SetPropOn','Prop'),('SetWrapx','Wrap'),('SetRightJustifyWrap','RightWrap'),('SetRightJustifyOn','RightOn'),('SetRightJustifyOff','RightOff'),('PrintString','Print'),('SetBackgroundOn','BackgroundOn'),('SetBackgroundOff','BackgroundOff')]:body=body.replace('CFont::'+native+'(','VarConsole_Backend'+cname+'(')
        body=body.replace('AsciiToUnicode(','VarConsole_BackendUnicode(').replace('SCREEN_SCALE_X(','VarConsole_BackendScaleX(').replace('SCREEN_SCALE_Y(','VarConsole_BackendScaleY(')
        for pad in PADS:body=body.replace('CPad::GetPad(VAR_CONSOLE_PAD)->Get'+pad+'()','VarConsole_Backend'+pad+'()')
        body=body.replace('FALSE','0')
        body=body.replace('{\n','{\n    // Preserve the complete original field accesses and branch ordering\n',1)
        chunks.append(aliases(ret+'\nVarConsole_'+name+'(VarConsoleState *console'+(','+args if args else '')+')\n'+body))
    code='\n\n'.join(chunks)
    assert not re.search(r'\b(?:CVarConsole|CVarConsoleEntry|CFont|CPad|CRGBA|wchar)\b|entry\.',code)
    return '''//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "VarConsole_backend.h"
#include "Draw_constants.h"
#include <stdio.h>
#ifdef FIX_BUGS
VarConsoleState VarConsole={.m_nCurPage=1};
#else
VarConsoleState VarConsole;
#endif
void VarConsole_Construct(VarConsoleState *console)
{
    // Retain the original conditional constructor without clearing untouched fields or entry records
#ifdef FIX_BUGS
    VarConsole_Initialise(console);
#else
    (void)console;
#endif
}

'''+code+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Remove overloaded calls using the verified actual owner type at every current registration site
    if re.search(r'(?m)^CVarConsole VarConsole;',source):return restarts.calls(HOST)
    if 'class CVarConsole\n' in source:
        from pathlib import Path
        original=(Path(__file__).resolve().parents[2]/'build/var-console-c-tests/before/src/renderer/VarConsole.cpp').read_text()
        return restarts.calls(header(source,original))
    if 'VarConsole.' not in source:return restarts.calls(source)
    labels={'Time (hour of day)':'UInt8','X PLAYER COORD':'Float','Y PLAYER COORD':'Float','Z PLAYER COORD':'Float','UPDATE PLAYER COORD':'Bool8'}
    boolean=['Print nearest object','Display collision in memory','Debug PlayerPed','Tweak Vehicle Handling','Show set pieces','Freeze time','Display number of atomics rendered','Display posn and framerate','Streaming Debug','Streaming Vehicle Debug','Printf Streaming Buffer contents','Draw HUD','Occlusion debug','Count polys in shadow']
    labels.update({label:'Bool' for label in boolean});lines=[]
    for line in source.splitlines(keepends=True):
        changed=line
        if 'VarConsole.Add(' in line:
            label=re.search(r'VarConsole\.Add\("([^"]+)"',line);assert label and label[1] in labels,line
            changed=line.replace('VarConsole.Add(','VarConsole_Add'+labels[label[1]]+'(&VarConsole,')
        else:changed=re.sub(r'VarConsole\.(\w+)\(\)',r'VarConsole_\1(&VarConsole)',line)
        if changed!=line:changed=changed.rstrip('\n')+' // rouz edit (ChatGPT)\n'
        lines.append(changed)
    return restarts.calls(''.join(lines))

def project(source):
    # Register a unique C11 controller and its native boundary header without adding object-name collisions
    entry='<ClCompile Include="..\\src\\renderer\\VarConsole.cpp" />'
    if '..\\src\\renderer\\VariableConsole.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\VariableConsole.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\VarConsole.h" />'
    if '..\\src\\renderer\\VarConsole_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\VarConsole_backend.h" />')
    return restarts.project(source)
