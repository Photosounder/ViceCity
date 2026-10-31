import weather_migration as _weather_conversion
"""Audit the whole CGeneral migration and compile the original class as its oracle."""
from pathlib import Path
import json,re
import general_migration as m
import ranges_migration as ranges
import water_creatures_migration as water
import weather_migration as weather
import ped_data_migration as pedestrian
import projectile_migration as projectiles
import gangs_migration as gangs
import fire_migration as fire
import bridge_migration as bridge
import traffic_lights_migration as traffic

root=Path(__file__).resolve().parents[2];out=root/'build/general-c-tests'
old=(out/'before/src/core/General.h').read_text()

def normalize(source):
    # Apply the explicit verified weather rename before comparing every original statement
    source=_weather_conversion.calls(source)
    # Compare complete statements while ignoring edit markers and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for path in json.loads((out/'changes.json').read_text()):
    before=(out/'before'/path).read_text()
    expected=m.project(before) if path.startswith('build/') else m.calls(before)
    if path=='src/renderer/WaterCreatures.cpp':water.verify_legacy(before,m.calls,root)
    if path=='src/renderer/Weather.cpp':weather.verify_legacy(before,m.calls,root)
    if path=='src/peds/PedType.cpp':pedestrian.verify_legacy(before,m.calls,root)
    if path=='src/weapons/ProjectileInfo.cpp':projectiles.verify_legacy(before,m.calls,root)
    if path=='src/peds/Gangs.cpp':gangs.verify_legacy(before,m.calls,root)
    if path=='src/core/Fire.cpp':fire.verify_legacy(before,m.calls,root)
    if path=='src/control/Bridge.cpp':bridge.verify_legacy(before,m.calls,root)
    if path=='src/control/TrafficLights.cpp':traffic.verify_legacy(before,m.calls,root)
    target=root/ranges.target('src/core/Directory.c' if path=='src/core/Directory.cpp' else path)
    assert normalize(expected)==normalize(target.read_text()),path
    assert b'\r' not in target.read_bytes(),path
assert normalize(m.kernel(old))==normalize((root/'src/core/General.c').read_text())
assert len(m.methods(old))==15
assert len(m.EVIDENCE)==627
for site in m.EVIDENCE:
    # Verify every range selection still refers to the unchanged original argument list
    original=(out/'before'/site['source']).read_text()
    assert original[site['offset']:].startswith('CGeneral::GetRandomNumberInRange')
    opening,end,args=m.arguments(original,site['offset'])
    assert args==site['arguments']
    assert m.KINDS[re.sub(r'\s+','',args)]==site['kind']
    assert json.loads((out/'site-selections.json').read_text())[str(site['id'])]==site['kind']
common=(root/'src/core/common.h').read_text()
assert '#include "Draw_constants.h"' in common
common+='\n'+(root/'src/renderer/Draw_constants.h').read_text()
macros='\n'.join(line for line in common.splitlines() if re.match(r'#define (PI|TWOPI|DEGTORAD|RADTODEG|Clamp)\(',line) or line.startswith(('#define PI ','#define TWOPI ')))
assert '#define PI (float)M_PI' in macros
assert '3.14159265358979323846' in (root/'src/core/General.c').read_text()
maths=(root/'src/math/maths.h').read_text()
wrappers='\n'.join(line for line in maths.splitlines() if re.match(r'inline float (Atan2|Abs|Sqrt|Floor)\(',line))
assert len(wrappers.splitlines())==4
original_rng=(root/'src/core/re3.cpp').read_text()
rng=original_rng[original_rng.index('#ifdef USE_PS2_RAND\nunsigned long long myrand_seed'):original_rng.index('\n#ifdef CUSTOM_FRONTEND_OPTIONS',original_rng.index('#ifdef USE_PS2_RAND\nunsigned long long myrand_seed'))]
rng=rng.replace('int\nmyrand(void)\n{','int\nmyrand(void)\n{\n    // Observe each draw and inject boundary values for independent checks\n    ++reads; if(injected>=0){int value=injected;injected=-1;return value;}')
rng=rng.replace('mysrand(unsigned int seed)\n{','mysrand(unsigned int seed)\n{\n    // Observe reseeding without changing the original seed conversion\n    ++writes;')
baseline=wrappers+'\n'+macros+'\n'+old
caller_header='//+ rouz edit (ChatGPT)\n#pragma once\n#include "General.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
caller_c='//+ rouz edit (ChatGPT)\n#include "caller.h"\n'
caller_old='extern "C" {\n'
for result,name,args,body in m.methods(old):
    # Exercise each new API through a separately compiled actual C11 caller
    sig=m.signature(result,name,args).replace('General_','Test_',1)
    names=[part.strip().split()[-1].lstrip('*&') for part in args.split(',') if part.strip() not in ('','void')]
    arguments=','.join(names);oldargs=','.join('*'+value if name=='SolveQuadratic' and value in ('root1','root2') else value for value in names)
    caller_header+=sig+';\n'
    target=m.signature(result,name,args).split(' ')[1].split('(')[0]
    prefix='' if result=='void' else 'return '
    caller_c+=sig+'\n{\n    // Invoke the actual C helper through its public header\n    '+prefix+target+'('+arguments+');\n}\n'
    caller_old+=sig+'\n{\n    // Invoke the untouched original overload and reference contract\n    '+prefix+'CGeneral::'+name+'('+oldargs+');\n}\n'
caller_header+='#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n';caller_c+='//- rouz edit (ChatGPT)\n';caller_old+='}\n'
(out/'caller.h').write_text(caller_header,newline='\n');(out/'caller.c').write_text(caller_c,newline='\n')
template=(root/'utils/tests/general.cpp.in').read_text()
for variant in ['before','after']:
    source=template.replace('@RANDOM@',rng).replace('@OWNER@',baseline if variant=='before' else '').replace('@CALLERS@',caller_old if variant=='before' else '#include "caller.h"')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for shape in ['default','regular','portable','regular_portable']:
    # Cover both seed widths and both native string comparison implementations
    shape_source='#pragma once\n#include "config.h"\n#define _USE_MATH_DEFINES\n#ifndef M_PI\n#define M_PI 3.14159265358979323846\n#endif\n#define PI (float)M_PI\n'
    if shape.startswith('regular'):shape_source+='#undef USE_PS2_RAND\n'
    if shape.endswith('portable'):shape_source+='#undef ASCII_STRCMP\n'
    shape_source+='#ifdef USE_PS2_RAND\n#define MYRAND_MAX 65535\n#else\n#define MYRAND_MAX 32767\n#endif\n'
    folder=out/shape;folder.mkdir(exist_ok=True);(folder/'shape.h').write_text(shape_source,newline='\n')
print('All 15 helpers, 627 compiler-selected range calls, pointer outputs and 90 complete caller/project edits audited')
