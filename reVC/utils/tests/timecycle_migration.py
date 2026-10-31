"""Move the complete lighting/time-cycle owner, tables and interpolation into C11."""
import re
import object_names_migration as objects
OBSERVER=None
TYPES={'uint8':'uint8_t','int8':'int8_t','int16':'int16_t','int32':'int32_t','CVector':'TimeCycleVector'}
OPERATIONS=['Initialise','Update','InterpolateI8','InterpolateU8','InterpolateI16','StartExtraColour','StopExtraColour']

def fields(header):
    # Retain every complete original static declaration including its exact dimensions
    return re.findall(r'(?m)^\s*static (uint8|int8|int16|int32|float|CVector) (m_\w+)([^;\n]*);',header)

def getters(header):
    # Keep all original inline accessor expressions and their exact declared return types
    return re.findall(r'static (float|int32|CVector &)\s*(Get\w+)\(void\) \{ return ([^;]+); \}',header)

def scalar(text):
    # Preserve signed widths and replace only original scalar and vector spellings
    for old,new in TYPES.items():text=re.sub(r'\b'+old+r'\b',new,text)
    return text

def header(original):
    # Expose complete canonical C storage and operations while preserving a mutable borrowed sun-vector accessor
    text='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "config.h"\ntypedef struct TimeCycleVector {float x,y,z;} TimeCycleVector;\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    for kind,name,dimensions in fields(original):text+='extern '+TYPES.get(kind,kind)+' TimeCycle_'+name+dimensions+';\n'
    text+='void TimeCycle_Initialise(void);\nvoid TimeCycle_Update(void);\n'
    for suffix,kind in [('I8','int8_t'),('U8','uint8_t'),('I16','int16_t')]:text+='float TimeCycle_Interpolate'+suffix+'('+kind+' *a,'+kind+' *b);\n'
    text+='void TimeCycle_StartExtraColour(int32_t c,bool fade);\nvoid TimeCycle_StopExtraColour(bool fade);\n'
    for kind,name,expression in getters(original):text+=('TimeCycleVector *' if kind=='CVector &' else scalar(kind)+' ')+ 'TimeCycle_'+name+'(void);\n'
    return text+'#ifdef __cplusplus\n}\nclass CVector;\nCVector TimeCycle_ToNativeVector(TimeCycleVector value);\n#endif\n//- rouz edit (ChatGPT)\n'

BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "Timecycle.h"
enum {TIMECYCLE_MOTION_BLUR_NONE=0,TIMECYCLE_MOTION_BLUR_LIGHT_SCENE=2};
#ifdef __cplusplus
extern "C" {
#endif
void TimeCycle_BackendDebug(const char *message);
uint8_t (*TimeCycle_BackendWorkBuffer(void))[55000];
int32_t TimeCycle_BackendBlurType(void);
void TimeCycle_BackendSetMotionBlur(int32_t red,int32_t green,int32_t blue,int32_t alpha,int32_t type);
void TimeCycle_BackendShadowValues(TimeCycleVector direction,float *frontX,float *frontY,float *sideX,float *sideY,float *displacementX,float *displacementY);
float TimeCycle_BackendCameraForwardZ(void);
bool TimeCycle_BackendCutscene(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"
#include "Camera.h"
#include "Shadows.h"
#include "CutsceneMgr.h"
#include "Timecycle_backend.h"
static_assert(int(TIMECYCLE_MOTION_BLUR_NONE)==int(MOTION_BLUR_NONE)&&int(TIMECYCLE_MOTION_BLUR_LIGHT_SCENE)==int(MOTION_BLUR_LIGHT_SCENE),"Original native blur constants");
CVector TimeCycle_ToNativeVector(TimeCycleVector value)
{
    // Copy the C value explicitly for native vector operations while canonical storage remains in C
    return CVector(value.x,value.y,value.z);
}
void TimeCycle_BackendDebug(const char *message)
{
    // Retain the original formatted debug output without moving any loading decisions into C++
    debug("%s",message);
}
uint8_t (*TimeCycle_BackendWorkBuffer(void))[55000]
{
    // Borrow the complete original shared file buffer with its actual compile-time extent
    return &work_buff;
}
int32_t TimeCycle_BackendBlurType(void)
{
    // Read the live camera blur type separately at each original short-circuit access
    return TheCamera.m_BlurType;
}
void TimeCycle_BackendSetMotionBlur(int32_t red,int32_t green,int32_t blue,int32_t alpha,int32_t type)
{
    // Invoke the original camera primitive after its original argument conversions
    TheCamera.SetMotionBlur(red,green,blue,alpha,type);
}
void TimeCycle_BackendShadowValues(TimeCycleVector direction,float *frontX,float *frontY,float *sideX,float *sideY,float *displacementX,float *displacementY)
{
    // Preserve the native shadow primitive and the six live output addresses selected by the C owner
    CShadows::CalcPedShadowValues(TimeCycle_ToNativeVector(direction),frontX,frontY,sideX,sideY,displacementX,displacementY);
}
float TimeCycle_BackendCameraForwardZ(void)
{
    // Read the live camera forward vector only after the original shadow calculation
    return TheCamera.GetForward().z;
}
bool TimeCycle_BackendCutscene(void)
{
    // Query cutscene playback only after the original force-rain and cull-zone tests
    return CCutsceneMgr::IsRunning();
}
//- rouz edit (ChatGPT)
'''

def kernel(source,originalHeader,vectorSource):
    # Retain the whole original data loader and update body while removing class scopes and overloads
    start=source.index('uint8 CTimeCycle::m_nAmbientRed[');source=source[start:]
    source=source[:source.rfind('//- rouz edit (ChatGPT)')].rstrip()
    source=re.sub(r'(?m)^//[+-] rouz edit \(ChatGPT\)\n','',source)
    for old,suffix in [('int8','I8'),('uint8','U8'),('int16','I16')]:source=source.replace('CTimeCycle::Interpolate('+old+' *a','TimeCycle_Interpolate'+suffix+'('+old+' *a')
    source=source.replace('CTimeCycle::','TimeCycle_')
    for kind,name,dimensions in fields(originalHeader):source=re.sub(r'(?<![\w])'+name+r'\b','TimeCycle_'+name,source)
    source=source.replace('#define INTERP(v) Interpolate(v[h1], v[h2])','')
    for kind,name,dimensions in fields(originalHeader):
        if dimensions=='[NUMHOURS][NUMWEATHERS]':
            suffix={'int8':'I8','uint8':'U8','int16':'I16'}[kind];source=source.replace('INTERP(TimeCycle_'+name+')','TimeCycle_Interpolate'+suffix+'(TimeCycle_'+name+'[h1],TimeCycle_'+name+'[h2])')
    source=source.replace('CVector &sunPos = GetSunDirection();','TimeCycleVector *sunPos = TimeCycle_GetSunDirection();').replace('sunPos.Normalise();','TimeCycle_Normalise(sunPos);').replace('sunPos.','sunPos->')
    source=source.replace('CShadows::CalcPedShadowValues(sunPos,','TimeCycle_BackendShadowValues(*sunPos,')
    source=source.replace('TheCamera.m_BlurType','TimeCycle_BackendBlurType()').replace('TheCamera.SetMotionBlur(','TimeCycle_BackendSetMotionBlur(').replace('TheCamera.GetForward().z','TimeCycle_BackendCameraForwardZ()').replace('CCutsceneMgr::IsRunning()','TimeCycle_BackendCutscene()')
    source=source.replace('MOTION_BLUR_NONE','TIMECYCLE_MOTION_BLUR_NONE').replace('MOTION_BLUR_LIGHT_SCENE','TIMECYCLE_MOTION_BLUR_LIGHT_SCENE')
    source=re.sub(r'\bwork_buff\b','(*TimeCycle_BackendWorkBuffer())',source)
    source=source.replace('debug(','TimeCycle_BackendDebug(').replace('Sin(sunAngle)','sinf(sunAngle)').replace('Cos(sunAngle)','cosf(sunAngle)')
    source=scalar(source)
    source=re.sub(r'(TimeCycle_(?:'+ '|'.join(OPERATIONS)+r')\([^\n]*\)\n\{)',lambda match:match[0]+'\n    // Preserve the complete original time-cycle operation and its ordered live accesses',source)
    normalise=vectorSource[vectorSource.index('void\nCVector::Normalise(void)'):vectorSource.index('\n}',vectorSource.index('void\nCVector::Normalise(void)'))+2]
    normalise=normalise.replace('void\nCVector::Normalise(void)','static void TimeCycle_Normalise(TimeCycleVector *value)').replace('MagnitudeSqr()','value->x*value->x + value->y*value->y + value->z*value->z').replace('RecipSqrt(sq)','1.0f/sqrtf(sq)')
    for field in ['x','y','z']:normalise=re.sub(r'(?<![\w>])'+field+r'\b','value->'+field,normalise)
    normalise=normalise.replace('\n{','\n{\n    // Preserve the complete original vector normalization arithmetic and zero-vector fallback',1)
    accessors=[]
    for kind,name,expression in getters(originalHeader):
        for fieldKind,field,dimensions in fields(originalHeader):expression=re.sub(r'\b'+field+r'\b','TimeCycle_'+field,expression)
        if kind=='CVector &':kind='TimeCycleVector *';expression='&'+expression
        accessors.append(scalar(kind)+' TimeCycle_'+name+'(void)\n{\n    // Preserve the original inline accessor value or live borrowed storage\n    return '+expression+';\n}')
    prefix='//+ rouz edit (ChatGPT)\n#define _CRT_SECURE_NO_WARNINGS\n#define _USE_MATH_DEFINES\n#include "Timecycle.h"\n#include "Timecycle_backend.h"\n#include "Clock.h"\n#include "Weather.h"\n#include "Timer.h"\n#include "ZoneCull.h"\n#include "FileMgr.h"\n#include "Draw_constants.h"\n#include <stdio.h>\n#include <math.h>\n#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n#define Min(a,b) ((a)<(b)?(a):(b))\n#define Max(a,b) ((a)>(b)?(a):(b))\n'
    return prefix+normalise+'\n\n'+source+'\n\n'+'\n\n'.join(accessors)+'\n//- rouz edit (ChatGPT)\n'

def cycle_calls(source):
    # Remove the complete owner and convert every qualified caller without changing scalar accessor semantics
    if 'class CTimeCycle\n' in source:return header(source)
    if 'uint8 CTimeCycle::m_nAmbientRed[' in source:
        if OBSERVER:OBSERVER(source)
        return HOST
    old=source
    source=re.sub(r'CTimeCycle::GetSunDirection\(\)\.([xyz])',r'TimeCycle_GetSunDirection()->\1',source)
    source=source.replace('CTimeCycle::GetSunDirection()','TimeCycle_ToNativeVector(*TimeCycle_GetSunDirection())')
    source=source.replace('vecsun = CTimeCycle::m_VectorToSun[CTimeCycle::m_CurrentStoredValue];','vecsun = TimeCycle_ToNativeVector(TimeCycle_m_VectorToSun[TimeCycle_m_CurrentStoredValue]);')
    source=source.replace('CTimeCycle::','TimeCycle_')
    if source==old:return source
    if old.lstrip().startswith('//+ rouz edit (ChatGPT)') and old.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def cycle_project(source):
    # Register a uniquely named C table/update object beside the existing typed native bindings
    for tag,old,new in [('ClCompile','Timecycle.cpp','TimeCycleSystem.c'),('ClInclude','Timecycle.h','Timecycle_backend.h')]:
        entry='<'+tag+' Include="..\\src\\renderer\\'+old+'" />'
        addition='<'+tag+' Include="..\\src\\renderer\\'+new+'"'+('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if tag=='ClCompile' else ' />')
        if '..\\src\\renderer\\'+new+'"' not in source:
            source=source.replace(entry,entry+'\n    '+addition)
            pattern=r'(<'+tag+r' Include="\.\.\\src\\renderer\\'+re.escape(old)+r'">\n\s*<Filter>renderer</Filter>\n\s*</'+tag+'>)'
            source=re.sub(pattern,lambda match:match[0]+'\n    '+match[0].replace(old,new),source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove every statement of the relocated whole historical producer and its original helper arithmetic
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/renderer/Timecycle.cpp').read_text())
    folder=root/'build/timecycle-c-tests/before';assert normalize(kernel(captured[0],(folder/'src/renderer/Timecycle.h').read_text(),(folder/'src/math/Vector.cpp').read_text()))==normalize((root/'src/renderer/TimeCycleSystem.c').read_text())

def verify_dependency(saved,current):
    # Permit only the exact full time-cycle caller projection through existing dependency byte guards
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Retain the complete time-cycle conversion before the mutually dependent C event and accident owners
    import events_migration as events
    return events.calls(cycle_calls(source))

def project(source):
    # Preserve prior registrations and object disambiguation before registering the C gameplay-event owner
    import events_migration as events
    return events.project(cycle_project(source))
