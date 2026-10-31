"""Prepare exact original methods, literal native leaves, save copies and independent C callers."""
from pathlib import Path
import re,json,sys
import stored_car_migration as m
from doors_migration import method
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/stored-car-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay';oldHeader=(saved/'src/control/Garages.h').read_text()
assert re.search(r'CVector\(void\)\s*\{\s*\}',(saved/'src/math/Vector.h').read_text())
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/name).read_text()==m.calls((saved/name).read_text()),name
for name in ['StoredCar.h','StoredCar.c','StoredCar_backend.h']:(out/name).write_text((tree/'src/control'/name).read_text(),newline='\n')
for directory,name in [('entities','EntityStatus.h'),('vehicles','VehicleBombTypes.h'),('core','StreamingInfo.h')]:(out/name).write_text((tree/'src'/directory/name).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
(out/'host.h').write_text((root/'utils/tests/stored_car_host.h.in').read_text(),newline='\n')
(out/'support.cpp').write_text((root/'utils/tests/stored_car_support.cpp.in').read_text(),newline='\n')
garage=oldHeader[oldHeader.index('class CGarage\n'):oldHeader.index('\n\tvoid OpenThisGarage();')]+'\n};\n'
(out/'owners.h').write_text('#pragma once\n#include "host.h"\n#ifdef ORIGINAL\n'+m.record(oldHeader)+'\n#else\n#include "StoredCar.h"\n#endif\n'+garage,newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+'\n'.join(method((saved/'src/control/Garages.cpp').read_text(),'CStoredCar::'+name) for name in ['StoreCar','RestoreCar','operator='])+'\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "StoredCar.h"\n#endif\n'
signatures=[('Init','void','void *self','self'),('Clear','void','void *self','self'),('HasCar','bool','void *self','self'),('Assign','const void *','void *self,const void *other','self,other'),('StoreCar','void','void *self,void *vehicle','self,vehicle'),('RestoreCar','void *','void *self','self')]
for name,result,args,values in signatures:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    old='((CStoredCar*)self)->'+name+'('+('*(const CStoredCar*)other' if name=='Assign' else '(CVehicle*)vehicle' if name=='StoreCar' else '')+');'
    if name=='Assign':old='return &(((CStoredCar*)self)->operator=(*(const CStoredCar*)other));'
    elif result!='void':old='return '+old
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original stored-record operation\n    '+old+'\n}\n'
    new='StoredCar_'+name+'((CStoredCar*)self'+(', (const CStoredCar*)other' if name=='Assign' else ', (CVehicle*)vehicle' if name=='StoreCar' else '')+');'
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise the public C operation through an independent strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+('return ' if result!='void' else '')+new+'\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
copies='#include "owners.h"\n'+m.garage_copy(m.garage_fields(oldHeader))+'\nextern "C" void Run_GarageAssign(void *self,const void *other)\n{\n    // Exercise the enclosing garage assignment at the original save boundary\n#ifdef ORIGINAL\n    *(CGarage*)self=*(const CGarage*)other;\n#else\n    Garage_AssignForSave((CGarage*)self,(const CGarage*)other);\n#endif\n}\n'
copies=copies.replace(m.garage_copy(m.garage_fields(oldHeader)),'#ifndef ORIGINAL\n'+m.garage_copy(m.garage_fields(oldHeader))+'#endif\n')
garage_observe='static void observeGarage(const CGarage *value)\n{\n    // Compare every enclosing field while excluding unspecified outer padding\n'
for field in m.garage_fields(oldHeader):
    garage_observe+=('    hash32(value->'+field+'?1:0);' if field in ['m_pDoor1','m_pDoor2','m_pTarget'] else '    bytes(&value->'+field+',sizeof(value->'+field+'));')+'\n'
garage_observe+='    ++states;\n}\n'
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('native.cpp','#include "host.h"\n#include "StoredCar_backend.h"\n'+m.native()),('copies.cpp',copies),('fixture.cpp',(root/'utils/tests/stored_car.cpp.in').read_text().replace('@GARAGE_OBSERVE@',garage_observe))]:(out/name).write_text(value,newline='\n')
print('Prepared six original/C operations, literal native leaves and explicit enclosing save assignment')
