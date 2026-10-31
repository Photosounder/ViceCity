"""Move the complete rampage policy and all canonical state from CDarkel into C11."""
import re
import object_names_migration as objects
from events_migration import arguments,body
OBSERVER=None
OPERATIONS=['CalcFade','DrawMessages','FrenzyOnGoing','Init','QueryModelsKilledByPlayer','ReadStatus','RegisterCarBlownUpByPlayer','RegisterKillByPlayer','RegisterKillNotByPlayer','ResetModelsKilledByPlayer','ResetOnPlayerDeath','StartFrenzy','Update','DealWithWeaponChangeAtEndOfFrenzy']
SIGNATURES=[('uint8_t','CalcFade','uint32_t time,uint32_t start,uint32_t end'),('void','DrawMessages','void'),('bool','FrenzyOnGoing','void'),('void','Init','void'),('uint16_t','QueryModelsKilledByPlayer','int32_t modelId'),('uint16_t','ReadStatus','void'),('void','RegisterCarBlownUpByPlayer','CVehicle *vehicle'),('void','RegisterKillByPlayer','CPed *victim,enum eWeaponType weapon,bool headshot'),('void','RegisterKillNotByPlayer','CPed *victim,enum eWeaponType weapontype'),('void','ResetModelsKilledByPlayer','void'),('void','ResetOnPlayerDeath','void'),('void','StartFrenzy','enum eWeaponType weaponType,int32_t time,uint16_t kill,int32_t modelId0,uint16_t *text,int32_t modelId2,int32_t modelId3,int32_t modelId4,bool standardSound,bool needHeadShot'),('void','Update','void'),('void','DealWithWeaponChangeAtEndOfFrenzy','void')]

def scalar(source):
    # Preserve every original scalar width and enum while removing C++ shorthand types
    for old,new in [('uint8','uint8_t'),('int8','int8_t'),('uint16','uint16_t'),('uint32','uint32_t'),('int32','int32_t'),('wchar','uint16_t'),('nil','NULL')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source=re.sub(r'(?<!enum )\beWeaponType\b','enum eWeaponType',source)
    return source

def fields(header):
    # Capture every complete static field including the entire registered-model counter array
    result=re.findall(r'static\s+(int32|uint16|bool|wchar)\s+(\*?)(\w+)(\[[^\]]+\])?\s*;',header);assert len(result)==18,len(result)
    return [(scalar(kind),star,name,array) for kind,star,name,array in result]

def header(original):
    # Expose every original canonical state field and operation through plain C declarations
    enums=original[original.index('enum\n'):original.index('\nclass CDarkel')]
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "ModelIndices.h"\n#include "WeaponType.h"\n#ifdef __cplusplus\nclass CVehicle;class CPed;\nextern "C" {\n#else\ntypedef struct CVehicle CVehicle;typedef struct CPed CPed;\n#endif\n'+enums+'\n'+''.join('extern '+kind+' '+star+'Darkel_'+name+array+';\n' for kind,star,name,array in fields(original))+''.join(ret+' Darkel_'+name+'('+args+');\n' for ret,name,args in SIGNATURES)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

LEAVES=[
('bool','Playback','void','return CReplay::IsPlayingBack();'),
('void','JustifyOff','void','CFont::SetJustifyOff();'),('void','BackgroundOff','void','CFont::SetBackgroundOff();'),('void','CentreSize','float size','CFont::SetCentreSize(size);'),('void','CentreOn','void','CFont::SetCentreOn();'),('void','PropOn','void','CFont::SetPropOn();'),('void','Scale','float x,float y','CFont::SetScale(x,y);'),('void','Color','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetColor(CRGBA(r,g,b,a));'),('void','FontStyle','int16_t style','CFont::SetFontStyle(style);'),('void','Print','float x,float y,uint16_t *text','CFont::PrintString(x,y,text);'),('void','CentreOff','void','CFont::SetCentreOff();'),('void','RightJustifyOn','void','CFont::SetRightJustifyOn();'),
('char *','String','void','return gString;'),('uint16_t *','Unicode','void','return gUString;'),('void','AsciiToUnicode','const char *source,uint16_t *target','AsciiToUnicode(source,target);'),
('int32_t','VehicleModel','CVehicle *vehicle','return vehicle->GetModelIndex();'),('int32_t','VehicleAppearance','CVehicle *vehicle','return vehicle->GetVehicleAppearance();'),('int32_t','PedModel','CPed *ped','return ped->GetModelIndex();'),('bool','ChrisCriminal','CPed *ped','return ped->bChrisCriminal;'),('uint32_t','PedType','CPed *ped','return ped->m_nPedType;'),
('int32_t *','CarsExploded','void','return &CStats::CarsExploded;'),('int32_t *','HelisDestroyed','void','return &CStats::HelisDestroyed;'),('int32_t *','BoatsExploded','void','return &CStats::BoatsExploded;'),('int32_t *','PeopleKilledByPlayer','void','return &CStats::PeopleKilledByPlayer;'),('int32_t *','PedsKilledOfThisType','void','return CStats::PedsKilledOfThisType;'),('int32_t *','HeadsPopped','void','return &CStats::HeadsPopped;'),('int32_t *','KillsSinceLastCheckpoint','void','return &CStats::KillsSinceLastCheckpoint;'),('int32_t *','PeopleKilledByOthers','void','return &CStats::PeopleKilledByOthers;'),('int32_t *','RandomPedType','void','return &CPopulation::m_AllRandomPedsThisType;'),
('void','ClearShortCut','void','CGameLogic::ClearShortCut();'),('void','RemoveDropOff','void','CGameLogic::RemoveShortCutDropOffPointForMission();'),
('CPlayerPed *','Player','void','return FindPlayerPed();'),('CVehicle *','PlayerVehicle','void','return FindPlayerVehicle();'),
('CWeapon *','CurrentWeapon','CPlayerPed *player','return player->GetWeapon();'),('CWeapon *','WeaponAt','CPlayerPed *player,uint8_t slot','return &player->GetWeapon(slot);'),('int32_t','WeaponSlot','CPlayerPed *player,enum eWeaponType type','return player->GetWeaponSlot(type);'),
('enum eWeaponType','WeaponType','CWeapon *weapon','return weapon->m_eWeaponType;'),('int32_t','AmmoTotal','CWeapon *weapon','return weapon->m_nAmmoTotal;'),
('void','RemoveWeaponAnims','CPlayerPed *player,int32_t type,float delta','player->RemoveWeaponAnims(type,delta);'),('void','GiveWeapon','CPlayerPed *player,enum eWeaponType type,uint32_t ammo,bool unused','player->GiveWeapon(type,ammo,unused);'),('void','CurrentType','CPlayerPed *player,enum eWeaponType type','player->SetCurrentWeapon(type);'),('void','CurrentSlot','CPlayerPed *player,int32_t slot','player->SetCurrentWeapon(slot);'),('int8_t','SelectedSlot','CPlayerPed *player','return player->m_nSelectedWepSlot;'),('uint8_t','CurrentSlotIndex','CPlayerPed *player','return player->m_currentWeapon;'),('void','MakeChanges','CPlayerPed *player,int32_t slot','player->MakeChangesForNewWeapon(slot);'),('void','SetAmmo','CPlayerPed *player,enum eWeaponType type,uint32_t ammo','player->SetAmmo(type,ammo);'),('void','ClearTarget','CPlayerPed *player','player->ClearWeaponTarget();'),
('CBaseModelInfo *','ModelInfo','int32_t model','return CModelInfo::GetModelInfo(model);'),('void','AddRef','CBaseModelInfo *model','model->AddRef();'),('void','RemoveRef','CBaseModelInfo *model','model->RemoveRef();'),('void','RemoveModel','CPlayerPed *player,int32_t model','player->RemoveWeaponModel(model);'),
('void','SetWeaponType','CWeapon *weapon,enum eWeaponType type','weapon->m_eWeaponType=type;'),('void','SetAmmoTotal','CWeapon *weapon,int32_t ammo','weapon->m_nAmmoTotal=ammo;'),('void','SetAmmoInClip','CWeapon *weapon,int32_t ammo','weapon->m_nAmmoInClip=ammo;'),('void','SetWeaponState','CWeapon *weapon,enum eWeaponState state','weapon->m_eWeaponState=state;'),('void','SetSelectedSlot','CPlayerPed *player,uint32_t slot','player->m_nSelectedWepSlot=slot;'),('void','RemoveDriveby','CPlayerPed *player','player->RemoveDrivebyAnims();'),('void','FrenzyPassed','void','CStats::AnotherKillFrenzyPassed();'),('CWanted *','Wanted','CPlayerPed *player','return player->m_pWanted;')]
LEAVES.append(('void','RemoveSlotModel','int32_t slot','FindPlayerPed()->RemoveWeaponModel(FindPlayerPed()->GetWeapon(slot).GetInfo()->m_nModelId);'))
BACKEND='//+ rouz edit (ChatGPT)\n#pragma once\n#include "Darkel.h"\n#include "Wanted.h"\n#include "WeaponInfo.h"\n#ifdef __cplusplus\nclass CPlayerPed;class CWeapon;class CBaseModelInfo;\nextern "C" {\n#else\ntypedef struct CPlayerPed CPlayerPed;typedef struct CWeapon CWeapon;typedef struct CBaseModelInfo CBaseModelInfo;\n#endif\n'+''.join(ret+' Darkel_Backend'+name+'('+args+');\n' for ret,name,args,code in LEAVES)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def host():
    # Keep only typed native primitives outside the complete C rampage policies
    return '//+ rouz edit (ChatGPT)\n#include "common.h"\n#include "Darkel_backend.h"\n#include "main.h"\n#include "PlayerPed.h"\n#include "Weapon.h"\n#include "Vehicle.h"\n#include "Replay.h"\n#include "Population.h"\n#include "GameLogic.h"\n#include "Stats.h"\n#include "Font.h"\n'+ '\n'.join(ret+' Darkel_Backend'+name+'('+args+')\n{\n    // Access the original typed native primitive at its original C policy access point\n    '+code+'\n}\n' for ret,name,args,code in LEAVES)+'//- rouz edit (ChatGPT)\n'

def weapon_fields(source,field,leaf):
    # Match one complete balanced weapon expression without consuming another argument's later field access
    for match in reversed(list(re.finditer(r'Darkel_Backend(?:WeaponAt|CurrentWeapon)\(',source))):
        end,args=arguments(source,match.end()-1)
        suffix=re.match(r'(?:\.|->)'+field+r'\b',source[end:])
        if suffix:source=source[:match.start()]+'Darkel_Backend'+leaf+'('+source[match.start():end]+')'+source[end+suffix.end():]
    return source

def kernel(source,originalHeader):
    # Preserve every complete original operation with exact function-name boundaries
    chunks=[]
    for ret,name,args in SIGNATURES:
        text=body(source,'CDarkel',name+'(');assert 'CDarkel::'+name+'(' in text
        opening=text.index('\n{');text=ret+' Darkel_'+name+'('+args+')'+text[opening:]
        text=text.replace('CDarkel::','Darkel_')
        for kind,star,field,array in fields(originalHeader):text=re.sub(r'(?<![\w])'+field+r'\b','Darkel_'+field,text)
        for operation in OPERATIONS:text=re.sub(r'(?<![\w])'+operation+r'\(', 'Darkel_'+operation+'(',text)
        for old,new in [('CReplay::IsPlayingBack()','Darkel_BackendPlayback()'),('CFont::SetJustifyOff(','Darkel_BackendJustifyOff('),('CFont::SetBackgroundOff(','Darkel_BackendBackgroundOff('),('CFont::SetCentreSize(','Darkel_BackendCentreSize('),('CFont::SetCentreOn(','Darkel_BackendCentreOn('),('CFont::SetPropOn(','Darkel_BackendPropOn('),('CFont::SetScale(','Darkel_BackendScale('),('CFont::SetFontStyle(','Darkel_BackendFontStyle('),('CFont::PrintString(','Darkel_BackendPrint('),('CFont::SetCentreOff(','Darkel_BackendCentreOff('),('CFont::SetRightJustifyOn(','Darkel_BackendRightJustifyOn(')]:text=text.replace(old,new)
        text=re.sub(r'CFont::SetColor\(CRGBA\(([^\n]*)\)\);',r'Darkel_BackendColor(\1);',text)
        text=re.sub(r'\bgString\b','Darkel_BackendString()',text);text=re.sub(r'\bgUString\b','Darkel_BackendUnicode()',text);text=text.replace('AsciiToUnicode(','Darkel_BackendAsciiToUnicode(')
        text=text.replace('vehicle->GetModelIndex()','Darkel_BackendVehicleModel(vehicle)').replace('vehicle->GetVehicleAppearance()','Darkel_BackendVehicleAppearance(vehicle)').replace('victim->GetModelIndex()','Darkel_BackendPedModel(victim)').replace('victim->bChrisCriminal','Darkel_BackendChrisCriminal(victim)').replace('victim->m_nPedType','Darkel_BackendPedType(victim)')
        for field in ['CarsExploded','HelisDestroyed','BoatsExploded','PeopleKilledByPlayer','PedsKilledOfThisType','HeadsPopped','KillsSinceLastCheckpoint','PeopleKilledByOthers']:text=text.replace('CStats::'+field,'(*Darkel_Backend'+field+'())' if field!='PedsKilledOfThisType' else 'Darkel_BackendPedsKilledOfThisType()')
        text=text.replace('CPopulation::m_AllRandomPedsThisType','(*Darkel_BackendRandomPedType())').replace('CStats::AnotherKillFrenzyPassed()','Darkel_BackendFrenzyPassed()').replace('CGameLogic::ClearShortCut()','Darkel_BackendClearShortCut()').replace('CGameLogic::RemoveShortCutDropOffPointForMission()','Darkel_BackendRemoveDropOff()')
        text=text.replace('FindPlayerPed()->m_pWanted','Darkel_BackendWanted(Darkel_BackendPlayer())')
        text=text.replace('player->GetWeapon(player->GetWeaponSlot(fixedWeapon))','Darkel_BackendWeaponAt(player,Darkel_BackendWeaponSlot(player,fixedWeapon))')
        text=text.replace('FindPlayerPed()->GetWeapon(slot)','Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot)').replace('FindPlayerPed()->GetWeapon(WEAPONSLOT_SUBMACHINEGUN)','Darkel_BackendWeaponAt(Darkel_BackendPlayer(),WEAPONSLOT_SUBMACHINEGUN)').replace('player->GetWeapon()','Darkel_BackendCurrentWeapon(player)')
        text=weapon_fields(text,'m_eWeaponType','WeaponType')
        text=weapon_fields(text,'m_nAmmoTotal','AmmoTotal')
        text=text.replace('Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot).GetInfo()','Darkel_BackendWeaponInfo(Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot))')
        text=text.replace('CModelInfo::GetModelInfo(','Darkel_BackendModelInfo(')
        text=re.sub(r'(?m)^(\s*)(Darkel_BackendModelInfo\([^\n;]+\))->(AddRef|RemoveRef)\(\);',r'\1Darkel_Backend\3(\2);',text)
        text=text.replace('player->RemoveWeaponAnims(','Darkel_BackendRemoveWeaponAnims(player,').replace('FindPlayerPed()->RemoveWeaponAnims(','Darkel_BackendRemoveWeaponAnims(Darkel_BackendPlayer(),')
        text=text.replace('player->GiveWeapon(fixedWeapon, 30000)','Darkel_BackendGiveWeapon(player,fixedWeapon,30000,true)').replace('player->GiveWeapon((eWeaponType)Darkel_InterruptedWeaponType, Darkel_AmmoInterruptedWeapon, true)','Darkel_BackendGiveWeapon(player,(eWeaponType)Darkel_InterruptedWeaponType,Darkel_AmmoInterruptedWeapon,true)')
        text=text.replace('player->SetCurrentWeapon(fixedWeapon)','Darkel_BackendCurrentType(player,fixedWeapon)').replace('player->SetCurrentWeapon(FindPlayerPed()->m_nSelectedWepSlot)','Darkel_BackendCurrentSlot(player,Darkel_BackendSelectedSlot(Darkel_BackendPlayer()))')
        text=text.replace('player->MakeChangesForNewWeapon(player->m_nSelectedWepSlot)','Darkel_BackendMakeChanges(player,Darkel_BackendSelectedSlot(player))').replace('player->MakeChangesForNewWeapon(player->m_currentWeapon)','Darkel_BackendMakeChanges(player,Darkel_BackendCurrentSlotIndex(player))')
        text=text.replace('player->SetAmmo(','Darkel_BackendSetAmmo(player,').replace('player->ClearWeaponTarget()','Darkel_BackendClearTarget(player)')
        text=text.replace('FindPlayerPed()->RemoveWeaponModel(','Darkel_BackendRemoveModel(Darkel_BackendPlayer(),').replace('player->RemoveWeaponModel(','Darkel_BackendRemoveModel(player,')
        text=text.replace('Darkel_BackendRemoveModel(Darkel_BackendPlayer(),Darkel_BackendWeaponInfo(Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot))->m_nModelId);','Darkel_BackendRemoveSlotModel(slot);')
        for field,leaf,value in [('m_eWeaponType','SetWeaponType','WEAPONTYPE_UNARMED'),('m_nAmmoTotal','SetAmmoTotal','0'),('m_nAmmoInClip','SetAmmoInClip','0'),('m_eWeaponState','SetWeaponState','WEAPONSTATE_READY')]:
            target='Darkel_BackendWeaponAt(Darkel_BackendPlayer(),slot)';needle=target+'.'+field+' = '+value+';'
            if field=='m_eWeaponType':needle='Darkel_BackendWeaponType('+target+') = '+value+';'
            if field=='m_nAmmoTotal':needle='Darkel_BackendAmmoTotal('+target+') = '+value+';'
            text=text.replace(needle,'Darkel_Backend'+leaf+'('+target+','+value+');')
        text=text.replace('player->m_nSelectedWepSlot = WeaponInfo_GetWeaponInfo((eWeaponType)Darkel_InterruptedWeaponSelected)->m_nWeaponSlot;','Darkel_BackendSetSelectedSlot(player,WeaponInfo_GetWeaponInfo((eWeaponType)Darkel_InterruptedWeaponSelected)->m_nWeaponSlot);')
        for value in ['WEAPONSLOT_SUBMACHINEGUN','WEAPONSLOT_UNARMED']:text=text.replace('FindPlayerPed()->m_nSelectedWepSlot = '+value+';','Darkel_BackendSetSelectedSlot(Darkel_BackendPlayer(),'+value+');')
        text=text.replace('player->RemoveDrivebyAnims()','Darkel_BackendRemoveDriveby(player)').replace('FindPlayerPed()','Darkel_BackendPlayer()').replace('FindPlayerVehicle()','Darkel_BackendPlayerVehicle()')
        text=scalar(text).replace('\n{','\n{\n    // Preserve the complete original '+name+' policy and each ordered external access',1);chunks.append(text)
    globals=''.join(kind+' '+star+'Darkel_'+name+array+';\n' for kind,star,name,array in fields(originalHeader))
    return '''//+ rouz edit (ChatGPT)
#include "Darkel_backend.h"
#include "Timer.h"
#include "Text.h"
#include "DMAudio.h"
#include "Draw.h"
#include "Draw_backend.h"
#include "Draw_constants.h"
#include "FontStyles.h"
#include "PedType.h"
#include "VehicleAppearance.h"
#include <stdio.h>
#define FRENZY_ANY_PED -1
#define FRENZY_ANY_CAR -2
#define Min(a,b) ((a)<(b)?(a):(b))
#define SCREEN_WIDTH (Draw_BackendScreenWidth())
#define SCREEN_HEIGHT (Draw_BackendScreenHeight())
#define SCREEN_STRETCH_X(a) ((a)*(float)SCREEN_WIDTH/DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a) ((a)*(float)SCREEN_HEIGHT/DEFAULT_SCREEN_HEIGHT)
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_RIGHT(a) (SCREEN_WIDTH-SCREEN_SCALE_X(a))
#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a)*DEFAULT_ASPECT_RATIO/Draw_GetAspectRatio())
#if defined(PROPER_SCALING) && !defined(FORCE_PC_SCALING)
#undef SCREEN_SCALE_Y
#define SCREEN_SCALE_Y(a) Draw_ScaleY(SCREEN_STRETCH_Y(a))
#endif
#else
#define SCREEN_SCALE_AR(a) (a)
#endif
'''+globals+'\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def darkel_calls(source):
    # Remove the complete static class and make the original default headshot argument explicit at every caller
    if 'class CDarkel\n' in source:return header(source)
    if 'int32 CDarkel::TimeLimit;' in source:
        if OBSERVER:OBSERVER(source)
        return host()
    original=source
    for match in reversed(list(re.finditer(r'CDarkel::RegisterKillByPlayer\s*\(',source))):
        end,args=arguments(source,match.end()-1)
        if len(args)==2:source=source[:match.start()]+'Darkel_RegisterKillByPlayer('+','.join(args)+',false)'+source[end:]
        else:assert len(args)==3
    source=source.replace('CDarkel::','Darkel_')
    if source==original:return source
    if original.lstrip().startswith('//+ rouz edit (ChatGPT)') and original.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def darkel_project(source):
    # Register the uniquely named C rampage object and its typed native boundary header
    for tag,old,new in [('ClCompile','Darkel.cpp','RampageSystem.c'),('ClInclude','Darkel.h','Darkel_backend.h')]:
        entry='<'+tag+' Include="..\\src\\control\\'+old+'" />';addition='<'+tag+' Include="..\\src\\control\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\control\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\control\\'+re.escape(old)+r'">\n\s*<Filter>control</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove full relocation of the historical rampage producer and its complete canonical state
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/Darkel.cpp').read_text())
    header=(root/'.diagnostics/darkel-c-tests/before/src/control/Darkel.h').read_text()
    assert normalize(kernel(captured[0],header))==normalize((root/'src/control/RampageSystem.c').read_text())

def verify_dependency(saved,current):
    # Permit only the entire explicit rampage caller projection through existing dependency guards
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Preserve the complete rampage projection before relocating the complete statistics owner
    import stats_migration as stats
    return stats.calls(darkel_calls(source))

def project(source):
    # Preserve previous registrations before adding the uniquely named statistics C object
    import stats_migration as stats
    return stats.project(darkel_project(source))
