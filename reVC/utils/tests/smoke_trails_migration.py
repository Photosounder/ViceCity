"""Move all six smoke algorithms, their records, tables and visibility helper into C11."""
import re
from audio_controls_migration import function
import moving_things_migration as moving_things
TYPES='''typedef struct SmokeTrailVector { float x,y,z; } SmokeTrailVector;
typedef struct SmokeTrail {
    SmokeTrailVector m_pos[16];
    float m_opacity[16];
    int32_t m_time[16];
    char m_unused[536];
    int32_t m_seed;
} SmokeTrail;
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
'''+TYPES+'''#ifdef __cplusplus
extern "C" {
#endif
extern bool SmokeTrails_CigOn;
void SmokeTrails_Init(void);
void SmokeTrails_Update(void);
void SmokeTrails_Render(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
DESCRIPTOR='''typedef struct SmokeTrailPoint { void *owner;float *x,*y,*z; } SmokeTrailPoint;
typedef struct SmokeTrailPoints { SmokeTrailPoint start,end; } SmokeTrailPoints;
typedef void (*SmokeTrailPointsCallback)(SmokeTrailPoints points);
'''
READERS=[
('bool','EnvMap','void','#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif'),
('bool','SphereVisible','SmokeTrailVector position,float radius','return TheCamera.IsSphereVisible(CVector(position.x,position.y,position.z),radius);'),
('bool','FirstPerson','void','return TheCamera.Using1stPersonWeaponMode();'),
('void *','PlayerPed','void','return FindPlayerPed();'),
('void *','PlayerVehicle','void','return FindPlayerVehicle();'),
('bool','Cutscene','void','return CCutsceneMgr::IsRunning();'),
('void *','Clump','void *ped','return ((CPlayerPed *)ped)->GetClump();'),
('void *','Hierarchy','void *clump','return GetAnimHierarchyFromSkinClump((RpClump *)clump);'),
('int32_t','HeadTag','void','return ConvertPedNode2BoneTag(PED_HEAD);'),
('int32_t','BoneIndex','void *hier,int32_t tag','return RpHAnimIDGetIndex((RpHAnimHierarchy *)hier,tag);'),
('void *','MatrixAt','void *hier,int32_t index','return &RpHAnimHierarchyGetMatrixArray((RpHAnimHierarchy *)hier)[index];'),
('void','WithPoints','SmokeTrailPointsCallback callback','RwV3d start,end;SmokeTrailPoints points={{&start,&start.x,&start.y,&start.z},{&end,&end.x,&end.y,&end.z}};callback(points);'),
('void','TransformPoint','void *point,void *matrix','RwV3dTransformPoints((RwV3d *)point,(RwV3d *)point,1,(RwMatrix *)matrix);'),
('float','Wind','void','return CWeather::Wind;'),
('void','SetRGBA','int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a','RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],r,g,b,a);'),
('void','SetPos','int32_t index,float x,float y,float z','RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);'),
('void','VertexAlpha','bool enabled','RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);'),
('void','SrcBlend','void','RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);'),
('void','DestBlend','void','RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);'),
('void','RasterNull','void','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);'),
('bool','Transform','int32_t count','return RwIm3DTransform(TempBufferRenderVertices,count,nil,rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA)!=nil;'),
('void','RenderIndexed','uint16_t *indices,int32_t count','RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE,indices,count);'),
('void','End','void','RwIm3DEnd();')]
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include "SmokeTrails.h"
'''+DESCRIPTOR+'''#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' SmokeTrails_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='//+ rouz edit (ChatGPT)\n#include "SmokeTrails_backend.h"\n'+''.join(result+' SmokeTrails_Backend'+name+'('+args+')\n{\n    // Preserve the actual typed owner operation and original native point lifetime\n    '+body+'\n}\n' for result,name,args,body in READERS)+'//- rouz edit (ChatGPT)\n'
HELPERS='''static float SmokeTrails_Distance(SmokeTrailVector a,SmokeTrailVector b)
{
    // Keep the original component subtraction and square-root expression order
    float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return sqrtf(x*x+y*y+z*z);
}
static SmokeTrailVector SmokeTrails_ReadPoint(SmokeTrailPoint point)
{
    // Capture each original implicit CVector value before entering record registration
    SmokeTrailVector value={*point.x,*point.y,*point.z};return value;
}
static void SmokeTrails_UpdateWithPoints(SmokeTrailPoints points);
static void SmokeTrail_Render(SmokeTrail *trail);
'''
METHODS=[('CSmokeTrail','RegisterPoint'),('CSmokeTrail','Init'),('CSmokeTrails','Init'),('CSmokeTrails','Render'),('CSmokeTrail','Render'),('CSmokeTrails','Update')]
KANGAROO='''\tif (!_CHEATCMP("21G"))
\t{
\t\tKangarooCheat();
\t\tswprintf(msg, L"Kangaroo");
\t\tCHud::SetHelpMessage((wchar *) msg, true);
\t}'''
GUARDED_KANGAROO='//+ rouz edit (ChatGPT)\n#ifdef KANGAROO_CHEAT\n\t// Keep the optional custom cheat consistent with its guarded implementation\n'+KANGAROO+'\n#endif\n//- rouz edit (ChatGPT)'

def method(source,owner,name):
    # Extract the untouched original function including its original same-line opening brace
    match=re.search(r'(?m)^void\n'+owner+'::'+name+r'\([^\n]*\)\s*\{',source);assert match,(owner,name)
    a=source.index('{',match.start());b=a+1;depth=1
    while depth:depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[match.start():b]

def globals_section(source):
    # Include the flag, complete records and both original mutable tables
    a=source.index('bool CSmokeTrails::CigOn = false;');b=source.index('uint8 ScrollCharSet',a);return source[a:b]

def classes(source):
    # Delimit the two complete final class declarations in the original header
    a=source.index('class CSmokeTrail {');return source[a:]

def calls(source):
    # Remove only the relocated storage and methods while preserving unrelated Fluff algorithms
    if KANGAROO in source and GUARDED_KANGAROO not in source:source=source.replace(KANGAROO,GUARDED_KANGAROO)
    if 'bool CSmokeTrails::CigOn = false;' in source:
        source=source.replace(globals_section(source),'')
        for owner,name in METHODS:source=source.replace(method(source,owner,name),'')
        source+='\n'+HOST
    if 'class CSmokeTrail {' in source:source=source.replace(classes(source),'//+ rouz edit (ChatGPT)\n#include "SmokeTrails.h"\n//- rouz edit (ChatGPT)\n')
    lines=[]
    for line in source.splitlines(keepends=True):
        if line.lstrip().startswith('//'):lines.append(line);continue
        if 'CSmokeTrails::' in line:lines.append(re.match(r'\s*',line)[0]+'// Use the C smoke-trail state or algorithm at the original call point\n')
        lines.append(line.replace('CSmokeTrails::','SmokeTrails_'))
    return moving_things.calls(''.join(lines))

def kernel(source):
    # Translate all original bodies and split only the original native temporary-vector lifetime
    helper=function(source,'','IsEffectSphereVisible').replace('const CVector &center','SmokeTrailVector center').replace('IsEffectSphereVisible','SmokeTrails_IsSphereVisible').replace('CustomPipes::bRenderingEnvMap','SmokeTrails_BackendEnvMap()').replace('TheCamera.IsSphereVisible(','SmokeTrails_BackendSphereVisible(')
    storage=globals_section(source).replace('bool CSmokeTrails::CigOn','bool SmokeTrails_CigOn').replace('CSmokeTrail CSmokeTrails::aSmoke','static SmokeTrail aSmoke').replace('RwImVertexIndex SmokeTrailIndices','static uint16_t SmokeTrailIndices').replace('float RandomSmoke','static float RandomSmoke')
    chunks=[method(source,owner,name) for owner,name in METHODS]
    original_update=chunks.pop();a=original_update.index('\tRwV3d startPos');gate=original_update[:a];work=original_update[a:]
    gate+='    // Keep both actual native points alive until the entire original update returns\n    SmokeTrails_BackendWithPoints(SmokeTrails_UpdateWithPoints);\n}'
    work=work.replace('\tRwV3d startPos = { 0.026f, 0.15f, 0.02f };','    // Initialize actual native point fields before the original hierarchy queries\n    *points.start.x=0.026f;*points.start.y=0.15f;*points.start.z=0.02f;').replace('\tRwV3d endPos = { 0.026f, 0.05f, 0.02f };','    *points.end.x=0.026f;*points.end.y=0.05f;*points.end.z=0.02f;')
    work='static void\nSmokeTrails_UpdateWithPoints(SmokeTrailPoints points) {\n'+work
    body='\n\n'.join(chunks+[gate,work])
    for name,args in [('RegisterPoint','CVector regPosition, float opacity'),('Init','int num'),('Render','void')]:
        body=body.replace('void\nCSmokeTrail::'+name+'('+args+')','static void\nSmokeTrail_'+name+'(SmokeTrail *trail'+(', '+args if args!='void' else '')+')')
    body=body.replace('CSmokeTrails::','SmokeTrails_').replace('CVector regPosition','SmokeTrailVector regPosition')
    for field in ['m_pos','m_time','m_opacity','m_seed']:body=re.sub(r'\b'+field+r'\b','trail->'+field,body)
    body=body.replace('(trail->m_pos[1] - trail->m_pos[2]).Magnitude()','SmokeTrails_Distance(trail->m_pos[1],trail->m_pos[2])')
    body=body.replace('aSmoke[i].Init(i)','SmokeTrail_Init(&aSmoke[i],i)').replace('aSmoke[i].Render()','SmokeTrail_Render(&aSmoke[i])')
    for index in range(3):body=body.replace('aSmoke['+str(index)+'].RegisterPoint(startPos,','SmokeTrail_RegisterPoint(&aSmoke['+str(index)+'],SmokeTrails_ReadPoint(points.start),')
    body=body.replace('CustomPipes::bRenderingEnvMap','SmokeTrails_BackendEnvMap()').replace('IsEffectSphereVisible(','SmokeTrails_IsSphereVisible(').replace('CWeather::Wind','SmokeTrails_BackendWind()')
    for old,new in [('TheCamera.Using1stPersonWeaponMode','FirstPerson'),('FindPlayerPed','PlayerPed'),('FindPlayerVehicle','PlayerVehicle'),('CCutsceneMgr::IsRunning','Cutscene')]:body=body.replace(old+'(','SmokeTrails_Backend'+new+'(')
    body=body.replace('SmokeTrails_BackendPlayerPed()->GetClump()','SmokeTrails_BackendClump(SmokeTrails_BackendPlayerPed())').replace('RpHAnimHierarchy *hier','void *hier').replace('GetAnimHierarchyFromSkinClump(','SmokeTrails_BackendHierarchy(')
    body=body.replace('ConvertPedNode2BoneTag(PED_HEAD)','SmokeTrails_BackendHeadTag()').replace('RpHAnimIDGetIndex(','SmokeTrails_BackendBoneIndex(').replace('RwMatrix *head = &RpHAnimHierarchyGetMatrixArray(hier)[idx]','void *head=SmokeTrails_BackendMatrixAt(hier,idx)')
    for name in ['start','end']:
        body=body.replace('RwV3dTransformPoints(&'+name+'Pos, &'+name+'Pos, 1, head)','SmokeTrails_BackendTransformPoint(points.'+name+'.owner,head)')
        body=re.sub(r'\b'+name+r'Pos\.([xyz])',lambda match:'(*points.'+name+'.'+match[1]+')',body)
    body=re.sub(r'RwIm3DVertexSetRGBA\(&TempBufferRenderVertices\[([^\]]+)\],',lambda match:'SmokeTrails_BackendSetRGBA('+match[1]+',',body)
    body=re.sub(r'RwIm3DVertexSetPos\(&TempBufferRenderVertices\[([^\]]+)\],',lambda match:'SmokeTrails_BackendSetPos('+match[1]+',',body)
    for value,flag in [('TRUE','true'),('FALSE','false')]:body=body.replace('RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)'+value+')','SmokeTrails_BackendVertexAlpha('+flag+')')
    for state,value,name in [('SRCBLEND','rwBLENDSRCALPHA','SrcBlend'),('DESTBLEND','rwBLENDINVSRCALPHA','DestBlend')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','SmokeTrails_Backend'+name+'()')
    body=body.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil)','SmokeTrails_BackendRasterNull()')
    body=re.sub(r'RwIm3DTransform\(TempBufferRenderVertices, ([^,]+), nil, rwIM3D_VERTEXXYZ \| rwIM3D_VERTEXRGBA\)',r'SmokeTrails_BackendTransform(\1)',body)
    body=body.replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE,','SmokeTrails_BackendRenderIndexed(').replace('RwIm3DEnd()','SmokeTrails_BackendEnd()');body=re.sub(r'\bint32\b','int32_t',body)
    combined=helper+'\n'+storage+'\n'+body
    combined=re.sub(r'(?m)^((?:SmokeTrail_|SmokeTrails_)\w+\([^\n]*\)\s*\{\n)',r'\1    // Preserve the original live reads, decisions and complete state-write order\n',combined)
    combined=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original branch and traversal order\n',combined,flags=re.M)
    assert not re.search(r'\b(?:CVector|CSmokeTrail|CSmokeTrails|RpHAnimHierarchy|RwMatrix|RwV3d|TempBufferRenderVertices|TheCamera|CCutsceneMgr|CustomPipes)\b',combined)
    return '//+ rouz edit (ChatGPT)\n#include "SmokeTrails_backend.h"\n#include "Timer.h"\n#include <math.h>\n#define Min(a,b) ((a)<(b)?(a):(b))\n'+HELPERS+combined+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Add one uniquely named C11 source and retain existing paired-object metadata
    entry='<ClCompile Include="..\\src\\renderer\\Fluff.cpp" />'
    if '..\\src\\renderer\\SmokeTrails.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\SmokeTrails.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\Fluff.h" />'
    for name in ['SmokeTrails.h','SmokeTrails_backend.h']:
        if '..\\src\\renderer\\'+name+'"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\'+name+'" />')
    return moving_things.project(source)
