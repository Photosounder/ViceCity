"""Prepare the exact original producer, independent C callers and complete table comparisons."""
from pathlib import Path
import json,re,sys
import ped_routes_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/ped-routes-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
assert re.search(r'CVector\(void\)\s*\{\s*\}',(saved/'src/math/Vector.h').read_text())
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['PedRoutes.h','PedRoutes.c']:(out/name).write_text((tree/'src/peds'/name).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\nenum { NUMPEDROUTES = 200 };\n',newline='\n')
(out/'original-PedRoutes.h').write_text((saved/'src/peds/PedRoutes.h').read_text(),newline='\n')
(out/'common.h').write_text('''#pragma once
#include <stdint.h>
#include "config.h"
typedef int16_t int16;typedef uint16_t uint16;
struct RwV3d { float x,y,z; };
class CVector : public RwV3d {
public:
    CVector(void) {}
    CVector(float x,float y,float z) { this->x=x;this->y=y;this->z=z; }
};
''',newline='\n')
(out/'owners.h').write_text('''#pragma once
#ifdef ORIGINAL
#include "common.h"
#include "original-PedRoutes.h"
extern CRouteNode gaRoutes[NUMPEDROUTES];
#else
#include "PedRoutes.h"
#endif
''',newline='\n')
entry='''#pragma once
#include <stdint.h>
typedef struct TestVector { float x,y,z; } TestVector;
#ifdef __cplusplus
extern "C" {
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/peds/PedRoutes.cpp').read_text(),flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "PedRoutes.h"\n#endif\n'
signatures=[('Initialise','void','void',''),('GetRouteThisPointIsOn','int16_t','int16_t point','point'),('GetRouteStart','int16_t','int16_t route','route'),('GetPointPosition','TestVector','int16_t point','point'),('AddRoutePoint','void','int16_t route,TestVector pos','route,pos'),('RemoveRoute','void','int16_t route','route')]
for name,result,args,values in signatures:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    old='CRouteNode::'+name+'('+values+');'
    if name=='AddRoutePoint':old='CRouteNode::AddRoutePoint(route,CVector(pos.x,pos.y,pos.z));'
    if name=='GetPointPosition':old='const CVector value=CRouteNode::GetPointPosition(point);\n    return TestVector{value.x,value.y,value.z};'
    elif result!='void':old='return '+old
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original operation with its native vector types\n    '+old+'\n}\n'
    new='PedRoutes_'+name+'('+values+');'
    if name=='AddRoutePoint':new='const PedRouteVector value={pos.x,pos.y,pos.z};\n    PedRoutes_AddRoutePoint(route,value);'
    if name=='GetPointPosition':new='const PedRouteVector value=PedRoutes_GetPointPosition(point);\n    const TestVector result={value.x,value.y,value.z};\n    return result;'
    elif result!='void':new='return '+new
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise each public operation through an independent strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+new+'\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('fixture.cpp',(root/'utils/tests/ped_routes.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all six original/C route operations, table and by-value vector calls')
