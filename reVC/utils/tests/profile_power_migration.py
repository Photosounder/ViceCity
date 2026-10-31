"""Remove the complete profiler and cut power-point classes with two standalone C translation units."""
import re
import object_names_migration as objects
OBSERVER=None
RENAMES={'src/core/Profile.cpp':'src/core/Profile.c','src/control/PowerPoints.cpp':'src/control/PowerPoints.c'}
PROFILE_FIELDS=['ms_afStartTime','ms_afCumulativeTime','ms_afEndTime','ms_afMaxEndTime','ms_afMaxCumulativeTime','ms_pProfileString','ms_aBarColours']
PROFILE_OPERATIONS=['Initialise','SuspendProfile','ShowResults']

def profile_header(original):
    # Preserve the original profile identifiers and expose the complete C-owned state without RenderWare types
    enum=original[original.index('enum eProfile'):original.index('class CProfile')].strip();enum=enum.replace('enum eProfile','typedef enum eProfile',1).replace('};','} eProfile;')
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "config.h"
'''+enum+'''
typedef struct ProfileColor {uint8_t red,green,blue,alpha;} ProfileColor;
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join('extern float Profile_'+name+'[NUM_PROFILES];\n' for name in PROFILE_FIELDS[:5])+'''extern const char *Profile_ms_pProfileString[NUM_PROFILES];
extern ProfileColor Profile_ms_aBarColours[NUM_PROFILES];
void Profile_Initialise(void);
void Profile_SuspendProfile(eProfile profile);
void Profile_ShowResults(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def profile_kernel(original):
    # Preserve every complete original operation, partial initialization and ordered Max expression
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M).strip().replace('CProfile::','Profile_')
    for name in PROFILE_FIELDS:source=re.sub(r'(?<![\w])'+name+r'\b','Profile_'+name,source)
    source=re.sub(r'\bConst\b','const',source);source=source.replace('RwRGBA','ProfileColor')
    source=re.sub(r'(Profile_ms_aBarColours\[[^\]]+\]\s*=\s*)\{',r'\1(ProfileColor){',source)
    for name in PROFILE_OPERATIONS:
        source=re.sub(r'(Profile_'+name+r'\([^\n]*\)\s*\n\{)',r'\1\n    // Preserve the complete original '+name+' operation and every intentionally omitted reset',source,count=1)
        source=source.replace('Profile_'+name+'()','Profile_'+name+'(void)')
    return '//+ rouz edit (ChatGPT)\n#include "Profile.h"\n#define Max(a,b) ((a)>(b)?(a):(b))\n\n'+source+'\n//- rouz edit (ChatGPT)\n'

def power_header(original):
    # Keep every cut-feature identifier and the original empty instance's one-byte storage extent
    enum=original[original.index('enum'):original.index('class CPowerPoint')].strip()
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
'''+enum+'''
typedef struct CPowerPoint {uint8_t reserved;} CPowerPoint;
#ifdef __cplusplus
extern "C" {
#endif
void PowerPoint_Update(CPowerPoint *self);
void PowerPoints_Init(void);
void PowerPoints_Update(void);
void PowerPoints_GenerateNewOne(float a,float b,float c,float d,float e,float f,uint8_t type);
void PowerPoints_Save(uint8_t **buffer,uint32_t *size);
void PowerPoints_Load(uint8_t *buffer,uint32_t size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

POWER_OPERATIONS=[('PowerPoint_Update','CPowerPoint *self',['self']),('PowerPoints_Init','void',[]),('PowerPoints_Update','void',[]),('PowerPoints_GenerateNewOne','float a,float b,float c,float d,float e,float f,uint8_t type',['a','b','c','d','e','f','type']),('PowerPoints_Save','uint8_t **buffer,uint32_t *size',['buffer','size']),('PowerPoints_Load','uint8_t *buffer,uint32_t size',['buffer','size'])]

def power_kernel(original):
    # Retain all six exact no-op contracts without inventing output initialization or beta-feature behavior
    bodies=re.findall(r'void CPowerPoints?::\w+\([^)]*\)\s*\{([^}]*)\}',original);assert len(bodies)==6 and all(not body.strip() for body in bodies)
    return '//+ rouz edit (ChatGPT)\n#include "PowerPoints.h"\n\n'+ '\n'.join('void '+name+'('+args+')\n{\n    // Preserve the original cut-feature operation without reading or changing caller storage\n    '+''.join('(void)'+value+';' for value in values)+'\n}\n' for name,args,values in POWER_OPERATIONS)+'//- rouz edit (ChatGPT)\n'

def profile_power_calls(source):
    # Remove all three complete classes and project the existing native power-point caller
    if 'class CProfile\n' in source:return profile_header(source)
    if 'float CProfile::ms_afStartTime[NUM_PROFILES];' in source:
        if OBSERVER:OBSERVER(('profile',source))
        return profile_kernel(source)
    if 'class CPowerPoints\n' in source:return power_header(source)
    if 'void CPowerPoint::Update()' in source:
        if OBSERVER:OBSERVER(('power',source))
        return power_kernel(source)
    original=source;source=source.replace('CProfile::','Profile_').replace('CPowerPoints::','PowerPoints_')
    if source==original:return source
    if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)'):return source
    return '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def profile_power_project(source):
    # Rename both complete producers and set their per-file C11 metadata in historical native projects
    for old,new in RENAMES.items():
        before='..\\'+old.replace('/','\\');after='..\\'+new.replace('/','\\');source=source.replace(before,after)
        if 'Microsoft.Cpp.targets' not in source:continue
        pattern=r'<ClCompile Include="'+re.escape(after)+r'"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated project options while requiring the renamed producer to compile as C11
            body=match[1] or '\n    '
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body=body.rstrip()+'\n      <'+tag+'>'+value+'</'+tag+'>\n    '
            return '<ClCompile Include="'+after+'">'+body+'</ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)

def verify_legacy(source,transform,root):
    # Prove relocation of the complete original producer into its renamed C translation unit
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    kind,original=captured[0];path='src/core/Profile.c' if kind=='profile' else 'src/control/PowerPoints.c'
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/path).read_text())
    assert normalize(profile_kernel(original) if kind=='profile' else power_kernel(original))==normalize((root/path).read_text())

def verify_dependency(saved,current):
    # Follow only the two explicit producer renames when checking historical dependency snapshots
    from pathlib import Path
    root=Path(__file__).resolve().parents[2];name=current.relative_to(root).as_posix();current=root/RENAMES.get(name,name)
    if saved.read_bytes()==current.read_bytes():return
    original=saved.read_text();expected=calls(original);assert expected!=original and expected==current.read_text(),str(current)


def calls(source):
    # Preserve prior conversions before replacing complete intrusive-list ownership
    import lists_migration as lists
    return lists.calls(profile_power_calls(source))

def project(source):
    # Retain previous registrations before the complete Lists.c producer rename
    import lists_migration as lists
    return lists.project(profile_power_project(source))
