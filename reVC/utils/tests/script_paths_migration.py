"""Move both scripted-path classes, complete algorithms and storage into C11."""
import re
import particle_mgr_migration as particle_mgr
METHODS=[('CScriptPath',n) for n in ['FindCoorsFromDistanceOnPath','Update','Clear','InitialiseOne','SetObjectToControl']]+[('CScriptPaths',n) for n in ['Init','Shutdown','Update','IsOneActive','Load','Save','Load_ForReplay','Save_ForReplay']]
FIELDS=['m_numNodes','m_pNode','m_fTotalLength','m_fSpeed','m_fPosition','m_fObjectLength','m_state','m_pObjects']
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef __cplusplus
class CObject;
struct CPlaneNode;
#else
typedef struct CObject CObject;
typedef struct CPlaneNode CPlaneNode;
#endif
enum { SCRIPT_PATH_DISABLED=0,SCRIPT_PATH_INITIALIZED,SCRIPT_PATH_ACTIVE };
typedef struct ScriptPathVector { float x,y,z; } ScriptPathVector;
typedef struct ScriptPath {
    int32_t m_numNodes;
    CPlaneNode *m_pNode;
    float m_fTotalLength,m_fSpeed,m_fPosition,m_fObjectLength;
    int32_t m_state;
    CObject *m_pObjects[6];
} ScriptPath;
#ifdef __cplusplus
extern "C" {
#endif
extern ScriptPath ScriptPaths_aArray[3];
extern CObject *g_pScriptPathObjects[18];
void ScriptPath_FindCoorsFromDistanceOnPath(ScriptPath *path,float t,float *x,float *y,float *z);
void ScriptPath_Update(ScriptPath *path);
void ScriptPath_Clear(ScriptPath *path);
void ScriptPath_InitialiseOne(ScriptPath *path,int32_t numNodes,float length);
void ScriptPath_SetObjectToControl(ScriptPath *path,CObject *object);
void ScriptPaths_Init(void);
void ScriptPaths_Shutdown(void);
void ScriptPaths_Update(void);
bool ScriptPaths_IsOneActive(void);
void ScriptPaths_Load(uint8_t *buffer,uint32_t size);
void ScriptPaths_Save(uint8_t *buffer,uint32_t *size);
void ScriptPaths_Load_ForReplay(void);
void ScriptPaths_Save_ForReplay(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
GEOMETRY='''typedef struct ScriptPathGeometry { ScriptPathVector pos1,pos2,right,up,forward; } ScriptPathGeometry;
typedef void (*ScriptPathObjectCallback)(ScriptPath *,int32_t,ScriptPathGeometry,void *);
enum { SCRIPT_PATH_NODE_SIZE=20 };
'''
READERS=[
('float','NodeT','ScriptPath *path,int32_t index','return path->m_pNode[index].t;'),
('float','NodeAxis','ScriptPath *path,int32_t index,int32_t axis','const CVector &v=path->m_pNode[index].p;return axis==0?v.x:axis==1?v.y:v.z;'),
('void','FreeNodes','CPlaneNode *nodes','free(nodes);'),
('CPlaneNode *','LoadNodes','const char *file,int32_t *count,float *length','return CPlane::LoadPath(file,*count,*length,false);'),
('CPlaneNode *','AllocateNodes','int32_t count','return (CPlaneNode *)malloc(sizeof(CPlaneNode)*count);'),
('void','ReadNode','CPlaneNode *nodes,int32_t index,const void *buffer','nodes[index]=*(const CPlaneNode *)buffer;'),
('void','WriteNode','void *buffer,CPlaneNode *nodes,int32_t index','*(CPlaneNode *)buffer=nodes[index];'),
('CPool *','ObjectPool','void','return CPools::GetObjectPool();'),
('void','ClearPhysics','CObject *object','object->m_phy_flagA08=false;'),
('void','Register','CObject *object,CObject **slot','object->RegisterReference((CEntity **)slot);'),
('ScriptPathVector','Position','CObject *object','const CVector &v=object->GetPosition();ScriptPathVector result={v.x,v.y,v.z};return result;'),
('ScriptPathVector','Forward','CObject *object','const CVector &v=object->GetForward();ScriptPathVector result={v.x,v.y,v.z};return result;'),
('void','SetPosition','CObject *object,ScriptPathVector value','object->SetPosition(CVector(value.x,value.y,value.z));'),
('void','UpdateRW','CObject *object','object->GetMatrix().UpdateRW();'),
('void','UpdateFrame','CObject *object','object->UpdateRwFrame();'),
('bool','BigBuilding','CObject *object','return object->bIsBIGBuilding;'),
('void','RemoveAndAdd','CObject *object','object->RemoveAndAdd();'),
('ScriptPathVector','PreviousPosition','void *matrix','const CVector &v=((CMatrix *)matrix)->GetPosition();ScriptPathVector result={v.x,v.y,v.z};return result;'),
('ScriptPathVector','PreviousForward','void *matrix','const CVector &v=((CMatrix *)matrix)->GetForward();ScriptPathVector result={v.x,v.y,v.z};return result;')]
for name in ['Right','Up','Forward']:READERS.append(('void','Set'+name,'CObject *object,ScriptPathVector value','object->Get'+name+'()=CVector(value.x,value.y,value.z);'))
for name,field in [('MoveSpeed','m_vecMoveSpeed'),('TurnSpeed','m_vecTurnSpeed'),('MoveFriction','m_vecMoveFriction'),('TurnFriction','m_vecTurnFriction')]:READERS.append(('void','Set'+name,'CObject *object,ScriptPathVector value','object->'+field+'=CVector(value.x,value.y,value.z);'))
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "ScriptPaths.h"
#include "Pool.h"
'''+GEOMETRY+'''#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' ScriptPaths_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''void ScriptPaths_BackendWithPrevious(ScriptPath *path,int32_t index,ScriptPathGeometry geometry,ScriptPathObjectCallback callback);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "ScriptPaths_backend.h"
static_assert(sizeof(CPlaneNode)==SCRIPT_PATH_NODE_SIZE,"Original native node serialization stride");
'''+''.join(result+' ScriptPaths_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed native operation and original borrowed field access\n    '+body+'\n}\n' for result,name,args,body in READERS)+'''void ScriptPaths_BackendWithPrevious(ScriptPath *path,int32_t index,ScriptPathGeometry geometry,ScriptPathObjectCallback callback)
{
    // Keep the original native matrix copy alive through the complete per-object update sequence
    CMatrix previous(path->m_pObjects[index]->GetMatrix());callback(path,index,geometry,&previous);
}
//- rouz edit (ChatGPT)
'''
HELPERS='''static ScriptPathVector ScriptPaths_Add(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve each original component addition before midpoint division
    ScriptPathVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static ScriptPathVector ScriptPaths_Subtract(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve each original component subtraction before direction or speed calculation
    ScriptPathVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static ScriptPathVector ScriptPaths_Divide(ScriptPathVector a,float value)
{
    // Divide the original components directly without replacing division by reciprocal multiplication
    ScriptPathVector result={a.x/value,a.y/value,a.z/value};return result;
}
static ScriptPathVector ScriptPaths_Cross(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve the native cross-product component order and operand order
    ScriptPathVector result={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};return result;
}
static void ScriptPaths_Normalise(ScriptPathVector *a)
{
    // Preserve the native reciprocal-square-root normalizer and its x-only zero fallback
    float sq=a->x*a->x+a->y*a->y+a->z*a->z;if(sq>0.0f){
        // Multiply each original component by the same reciprocal square root
        float inv=1.0f/sqrtf(sq);a->x*=inv;a->y*=inv;a->z*=inv;
    }else a->x=1.0f;
}
static float ScriptPaths_Heading(ScriptPathVector a)
{
    // Preserve the original negated-x heading arguments
    return atan2f(-a.x,a.y);
}
static bool ScriptPaths_Different(ScriptPathVector a,ScriptPathVector b)
{
    // Preserve the original ordered component inequality checks
    return a.x!=b.x || a.y!=b.y || a.z!=b.z;
}
static void ScriptPaths_UpdateObject(ScriptPath *path,int32_t i,ScriptPathGeometry geometry,void *previous);
'''

def method(source,owner,name):
    # Delimit every original method without changing its opening-brace formatting
    match=re.search(r'(?m)^(?:void|bool)\s+'+owner+r'::'+name+r'\([^\n]*\)\s*\{',source);assert match,(owner,name)
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]

def classes(source):
    # Include both complete original declarations and their original state enum
    a=source.index('enum {\n\tSCRIPT_PATH_DISABLED');b=source.index('\n};',source.index('class CScriptPaths\n',a))+len('\n};');return source[a:b]

def calls(source):
    # Replace both classes and retain every original caller argument and save callback type
    if 'CScriptPath' not in source and 'FixScriptPaths(uint8' not in source:return particle_mgr.calls(source)
    if 'CScriptPath CScriptPaths::aArray[3];' in source:
        source=source.replace('CScriptPath CScriptPaths::aArray[3];','').replace('CObject *g_pScriptPathObjects[18];','')
        for owner,name in METHODS:source=source.replace(method(source,owner,name),'')
        source=re.sub(r'#ifdef COMPATIBLE_SAVES\n#define SCRIPTPATHS_SAVE_SIZE 0x9C\n#else\n#define SCRIPTPATHS_SAVE_SIZE sizeof\(aArray\)\n#endif\n','',source)
        source=source.replace('#include "Fluff.h"','#include "Fluff.h"\n#include "Plane.h" // rouz edit (ChatGPT)',1)+'\n'+HOST
    if 'class CScriptPath\n' in source:
        source=source.replace(classes(source),'//+ rouz edit (ChatGPT)\n#include "ScriptPaths.h"\n//- rouz edit (ChatGPT)')
        source=source.replace('#include "common.h"\n#include "Vector.h"\n#include "Object.h"\n#include "Plane.h"\n','')
    if 'FixScriptPaths(uint8' in source and '#include "Plane.h"' not in source:source=source.replace('#include "Fluff.h"','#include "Fluff.h"\n#include "Plane.h" // rouz edit (ChatGPT)',1)
    lines=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CScriptPaths::aArray','ScriptPaths_aArray')
        for name in ['InitialiseOne','SetObjectToControl']:changed=re.sub(r'ScriptPaths_aArray\[(.*?)\]\.'+name+r'\(([^\n]*)\);',lambda m:'ScriptPath_'+name+'(&ScriptPaths_aArray['+m[1]+'],'+m[2]+');',changed)
        changed=re.sub(r'ScriptPaths_aArray\[(.*?)\]\.Clear\(\)',r'ScriptPath_Clear(&ScriptPaths_aArray[\1])',changed).replace('CScriptPaths::','ScriptPaths_')
        if changed!=line and not line.lstrip().startswith('//'):lines.append(re.match(r'\s*',line)[0]+'// Enter the C scripted-path operation at the original call point\n')
        lines.append(changed)
    return particle_mgr.calls(''.join(lines))

def kernel(source):
    # Translate every complete original record/controller operation including both save formats and replay storage
    chunks=[]
    for owner,name in METHODS:
        text=method(source,owner,name)
        if owner=='CScriptPath':
            text=text.replace('CScriptPath::'+name+'(','ScriptPath_'+name+'(ScriptPath *path,')
            text=text.replace('(ScriptPath *path,void)','(ScriptPath *path)')
        else:text=text.replace('CScriptPaths::'+name,'ScriptPaths_'+name)
        chunks.append(text)
    body='\n\n'.join(chunks)
    a=body.index('\t\t\t\tCMatrix prevMat(');b=body.index('\n',body.index('m_pObjects[i]->m_vecTurnFriction = CVector(0.0f, 0.0f, 0.0f);',a))
    per_object=body[a:b];per_object=per_object[per_object.index('\t\t\t\tCVector prevPosition'):]
    body=body[:a]+'                // Keep the actual native matrix copy alive until all original object writes finish\n                ScriptPathGeometry geometry={pos1,pos2,newRight,newUp,newForward};\n                ScriptPaths_BackendWithPrevious(path,i,geometry,ScriptPaths_UpdateObject);'+body[b:]
    body+='\n\nstatic void ScriptPaths_UpdateObject(ScriptPath *path,int32_t i,ScriptPathGeometry geometry,void *previous)\n{\n    // Reuse the same captured geometry throughout the original per-object lifetime\n    ScriptPathVector pos1=geometry.pos1,pos2=geometry.pos2,newRight=geometry.right,newUp=geometry.up,newForward=geometry.forward;\n'+per_object+'\n}\n'
    body=body.replace('CVector pos1, pos2;','ScriptPathVector pos1,pos2;').replace('CVector newForward, newUp(0.0f, 0.0f, 1.0f), newRight;','ScriptPathVector newForward,newUp={0.0f,0.0f,1.0f},newRight;')
    body=body.replace('newForward = pos2 - pos1','newForward=ScriptPaths_Subtract(pos2,pos1)').replace('newForward.Normalise()','ScriptPaths_Normalise(&newForward)').replace('newRight.Normalise()','ScriptPaths_Normalise(&newRight)').replace('CrossProduct(','ScriptPaths_Cross(')
    body=body.replace('FindCoorsFromDistanceOnPath(t1,','ScriptPath_FindCoorsFromDistanceOnPath(path,t1,').replace('FindCoorsFromDistanceOnPath(t2,','ScriptPath_FindCoorsFromDistanceOnPath(path,t2,')
    body=body.replace('CVector prevPosition = m_pObjects[i]->GetPosition()','ScriptPathVector prevPosition=ScriptPaths_BackendPosition(m_pObjects[i])')
    body=body.replace('m_pObjects[i]->SetPosition((pos1 + pos2) / 2.0f)','ScriptPaths_BackendSetPosition(m_pObjects[i],ScriptPaths_Divide(ScriptPaths_Add(pos1,pos2),2.0f))')
    for axis in ['Right','Up','Forward']:body=body.replace('m_pObjects[i]->Get'+axis+'() = new'+axis,'ScriptPaths_BackendSet'+axis+'(m_pObjects[i],new'+axis+')')
    body=body.replace('m_pObjects[i]->GetMatrix().UpdateRW()','ScriptPaths_BackendUpdateRW(m_pObjects[i])').replace('m_pObjects[i]->UpdateRwFrame()','ScriptPaths_BackendUpdateFrame(m_pObjects[i])')
    body=body.replace('!m_pObjects[i]->bIsBIGBuilding && prevPosition != m_pObjects[i]->GetPosition()','!ScriptPaths_BackendBigBuilding(m_pObjects[i]) && ScriptPaths_Different(prevPosition,ScriptPaths_BackendPosition(m_pObjects[i]))').replace('m_pObjects[i]->RemoveAndAdd()','ScriptPaths_BackendRemoveAndAdd(m_pObjects[i])')
    body=body.replace('m_pObjects[i]->m_vecMoveSpeed = (m_pObjects[i]->GetPosition() - prevMat.GetPosition()) / (*Timer_GetTimeStep())','ScriptPaths_BackendSetMoveSpeed(m_pObjects[i],ScriptPaths_Divide(ScriptPaths_Subtract(ScriptPaths_BackendPosition(m_pObjects[i]),ScriptPaths_BackendPreviousPosition(previous)),(*Timer_GetTimeStep())))')
    body=body.replace('m_pObjects[i]->GetForward().Heading()','ScriptPaths_Heading(ScriptPaths_BackendForward(m_pObjects[i]))').replace('prevMat.GetForward().Heading()','ScriptPaths_Heading(ScriptPaths_BackendPreviousForward(previous))')
    for name in ['TurnSpeed','MoveFriction','TurnFriction']:body=re.sub(r'm_pObjects\[i\]->m_vec'+name+r' = CVector\(([^()]*)\)',r'ScriptPaths_BackendSet'+name+r'(m_pObjects[i],(ScriptPathVector){\1})',body)
    body=body.replace('free(m_pNode)','ScriptPaths_BackendFreeNodes(m_pNode)').replace('CPlane::LoadPath(Dest, m_numNodes, m_fTotalLength, false)','ScriptPaths_BackendLoadNodes(Dest,&m_numNodes,&m_fTotalLength)')
    body=body.replace('pObj->RegisterReference((CEntity**)&m_pObjects[i])','ScriptPaths_BackendRegister(pObj,&m_pObjects[i])').replace('pObj->m_phy_flagA08 = false','ScriptPaths_BackendClearPhysics(pObj)')
    body=body.replace('CPools::GetObjectPool()','ScriptPaths_BackendObjectPool()')
    for expression in ['aArray[i].m_pObjects[j]','pPath->m_pObjects[j]']:body=body.replace(expression+'->m_phy_flagA08 = false','ScriptPaths_BackendClearPhysics('+expression+')')
    body=body.replace('aArray[i].m_pNode = (CPlaneNode*)malloc(sizeof(CPlaneNode)*aArray[i].m_numNodes)','aArray[i].m_pNode=ScriptPaths_BackendAllocateNodes(aArray[i].m_numNodes)')
    body=body.replace('aArray[i].m_pNode[j] = *(CPlaneNode*)buf','ScriptPaths_BackendReadNode(aArray[i].m_pNode,j,buf)').replace('*(CPlaneNode*)buf = aArray[i].m_pNode[j]','ScriptPaths_BackendWriteNode(buf,aArray[i].m_pNode,j)')
    body=body.replace('sizeof(CPlaneNode)','SCRIPT_PATH_NODE_SIZE').replace('sizeof(aArray[i].m_pNode[j])','SCRIPT_PATH_NODE_SIZE')
    body=body.replace('aArray[i].Clear()','ScriptPath_Clear(&aArray[i])').replace('aArray[i].Update()','ScriptPath_Update(&aArray[i])')
    for expression in ['m_pNode[i + 1].t','m_pNode[i].t']:body=body.replace(expression,'ScriptPaths_BackendNodeT(path,'+('i+1' if '+ 1' in expression else 'i')+')')
    for axis,index in [('x',0),('y',1),('z',2)]:
        for expression,slot in [('m_numNodes - 1','m_numNodes-1'),('i + 1','i+1'),('i','i')]:body=body.replace('m_pNode['+expression+'].p.'+axis,'ScriptPaths_BackendNodeAxis(path,'+slot+','+str(index)+')')
    for field in FIELDS:body=re.sub(r'(?<![\w.>])'+field+r'\b','path->'+field,body)
    body=body.replace('CScriptPath','ScriptPath').replace('aArray','ScriptPaths_aArray').replace('uintptr','uintptr_t').replace('(float)TWOPI','SCRIPT_PATHS_TWOPI').replace('(float)PI','SCRIPT_PATHS_PI')
    for old,new in [('int32','int32_t'),('uint32','uint32_t'),('uint8','uint8_t')]:body=re.sub(r'\b'+old+r'\b',new,body)
    body=body.replace('nil','NULL')
    body=re.sub(r'(?m)^((?:void|bool)\s+ScriptPaths?_\w+\([^\n]*\)\s*\{\n)',r'\1    // Preserve every original live read, save cursor and state-write ordering\n',body)
    body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original callback and traversal order\n',body,flags=re.M)
    code=re.sub(r'//[^\n]*|"[^"\n]*"','',body);assert not re.search(r'\b(?:CVector|CMatrix|CScriptPath|CScriptPaths|CPlane|CPools|CEntity|prevMat|std)\b',code)
    prefix='''//+ rouz edit (ChatGPT)
#include "ScriptPaths_backend.h"
#include "Timer.h"
#include <math.h>
#include <stdio.h>
#ifdef VALIDATE_SAVE_SIZE
void re3_assert(const char *,const char *,unsigned int,const char *);
#ifdef assert
#undef assert
#endif
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#endif
#include "SaveBuf.h"
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#define SCRIPT_PATHS_PI 3.14159265358979323846f
#define SCRIPT_PATHS_TWOPI 6.28318530717958647692f
#ifdef COMPATIBLE_SAVES
#define SCRIPTPATHS_SAVE_SIZE 0x9C
#else
#define SCRIPTPATHS_SAVE_SIZE sizeof(ScriptPaths_aArray)
#endif
ScriptPath ScriptPaths_aArray[3];
CObject *g_pScriptPathObjects[18];
'''
    return prefix+HELPERS+body+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register one unique C11 unit and preserve every existing paired object-name distinction
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\ScriptPaths.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\ScriptPaths.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['ScriptPaths.h','ScriptPaths_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return particle_mgr.project(source)
