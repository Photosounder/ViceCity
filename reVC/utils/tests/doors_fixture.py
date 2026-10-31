"""Prepare complete original/C door policies with literal vector math and explicit owner lifetimes."""
from pathlib import Path
import re,json,sys
import doors_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/doors-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/name).read_text()==m.calls((saved/name).read_text()),name
    assert (root/'src/vehicles/DoorSystem.c').read_text()==m.kernel((saved/'src/vehicles/Door.cpp').read_text(),(saved/'src/vehicles/Door.h').read_text())
for name in ['Door.h','DoorSystem.c','Door_backend.h','DoorTypes.h']:(out/name).write_text((tree/'src/vehicles'/name).read_text(),newline='\n')
for name,path in [('Vector.h','src/math/Vector.h'),('Vector2D.h','src/math/Vector2D.h'),('maths.h','src/math/maths.h'),('ColPoint.h','src/collision/ColPoint.h')]:(out/name).write_text((saved/path).read_text(),newline='\n')
assert re.search(r'CVector\(void\)\s*\{\s*\}',(saved/'src/math/Vector.h').read_text())
assert re.search(r'CVector2D\(void\)\s*\{\s*\}',(saved/'src/math/Vector2D.h').read_text())
assert 'CColPoint(' not in (saved/'src/collision/ColPoint.h').read_text()
(out/'original-Door.h').write_text((saved/'src/vehicles/Door.h').read_text(),newline='\n')
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>
typedef int8_t int8;typedef uint8_t uint8;
struct RwV3d {float x,y,z;};
#include "maths.h"
#include "Vector.h"
#include "Vector2D.h"
#include "ColPoint.h"
#ifdef ORIGINAL
#include "original-Door.h"
#else
#include "Door.h"
#endif
class CMatrix {public:float rx,ry,rz,fx,fy,fz,ux,uy,uz;};
class CVehicle {public:
CVector m_vecMoveSpeed,m_vecMoveFriction,m_vecTurnFriction,m_vecTurnSpeed;
CMatrix matrix;
CVector GetSpeed(const CVector &r);
CMatrix &GetMatrix(){return matrix;}
};
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
''';(out/'owners.h').write_text(owners,newline='\n')
(out/'common.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n');(out/'Vehicle.h').write_text('#pragma once\n#include "owners.h"\n',newline='\n')
geometry='#include "owners.h"\n'+m.method((saved/'src/entities/Physical.cpp').read_text(),'CPhysical::GetSpeed').replace('CPhysical::','CVehicle::')+'\n'
vector=(saved/'src/math/Vector.cpp').read_text()
for signature in ['CrossProduct','Multiply3x3(const CVector &vec, const CMatrix &mat)']:
    marker='CrossProduct' if signature=='CrossProduct' else 'Multiply3x3'
    subset=vector if marker=='CrossProduct' else vector[vector.index('CVector\n'+signature):]
    geometry+=m.method(subset,marker)+'\n'
geometry+='\n#ifndef ORIGINAL\n'+m.HOST+'\n#endif\n'
ops=[]
for owner,prefix,names in [('CDoor','Door',m.CAR),('CTrainDoor','TrainDoor',m.TRAIN)]:
    ops.append((prefix+'_Init','void','void *self',owner,['self'],None))
    params='float low,float high,int8_t dir,int8_t axis'
    ops.append((prefix+'_SetLimits','void','void *self,'+params,owner,['self','low','high','dir','axis'],'Init'))
    for name in names:
        result='void' if name in ['Open','Process'] else 'bool' if name in ['IsFullyOpen','IsClosed'] else 'float'
        args='void *self'+(',float ratio' if name=='Open' else ',void *vehicle' if name=='Process' else '')
        values=['self']+(['ratio'] if name=='Open' else ['vehicle'] if name=='Process' else [])
        ops.append((prefix+'_'+name,result,args,owner,values,name))
entry='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/vehicles/Door.cpp').read_text(),flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "Door.h"\n#endif\n'
for name,result,args,owner,values,oldName in ops:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    native=('new(self) '+owner+'();') if oldName is None else '(('+owner+'*)self)->'+oldName+'('+','.join('(CVehicle*)'+value if value=='vehicle' else value for value in values[1:])+');'
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original operation with its native record and parameter types\n    '+('return ' if result!='void' else '')+native+'\n}\n'
    plain=['('+owner+'*)self']+['(CVehicle*)vehicle' if value=='vehicle' else value for value in values[1:]]
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise the public operation through an independent strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if result!='void' else '')+name+'('+','.join(plain)+');\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
fixture=(root/'utils/tests/doors.cpp.in').read_text()
for token,block in [('CAR',m.CAR_INIT),('BOAT',m.BOAT_INIT),('TRAIN',m.TRAIN_INIT)]:fixture=fixture.replace('@'+token+'_INIT@','#ifndef ORIGINAL\n'+block+'\n#endif\n')
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('geometry.cpp',geometry),('fixture.cpp',fixture)]:(out/name).write_text(value,newline='\n')
print('Prepared all 16 original/C door operations, literal physical/vector math and all three owner initialization sites')
