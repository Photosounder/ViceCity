"""Move the complete lighting owner and its vector policy into genuine C."""
import re
from rect_migration import mask

RENAMES={'src/rw/Lights.cpp':'src/rw/Lights.c'}
OPERATIONS=['SetLightsWithTimeOfDayColour','LightsCreate','LightsDestroy','WorldReplaceNormalLightsWithScorched','WorldReplaceScorchedLightsWithNormal','AddAnExtraDirectionalLight','RemoveExtraDirectionalLights','SetAmbientAndDirectionalColours','SetFlashyColours','SetFlashyColours_Mild','SetBrightMarkerColours','ReSetAmbientAndDirectionalColours','DeActivateDirectional','ActivateDirectional','SetFullAmbient','SetAmbientColours','SetAmbientColoursForPedsCarsAndObjects','SetAmbientColoursToIndicateRoadGroup']
NAMES={name:'Lights_'+({'LightsCreate':'Create','LightsDestroy':'Destroy'}.get(name,name)) for name in OPERATIONS}
HANDLES={'pAmbient':'Lights_Ambient','pDirect':'Lights_Direct','pExtraDirectionals':'Lights_ExtraDirectionals'}
HOST=[
    ('float','Multiplier','void','CCoronas::LightsMult'),
    ('int','Brightness','void','FrontEndMenuManager.m_PrefsBrightness'),
    ('void *','Create','int kind','RpLightCreate(kind==LIGHTS_AMBIENT?rpLIGHTAMBIENT:rpLIGHTDIRECTIONAL)'),
    ('void','Destroy','void *light','RpLightDestroy((RpLight*)light)'),
    ('void','Flags','void *light,int active','RpLightSetFlags((RpLight*)light,active?rpLIGHTLIGHTATOMICS:0)'),
    ('void','Radius','void *light,float radius','RpLightSetRadius((RpLight*)light,radius)'),
    ('void *','CreateFrame','void','RwFrameCreate()'),
    ('void','DestroyFrame','void *frame','RwFrameDestroy((RwFrame*)frame)'),
    ('void *','Frame','void *light','RpLightGetFrame((RpLight*)light)'),
    ('void','SetFrame','void *light,void *frame','RpLightSetFrame((RpLight*)light,(RwFrame*)frame)'),
    ('void *','Matrix','void *frame','RwFrameGetMatrix((RwFrame*)frame)'),
    ('void','UpdateMatrix','void *matrix','RwMatrixUpdate((RwMatrix*)matrix)'),
    ('void','UpdateObjects','void *frame','RwFrameUpdateObjects((RwFrame*)frame)'),
    ('void','Add','void *world,void *light','RpWorldAddLight((RpWorld*)world,(RpLight*)light)'),
    ('void','Remove','void *world,void *light','RpWorldRemoveLight((RpWorld*)world,(RpLight*)light)'),
]

def header():
    # Expose borrowed opaque SDK handles and unique C operation names
    value='''//+ rouz edit (ChatGPT)
#pragma once
typedef struct LightsColour {float red,green,blue,alpha;} LightsColour;
#ifdef __cplusplus
extern "C" {
#endif
extern void *Lights_Ambient,*Lights_Direct,*Lights_ExtraDirectionals[4];
extern int LightStrengths[4],NumExtraDirLightsInWorld;
void Lights_SetLightsWithTimeOfDayColour(void *world);
void *Lights_Create(void *world);
void Lights_Destroy(void *world);
void Lights_WorldReplaceNormalLightsWithScorched(void *world,float l);
void Lights_WorldReplaceScorchedLightsWithNormal(void *world);
void Lights_AddAnExtraDirectionalLight(void *world,float dirx,float diry,float dirz,float red,float green,float blue);
void Lights_RemoveExtraDirectionalLights(void *world);
void Lights_SetAmbientAndDirectionalColours(float f);
void Lights_SetFlashyColours(float f);
void Lights_SetFlashyColours_Mild(float f);
void Lights_SetBrightMarkerColours(float f);
void Lights_ReSetAmbientAndDirectionalColours(void);
void Lights_DeActivateDirectional(void);
void Lights_ActivateDirectional(void);
void Lights_SetAmbientColours(void);
void Lights_SetAmbientColoursForPedsCarsAndObjects(void);
void Lights_SetAmbientColoursToIndicateRoadGroup(int i);
void Lights_SetAmbientColour(const LightsColour *colour);
void Lights_SetFullAmbient(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return value

def backend_header():
    # Keep all matrix bytes in C storage while borrowing native vector scalar subobjects without aliasing records
    value='''//+ rouz edit (ChatGPT)
#pragma once
#include "Lights.h"
#include <stdint.h>
#include <stdbool.h>
typedef struct LightsVector {float x,y,z;} LightsVector;
typedef struct LightsMatrix {LightsVector right;uint32_t flags;LightsVector up;uint32_t pad1;LightsVector at;uint32_t pad2;LightsVector pos;uint32_t pad3;} LightsMatrix;
typedef struct LightsVectorOut {float *x,*y,*z;} LightsVectorOut;
enum {LIGHTS_AMBIENT,LIGHTS_DIRECTIONAL};
#ifdef __cplusplus
extern "C" {
#endif
bool Lights_HostBlurColours(void);
void Lights_HostColour(void *light,const LightsColour *colour);
void Lights_HostRotate(void *frame,const LightsVector *axis,float angle);
void Lights_HostTransform(void *frame,const LightsMatrix *matrix);
LightsVectorOut Lights_HostAt(void *matrix);
'''
    value+=''.join(result+' Lights_Host'+name+'('+args+');\n' for result,name,args,_ in HOST)
    return value+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def kernel(original):
    # Preserve complete original lighting operations and implement the original vector arithmetic directly in C
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=re.sub(r'#ifdef EXTENDED_COLOURFILTER\n[\s\S]*?#endif\n','',source,count=1)
    source=source.replace('USEBLURCOLORS','Lights_HostBlurColours()').replace('CCoronas::LightsMult','Lights_HostMultiplier()').replace('FrontEndMenuManager.m_PrefsBrightness','Lights_HostBrightness()')
    source=source.replace('SetAmbientColours(RwRGBAReal *color)','Lights_SetAmbientColour(const LightsColour *color)')
    for old,new in NAMES.items():source=re.sub(r'\b'+old+r'\b',new,source)
    for old,new in HANDLES.items():source=re.sub(r'\b'+old+r'\b',new,source)
    source=source.replace('TimeCycle_ToNativeVector(TimeCycle_m_VectorToSun[TimeCycle_m_CurrentStoredValue])','TimeCycle_m_VectorToSun[TimeCycle_m_CurrentStoredValue]')
    source=source.replace('CVector vec1, vec2, vecsun;','TimeCycleVector vec1, vec2, vecsun;')
    source=source.replace('CVector(0.0f, 0.0f, 1.0f)','(TimeCycleVector){0.0f, 0.0f, 1.0f}').replace('CrossProduct(','Lights_CrossProduct(').replace('vec2.Normalise();','Lights_Normalise(&vec2);')
    source=source.replace('RwV3d *dir;','LightsVectorOut dir;')
    for axis in ['x','y','z']:source=source.replace('dir->'+axis,'*dir.'+axis)
    for old,new in [('RpWorld','void'),('RpLight','void'),('RwFrame','void'),('RwRGBAReal','LightsColour'),('RwMatrix','LightsMatrix'),('RwV3d','LightsVector'),('uint8','uint8_t'),('nil','NULL'),('rpLIGHTAMBIENT','LIGHTS_AMBIENT'),('rpLIGHTDIRECTIONAL','LIGHTS_DIRECTIONAL'),('rpLIGHTLIGHTATOMICS','1')]:source=re.sub(r'\b'+old+r'\b',new,source)
    primitives={'RpLightCreate':'Create','RpLightDestroy':'Destroy','RpLightSetFlags':'Flags','RpLightSetColor':'Colour','RpLightSetRadius':'Radius','RwFrameCreate':'CreateFrame','RwFrameDestroy':'DestroyFrame','RpLightGetFrame':'Frame','RpLightSetFrame':'SetFrame','RwFrameGetMatrix':'Matrix','RwMatrixGetAt':'At','RwMatrixUpdate':'UpdateMatrix','RwFrameUpdateObjects':'UpdateObjects','RpWorldAddLight':'Add','RpWorldRemoveLight':'Remove','RwFrameRotate':'Rotate','RwFrameTransform':'Transform'}
    for old,new in primitives.items():source=re.sub(r'\b'+old+r'\b','Lights_Host'+new,source)
    source=source.replace(', rwCOMBINEREPLACE)',')').replace(', rwCOMBINEPRECONCAT)',')')
    source=source.replace('Lights_SetLightsWithTimeOfDayColour(void *)','Lights_SetLightsWithTimeOfDayColour(void *world)')
    for name in list(NAMES.values())+['Lights_SetAmbientColour']:
        # Describe the complete original operation before its unchanged control flow
        source=re.sub(r'(?m)^('+name+r'\([^\n]*\)\n\{\n)',lambda match:match[1]+'    // Preserve the complete original lighting operation and its mutation order\n',source)
    prefix='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "Lights.h"
#include "Lights_backend.h"
#include "Timer.h"
#include "Timecycle.h"
#include "Weather.h"
#include "ZoneCull.h"
#include <stddef.h>
#include <math.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
static TimeCycleVector Lights_CrossProduct(TimeCycleVector a,TimeCycleVector b)
{
    // Preserve the original vector cross product evaluation order
    const TimeCycleVector value={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};return value;
}
static void Lights_Normalise(TimeCycleVector *value)
{
    // Preserve the original zero-vector fallback and reciprocal square root arithmetic
    float sq=value->x*value->x+value->y*value->y+value->z*value->z;
    if(sq>0.0f){
        // Scale all three components with the original reciprocal square root
        float invsqrt=1.0f/sqrtf(sq);value->x*=invsqrt;value->y*=invsqrt;value->z*=invsqrt;
    }else value->x=1.0f;
}
'''
    return prefix+source+'\n//- rouz edit (ChatGPT)\n'

def native():
    # Restrict native code to SDK primitives and external owner metadata
    value='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "Lights_backend.h"
#include "Coronas.h"
#include "Frontend.h"
#include "MBlur.h"
#ifdef EXTENDED_COLOURFILTER
#include "postfx.h"
#endif
#include <stddef.h>
static_assert(sizeof(LightsColour)==sizeof(RwRGBAReal),"Preserve native colour storage");
static_assert(alignof(LightsColour)==alignof(RwRGBAReal),"Preserve native colour alignment");
static_assert(sizeof(LightsMatrix)==sizeof(RwMatrix),"Preserve native matrix storage");
static_assert(alignof(LightsMatrix)==alignof(RwMatrix),"Preserve native matrix alignment");
'''
    for own,sdk,fields in [('LightsColour','RwRGBAReal',['red','green','blue','alpha']),('LightsMatrix','RwMatrix',['right','flags','up','pad1','at','pad2','pos','pad3'])]:
        # Require every copied SDK field to have the same byte representation on each actual target
        for field in fields:value+='static_assert(offsetof('+own+','+field+')==offsetof('+sdk+','+field+'),"Preserve '+field+' offset");\n'
    value+='''bool Lights_HostBlurColours(void)
{
    // Read the original active colour-filter preference at each original call site
#ifdef EXTENDED_COLOURFILTER
    return CPostFX::UseBlurColours();
#else
    return CMBlur::BlurOn;
#endif
}
void Lights_HostColour(void *light,const LightsColour *colour)
{
    // Copy native colour bytes without reading the original uninitialized alpha as a scalar
    RwRGBAReal native;memcpy(&native,colour,sizeof(native));RpLightSetColor((RpLight*)light,&native);
}
void Lights_HostRotate(void *frame,const LightsVector *axis,float angle)
{
    // Retain the original SDK rotation and preconcatenation mode
    const RwV3d native={axis->x,axis->y,axis->z};RwFrameRotate((RwFrame*)frame,&native,angle,rwCOMBINEPRECONCAT);
}
void Lights_HostTransform(void *frame,const LightsMatrix *matrix)
{
    // Preserve all original matrix bytes and the SDK replacement operation without aliasing record types
    RwMatrix native;memcpy(&native,matrix,sizeof(native));RwFrameTransform((RwFrame*)frame,&native,rwCOMBINEREPLACE);
}
LightsVectorOut Lights_HostAt(void *matrix)
{
    // Borrow the actual native direction components while keeping scalar writes in the C owner
    RwV3d *at=RwMatrixGetAt((RwMatrix*)matrix);const LightsVectorOut value={&at->x,&at->y,&at->z};return value;
}
'''
    for result,name,args,expression in HOST:
        # Forward each individual primitive without moving lighting policy into C++
        value+=result+' Lights_Host'+name+'('+args+')\n{\n    // Preserve the original SDK primitive or external owner metadata access\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
    return value+'//- rouz edit (ChatGPT)\n'

def lights_calls(source):
    # Convert the entire owner and every client to one self-contained C API
    if 'SetLightsWithTimeOfDayColour(RpWorld *)\n{' in source:return kernel(source)
    if 'extern RpLight *pAmbient;' in source and 'void SetAmbientColours(RwRGBAReal *color);' in source:return header()
    original=source
    source=source.replace('RwRGBAReal color={value.red,value.green,value.blue,value.alpha};SetAmbientColours(&color);','LightsColour color={value.red,value.green,value.blue,value.alpha};Lights_SetAmbientColour(&color);')
    code=mask(source)
    changes=[]
    for match in re.finditer(r'\b(?:'+'|'.join([*NAMES,*HANDLES])+r')\b',code):
        # Keep SDK handle dereferences typed only in consumers that still own SDK operations
        name=match[0]
        if name in NAMES:value=NAMES[name]
        elif name=='pExtraDirectionals':
            start=match.end()
            while start<len(code) and code[start].isspace():start+=1
            assert code[start]=='[',source[match.start():match.end()+60]
            end=start+1;depth=1
            while depth:
                # Match the complete array expression without changing its evaluation
                if code[end]=='[':depth+=1
                elif code[end]==']':depth-=1
                end+=1
            changes.append((match.start(),end,'((RpLight*)'+HANDLES[name]+source[match.end():end]+')'));continue
        else:value='((RpLight*)'+HANDLES[name]+')'
        changes.append((match.start(),match.end(),value))
    for start,end,value in reversed(changes):source=source[:start]+value+source[end:]
    if source!=original:
        # Describe each changed client block before its use of C policy or borrowed SDK handles
        lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True)
        for index in reversed(range(len(lines))):
            if re.search(r'\bLights_\w+',code[index]):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Use the complete C lighting owner and its borrowed SDK handles\n')
        return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'
    return source

def lights_project(source):
    # Compile the entire lighting owner as C11 with separate native SDK primitives
    import object_names_migration as objects
    source=source.replace('src\\rw\\Lights.cpp','src\\rw\\Lights.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\Lights\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve all existing per-file metadata and select the C compiler for lighting policy
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>'
            native=match[1][:-len('Lights.c')]+'Lights_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Retain complete lighting ownership before applying both complete C pool records and operations
    import texture_pools_migration as pools
    return pools.calls(lights_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting the complete C texture-pool owner
    import texture_pools_migration as pools
    return pools.project(lights_project(source))
