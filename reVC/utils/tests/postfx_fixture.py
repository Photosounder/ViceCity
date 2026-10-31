"""Prepare entire original/C post-processing owners with independent callers and controlled SDK operations."""
from pathlib import Path
import re
import postfx_migration as m
from memory_heap_migration import body
from mblur_schema import SETTERS,RW_STATES,BLENDS

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/postfx-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "MBlur.h"
#include "MBlur_backend.h"
#include "Lights_backend.h"
#include "Camera_effects.h"
extern "C" void re3_assert(const char*,const char*,unsigned int,const char*);
typedef int32_t int32,RwInt32,RwBool;
typedef uint32_t uint32,RwUInt32;
typedef uint8_t uint8;
typedef uint16_t RwImVertexIndex;
#define nil NULL
#define TRUE 1
#define FALSE 0
#define Min(a,b) ((a)<(b)?(a):(b))
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#ifdef RW_GL3
#define RW_OPENGL
#endif
#define HALFPX 0.0f
struct RwRGBA {uint8 red,green,blue,alpha;};
struct RwIm2DVertex {float x,y,z,cameraZ,reciprocal,u,v;RwRGBA colour;};
struct RwRaster {unsigned id;int width,height,depth;bool alive;};
struct RwCamera {RwRaster *frameBuffer;float nearClip;};
struct TestShader {
    unsigned id,kind;bool alive;int32 uniformLocations[32];
    static TestShader *create(const char**,const char**);
    void destroy(void);void use(void);
};
struct TestDevice {int SetPixelShaderConstantF(uint32,const float*,uint32);};
namespace rw {
namespace d3d {extern TestDevice *d3ddevice;extern void *im2dOverridePS;void *createPixelShader(void*);void destroyPixelShader(void*);}
namespace gl3 {typedef TestShader Shader;extern Shader *im2dOverrideShader;extern const char *shaderDecl,*header_vert_src,*header_frag_src;int32 registerUniform(const char*);}
}
void glUniform3fv(int32,int32,const float*);
void glUniform4fv(int32,int32,const float*);
RwRaster *RwCameraGetRaster(RwCamera*);
float RwCameraGetNearClipPlane(RwCamera*);
int RwRasterGetWidth(RwRaster*);
int RwRasterGetHeight(RwRaster*);
int RwRasterGetDepth(RwRaster*);
RwRaster *RwRasterCreate(int,int,int,uint32);
void RwRasterDestroy(RwRaster*);
void RwRasterPushContext(RwRaster*);
void RwRasterPopContext(void);
void RwRasterRenderFast(RwRaster*,int,int);
float RwIm2DGetNearScreenZ(void);
void RwIm2DRenderIndexedPrimitive(int32,RwIm2DVertex*,int32,RwImVertexIndex*,int32);
void DefinedState(void);
void PushRendergroup(const char*);
void PopRendergroup(void);
#ifndef FINAL
#define PUSH_RENDERGROUP(name) PushRendergroup(name)
#define POP_RENDERGROUP() PopRendergroup()
#else
#define PUSH_RENDERGROUP(name)
#define POP_RENDERGROUP()
#endif
inline float Pow(float x,float y){return powf(x,y);}
'''
owners+='enum RwRenderState {'+','.join('rwRENDERSTATE'+name+'='+str(10+i) for i,name in enumerate(RW_STATES))+'};\n'
owners+='enum {'+','.join('rwBLEND'+name+'='+str(1+i) for i,name in enumerate(BLENDS))+',rwFILTERNEAREST=7,rwPRIMTYPETRILIST=4,rwRASTERTYPECAMERATEXTURE=5};\nvoid RwRenderStateSet(RwRenderState,void*);\n'
for name,(args,_) in SETTERS.items():owners+='void RwIm2DVertexSet'+name+'(RwIm2DVertex *vertex,'+args+');\n'
owners+='''#ifdef ORIGINAL
#include "original-api.h"
#else
#include "postfx.h"
#endif
'''
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
enum {OP_Init,OP_Open,OP_Close,OP_Blur,OP_Sniper,OP_Shader,OP_Motion,OP_Render,OP_Smooth,OP_NeedBack,OP_NeedFront,OP_GetBack,OP_UseBlur,OP_BindOpen,OP_BindClose,OP_BindRender,OP_BindLighting};
typedef struct TestPostArgs {void *camera;uint32_t red,green,blue,alpha,blur;int32_t type;} TestPostArgs;
typedef struct TestPostState {void *front,*back;bool initialised,blur,motion;int effect;float intensity;int previous[4][20],average[4],next,count;} TestPostState;
#ifdef __cplusplus
extern "C" {
#endif
int32_t TestPost(unsigned operation,TestPostArgs *args);
void TestPostGetState(TestPostState *state);
void TestPostSetState(const TestPostState *state);
#ifdef __cplusplus
}
#endif
'''
cases='''case OP_Init:CPostFX::InitOnce();break;
case OP_Open:CPostFX::Open((RwCamera*)args->camera);break;
case OP_Close:CPostFX::Close();break;
case OP_Blur:CPostFX::RenderOverlayBlur((RwCamera*)args->camera,args->red,args->green,args->blue,args->alpha);break;
case OP_Sniper:CPostFX::RenderOverlaySniper((RwCamera*)args->camera,args->red,args->green,args->blue,args->alpha);break;
case OP_Shader:CPostFX::RenderOverlayShader((RwCamera*)args->camera,args->red,args->green,args->blue,args->alpha);break;
case OP_Motion:CPostFX::RenderMotionBlur((RwCamera*)args->camera,args->blur);break;
case OP_Render:CPostFX::Render((RwCamera*)args->camera,args->red,args->green,args->blue,args->blur,args->type,args->alpha);break;
case OP_Smooth:CPostFX::SmoothColor(args->red,args->green,args->blue,args->alpha);break;
case OP_NeedBack:return CPostFX::NeedBackBuffer();
case OP_NeedFront:return CPostFX::NeedFrontBuffer(args->type);
case OP_GetBack:CPostFX::GetBackBuffer((RwCamera*)args->camera);break;
case OP_UseBlur:return CPostFX::UseBlurColours();
case OP_BindOpen:MBlur_HostPostOpen(args->camera);break;
case OP_BindClose:MBlur_HostPostClose();break;
case OP_BindRender:MBlur_HostPostRender(args->camera,args->red,args->green,args->blue,args->blur,args->type,args->alpha);break;
'''
for phase in ['before','after']:
    # Compare whole original C++ calls with an independently compiled C public consumer
    code=cases
    if phase=='after':code=code.replace('CPostFX::','PostFX_').replace('(RwCamera*)args->camera','args->camera')
    value=('#include "owners.h"\n' if phase=='before' else '#include "postfx.h"\n#include "MBlur_backend.h"\n#include "Lights_backend.h"\n#include <string.h>\n')+'#include "entry.h"\n'
    value+='int32_t TestPost(unsigned operation,TestPostArgs *args)\n{\n    // Dispatch complete owner and binding operations without reproducing rendering or buffer policy\n    if(operation==OP_BindLighting)return Lights_HostBlurColours();\n#ifdef EXTENDED_COLOURFILTER\n    switch(operation){\n'+code+'    }\n#endif\n    return 0;\n}\n'
    fields={'front':'pFrontBuffer','back':'pBackBuffer','initialised':'bJustInitialised','effect':'EffectSwitch','blur':'BlurOn','motion':'MotionBlurOn','intensity':'Intensity','next':'Next','count':'NumValues'};prefix='CPostFX::' if phase=='before' else 'PostFX_'
    value+='void TestPostGetState(TestPostState *state)\n{\n    // Observe actual shared storage through the public declarations rather than copied owner policy\n    memset(state,0,sizeof(*state));\n#ifdef EXTENDED_COLOURFILTER\n'
    for key,name in fields.items():value+='    state->'+key+'='+prefix+name+';\n'
    for i,channel in enumerate(['Red','Green','Blue','Alpha']):value+='    memcpy(state->previous['+str(i)+'],'+prefix+'Prev'+channel+',sizeof(state->previous['+str(i)+']));state->average['+str(i)+']='+prefix+'Avg'+channel+';\n'
    value+='#endif\n}\nvoid TestPostSetState(const TestPostState *state)\n{\n    // Supply caller-controlled shared inputs directly to canonical owner storage\n#ifdef EXTENDED_COLOURFILTER\n'
    for key,name in fields.items():value+='    '+prefix+name+'='+('(RwRaster*)' if phase=='before' and key in ['front','back'] else '')+'state->'+key+';\n'
    for i,channel in enumerate(['Red','Green','Blue','Alpha']):value+='    memcpy('+prefix+'Prev'+channel+',state->previous['+str(i)+'],sizeof(state->previous['+str(i)+']));'+prefix+'Avg'+channel+'=state->average['+str(i)+'];\n'
    value+='#endif\n}\n';(probe/(phase+'-caller.'+('cpp' if phase=='before' else 'c'))).write_text(value,newline='\n')
oldBindings=[]
for file,names in [('src/rw/Lights_native.cpp',['Lights_HostBlurColours']),('src/renderer/MBlur_native.cpp',['MBlur_HostPostOpen','MBlur_HostPostClose','MBlur_HostPostRender'])]:
    # Retain each complete original forwarding body and its actual public C ABI declaration
    source=(out/'before'/file).read_text()
    for name in names:
        match=re.search(r'(?m)^(?:void|bool) '+name+r'\([^\n]*\)\n\{',source);assert match,name;signature=match[0][:-2];value=signature+'\n{'+body(source,signature)+'}\n'
        oldBindings.append(value if name.startswith('Lights_') else '#ifdef EXTENDED_COLOURFILTER\n'+value+'#endif\n')
original=re.sub(r'^#include "(?!shaders/)[^\n]*\n','',(out/'before/src/extras/postfx.cpp').read_text(),flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
for name,value in [('owners.h',owners),('entry.h',entry),('original-api.h',(out/'before/src/extras/postfx.h').read_text()),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "PostFX_backend.h"\n'+native),('before-bindings.cpp','#include "owners.h"\n'+''.join(oldBindings))]:
    # Materialize complete producers and native boundaries against the controlled external SDK declarations
    (probe/name).write_text(value,newline='\n')
template=root/'utils/tests/postfx_fixture.cpp.in'
if template.exists():
    # Implement individual SDK setters while keeping every coefficient and vertex-building algorithm inside the actual producers
    fixture=template.read_text();fields={'ScreenX':'x','ScreenY':'y','ScreenZ':'z','CameraZ':'cameraZ','RecipCameraZ':'reciprocal','U':'u','V':'v'}
    for i,(name,(args,_)) in enumerate(SETTERS.items()):
        fixture+='\nvoid RwIm2DVertexSet'+name+'(RwIm2DVertex *vertex,'+args+')\n{\n    // Observe each original SDK setter argument and its initialized vertex subobjects\n    event('+str(100+i)+');'
        if name=='IntRGBA':fixture+='uint8_t values[]={red,green,blue,alpha};hashBytes(values,sizeof(values));vertex->colour={red,green,blue,alpha};'
        else:
            fixture+='hashBytes(&value,sizeof(value));vertex->'+fields[name]+'=value;'
            if name in ['U','V']:fixture+='hashBytes(&reciprocal,sizeof(reciprocal));'
        fixture+='\n}\n'
    (probe/'fixture.cpp').write_text(fixture,newline='\n')
print('Prepared all 13 complete original/C post-processing operations and four original/C forwarding operations')
