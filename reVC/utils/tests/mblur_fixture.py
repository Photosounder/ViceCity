"""Prepare complete original/C motion blur with controlled SDK primitives and independent callers."""
from pathlib import Path
import re
import mblur_migration as m
from mblur_schema import RW_STATES,BLENDS,STENCIL,SETTERS

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/mblur-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
common=(out/'before/src/core/common.h').read_text();rgba=common[common.index('class CRGBA\n'):common.index('\n#if (defined(_MSC_VER))',common.index('class CRGBA\n'))]
screen=common[common.index('// game uses maximumWidth/Height'):common.index('#include "maths.h"')]
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "Draw.h"
#include "Draw_constants.h"
#include "General.h"
#include "Timecycle.h"
#include "Timer.h"
typedef int32_t int32,RwInt32,RwBool;
typedef uint32_t uint32,RwUInt32;
typedef uint8_t uint8,RwUInt8;
typedef unsigned long DWORD;
typedef DWORD *LPDWORD;
typedef uint16_t RwImVertexIndex;
#define nil NULL
#define TRUE 1
#define FALSE 0
#define RWCORE_H
#define Max(a,b) ((a)>(b)?(a):(b))
#define Min(a,b) ((a)<(b)?(a):(b))
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#if !defined(LIBRW)||defined(RWHALFPIXEL)
#define HALFPX 0.5f
#else
#define HALFPX 0.0f
#endif
struct RwRGBA {uint8 red,green,blue,alpha;};
struct RwRect {int32 x,y,w,h;};
struct RwIm2DVertex {float x,y,z,cameraZ,reciprocal,u,v;RwRGBA colour;};
struct RwRaster {unsigned id;int32 width,height,depth;uint32 flags;bool alive;RwRaster *parent;};
struct RwCamera {unsigned id;RwRaster *frameBuffer;float nearClip;};
struct TestScreen {int32 width,height;};
extern TestScreen RsGlobal;
struct TestCamera {int fade;int GetScreenFadeStatus(void);};
extern TestCamera TheCamera;
struct TestFrontend {int32 m_PrefsBrightness;};
extern TestFrontend FrontEndMenuManager;
struct CHud {static int32 m_VehicleState,m_ZoneState;};
struct D3DCAPS8 {uint32 MaxTextureWidth,MaxTextureHeight;};
extern "C" D3DCAPS8 _RwD3D8DeviceCaps;
extern DWORD _dwMemTotalVideo;
void _GetVideoMemInfo(LPDWORD,LPDWORD);
extern RwRaster *gpDotRaster,*gpHeatHazeRaster,*gpRainDripRaster[2],*gpRainDripDarkRaster[2],*gpCarSplashRaster[1];
extern "C" void re3_debug(const char*,...);
#define debug(format,...) re3_debug("[DBG]: " format, ##__VA_ARGS__)
#ifndef FINAL
void PushRendergroup(const char*);
void PopRendergroup(void);
#define PUSH_RENDERGROUP(name) PushRendergroup(name)
#define POP_RENDERGROUP() PopRendergroup()
#else
#define PUSH_RENDERGROUP(name)
#define POP_RENDERGROUP()
#endif
inline float Pow(float x,float y){return powf(x,y);}
void *RwCameraGetRaster(RwCamera*)=delete;
'''
owners=owners.replace('void *RwCameraGetRaster(RwCamera*)=delete;','')+screen+'\n'+rgba+'\n'+'\n\n'.join(m.camera_blocks((out/'before/src/core/Camera.h').read_text()))+'\n'
owners+='enum RwRenderState {'+','.join('rwRENDERSTATE'+name+'='+str(10+index) for index,name in enumerate(RW_STATES))+'};\n'
owners+='enum {'+','.join('rwBLEND'+name+'='+str(1+index) for index,name in enumerate(BLENDS))+',rwFILTERNEAREST=7,rwPRIMTYPETRILIST=4,rwRASTERTYPECAMERATEXTURE=5,rwRASTERDONTALLOCATE=128};\n'
owners+='namespace rw {enum {'+','.join(name+'='+str(100+index) for index,(name,_) in enumerate(STENCIL))+',STENCILALWAYS=8,STENCILEQUAL=3,STENCILKEEP=1,STENCILREPLACE=3};void SetRenderState(int32,uint32);}\n'
owners+='enum {'+','.join('D3DRS_'+legacy+'='+str(200+index) for index,(_,legacy) in enumerate(STENCIL))+',D3DCMP_ALWAYS=8,D3DCMP_EQUAL=3,D3DSTENCILOP_KEEP=1,D3DSTENCILOP_REPLACE=3};\n'
owners+='''RwRaster *RwCameraGetRaster(RwCamera*);
float RwCameraGetNearClipPlane(RwCamera*);
int32 RwRasterGetWidth(RwRaster*);
int32 RwRasterGetHeight(RwRaster*);
int32 RwRasterGetDepth(RwRaster*);
RwRaster *RwRasterCreate(int32,int32,int32,uint32);
void RwRasterDestroy(RwRaster*);
RwRaster *RwRasterSubRaster(RwRaster*,RwRaster*,RwRect*);
void RwRasterPushContext(RwRaster*);
void RwRasterPopContext(void);
void RwRasterRenderFast(RwRaster*,int32,int32);
float RwIm2DGetNearScreenZ(void);
void RwIm2DRenderIndexedPrimitive(int32,RwIm2DVertex*,int32,RwImVertexIndex*,int32);
void RwRenderStateSet(RwRenderState,void*);
void RwD3D8SetRenderState(uint32,uint32);
void DefinedState(void);
#ifdef EXTENDED_COLOURFILTER
struct CPostFX {static void Open(RwCamera*);static void Close(void);static void Render(RwCamera*,uint32,uint32,uint32,uint32,int32,uint32);};
#endif
namespace SoftwarePolygons {
bool FramePending(void);
bool CapturingWorldEffects(void);
bool BeginScreenTextureSampling(void);
void RenderScreenRefraction(float,float,float,float,int,int,RwRaster*,float,float,float,float,int);
void RenderScreenTexture(float,float,float,float,int,int,RwRaster*,int,int,int,int,bool);
}
'''
for name,(args,_) in SETTERS.items():owners+='void RwIm2DVertexSet'+name+'(RwIm2DVertex *vertex,'+args+');\n'
owners+='''#ifdef ORIGINAL
#include "original-api.h"
#else
#include "MBlur_native.h"
#endif
'''
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct TestBlurArgs {
    void *camera,*raster,*vertices,*rectangle;
    int32_t *x,*y,*w,*h;
    uint32_t red,green,blue,alpha,blur,blurAlpha;
    int32_t type,fullTexture,discardOpenResult;
    float u1,v1,u2,v2,z,drunkness,x1,y1,x2,y2;
} TestBlurArgs;
typedef struct TestBlurState {void *front;bool justInitialised,scaled,on;float drunkness;int32_t count;} TestBlurState;
enum {OP_Open,OP_Close,OP_Immediate,OP_Vertices,OP_Render,OP_Overlay,OP_SetDrunk,OP_ClearDrunk,OP_Inside,OP_Add,OP_Effects};
#ifdef __cplusplus
extern "C" {
#endif
int32_t TestBlur(unsigned operation,TestBlurArgs *args);
void TestBlurGetState(TestBlurState *state);
void TestBlurSetState(const TestBlurState *state);
#ifdef __cplusplus
}
#endif
'''
before='''case OP_Open:
#if defined(GTA_PS2) && !defined(EXTENDED_COLOURFILTER)
    // Discard only the known successful fresh-open result because the original PS2 body has no return there
    if(args->discardOpenResult){CMBlur::MotionBlurOpen((RwCamera*)args->camera);return 0;}
#endif
    return CMBlur::MotionBlurOpen((RwCamera*)args->camera);
case OP_Close:return CMBlur::MotionBlurClose();
case OP_Immediate:CMBlur::CreateImmediateModeData((RwCamera*)args->camera,(RwRect*)args->rectangle);break;
case OP_Vertices:{RwRGBA colour={(uint8)args->red,(uint8)args->green,(uint8)args->blue,(uint8)args->alpha};CMBlur::CreateImmediateModeData((RwCamera*)args->camera,(RwRect*)args->rectangle,(RwIm2DVertex*)args->vertices,colour,args->u1,args->v1,args->u2,args->v2,args->z,args->fullTexture);break;}
case OP_Render:CMBlur::MotionBlurRender((RwCamera*)args->camera,args->red,args->green,args->blue,args->blur,args->type,args->blurAlpha);break;
case OP_Overlay:{RwRGBA colour={(uint8)args->red,(uint8)args->green,(uint8)args->blue,(uint8)args->alpha};CMBlur::OverlayRender((RwCamera*)args->camera,(RwRaster*)args->raster,colour,args->type,args->blurAlpha);break;}
case OP_SetDrunk:CMBlur::SetDrunkBlur(args->drunkness);break;
case OP_ClearDrunk:CMBlur::ClearDrunkBlur();break;
case OP_Inside:return CMBlur::PosInside((RwRect*)args->rectangle,args->x1,args->y1,args->x2,args->y2);
case OP_Add:return CMBlur::AddRenderFx((RwCamera*)args->camera,(RwRect*)args->rectangle,args->z,(FxType)args->type);
case OP_Effects:CMBlur::OverlayRenderFx((RwCamera*)args->camera,(RwRaster*)args->raster);break;
'''
after='''case OP_Open:
#if defined(GTA_PS2) && !defined(EXTENDED_COLOURFILTER)
    // Avoid consuming an unspecified result while retaining all defined early-return comparisons
    if(args->discardOpenResult){MBlur_MotionBlurOpen(args->camera);return 0;}
#endif
    return MBlur_MotionBlurOpen(args->camera);
case OP_Close:return MBlur_MotionBlurClose();
case OP_Immediate:MBlur_CreateImmediateModeData(args->camera,rect);break;
case OP_Vertices:{MBlurColour colour={(uint8_t)args->red,(uint8_t)args->green,(uint8_t)args->blue,(uint8_t)args->alpha};MBlur_CreateImmediateModeVertices(args->camera,rect,args->vertices,colour,args->u1,args->v1,args->u2,args->v2,args->z,args->fullTexture);break;}
case OP_Render:MBlur_MotionBlurRender(args->camera,args->red,args->green,args->blue,args->blur,args->type,args->blurAlpha);break;
case OP_Overlay:{MBlurColour colour={(uint8_t)args->red,(uint8_t)args->green,(uint8_t)args->blue,(uint8_t)args->alpha};MBlur_OverlayRender(args->camera,args->raster,colour,args->type,args->blurAlpha);break;}
case OP_SetDrunk:MBlur_SetDrunkBlur(args->drunkness);break;
case OP_ClearDrunk:MBlur_ClearDrunkBlur();break;
case OP_Inside:return MBlur_PosInside(rect,args->x1,args->y1,args->x2,args->y2);
case OP_Add:return MBlur_AddRenderFx(args->camera,rect,args->z,(FxType)args->type);
case OP_Effects:MBlur_OverlayRenderFx(args->camera,args->raster);break;
'''
for phase,cases in [('before',before),('after',after)]:
    # Enter every complete original or C operation through independent SDK and pure C consumers
    value=('#include "owners.h"\n' if phase=='before' else '#include "MBlur.h"\n')+'#include "entry.h"\nint32_t TestBlur(unsigned operation,TestBlurArgs *args)\n{\n    // Dispatch complete owner operations without reproducing their blur or effect policy\n'
    if phase=='after':value+='    MBlurRectRef rect={args->x,args->y,args->w,args->h};\n'
    value+='    switch(operation){\n'+cases+'    }return 0;\n}\n'
    fields={'front':'pFrontBuffer','justInitialised':'ms_bJustInitialised','scaled':'ms_bScaledBlur','on':'BlurOn','drunkness':'Drunkness','count':'pBufVertCount'}
    value+='void TestBlurGetState(TestBlurState *state)\n{\n    // Observe each original shared variable through the independent public interface\n'
    for key,name in fields.items():value+='    state->'+key+'='+('CMBlur::' if phase=='before' else 'MBlur_')+name+';\n'
    value+='}\nvoid TestBlurSetState(const TestBlurState *state)\n{\n    // Set caller-controlled test inputs directly in the original live globals without shadow state\n'
    for key,name in fields.items():value+='    '+('CMBlur::' if phase=='before' else 'MBlur_')+name+'='+('(RwRaster*)' if phase=='before' and key=='front' else '')+'state->'+key+';\n'
    value+='}\n';(probe/(phase+'-caller.'+('cpp' if phase=='before' else 'c'))).write_text(value,newline='\n')
original=re.sub(r'^#include[^\n]*\n','',(out/'before/src/renderer/MBlur.cpp').read_text(),flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
printf='''#pragma once
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
int TestBlurPrintf(const char*,...);
#ifdef __cplusplus
}
#endif
#define printf TestBlurPrintf
'''
for name,value in [('owners.h',owners),('entry.h',entry),('printf.h',printf),('original-api.h',(out/'before/src/renderer/MBlur.h').read_text()),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "MBlur_backend.h"\n'+native)]:
    # Retain entire producer bodies and all typed SDK trampoline code against controlled external primitives
    (probe/name).write_text(value,newline='\n')
template=root/'utils/tests/mblur_fixture.cpp.in'
if template.exists():
    # Supply every actual SDK vertex primitive independently of both complete owner implementations
    fixture=template.read_text()
    fields={'ScreenX':'x','ScreenY':'y','ScreenZ':'z','CameraZ':'cameraZ','RecipCameraZ':'reciprocal','U':'u','V':'v'}
    for index,(name,(args,values)) in enumerate(SETTERS.items()):
        # Observe all original SDK arguments including reciprocal texture-coordinate inputs and narrowed channels
        fixture+='\nvoid RwIm2DVertexSet'+name+'(RwIm2DVertex *vertex,'+args+')\n{\n    // Retain exact SDK setter arguments and initialized native vertex fields\n    event('+str(100+index)+');'
        if name=='IntRGBA':fixture+='uint8_t values[]={red,green,blue,alpha};hashBytes(values,sizeof(values));vertex->colour={red,green,blue,alpha};'
        else:
            fixture+='hashBytes(&value,sizeof(value));vertex->'+fields[name]+'=value;'
            if name in ['U','V']:fixture+='hashBytes(&reciprocal,sizeof(reciprocal));'
        fixture+='\n}\n'
    for index,name in enumerate(['AmbientRed','AmbientGreen','AmbientBlue','DirectionalRed','DirectionalGreen','DirectionalBlue']):
        # Supply independently observable time-of-day inputs rather than copying overlay tint policy into the fixture
        fixture+='\nfloat TimeCycle_Get'+name+'(void)\n{\n    // Return controlled external time-of-day data at each original lookup site\n    event('+str(70+index)+');return lightColours['+str(index)+'];\n}\n'
    (probe/'fixture.cpp').write_text(fixture,newline='\n')
print('Prepared all 11 complete original/C blur operations and independently compiled public callers')
