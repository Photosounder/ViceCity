"""Convert both complete streak classes and their ordered shared geometry initialization."""
import re
import object_names_migration as objects
import bullet_traces_migration as bullet_traces

HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
typedef struct MotionBlurStreakVector { float x,y,z; } MotionBlurStreakVector;
typedef struct MotionBlurStreak {
    uintptr_t m_id;
    uint8_t m_red,m_green,m_blue;
    MotionBlurStreakVector m_pos1[3],m_pos2[3];
    bool m_isValid[3];
} MotionBlurStreak;
#ifdef __cplusplus
extern "C" {
#endif
void MotionBlurStreak_Update(MotionBlurStreak *streak);
void MotionBlurStreak_Render(MotionBlurStreak *streak);
void MotionBlurStreaks_InitGeometry(void);
void MotionBlurStreaks_Init(void);
void MotionBlurStreaks_Update(void);
void MotionBlurStreaks_RegisterStreak(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,MotionBlurStreakVector p1,MotionBlurStreakVector p2);
bool MotionBlurStreaks_IsRegisteredForCurrentFrame(uintptr_t id);
void MotionBlurStreaks_Render(void);
#ifdef __cplusplus
}
class CVector;
MotionBlurStreakVector MotionBlurStreaks_FromVector(CVector value);
void MotionBlurStreaks_RegisterStreakLegacy(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,CVector p1,CVector p2);
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('void','FogColor','uint32_t color','RwRenderStateSet(rwRENDERSTATEFOGCOLOR,(void *)(uintptr_t)color);','Set the original typed packed fog-color state'),
('void','SrcBlend','void','RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);','Select the original source blend factor'),
('void','DestBlend','void','RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);','Select the original destination blend factor'),
('void','Raster','void','RwRenderStateSet(rwRENDERSTATETEXTURERASTER,(void *)FALSE);','Clear the original typed texture-raster state'),
('void','LittleTest','void','LittleTest();','Preserve the original shared-buffer hook'),
('bool','Transform','void','return RwIm3DTransform(StreakVertices,4,nil,rwIM3D_VERTEXUV)!=nil;','Transform the original typed four-vertex array with UV flags'),
('void','RenderIndexed','uint16_t *indices','RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,indices,12);','Render all twelve C-owned indices through the original typed primitive'),
('void','End','void','RwIm3DEnd();','Complete the original successful immediate draw')]
for name,state in [('ZWrite','ZWRITEENABLE'),('VertexAlpha','VERTEXALPHAENABLE'),('Fog','FOGENABLE')]:READERS.append(('void',name,'bool value','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)value);','Set the original typed '+name+' state'))
for name,args,values in [('U','float value','value'),('V','float value','value'),('RGBA','int r,int g,int b,int a','r,g,b,a'),('Pos','float x,float y,float z','x,y,z')]:READERS.append(('void','Set'+name,'int index,'+args,'RwIm3DVertexSet'+name+'(&StreakVertices[index],'+values+');','Write the shared typed vertex '+name+' fields'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "MotionBlurStreaks.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' MotionBlurStreaks_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Timecycle.h"
#include "MotionBlurStreaks_backend.h"
static RwIm3DVertex StreakVertices[4];
'''+''.join(result+' MotionBlurStreaks_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'''
MotionBlurStreakVector MotionBlurStreaks_FromVector(CVector value)
{
    // Copy actual typed components without assuming legacy object layout
    MotionBlurStreakVector result={value.x,value.y,value.z};return result;
}
void MotionBlurStreaks_RegisterStreakLegacy(uintptr_t id,uint8_t r,uint8_t g,uint8_t b,CVector p1,CVector p2)
{
    // Retain original by-value input copies before entering the complete C registration
    MotionBlurStreaks_RegisterStreak(id,r,g,b,MotionBlurStreaks_FromVector(p1),MotionBlurStreaks_FromVector(p2));
}
//- rouz edit (ChatGPT)
'''

def section(source):
    # Delimit the seven complete streak methods without touching the neighboring trace algorithms
    a=source.index('CRegisteredMotionBlurStreak CMotionBlurStreaks::aStreaks[NUMMBLURSTREAKS];');b=source.index('CBulletTrace CBulletTraces::aTraces[NUMBULLETTRACES];',a)
    return source[a:b]

def geometry(source):
    # Preserve every original UV and index assignment at its special-effect initialization point
    a=source.index('\tRwIm3DVertexSetU(&StreakVertices[0]');b=source.index('\n\tCMotionBlurStreaks::Init();',a)
    return source[a:b]

def calls(source):
    # Remove both owners and adapt typed consumers at explicit input boundaries
    if 'CRegisteredMotionBlurStreak' not in source and 'CMotionBlurStreaks::' not in source:return bullet_traces.calls(source)
    if 'class CRegisteredMotionBlurStreak\n' in source:
        a=source.index('class CRegisteredMotionBlurStreak\n');b=source.index('struct CBulletTrace',a)
        source=source[:a]+'#include "MotionBlurStreaks.h"\n\n'+source[b:]
    if 'CRegisteredMotionBlurStreak CMotionBlurStreaks::aStreaks[NUMMBLURSTREAKS];' in source:
        initialization=geometry(source);source=source.replace(section(source),'')
        source=source.replace('RwIm3DVertex StreakVertices[4];\n','').replace('RwImVertexIndex StreakIndexList[12];\n','')
        source=source.replace(initialization,'\t// Initialize streak geometry at the original point in the shared effect setup\n\tMotionBlurStreaks_InitGeometry();\n')
    source=source.replace('CRegisteredMotionBlurStreak','MotionBlurStreak')
    source=source.replace('CMotionBlurStreaks::RegisterStreak(','MotionBlurStreaks_RegisterStreakLegacy(').replace('CMotionBlurStreaks::','MotionBlurStreaks_')
    source=source.replace('reflectionStreaks[i].Render();','MotionBlurStreak_Render(&reflectionStreaks[i]);')
    source=re.sub(r'(streak->m_pos[12]\[[01]\] = )([^;\n]+);',r'\1MotionBlurStreaks_FromVector(\2);',source)
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bMotionBlurStreaks_(?:Init|Update|RegisterStreakLegacy|Render|IsRegisteredForCurrentFrame)\(',line) or 'MotionBlurStreak_Render(&reflectionStreaks[i])' in line:
            lines.append(re.match(r'\s*',line)[0]+'// Enter the C motion-blur streak algorithm with the original owner expressions\n')
        elif re.search(r'streak->m_pos[12]\[[01]\] = MotionBlurStreaks_FromVector',line):lines.append(re.match(r'\s*',line)[0]+'// Copy the original reflection sample into its plain C vector record\n')
        lines.append(line)
    return bullet_traces.calls(''.join(lines))

def kernel(source):
    # Relocate every original method and the complete shared geometry setup
    setup=geometry(source);body=section(source)
    body=body.replace('CRegisteredMotionBlurStreak CMotionBlurStreaks::aStreaks[NUMMBLURSTREAKS];','static MotionBlurStreak aStreaks[NUMMBLURSTREAKS];')
    body=body.replace('CRegisteredMotionBlurStreak::Update(void)','MotionBlurStreak_Update(MotionBlurStreak *streak)').replace('CRegisteredMotionBlurStreak::Render(void)','MotionBlurStreak_Render(MotionBlurStreak *streak)')
    a=body.index('void\nMotionBlurStreak_Update');b=body.index('void\nCMotionBlurStreaks::Init',a)
    member=body[a:b];member=re.sub(r'(?<![\w.>])\b(m_id|m_red|m_green|m_blue|m_pos1|m_pos2|m_isValid)\b',r'streak->\1',member);body=body[:a]+member+body[b:]
    body=body.replace('CMotionBlurStreaks::','MotionBlurStreaks_').replace('uintptr id','uintptr_t id').replace('uint8 r, uint8 g, uint8 b, CVector p1, CVector p2','uint8_t r, uint8_t g, uint8_t b, MotionBlurStreakVector p1, MotionBlurStreakVector p2')
    body=body.replace('aStreaks[i].Update()','MotionBlurStreak_Update(&aStreaks[i])').replace('aStreaks[i].Render()','MotionBlurStreak_Render(&aStreaks[i])')
    old='RwRenderStateSet(rwRENDERSTATEFOGCOLOR,\n\t\t\t\t\t(void*)RWRGBALONG(CTimeCycle::GetFogRed(), CTimeCycle::GetFogGreen(), CTimeCycle::GetFogBlue(), 255));'
    assert old in body
    body=body.replace(old,'MotionBlurStreaks_BackendFogColor(((uint32_t)255 << 24) | ((uint32_t)TimeCycle_GetFogRed() << 16) | ((uint32_t)TimeCycle_GetFogGreen() << 8) | (uint32_t)TimeCycle_GetFogBlue());')
    for state,name in [('ZWRITEENABLE','ZWrite'),('VERTEXALPHAENABLE','VertexAlpha'),('FOGENABLE','Fog')]:body=re.sub(r'RwRenderStateSet\(rwRENDERSTATE'+state+r', \(void\s*\*\)(TRUE|FALSE)\)',lambda match:'MotionBlurStreaks_Backend'+name+'('+('true' if match[1]=='TRUE' else 'false')+')',body)
    body=body.replace('RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA)','MotionBlurStreaks_BackendSrcBlend()').replace('RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA)','MotionBlurStreaks_BackendDestBlend()').replace('RwRenderStateSet(rwRENDERSTATETEXTURERASTER, (void*)FALSE)','MotionBlurStreaks_BackendRaster()')
    setup=re.sub(r'\bStreakIndexList\b','StreakIndexList',setup)
    combined='static uint16_t StreakIndexList[12];\n\nvoid MotionBlurStreaks_InitGeometry(void)\n{\n'+setup+'}\n\n'+body
    combined=re.sub(r'RwIm3DVertexSet(RGBA|Pos|U|V)\(&StreakVertices\[([^\]]+)\],',r'MotionBlurStreaks_BackendSet\1(\2,',combined)
    combined=combined.replace('LittleTest()','MotionBlurStreaks_BackendLittleTest()').replace('RwIm3DTransform(StreakVertices, 4, nil, rwIM3D_VERTEXUV)','MotionBlurStreaks_BackendTransform()').replace('RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST, StreakIndexList, 12)','MotionBlurStreaks_BackendRenderIndexed(StreakIndexList)').replace('RwIm3DEnd()','MotionBlurStreaks_BackendEnd()')
    combined=combined.replace('\n{\n','\n{\n\t// Preserve the original ordered sample, pool and shared-geometry operations\n')
    combined=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Retain the original short circuits, initialization and write order\n',combined,flags=re.M)
    assert not re.search(r'\b(?:CRegisteredMotionBlurStreak|CMotionBlurStreaks|CVector|RwIm3DVertex|CTimeCycle)\b',combined)
    return '//+ rouz edit (ChatGPT)\n#include "MotionBlurStreaks_backend.h"\n#include "Timecycle.h"\n\n'+combined.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register complete C11 logic and its new typed C++ backend with unique objects
    if '..\\src\\renderer\\MotionBlurStreaks.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.cpp" />'
        source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\MotionBlurStreaks.cpp" />\n    <ClCompile Include="..\\src\\renderer\\MotionBlurStreaks.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>'
        source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.cpp','MotionBlurStreaks.cpp')+'\n    '+entry.replace('SpecialFX.cpp','MotionBlurStreaks.c'))
    for name in ['MotionBlurStreaks.h','MotionBlurStreaks_backend.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    return bullet_traces.project(objects.project(source))
