"""Compare all original cloud/background code with the complete C cloud implementation."""
from pathlib import Path
import re,json,runpy
import cloud_background_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/cloud-background-c-tests'
original=(out/'before/src/renderer/Clouds.cpp').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.kernel(old,original) if path.endswith('Clouds.c') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert len(re.findall(r'\nCClouds::\w+\(',m.methods_section(original)))==2
for path in ['src/math/Vector.cpp','src/math/Matrix.cpp','src/math/Rect.cpp','src/renderer/Sprite2d.h']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
runpy.run_path(str(root/'utils/tests/cloud_effects_fixture.py'))
previous=root/'build/cloud-effects-c-tests'
for path in previous.iterdir():
    if path.suffix in ['.h','.c','.cpp']:(out/path.name).write_text(path.read_text(),newline='\n')
extra='''typedef struct TestHorizonState { float current[6],last[6];bool have;uint8_t colors[3][4]; } TestHorizonState;
TestHorizonState TestBh_Get(void);void TestBh_Set(const TestHorizonState *state);
void *TestBh_Color(int32_t index);
void TestBh_Background(int16_t r,int16_t g,int16_t b,int16_t br,int16_t bg,int16_t bb,int16_t alpha);
void TestBh_Horizon(void);void TestBh_Project(CloudsVector in,float *x,float *y);
void TestBh_Mirror(const float *in,float *out);
'''
caller=(out/'caller.h').read_text().replace('#ifdef __cplusplus\n}\n#endif',extra+'#ifdef __cplusplus\n}\n#endif');(out/'caller.h').write_text(caller,newline='\n')
fields=['HorizonX','Clouds_ms_horizonZ','HorizonSkyX','HorizonSkyY','HorizonLineX','HorizonLineY'];statefields=['x','z','skyX','skyY','lineX','lineY']
for before in [True,False]:
    path=out/('original.cpp' if before else 'observed-kernel.c');text=path.read_text()
    if before:
        definitions='\n'.join(line for line in original.splitlines() if line.startswith(('#define SMALLSTRIPHEIGHT','#define HORIZSTRIPHEIGHT')))
        addition='\n'+definitions+'\n#ifndef SQR\n#define SQR(x) ((x)*(x))\n#endif\n'+''.join('CRGBA CClouds::ms_colour'+name+';\n' for name in ['Top','Bottom','BkGrd'])+m.globals_section(original)+'\n'+m.methods_section(original)
        for name in ['CloudRotation','IndividualRotation','ms_cameraRoll','ms_horizonZ','ms_HorizonTilt']:addition=addition.replace('Clouds_'+name,'CClouds::'+name)
        text+=addition+'\nextern "C" {\n'
        text+='void *TestBh_Color(int32_t index)\n{\n    // Address the actual original typed color owner for alias-aware observations\n    CRGBA *colors[]={&CClouds::ms_colourTop,&CClouds::ms_colourBottom,&CClouds::ms_colourBkGrd};return colors[index];\n}\n'
    text+='TestHorizonState TestBh_Get(void)\n{\n    // Observe each real current/cache scalar and actual color component independently\n    TestHorizonState result;\n'
    for i,field in enumerate(fields):text+='    result.current['+str(i)+']='+field.replace('Clouds_ms_horizonZ','CClouds::ms_horizonZ' if before else 'Clouds_ms_horizonZ')+';result.last['+str(i)+']=LastSafeHorizon.'+statefields[i]+';\n'
    text+='    result.have=HaveLastSafeHorizon;for(int i=0;i<3;i++){\n        // Copy declared color bytes without assuming any C++ record representation\n'
    if before:text+='        const CRGBA &c=*(const CRGBA *)TestBh_Color(i);result.colors[i][0]=c.r;result.colors[i][1]=c.g;result.colors[i][2]=c.b;result.colors[i][3]=c.a;\n'
    else:text+='        CloudsColorFields c=Clouds_BackendColorFields(i);result.colors[i][0]=*c.r;result.colors[i][1]=*c.g;result.colors[i][2]=*c.b;result.colors[i][3]=*c.a;\n'
    text+='    }return result;\n}\nvoid TestBh_Set(const TestHorizonState *value)\n{\n    // Seed actual scalars and byte fields while retaining color identities and cache storage\n'
    for i,field in enumerate(fields):text+='    '+field.replace('Clouds_ms_horizonZ','CClouds::ms_horizonZ' if before else 'Clouds_ms_horizonZ')+'=value->current['+str(i)+'];LastSafeHorizon.'+statefields[i]+'=value->last['+str(i)+'];\n'
    text+='    HaveLastSafeHorizon=value->have;for(int i=0;i<3;i++){\n        // Update the original actual color fields through their own types\n'
    if before:text+='        CRGBA &c=*(CRGBA *)TestBh_Color(i);c.r=value->colors[i][0];c.g=value->colors[i][1];c.b=value->colors[i][2];c.a=value->colors[i][3];\n'
    else:text+='        CloudsColorFields c=Clouds_BackendColorFields(i);*c.r=value->colors[i][0];*c.g=value->colors[i][1];*c.b=value->colors[i][2];*c.a=value->colors[i][3];\n'
    text+='    }\n}\n'
    if before:text+='void TestBh_Background(int16_t r,int16_t g,int16_t b,int16_t br,int16_t bg,int16_t bb,int16_t alpha)\n{\n    // Enter the complete original public background method\n    CClouds::RenderBackground(r,g,b,br,bg,bb,alpha);\n}\nvoid TestBh_Horizon(void)\n{\n    // Enter the complete original horizon method\n    CClouds::RenderHorizon();\n}\n'
    text+='void TestBh_Project(CloudsVector in,float *x,float *y)\n{\n    // Exercise the complete original or C projection helper with legal independent coordinates\n'
    text+=('    CVector value(in.x,in.y,in.z);CalcScreenCoorsNoClip(value,*x,*y);\n' if before else '    CalcScreenCoorsNoClip(in,x,y);\n')+'}\n'
    text+='void TestBh_Mirror(const float *in,float *out)\n{\n    // Exercise all six original mirror expressions on actual horizon values\n    HorizonState value={in[0],in[1],in[2],in[3],in[4],in[5]};value=GetMirroredPoleHorizonState(value);\n'
    for i,name in enumerate(statefields):text+='    out['+str(i)+']=value.'+name+';\n'
    text+='}\n'
    if before:text+='}\n'
    path.write_text(text,newline='\n')
host=(out/'host.cpp').read_text()+'\n#include "caller.h"\nextern "C" void *TestBh_Color(int32_t index)\n{\n    // Address the actual preserved typed color owner in the new boundary\n    return &BackgroundColors[index];\n}\n';(out/'host.cpp').write_text(host,newline='\n')
ccaller=(out/'caller.c').read_text()+'''void TestBh_Background(int16_t r,int16_t g,int16_t b,int16_t br,int16_t bg,int16_t bb,int16_t alpha)
{
    // Enter the complete C11 public background API from another C unit
    Clouds_RenderBackground(r,g,b,br,bg,bb,alpha);
}
void TestBh_Horizon(void)
{
    // Enter the complete C11 public horizon API from another C unit
    Clouds_RenderHorizon();
}
''';(out/'caller.c').write_text(ccaller,newline='\n')
vector=(out/'before/src/math/Vector.cpp').read_text();math=(out/'original-math.cpp').read_text()
math+='\n'+m.function(vector,'','CrossProduct')+'\n'+m.function(vector,'','operator\\*')+'\n';(out/'original-math.cpp').write_text(math,newline='\n')
template=(root/'utils/tests/cloud_effects.cpp.in').read_text()
needle='event(11);word(state);word((uint32_t)(uintptr_t)value);return 1;';assert needle in template
template=template.replace(needle,'event(11);word(state);word((uint32_t)(uintptr_t)value);BackgroundStateHook(state,value);return 1;')
last=template.rfind('\n}');template=template[:last]+'\n    return 0;'+template[last:]
prefix='#include "common.h"\nstatic void BackgroundStateHook(RwRenderState,void *);\n#define main CloudEffectsCorpus\n'
(out/'test.cpp').write_text(prefix+template+'\n#undef main\n'+(root/'utils/tests/cloud_background_cases.cpp.in').read_text(),newline='\n')
print('Audited two remaining public methods, eleven complete helpers, original typed color aliases and all six complete changed files')
