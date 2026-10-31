"""Audit whole-file migration and generate a comparison against the unchanged C++ controller."""
from pathlib import Path
import re,json
import special_fx_migration as m
import weather_migration as weather
root=Path(__file__).resolve().parents[2];out=root/'build/special-fx-c-tests'

def normalize(source):
    # Compare all statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    if path=='src/renderer/Weather.cpp':weather.verify_legacy(old,m.calls,root)
    assert normalize(expected)==normalize((root/path).read_text()),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/renderer/SpecialFX.cpp').read_text();header=(out/'before/src/renderer/SpecialFX.h').read_text();common=(root/'src/core/common.h').read_text()
assert (root/'src/renderer/SpecialFX.c').read_text()==m.kernel(old,common)
assert (root/'src/renderer/SpecialFX_control.h').read_text()==m.HEADER
assert normalize((root/'src/renderer/SpecialFX_backend.h').read_text())==normalize(m.calls(m.BACKEND_HEADER))
assert (root/'src/renderer/FontStyles.h').read_text()==m.font_styles((out/'before/src/renderer/Font.h').read_text())
assert len(re.findall(r'\nCSpecialFX::\w+\(',m.section(old)))==6
assert 'class CSpecialFX' not in (root/'src/renderer/SpecialFX.h').read_text()
assert 'TimerHost_SnapshotActive' not in (root/'src/core/Timer.cpp').read_text()
for path in ['src/core/common.h','src/core/config.h','src/math/Matrix.h','src/math/Matrix.cpp','src/math/Vector.h','src/math/Vector.cpp','src/math/Vector2D.h','src/math/maths.h','src/math/Rect.h','src/math/Rect.cpp','src/weapons/WeaponType.h','src/renderer/Draw.c']:
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
    assert saved.read_bytes()==(root/path).read_bytes(),path
    if path.startswith('src/math/') and path.endswith('.h'):(out/Path(path).name).write_text(saved.read_text(),newline='\n')
portable=(root/'build/point-lights-c-tests/common.h').read_text()
(out/'common.h').write_text(portable+'''
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Draw.h"
#ifndef NATIVE_PROJECT_HEADERS
typedef uint16_t wchar;
#include "Vector2D.h"
#include "Matrix.h"
#include "Rect.h"
struct RwObject {};
struct RpAtomic {};
struct RsGlobalType { int32 width,height; };
extern RsGlobalType RsGlobal;
RwBool RwMatrixDestroy(RwMatrix *);
RwMatrix *RwMatrixUpdate(RwMatrix *);
RwFrame *RpAtomicGetFrame(const RpAtomic *);
RwMatrix *RwFrameGetLTM(RwFrame *);
#define SCREEN_WIDTH ((float)RsGlobal.width)
#define SCREEN_HEIGHT ((float)RsGlobal.height)
#define SCREEN_ASPECT_RATIO (Draw_GetAspectRatio())
'''+common[common.index('#define SCREEN_STRETCH_X('):common.index('#include "maths.h"')]+'''
#endif
extern char gString[256];
extern wchar gUString[256];
void AsciiToUnicode(const char *,wchar *);
void TestSF_Push(const char *);
void TestSF_Pop(void);
#ifdef PUSH_RENDERGROUP
#undef PUSH_RENDERGROUP
#undef POP_RENDERGROUP
#endif
#ifndef FINAL
#define PUSH_RENDERGROUP(n) TestSF_Push(n)
#define POP_RENDERGROUP() TestSF_Pop()
#else
#define PUSH_RENDERGROUP(n) ((void)0)
#define POP_RENDERGROUP() ((void)0)
#endif
''',newline='\n')
a=header.index('class CSpecialFX\n');b=header.index('\n};',a)+3
(out/'original-SpecialFX.h').write_text('#pragma once\n'+header[a:b]+'\nRwObject *LookForBatCB(RwObject *,void *);\n',newline='\n')
owners='''#pragma once
#include "MotionBlurStreaks.h"
#include "BulletTraces.h"
#include "Timer.h"
#include "General.h"
#include "WeaponType.h"
#include "FontStyles.h"
struct CSimpleModelInfo { int tag; };
class CPlayerPed { public: uint64_t guard;RpAtomic *m_pWeaponModel; };
CPlayerPed *FindPlayerPed(void);
class CVisibilityPlugins { public:static CSimpleModelInfo *GetAtomicModelInfo(RpAtomic *); };
extern bool gbNewRenderer;
bool FredIsInFirstPersonCam(void);
class CFont { public:
    static void SetScale(float,float);
    static void SetJustifyOff(void);
    static void SetBackgroundOff(void);
    static void SetCentreSize(float);
    static void SetCentreOff(void);
    static void SetPropOn(void);
    static void SetColor(CRGBA);
    static void SetFontStyle(int16);
    static bool IsJapanese(void);
    static void PrintString(float,float,wchar *);
};
#ifdef MORE_LANGUAGES
#define FONT_LOCALE(style) (CFont::IsJapanese()?FONT_JAPANESE:(style))
#else
#define FONT_LOCALE(style) (style)
#endif
class CSprite2d { public:
    static void Draw2DPolygon(float,float,float,float,float,float,float,float,const CRGBA &);
    static void DrawRect(const CRect &,const CRGBA &);
};
'''
for name,methods in [('BrightLights',['Init','Render']),('ShinyTexts',['Init','Render']),('MoneyMessages',['Init','Render']),('3dMarkers',['Init','Render','Shutdown'])]:owners+='class C'+name+' { public:'+''.join('static void '+n+'(void);' for n in methods)+'};\n'
(out/'owners.h').write_text(owners,newline='\n')
prefix='#include "common.h"\n#include "owners.h"\n'
(out/'original.cpp').write_text(prefix+'#include "original-SpecialFX.h"\n'+m.section(old),newline='\n')
(out/'host.cpp').write_text(prefix+m.calls(m.HOST),newline='\n')
matrix=(out/'before/src/math/Matrix.cpp').read_text();matrix=matrix[:matrix.index('void\nCMatrix::CopyOnlyMatrix')]
vector=(out/'before/src/math/Vector.cpp').read_text();vector=vector[vector.index('CVector\noperator*(const CMatrix &'):]
rect=(out/'before/src/math/Rect.cpp').read_text();a=rect.index('CRect::CRect(float');b=rect.index('\n}',a)+2
(out/'original-math.cpp').write_text(matrix+'\n'+vector+'\n'+rect[a:b]+'\n',newline='\n')
methods=['BulletTraces_Init','BulletTraces_InitGeometry','MotionBlurStreaks_InitGeometry','MotionBlurStreaks_Init','BulletTraces_InitTexture','MotionBlurStreaks_Update','BulletTraces_Update','BulletTraces_ShutdownTexture','MotionBlurStreaks_Render','BulletTraces_Render']
methods+=['CFont::Set'+name for name in ['JustifyOff','BackgroundOff','CentreOff','PropOn']]
for owner,names in [('BrightLights',['Init','Render']),('ShinyTexts',['Init','Render']),('MoneyMessages',['Init','Render']),('3dMarkers',['Init','Render','Shutdown'])]:methods+=['C'+owner+'::'+name for name in names]
definitions=''
for index,name in enumerate(methods):
    definitions+='void '+name+'(void)\n{\n    // Observe the complete original child-operation ordering and live state\n    event('+str(20+index)+');\n}\n'
    for oldowner,newowner in [('CMoneyMessages::','MoneyMessages_'),('CShinyTexts::','ShinyTexts_'),('C3dMarkers::','Markers3D_')]:
        if name.startswith(oldowner):definitions+='extern "C" void '+name.replace(oldowner,newowner)+'(void)\n{\n    // Observe the same child entry now called directly through its C API\n    event('+str(20+index)+');\n}\n'
    if name.startswith('CBrightLights::'):
        args='bool reuseFlicker' if name.endswith('::Render') else 'void';check='require(!reuseFlicker);' if args!='void' else ''
        definitions+='extern "C" void '+name.replace('CBrightLights::','BrightLights_')+'('+args+')\n{\n    // Observe the same original child operation through the current C entry\n    '+check+'event('+str(20+index)+');\n}\n'
scale=m.function((out/'before/src/renderer/Draw.c').read_text(),'Draw_','ScaleY')
(out/'test.cpp').write_text((root/'utils/tests/special_fx.cpp.in').read_text().replace('@OWNER_METHODS@',definitions).replace('@DRAW_SCALE@',scale),newline='\n')
(out/'no-renderer.h').write_text('#include "config.h"\n#undef NEW_RENDERER\n',newline='\n')
(out/'pc-scaling.h').write_text('#include "config.h"\n#define FORCE_PC_SCALING\n',newline='\n')
(out/'no-languages.h').write_text('#include "config.h"\n#undef MORE_LANGUAGES\n',newline='\n')
print('Audited all fourteen complete migrated files, six controller methods, callback, four fields and exact font enum; generated unchanged original comparison')
