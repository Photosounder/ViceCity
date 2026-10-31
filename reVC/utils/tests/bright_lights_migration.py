"""Move both bright-light classes, geometry tables, flicker and reflection algorithms to C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import boat_foam_migration as boat
ENUM='''enum {
    BRIGHTLIGHT_INVALID,
    BRIGHTLIGHT_TRAFFIC_GREEN,BRIGHTLIGHT_TRAFFIC_YELLOW,BRIGHTLIGHT_TRAFFIC_RED,
    BRIGHTLIGHT_FRONT_LONG,BRIGHTLIGHT_FRONT_SMALL,BRIGHTLIGHT_FRONT_BIG,BRIGHTLIGHT_FRONT_TALL,
    BRIGHTLIGHT_REAR_LONG,BRIGHTLIGHT_REAR_SMALL,BRIGHTLIGHT_REAR_BIG,BRIGHTLIGHT_REAR_TALL,
    BRIGHTLIGHT_SIREN,
    BRIGHTLIGHT_FRONT=BRIGHTLIGHT_FRONT_LONG,BRIGHTLIGHT_REAR=BRIGHTLIGHT_REAR_LONG
};
'''
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct BrightLightVector { float x,y,z; } BrightLightVector;
typedef struct BrightLight {
    BrightLightVector m_pos,m_up,m_side,m_front;
    float m_camDist;
    uint8_t m_type,m_red,m_green,m_blue;
} BrightLight;
'''+ENUM+'''#ifdef __cplusplus
extern "C" {
#endif
void BrightLights_Init(void);
void BrightLights_RegisterOne(BrightLightVector pos,BrightLightVector up,BrightLightVector side,BrightLightVector front,uint8_t type,uint8_t red,uint8_t green,uint8_t blue);
void BrightLights_Render(bool reuseFlicker);
void BrightLights_RenderOutGeometryBuffer(void);
#ifdef __cplusplus
}
class CVector;
BrightLightVector BrightLights_FromVector(CVector value);
void BrightLights_RegisterOneLegacy(CVector pos,CVector up,CVector side,CVector front,uint8_t type,uint8_t red=0,uint8_t green=0,uint8_t blue=0);
#endif
//- rouz edit (ChatGPT)
'''
GUARD='#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n'
READERS=[]
for name in ['Position','Right','Up']:READERS.append(('BrightLightVector','Camera'+name,'void','const CVector &v=TheCamera.Get'+name+'();\n    BrightLightVector result={v.x,v.y,v.z};return result;','Copy the original live main-camera '+name+' value'))
READERS += [
('bool','EnvMap','void',GUARD+'    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif','Read the original reflection flag at its original operand'),
('void *','EnvCamera','void',GUARD+'    return CustomPipes::EnvMapCam;\n#else\n    return NULL;\n#endif','Read the original live reflection-camera pointer'),
('void *','CameraFrame','void *camera','return RwCameraGetFrame((RwCamera *)camera);','Enter the original RenderWare camera-frame operation'),
('void *','FrameMatrix','void *frame','return RwFrameGetMatrix((RwFrame *)frame);','Retain each original RenderWare matrix query'),
('void','LittleTest','void','LittleTest();','Invoke the original buffer hook before live transform-count reads'),
('void','ClearRaster','void','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);','Clear the original texture raster state')]
for name,field in [('Position','pos'),('Right','right'),('Up','up')]:READERS.append(('BrightLightVector','Matrix'+name,'const void *matrix','const RwV3d &v=((const RwMatrix *)matrix)->'+field+';\n    BrightLightVector result={v.x,v.y,v.z};return result;','Read the actual typed matrix '+field+' components'))
for name,state in [('VertexAlpha','VERTEXALPHAENABLE'),('ZWrite','ZWRITEENABLE')]:READERS.append(('void',name,'bool enabled','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)enabled);','Set the original typed '+name+' state'))
READERS += [('void','SrcBlend','void','RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);','Select the original source blend factor'),('void','DestBlend','void','RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);','Select the original destination blend factor')]
for name,args,values in [('RGBA','uint8_t r,uint8_t g,uint8_t b,uint8_t a','r,g,b,a'),('Pos','float x,float y,float z','x,y,z')]:READERS.append(('void','Set'+name,'int32_t index,'+args,'RwIm3DVertexSet'+name+'(&TempBufferRenderVertices[index],'+values+');','Write the original typed shared vertex '+name+' fields'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "BrightLights.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' BrightLights_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "BrightLights_backend.h"
#include "Camera.h"
#include "RenderBuffer.h"
#include "main.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
'''+''.join(result+' BrightLights_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'''
BrightLightVector BrightLights_FromVector(CVector value)
{
    // Copy actual typed components without assuming legacy vector layout
    BrightLightVector result={value.x,value.y,value.z};return result;
}
void BrightLights_RegisterOneLegacy(CVector pos,CVector up,CVector side,CVector front,uint8_t type,uint8_t red,uint8_t green,uint8_t blue)
{
    // Retain all original by-value CVector captures before entering the C registration
    BrightLights_RegisterOne(BrightLights_FromVector(pos),BrightLights_FromVector(up),BrightLights_FromVector(side),BrightLights_FromVector(front),type,red,green,blue);
}
//- rouz edit (ChatGPT)
'''

def section(source):
    # Delimit all four methods, their camera helper and every geometry/flicker/cache table
    a=source.index('#define BRIGHTLIGHTS_MAX_DIST');b=source.index('CRGBA FoamColour(',a);return source[a:b]

def record_header(source):
    # Expose private oracle observations without changing any field or method body
    a=source.index('enum\n{\n\tBRIGHTLIGHT_INVALID');b=source.index('#include "ShinyTexts.h"',a)
    return source[a:b].replace('class CBrightLights\n{','class CBrightLights\n{\npublic:')

def calls(source):
    # Remove both owners and route the existing controller, vehicle and reflection calls to C
    if '#define BRIGHTLIGHTS_MAX_DIST' in source:source=source.replace(section(source),'')
    if 'class CBrightLight\n' in source:
        a=source.index('enum\n{\n\tBRIGHTLIGHT_INVALID');b=source.index('#include "ShinyTexts.h"',a);source=source[:a]+'#include "BrightLights.h"\n\n'+source[b:]
    for name in ['Init','Render']:
        if 'SpecialFX_BackendBrightLights'+name+'(void)\n{' in source:source=source.replace(function(source,'SpecialFX_BackendBrightLights',name)+'\n','')
        source=re.sub(r'^void SpecialFX_BackendBrightLights'+name+r'\(void\);\n','',source,flags=re.M)
        if 'SpecialFX_BackendBrightLights'+name+'(' in source:
            source=source.replace('SpecialFX_BackendBrightLights'+name+'()','BrightLights_'+name+'('+('false' if name=='Render' else '')+')')
            if '#include "BrightLights.h"' not in source:source=source.replace('#include "SpecialFX_backend.h"','#include "SpecialFX_backend.h"\n#include "BrightLights.h"')
    source=source.replace('CBrightLights::RegisterOne(','BrightLights_RegisterOneLegacy(').replace('CBrightLights::Render()','BrightLights_Render(false)').replace('CBrightLights::','BrightLights_')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bBrightLights_(?:Init|Render|RegisterOneLegacy)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the C bright-light algorithm with the original arguments\n')
        lines.append(line)
    return boat.calls(''.join(lines))

HELPERS='''static BrightLightVector BrightLights_Add(BrightLightVector a,BrightLightVector b)
{
    // Preserve component-wise original vector addition
    BrightLightVector r={a.x+b.x,a.y+b.y,a.z+b.z};return r;
}
static BrightLightVector BrightLights_LeftScale(float a,BrightLightVector b)
{
    // Preserve original scalar-times-vector operand order
    BrightLightVector r={a*b.x,a*b.y,a*b.z};return r;
}
static BrightLightVector BrightLights_Subtract(BrightLightVector a,BrightLightVector b)
{
    // Preserve component-wise original vector subtraction
    BrightLightVector r={a.x-b.x,a.y-b.y,a.z-b.z};return r;
}
static float BrightLights_Squared(BrightLightVector value)
{
    // Retain the original sum order before any square root
    return value.x*value.x+value.y*value.y+value.z*value.z;
}
static float BrightLights_Distance(BrightLightVector a,BrightLightVector b)
{
    // Retain the original single-precision magnitude operation
    return sqrtf(BrightLights_Squared(BrightLights_Subtract(a,b)));
}
static void BrightLights_Normalise(BrightLightVector *value)
{
    // Preserve the complete original vector normalizer including its zero branch
    float sq=BrightLights_Squared(*value);
    if(sq>0.0f){
        // Multiply original components by the same reciprocal square root
        float invsqrt=1.0f/sqrtf(sq);value->x*=invsqrt;value->y*=invsqrt;value->z*=invsqrt;
    }else{
        // Retain the original x-only fallback
        value->x=1.0f;
    }
}
'''

def kernel(source):
    # Transcribe every original method and preserve the complete reflection/flicker lifetime
    body=section(source).replace('int CBrightLights::NumBrightLights;','static int32_t NumBrightLights;').replace('CBrightLight CBrightLights::aBrightLights[NUMBRIGHTLIGHTS];','static BrightLight aBrightLights[NUMBRIGHTLIGHTS];').replace('CBrightLight','BrightLight')
    body=body.replace('BrightLights::','BrightLights_').replace('CBrightLights::','BrightLights_')
    for name in ['RenderOutGeometryBuffer']:body=re.sub(r'(?<![\w:])'+name+r'\(',r'BrightLights_'+name+'(',body)
    body=body.replace('GetBrightLightsCameraPosition','BrightLights_GetCameraPosition')
    body=body.replace('CVector cameraPosition = TheCamera.GetPosition()','BrightLightVector cameraPosition = BrightLights_BackendCameraPosition()')
    for name in ['Right','Up']:body=body.replace('TheCamera.Get'+name+'()','BrightLights_BackendCamera'+name+'()')
    body=body.replace('CustomPipes::bRenderingEnvMap','BrightLights_BackendEnvMap()').replace('CustomPipes::EnvMapCam','BrightLights_BackendEnvCamera()')
    body=body.replace('(RwCamera*)BrightLights_BackendEnvCamera()','BrightLights_BackendEnvCamera()').replace('RwCamera *effectCamera','void *effectCamera').replace('RwFrame *effectFrame','void *effectFrame').replace('const RwMatrix *cameraMatrix','const void *cameraMatrix')
    body=body.replace('RwCameraGetFrame(','BrightLights_BackendCameraFrame(').replace('RwFrameGetMatrix(','BrightLights_BackendFrameMatrix(')
    body=body.replace('CVector(BrightLights_BackendFrameMatrix(effectFrame)->pos)','BrightLights_BackendMatrixPosition(BrightLights_BackendFrameMatrix(effectFrame))')
    for name in ['Right','Up']:body=body.replace('CVector(cameraMatrix->'+name.lower()+')','BrightLights_BackendMatrix'+name+'(cameraMatrix)').replace('brightLightCamera'+name+'.Normalise()','BrightLights_Normalise(&brightLightCamera'+name+')')
    body=body.replace('(pos - BrightLights_GetCameraPosition()).Magnitude()','BrightLights_Distance(pos,BrightLights_GetCameraPosition())')
    statement='aBrightLights[NumBrightLights].m_camDist = BrightLights_Distance(pos,BrightLights_GetCameraPosition());'
    body=body.replace(statement,'// Complete the original C++ assignment right operand before choosing its live destination\n\tfloat cameraDistance=BrightLights_Distance(pos,BrightLights_GetCameraPosition());\n\taBrightLights[NumBrightLights].m_camDist=cameraDistance;')
    body=body.replace('(aBrightLights[i].m_pos - BrightLightsEnvMap[j].m_pos).MagnitudeSqr()','BrightLights_Squared(BrightLights_Subtract(aBrightLights[i].m_pos,BrightLightsEnvMap[j].m_pos))')
    for expr in ['BrightLightsEnvMap[j].m_pos','aBrightLights[i].m_pos']:body=body.replace('('+expr+' - brightLightCameraPosition).Magnitude()','BrightLights_Distance('+expr+',brightLightCameraPosition)')
    for name in ['TrafficLights','LongCarHeadLights','SmallCarHeadLights','BigCarHeadLights','TallCarHeadLights','SirenLights']:
        parts=[(name+'Side[j]','brightLightCameraRight' if name=='SirenLights' else 'aBrightLights[i].m_side'),(name+'Up[j]','brightLightCameraUp' if name=='SirenLights' else 'aBrightLights[i].m_up')]
        if 'CarHeadLights' in name:parts.append((name+'Front[j]','aBrightLights[i].m_front'))
        expression='BrightLights_LeftScale('+parts[0][0]+','+parts[0][1]+')'
        for a,b in parts[1:]:expression='BrightLights_Add('+expression+',BrightLights_LeftScale('+a+','+b+'))'
        expression='BrightLights_Add('+expression+',aBrightLights[i].m_pos)'
        body,count=re.subn(r'pos = '+name+r'Side\[j\][\s\S]*?;',lambda _: 'pos = '+expression+';',body);assert count==1,name
    body=body.replace('CVector','BrightLightVector').replace('RwImVertexIndex','uint16_t')
    body=re.sub(r'RwIm3DVertexSet(RGBA|Pos)\(&TempBufferRenderVertices\[([^\]]+)\],',r'BrightLights_BackendSet\1(\2,',body)
    for name,state in [('VertexAlpha','VERTEXALPHAENABLE'),('ZWrite','ZWRITEENABLE')]:
        for value,cvalue in [('TRUE','true'),('FALSE','false')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','BrightLights_Backend'+name+'('+cvalue+')')
    body=body.replace('RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA)','BrightLights_BackendSrcBlend()').replace('RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA)','BrightLights_BackendDestBlend()').replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil)','BrightLights_BackendClearRaster()')
    body=body.replace('LittleTest()','BrightLights_BackendLittleTest()').replace('RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXUV)','RenderBuffer_BackendTransform(TempBufferVerticesStored)').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored)','RenderBuffer_BackendRenderIndexed(TempBufferIndicesStored)').replace('RwIm3DEnd()','RenderBuffer_BackendEnd()')
    body=body.replace('nil','NULL');body=re.sub(r'\buint8\b','uint8_t',body)
    body=body.replace('\n{\n','\n{\n\t// Retain the complete original bright-light algorithm and live write order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve original conditions, arithmetic and reflection restoration\n',body,flags=re.M)
    assert not re.search(r'\b(?:CBrightLight|CBrightLights|CVector|RwCamera|RwFrame|RwMatrix|RwRenderStateSet|TheCamera|CustomPipes)\b',body)
    return '//+ rouz edit (ChatGPT)\n#include "BrightLights_backend.h"\n#include "RenderBuffer_backend.h"\n#include "General.h"\n#include <stddef.h>\n#include <math.h>\n\n'+HELPERS+body.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register C11 algorithms and typed backend with distinct paired object names
    if '..\\src\\renderer\\BrightLights.c"' not in source:
        match=re.search(r'<ClCompile Include="\.\.\\src\\renderer\\ShinyTexts\.c">[\s\S]*?</ClCompile>',source)
        if match:
            entry=match[0]
            if '<Filter>' in entry:source=source.replace(entry,entry+'\n    '+entry.replace('ShinyTexts.c','BrightLights.cpp')+'\n    '+entry.replace('ShinyTexts.c','BrightLights.c'))
            else:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\BrightLights.cpp" />\n    '+entry.replace('ShinyTexts.c','BrightLights.c'))
    for name in ['BrightLights.h','BrightLights_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    return boat.project(source)
