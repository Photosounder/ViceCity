"""Remove all three user-display classes and move their complete control logic into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
OPERATIONS=[('CPlaceName','PlaceName',name) for name in ['Init','Process','Display','ProcessAfterFrontEndShutDown']]+[('CCurrentVehicle','CurrentVehicle',name) for name in ['Init','Process','Display']]+[('CUserDisplay','UserDisplay',name) for name in ['Init','Process']]
FIELDS={'CPlaceName':['m_pZone','m_pZone2','m_nAdditionalTimer'],'CCurrentVehicle':['m_pCurrentVehicle']}
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "Pager.h"
#include "OnscreenTimer.h"

typedef struct CZone CZone;
#ifdef __cplusplus
class CVehicle;
#else
typedef struct CVehicle CVehicle;
#endif
typedef struct CPlaceName {
    CZone *m_pZone;
    CZone *m_pZone2;
    int16_t m_nAdditionalTimer;
} CPlaceName;
typedef struct CCurrentVehicle {
    CVehicle *m_pCurrentVehicle;
} CCurrentVehicle;
#ifdef __cplusplus
extern "C" {
#endif
extern CPlaceName PlaceName_state;
extern CCurrentVehicle CurrentVehicle_state;
void PlaceName_Init(CPlaceName *self);
void PlaceName_Process(CPlaceName *self);
void PlaceName_Display(CPlaceName *self);
void PlaceName_ProcessAfterFrontEndShutDown(CPlaceName *self);
void CurrentVehicle_Init(CCurrentVehicle *self);
void CurrentVehicle_Process(CCurrentVehicle *self);
void CurrentVehicle_Display(CCurrentVehicle *self);
void UserDisplay_Init(void);
void UserDisplay_Process(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "User.h"
#include "Zones.h"
#ifdef __cplusplus
extern "C" {
#endif
ZonesPosition UserDisplay_BackendPlayerPosition(void);
void UserDisplay_BackendSetZoneName(uint16_t *name);
uint16_t **UserDisplay_BackendLastZoneName(void);
uint32_t *UserDisplay_BackendZoneState(void);
bool UserDisplay_BackendInVehicle(void);
CVehicle *UserDisplay_BackendVehicle(void);
CVehicle *UserDisplay_BackendShortCutTaxi(void);
int16_t UserDisplay_BackendModelIndex(const CVehicle *vehicle);
char *UserDisplay_BackendGameName(int16_t model);
uint16_t *UserDisplay_BackendText(const char *name);
void UserDisplay_BackendSetVehicleName(uint16_t *name);
bool UserDisplay_BackendPlayingBack(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "GameLogic.h"
#include "Hud.h"
#include "PlayerPed.h"
#include "Replay.h"
#include "Text.h"
#include "Vehicle.h"
#include "World.h"
#include "User_backend.h"

ZonesPosition UserDisplay_BackendPlayerPosition(void)
{
    // Capture the original player position once before either navigation-zone query
    CVector position=CWorld::Players[CWorld::PlayerInFocus].GetPos();
    return ZonesPosition_FromVector(position);
}
void UserDisplay_BackendSetZoneName(uint16_t *name)
{
    // Publish the selected original UTF-16 label through the native HUD owner
    CHud::SetZoneName(name);
}
uint16_t **UserDisplay_BackendLastZoneName(void)
{
    // Expose the original HUD history slot for the ordered C reset
    return &CHud::m_pLastZoneName;
}
uint32_t *UserDisplay_BackendZoneState(void)
{
    // Expose the original HUD state slot for the following C reset
    return &CHud::m_ZoneState;
}
bool UserDisplay_BackendInVehicle(void)
{
    // Query the live focused pedestrian at the original vehicle-state check
    return CWorld::Players[CWorld::PlayerInFocus].m_pPed->InVehicle();
}
CVehicle *UserDisplay_BackendVehicle(void)
{
    // Read the focused pedestrian again after the original predicate has returned
    return CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pMyVehicle;
}
CVehicle *UserDisplay_BackendShortCutTaxi(void)
{
    // Convert the original typed taxi pointer through its native vehicle base
    return CGameLogic::pShortCutTaxi;
}
int16_t UserDisplay_BackendModelIndex(const CVehicle *vehicle)
{
    // Read the model index through the actual native vehicle inheritance
    return vehicle->GetModelIndex();
}
char *UserDisplay_BackendGameName(int16_t model)
{
    // Resolve the original typed model record and borrow its existing game-name array
    return ((CVehicleModelInfo*)CModelInfo::GetModelInfo(model))->m_gameName;
}
uint16_t *UserDisplay_BackendText(const char *name)
{
    // Perform the original native text lookup after the C display controller selects a model
    return TheText.Get(name);
}
void UserDisplay_BackendSetVehicleName(uint16_t *name)
{
    // Publish the original borrowed UTF-16 vehicle name through the native HUD owner
    CHud::SetVehicleName(name);
}
bool UserDisplay_BackendPlayingBack(void)
{
    // Query the original replay predicate at the compile-time guarded update gate
    return CReplay::IsPlayingBack();
}
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Preserve every complete original body while replacing constructors with equivalent static C state
    for owner in ['CPlaceName','CCurrentVehicle']:assert re.search(owner+r'::'+owner+r'\(\)\s*\{\s*Init\(\);\s*\}',source)
    chunks=[]
    for owner,prefix,name in OPERATIONS:
        text=function(source,owner+'::',name)
        text=re.sub(owner+'::'+name+r'\((?:void)?\)', prefix+'_'+name+'('+(owner+' *self' if owner!='CUserDisplay' else 'void')+')',text,1)
        for field in FIELDS.get(owner,[]):text=re.sub(r'(?<![\w.>])'+field+r'\b','self->'+field,text)
        if owner!='CUserDisplay':text=re.sub(r'(?<![\w])Display\(\)',prefix+'_Display(self)',text)
        text=text.replace('CVector pos = CWorld::Players[CWorld::PlayerInFocus].GetPos();','ZonesPosition pos = UserDisplay_BackendPlayerPosition();').replace('ZonesPosition_FromVector(*(&pos))','pos')
        for old,new in [('CHud::SetZoneName','UserDisplay_BackendSetZoneName'),('CHud::m_pLastZoneName','(*UserDisplay_BackendLastZoneName())'),('CHud::m_ZoneState','(*UserDisplay_BackendZoneState())'),('CWorld::Players[CWorld::PlayerInFocus].m_pPed->InVehicle()','UserDisplay_BackendInVehicle()'),('CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pMyVehicle','UserDisplay_BackendVehicle()'),('CGameLogic::pShortCutTaxi','UserDisplay_BackendShortCutTaxi()'),('((CVehicleModelInfo*)CModelInfo::GetModelInfo(self->m_pCurrentVehicle->GetModelIndex()))->m_gameName','UserDisplay_BackendGameName(UserDisplay_BackendModelIndex(self->m_pCurrentVehicle))'),('TheText.Get','UserDisplay_BackendText'),('CHud::SetVehicleName','UserDisplay_BackendSetVehicleName'),('CReplay::IsPlayingBack()','UserDisplay_BackendPlayingBack()')]:text=text.replace(old,new)
        for category in ['PlaceName','CurrentVehicle']:
            for method in ['Init','Process']:text=text.replace(category+'.'+method+'()',category+'_'+method+'(&'+category+'_state)')
        for old,new in [('wchar','uint16_t'),('int16','int16_t'),('nil','NULL')]:text=re.sub(r'\b'+old+r'\b',new,text)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and its ordered live accesses',1);chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#include "User.h"
#include "User_backend.h"
#include "Zones.h"
#include <stddef.h>
CPlaceName PlaceName_state;
CCurrentVehicle CurrentVehicle_state;

'''+ '\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def user_calls(source):
    # Remove all three complete classes and update the two actual external consumers
    if 'class CUserDisplay\n' in source:return HEADER
    if 'CPlaceName::CPlaceName()' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    if 'CUserDisplay::' not in source:return source
    old=source
    source=source.replace('CUserDisplay::PlaceName.ProcessAfterFrontEndShutDown()','PlaceName_ProcessAfterFrontEndShutDown(&PlaceName_state)').replace('CUserDisplay::Init()','UserDisplay_Init()').replace('CUserDisplay::Process()','UserDisplay_Process()')
    if source==old:return source
    if old.lstrip().startswith('//+ rouz edit (ChatGPT)') and old.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def user_project(source):
    # Register uniquely named C state/control logic and the typed native field boundaries
    for tag,old,new in [('ClCompile','User.cpp','UserDisplay.c'),('ClInclude','User.h','User_backend.h')]:
        entry='<'+tag+' Include="..\\src\\core\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\core\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\core\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\core\\'+re.escape(old)+r'">\n\s*<Filter>core</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove all nine old producer bodies and both constructor-equivalent canonical records
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/core/User.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/core/UserDisplay.c').read_text())


def calls(source):
    # Forward historical sources through the separately verified full message-manager conversion
    import messages_migration as messages
    return messages.calls(user_calls(source))

def project(source):
    # Retain every earlier owner before registering the C message queues and shared action enum
    import messages_migration as messages
    return messages.project(user_project(source))
