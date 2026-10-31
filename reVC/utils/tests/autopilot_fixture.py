"""Prepare whole original/C autopilot comparisons with actual curve arithmetic and save-buffer primitives."""
from pathlib import Path
import re,json,sys
import autopilot_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/autopilot-c-tests';saved=out/'before';preview=out/'preview'
old=(saved/'src/control/AutoPilot.cpp').read_text();header=(saved/'src/control/AutoPilot.h').read_text()
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    for name in ['AutoPilotSystem.c','AutoPilot_backend.h']:assert (root/'src/control'/name).read_text()==(preview/name).read_text(),name
for name,path in [('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('maths.h','src/math/maths.h'),('Curves.h','src/control/Curves.h'),('Curves.c','src/control/Curves.c'),('SaveBuf.h','src/save/SaveBuf.h')]:(out/name).write_text((saved/path).read_text(),newline='\n')
assert re.search(r'CVector\(void\)\s*\{\s*\}',(saved/'src/math/Vector.h').read_text())
assert re.search(r'CVuVector\(void\)\s*\{\s*\}',(saved/'src/math/VuVector.h').read_text())
vehicle=(saved/'src/vehicles/Vehicle.h').read_text();laterMembers=vehicle[vehicle.index('CAutoPilot AutoPilot;'):vehicle.index('CVehicle(void)')]
laterValueTypes=re.findall(r'\b(C\w+)\s+(\w+)\s*(?:\[[^\]]+\])?\s*;',laterMembers)
assert laterValueTypes==[('CAutoPilot','AutoPilot'),('CStoredCollPoly','m_aCollPolys')],laterValueTypes
poly=(saved/'src/collision/Collision.h').read_text();poly=poly[poly.index('struct CStoredCollPoly'):poly.index('\n};',poly.index('struct CStoredCollPoly'))]
assert 'CStoredCollPoly(' not in poly
config=(saved/'src/core/config.h').read_text();capacity=int(re.search(r'NUM_PATH_NODES_IN_AUTOPILOT\s*=\s*(\d+)',config)[1]);assert capacity==8
(out/'config.h').write_text('#pragma once\n#define NUM_PATH_NODES_IN_AUTOPILOT '+str(capacity)+'\n',newline='\n')
(out/'original-AutoPilot.h').write_text(header,newline='\n')
(out/'Timer.h').write_text('''#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
const uint32_t *Timer_GetTimeInMilliseconds(void);
#ifdef __cplusplus
}
#endif
''',newline='\n')
pathHeader=(saved/'src/control/PathFind.h').read_text();link=pathHeader[pathHeader.index('struct CCarPathLink'):pathHeader.index('// This is what',pathHeader.index('struct CCarPathLink'))]
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory>
#include <new>
typedef int8_t int8;typedef int16_t int16;typedef int32_t int32;typedef uint8_t uint8;typedef uint32_t uint32;
#define nil NULL
#define VALIDATE_SIZE(type,size)
#define Max(a,b) ((a)>(b)?(a):(b))
#define BIT(bit) (1u<<(bit))
#define LANE_WIDTH 5.0f
#include "config.h"
#include "maths.h"
struct RwV3d {float x,y,z;};
#include "Vector.h"
#include "Vector2D.h"
#include "Timer.h"
struct CPathNode {int identity;};class CVehicle {public:int identity;};
@LINK@
struct TestPaths {CCarPathLink m_carPathLinks[64];};extern TestPaths ThePaths;
#ifdef TEST_AUTOPILOT_ORIGINAL
#include "original-AutoPilot.h"
#else
#include "@PREVIEW@/AutoPilot.h"
#endif
#include "Curves.h"
#include "SaveBuf.h"
#include "entry.h"
'''.replace('@LINK@',link).replace('@PREVIEW@',preview.as_posix())
(out/'owners.h').write_text(owners,newline='\n');(out/'common.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
api='''#pragma once
#include <stdint.h>
#include "config.h"
#ifdef __cplusplus
extern "C" {
#else
typedef struct CAutoPilot CAutoPilot;
#endif
'''
signatures=[('void','Init','CAutoPilot *self',['self']),('void','ModifySpeed','CAutoPilot *self,float speed',['self','speed']),('void','RemoveOnePathNode','CAutoPilot *self',['self']),('float','GetCruiseSpeed','CAutoPilot *self',['self']),('void','Copy','CAutoPilot *self,const CAutoPilot *source',['self','source']),('void','Save','CAutoPilot *self,uint8_t **buf',['self','buf']),('void','Load','CAutoPilot *self,uint8_t **buf',['self','buf'])]
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef TEST_AUTOPILOT_ORIGINAL\n#include "'+(preview/'AutoPilot.h').as_posix()+'"\n#endif\n'
for ret,name,args,values in signatures:
    guard='#ifdef COMPATIBLE_SAVES\n' if name in ['Save','Load'] else '';tail='#endif\n' if guard else ''
    api+=guard+ret+' Before_AutoPilot_'+name+'('+args+');\n'+ret+' Run_AutoPilot_'+name+'('+args+');\n'+tail
    native='std::allocator<CAutoPilot>().construct(self);' if name=='Init' else '*self=*source;' if name=='Copy' else ('return ' if ret!='void' else '')+'self->'+name+'('+('*buf' if name in ['Save','Load'] else ','.join(values[1:]))+');'
    plain='*self=*source;' if name=='Copy' else ('return ' if ret!='void' else '')+'AutoPilot_'+name+'('+','.join(values)+');'
    original+=guard+'extern "C" '+ret+' Before_AutoPilot_'+name+'('+args+')\n{\n    // Invoke the entire original member operation or trivial copy on caller-owned record storage\n    '+native+'\n}\n'+tail
    caller+=guard+ret+' Run_AutoPilot_'+name+'('+args+')\n{\n    // Exercise the complete operation or plain record copy through a standalone strict C11 caller\n#ifdef TEST_AUTOPILOT_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_AutoPilot_'+name+'('+','.join(values)+');\n#else\n    '+plain+'\n#endif\n}\n'+tail
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(preview/'AutoPilot_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.host(),flags=re.M),newline='\n');(out/'kernel.c').write_text((preview/'AutoPilotSystem.c').read_text(),newline='\n')
observe='''static void Fields(const CAutoPilot *self)
{
    // Observe all declared fields while retaining raw float and scalar representations
'''
for kind,name,suffix in m.fields(header):
    if name=='m_vecDestinationCoors':observe+='    Floating(self->'+name+'.x);Floating(self->'+name+'.y);Floating(self->'+name+'.z);\n'
    elif name=='m_aPathFindNodesInfo':observe+='    for(int i=0;i<NUM_PATH_NODES_IN_AUTOPILOT;i++)Number(self->'+name+'[i]?1+(unsigned)(self->'+name+'[i]-nodes):0);\n'
    elif name=='m_pTargetCar':observe+='    Number(self->'+name+'?1+(unsigned)(self->'+name+'-cars):0);\n'
    elif kind=='float':observe+='    Floating(self->'+name+');\n'
    else:observe+='    Number((uint32_t)self->'+name+');\n'
observe+='}\n'
(out/'observations.h').write_text(observe,newline='\n')
expected='''static void ExpectedSave(const CAutoPilot *self,uint8_t *target)
{
    // Construct the independent packed payload from the immutable original serialized field sequence
    size_t offset=0;uint8_t flags=(uint8_t)(self->m_bSlowedDownBecauseOfCars|(self->m_bSlowedDownBecauseOfPeds<<1)|(self->m_bStayInCurrentLevel<<2)|(self->m_bStayInFastLane<<3)|(self->m_bIgnorePathfinding<<4));
'''
for line in m.operation(old,'Save').splitlines():
    field=re.search(r'WriteSaveBuf\(&buf, &(\w+(?:\.[xyz])?), sizeof\(([^)]+)\)\)',line)
    zero=re.search(r'ZeroSaveBuf\(&buf, (\d+)\)',line)
    if field:
        expr='flags' if field[1]=='flags' else 'self->'+field[1];expected+='    memcpy(target+offset,&'+expr+',sizeof('+expr+'));offset+=sizeof('+expr+');\n'
    elif zero:expected+='    memset(target+offset,0,'+zero[1]+');offset+='+zero[1]+';\n'
expected+='    Check(offset==120);\n}\n';(out/'expected-save.h').write_text(expected,newline='\n')
(out/'fixture.cpp').write_text((root/'utils/tests/autopilot.cpp.in').read_text(),newline='\n')
print('Prepared complete original/C autopilot policies, 33-field observations, actual curve/save primitives and independent 120-byte save expectations')
