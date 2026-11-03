"""Move all seven pager algorithms and the canonical eight-message queue into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
METHODS=['Init','Process','Display','AddMessage','AddMessageWithNumber','ClearMessages','RestartCurrentMessage']
FIELDS=['m_nNumDisplayLetters','m_messages']
MESSAGE_FIELDS=['m_pText','m_nSpeedMs','m_nCurrentPosition','m_nStringLength','m_nPriority','m_nTimeToChangePosition','unused','m_nNumber']
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>

typedef struct PagerMessage {
    uint16_t *m_pText;
    uint16_t m_nSpeedMs;
    int16_t m_nCurrentPosition;
    uint16_t m_nStringLength;
    uint16_t m_nPriority;
    uint32_t m_nTimeToChangePosition;
    int16_t unused;
    int32_t m_nNumber[6];
} PagerMessage;
#define NUMPAGERMESSAGES 8
typedef struct CPager {
    int16_t m_nNumDisplayLetters;
    PagerMessage m_messages[NUMPAGERMESSAGES];
} CPager;
#ifdef __cplusplus
extern "C" {
#endif
extern CPager Pager_state;
void Pager_Init(CPager *self);
void Pager_Process(CPager *self);
void Pager_Display(CPager *self);
void Pager_AddMessage(CPager *self,uint16_t *str,uint16_t speed,uint16_t priority,uint16_t a5);
void Pager_AddMessageWithNumber(CPager *self,uint16_t *str,int32_t n1,int32_t n2,int32_t n3,int32_t n4,int32_t n5,int32_t n6,uint16_t speed,uint16_t priority,uint16_t a11);
void Pager_ClearMessages(CPager *self);
void Pager_RestartCurrentMessage(CPager *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
bool Pager_BackendWidescreen(void);
bool Pager_BackendDrawHud(void);
uint16_t Pager_BackendBigMessage0(void);
uint16_t Pager_BackendBigMessage2(void);
void Pager_BackendSetMessage(uint16_t *message);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Messages.h"
#include "Hud.h"
#include "Camera.h"
#include "Pager_backend.h"

bool Pager_BackendWidescreen(void)
{
    // Read the live camera flag at the original short-circuit position
    return TheCamera.m_WideScreenOn;
}
bool Pager_BackendDrawHud(void)
{
    // Read the live HUD visibility flag after the original camera test
    return CHud::m_Wants_To_Draw_Hud;
}
uint16_t Pager_BackendBigMessage0(void)
{
    // Read the original first message channel only when the preceding gates permit it
    return CHud::m_BigMessage[0][0];
}
uint16_t Pager_BackendBigMessage2(void)
{
    // Read the original third message channel only when the preceding gates permit it
    return CHud::m_BigMessage[2][0];
}
void Pager_BackendSetMessage(uint16_t *message)
{
    // Publish the original scroll slice through the existing synchronous HUD copy
    CHud::SetPagerMessage(message);
}
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Translate every complete original queue and display algorithm without replacing its decisions
    chunks=[]
    for name in METHODS:
        text=function(source,'CPager::',name)
        text=text.replace('CPager::'+name+'(', 'Pager_'+name+'(CPager *self'+('' if re.search(r'CPager::'+name+r'\(\s*\)',source) else ','))
        for field in FIELDS:text=re.sub(r'(?<![\w.>])'+field+r'\b','self->'+field,text)
        for method in ['ClearMessages','Display','RestartCurrentMessage']:text=re.sub(r'(?<![\w])'+method+r'\(\)', 'Pager_'+method+'(self)',text)
        for old,new in [('TheCamera.m_WideScreenOn','Pager_BackendWidescreen()'),('CHud::m_Wants_To_Draw_Hud','Pager_BackendDrawHud()'),('CHud::m_BigMessage[0][0]','Pager_BackendBigMessage0()'),('CHud::m_BigMessage[2][0]','Pager_BackendBigMessage2()'),('CMessages::AddToPreviousBriefArray','Messages_AddToPreviousBriefArray'),('CMessages::InsertNumberInString','Messages_InsertNumberInString'),('CMessages::GetWideStringLength','Messages_GetWideStringLength'),('CHud::SetPagerMessage','Pager_BackendSetMessage')]:text=text.replace(old,new)
        for old,new in [('wchar','uint16_t'),('uint16','uint16_t'),('int16','int16_t'),('int32','int32_t'),('uint32','uint32_t'),('nil','NULL')]:text=re.sub(r'\b'+old+r'\b',new,text)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation with explicit queue storage',1);chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#include "Pager.h"
#include "Pager_backend.h"
#include "Messages.h"
#include "Timer.h"
#include <stddef.h>
CPager Pager_state;

'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def pager_calls(source):
    # Remove the complete pager class and replace every actual canonical-queue call
    if 'class CPager\n' in source:return HEADER
    if 'CPager::Init()' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    if 'CUserDisplay::Pager' not in source and 'static CPager Pager;' not in source and 'Pager.Init();' not in source and 'Pager.Process();' not in source:return source
    old=source
    source=source.replace('\tstatic CPager Pager;\n','').replace('CPager CUserDisplay::Pager;\n','')
    for name in METHODS:source=re.sub(r'\bCUserDisplay::Pager\.'+name+r'\(', 'Pager_'+name+'(&Pager_state'+('' if name in ['Init','Process','Display','ClearMessages','RestartCurrentMessage'] else ','),source)
    for name in ['Init','Process']:source=source.replace('Pager.'+name+'()','Pager_'+name+'(&Pager_state)')
    if source==old:return source
    if old.lstrip().startswith('//+ rouz edit (ChatGPT)') and old.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def pager_project(source):
    # Register the uniquely named C queue and its native typed reader declarations
    for tag,old,new in [('ClCompile','Pager.cpp','PagerQueue.c'),('ClInclude','Pager.h','Pager_backend.h')]:
        entry='<'+tag+' Include="..\\src\\text\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\text\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\text\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\text\\'+re.escape(old)+r'">\n\s*<Filter>text</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove the whole old producer relocation in addition to all eight native bindings
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/text/Pager.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/text/PagerQueue.c').read_text())


def calls(source):
    # Forward historical whole sources through the separately verified user-display class removal
    import user_display_migration as user
    return user.calls(pager_calls(source))

def project(source):
    # Retain every earlier controller before registering the C user-display owner
    import user_display_migration as user
    return user.project(pager_project(source))
