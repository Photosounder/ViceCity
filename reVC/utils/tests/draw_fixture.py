"""Audit full drawing conversion and generate complete original/C callers."""
from pathlib import Path
import json,re
import player_skin_migration as skins
import draw_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/draw-c-tests'
def normalized(source):return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
paths=json.loads((out/'changes.json').read_text())
for path in paths:
    original=(out/'before'/path).read_text();expected=m.project(original) if path.startswith('build/') else m.calls(original)
    if path=='src/renderer/PlayerSkin.cpp':skins.verify_legacy(original,m.calls,root)
    assert normalized(expected)==normalized((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/Draw.cpp').read_text();header=(out/'before/src/renderer/Draw.h').read_text()
assert normalized(m.kernel(old,header))==normalized((root/'src/renderer/Draw.c').read_text())
assert (root/'src/renderer/Draw_backend.h').read_text()==m.BACKEND_HEADER
assert (root/'src/renderer/Draw_constants.h').read_text()==m.constants((out/'before/src/core/common.h').read_text())
assert len(re.findall(r'static (?:void|float) \w+\([^\n]*\) \{',header))==7
assert len(re.findall(r'^(?:float )?CDraw::\w+\(',old,re.M))==5
enum_end=header.index('class CDraw')
originalHeader='#ifndef NATIVE_PROJECT_HEADERS\n'+header[:enum_end]+'#endif\n'+header[enum_end:]
(out/'original-Draw.h').write_text(originalHeader,newline='\n')
commonInclude='#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/'src/core/common.h').as_posix()+'"\n#else\n#include "common.h"\n#endif\n'
ownerPaths={'Frontend.h':'src/core/Frontend.h','Camera.h':'src/core/Camera.h','CutsceneMgr.h':'src/animation/CutsceneMgr.h'}
def ownerIncludes(source):
    # Use actual owner declarations for native runs and narrow models only for portable comparisons
    for name,path in ownerPaths.items():source=source.replace('#include "'+name+'"','#ifdef NATIVE_PROJECT_HEADERS\n#include "'+(root/path).as_posix()+'"\n#else\n#include "'+name+'"\n#endif')
    return source
(out/'original.cpp').write_text(ownerIncludes(old.replace('#include "common.h"',commonInclude).replace('#include "Draw.h"','#include "original-Draw.h"')),newline='\n')
common='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "config.h"
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#include "Draw_constants.h"
typedef uint8_t uint8;
typedef int8_t int8;
inline float Atan(float x) { return atanf(x); }
inline double DrawTestTan(double x) { return tan(x); }
#define tan(x) DrawTestTan((double)(x))
struct DrawTestScreen { int32_t width,height; };
extern DrawTestScreen RsGlobal;
struct DrawTestRouz { float fov_mul; };
extern DrawTestRouz rouz;
#if defined(GTA_PS2)
#ifdef GTA_PAL
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)512)
#else
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)448)
#endif
#else
#define SCREEN_WIDTH ((float)RsGlobal.width)
#define SCREEN_HEIGHT ((float)RsGlobal.height)
#endif
'''
(out/'common.h').write_text(common,newline='\n')
(out/'Frontend.h').write_text('#pragma once\nstruct DrawTestMenu { int8_t m_PrefsUseWideScreen; };\nextern DrawTestMenu FrontEndMenuManager;\n',newline='\n')
(out/'Camera.h').write_text('#pragma once\nstruct DrawTestCamera { bool m_WideScreenOn; };\nextern DrawTestCamera TheCamera;\n',newline='\n')
(out/'CutsceneMgr.h').write_text('#pragma once\nclass CCutsceneMgr { public: static bool running; static bool IsRunning(void) { return running; } };\n',newline='\n')
profiles={
 'default':'','legacy':'#undef ASPECT_RATIO_SCALE\n#undef PROPER_SCALING\n#undef FIX_RADAR\n#undef FIX_SPRITES\n',
 'aspect_only':'#undef PROPER_SCALING\n','force_pc':'#define FORCE_PC_SCALING\n',
 'ps2':'#define GTA_PS2\n','pal':'#define GTA_PS2\n#define GTA_PAL\n',
}
for name,extra in profiles.items():(out/(name+'.h')).write_text('#pragma once\n#include "config.h"\n'+extra,newline='\n')
functions=[('void','SetNearClipZ','float nearclip','nearclip',''),('float','GetNearClipZ','void','',''),('void','SetFarClipZ','float farclip','farclip',''),('float','GetFarClipZ','void','',''),('void','SetFOV','float fov','fov',''),('float','GetFOV','void','',''),('float','GetScaledFOV','void','',''),('float','CalculateAspectRatio','void','',''),('float','ConvertFOV','float fov','fov','ASPECT_RATIO_SCALE'),('float','GetAspectRatio','void','',''),('void','SetAspectRatio','float ratio','ratio',''),('float','ScaleY','float y','y','PROPER_SCALING')]
globals=[('float','ms_fLODDistance',''),('uint8_t','FadeValue',''),('uint8_t','FadeRed',''),('uint8_t','FadeGreen',''),('uint8_t','FadeBlue',''),('bool','ms_bProperScaling','PROPER_SCALING'),('bool','ms_bFixRadar','FIX_RADAR'),('bool','ms_bFixSprites','FIX_SPRITES')]
protos=[];callers={'before':commonInclude+'#include "original-Draw.h"\n#include "caller.h"\n','after':'#include "Draw.h"\n#include "caller.h"\n'}
for typ,name,args,params,condition in functions:
    proto=typ+' TestDraw_'+name+'('+args+')';protos.append(('#ifdef '+condition+'\n' if condition else '')+proto+';\n'+('#endif\n' if condition else ''))
    for variant in callers:
        operation=('CDraw::' if variant=='before' else 'Draw_')+name+'('+params+')'
        callers[variant]+=('#ifdef '+condition+'\n' if condition else '')+proto+'\n{\n\t// Exercise the complete drawing operation through the same external entry point\n\t'+('return ' if typ!='void' else '')+operation+';\n}\n'+('#endif\n' if condition else '')
for typ,name,condition in globals:
    proto=typ+' *TestDraw_'+name+'(void)';protos.append(('#ifdef '+condition+'\n' if condition else '')+proto+';\n'+('#endif\n' if condition else ''))
    for variant in callers:
        callers[variant]+=('#ifdef '+condition+'\n' if condition else '')+proto+'\n{\n\t// Return the original live setting address for direct writes and debug sliders\n\treturn &'+('CDraw::' if variant=='before' else 'Draw_')+name+';\n}\n'+('#endif\n' if condition else '')
(out/'caller.h').write_text('#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "config.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+''.join(protos)+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
for variant,source in callers.items():(out/('original-caller.cpp' if variant=='before' else 'caller.c')).write_text(source,newline='\n')
(out/'test.cpp').write_text(ownerIncludes((root/'utils/tests/draw.cpp.in').read_text().replace('@COMMON@',commonInclude)),newline='\n')
(out/'mock-host.cpp').write_text((root/'src/renderer/Draw.cpp').read_text(),newline='\n')
references=sum(len(re.findall(r'\bCDraw::\w+', (out/'before'/path).read_text())) for path in paths if path.startswith('src/') and path!='src/renderer/Draw.cpp')
(out/'caller-counts.json').write_text(json.dumps({path:len(re.findall(r'\bCDraw::\w+', (out/'before'/path).read_text())) for path in paths if path.endswith('.cpp') and path!='src/renderer/Draw.cpp'},indent=2))
print('Audited 12 drawing operations, seven inline bodies, eight public settings, six typed readers, shared constants,',references,'caller references and 18 full source/project transformations')
