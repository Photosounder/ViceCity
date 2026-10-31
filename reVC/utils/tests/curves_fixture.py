"""Audit the entire curve conversion and generate untouched C++ versus C11 models."""
from pathlib import Path
import json,re
import curves_migration as m

root=Path(__file__).resolve().parents[2];out=root/'build/curves-c-tests'

def normalize(source):
    # Compare all statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/control/Curves.cpp').read_text()
for path in json.loads((out/'changes.json').read_text()):
    original=(out/'before'/path).read_text()
    if path.endswith('/Curves.cpp'):
        target=root/'src/control/Curves.c';expected=m.kernel(original)
        assert not (root/path).exists()
    else:target=root/path;expected=m.project(original) if path.startswith('build/') else m.calls(original)
    assert normalize(expected)==normalize(target.read_text()),path
    assert b'\r' not in target.read_bytes(),path
assert 'class CCurves' not in (root/'src/control/Curves.h').read_text()
assert len(re.findall(r'CCurves::CalcSpeedScaleFactor\(',old))==1
assert len(re.findall(r'CCurves::CalcCurvePoint\(',old))==1
consumers=''.join((out/'before'/path).read_text() for path in ['src/control/AutoPilot.cpp','src/control/CarCtrl.cpp'])
assert consumers.count('CCurves::CalcSpeedScaleFactor(')==6 and consumers.count('CCurves::CalcCurvePoint(')==2
for path in ['src/math/Vector.h','src/math/Vector2D.h','src/math/maths.h','vendor/librw/src/rwbase.h']:
    # Keep the real original vector operators and float wrappers as immutable baseline dependencies
    saved=out/'before'/path
    if not saved.exists():saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes())
vector=(out/'before/src/math/Vector.h').read_text();vector2=(out/'before/src/math/Vector2D.h').read_text();maths=(out/'before/src/math/maths.h').read_text()
rw=(out/'before/vendor/librw/src/rwbase.h').read_text();a=rw.index('struct V3d\n{');b=rw.index('\n};',a)+3
rw='namespace rw { typedef float float32; typedef int32_t int32; struct Matrix;\n'+rw[a:b]+'\n}\ntypedef rw::V3d RwV3d;\n'
(out/'Vector.h').write_text(vector,newline='\n')
(out/'Vector2D.h').write_text(vector2,newline='\n')
(out/'maths.h').write_text(maths,newline='\n')
(out/'rw-vector.h').write_text(rw,newline='\n')
signatures='''#pragma once
#include "Curves.h"
#ifdef __cplusplus
extern "C" {
#endif
float TestCurves_CSpeed(CurvesVector a,CurvesVector b,float ax,float ay,float bx,float by);
void TestCurves_CPoint(CurvesVector a,CurvesVector b,CurvesVector da,CurvesVector db,float between,int32_t time,CurvesVector *position,CurvesVector *direction);
#ifdef __cplusplus
}
#endif
'''
(out/'caller.h').write_text(signatures,newline='\n')
(out/'caller.c').write_text('''#include "caller.h"
float TestCurves_CSpeed(CurvesVector a,CurvesVector b,float ax,float ay,float bx,float by)
{
    // Exercise the public speed API through a separately compiled C11 caller
    return Curves_CalcSpeedScaleFactor(a,b,ax,ay,bx,by);
}
void TestCurves_CPoint(CurvesVector a,CurvesVector b,CurvesVector da,CurvesVector db,float between,int32_t time,CurvesVector *position,CurvesVector *direction)
{
    // Exercise the public point API and its output aliases through C11
    Curves_CalcCurvePoint(a,b,da,db,between,time,position,direction);
}
''',newline='\n')
bridge='''static float TestCurves_Speed(CVector *a,CVector *b,float ax,float ay,float bx,float by)
{
    // Exercise the same complete value-conversion pattern as the production callers
    return CCurves::CalcSpeedScaleFactor(a,b,ax,ay,bx,by);
}
static void TestCurves_Point(CVector *a,CVector *b,CVector *da,CVector *db,float between,int32_t time,CVector *position,CVector *direction)
{
    // Exercise the production output-copy pattern including aliased C++ vectors
    CCurves::CalcCurvePoint(a,b,da,db,between,time,position,direction);
}
'''
template=(root/'utils/tests/curves.cpp.in').read_text()
for variant in ['before','after']:
    if variant=='before':
        owner=(out/'before/src/control/Curves.h').read_text()+'\n'+old.replace('#include "common.h"','').replace('#include "Curves.h"','')
        adapter=bridge
    else:
        owner='#include "caller.h"\n#define Curves_CalcSpeedScaleFactor TestCurves_CSpeed\n#define Curves_CalcCurvePoint TestCurves_CPoint\n'
        adapter=m.calls(bridge)+'\n#undef Curves_CalcSpeedScaleFactor\n#undef Curves_CalcCurvePoint\n'
    (out/(variant+'.cpp')).write_text(template.replace('@OWNER@',owner).replace('@BRIDGE@',adapter),newline='\n')
for shape in ['default','vanilla']:
    shape_header='#pragma once\n#include "config.h"\n'+('#undef FIX_BUGS\n' if shape=='vanilla' else '')
    folder=out/shape;folder.mkdir(exist_ok=True);(folder/'shape.h').write_text(shape_header,newline='\n')
print('Both complete curve algorithms, exact vector/math dependencies, all eight calls and six full source/project transformations audited')
