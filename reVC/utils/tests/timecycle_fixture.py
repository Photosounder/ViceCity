"""Prepare whole original/C lighting owner comparisons with all canonical table observations."""
from pathlib import Path
import sys,re,json
import timecycle_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/timecycle-c-tests';production='--production' in sys.argv;include=root/'src/renderer' if production else out/'preview'
old=(out/'before/src/renderer/Timecycle.cpp').read_text();header=(out/'before/src/renderer/Timecycle.h').read_text();vector=(out/'before/src/math/Vector.cpp').read_text();fields=m.fields(header)
if production:
    for path in json.loads((out/'changes.json').read_text()):
        original=(out/'before'/path).read_text();assert (root/path).read_text()==(m.project(original) if path.startswith('build/') else m.calls(original)),path
        assert b'\r' not in (root/path).read_bytes(),path
    assert (include/'TimeCycleSystem.c').read_text()==m.kernel(old,header,vector) and (include/'Timecycle_backend.h').read_text()==m.BACKEND
for path in ['src/math/Vector.h','src/math/maths.h']:(out/Path(path).name).write_text((out/'before'/path).read_text(),newline='\n')
(out/'original-Timecycle.h').write_text(header.replace('class CTimeCycle\n{','class CTimeCycle\n{\npublic:'),newline='\n')
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "config.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
typedef uint8_t uint8;typedef int8_t int8;typedef int16_t int16;typedef uint16_t uint16;typedef int32_t int32;typedef uint32_t uint32;
struct RwV3d {float x,y,z;};
#define nil NULL
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define ASSERT assert
#include "maths.h"
#include "Vector.h"
#include "Draw_constants.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#undef debug
#define debug(...) TestTimeCycle_Debug(__VA_ARGS__)
void TestTimeCycle_Debug(const char *format,...);
'''
(out/'common.h').write_text(common,newline='\n')
owners='''#pragma once
#include "common.h"
#include "Clock.h"
#include "Weather.h"
#include "Timer.h"
#include "ZoneCull.h"
#include "FileMgr.h"
#ifdef TEST_TIMECYCLE_ORIGINAL
#include "original-Timecycle.h"
typedef CVector TimeCycleVector;
#define TC(name) CTimeCycle::name
#else
#include "'''+(include/'Timecycle.h').as_posix()+'''"
#define TC(name) TimeCycle_##name
#endif
enum {MOTION_BLUR_NONE=0,MOTION_BLUR_SNIPER=1,MOTION_BLUR_LIGHT_SCENE=2};
extern uint8 work_buff[55000];
struct TestObservedBlur {int value;operator int()const;TestObservedBlur &operator=(int);};
class TestCycleCamera {public:TestObservedBlur m_BlurType;CVector forward;const CVector &GetForward();void SetMotionBlur(int red,int green,int blue,int alpha,int type);};
extern TestCycleCamera TheCamera;
class CCutsceneMgr {public:static bool IsRunning();};
class CShadows {public:static void CalcPedShadowValues(CVector direction,float *frontX,float *frontY,float *sideX,float *sideY,float *displacementX,float *displacementY);static void RealCalcPedShadowValues(CVector direction,float *frontX,float *frontY,float *sideX,float *sideY,float *displacementX,float *displacementY);};
#include "entry.h"
'''
(out/'owners.h').write_text(owners,newline='\n')
api='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
caller='#include "'+(include/'Timecycle.h').as_posix()+'"\n#include "entry.h"\n'
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
signatures=re.findall(r'(?m)^(void|float|int32_t|TimeCycleVector \*)\s*(TimeCycle_\w+)\(([^\n]*)\);',m.header(header));assert len(signatures)==69
for ret,name,args in signatures:
    values=[] if args=='void' else [re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')];operation=name[len('TimeCycle_'):];member='Interpolate' if operation.startswith('Interpolate') else operation
    api+=ret+' Run_'+name+'('+args+');\n'+ret+' Before_'+name+'('+args+');\n'
    original+='extern "C" '+ret+' Before_'+name+'('+args+')\n{\n    // Invoke the entire original member without changing its declared scalar conversions\n    '+('return ' if ret!='void' else '')+('&' if operation=='GetSunDirection' else '')+'CTimeCycle::'+member+'('+','.join(values)+');\n}\n'
    caller+=ret+' Run_'+name+'('+args+')\n{\n    // Invoke the complete public operation through a standalone C11 translation unit\n#ifdef TEST_TIMECYCLE_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if ret!='void' else '')+name+'('+','.join(values)+');\n#endif\n}\n'
api+='void TestCycle_Coefficients(float *values);\nvoid BeforeCycle_Coefficients(float *values);\nvoid AfterCycle_Coefficients(float *values);\nvoid TestCycle_Normalise(float *x,float *y,float *z);\nvoid BeforeCycle_Normalise(float *x,float *y,float *z);\nvoid AfterCycle_Normalise(float *x,float *y,float *z);\n#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n')
for name,args,values in [('Coefficients','float *values','values'),('Normalise','float *x,float *y,float *z','x,y,z')]:caller+='void TestCycle_'+name+'('+args+')\n{\n    // Observe the original private interpolation or complete native-vector primitive through the C caller\n#ifdef TEST_TIMECYCLE_ORIGINAL\n    BeforeCycle_'+name+'('+values+');\n#else\n    AfterCycle_'+name+'('+values+');\n#endif\n}\n'
def coefficients(prefix):
    # Observe every original private coefficient without introducing a public production API
    return prefix+'void '+('Before' if prefix else 'After')+'Cycle_Coefficients(float *values)\n{\n    // Read all four original private bilinear coefficients after each complete operation\n    '+''.join('values['+str(i)+']=interp_c'+str(i)+';' for i in range(4))+'\n}\n'
original+=coefficients('extern "C" ')
original+='''extern "C" void BeforeCycle_Normalise(float *x,float *y,float *z)
{
    // Execute the complete original native-vector normalization primitive on the same scalar inputs
    CVector value(*x,*y,*z);value.Normalise();*x=value.x;*y=value.y;*z=value.z;
}
'''
(out/'original.cpp').write_text(original,newline='\n');(out/'caller.c').write_text(caller,newline='\n')
kernel='#include "'+(include/'TimeCycleSystem.c').as_posix()+'"\n'+coefficients('')+'''void AfterCycle_Normalise(float *x,float *y,float *z)
{
    // Exercise the complete C normalization helper independently of the guaranteed nonzero sun direction
    TimeCycleVector value={*x,*y,*z};TimeCycle_Normalise(&value);*x=value.x;*y=value.y;*z=value.z;
}
''';(out/'kernel.c').write_text(kernel,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(include/'Timecycle_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
start=vector.index('void\nCVector::Normalise(void)');normalise=vector[start:vector.index('\n}',start)+2]
shadows=(out/'before/src/renderer/Shadows.cpp').read_text();start=shadows.index('void\nCShadows::CalcPedShadowValues');shadow=shadows[start:shadows.index('\n}',start)+2].replace('CShadows::CalcPedShadowValues','CShadows::RealCalcPedShadowValues')
(out/'native.cpp').write_text('#include "owners.h"\n'+normalise+'\n'+shadow+'\n',newline='\n')
state='static void ObserveFields()\n{\n    // Observe all 119 complete canonical arrays and scalars without hashing a class or padding\n'
seed='static void SeedFields(unsigned salt)\n{\n    // Seed every original table, current scalar and complete sixteen-element history with defined values\n'
for index,(kind,name,dims) in enumerate(fields):
    state+='    Bytes(&TC('+name+'),sizeof(TC('+name+')));\n'
    target='TC('+name+')'
    if dims=='[NUMHOURS][NUMWEATHERS]':seed+='    for(int h=0;h<NUMHOURS;h++)for(int w=0;w<NUMWEATHERS;w++)'+target+'[h][w]=('+m.scalar(kind)+')'+('500+(h*97+w*211+salt)%5000' if kind=='int16' else '(h*13+w*37+salt)%120' if kind=='int8' else '(h*13+w*37+salt)%250')+';\n'
    elif kind=='CVector':seed+='    for(int i=0;i<16;i++){'+target+'[i].x=i*.03f;'+target+'[i].y=1.0f;'+target+'[i].z=.2f-i*.01f;}\n'
    elif dims:seed+='    for(int i=0;i<16;i++)'+target+'[i]=(i-7)*.125f;\n'
    else:seed+='    '+target+'='+('1.25f' if kind=='float' else '17')+';\n'
seed+='    TC(m_FogReduction)=0;TC(m_CurrentStoredValue)=0;TC(m_ExtraColour)=0;TC(m_ExtraColourInter)=0.0f;TC(m_bExtraColourOn)=0;\n}\n';state+='}\n'
observe='static void ObserveGetters()\n{\n    // Enter all 62 original inline accessors through their genuine C operations and retain the borrowed vector identity\n'
for kind,name,expression in m.getters(header):
    if kind=='CVector &':observe+='    Check(Run_TimeCycle_GetSunDirection()==&TC(m_VectorToSun)[TC(m_CurrentStoredValue)]);\n'
    elif kind=='float':observe+='    Floating(Run_TimeCycle_'+name+'());\n'
    else:observe+='    Number((uint32_t)Run_TimeCycle_'+name+'());\n'
observe+='}\n'
(out/'observations.h').write_text(state+seed+observe,newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/timecycle.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Prepared all 69 complete original/C operations, 119 canonical storage declarations and whole native normalization/shadow primitives:', 'production' if production else 'staged')
