"""Move the complete special-effects controller, snapshot state and callback into C."""
import re
from audio_controls_migration import function
import object_names_migration as objects
import money_messages_migration as money
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
extern bool SpecialFX_bVideoCam,SpecialFX_bLiftCam,SpecialFX_bSnapShotActive;
extern int32_t SpecialFX_SnapShotFrames;
void SpecialFX_Init(void);
void SpecialFX_Update(void);
void SpecialFX_Shutdown(void);
void SpecialFX_Render(void);
void SpecialFX_AddWeaponStreak(int32_t type);
void SpecialFX_Render2DFXs(void);
void *SpecialFX_LookForBat(void *object,void *data);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
MATRIX='typedef struct SpecialFXMatrix { float rx,ry,rz,fx,fy,fz,ux,uy,uz,px,py,pz; } SpecialFXMatrix;\n'
READERS=[
('void *','WeaponMatrix','void','static CMatrix matrix;return &matrix;','Retain the original lazy weapon matrix and its C++ lifetime'),
('void *','CallbackMatrix','void','static CMatrix matrix;return &matrix;','Retain the independent original lazy callback matrix'),
('void','SetMatrix','void *target,void *source','*(CMatrix *)target=CMatrix((RwMatrix *)source);','Preserve original typed temporary construction and matrix assignment'),
('SpecialFXMatrix','ReadMatrix','void *matrix','const CMatrix &m=*(CMatrix *)matrix;\n    SpecialFXMatrix result={m.rx,m.ry,m.rz,m.fx,m.fy,m.fz,m.ux,m.uy,m.uz,m.px,m.py,m.pz};return result;','Copy actual named matrix coefficients without assuming its binary layout'),
('void *','PlayerPed','void','return FindPlayerPed();','Preserve every original live player-ped query'),
('void *','Weapon','void *ped','return ((CPlayerPed *)ped)->m_pWeaponModel;','Read the actual player weapon-model field at its original use'),
('void *','AtomicFrame','void *atomic','return RpAtomicGetFrame((RpAtomic *)atomic);','Read the original typed atomic frame'),
('void *','FrameLTM','void *frame','return RwFrameGetLTM((RwFrame *)frame);','Enter the original typed frame local-to-world operation'),
('void *','ModelInfo','void *object','return CVisibilityPlugins::GetAtomicModelInfo((RpAtomic *)object);','Read the original typed atomic model-info pointer'),
('float','ScreenWidth','void','return SCREEN_WIDTH;','Read the original live screen-width macro'),
('float','ScreenHeight','void','return SCREEN_HEIGHT;','Read the original live screen-height macro'),
('char *','String','void','return gString;','Borrow the actual shared ASCII buffer'),
('uint16_t *','Unicode','void','return gUString;','Borrow the actual shared game Unicode buffer'),
('void','AsciiToUnicode','const char *source,uint16_t *target','AsciiToUnicode(source,target);','Enter the original typed game text converter'),
('void','FontScale','float x,float y','CFont::SetScale(x,y);','Set the original font scale with C-computed values'),
('void','FontCentreSize','float value','CFont::SetCentreSize(value);','Retain the original nominally unused centre-size write'),
('void','FontColor','uint8_t r,uint8_t g,uint8_t b,uint8_t a','CFont::SetColor(CRGBA(r,g,b,a));','Construct the original typed font color after original narrowing'),
('void','FontStyle','int16_t style','CFont::SetFontStyle(style);','Set the original typed font style selected by C'),
('bool','Japanese','void','#ifdef MORE_LANGUAGES\n    return CFont::IsJapanese();\n#else\n    return false;\n#endif','Read the original locale predicate only when the original macro uses it'),
('void','PrintString','float x,float y,uint16_t *text','CFont::PrintString(x,y,text);','Print through the exact original typed Unicode overload'),
('void','Polygon','float x1,float y1,float x2,float y2,float x3,float y3,float x4,float y4,uint8_t r,uint8_t g,uint8_t b,uint8_t a','CSprite2d::Draw2DPolygon(x1,y1,x2,y2,x3,y3,x4,y4,CRGBA(r,g,b,a));','Construct the original typed polygon color at its use point'),
('void','Rect','float left,float top,float right,float bottom,uint8_t r,uint8_t g,uint8_t b,uint8_t a','CSprite2d::DrawRect(CRect(left,top,right,bottom),CRGBA(r,g,b,a));','Retain the exact original rectangle constructor argument order'),
('void','Push','const char *name','PUSH_RENDERGROUP(name);','Preserve the original debug label and FINAL configuration'),
('void','Pop','void','POP_RENDERGROUP();','Close the original debug group'),
('bool','NewRenderer','void','#ifdef NEW_RENDERER\n    return gbNewRenderer;\n#else\n    return false;\n#endif','Read the original optional renderer flag at its short-circuit operand'),
('bool','FirstPerson','void','#ifdef NEW_RENDERER\n    return FredIsInFirstPersonCam();\n#else\n    return false;\n#endif','Enter the original optional first-person camera query')]
for name in ['JustifyOff','BackgroundOff','CentreOff','PropOn']:READERS.append(('void','Font'+name,'void','CFont::Set'+name+'();','Preserve the original typed font '+name+' operation'))
for owner,methods in [('BrightLights',['Init','Render']),('ShinyTexts',['Init','Render']),('MoneyMessages',['Init','Render']),('3dMarkers',['Init','Render','Shutdown'])]:
    for method in methods:READERS.append(('void',owner+method,'void','C'+owner+'::'+method+'();','Enter the original '+owner+' '+method+' operation'))
for name,state,standard in [('SrcBlend','SRCBLEND','rwBLENDSRCALPHA'),('DestBlend','DESTBLEND','rwBLENDINVSRCALPHA')]:READERS.append(('void',name,'bool additive','RwRenderStateSet(rwRENDERSTATE'+state+',(void *)(uintptr_t)(additive?rwBLENDONE:'+standard+'));','Set the original typed '+name+' factor'))
BACKEND_HEADER='//+ rouz edit (ChatGPT)\n#pragma once\n#include "SpecialFX_control.h"\n#include "MotionBlurStreaks.h"\n'+MATRIX+'#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(result+' SpecialFX_Backend'+name+'('+args+');\n' for result,name,args,body,comment in READERS)+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
HOST='''//+ rouz edit (ChatGPT)
#include "SpecialFX_backend.h"
'''+''.join(result+' SpecialFX_Backend'+name+'('+args+')\n{\n    // '+comment+'\n    '+body+'\n}\n' for result,name,args,body,comment in READERS)+'''
RwObject *LookForBatCB(RwObject *object,void *data)
{
    // Keep the original typed callback entry while C controls every decision and computation
    return (RwObject *)SpecialFX_LookForBat(object,data);
}
//- rouz edit (ChatGPT)
'''
def section(source):
    # Delimit every complete controller method and the associated unused callback
    a=source.index('bool CSpecialFX::bVideoCam;');b=source.index('RpAtomic *\nMarkerAtomicCB(',a);return source[a:b]

def font_enum(source):
    # Extract the entire original global font-style enum including language guards
    a=source.index('enum {\n\tFONT_BANK,');b=source.index('\n};',a)+3;return source[a:b]

def font_styles(source):
    # Share the original plain enum without changing any value or conditional member
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include "config.h"\n'+font_enum(source)+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Remove the former owner and retain all other special-effect algorithms
    if 'enum {\n\tFONT_BANK,' in source:source=source.replace(font_enum(source),'#include "FontStyles.h"')
    if 'TimerHost_SnapshotActive(void)\n{' in source:
        old='extern "C" bool\n'+function(source,'TimerHost_','SnapshotActive')+'\n'
        assert old in source
        source=source.replace(old,'').replace('#include "SpecialFX.h"\n','')
    if 'CSpecialFX::' not in source and 'class CSpecialFX\n' not in source:return money.calls(source)
    if 'class CSpecialFX\n' in source:source=re.sub(r'class CSpecialFX\n\{.*?\n\};','#include "SpecialFX_control.h"',source,flags=re.S)
    if 'bool CSpecialFX::bVideoCam;' in source:source=source.replace(section(source),'')+'\n'+HOST
    source=source.replace('CSpecialFX::','SpecialFX_')
    source=source.replace('"SpecialFX_Render"','"CSpecialFX::Render"')
    lines=[]
    for line in source.splitlines(keepends=True):
        if re.search(r'\bSpecialFX_(?:Init|Shutdown|Update|Render|Render2DFXs|AddWeaponStreak)\(',line) and 'normally rendered in' not in line:lines.append(re.match(r'\s*',line)[0]+'// Enter the C special-effects controller with the original arguments\n')
        lines.append(line)
    return money.calls(''.join(lines))

def scaling(source):
    # Preserve the complete original scaling expressions and all configuration guards
    a=source.index('#define SCREEN_STRETCH_X(');b=source.index('#include "maths.h"',a)
    return '#define SCREEN_WIDTH (SpecialFX_BackendScreenWidth())\n#define SCREEN_HEIGHT (SpecialFX_BackendScreenHeight())\n#define SCREEN_ASPECT_RATIO (Draw_GetAspectRatio())\n'+source[a:b]

def kernel(source,common):
    # Relocate all six methods, callback, fields and overlay computations
    body=section(source);body=body.replace('static CMatrix matrix;','void *matrix=SpecialFX_BackendWeaponMatrix();').replace('static CMatrix MatLTM;','void *MatLTM=SpecialFX_BackendCallbackMatrix();')
    body=body.replace('CSpecialFX::','SpecialFX_').replace('"SpecialFX_Render"','"CSpecialFX::Render"')
    body=body.replace('RwObject*\nLookForBatCB(RwObject *object, void *data)','void *\nSpecialFX_LookForBat(void *object,void *data)')
    body=body.replace('FindPlayerPed()->m_pWeaponModel','SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed())').replace('FindPlayerPed()','SpecialFX_BackendPlayerPed()')
    body=body.replace('CVisibilityPlugins::GetAtomicModelInfo((RpAtomic*)object)','SpecialFX_BackendModelInfo(object)').replace('(CSimpleModelInfo*)data','data')
    body=body.replace('RpAtomicGetFrame(SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()))','SpecialFX_BackendAtomicFrame(SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()))').replace('RpAtomicGetFrame((RpAtomic*)object)','SpecialFX_BackendAtomicFrame(object)')
    body=body.replace('matrix = RwFrameGetLTM(SpecialFX_BackendAtomicFrame(SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed())));','SpecialFX_BackendSetMatrix(matrix,SpecialFX_BackendFrameLTM(SpecialFX_BackendAtomicFrame(SpecialFX_BackendWeapon(SpecialFX_BackendPlayerPed()))));')
    body=body.replace('MatLTM = CMatrix(RwFrameGetLTM(SpecialFX_BackendAtomicFrame(object)));','SpecialFX_BackendSetMatrix(MatLTM,SpecialFX_BackendFrameLTM(SpecialFX_BackendAtomicFrame(object)));')
    body=re.sub(r'(matrix|MatLTM) \* CVector\(([^)]+)\)',r'SpecialFX_Transform(\1,(MotionBlurStreakVector){\2})',body).replace('CVector ','MotionBlurStreakVector ')
    body=body.replace('MotionBlurStreaks_RegisterStreakLegacy(','MotionBlurStreaks_RegisterStreak(')
    for result,name,args,code,comment in READERS:
        if name.startswith(('BrightLights','ShinyTexts','MoneyMessages','3dMarkers')):
            owner,method=next((owner,method) for owner in ['BrightLights','ShinyTexts','MoneyMessages','3dMarkers'] for method in ['Init','Render','Shutdown'] if name==owner+method)
            body=body.replace('C'+owner+'::'+method+'()','SpecialFX_Backend'+name+'()')
    body=body.replace('gbNewRenderer','SpecialFX_BackendNewRenderer()').replace('FredIsInFirstPersonCam()','SpecialFX_BackendFirstPerson()')
    body=body.replace('PUSH_RENDERGROUP(','SpecialFX_BackendPush(').replace('POP_RENDERGROUP()','SpecialFX_BackendPop()')
    for a,b in [('SetScale','FontScale'),('SetJustifyOff','FontJustifyOff'),('SetBackgroundOff','FontBackgroundOff'),('SetCentreSize','FontCentreSize'),('SetCentreOff','FontCentreOff'),('SetPropOn','FontPropOn'),('SetFontStyle','FontStyle'),('PrintString','PrintString')]:body=body.replace('CFont::'+a+'(','SpecialFX_Backend'+b+'(')
    body=re.sub(r'CFont::SetColor\(CRGBA\(([^)]+)\)\)',r'SpecialFX_BackendFontColor(\1)',body)
    body=body.replace('FONT_LOCALE(FONT_STANDARD)','SPECIAL_FX_FONT_LOCALE(FONT_STANDARD)')
    body=body.replace('gString','SpecialFX_BackendString()').replace('gUString','SpecialFX_BackendUnicode()').replace('AsciiToUnicode(','SpecialFX_BackendAsciiToUnicode(')
    body=re.sub(r'CSprite2d::Draw2DPolygon\(([^;\n]+), CRGBA\(([^)]+)\)\)',r'SpecialFX_BackendPolygon(\1,\2)',body)
    body=re.sub(r'CSprite2d::DrawRect\(CRect\(([^)]+)\), CRGBA\(([^)]+)\)\)',r'SpecialFX_BackendRect(\1,\2)',body)
    for name,state,standard in [('SrcBlend','SRCBLEND','rwBLENDSRCALPHA'),('DestBlend','DESTBLEND','rwBLENDINVSRCALPHA')]:
        for factor,value in [('rwBLENDONE','true'),(standard,'false')]:body=re.sub(r'RwRenderStateSet\(rwRENDERSTATE'+state+r', \(void\s*\*\)'+factor+r'\)',r'SpecialFX_Backend'+name+'('+value+')',body)
    body=body.replace('nil','NULL');body=re.sub(r'\bint32\b','int32_t',body);body=re.sub(r'\buintptr\b','uintptr_t',body)
    body=body.replace('\n{\n','\n{\n\t// Preserve the complete original effect, overlay and state-update order\n');body=re.sub(r'^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Retain original live reads, short circuits and writes\n',body,flags=re.M)
    helpers='''#ifdef MORE_LANGUAGES
#define SPECIAL_FX_FONT_LOCALE(style) (SpecialFX_BackendJapanese()?FONT_JAPANESE:(style))
#else
#define SPECIAL_FX_FONT_LOCALE(style) (style)
#endif
static MotionBlurStreakVector SpecialFX_Transform(void *matrix,MotionBlurStreakVector v)
{
    // Retain the complete original matrix-vector products and additions in C
    SpecialFXMatrix m=SpecialFX_BackendReadMatrix(matrix);
    MotionBlurStreakVector result={m.rx*v.x+m.fx*v.y+m.ux*v.z+m.px,m.ry*v.x+m.fy*v.y+m.uy*v.z+m.py,m.rz*v.x+m.fz*v.y+m.uz*v.z+m.pz};return result;
}
bool TimerHost_SnapshotActive(void)
{
    // Read the same live snapshot flag now owned by the C controller
    return SpecialFX_bSnapShotActive;
}
'''
    assert not re.search(r'\b(?:CSpecialFX|CMatrix|CVector|CSprite2d|CFont|CRGBA|CRect|CTimeCycle|RwObject|RwRenderStateSet)\b',re.sub(r'"[^"\n]*"','',body))
    return money.calls('//+ rouz edit (ChatGPT)\n#include "SpecialFX_backend.h"\n#include "BulletTraces.h"\n#include "Timer.h"\n#include "General.h"\n#include "Draw.h"\n#include "Draw_constants.h"\n#include "FontStyles.h"\n#include "WeaponType.h"\n#include <stdio.h>\n#include <stddef.h>\n\n'+scaling(common)+helpers+body.rstrip()+'\n//- rouz edit (ChatGPT)\n')

def project(source):
    # Compile the C controller alongside the retained typed effects with distinct objects
    if '..\\src\\renderer\\SpecialFX.c"' not in source:
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.cpp" />';addition='<ClCompile Include="..\\src\\renderer\\SpecialFX.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+addition)
        entry='<ClCompile Include="..\\src\\renderer\\SpecialFX.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>';source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.cpp','SpecialFX.c'))
    for name in ['SpecialFX_control.h','SpecialFX_backend.h','FontStyles.h']:
        if '..\\src\\renderer\\'+name+'"' in source:continue
        for entry in ['<ClInclude Include="..\\src\\renderer\\SpecialFX.h" />','<ClInclude Include="..\\src\\renderer\\SpecialFX.h">\n      <Filter>renderer</Filter>\n    </ClInclude>']:source=source.replace(entry,entry+'\n    '+entry.replace('SpecialFX.h',name))
    return money.project(source)
