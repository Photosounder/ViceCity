"""Build whole shadow producers and independent public callers on the controlled SDK."""
from pathlib import Path
import re
import shadow_cameras_schema as schema
import shadow_cameras_resources as resources
import shadow_cameras_effects as effects
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shadow-cameras-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True);peer=out/'peer-runtime';tree=out/'overlay'
owners=(peer/'owners.h').read_text();owners=re.sub(r'#ifdef ORIGINAL\n#include "original-api.h"\n#else\n#include "RwHelper.h"\n#endif', '#include "RwHelper.h"',owners)
owners=owners.replace('struct RwTexture {unsigned id;};','struct RwTexture {unsigned id;bool alive;RwRaster *raster;int32 address,filter;};').replace('struct RwObject {','struct RwRaster;\nstruct RwObject {')
owners=owners.replace('RwV2d viewWindow;','RwV2d viewWindow;float nearClip,farClip;int32 projection;')
owners+='\n#include "shadow-sdk.h"\n'
(probe/'owners.h').write_text(owners,newline='\n')
for name in ['entry.h','after-caller.c']:
    # Retain the existing complete C helper peer and independent caller for nested SDK tests
    (probe/name).write_bytes((peer/name).read_bytes())
for name,source in [('helper-native.cpp',(tree/'src/rw/RwHelper_native.cpp').read_text()),('shadow-native.cpp',(tree/'src/renderer/Shadow_native.cpp').read_text())]:
    # Compile the complete actual primitive boundaries against independently controlled SDK declarations
    (probe/name).write_text('#include "owners.h"\n#include "'+('RwHelper_backend.h' if name=='helper-native.cpp' else 'Shadow_backend.h')+'"\n'+re.sub(r'^#include[^\n]*\n','',source,flags=re.M),newline='\n')
for cls in ['ShadowCamera','CutsceneShadow']:
    # Preserve every original operation body and class declaration in independent translation units
    header=(out/f'before/src/renderer/{cls}.h').read_text().replace('#include "ShadowCamera.h"','#include "original-ShadowCamera.h"')
    (probe/f'original-{cls}.h').write_text(header,newline='\n')
    source=(out/f'before/src/renderer/{cls}.cpp').read_text()
    (probe/f'original-{cls}.cpp').write_text('#include "owners.h"\n#include "original-'+cls+'.h"\n'+re.sub(r'^#include[^\n]*\n','',source,flags=re.M),newline='\n')
operations=['Camera'+v for v in schema.CAMERA_OPERATIONS]+['Parent'+v for v in schema.CUTSCENE_OPERATIONS]+['RenderAtomic']
entry="""#pragma once
#include <stdint.h>
#include <stdbool.h>
enum {"""+','.join('SH_'+v for v in operations)+"""};
typedef struct ShadowArgs {void *self,*object,*output;float values[4];int32_t size,passes,type;bool resample,gradient;uint8_t colour[4];} ShadowArgs;
typedef struct ShadowResult {void *object;float sphere[4];int32_t value;} ShadowResult;
#ifdef __cplusplus
extern "C" {
#endif
void TestShadow(unsigned operation,ShadowArgs *args,ShadowResult *result);
size_t ShadowRecordSize(bool parent);
void ShadowRecordState(void *record,bool parent,uintptr_t *handles,int32_t *flags,float *spheres);
#ifdef __cplusplus
}
#endif
"""
(probe/'shadow-entry.h').write_text(entry,newline='\n')
for original in [True,False]:
    # Exercise the original overloaded methods and the actual C API from independent language callers
    source=('#include "owners.h"\n#include "original-CutsceneShadow.h"\n#include <new>\n' if original else '#include "CutsceneShadow.h"\n#include "Shadow_backend.h"\n')+'#include "shadow-entry.h"\n'
    source+='size_t ShadowRecordSize(bool parent)\n{\n    // Allocate the exact public record size without constructing additional owners\n    return parent?sizeof('+('CCutsceneShadow' if original else 'CutsceneShadow')+'):sizeof('+('CShadowCamera' if original else 'ShadowCamera')+');\n}\n'
    source+='void TestShadow(unsigned operation,ShadowArgs *args,ShadowResult *result)\n{\n    // Invoke complete original or C operations while preserving the public argument boundary\n    '+('CShadowCamera' if original else 'ShadowCamera')+' *camera=('+('CShadowCamera' if original else 'ShadowCamera')+'*)args->self;'+('CCutsceneShadow' if original else 'CutsceneShadow')+' *parent=('+('CCutsceneShadow' if original else 'CutsceneShadow')+'*)args->self;\n    result->object=NULL;result->value=0;switch(operation){\n'
    for op in operations:
        # Preserve allocation, overload selection, and SDK argument types only in the original caller
        if op=='RenderAtomic':expr='ShadowRenderCallBack((RpAtomic*)args->object,NULL)' if original else 'Shadow_RenderAtomic(args->object,NULL)';statement='result->object='+expr+';'
        else:
            parentOp=op.startswith('Parent');method=op[6:] if parentOp else op[6:];receiver='parent' if parentOp else 'camera';prefix='CutsceneShadow_' if parentOp else 'ShadowCamera_';kind=('CCutsceneShadow' if parentOp else 'CShadowCamera') if original else ''
            args=[];returns='object'
            if method=='Init':statement=('new('+receiver+') '+kind+'();') if original else prefix+method+'('+receiver+');'
            elif method=='Deinit':statement=(receiver+'->~'+kind+'();') if original else prefix+method+'('+receiver+');'
            else:
                if method=='Create':args=['(RwObject*)args->object','args->size','args->resample','args->passes','args->gradient'] if parentOp and original else ['args->object','args->size','args->resample','args->passes','args->gradient'] if parentOp else ['args->size'];returns='value' if parentOp else 'object'
                elif method=='SetFrustum':args=['args->values[0]']
                elif method=='SetLight':args=['(RpLight*)args->object' if original else 'args->object']
                elif method=='SetCenter':args=['(RwV3d*)args->object' if original else '(ShadowVectorRef){&((ShadowVector*)args->object)->x,&((ShadowVector*)args->object)->y,&((ShadowVector*)args->object)->z}']
                elif method in ['UpdateClump','UpdateAtomic']:args=['('+('RpClump' if method=='UpdateClump' else 'RpAtomic')+'*)args->object' if original else 'args->object']
                elif method in ['RasterResample','RasterBlur','RasterGradient']:args=['(RwRaster*)args->object' if original else 'args->object']+(['args->passes'] if method=='RasterBlur' else [])
                elif method in ['DrawOutlineBorder','DrawBorderAroundTexture']:args=['*(RwRGBA*)args->colour' if original else '(const ShadowColour*)args->colour']
                elif method=='GetShadowCamera':args=['args->type']
                elif method=='SetLightProperties':args=['args->values[0]','args->values[1]','args->gradient']
                if method=='IsInitialized':returns='value'
                if method in ['Destroy','InvertRaster','DrawBorderAroundTexture']:returns=None
                if method=='GetBaseSphere':statement='{'+('RwSphere' if original else 'ShadowSphere')+' value='+('parent->GetBaseSphere()' if original else 'CutsceneShadow_GetBaseSphere(parent)')+';result->sphere[0]=value.center.x;result->sphere[1]=value.center.y;result->sphere[2]=value.center.z;result->sphere[3]=value.radius;}'
                else:
                    actual='Update' if original and method in ['UpdateClump','UpdateAtomic'] else method
                    expr=receiver+'->'+actual+'('+','.join(args)+')' if original else prefix+actual+'('+','.join([receiver]+args)+')'
                    statement=('result->'+returns+'=' if returns else '')+expr+';'
        source+='case SH_'+op+':'+statement+'break;\n'
    source+='}}\n'
    if original:source=source.replace('void TestShadow(', 'RpAtomic *ShadowRenderCallBack(RpAtomic*,void*);\nvoid TestShadow(')
    source+='void ShadowRecordState(void *record,bool isParent,uintptr_t *handles,int32_t *flags,float *spheres)\n{\n    // Observe only defined public record fields and normalize SDK handles in the fixture\n    '+('CCutsceneShadow' if original else 'CutsceneShadow')+' *parent=('+('CCutsceneShadow' if original else 'CutsceneShadow')+'*)record;'+('CShadowCamera' if original else 'ShadowCamera')+' *camera=isParent?&parent->m_Camera:('+('CShadowCamera' if original else 'ShadowCamera')+'*)record;handles[0]=(uintptr_t)camera->m_pCamera;handles[1]=(uintptr_t)camera->m_pTexture;if(isParent){\n        // Retain camera declaration order and leave uninitialized sphere storage unread until creation\n        handles[2]=(uintptr_t)parent->m_ResampleCamera.m_pCamera;handles[3]=(uintptr_t)parent->m_ResampleCamera.m_pTexture;handles[4]=(uintptr_t)parent->m_BlurCamera.m_pCamera;handles[5]=(uintptr_t)parent->m_BlurCamera.m_pTexture;handles[6]=(uintptr_t)parent->m_GradientCamera.m_pCamera;handles[7]=(uintptr_t)parent->m_GradientCamera.m_pTexture;handles[8]=(uintptr_t)parent->m_pObject;handles[9]=(uintptr_t)parent->m_pLight;flags[0]=parent->m_nRwObjectType;flags[1]=parent->m_bResample;flags[2]=parent->m_nBlurPasses;flags[3]=parent->m_bGradient;if(spheres){spheres[0]=parent->m_BoundingSphere.center.x;spheres[1]=parent->m_BoundingSphere.center.y;spheres[2]=parent->m_BoundingSphere.center.z;spheres[3]=parent->m_BoundingSphere.radius;spheres[4]=parent->m_BaseSphere.center.x;spheres[5]=parent->m_BaseSphere.center.y;spheres[6]=parent->m_BaseSphere.center.z;spheres[7]=parent->m_BaseSphere.radius;}}\n}\n'
    (probe/('before-shadow.cpp' if original else 'after-shadow.c')).write_text(source,newline='\n')

# Materialize controlled SDK implementations only after all public owner callers are complete
(probe/'shadow-sdk.h').write_text((root/'utils/tests/shadow_cameras_sdk.h.in').read_text(),newline='\n')
fixture=(peer/'fixture.cpp').read_text().replace('int main(void)','int PriorHelperMain(void)')
fixture=fixture.replace('bytes(expression,strlen(expression));','')
fixture=fixture.replace('bytes(vertices,count*sizeof(*vertices));','for(int32 i=0;i<count;++i){bytes(&vertices[i].x,4*sizeof(float));bytes(&vertices[i].colour,sizeof(RwRGBA));}')
fixture=fixture.replace('event(8,state,(uint32)(uintptr_t)value);renderStates[state]=(uint32)(uintptr_t)value;', 'uint32 token=state==rwRENDERSTATETEXTURERASTER&&(uintptr_t)value>4096?identity(value):(uint32)(uintptr_t)value;event(8,state,token);renderStates[state]=token;')
fixture=fixture.replace('value->alive=true;return value;','value->alive=true;value->nearClip=0.5f;value->farClip=40.0f;return value;',1)
fixture=fixture.replace('++draws;if(insideQuad)', '++draws;ShadowDrawReentry(vertices,count);if(insideQuad)')
fixture='#include <initializer_list>\n#include \"owners.h\"\nvoid ShadowDrawReentry(RwIm2DVertex*,int32);\n'+fixture+'\n'+(root/'utils/tests/shadow_cameras_sdk.cpp.in').read_text()+'\n'+(root/'utils/tests/shadow_cameras_cases.cpp.in').read_text()
(probe/'fixture.cpp').write_text(fixture,newline='\n')
