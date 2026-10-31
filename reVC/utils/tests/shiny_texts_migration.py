"""Move shiny-text records, registration, reflection replay and geometry control to C11."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import bright_lights_migration as bright
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct ShinyTextVector { float x,y,z; } ShinyTextVector;
typedef struct ShinyTextUV { float x,y; } ShinyTextUV;
typedef struct ShinyText {
    ShinyTextVector m_verts[4];
    ShinyTextUV m_texCoords[4];
    float m_camDist;
    uint8_t m_type,m_red,m_green,m_blue;
} ShinyText;
enum { SHINYTEXT_WALK=1,SHINYTEXT_FLAT };
#ifdef __cplusplus
extern "C" {
#endif
void ShinyTexts_Init(void);
void ShinyTexts_RegisterOne(ShinyTextVector p0,ShinyTextVector p1,ShinyTextVector p2,ShinyTextVector p3,float u0,float v0,float u1,float v1,float u2,float v2,float u3,float v3,uint8_t type,uint8_t red,uint8_t green,uint8_t blue,float maxDist);
void ShinyTexts_Render(void);
void ShinyTexts_RenderForEnvMap(void);
void ShinyTexts_RenderOutGeometryBuffer(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
GUARD='#if defined(LIBRW) && defined(EXTENDED_PIPELINES)\n'
READERS=[
('ShinyTextVector','Camera','void','const CVector &v=TheCamera.GetPosition();\n    ShinyTextVector result={v.x,v.y,v.z};return result;','Copy the original live main-camera position at its original query'),
('bool','EnvMap','void',GUARD+'    return CustomPipes::bRenderingEnvMap;\n#else\n    return false;\n#endif','Read the original optional reflection-pass flag'),
('void *','EnvCamera','void',GUARD+'    return CustomPipes::EnvMapCam;\n#else\n    return NULL;\n#endif','Read the live reflection camera on each original use'),
('void *','EnvFrame','void *camera',GUARD+'    return ((RwCamera *)camera)->getFrame();\n#else\n    return NULL;\n#endif','Enter the original typed reflection frame method'),
('void *','FrameLTM','void *frame',GUARD+'    return ((RwFrame *)frame)->getLTM();\n#else\n    return NULL;\n#endif','Enter the original typed reflection local-to-world method'),
('ShinyTextVector','MatrixPosition','void *matrix','const RwV3d &v=((RwMatrix *)matrix)->pos;\n    ShinyTextVector result={v.x,v.y,v.z};return result;','Copy actual matrix position fields without assuming its layout'),
('void *','WalkTexture','void','return gpWalkDontTex;','Read the current walk texture at every original use'),
('void *','Raster','void *texture','return RwTextureGetRaster((RwTexture *)texture);','Enter the original typed texture-to-raster operation'),
('void','SetRaster','void *raster','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);','Preserve the original texture-raster state change'),
('void','LittleTest','void','LittleTest();','Retain the original shared-buffer hook before transformation')]
for name,state in [('VertexAlpha','VERTEXALPHAENABLE'),('ZWrite','ZWRITEENABLE')]:READERS.append(('void',name,'bool enabled','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)enabled);','Set the original typed '+name+' state'))
for name,state,standard in [('SrcBlend','SRCBLEND','rwBLENDSRCALPHA'),('DestBlend','DESTBLEND','rwBLENDINVSRCALPHA')]:READERS.append(('void',name,'bool additive','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)(additive?rwBLENDONE:'+standard+'));','Set the original typed '+name+' factor'))
for name,args,values in [('RGBA','uint8_t r,uint8_t g,uint8_t b,uint8_t a','r,g,b,a'),('Pos','float x,float y,float z','x,y,z'),('U','float value','value'),('V','float value','value')]:READERS.append(('void','Set'+name,'int32_t index,'+args,'RwIm3DVertexSet'+name+'(&TempBufferRenderVertices[index],'+values+');','Write the actual typed shared vertex '+name+' fields'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "ShinyTexts.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' ShinyTexts_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "ShinyTexts_backend.h"
#include "Camera.h"
#include "RenderBuffer.h"
#include "Shadows.h"
#include "main.h"
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h"
#endif
'''+''.join(result+' ShinyTexts_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'//- rouz edit (ChatGPT)\n'

def section(source):
    # Delimit all five methods and both complete arrays without touching neighboring effects
    a=source.index('int CShinyTexts::NumShinyTexts;');b=source.index('uint32 CSpecialParticleStuff::BoatFromStart;',a)
    b=source.rindex('CRGBA FoamColour(',a,b)
    return source[a:b]

def record_header(source):
    # Expose only private test observations while retaining every declared field and method
    a=source.index('enum\n{\n\tSHINYTEXT_WALK');b=source.index('#include "MoneyMessages.h"',a)
    return source[a:b].replace('class CShinyTexts \n{','class CShinyTexts \n{\npublic:')

def calls(source):
    # Remove both owners and direct existing controller and reflection calls to C
    if 'int CShinyTexts::NumShinyTexts;' in source:source=source.replace(section(source),'')
    if 'class CShinyText\n' in source:
        a=source.index('enum\n{\n\tSHINYTEXT_WALK');b=source.index('#include "MoneyMessages.h"',a);source=source[:a]+'#include "ShinyTexts.h"\n\n'+source[b:]
    for name in ['Init','Render']:
        if 'SpecialFX_BackendShinyTexts'+name+'(void)\n{' in source:source=source.replace(function(source,'SpecialFX_BackendShinyTexts',name)+'\n','')
        source=re.sub(r'^void SpecialFX_BackendShinyTexts'+name+r'\(void\);\n','',source,flags=re.M)
        if 'SpecialFX_BackendShinyTexts'+name+'(' in source:
            source=source.replace('SpecialFX_BackendShinyTexts'+name+'(','ShinyTexts_'+name+'(')
            if '#include "ShinyTexts.h"' not in source:source=source.replace('#include "SpecialFX_backend.h"','#include "SpecialFX_backend.h"\n#include "ShinyTexts.h"')
    source=source.replace('CShinyTexts::','ShinyTexts_')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bShinyTexts_(?:Init|Render|RenderForEnvMap)\(',line):lines.append(re.match(r'\s*',line)[0]+'// Enter the C shiny-text algorithm at the original effect pass\n')
        lines.append(line)
    return bright.calls(''.join(lines))

def kernel(source):
    # Retain all original eligibility, reflection caching, vertex writes and shared-counter arithmetic
    body=section(source).replace('int CShinyTexts::NumShinyTexts;','static int32_t NumShinyTexts;').replace('CShinyText CShinyTexts::aShinyTexts[NUMSHINYTEXTS];','static ShinyText aShinyTexts[NUMSHINYTEXTS];').replace('static CShinyText EnvMapShinyTexts','static ShinyText EnvMapShinyTexts').replace('static int EnvMapShinyTextCount','static int32_t EnvMapShinyTextCount')
    body=body.replace('CShinyTexts::','ShinyTexts_')
    for name in ['Render','RenderOutGeometryBuffer']:body=re.sub(r'(?<![\w:])'+name+r'\(',r'ShinyTexts_'+name+'(',body)
    body=body.replace('CVector p0, CVector p1, CVector p2, CVector p3','ShinyTextVector p0, ShinyTextVector p1, ShinyTextVector p2, ShinyTextVector p3')
    body=body.replace('CVector shinyTextCameraPosition = TheCamera.GetPosition()','ShinyTextVector shinyTextCameraPosition = ShinyTexts_BackendCamera()')
    body=body.replace('CustomPipes::bRenderingEnvMap','ShinyTexts_BackendEnvMap()').replace('CustomPipes::EnvMapCam','ShinyTexts_BackendEnvCamera()')
    body=body.replace('ShinyTexts_BackendEnvCamera()->getFrame()->getLTM()->pos','ShinyTexts_BackendMatrixPosition(ShinyTexts_BackendFrameLTM(ShinyTexts_BackendEnvFrame(ShinyTexts_BackendEnvCamera())))')
    body=body.replace('CVector(ShinyTexts_BackendMatrixPosition(ShinyTexts_BackendFrameLTM(ShinyTexts_BackendEnvFrame(ShinyTexts_BackendEnvCamera()))))','ShinyTexts_BackendMatrixPosition(ShinyTexts_BackendFrameLTM(ShinyTexts_BackendEnvFrame(ShinyTexts_BackendEnvCamera())))')
    body=body.replace('ShinyTexts_BackendEnvCamera()->getFrame()','ShinyTexts_BackendEnvFrame(ShinyTexts_BackendEnvCamera())')
    body=body.replace('(p0 - shinyTextCameraPosition).Magnitude()','ShinyTexts_Distance(p0,shinyTextCameraPosition)')
    body=body.replace('RwTexture *lastTex','void *lastTex').replace('gpWalkDontTex','ShinyTexts_BackendWalkTexture()').replace('RwTextureGetRaster(','ShinyTexts_BackendRaster(')
    body=body.replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, ','ShinyTexts_BackendSetRaster(')
    for name,state in [('VertexAlpha','VERTEXALPHAENABLE'),('ZWrite','ZWRITEENABLE')]:
        for value,cvalue in [('TRUE','true'),('FALSE','false')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','ShinyTexts_Backend'+name+'('+cvalue+')')
    for name,state,standard in [('SrcBlend','SRCBLEND','rwBLENDSRCALPHA'),('DestBlend','DESTBLEND','rwBLENDINVSRCALPHA')]:
        for value,cvalue in [('rwBLENDONE','true'),(standard,'false')]:body=body.replace('RwRenderStateSet(rwRENDERSTATE'+state+', (void*)'+value+')','ShinyTexts_Backend'+name+'('+cvalue+')')
    body=re.sub(r'RwIm3DVertexSet(RGBA|Pos|U|V)\(&TempBufferRenderVertices\[([^\]]+)\],',r'ShinyTexts_BackendSet\1(\2,',body)
    body=body.replace('LittleTest()','ShinyTexts_BackendLittleTest()').replace('RwIm3DTransform(TempBufferRenderVertices, TempBufferVerticesStored, nil, rwIM3D_VERTEXUV)','RenderBuffer_BackendTransform(TempBufferVerticesStored)').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, TempBufferRenderIndexList, TempBufferIndicesStored)','RenderBuffer_BackendRenderIndexed(TempBufferIndicesStored)').replace('RwIm3DEnd()','RenderBuffer_BackendEnd()')
    body=body.replace('nil','NULL');body=re.sub(r'\buint8\b','uint8_t',body)
    body=body.replace('\n{\n','\n{\n\t// Retain the complete original shiny-text algorithm and live operation order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve original conditions, shared writes and reflection lifetime\n',body,flags=re.M)
    helpers='''static float ShinyTexts_Distance(ShinyTextVector a,ShinyTextVector b)
{
    // Preserve the original vector subtraction, sum order and single-precision square root
    float x=a.x-b.x,y=a.y-b.y,z=a.z-b.z;return sqrtf(x*x+y*y+z*z);
}
'''
    assert not re.search(r'\b(?:CShinyText|CShinyTexts|CVector|RwTexture|RwIm3DTransform|RwRenderStateSet|TheCamera|CustomPipes)\b',body)
    return '//+ rouz edit (ChatGPT)\n#include "ShinyTexts_backend.h"\n#include "RenderBuffer_backend.h"\n#include <math.h>\n#include <stddef.h>\n\n'+helpers+body.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register the C logic and typed backend with separate source-extension object names
    if '..\\src\\renderer\\ShinyTexts.c"' not in source:
        pattern=r'<ClCompile Include="\.\.\\src\\renderer\\MoneyMessages\.c">[\s\S]*?</ClCompile>'
        match=re.search(pattern,source)
        if match:
            entry=match[0]
            if '<Filter>' in entry:source=source.replace(entry,entry+'\n    '+entry.replace('MoneyMessages.c','ShinyTexts.cpp')+'\n    '+entry.replace('MoneyMessages.c','ShinyTexts.c'))
            else:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\ShinyTexts.cpp" />\n    '+entry.replace('MoneyMessages.c','ShinyTexts.c'))
    for name in ['ShinyTexts.h','ShinyTexts_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    return bright.project(source)
