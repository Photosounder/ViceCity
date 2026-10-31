"""Replace the debug-console class, record, singleton and variadic path with C11."""
import re
import cloud_effects_migration as clouds
FIELDS=['m_nLineCount','m_nCurrentLine','Buffers','m_aTimer','m_aRed','m_aGreen','m_aBlue']
TYPE='''enum { CONSOLE_MAX_LINES=8,CONSOLE_MAX_STR_LEN=40 };
typedef struct Console {
    uint8_t m_nLineCount;
    uint8_t m_nCurrentLine;
    uint16_t Buffers[CONSOLE_MAX_LINES][CONSOLE_MAX_STR_LEN];
    uint32_t m_aTimer[CONSOLE_MAX_LINES];
    uint8_t m_aRed[CONSOLE_MAX_LINES];
    uint8_t m_aGreen[CONSOLE_MAX_LINES];
    uint8_t m_aBlue[CONSOLE_MAX_LINES];
} Console;
'''
HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n'+TYPE+'''#ifdef __cplusplus
extern "C" {
#endif
extern Console TheConsole;
void Console_Init(Console *console);
void Console_AddLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b);
void Console_AddOneLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b);
void Console_Display(Console *console);
void cprintf(char *format,...);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('int32_t','ScreenWidth','void','return RsGlobal.width;','Read the original live integer screen width'),
('void','FontScale','float x,float y','CFont::SetScale(x,y);','Retain the original fixed font scale'),
('void','FontColor','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetColor(CRGBA(r,g,b,a));','Construct the original typed font color from live C record bytes'),
('void','PrintString','float x,float y,uint16_t *text','CFont::PrintString(x,y,text);','Borrow the actual mutable C Unicode row through the original print overload'),
('void','FontStyle','int16_t style','CFont::SetFontStyle(style);','Select the original font style'),
('void','FontRightJustifyWrap','float wrap','CFont::SetRightJustifyWrap(wrap);','Retain the original right-justify wrap write'),
('void','FontWrapx','float wrap','CFont::SetWrapx(wrap);','Retain the original live screen-width wrap value')]
for name in ['PropOn','BackgroundOff','CentreOff','RightJustifyOff','JustifyOn','BackGroundOnlyTextOff','PropOff']:READERS.append(('void','Font'+name,'void','CFont::Set'+name+'();','Retain the original font '+name+' operation'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Console.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' Console_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "Console_backend.h"\n#include "Font.h"\n'+''.join(result+' Console_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'//- rouz edit (ChatGPT)\n'

def section(source):
    # Retain all three original out-of-line methods and the complete variadic function
    a=source.index('#define CONSOLE_X_POS');b=source.index('\n}',source.index('cprintf(char* format',a))+2;return source[a:b]+'\n'

def kernel(source,header):
    # Translate each original statement and address the actual C record directly
    body=section(source).replace('CConsole TheConsole;','Console TheConsole;')
    body=body.replace('CConsole::AddLine(char *s, uint8 r, uint8 g, uint8 b)','Console_AddLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b)').replace('CConsole::AddOneLine(char *s, uint8 r, uint8 g, uint8 b)','Console_AddOneLine(Console *console,char *s,uint8_t r,uint8_t g,uint8_t b)').replace('CConsole::Display()','Console_Display(Console *console)')
    body=body.replace('TheConsole.AddLine(','Console_AddLine(&TheConsole,')
    body=re.sub(r'(?<![\w.])AddOneLine\(',r'Console_AddOneLine(console,',body)
    for name in FIELDS:body=re.sub(r'\b'+name+r'\b','console->'+name,body)
    for name in ['LINES','STR_LEN']:body=re.sub(r'\bMAX_'+name+r'\b','CONSOLE_MAX_'+name,body)
    body=re.sub(r'CFont::SetColor\(CRGBA\(([^)]+)\)\)',r'Console_BackendFontColor(\1)',body)
    body=body.replace('CFont::PrintString(','Console_BackendPrintString(').replace('RsGlobal.width','Console_BackendScreenWidth()')
    for name in ['Scale','FontStyle','RightJustifyWrap','Wrapx','PropOn','BackgroundOff','CentreOff','RightJustifyOff','JustifyOn','BackGroundOnlyTextOff','PropOff']:body=body.replace('CFont::Set'+name+'(','Console_Backend'+('FontStyle' if name=='FontStyle' else 'Font'+name)+'(')
    for old,new in [('uint8','uint8_t'),('int32','int32_t'),('int16','int16_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    comments={'AddLine':'Split the live input with the original forty-byte copy and thirty-nine-byte advance','AddOneLine':'Copy each input byte directly into the live Unicode row before publishing metadata','Display':'Set original font state before expiring and drawing the live ring contents'}
    for name,comment in comments.items():body=re.sub(r'(Console_'+name+r'\([^\n]*\)\n\{\n)',lambda match:match[0]+'    // '+comment+'\n',body)
    body=body.replace('cprintf(char* format, ...)\n{\n','cprintf(char* format, ...)\n{\n    // Retain the original variadic formatting and singleton insertion sequence\n')
    body=re.sub(r'(^\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original loop reads, writes and iteration order\n',body,flags=re.M)
    init=re.search(r'void Init\(\) \{ ([^}]+) \}',header)[1]
    for name in FIELDS:init=re.sub(r'\b'+name+r'\b','console->'+name,init)
    body+='\nvoid Console_Init(Console *console)\n{\n    // Reset only the two original counters and retain all row, timer and color bytes\n    '+init+'\n}\n'
    assert not re.search(r'\b(?:CConsole|CFont|uint8|int32|int16|wchar|MAX_LINES|MAX_STR_LEN)\b',body)
    return '//+ rouz edit (ChatGPT)\n#include "Console_backend.h"\n#include "Timer.h"\n#include "FontStyles.h"\n#include <stdarg.h>\n#include <stdio.h>\n#include <string.h>\n\n'+body+'//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace the whole former owner and its complete instance call points
    if 'CConsole TheConsole;' in source and 'CConsole::Display()' in source:return HOST
    if 'class CConsole\n' in source:return HEADER
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'TheConsole\.(?:Init|Display)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the complete C console implementation with the original singleton\n')
        lines.append(re.sub(r'TheConsole\.(Init|Display)\(\)',r'Console_\1(&TheConsole)',line))
    return clouds.calls(''.join(lines))

def project(source):
    # Add the C11 record and algorithms beside their typed font boundary
    if '..\\src\\renderer\\Console.c"' not in source:
        match=re.search(r'<ClCompile Include="\.\.\\src\\renderer\\Console\.cpp"\s*(?:/>|>[\s\S]*?</ClCompile>)',source)
        if match:
            entry=match[0];addition=entry.replace('Console.cpp','Console.c') if '<Filter>' in entry else '<ClCompile Include="..\\src\\renderer\\Console.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
            source=source.replace(entry,entry+'\n    '+addition)
    if '..\\src\\renderer\\Console_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\Console.h" />','<ClInclude Include="..\\src\\renderer\\Console.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('Console.h','Console_backend.h'))
    return clouds.project(source)
