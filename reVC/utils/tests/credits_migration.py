"""Move the complete credits sequence, state and reference helpers into C11."""
import re
import console_migration as console
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Credits_Init(void);
void Credits_Start(void);
void Credits_Stop(void);
bool Credits_AreCreditsDone(void);
void Credits_Render(void);
void Credits_PrintCreditSpace(float space,uint32_t *line);
void Credits_PrintCreditText(float scaleX,float scaleY,uint16_t *text,uint32_t *lineoffset,float scrolloffset);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('bool','MenuActive','void','return FrontEndMenuManager.m_bMenuActive;','Read the original live menu-active flag'),
('bool','French','void','return FrontEndMenuManager.m_PrefsLanguage==CMenuManager::LANGUAGE_FRENCH;','Read the original French language predicate'),
('bool','Italian','void','return FrontEndMenuManager.m_PrefsLanguage==CMenuManager::LANGUAGE_ITALIAN;','Read the original Italian language predicate'),
('bool','Spanish','void','return FrontEndMenuManager.m_PrefsLanguage==CMenuManager::LANGUAGE_SPANISH;','Read the original Spanish language predicate'),
('void','UpdatePads','void','CPad::UpdatePads();','Update the original input owner before each credit line'),
('bool','CrossJustDown','void','return CPad::GetPad(0)->GetCrossJustDown();','Retain the exact pad lookup and original button predicate'),
('void','DefinedState','void','DefinedState();','Restore the original defined render state'),
('uint16_t *','Text','const char *key','return TheText.Get(key);','Borrow the actual mutable Unicode text returned for each original key'),
('float','ScreenWidth','void','return SCREEN_WIDTH;','Read the original live screen width'),
('float','ScreenHeight','void','return SCREEN_HEIGHT;','Read the original live screen height'),
('void','FontScale','float x,float y','CFont::SetScale(x,y);','Set font scale after the original C screen calculations'),
('void','FontColor','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetColor(CRGBA(r,g,b,a));','Retain the original typed color construction'),
('void','PrintString','float x,float y,uint16_t *text','CFont::PrintString(x,y,text);','Borrow the original Unicode buffer through the exact print overload'),
('void','FontCentreSize','float value','CFont::SetCentreSize(value);','Set the original C-computed font centre size'),
('void','FontStyle','int16_t style','CFont::SetFontStyle(style);','Select the exact original font style'),
('void','DrawFonts','void','CFont::DrawFonts();','Flush the original credits font submissions'),
('bool','CutsceneBorders','void','#ifdef CUTSCENE_BORDERS_SWITCH\n    return CMenuManager::m_PrefsCutsceneBorders;\n#else\n    return true;\n#endif','Read the original optional cutscene-border preference'),
('bool','Widescreen','void','return TheCamera.m_WideScreenOn;','Read the current camera widescreen flag'),
('void','DrawBorders','void','TheCamera.DrawBordersForWideScreen();','Draw the original camera widescreen borders')]
for name in ['JustifyOff','BackgroundOff','CentreOn','PropOn']:READERS.append(('void','Font'+name,'void','CFont::Set'+name+'();','Retain the original font '+name+' operation'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Credits.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' Credits_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Credits_backend.h"
#include "Font.h"
#include "Frontend.h"
#include "RwHelper.h"
#include "Camera.h"
#include "Text.h"
#include "Pad.h"
'''+''.join(result+' Credits_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'//- rouz edit (ChatGPT)\n'

def section(source):
    # Keep every original credits statement and localization key in its original order
    a=source.index('bool CCredits::bCreditsGoing;');b=source.index('\n}',source.index('bool CCredits::AreCreditsDone',a))+2;return source[a:b]+'\n'

def kernel(source,common):
    # Translate all seven methods without replacing the original unrolled sequence
    body=section(source).replace('bool CCredits::bCreditsGoing;','static bool bCreditsGoing;').replace('uint32 CCredits::CreditsStartTime;','static uint32_t CreditsStartTime;').replace('CCredits::','Credits_')
    names=['Init','Start','Stop','AreCreditsDone','Render','PrintCreditSpace','PrintCreditText']
    for name in names:body=re.sub(r'(?<![\w:])'+name+r'\(','Credits_'+name+'(',body)
    body=body.replace('uint32 &lineoffset','uint32_t *lineoffset').replace('uint32 &line','uint32_t *line').replace('wchar *text','uint16_t *text')
    a=body.index('void\nCredits_PrintCreditSpace');b=body.index('void\nCredits_Render',a)
    helpers=body[a:b];helpers=re.sub(r'\blineoffset\b','(*lineoffset)',helpers);helpers=re.sub(r'\bline\b','(*line)',helpers)
    helpers=helpers.replace('uint32_t *(*lineoffset)','uint32_t *lineoffset').replace('uint32_t *(*line)','uint32_t *line')
    body=body[:a]+helpers+body[b:]
    a=body.index('void\nCredits_Render');b=body.index('bool Credits_AreCreditsDone',a)
    render=body[a:b];render=re.sub(r'(Credits_PrintCredit(?:Text|Space)\([^\n]+), lineoffset([,)])',r'\1, &lineoffset\2',render)
    body=body[:a]+render+body[b:]
    body=body.replace('FrontEndMenuManager.m_bMenuActive','Credits_BackendMenuActive()')
    for name in ['FRENCH','ITALIAN','SPANISH']:body=body.replace('FrontEndMenuManager.m_PrefsLanguage == CMenuManager::LANGUAGE_'+name,'Credits_Backend'+name.title()+'()')
    body=body.replace('CMenuManager::m_PrefsCutsceneBorders','Credits_BackendCutsceneBorders()').replace('TheCamera.m_WideScreenOn','Credits_BackendWidescreen()').replace('TheCamera.DrawBordersForWideScreen()','Credits_BackendDrawBorders()')
    body=body.replace('CPad::UpdatePads()','Credits_BackendUpdatePads()').replace('CPad::GetPad(0)->GetCrossJustDown()','Credits_BackendCrossJustDown()').replace('TheText.Get(','Credits_BackendText(').replace('DefinedState()','Credits_BackendDefinedState()')
    body=re.sub(r'CFont::SetColor\(CRGBA\(([^)]+)\)\)',r'Credits_BackendFontColor(\1)',body)
    for name in ['Scale','CentreSize','FontStyle','JustifyOff','BackgroundOff','CentreOn','PropOn']:body=body.replace('CFont::Set'+name+'(','Credits_Backend'+('FontStyle' if name=='FontStyle' else 'Font'+name)+'(')
    body=body.replace('CFont::DrawFonts()','Credits_BackendDrawFonts()').replace('CFont::PrintString(','Credits_BackendPrintString(').replace('(uint16*)text','text')
    body=re.sub(r'\buint32\b','uint32_t',body)
    comments={'Init':'Stop credits without resetting the original start timestamp','Start':'Publish the active flag before reading the original live timer','Stop':'Clear only the original active flag','PrintCreditSpace':'Preserve float compound-assignment rounding before the original unsigned conversion','PrintCreditText':'Update input before printing and advancing the live borrowed line offset','Render':'Render the complete original sequence with live language and viewport settings','AreCreditsDone':'Return the inverse of the original active flag'}
    for name,comment in comments.items():body=re.sub(r'(Credits_'+name+r'\([^\n]*\)\n\{\n)',lambda match:match[0]+'    // '+comment+'\n',body)
    body=body.replace('else {\n','else {\n        // Preserve the original clipping and line advance after all font callbacks\n').replace('if (y > 20.0f && DEFAULT_SCREEN_HEIGHT - 20.0f > y) {\n','if (y > 20.0f && DEFAULT_SCREEN_HEIGHT - 20.0f > y) {\n            // Submit both original shadow and foreground strings through the same live text pointer\n').replace('\t{\n\t\tbCreditsGoing = false;','\t{\n        // Stop only after the original final scroll-boundary test\n\t\tbCreditsGoing = false;')
    assert not re.search(r'\b(?:CCredits|CPad|CFont|CMenuManager|FrontEndMenuManager|TheCamera|TheText|wchar|uint32)\b',body)
    macros=common[common.index('#define SCREEN_STRETCH_X('):common.index('#include "maths.h"')]
    return '//+ rouz edit (ChatGPT)\n#include "Credits_backend.h"\n#include "Timer.h"\n#include "Draw.h"\n#include "Draw_constants.h"\n#include "FontStyles.h"\n#define SCREEN_WIDTH (Credits_BackendScreenWidth())\n#define SCREEN_HEIGHT (Credits_BackendScreenHeight())\n#define SCREEN_ASPECT_RATIO (Draw_GetAspectRatio())\n'+macros+'\n'+body+'//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace the whole credits class and every original caller without changing other code
    if 'bool CCredits::bCreditsGoing;' in source:return console.calls(HOST)
    if 'class CCredits\n' in source:return HEADER
    lines=[]
    for line in source.splitlines(keepends=True):
        if 'CCredits::' in line and not line.lstrip().startswith('//'):lines.append(re.match(r'\s*',line)[0]+'// Enter the complete C credits implementation with the original arguments\n')
        lines.append(line.replace('CCredits::','Credits_'))
    return console.calls(''.join(lines))

def project(source):
    # Add the C11 sequence beside its typed backend and preserve unique object paths
    if '..\\src\\renderer\\Credits.c"' not in source:
        match=re.search(r'<ClCompile Include="\.\.\\src\\renderer\\Credits\.cpp"\s*(?:/>|>[\s\S]*?</ClCompile>)',source)
        if match:
            entry=match[0]
            addition=entry.replace('Credits.cpp','Credits.c') if '<Filter>' in entry else '<ClCompile Include="..\\src\\renderer\\Credits.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
            source=source.replace(entry,entry+'\n    '+addition)
    if '..\\src\\renderer\\Credits_backend.h"' not in source:
        for entry in ['<ClInclude Include="..\\src\\renderer\\Credits.h" />','<ClInclude Include="..\\src\\renderer\\Credits.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('Credits.h','Credits_backend.h'))
    return console.project(source)
