"""Relocate the complete debug text, screen-string and line queues into C11."""
import re
import object_names_migration as objects
from events_migration import arguments
OBSERVER=None
OPERATIONS=['DebugInitTextBuffer','DebugAddText','DebugDisplayTextBuffer','DisplayScreenStrings','PrintAt','AddLine','DrawLines']
FIELDS=['ms_nCurrentTextLine','ms_aTextBuffer','ms_aScreenStrs','ms_nScreenStrs','ms_aLines','ms_nLines']
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
enum {Debug_MAX_LINES=15,Debug_MAX_STR_LEN=80,Debug_MAX_SCREEN_STRS=100,Debug_MAX_DEBUG_LINES=100};
typedef struct Debug_Vector {float x,y,z;} Debug_Vector;
typedef struct Debug_ScreenStr {int x,y;char str[256];} Debug_ScreenStr;
typedef struct Debug_Line {Debug_Vector p1,p2;uint32_t c1,c2;} Debug_Line;
#ifdef __cplusplus
extern "C" {
#endif
extern bool gbDebugStuffInRelease;
extern int16_t Debug_ms_nCurrentTextLine;
extern char Debug_ms_aTextBuffer[Debug_MAX_LINES][Debug_MAX_STR_LEN];
extern Debug_ScreenStr Debug_ms_aScreenStrs[Debug_MAX_SCREEN_STRS];
extern int Debug_ms_nScreenStrs;
extern Debug_Line Debug_ms_aLines[Debug_MAX_DEBUG_LINES];
extern int Debug_ms_nLines;
void Debug_DebugInitTextBuffer(void);
void Debug_DebugAddText(const char *str);
void Debug_DebugDisplayTextBuffer(void);
void Debug_DisplayScreenStrings(void);
void Debug_PrintAt(const char *str,int x,int y);
void Debug_AddLine(Debug_Vector p1,Debug_Vector p2,uint32_t c1,uint32_t c2);
void Debug_DrawLines(void);
#ifdef __cplusplus
}
class CVector;
Debug_Vector Debug_FromVector(CVector value);
#endif
//- rouz edit (ChatGPT)
'''
LEAVES=[('void',name,'void','CFont::'+name+'();') for name in ['SetPropOn','SetPropOff','SetBackgroundOff','SetCentreOff','SetRightJustifyOn','SetRightJustifyOff','SetJustifyOn','SetJustifyOff','SetBackGroundOnlyTextOff','DrawFonts']]
LEAVES += [('void','SetScale','float x,float y','CFont::SetScale(x,y);'),('void','SetRightJustifyWrap','float wrap','CFont::SetRightJustifyWrap(wrap);'),('void','SetWrapx','float wrap','CFont::SetWrapx(wrap);'),('void','SetFontStyle','int16_t style','CFont::SetFontStyle(style);'),('void','Color','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetColor(CRGBA(r,g,b,a));'),('void','PrintString','float x,float y,uint16_t *text','CFont::PrintString(x,y,text);'),('void','AsciiToUnicode','const char *source,uint16_t *target','AsciiToUnicode(source,target);'),('uint16_t *','Unicode','void','return gUString;'),('void','ObrsPrintfString','const char *str,int16_t x,int16_t y','ObrsPrintfString(str,x,y);'),('void','TextureNone','void','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);'),('void','DepthTestOff','void','RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void*)FALSE);')]
BACKEND='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Debug.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(kind+' Debug_Backend'+name+'('+args+');\n' for kind,name,args,code in LEAVES)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def host():
    # Retain only native font, scratch-buffer and render-state primitives
    source='//+ rouz edit (ChatGPT)\n'+''.join('#include "'+name+'.h"\n' for name in ['common','Debug_backend','Font','RwHelper','main','Text'])
    source+='Debug_Vector Debug_FromVector(CVector value)\n{\n    // Copy the original three float components at the native argument boundary\n    Debug_Vector result={value.x,value.y,value.z};return result;\n}\n'
    return source+'\n'.join(kind+' Debug_Backend'+name+'('+args+')\n{\n    // Access the original native primitive at its original C policy access point\n    '+code+'\n}\n' for kind,name,args,code in LEAVES)+'//- rouz edit (ChatGPT)\n'

def kernel(source):
    # Relocate every complete original queue operation and every stale buffer byte
    text=re.sub(r'^#include[^\n]*\n','',source,flags=re.M).replace('//+ rouz edit (ChatGPT)','').replace('//- rouz edit (ChatGPT)','').strip()
    text=text.replace('CDebug::','Debug_')
    for name in FIELDS+['MAX_LINES','MAX_STR_LEN','MAX_SCREEN_STRS','MAX_DEBUG_LINES']:text=re.sub(r'(?<![\w])'+name+r'\b','Debug_'+name,text)
    for old,new in [('int16','int16_t'),('int32','int32_t'),('uint32','uint32_t'),('CVector','Debug_Vector'),('Line','Debug_Line'),('nil','NULL')]:text=re.sub(r'\b'+old+r'\b',new,text)
    for kind,name,args,code in LEAVES:
        if name not in ['Color','AsciiToUnicode','Unicode','ObrsPrintfString','TextureNone','DepthTestOff']:text=text.replace('CFont::'+name+'(','Debug_Backend'+name+'(')
    text=re.sub(r'CFont::SetColor\(CRGBA\(([^()]*)\)\)',r'Debug_BackendColor(\1)',text)
    text=text.replace('AsciiToUnicode(','Debug_BackendAsciiToUnicode(').replace('ObrsPrintfString(','Debug_BackendObrsPrintfString(').replace('gUString','Debug_BackendUnicode()')
    text=text.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL)','Debug_BackendTextureNone()').replace('RwRenderStateSet(rwRENDERSTATEZTESTENABLE, (void*)FALSE)','Debug_BackendDepthTestOff()')
    for name in OPERATIONS:text=re.sub(r'(Debug_'+name+r'\([^\n]*\)\s*\n\{)',r'\1\n    // Preserve the complete original '+name+' operation and every live queue access',text,count=1)
    for name in ['DebugInitTextBuffer','DebugDisplayTextBuffer','DisplayScreenStrings']:text=text.replace('Debug_'+name+'()','Debug_'+name+'(void)')
    return '//+ rouz edit (ChatGPT)\n#include "Debug_backend.h"\n#include "Lines.h"\n#include "FontStyles.h"\n#include <string.h>\n\n'+text+'\n//- rouz edit (ChatGPT)\n'

def debug_display_calls(source):
    # Remove the entire static owner and project every native caller with the original vector argument copies
    if 'class CDebug\n' in source:return HEADER
    if 'int16 CDebug::ms_nCurrentTextLine;' in source:
        if OBSERVER:OBSERVER(source)
        return host()
    original=source;token='CDebug::AddLine(';pos=0
    while (index:=source.find(token,pos))>=0:
        end,args=arguments(source,index+len(token)-1);assert len(args)==4
        value='Debug_AddLine('+','.join('Debug_FromVector('+v+')' if i<2 else v for i,v in enumerate(args))+')';source=source[:index]+value+source[end:];pos=index+len(value)
    source=source.replace('CDebug::','Debug_')
    if source==original:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def debug_display_project(source):
    # Register a unique C display object and typed native boundary without adding another paired basename
    for tag,old,new in [('ClCompile','Debug.cpp','DebugDisplaySystem.c'),('ClInclude','Debug.h','Debug_backend.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />';addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove full relocation of the original debug producer and all canonical queue storage
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/core/Debug.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/core/DebugDisplaySystem.c').read_text())

def verify_dependency(saved,current):
    # Permit only the explicit complete debug-display projection through historical dependency guards
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Preserve the complete debug-display projection before replacing the embedded autopilot class
    import autopilot_migration as autopilot
    return autopilot.calls(debug_display_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named autopilot C object
    import autopilot_migration as autopilot
    return autopilot.project(debug_display_project(source))
