"""Audit complete cloud effects and compare the unchanged original rendering methods."""
from pathlib import Path
import re,json
import cloud_effects_migration as m
import timecycle_migration as cycle
import timecycle_test_dependencies as cycle_dependencies
root=Path(__file__).resolve().parents[2];out=root/'build/cloud-effects-c-tests'

def normalize(source):
    # Compare every original statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Clouds.cpp').read_text();header=(out/'before/src/renderer/Clouds.h').read_text()
for name,value in [('Clouds.c',m.kernel(old)),('Clouds_control.h',m.background.calls(m.HEADER)),('Clouds_backend.h',m.background.calls(m.BACKEND_HEADER))]:assert (root/'src/renderer'/name).read_text()==value
assert len(re.findall(r'\nCClouds::\w+\(',m.section(old)))==5
for path in ['src/core/common.h','src/core/config.h','src/math/Vector.h','src/math/Vector.cpp','src/math/Vector2D.h','src/math/Matrix.h','src/math/Rect.h','src/math/maths.h','src/renderer/Sprite.h','src/renderer/Weather.h','src/renderer/Coronas.h','src/renderer/Timecycle.h','src/core/Clock.c']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    if path=='src/renderer/Weather.h':assert m.calls(saved.read_text())==(root/path).read_text(),path
    elif path=='src/renderer/Timecycle.h':cycle.verify_dependency(saved,root/path)
    else:assert saved.read_bytes()==(root/path).read_bytes(),path
    if path.startswith('src/math/') and path.endswith('.h'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
common=(root/'build/special-fx-c-tests/common.h').read_text();saved=out/'before/fixture-common.h'
if not saved.exists():saved.write_text(common,newline='\n')
assert saved.read_text()==common
for name in ['Clouds_control.h','Clouds_backend.h']:(out/name).write_text((root/'src/renderer'/name).read_text(),newline='\n')
extra='''
#ifndef NATIVE_PROJECT_HEADERS
struct RwRaster;
RwTexture *RwTextureRead(const char *,const char *);
RwBool RwTextureDestroy(RwTexture *);
RwRaster *RwTextureGetRaster(const RwTexture *);
RwFrame *RwCameraGetFrame(const RwCamera *);
RwMatrix *RwFrameGetMatrix(RwFrame *);
RwReal RwCameraGetNearClipPlane(const RwCamera *);
RwReal RwCameraGetFarClipPlane(const RwCamera *);
void RwV3dAdd(RwV3d *,const RwV3d *,const RwV3d *);
#endif
'''
(out/'common.h').write_text(common+extra,newline='\n')
sprite=(out/'before/src/renderer/Sprite.h').read_text();methods=[]
for name in ['CalcScreenCoors','InitSpriteBuffer','FlushSpriteBuffer','RenderOneXLUSprite','RenderBufferedOneXLUSprite','RenderBufferedOneXLUSprite_Rotate_Dimension','RenderBufferedOneXLUSprite_Rotate_Aspect','RenderBufferedOneXLUSprite_Rotate_2Colours']:methods.append(next(line.strip() for line in sprite.splitlines() if re.search(r'\b'+name+r'\(',line)))
owners='''#pragma once
#include "Clouds_control.h"
#include "Clock.h"
#include "Timer.h"
#include "General.h"
#include "RenderBuffer.h"
class CSprite { public:
'''+ '\n'.join(methods)+'''
};
class CTxdStore { public:static void PushCurrentTxd(void);static int FindTxdSlot(const char *);static void SetCurrentTxd(int);static void PopCurrentTxd(void); };
class CCamera { public:float Orientation;CVector position,up,forward;CMatrix m_viewMatrix;CVector &GetPosition(void){return position;}CVector &GetUp(void){return up;}CVector &GetForward(void){return forward;} };
extern CCamera TheCamera;
class CGame { public:static bool CanSeeOutSideFromCurrArea(void); };
class CCoronas { public:static float SunScreenX,SunScreenY;static int MoonSize;static bool SunBlockedByClouds; };
extern RwTexture *gpCoronaTexture[9];
class CWeather { public:static float Wind,Foggyness,CloudCoverage,ExtraSunnyness,Rainbow; };
class CTimeCycle { public:
static int32 colors[9];static CVector sun;static float ambient[3];
'''+''.join('static int32 Get'+name+'(void){return colors['+str(i)+'];}\n' for i,name in enumerate(['LowCloudsRed','LowCloudsGreen','LowCloudsBlue','FluffyCloudsTopRed','FluffyCloudsTopGreen','FluffyCloudsTopBlue','FluffyCloudsBottomRed','FluffyCloudsBottomGreen','FluffyCloudsBottomBlue']))+'''static CVector &GetSunDirection(void){return sun;}
static float GetAmbientRed(void){return ambient[0];}static float GetAmbientGreen(void){return ambient[1];}static float GetAmbientBlue(void){return ambient[2];}
};
extern bool gbShowCollisionPolys;
namespace SoftwarePolygons { bool CapturingWorldEffects(void); }
class CSprite2d { public:
static void DrawAnyRect(float,float,float,float,float,float,float,float,const CRGBA &,const CRGBA &,const CRGBA &,const CRGBA &);
static void DrawRect(const CRect &,const CRGBA &,const CRGBA &,const CRGBA &,const CRGBA &);
};
'''
owners+=cycle_dependencies.declarations(m.HOST+m.background.HOST)
(out/'owners.h').write_text(owners,newline='\n')
for name in ['Sprite.h','TxdStore.h','Camera.h','Game.h','Coronas.h','Weather.h','Timecycle.h','Sprite2d.h','SoftwarePolygons.h']:(out/name).write_text('#pragma once\n#include "owners.h"\n',newline='\n')
(out/'original-Clouds.h').write_text(header,newline='\n')
tables=re.findall(r'float (\w+)\[(\d+)\] =',old);assert sum(int(n) for name,n in tables)==174
proto='''typedef struct TestCloudState { float rotation;uint32_t individual;float roll,horizon,tilt;void *textures[5];bool visible[37];float tables[174];uint8_t bow[18]; } TestCloudState;
TestCloudState TestCl_Get(void);void TestCl_Set(const TestCloudState *state);
void TestCl_Init(void);void TestCl_Shutdown(void);void TestCl_Update(void);void TestCl_Render(void);void TestCl_Env(void *camera);
void TestCl_BindVisibility(bool *flags);
void TestCl_Quad(CloudsVector center,CloudsVector right,CloudsVector up,float w,float h,float rotation,CloudsColor top,CloudsColor bottom,bool same);
'''
(out/'caller.h').write_text('#pragma once\n#include "Clouds_control.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+proto+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
for original in [True,False]:
    owner='CClouds::' if original else 'Clouds_'
    text=('#include "common.h"\n#include "owners.h"\n#include "original-Clouds.h"\n#include "caller.h"\nRwTexture *gpCloudTex[5];\n'+''.join(('uint32' if kind=='uint32_t' else kind)+' CClouds::'+name+';\n' for kind,name in m.SCALARS)+m.section(old)) if original else (root/'src/renderer/Clouds.c').read_text()
    needle='static bool bCloudOnScreen[37];';assert needle in text
    text=text.replace(needle,needle+'\n        TestCl_BindVisibility(bCloudOnScreen);')
    if not original:text='#include "caller.h"\n'+text
    if original:text+='\nextern "C" {\n'
    text+='\nstatic bool *Visibility;\nvoid TestCl_BindVisibility(bool *flags)\n{\n    // Observe the real function-local static array through its own correctly typed pointer\n    Visibility=flags;\n}\n'
    text+='TestCloudState TestCl_Get(void)\n{\n    // Observe all scalar state, actual texture handles, live flags and unmodified layout tables\n    TestCloudState state;state.rotation='+owner+'CloudRotation;state.individual='+owner+'IndividualRotation;state.roll='+owner+'ms_cameraRoll;state.horizon='+owner+'ms_horizonZ;state.tilt='+owner+'ms_HorizonTilt;\n    for(int i=0;i<5;i++)state.textures[i]=gpCloudTex[i];for(int i=0;i<37;i++)state.visible[i]=Visibility?Visibility[i]:false;\n'
    offset=0
    for name,n in tables:text+='    for(int i=0;i<'+n+';i++)state.tables['+str(offset)+'+i]='+name+'[i];\n';offset+=int(n)
    for i,name in enumerate(['BowRed','BowGreen','BowBlue']):text+='    for(int j=0;j<6;j++)state.bow['+str(i*6)+'+j]='+name+'[j];\n'
    text+='    return state;\n}\nvoid TestCl_Set(const TestCloudState *state)\n{\n    // Seed each real scalar, resource slot and static independently without assuming any owner layout\n    '+owner+'CloudRotation=state->rotation;'+owner+'IndividualRotation=state->individual;'+owner+'ms_cameraRoll=state->roll;'+owner+'ms_horizonZ=state->horizon;'+owner+'ms_HorizonTilt=state->tilt;\n    for(int i=0;i<5;i++)gpCloudTex[i]='+('(RwTexture *)' if original else '')+'state->textures[i];for(int i=0;i<37;i++)if(Visibility)Visibility[i]=state->visible[i];\n}\n'
    if original:
        for name in ['Init','Shutdown','Update','Render']:text+='void TestCl_'+name+'(void)\n{\n    // Enter the complete unchanged original effect method\n    CClouds::'+name+'();\n}\n'
    text+='void TestCl_Env(void *camera)\n{\n    // Exercise the exact optional reflection method and its guards\n#ifdef REVC_SOFTWARE_POLYGONS\n    '+owner+'RenderForEnvMap('+('(RwCamera *)' if original else '')+'camera);\n#else\n    (void)camera;\n#endif\n}\n'
    text+='void TestCl_Quad(CloudsVector center,CloudsVector right,CloudsVector up,float w,float h,float rotation,CloudsColor top,CloudsColor bottom,bool same)\n{\n    // Compare the whole private quad algorithm using legal vectors and colors in each language\n#ifdef REVC_SOFTWARE_POLYGONS\n'
    if original:text+='    CVector c(center.x,center.y,center.z),r(right.x,right.y,right.z),u(up.x,up.y,up.z);CRGBA t(top.r,top.g,top.b,top.a),b(bottom.r,bottom.g,bottom.b,bottom.a);StoreEnvMapCloudQuad(c,r,u,w,h,rotation,t,same?t:b);\n'
    else:text+='    StoreEnvMapCloudQuad(&center,&right,&up,w,h,rotation,&top,same?&top:&bottom);\n'
    text+='#else\n    (void)center;(void)right;(void)up;(void)w;(void)h;(void)rotation;(void)top;(void)bottom;(void)same;\n#endif\n}\n'
    if original:text+='}\n'
    (out/('original.cpp' if original else 'observed-kernel.c')).write_text(text,newline='\n')
caller='#include "caller.h"\n'
for name in ['Init','Shutdown','Update','Render']:caller+='void TestCl_'+name+'(void)\n{\n    // Enter the complete public effect method from a separate C11 unit\n    Clouds_'+name+'();\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "common.h"\n#include "owners.h"\n'+cycle.calls(m.HOST+m.background.HOST)+cycle_dependencies.definitions(m.HOST+m.background.HOST),newline='\n')
vector=(out/'before/src/math/Vector.cpp').read_text();a=vector.index('void\nCVector::Normalise');b=vector.index('\nCVector\nCrossProduct',a)
matrix=(root/'src/math/Matrix.cpp').read_text();rect=(root/'src/math/Rect.cpp').read_text();ra=rect.index('CRect::CRect(float');rb=rect.index('\n}',ra)+2
(out/'original-math.cpp').write_text('#include "common.h"\n'+vector[a:b]+'\n'+m.function(matrix,'CMatrix::','CMatrix')+'\n'+m.function(matrix,'CMatrix::','~CMatrix')+'\n'+rect[ra:rb],newline='\n')
absolute=(root/'build/boat-foam-c-tests/fast-absf.c').read_text()
actualAbs=Path('C:/msys/home/rouziclib/math/ieee754.c').read_text()
assert normalize(m.function(absolute,'','fast_absf'))==normalize(m.function(actualAbs,'','fast_absf'))
(out/'fast-absf.c').write_text(absolute,newline='\n')
unused='''
extern "C" uint32_t Timer_frameCounter;
uint32_t Timer_frameCounter;
float CTimeCycle::ambient[3];bool gbShowCollisionPolys;
bool SoftwarePolygons::CapturingWorldEffects(void)
{
    // Reject accidental background calls in the effects-only regression corpus
    abort();
}
void CSprite2d::DrawAnyRect(float,float,float,float,float,float,float,float,const CRGBA &,const CRGBA &,const CRGBA &,const CRGBA &)
{
    // Reject accidental background calls in the effects-only regression corpus
    abort();
}
void CSprite2d::DrawRect(const CRect &,const CRGBA &,const CRGBA &,const CRGBA &,const CRGBA &)
{
    // Reject accidental background calls in the effects-only regression corpus
    abort();
}
RwBool RwMatrixDestroy(RwMatrix *)
{
    // Reject unexpected ownership of a mock camera's actual CMatrix
    abort();return 0;
}
'''
(out/'test.cpp').write_text((root/'utils/tests/cloud_effects.cpp.in').read_text()+unused,newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all five complete methods, reflection quad geometry, 174 float table entries and seven entire source/project files')
