"""Relocate all drawing settings and calculations to C without changing their math."""
import re
import wind_migration as wind
HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
@ENUM@
#ifdef __cplusplus
extern "C" {
#endif
extern float Draw_ms_fLODDistance;
extern uint8_t Draw_FadeValue, Draw_FadeRed, Draw_FadeGreen, Draw_FadeBlue;
#ifdef PROPER_SCALING
extern bool Draw_ms_bProperScaling;
#endif
#ifdef FIX_RADAR
extern bool Draw_ms_bFixRadar;
#endif
#ifdef FIX_SPRITES
extern bool Draw_ms_bFixSprites;
#endif
void Draw_SetNearClipZ(float nearclip);
float Draw_GetNearClipZ(void);
void Draw_SetFarClipZ(float farclip);
float Draw_GetFarClipZ(void);
void Draw_SetFOV(float fov);
float Draw_GetFOV(void);
float Draw_GetScaledFOV(void);
float Draw_CalculateAspectRatio(void);
#ifdef ASPECT_RATIO_SCALE
float Draw_ConvertFOV(float fov);
#endif
float Draw_GetAspectRatio(void);
void Draw_SetAspectRatio(float ratio);
#ifdef PROPER_SCALING
float Draw_ScaleY(float y);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
BACKEND_HEADER='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
int8_t Draw_BackendAspectPreference(void);
bool Draw_BackendWidescreen(void);
float Draw_BackendScreenWidth(void);
float Draw_BackendScreenHeight(void);
bool Draw_BackendCutsceneRunning(void);
float Draw_BackendFovMultiplier(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Frontend.h"
#include "Camera.h"
#include "CutsceneMgr.h"
#include "Draw_backend.h"

int8_t Draw_BackendAspectPreference(void)
{
	// Read the original signed menu preference
	return FrontEndMenuManager.m_PrefsUseWideScreen;
}
bool Draw_BackendWidescreen(void)
{
	// Read the live camera widescreen flag at the original branch point
	return TheCamera.m_WideScreenOn;
}
float Draw_BackendScreenWidth(void)
{
	// Retain the original platform-dependent screen width and float conversion
	return SCREEN_WIDTH;
}
float Draw_BackendScreenHeight(void)
{
	// Retain the original platform-dependent screen height and float conversion
	return SCREEN_HEIGHT;
}
bool Draw_BackendCutsceneRunning(void)
{
	// Query the original cutscene owner's running state
	return CCutsceneMgr::IsRunning();
}
float Draw_BackendFovMultiplier(void)
{
	// Read the original live float FOV multiplier
	return rouz.fov_mul;
}
//- rouz edit (ChatGPT)
'''

def constants(source):
    # Move the exact shared dimensions and angle macros into a C-compatible header
    a=source.index('#if defined(PROPER_SCALING)');b=source.index('// game uses maximumWidth/Height',a)
    dimensions=source[a:b].rstrip()
    heights=re.search(r'#define SCREEN_HEIGHT_PAL[^\n]*\n#define SCREEN_HEIGHT_NTSC[^\n]*',source)[0]
    angles=re.search(r'#define PI \(float\)M_PI\n[\s\S]*?#define RADTODEG[^\n]*',source)[0]
    return '//+ rouz edit (ChatGPT)\n#pragma once\n#include "config.h"\n'+dimensions+'\n\n'+heights+'\n\n'+angles+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Keep every existing direct read, write and address while removing the class scope
    if 'class CDraw\n' in source:
        enum=source[source.index('enum eAspectRatio'):source.index('class CDraw')].rstrip()
        return HEADER.replace('@ENUM@',enum)
    if 'float CDraw::ms_fAspectRatio' in source:return HOST
    if '#define DEFAULT_ASPECT_RATIO' in source:
        a=source.index('#if defined(PROPER_SCALING)');b=source.index('// game uses maximumWidth/Height',a)
        source=source[:a]+'//+ rouz edit (ChatGPT)\n#include "Draw_constants.h"\n//- rouz edit (ChatGPT)\n\n'+source[b:]
        source=re.sub(r'#define SCREEN_HEIGHT_PAL[^\n]*\n#define SCREEN_HEIGHT_NTSC[^\n]*\n','',source)
        source=re.sub(r'#define PI \(float\)M_PI\n[\s\S]*?#define RADTODEG[^\n]*\n','',source)
    return wind.calls(re.sub(r'\bCDraw::(\w+)\b',r'Draw_\1',source))

def kernel(source,header):
    # Preserve the complete out-of-line bodies, private storage and seven former inline bodies
    source=source[source.index('float CDraw::ms_fAspectRatio'):]
    source=re.sub(r'\bCDraw::(\w+)\b',r'Draw_\1',source)
    members=['ms_fNearClipZ','ms_fFarClipZ','ms_fFOV','ms_fAspectRatio','ms_fScaledFOV','ms_bProperScaling']
    for member in members:source=re.sub(r'(?<!Draw_)\b'+member+r'\b','Draw_'+member,source)
    for member in members[:5]:source=source.replace('float Draw_'+member,'static float Draw_'+member)
    source=source.replace('uint8 Draw_','uint8_t Draw_')
    source=source.replace('FindAspectRatio','Draw_FindAspectRatio').replace('float\nDraw_FindAspectRatio','static float\nDraw_FindAspectRatio')
    source=source.replace('FrontEndMenuManager.m_PrefsUseWideScreen','Draw_BackendAspectPreference()').replace('TheCamera.m_WideScreenOn','Draw_BackendWidescreen()')
    source=source.replace('SCREEN_WIDTH / SCREEN_HEIGHT','Draw_BackendScreenWidth() / Draw_BackendScreenHeight()').replace('CCutsceneMgr::IsRunning()','Draw_BackendCutsceneRunning()').replace('rouz.fov_mul','Draw_BackendFovMultiplier()')
    source=re.sub(r'(?<!Draw_)\b(GetAspectRatio|ConvertFOV)\(',r'Draw_\1(',source)
    source=source.replace('Atan(tan(hfov/2) / ar1)','atanf((float)(tan(hfov/2) / ar1))').replace('Atan(tan(vfov/2) * ar2)','atanf((float)(tan(vfov/2) * ar2))')
    comments={'FindAspectRatio':'Select the original signed aspect preference and platform screen ratio','CalculateAspectRatio':'Retain the original preference branches and theatrical aspect calculation','ConvertFOV':'Retain double tangent, float arctangent and the original conversion order','GetScaledFOV':'Apply the live multiplier only in the original aspect-scaling profile','SetFOV':'Update scaled FOV before the original stored FOV','ScaleY':'Retain the original live proper-scaling flag and screen-height factor'}
    for name,comment in comments.items():
        source=re.sub(r'(\bDraw_'+name+r'\([^\n]*\)\n\{)',r'\1\n\t// '+comment,source)
    for typ,name,arg,body in re.findall(r'static (void|float) (\w+)\(([^\n]*)\) \{ ([^\n]*) \}',header):
        assert name in ['SetNearClipZ','GetNearClipZ','SetFarClipZ','GetFarClipZ','GetFOV','GetAspectRatio','SetAspectRatio']
        for member in members:body=re.sub(r'\b'+member+r'\b','Draw_'+member,body)
        source+='\n'+typ+'\nDraw_'+name+'('+arg+')\n{\n\t// Preserve the original scalar getter or setter\n\t'+body+'\n}\n'
    return '''//+ rouz edit (ChatGPT)
#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw.h"
#include "Draw_constants.h"
#include "Draw_backend.h"

'''+source.rstrip()+'\n//- rouz edit (ChatGPT)\n'

def project(source):
    # Register the C calculations beside the six typed owner readers
    entry='<ClCompile Include="..\\src\\renderer\\Draw.cpp" />'
    source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\Draw.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClCompile Include="..\\src\\renderer\\Draw.cpp">\n      <Filter>renderer</Filter>\n    </ClCompile>'
    source=source.replace(entry,entry+'\n    '+entry.replace('Draw.cpp','Draw.c'))
    for name in ['Draw_backend.h','Draw_constants.h']:
        entry='<ClInclude Include="..\\src\\renderer\\Draw.h" />'
        source=source.replace(entry,entry+'\n    '+entry.replace('Draw.h',name))
        entry='<ClInclude Include="..\\src\\renderer\\Draw.h">\n      <Filter>renderer</Filter>\n    </ClInclude>'
        source=source.replace(entry,entry+'\n    '+entry.replace('Draw.h',name))
    return wind.project(source)
