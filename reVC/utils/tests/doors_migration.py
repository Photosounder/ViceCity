"""Replace complete car and train door owners with explicit C state and native geometry leaves."""
import re
import object_names_migration as objects
CAR=['Open','Process','RetAngleWhenClosed','RetAngleWhenOpen','GetAngleOpenRatio','IsFullyOpen','IsClosed']
TRAIN=['Open','RetTranslationWhenClosed','RetTranslationWhenOpen','IsFullyOpen','IsClosed']
CAR_INIT='''
    // Initialize every door at the former member-constructor boundary
    for(int doorIndex=0;doorIndex<VEHICLE_DOOR_COUNT;++doorIndex)
        Door_Init(&Doors[doorIndex]);'''
BOAT_INIT='''
    // Initialize the boom at the former door-constructor boundary
    Door_Init(&m_boom);'''
TRAIN_INIT='''
    // Initialize both doors even when the optional train body is disabled
    for(int doorIndex=0;doorIndex<2;++doorIndex)
        TrainDoor_Init(&Doors[doorIndex]);'''

def method(source,name):
    # Retain complete nested bodies and their original declaration line
    marker=source.index(name+'(');start=source.rfind('\n',0,marker)+1
    if not source[start:marker].strip():start=source.rfind('\n',0,start-1)+1
    opening=source.index('{',marker);depth=1;end=opening+1
    while depth:
        if source[end]=='{':depth+=1
        elif source[end]=='}':depth-=1
        end+=1
    return source[start:end]

def header(original):
    # Keep the original field ordering with a plain three-float velocity record
    source='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "DoorTypes.h"
typedef struct DoorVector {float x,y,z;} DoorVector;
'''
    for owner,start,end in [('CDoor','\tfloat m_fMaxAngle;','\n\tCDoor(void);'),('CTrainDoor','\tfloat m_fClosedPosn;','\n\tCTrainDoor(void);')]:
        fields=original[original.index(start):original.index(end)].replace('int8 ','int8_t ').replace('CVector ','DoorVector ')
        source+='typedef struct '+owner+' {\n'+fields+'\n} '+owner+';\n'
    source+='''#ifdef __cplusplus
class CVehicle;
extern "C" {
#else
typedef struct CVehicle CVehicle;
#endif
void Door_Init(CDoor *self);
void Door_SetLimits(CDoor *self,float minAngle,float maxAngle,int8_t dir,int8_t axis);
void Door_Open(CDoor *self,float ratio);
void Door_Process(CDoor *self,CVehicle *vehicle);
float Door_RetAngleWhenClosed(CDoor *self);
float Door_RetAngleWhenOpen(CDoor *self);
float Door_GetAngleOpenRatio(CDoor *self);
bool Door_IsFullyOpen(CDoor *self);
bool Door_IsClosed(CDoor *self);
void TrainDoor_Init(CTrainDoor *self);
void TrainDoor_SetLimits(CTrainDoor *self,float open,float closed,int8_t dir,int8_t axis);
void TrainDoor_Open(CTrainDoor *self,float ratio);
float TrainDoor_RetTranslationWhenClosed(CTrainDoor *self);
float TrainDoor_RetTranslationWhenOpen(CTrainDoor *self);
bool TrainDoor_IsFullyOpen(CTrainDoor *self);
bool TrainDoor_IsClosed(CTrainDoor *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return source

BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "Door.h"
#ifdef __cplusplus
extern "C" {
#endif
DoorVector Door_BackendSpeed(CVehicle *vehicle,const DoorVector *offset);
DoorVector Door_BackendToLocal(CVehicle *vehicle,const DoorVector *difference);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Vehicle.h"
#include "Door.h"
#include "Door_backend.h"

DoorVector Door_BackendSpeed(CVehicle *vehicle,const DoorVector *offset)
{
    // Read the original physical-point velocity through the existing vehicle operation
    CVector speed=vehicle->GetSpeed(CVector(offset->x,offset->y,offset->z));
    DoorVector result={speed.x,speed.y,speed.z};
    return result;
}

DoorVector Door_BackendToLocal(CVehicle *vehicle,const DoorVector *difference)
{
    // Retain the original vector-first matrix multiply and its coordinate convention
    CVector local=Multiply3x3(CVector(difference->x,difference->y,difference->z),vehicle->GetMatrix());
    DoorVector result={local.x,local.y,local.z};
    return result;
}
//- rouz edit (ChatGPT)
'''

def kernel(original,originalHeader):
    # Relocate both complete owners while keeping the geometry policy and all ordered state changes in C
    source='''//+ rouz edit (ChatGPT)
#include "Door.h"
#include "Door_backend.h"
#include <string.h>
#include <math.h>
#define Abs(value) fabsf(value)
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))

'''
    for owner,prefix,operations in [('CDoor','Door',CAR),('CTrainDoor','TrainDoor',TRAIN)]:
        definition=method(original,owner+'::'+owner);body=definition[definition.index('{')+1:-1].replace('this','self')
        source+='void\n'+prefix+'_Init('+owner+' *self)\n{\n    // Preserve the original full-record zeroing including padding'+body+'}\n\n'
        definition=method(originalHeader,'void Init') if owner=='CDoor' else method(originalHeader[originalHeader.index('class CTrainDoor'):],'void Init')
        body=definition[definition.index('{')+1:-1]
        for name in re.findall(r'\b(m_\w+)\s*=',body):body=re.sub(r'\b'+name+r'\b','self->'+name,body)
        args='float minAngle,float maxAngle,int8_t dir,int8_t axis' if owner=='CDoor' else 'float open,float closed,int8_t dir,int8_t axis'
        source+='void\n'+prefix+'_SetLimits('+owner+' *self,'+args+')\n{\n    // Change only the original limit and orientation fields'+body+'}\n\n'
        for name in operations:
            definition=method(original,owner+'::'+name);result=definition[:definition.index(owner+'::')].strip();body=definition[definition.index('{')+1:-1]
            body=re.sub(r'(?<![\w>.])m_(\w+)\b',r'self->m_\1',body)
            for called in operations:body=re.sub(r'(?<![\w_])'+called+r'\(\)',prefix+'_'+called+'(self)',body)
            if name=='Process':
                body=body.replace('static CVector vecOffset(1.0f, 0.0f, 0.0f);','static DoorVector vecOffset={1.0f,0.0f,0.0f};')
                body=body.replace('CVector speed = vehicle->GetSpeed(vecOffset);','DoorVector speed = Door_BackendSpeed(vehicle,&vecOffset);')
                body=body.replace('CVector vecSpeedDiff = speed - self->m_vecSpeed;','DoorVector vecSpeedDiff={speed.x-self->m_vecSpeed.x,speed.y-self->m_vecSpeed.y,speed.z-self->m_vecSpeed.z};')
                body=body.replace('Multiply3x3(vecSpeedDiff, vehicle->GetMatrix())','Door_BackendToLocal(vehicle,&vecSpeedDiff)')
            args=',float ratio' if name=='Open' else ',CVehicle *vehicle' if name=='Process' else ''
            source+=result+'\n'+prefix+'_'+name+'('+owner+' *self'+args+')\n{\n    // Preserve the complete original door operation and its ordered state updates'+body+'}\n\n'
    return source+'//- rouz edit (ChatGPT)\n'

def doors_calls(source):
    # Replace complete owners and project the exact known receiver types and constructor sites
    if 'class CDoor\n' in source and 'class CTrainDoor\n' in source:return header(source)
    if 'CDoor::CDoor(void)' in source:return HOST
    original=source
    for marker,block in [('CAutomobile::CAutomobile(int32 id, uint8 CreatedBy)\n : CVehicle(CreatedBy)\n{',CAR_INIT),('CBoat::CBoat(int mi, uint8 owner) : CVehicle(owner)\n{',BOAT_INIT),('CTrain::CTrain(int32 id, uint8 CreatedBy)\n : CVehicle(CreatedBy)\n{',TRAIN_INIT)]:
        if marker in source and block not in source:source=source.replace(marker,marker+block,1)
    trainOwner='CTrain::CTrain(' in source
    receiver=r'(\(\(CTrain\*\)train\)->Doors\[[^\]]+\]|target->Doors\[[^\]]+\]|(?:car->)?Doors\[[^\]]+\]|m_boom)'
    operations='Init|'+ '|'.join(sorted(set(CAR+TRAIN)))
    def convert(match):
        # Select the concrete record type before converting the old member-call argument list
        value,name=match[1],match[2];prefix='TrainDoor' if trainOwner and value!='m_boom' or value.startswith(('((CTrain*)train)','target->Doors')) else 'Door'
        name='SetLimits' if name=='Init' else name
        return prefix+'_'+name+'(&'+value+(', ' if name in ['SetLimits','Open','Process'] else '')
    source=re.sub(receiver+r'\.('+operations+r')\(',convert,source)
    for owner in ['CDoor','CTrainDoor']:source=source.replace('class '+owner+';','struct '+owner+';')
    if source==original:return source
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def doors_project(source):
    # Register the separate C producer while retaining the two typed native geometry leaves
    for tag,old,new in [('ClCompile','Door.cpp','DoorSystem.c'),('ClInclude','Door.h','Door_backend.h')]:
        entry='<'+tag+' Include="..\\src\\vehicles\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\vehicles\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\vehicles\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\vehicles\\'+re.escape(old)+r'">\n\s*<Filter>vehicles</Filter>\n\s*</'+tag+'>)';source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)


def calls(source):
    # Preserve both complete door owners before the C damage owner and its typed callers
    import damage_migration as damage
    return damage.calls(doors_calls(source))

def project(source):
    # Retain prior registrations before the complete damage producer rename to C11
    import damage_migration as damage
    return damage.project(doors_project(source))
