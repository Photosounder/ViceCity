"""Move the complete on-screen clock/counter controller and canonical state to C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects

METHODS=['Init','Process','ProcessForDisplay','ClearCounter','ClearClock','AddCounter','AddClock']
OPERATIONS=[('COnscreenTimer','OnscreenTimer',name) for name in METHODS]+[('COnscreenTimerEntry','OnscreenClock','Process'),('COnscreenTimerEntry','OnscreenClock','ProcessForDisplayClock'),('COnscreenCounterEntry','OnscreenCounter','ProcessForDisplayCounter')]
FIELDS={'COnscreenTimer':['m_sClocks','m_sCounters','m_bProcessed','m_bDisabled'],'COnscreenTimerEntry':['m_nClockOffset','m_aClockText','m_aClockBuffer','m_bClockProcessed','m_bClockGoingDown'],'COnscreenCounterEntry':['m_nCounterOffset','m_aCounterText','m_nType','m_aCounterBuffer','m_bCounterProcessed']}
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"

enum { COUNTER_DISPLAY_NUMBER, COUNTER_DISPLAY_BAR };
typedef struct COnscreenTimerEntry {
    uint32_t m_nClockOffset;
    char m_aClockText[10];
    char m_aClockBuffer[40];
    bool m_bClockProcessed;
    bool m_bClockGoingDown;
} COnscreenTimerEntry;
typedef struct COnscreenCounterEntry {
    uint32_t m_nCounterOffset;
    char m_aCounterText[10];
    uint16_t m_nType;
    char m_aCounterBuffer[40];
    bool m_bCounterProcessed;
} COnscreenCounterEntry;
typedef struct COnscreenTimer {
    COnscreenTimerEntry m_sClocks[NUMONSCREENCLOCKS];
    COnscreenCounterEntry m_sCounters[NUMONSCREENCOUNTERS];
    bool m_bProcessed;
    bool m_bDisabled;
} COnscreenTimer;
#ifdef __cplusplus
extern "C" {
#endif
extern COnscreenTimer OnscreenTimer_state;
void OnscreenTimer_Init(COnscreenTimer *self);
void OnscreenTimer_Process(COnscreenTimer *self);
void OnscreenTimer_ProcessForDisplay(COnscreenTimer *self);
void OnscreenTimer_ClearCounter(COnscreenTimer *self,uint32_t offset);
void OnscreenTimer_ClearClock(COnscreenTimer *self,uint32_t offset);
void OnscreenTimer_AddCounter(COnscreenTimer *self,uint32_t offset,uint16_t type,char *text,uint16_t pos);
void OnscreenTimer_AddClock(COnscreenTimer *self,uint32_t offset,char *text,bool bGoingDown);
void OnscreenClock_Process(COnscreenTimerEntry *self);
void OnscreenClock_ProcessForDisplayClock(COnscreenTimerEntry *self);
void OnscreenCounter_ProcessForDisplayCounter(COnscreenCounterEntry *self);
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
int32_t *OnscreenTimer_BackendVariable(int32_t offset);
bool OnscreenTimer_BackendPlayingBack(void);
bool OnscreenTimer_BackendDrawHud(void);
bool OnscreenTimer_BackendWidescreen(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Script.h"
#include "Replay.h"
#include "Hud.h"
#include "Camera.h"
#include "OnscreenTimer_backend.h"

int32_t *OnscreenTimer_BackendVariable(int32_t offset)
{
    // Return the actual script variable pointer at the original lookup point
    return CTheScripts::GetPointerToScriptVariable(offset);
}
bool OnscreenTimer_BackendPlayingBack(void)
{
    // Query the live replay predicate before the disabled flag is examined
    return CReplay::IsPlayingBack();
}
bool OnscreenTimer_BackendDrawHud(void)
{
    // Read the live HUD visibility flag without resetting retained display state
    return CHud::m_Wants_To_Draw_Hud;
}
bool OnscreenTimer_BackendWidescreen(void)
{
    // Read the camera flag only when the original tick condition reaches it
    return TheCamera.m_WideScreenOn;
}
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Translate all complete original bodies with explicit receivers and unchanged ordered accesses
    chunks=[]
    for owner,prefix,name in OPERATIONS:
        text=function(source,owner+'::',name).replace(owner+'::'+name+'(',prefix+'_'+name+'('+owner+' *self'+(',' if re.search(owner+r'::'+name+r'\(\s*\)',source) is None else ''))
        for field in FIELDS[owner]:text=re.sub(r'(?<![\w.>])'+field+r'\b','self->'+field,text)
        text=re.sub(r'(self->m_sClocks\[i\])\.Process\(\)',r'OnscreenClock_Process(&\1)',text)
        text=re.sub(r'(self->m_sClocks\[i\])\.ProcessForDisplayClock\(\)',r'OnscreenClock_ProcessForDisplayClock(&\1)',text)
        text=re.sub(r'(self->m_sCounters\[i\])\.ProcessForDisplayCounter\(\)',r'OnscreenCounter_ProcessForDisplayCounter(&\1)',text)
        for old,new in [('CReplay::IsPlayingBack()','OnscreenTimer_BackendPlayingBack()'),('CHud::m_Wants_To_Draw_Hud','OnscreenTimer_BackendDrawHud()'),('TheCamera.m_WideScreenOn','OnscreenTimer_BackendWidescreen()'),('CTheScripts::GetPointerToScriptVariable','OnscreenTimer_BackendVariable'),('int32(Timer_GetTimeStepInMilliseconds())','(int32_t)Timer_GetTimeStepInMilliseconds()')]:text=text.replace(old,new)
        for old,new in [('uint32','uint32_t'),('int32','int32_t'),('uint16','uint16_t')]:text=re.sub(r'\b'+old+r'\b',new,text)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation with explicit record storage',1)
        chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "OnscreenTimer.h"
#include "OnscreenTimer_backend.h"
#include "Timer.h"
#include "DMAudio.h"
#include <stdio.h>
#include <string.h>
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
_Static_assert(sizeof(COnscreenTimerEntry)==56,"Actual original clock layout");
_Static_assert(sizeof(COnscreenCounterEntry)==60,"Actual original counter layout");
_Static_assert(sizeof(COnscreenTimer)==240,"Actual original display controller layout");
COnscreenTimer OnscreenTimer_state;

'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def onscreen_calls(source):
    # Remove the three complete classes and replace every actual canonical-state caller
    if 'class COnscreenTimerEntry\n' in source:return HEADER
    if 'COnscreenTimer::Init()' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    old=source
    source=source.replace('\tstatic COnscreenTimer OnscnTimer;\n','').replace('COnscreenTimer CUserDisplay::OnscnTimer;\n','')
    for name in METHODS:
        source=re.sub(r'\bCUserDisplay::OnscnTimer\.'+name+r'\(', 'OnscreenTimer_'+name+'(&OnscreenTimer_state'+('' if name in ['Init','Process','ProcessForDisplay'] else ','),source)
    source=source.replace('CUserDisplay::OnscnTimer','OnscreenTimer_state')
    for name in ['Init','Process']:source=source.replace('OnscnTimer.'+name+'()','OnscreenTimer_'+name+'(&OnscreenTimer_state)')
    if source==old:return source
    if old.lstrip().startswith('//+ rouz edit (ChatGPT)') and old.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def onscreen_project(source):
    # Give the C controller a distinct basename and retain four typed native readers
    for tag,old,new in [('ClCompile','OnscreenTimer.cpp','OnscreenDisplay.c'),('ClInclude','OnscreenTimer.h','OnscreenTimer_backend.h')]:
        entry='<'+tag+' Include="..\\src\\control\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\control\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\control\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\control\\'+re.escape(old)+r'">\n\s*<Filter>control</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove the complete old producer body and all four native readers after historical transforms
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/OnscreenTimer.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/control/OnscreenDisplay.c').read_text())


def calls(source):
    # Forward historical whole sources through the separately verified pager conversion
    import pager_migration as pager
    return pager.calls(onscreen_calls(source))

def project(source):
    # Retain every prior controller before registering the C pager queue
    import pager_migration as pager
    return pager.project(onscreen_project(source))
