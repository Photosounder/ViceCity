"""Move all eleven scenery controller/list/entity algorithms and complete records into C11."""
import re
import scroll_bar_migration as scroll_bar
MODELS=['LIGHTBEAM','AIRPORTRADAR','MALLFAN','HOTELFAN_NIGHT','HOTELFAN_DAY','HOTROOMFAN','BLIMP_NIGHT','BLIMP_DAY']
TYPES='''#define NUMMOVINGTHINGS 48
typedef struct MovingThingVector { float x,y,z; } MovingThingVector;
typedef struct MovingThing {
    struct MovingThing *m_pNext,*m_pPrev;
    int16_t m_nType,m_farAway;
    MovingThingVector m_vecPosn;
    CEntity *m_pEntity;
} MovingThing;
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CEntity;
#else
typedef struct CEntity CEntity;
#endif
'''+TYPES+'''#ifdef __cplusplus
extern "C" {
#endif
extern MovingThing MovingThings_StartCloseList,MovingThings_EndCloseList;
extern MovingThing MovingThings_aMovingThings[NUMMOVINGTHINGS];
extern int16_t MovingThings_Num;
void MovingThings_Init(void);
void MovingThings_Shutdown(void);
void MovingThings_Update(void);
void MovingThings_Render(void);
#ifdef REVC_SOFTWARE_POLYGONS
void MovingThings_RenderForEnvMap(void *camera);
#endif
void MovingThings_RegisterOne(CEntity *entity,uint16_t type);
void MovingThings_PossiblyAddThisEntity(CEntity *entity);
void MovingThing_Update(MovingThing *thing);
void MovingThing_AddToList(MovingThing *thing,MovingThing *after);
void MovingThing_RemoveFromList(MovingThing *thing);
int16_t MovingThing_SizeList(MovingThing *thing);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('int32_t','SectorsX','void','return NUMSECTORS_X;'),('int32_t','SectorsY','void','return NUMSECTORS_Y;'),
('void *','SectorFirst','int32_t x,int32_t y','return CWorld::GetSector(x,y)->m_lists[ENTITYLIST_BUILDINGS].first;'),
('void *','BigFirst','int32_t level','return CWorld::GetBigBuildingList((eLevelName)level).first;'),
('void *','NodeNext','void *node','return ((CPtrNode *)node)->next;'),('CEntity *','NodeEntity','void *node','return (CEntity *)((CPtrNode *)node)->item;'),
('int32_t','ScrollCount','void','return ARRAY_SIZE(aScrollBars);'),
('void','ScrollInit','int32_t index,MovingThingVector first,MovingThingVector last,uint8_t type,uint8_t r,uint8_t g,uint8_t b,float scale','aScrollBars[index].Init(CVector(first.x,first.y,first.z),CVector(last.x,last.y,last.z),type,r,g,b,scale);'),
('void','ScrollVisibility','int32_t index,bool visible','aScrollBars[index].SetVisibility(visible);'),
('bool','ScrollVisible','int32_t index','return aScrollBars[index].IsVisible();'),
('void','ScrollUpdate','int32_t index','aScrollBars[index].Update();'),('void','ScrollRender','int32_t index','aScrollBars[index].Render();'),
('void','ScrollEnv','int32_t index,void *camera','#ifdef REVC_SOFTWARE_POLYGONS\n    aScrollBars[index].RenderForEnvMap((RwCamera *)camera);\n#else\n    (void)index;(void)camera;\n#endif'),
('void','EscalatorsInit','void','CEscalators::Init();'),('void','EscalatorsShutdown','void','CEscalators::Shutdown();'),('void','EscalatorsUpdate','void','CEscalators::Update();'),
('MovingThingVector','Position','CEntity *entity','const CVector &v=entity->GetPosition();MovingThingVector result={v.x,v.y,v.z};return result;'),
('MovingThingVector','CameraPosition','void','const CVector &v=TheCamera.GetPosition();MovingThingVector result={v.x,v.y,v.z};return result;'),
('float','PositionAxis','CEntity *entity,int32_t axis','const CVector &v=entity->GetPosition();return axis==0?v.x:axis==1?v.y:v.z;'),
('float','CameraAxis','int32_t axis','const CVector &v=TheCamera.GetPosition();return axis==0?v.x:axis==1?v.y:v.z;'),
('void','SetRight','CEntity *entity,float x,float y,float z','entity->GetRight()=CVector(x,y,z);'),
('void','SetForward','CEntity *entity,float x,float y,float z','entity->GetForward()=CVector(x,y,z);'),
('void','SetUp','CEntity *entity,float x,float y,float z','entity->GetUp()=CVector(x,y,z);'),
('void','SetPosition','CEntity *entity,float x,float y,float z','entity->SetPosition(CVector(x,y,z));'),
('void','UpdateRW','CEntity *entity','entity->GetMatrix().UpdateRW();'),('void','UpdateFrame','CEntity *entity','entity->UpdateRwFrame();'),
('int16_t','ModelIndex','CEntity *entity','return entity->GetModelIndex();'),
('void','Corona','MovingThingVector position','CVector coors(position.x,position.y,position.z);CCoronas::RegisterCorona(43,128,128,100,255,coors,70.0f,600.0f,0.0f,CCoronas::TYPE_STAR,CCoronas::REFLECTION_OFF,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_OFF,0.0f);'),
('void','Push','const char *name','PUSH_RENDERGROUP(name);'),('void','Pop','void','POP_RENDERGROUP();')]
for name in MODELS:READERS.append(('int16_t','Model'+name,'void','return MI_'+name+';'))
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "MovingThings.h"
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' MovingThings_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='//+ rouz edit (ChatGPT)\n#include "MovingThings_backend.h"\n'+''.join(result+' MovingThings_Backend'+name+'('+args+')\n{\n    // Preserve the original typed owner operation and live native field access\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''static MovingThingVector MovingThings_Subtract(MovingThingVector a,MovingThingVector b)
{
    // Keep original per-component subtraction and vector value boundaries
    MovingThingVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static float MovingThings_Magnitude(MovingThingVector a)
{
    // Preserve the original square-root argument and left-associated sum order
    return sqrtf(a.x*a.x+a.y*a.y+a.z*a.z);
}
static MovingThingVector MovingThings_Divide(MovingThingVector a,float divisor)
{
    // Retain unguarded component division including the original coincident-position behavior
    MovingThingVector result={a.x/divisor,a.y/divisor,a.z/divisor};return result;
}
static MovingThingVector MovingThings_Scale(MovingThingVector a,float scale)
{
    // Preserve the original separate component multiplication before subtraction
    MovingThingVector result={a.x*scale,a.y*scale,a.z*scale};return result;
}
static float MovingThings_Dot(MovingThingVector a,MovingThingVector b)
{
    // Retain the original dot-product expression and float arithmetic order
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
'''
METHODS=[('CMovingThings',name) for name in ['Init','Shutdown','Update','Render','RenderForEnvMap','RegisterOne','PossiblyAddThisEntity']]+[('CMovingThing',name) for name in ['Update','AddToList','RemoveFromList','SizeList']]

def method(source,owner,name):
    # Extract untouched original functions with either opening-brace style
    match=re.search(r'(?m)^(?:void|int16)\s+'+owner+'::'+name+r'\([^\n]*\)\s*\{',source);assert match,(owner,name)
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]

def storage(source):
    # Include exactly the four original static controller definitions
    a=source.index('CMovingThing CMovingThings::StartCloseList;');b=source.index('CMovingThing CMovingThings::aMovingThings[NUMMOVINGTHINGS];',a)+len('CMovingThing CMovingThings::aMovingThings[NUMMOVINGTHINGS];');return source[a:b]

def distances(source):
    # Retain all five original mutable update-distance entries
    return re.search(r'static float maxUpdateDists\[5\] = \{[^\n]+;',source)[0]

def classes(source):
    # Delimit both complete original record/controller classes without changing scroll bars
    a=source.index('class CMovingThing\n');b=source.index('class CScrollBar\n',a);return source[a:b]

def calls(source):
    # Remove only the relocated definitions, algorithms and record/controller declarations
    if 'CMovingThing CMovingThings::StartCloseList;' in source:
        source=source.replace(storage(source),'').replace(distances(source),'')
        for owner,name in METHODS:source=source.replace(method(source,owner,name),'')
        source+='\n'+HOST
    if 'class CMovingThing\n' in source:source=source.replace(classes(source),'//+ rouz edit (ChatGPT)\n#include "MovingThings.h"\n//- rouz edit (ChatGPT)\n\n')
    lines=[]
    for line in source.splitlines(keepends=True):
        if line.lstrip().startswith('//'):lines.append(line);continue
        if 'CMovingThings::' in line:lines.append(re.match(r'\s*',line)[0]+'// Enter the C moving-scenery algorithm at the original call point\n')
        lines.append(line.replace('CMovingThings::','MovingThings_'))
    return scroll_bar.calls(''.join(lines))

def kernel(source):
    # Translate every complete original controller, entity and intrusive-list algorithm
    chunks=[]
    for owner,name in METHODS:
        text=method(source,owner,name)
        if name=='RenderForEnvMap':text='#ifdef REVC_SOFTWARE_POLYGONS\n'+text+'\n#endif'
        chunks.append(text)
    body=storage(source)+'\n'+distances(source)+'\n'+'\n\n'.join(chunks)
    body=body.replace('"CMovingThings::Render"','"@@GROUP@@"')
    for name in ['Init','Shutdown','Update','Render']:body=body.replace('CMovingThings::'+name+'()','MovingThings_'+name+'(void)')
    body=body.replace('CMovingThings::RenderForEnvMap(RwCamera *camera)','MovingThings_RenderForEnvMap(void *camera)').replace('CMovingThings::RegisterOne','MovingThings_RegisterOne').replace('CMovingThings::PossiblyAddThisEntity','MovingThings_PossiblyAddThisEntity')
    for name,args in [('Update',''),('AddToList','CMovingThing *pThing'),('RemoveFromList',''),('SizeList','')]:body=body.replace('CMovingThing::'+name+'('+args+')','MovingThing_'+name+'(MovingThing *thing'+(', '+args if args else '')+')')
    for name in ['StartCloseList','EndCloseList','Num','aMovingThings']:
        body=body.replace('CMovingThings::'+name,'MovingThings_'+name);body=re.sub(r'(?<![\w:])'+name+r'\b','MovingThings_'+name,body)
    body=body.replace('CMovingThing','MovingThing')
    for field in ['m_nType','m_farAway','m_pEntity','m_vecPosn','m_pNext','m_pPrev']:body=re.sub(r'(?<![.>\w])'+field+r'\b','thing->'+field,body)
    body=body.replace('MovingThings_aMovingThings[i].Update()','MovingThing_Update(&MovingThings_aMovingThings[i])').replace('MovingThings_aMovingThings[MovingThings_Num].AddToList(&MovingThings_StartCloseList)','MovingThing_AddToList(&MovingThings_aMovingThings[MovingThings_Num],&MovingThings_StartCloseList)')
    body=re.sub(r'(?<!\w)PossiblyAddThisEntity\(',r'MovingThings_PossiblyAddThisEntity(',body);body=re.sub(r'(?<!\w)RegisterOne\(',r'MovingThings_RegisterOne(',body)
    body=body.replace('AddToList(&MovingThings_StartCloseList)','MovingThing_AddToList(thing,&MovingThings_StartCloseList)').replace('RemoveFromList();','MovingThing_RemoveFromList(thing);').replace('= this;','= thing;')
    body=body.replace('CPtrNode *pNode = CWorld::GetSector(i, j)->m_lists[ENTITYLIST_BUILDINGS].first','void *pNode=MovingThings_BackendSectorFirst(i,j)').replace('CPtrNode *pNode = CWorld::GetBigBuildingList((eLevelName)i).first','void *pNode=MovingThings_BackendBigFirst(i)').replace('pNode = pNode->next','pNode=MovingThings_BackendNodeNext(pNode)').replace('(CEntity *)pNode->item','MovingThings_BackendNodeEntity(pNode)')
    body=body.replace('NUMSECTORS_X','MovingThings_BackendSectorsX()').replace('NUMSECTORS_Y','MovingThings_BackendSectorsY()')
    body=body.replace('aScrollBars[0].Init(CVector(-1069.209f, 1320.126f, 18.848f), CVector(-1069.209f, 1342.299f, 22.612f), SCROLL_ARENA_STRING, 128, 255, 0, 0.3f)','MovingThings_BackendScrollInit(0,(MovingThingVector){-1069.209f,1320.126f,18.848f},(MovingThingVector){-1069.209f,1342.299f,22.612f},0,128,255,0,0.3f)')
    body=body.replace('aScrollBars[0].SetVisibility(false)','MovingThings_BackendScrollVisibility(0,false)').replace('ARRAY_SIZE(aScrollBars)','MovingThings_BackendScrollCount()')
    for old,new in [('IsVisible','ScrollVisible'),('Update','ScrollUpdate'),('Render','ScrollRender')]:body=body.replace('aScrollBars[i].'+old+'()','MovingThings_Backend'+new+'(i)')
    body=body.replace('aScrollBars[i].RenderForEnvMap(camera)','MovingThings_BackendScrollEnv(i,camera)')
    for name in ['Init','Shutdown','Update']:body=body.replace('CEscalators::'+name+'()','MovingThings_BackendEscalators'+name+'()')
    for axis,index in [('x',0),('y',1),('z',2)]:
        body=body.replace('thing->m_pEntity->GetPosition().'+axis,'MovingThings_BackendPositionAxis(thing->m_pEntity,'+str(index)+')').replace('TheCamera.GetPosition().'+axis,'MovingThings_BackendCameraAxis('+str(index)+')')
    body=body.replace('pEnt->GetPosition()','MovingThings_BackendPosition(pEnt)').replace('thing->m_pEntity->GetPosition()','MovingThings_BackendPosition(thing->m_pEntity)').replace('TheCamera.GetPosition()','MovingThings_BackendCameraPosition()')
    for name in ['Right','Forward','Up']:body=re.sub(r'thing->m_pEntity->Get'+name+r'\(\) = CVector\(([^;]+)\);',lambda match:'MovingThings_BackendSet'+name+'(thing->m_pEntity,'+match[1]+');',body)
    body=body.replace('thing->m_pEntity->SetPosition(CVector(350.0f * c - 465.0f, 350.0f * s + 1163.0f, 260.0f))','MovingThings_BackendSetPosition(thing->m_pEntity,350.0f*c-465.0f,350.0f*s+1163.0f,260.0f)')
    body=body.replace('thing->m_pEntity->GetMatrix().UpdateRW()','MovingThings_BackendUpdateRW(thing->m_pEntity)').replace('thing->m_pEntity->UpdateRwFrame()','MovingThings_BackendUpdateFrame(thing->m_pEntity)').replace('pEnt->GetModelIndex()','MovingThings_BackendModelIndex(pEnt)')
    for name in MODELS:body=re.sub(r'\bMI_'+name+r'\b','MovingThings_BackendModel'+name+'()',body)
    body=body.replace('CVector delta = MovingThings_BackendPosition(thing->m_pEntity) - MovingThings_BackendCameraPosition()','MovingThingVector delta=MovingThings_Subtract(MovingThings_BackendPosition(thing->m_pEntity),MovingThings_BackendCameraPosition())').replace('delta /= delta.Magnitude()','delta=MovingThings_Divide(delta,MovingThings_Magnitude(delta))')
    body=body.replace('DotProduct(delta, CVector(c, s, 0.0f))','MovingThings_Dot(delta,(MovingThingVector){c,s,0.0f})').replace('CVector coors = MovingThings_BackendPosition(thing->m_pEntity) - 10.0f * delta','MovingThingVector coors=MovingThings_Subtract(MovingThings_BackendPosition(thing->m_pEntity),MovingThings_Scale(delta,10.0f))')
    body=body.replace('CCoronas::RegisterCorona(43, 128, 128, 100, 255, coors, 70.0f, 600.0f, 0.0f, CCoronas::TYPE_STAR, CCoronas::REFLECTION_OFF, CCoronas::LOSCHECK_OFF, CCoronas::STREAK_OFF, 0.0f)','MovingThings_BackendCorona(coors)')
    for old,new in [('Sin','sinf'),('Cos','cosf'),('Abs','fabsf')]:body=body.replace(old+'(',new+'(')
    body=body.replace('PUSH_RENDERGROUP(','MovingThings_BackendPush(').replace('POP_RENDERGROUP()','MovingThings_BackendPop()').replace('"@@GROUP@@"','"CMovingThings::Render"').replace('nil','NULL')
    for old,new in [('int16','int16_t'),('uint16','uint16_t'),('int32','int32_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    body=re.sub(r'(?m)^((?:void|int16_t)\s+(?:MovingThings_|MovingThing_)\w+\([^\n]*\)\s*\{\n)',r'\1    // Preserve all original live reads, traversal decisions and state-write ordering\n',body)
    body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original branch and traversal order\n',body,flags=re.M)
    code=re.sub(r'//[^\n]*|"[^"\n]*"','',body)
    assert not re.search(r'\b(?:CVector|CMovingThing|CMovingThings|CWorld|CPtrNode|CScrollBar|CEscalators|TheCamera|CCoronas|aScrollBars)\b',code)
    prefix='//+ rouz edit (ChatGPT)\n#include "MovingThings_backend.h"\n#include "Timer.h"\n#include "Clock.h"\n#include "GameLevels.h"\n#include "PlaneTrails.h"\n#include "PlaneBanners.h"\n#include "SmokeTrails.h"\n#include "PointLights.h"\n#include "Draw_constants.h"\n#include <math.h>\n#include <stddef.h>\n#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n#define SQR(a) ((a)*(a))\n'
    return scroll_bar.calls(prefix+HELPERS+body+'\n//- rouz edit (ChatGPT)\n')

def project(source):
    # Add a uniquely named C11 source and preserve every existing paired-object name
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\MovingThings.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\MovingThings.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['MovingThings.h','MovingThings_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return scroll_bar.project(source)

HOST=scroll_bar.calls(HOST)
BACKEND_HEADER=scroll_bar.calls(BACKEND_HEADER)
