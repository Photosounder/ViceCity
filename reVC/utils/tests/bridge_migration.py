"""Move the entire bridge controller, canonical state and pool decisions into C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import traffic_lights_migration as traffic
METHODS=['Init','Update','ShouldLightsBeFlashing','FindBridgeEntities','ThisIsABridgeObjectMovingUp']
FIELDS=[('CEntity *','pLiftRoad'),('CEntity *','pLiftPart'),('CEntity *','pWeight'),('int32_t','State'),('int32_t','OldState'),('float','DefaultZLiftPart'),('float','DefaultZLiftRoad'),('float','DefaultZLiftWeight'),('float','OldLift'),('uint32_t','TimeOfBridgeBecomingOperational')]
OBSERVER=None
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "BridgeStates.h"
#ifdef __cplusplus
class CEntity;
extern "C" {
#else
typedef struct CEntity CEntity;
#endif
'''+''.join('extern '+kind+' Bridge_'+name+';\n' for kind,name in FIELDS)+'''void Bridge_Init(void);
void Bridge_Update(void);
bool Bridge_ShouldLightsBeFlashing(void);
void Bridge_FindBridgeEntities(void);
bool Bridge_ThisIsABridgeObjectMovingUp(int32_t index);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BOUNDARIES=[
('CPool *','BuildingPool','void','return CPools::GetBuildingPool();'),
('CEntity *','Building','void *slot','return (CBuilding *)slot;'),
('int32_t','Model','CEntity *entity','return entity->GetModelIndex();'),
('int32_t','LiftModel','void','return MI_BRIDGELIFT;'),
('int32_t','RoadModel','void','return MI_BRIDGEROADSEGMENT;'),
('int32_t','WeightModel','void','return MI_BRIDGEWEIGHT;'),
('float','PositionZ','CEntity *entity','return entity->GetPosition().z;'),
('void','SetPositionZ','CEntity *entity,float value','entity->GetMatrix().GetPosition().z=value;'),
('void','UpdateMatrix','CEntity *entity','entity->GetMatrix().UpdateRW();'),
('void','UpdateFrame','CEntity *entity','entity->UpdateRwFrame();'),
('int32_t','CommercialPassed','void','return CStats::CommercialPassed;'),
('void','SetLights','float x1,float x2,float y1,float y2,bool enabled','ThePaths.SetLinksBridgeLights(x1,x2,y1,y2,enabled);')]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "Bridge.h"
#include "Pool.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef GTA_BRIDGE
'''+''.join(kind+' Bridge_Backend'+name+'('+args+');\n' for kind,name,args,body in BOUNDARIES)+'''#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Bridge_backend.h"
#ifdef GTA_BRIDGE
#include "Pools.h"
#include "ModelIndices.h"
#include "PathFind.h"
#include "Stats.h"
'''+''.join(kind+' Bridge_Backend'+name+'('+args+')\n{\n    // Enter the original typed native field or action at the C decision point\n    '+body+'\n}\n' for kind,name,args,body in BOUNDARIES)+'''#endif
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Transform every complete original operation while retaining feature gates and live access order
    chunks=[]
    for name in METHODS:
        text=function(source,'CBridge::',name)
        text=re.sub(r'(?m)^\s*//[+-] rouz edit \(ChatGPT\)\n','\n',text).replace(' // rouz edit (ChatGPT)','')
        text=text.replace('CBridge::','Bridge_').replace('nil','NULL')
        text=text.replace('CPools::GetBuildingPool()','Bridge_BackendBuildingPool()')
        text=text.replace('CBuilding* entry = ((CBuilding*)CPool_GetSlot(Bridge_BackendBuildingPool(), i));','CEntity *entry = Bridge_BackendBuilding(CPool_GetSlot(Bridge_BackendBuildingPool(),i));')
        for field in ['pLiftRoad','pLiftPart','pWeight']:
            text=text.replace(field+'->GetPosition().z','Bridge_BackendPositionZ('+field+')')
            text=re.sub(re.escape(field)+r'->GetMatrix\(\).GetPosition\(\).z = ([^;]+);',lambda match:'Bridge_BackendSetPositionZ('+field+','+match[1]+');',text)
            text=text.replace(field+'->GetMatrix().UpdateRW()','Bridge_BackendUpdateMatrix('+field+')').replace(field+'->UpdateRwFrame()','Bridge_BackendUpdateFrame('+field+')')
        text=text.replace('entry->GetModelIndex()','Bridge_BackendModel(entry)')
        for model,target in [('MI_BRIDGELIFT','LiftModel'),('MI_BRIDGEROADSEGMENT','RoadModel'),('MI_BRIDGEWEIGHT','WeightModel')]:text=text.replace(model,'Bridge_Backend'+target+'()')
        text=text.replace('CStats::CommercialPassed','Bridge_BackendCommercialPassed()').replace('ThePaths.SetLinksBridgeLights(','Bridge_BackendSetLights(')
        for method in METHODS:text=re.sub(r'(?<![\w])'+method+r'\(', 'Bridge_'+method+'(',text)
        for kind,field in FIELDS:text=re.sub(r'(?<![\w])\b'+field+r'\b','Bridge_'+field,text)
        text=text.replace('uint16','uint16_t').replace('int index','int32_t index')
        if name!='ThisIsABridgeObjectMovingUp':text=text.replace('Bridge_'+name+'()','Bridge_'+name+'(void)',1)
        text=text.replace('\n{','\n{\n    // Preserve the complete original '+name+' operation and ordered live accesses',1)
        chunks.append(text)
    return '''//+ rouz edit (ChatGPT)
#include "Bridge_backend.h"
#include "Timer.h"
#include <stddef.h>
'''+''.join(kind+' Bridge_'+field+';\n' for kind,field in FIELDS)+'\n'+'\n\n'.join(chunks)+'\n//- rouz edit (ChatGPT)\n'

def bridge_calls(source):
    # Remove the static class and enter the C operations or canonical fields from every complete caller
    if 'class CBridge\n' in source:return HEADER
    if re.search(r'(?m)^CEntity \*CBridge::pLiftRoad;',source):
        if OBSERVER:OBSERVER(source)
        return HOST
    updated=source.replace('CBridge::','Bridge_')
    if updated==source:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return updated
    return '//+ rouz edit (ChatGPT)\n'+updated.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Forward historical owners through the separately verified traffic-light class removal
    return traffic.calls(bridge_calls(source))

def project(source):
    # Register a distinct C11 controller beside the remaining typed native operations
    for tag,old,new in [('ClCompile','Bridge.cpp','BridgeControl.c'),('ClInclude','Bridge.h','Bridge_backend.h')]:
        entry='<'+tag+' Include="..\\src\\control\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\control\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\control\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\control\\'+re.escape(old)+r'">\n\s*<Filter>control</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return traffic.project(source)

def legacy_field_spelling(source):
    # Restore field names before an older kernel translator binds each complete original native access
    for kind,name in FIELDS:source=re.sub(r'\bBridge_'+name+r'\b','CBridge::'+name,source)
    return source

def verify_legacy(source,transform,root):
    # Prove the complete relocated bridge controller and native actions from earlier immutable sources
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/control/Bridge.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/control/BridgeControl.c').read_text())

def verify_dependency(saved,current):
    # Retain byte guards unless the entire dependency follows this exact class removal
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original)
    assert expected!=original and expected==current.read_text(),str(current)
