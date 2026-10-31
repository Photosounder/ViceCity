"""Move both money-message classes and all four algorithms to plain C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import shiny_texts_migration as shiny
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct MoneyMessageVector { float x,y,z; } MoneyMessageVector;
typedef struct MoneyMessageColor { uint8_t r,g,b,a; } MoneyMessageColor;
typedef struct MoneyMessage {
    uint32_t m_nTimeRegistered;
    MoneyMessageVector m_vecPosition;
    uint16_t m_aText[16];
    MoneyMessageColor m_Colour;
    float m_fSize,m_fOpacity;
} MoneyMessage;
#ifdef __cplusplus
extern "C" {
#endif
void MoneyMessage_Render(MoneyMessage *message);
void MoneyMessages_Init(void);
void MoneyMessages_Render(void);
void MoneyMessages_RegisterOne(MoneyMessageVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity);
#ifdef __cplusplus
}
class CVector;
MoneyMessageVector MoneyMessages_FromVector(CVector position);
void MoneyMessages_RegisterOneLegacy(CVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity);
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('bool','Project','MoneyMessageVector position,MoneyMessageVector *out,float *width,float *height,bool farclip','RwV3d projected;\n    bool visible=CSprite::CalcScreenCoors(CVector(position.x,position.y,position.z),&projected,width,height,farclip);\n    if(visible){\n        // Copy the actual typed projection only on the original successful path\n        out->x=projected.x;out->y=projected.y;out->z=projected.z;\n    }\n    return visible;','Project through the exact original typed sprite operation'),
('void','AsciiToUnicode','const char *source,uint16_t *target','AsciiToUnicode(source,target);','Enter the existing converter with the original live source and destination'),
('float','ScreenWidth','void','return SCREEN_WIDTH;','Read the original live screen width at its use point'),
('float','ScreenHeight','void','return SCREEN_HEIGHT;','Read the original live screen height at its use point'),
('void','FontScale','float x,float y','CFont::SetScale(x,y);','Set the original font scale after the original C calculations'),
('void','FontCentreSize','float value','CFont::SetCentreSize(value);','Retain the original full-screen centre size'),
('void','FontColor','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetColor(CRGBA(r,g,b,a));','Retain the original color construction and alpha narrowing'),
('void','PrintString','float x,float y,uint16_t *text','CFont::PrintString(x,y,text);','Enter the exact original mutable Unicode print overload'),
('bool','Japanese','void','#ifdef MORE_LANGUAGES\n    return CFont::IsJapanese();\n#else\n    return false;\n#endif','Preserve the original otherwise-unused locale expression')]
for name in ['PropOn','BackgroundOff','CentreOn','JustifyOff','BackGroundOnlyTextOff']:READERS.append(('void','Font'+name,'void','CFont::Set'+name+'();','Retain the original typed font '+name+' operation'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "MoneyMessages.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' MoneyMessages_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "MoneyMessages_backend.h"
#include "Sprite.h"
#include "Font.h"
'''+''.join(result+' MoneyMessages_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'''
MoneyMessageVector MoneyMessages_FromVector(CVector position)
{
    // Copy the actual typed components without assuming the CVector object layout
    MoneyMessageVector result={position.x,position.y,position.z};return result;
}
void MoneyMessages_RegisterOneLegacy(CVector position,const char *text,uint8_t r,uint8_t g,uint8_t b,float size,float opacity)
{
    // Preserve the original by-value position capture before any text conversion
    MoneyMessages_RegisterOne(MoneyMessages_FromVector(position),text,r,g,b,size,opacity);
}
//- rouz edit (ChatGPT)
'''

def section(source):
    # Delimit all four algorithms and their complete private pool definition
    a=source.index('#define MONEY_MESSAGE_LIFETIME_MS 2000');b=source.index('CRGBA FoamColour(',a);return source[a:b]

def record_header(source):
    # Expose private test observations without changing any declared field or method
    a=source.index('class CMoneyMessage\n');b=source.index('class CSpecialParticleStuff',a)
    return source[a:b].replace('class CMoneyMessage\n{','class CMoneyMessage\n{\npublic:').replace('class CMoneyMessages\n{','class CMoneyMessages\n{\npublic:')

def calls(source):
    # Remove the two owners and direct the C controller to the C manager
    if '#define MONEY_MESSAGE_LIFETIME_MS 2000' in source:source=source.replace(section(source),'')
    if 'class CMoneyMessage\n' in source:
        a=source.index('class CMoneyMessage\n');b=source.index('class CSpecialParticleStuff',a);source=source[:a]+'#include "MoneyMessages.h"\n\n'+source[b:]
    for name in ['Init','Render']:
        if 'SpecialFX_BackendMoneyMessages'+name+'(void)\n{' in source:source=source.replace(function(source,'SpecialFX_BackendMoneyMessages',name)+'\n','')
        source=re.sub(r'^void SpecialFX_BackendMoneyMessages'+name+r'\(void\);\n','',source,flags=re.M)
        if 'SpecialFX_BackendMoneyMessages'+name+'(' in source:
            source=source.replace('SpecialFX_BackendMoneyMessages'+name+'(','MoneyMessages_'+name+'(')
            if '#include "MoneyMessages.h"' not in source:source=source.replace('#include "SpecialFX_backend.h"','#include "SpecialFX_backend.h"\n#include "MoneyMessages.h"')
    source=source.replace('CMoneyMessages::RegisterOne(','MoneyMessages_RegisterOneLegacy(').replace('CMoneyMessages::','MoneyMessages_')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bMoneyMessages_(?:Init|Render|RegisterOneLegacy)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the C money-message algorithm with the original arguments\n')
        lines.append(line)
    return shiny.calls(''.join(lines))

def scaling(source):
    # Keep the entire original scaling expressions and configuration guards
    a=source.index('#define SCREEN_STRETCH_X(');b=source.index('#include "maths.h"',a)
    return '#define SCREEN_WIDTH (MoneyMessages_BackendScreenWidth())\n#define SCREEN_HEIGHT (MoneyMessages_BackendScreenHeight())\n#define SCREEN_ASPECT_RATIO (Draw_GetAspectRatio())\n'+source[a:b]

def kernel(source,common):
    # Transcribe all algorithms while preserving partial initialization and every live read
    body=section(source).replace('CMoneyMessage CMoneyMessages::aMoneyMessages[NUMMONEYMESSAGES];','static MoneyMessage aMoneyMessages[NUMMONEYMESSAGES];')
    body=body.replace('CMoneyMessage::Render()','MoneyMessage_Render(MoneyMessage *message)')
    a=body.index('void\nMoneyMessage_Render');b=body.index('void\nCMoneyMessages::Init',a)
    member=body[a:b];member=re.sub(r'\b(m_nTimeRegistered|m_vecPosition|m_aText|m_Colour|m_fSize|m_fOpacity)\b',r'message->\1',member)
    body=body[:a]+member+body[b:]
    body=body.replace('CMoneyMessages::RegisterOne(CVector vecPos, const char *pText, uint8 bRed, uint8 bGreen, uint8 bBlue, float fSize, float fOpacity)','MoneyMessages_RegisterOne(MoneyMessageVector vecPos,const char *pText,uint8_t bRed,uint8_t bGreen,uint8_t bBlue,float fSize,float fOpacity)')
    body=body.replace('CMoneyMessages::','MoneyMessages_').replace('MoneyMessages_Init()','MoneyMessages_Init(void)').replace('MoneyMessages_Render()','MoneyMessages_Render(void)')
    body=body.replace('aMoneyMessages[i].Render()','MoneyMessage_Render(&aMoneyMessages[i])')
    body=body.replace('CSprite::CalcScreenCoors(message->m_vecPosition + CVector(0.0f, 0.0f, fLifeTime),','MoneyMessages_BackendProject(MoneyMessages_Rise(message->m_vecPosition,fLifeTime),')
    body=body.replace('RwV3d vecOut','MoneyMessageVector vecOut').replace('Min(','MONEY_MESSAGES_MIN(')
    body=body.replace('AsciiToUnicode(','MoneyMessages_BackendAsciiToUnicode(')
    for name in ['PropOn','BackgroundOff','CentreOn','JustifyOff','BackGroundOnlyTextOff','Scale','CentreSize']:body=body.replace('CFont::Set'+name+'(','MoneyMessages_BackendFont'+name+'(')
    body=body.replace('CFont::SetColor(CRGBA(message->m_Colour.r, message->m_Colour.g, message->m_Colour.b, (255.0f - 255.0f * fLifeTime) * message->m_fOpacity))','MoneyMessages_BackendFontColor(message->m_Colour.r,message->m_Colour.g,message->m_Colour.b,(255.0f - 255.0f * fLifeTime) * message->m_fOpacity)')
    body=body.replace('CFont::PrintString(','MoneyMessages_BackendPrintString(').replace('FONT_LOCALE(FONT_STANDARD)','MONEY_MESSAGES_FONT_LOCALE(FONT_STANDARD)')
    body=body.replace('.m_Colour.red','.m_Colour.r').replace('.m_Colour.green','.m_Colour.g').replace('.m_Colour.blue','.m_Colour.b')
    for a,b in [('uint32','uint32_t'),('int32','int32_t')]:body=re.sub(r'\b'+a+r'\b',b,body)
    body=body.replace('\n{\n','\n{\n\t// Retain the complete original money-message operation and write order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve original conditions and live owner reads\n',body,flags=re.M)
    helpers='''#define MONEY_MESSAGES_MIN(a,b) ((a)<(b)?(a):(b))
#ifdef MORE_LANGUAGES
#define MONEY_MESSAGES_FONT_LOCALE(style) (MoneyMessages_BackendJapanese()?FONT_JAPANESE:(style))
#else
#define MONEY_MESSAGES_FONT_LOCALE(style) (style)
#endif
static MoneyMessageVector MoneyMessages_Rise(MoneyMessageVector position,float height)
{
    // Preserve all three additions including the original signed-zero behavior
    MoneyMessageVector result={position.x+0.0f,position.y+0.0f,position.z+height};return result;
}
'''
    assert not re.search(r'\b(?:CMoneyMessage|CMoneyMessages|CVector|CFont|CSprite|RwV3d|CRGBA)\b',body)
    return '//+ rouz edit (ChatGPT)\n#include "MoneyMessages_backend.h"\n#include "Timer.h"\n#include "Draw.h"\n#include "Draw_constants.h"\n#include "FontStyles.h"\n\n'+scaling(common)+helpers+body.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register both actual compilation units with distinct C and C++ object names
    if '..\\src\\renderer\\MoneyMessages.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n      <ObjectFileName>$(IntDir)SpecialFX.c.obj</ObjectFileName>\n    </ClCompile>'
        for original in [entry,entry.replace('      <ObjectFileName>$(IntDir)SpecialFX.c.obj</ObjectFileName>\n','')]:
            source=source.replace(original,original+'\n    <ClCompile Include="..\\src\\renderer\\MoneyMessages.cpp" />\n    '+original.replace('SpecialFX.c','MoneyMessages.c'))
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.c">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.c','MoneyMessages.cpp')+'\n    '+entry.replace('SpecialFX.c','MoneyMessages.c'))
    for name in ['MoneyMessages.h','MoneyMessages_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    return shiny.project(source)
