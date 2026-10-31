"""Prepare whole original/C lighting producers with independently controlled SDK operations."""
from pathlib import Path
import re
import lights_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/lights-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define nil NULL
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
typedef uint8_t uint8;
struct RwV3d {float x,y,z;};
struct RwRGBAReal {float red,green,blue,alpha;};
struct RwMatrix {RwV3d right;uint32_t flags;RwV3d up;uint32_t pad1;RwV3d at;uint32_t pad2;RwV3d pos;uint32_t pad3;};
struct RwFrame {unsigned id;bool alive;RwMatrix matrix;};
struct RpLight {unsigned id;bool alive;int kind,flags;float radius;RwRGBAReal colour;RwFrame *frame;};
struct RpWorld {unsigned id,count;};
enum {rpLIGHTDIRECTIONAL=1,rpLIGHTAMBIENT=2,rpLIGHTLIGHTATOMICS=1,rwCOMBINEREPLACE=0,rwCOMBINEPRECONCAT=1};
#include "maths.h"
#include "Vector.h"
#include "Timecycle.h"
#include "Timer.h"
#include "Weather.h"
#include "ZoneCull.h"
struct CCoronas {static float LightsMult;};
struct CMBlur {static bool BlurOn;};
struct CPostFX {static bool UseBlurColours(void);};
struct TestFrontend {int32_t m_PrefsBrightness;};
extern TestFrontend FrontEndMenuManager;
RpLight *RpLightCreate(int);
void RpLightDestroy(RpLight*);
void RpLightSetFlags(RpLight*,int);
void RpLightSetColor(RpLight*,const RwRGBAReal*);
void RpLightSetRadius(RpLight*,float);
RwFrame *RwFrameCreate(void);
void RwFrameDestroy(RwFrame*);
RwFrame *RpLightGetFrame(RpLight*);
void RpLightSetFrame(RpLight*,RwFrame*);
RwMatrix *RwFrameGetMatrix(RwFrame*);
RwV3d *RwMatrixGetAt(RwMatrix*);
void RwMatrixUpdate(RwMatrix*);
void RwFrameUpdateObjects(RwFrame*);
void RpWorldAddLight(RpWorld*,RpLight*);
void RpWorldRemoveLight(RpWorld*,RpLight*);
void RwFrameRotate(RwFrame*,const RwV3d*,float,int);
void RwFrameTransform(RwFrame*,const RwMatrix*,int);
#ifdef ORIGINAL
#include "original-api.h"
#else
#include "Lights.h"
#endif
'''
entry='''#pragma once
enum {OP_Time,OP_Create,OP_Destroy,OP_Scorch,OP_Normal,OP_Extra,OP_Remove,OP_Scale,OP_Flash,OP_Mild,OP_Marker,OP_Reset,OP_Off,OP_On,OP_Full,OP_Ambient,OP_Objects,OP_Road,OP_Colour};
typedef struct TestLightsColour {float red,green,blue,alpha;} TestLightsColour;
#ifdef __cplusplus
extern "C" {
#endif
void *TestLights(unsigned operation,void *world,const float *values);
void *TestLightsHandle(unsigned index);
#ifdef __cplusplus
}
#endif
'''
cases='''case OP_Time:SetLightsWithTimeOfDayColour((RpWorld*)world);break;
case OP_Create:result=LightsCreate((RpWorld*)world);break;
case OP_Destroy:LightsDestroy((RpWorld*)world);break;
case OP_Scorch:WorldReplaceNormalLightsWithScorched((RpWorld*)world,values[0]);break;
case OP_Normal:WorldReplaceScorchedLightsWithNormal((RpWorld*)world);break;
case OP_Extra:AddAnExtraDirectionalLight((RpWorld*)world,values[0],values[1],values[2],values[3],values[4],values[5]);break;
case OP_Remove:RemoveExtraDirectionalLights((RpWorld*)world);break;
case OP_Scale:SetAmbientAndDirectionalColours(values[0]);break;
case OP_Flash:SetFlashyColours(values[0]);break;
case OP_Mild:SetFlashyColours_Mild(values[0]);break;
case OP_Marker:SetBrightMarkerColours(values[0]);break;
case OP_Reset:ReSetAmbientAndDirectionalColours();break;
case OP_Off:DeActivateDirectional();break;
case OP_On:ActivateDirectional();break;
case OP_Full:SetFullAmbient();break;
case OP_Ambient:SetAmbientColours();break;
case OP_Objects:SetAmbientColoursForPedsCarsAndObjects();break;
case OP_Road:SetAmbientColoursToIndicateRoadGroup((int)values[0]);break;
case OP_Colour:{RwRGBAReal colour={values[0],values[1],values[2],values[3]};SetAmbientColours(&colour);break;}
'''
for phase in ['before','after']:
    # Enter every complete operation through a separate old C++ or new C public caller
    code=cases
    if phase=='after':
        code=code.replace('SetAmbientColours(&colour)','Lights_SetAmbientColour(&colour)').replace('RwRGBAReal','LightsColour').replace('(RpWorld*)world','world')
        for old,new in m.NAMES.items():code=re.sub(r'\b'+old+r'\b',new,code)
    handles='return index==0?pAmbient:index==1?pDirect:pExtraDirectionals[index-2];' if phase=='before' else 'return index==0?Lights_Ambient:index==1?Lights_Direct:Lights_ExtraDirectionals[index-2];'
    value=('#include "owners.h"\n' if phase=='before' else '#include "Lights.h"\n#include <stddef.h>\n')+'#include "entry.h"\n'
    value+='void *TestLights(unsigned operation,void *world,const float *values)\n{\n    // Dispatch the entire original or C owner without reproducing its lighting decisions\n    void *result=NULL;switch(operation){\n'+code+'    }return result;\n}\n'
    value+='void *TestLightsHandle(unsigned index)\n{\n    // Expose each public borrowed light handle through an independent language consumer\n    '+handles+'\n}\n'
    (probe/(phase+'-caller.'+('cpp' if phase=='before' else 'c'))).write_text(value,newline='\n')
original=(out/'before/src/rw/Lights.cpp').read_text()
original=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
vector=(out/'before/src/math/Vector.cpp').read_text();vector=vector[vector.index('void\nCVector::Normalise'):vector.index('CVector\nMultiply3x3')]
fixture=(root/'utils/tests/lights_fixture.cpp.in').read_text()
for index,name in enumerate(re.findall(r'float (TimeCycle_Get(?:Ambient|Directional)\w+)\(void\);',(out/'before/src/renderer/Timecycle.h').read_text())):
    # Supply independent time-of-day scalar inputs and observe each original getter call
    fixture+='\nfloat '+name+'(void)\n{\n    // Return controlled external time-of-day metadata without implementing lighting policy\n    event(30,'+str(index)+');return timeColours['+str(index)+'];\n}\n'
for name,value in [('owners.h',owners),('entry.h',entry),('original-api.h',(out/'before/src/rw/Lights.h').read_text()),('original.cpp','#include "owners.h"\n'+original+'\n'+vector),('native.cpp','#include "owners.h"\n#include "Lights_backend.h"\n'+native),('fixture.cpp',fixture)]:
    # Materialize whole owners and the full actual native bridge against controlled external SDK leaves
    (probe/name).write_text(value,newline='\n')
print('Prepared all 19 original/C lighting operations and the whole native primitive bridge')
