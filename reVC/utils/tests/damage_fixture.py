"""Prepare complete damage policies, exact old bit helpers and real constructor/save integration."""
from pathlib import Path
import re,json,sys
import damage_migration as m
from doors_migration import method,CAR_INIT
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/damage-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
assert re.search(r'CVector\(void\)\s*\{\s*\}',(saved/'src/math/Vector.h').read_text())
assert 'CColPoint(' not in (saved/'src/collision/ColPoint.h').read_text()
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
    assert not (root/'src/vehicles/DamageManager.cpp').exists()
for name in ['DamageManager.h','DamageManager.c','VehicleDamageParts.h','Door.h','DoorTypes.h','WheelStatus.h']:(out/name).write_text((tree/'src/vehicles'/name).read_text(),newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n');(out/'original-DamageManager.h').write_text((saved/'src/vehicles/DamageManager.h').read_text(),newline='\n')
common=(saved/'src/core/common.h').read_text();helper=common[common.index('#define MASK(p, s)'):common.index('\n#include "skeleton.h"')]
(out/'common.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <new>
typedef uint8_t uint8;typedef uint32_t uint32;typedef int32_t int32;
#define VALIDATE_SIZE(type,size)
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
'''+helper+'\n',newline='\n')
owners='''#pragma once
#include "common.h"
#include "VehicleDamageParts.h"
#include "Door.h"
#ifdef ORIGINAL
#include "original-DamageManager.h"
extern float G_aComponentDamage[7];
#else
#include "DamageManager.h"
#endif
''';(out/'owners.h').write_text(owners,newline='\n')
ops=[('Init','void','void *self',['self'])]
signatures={'ResetDamageStatus':('void',''), 'FuckCarCompletely':('void',''),'ApplyDamage':('bool','int32_t component,float damage,float unused'),'GetComponentGroup':('bool','int32_t component,void *group,uint8_t *subComp'),'SetDoorStatus':('void','int32_t door,uint32_t status'),'GetDoorStatus':('int32_t','int32_t door'),'ProgressDoorDamage':('bool','uint8_t door'),'SetPanelStatus':('void','int32_t panel,uint32_t status'),'GetPanelStatus':('int32_t','int32_t panel'),'ProgressPanelDamage':('bool','uint8_t panel'),'SetLightStatus':('void','int32_t light,uint32_t status'),'GetLightStatus':('int32_t','int32_t light'),'SetWheelStatus':('void','int32_t wheel,uint32_t status'),'GetWheelStatus':('int32_t','int32_t wheel'),'ProgressWheelDamage':('bool','uint8_t wheel'),'SetEngineStatus':('void','uint32_t status'),'GetEngineStatus':('int32_t',''),'ProgressEngineDamage':('bool','')}
for name in m.OPERATIONS:
    result,args=signatures[name];values=['self']+re.findall(r'(\w+)(?:,|$)',args);ops.append((name,result,'void *self'+(','+args if args else ''),values))
ops.append(('GetPanelStatusFromWord','int32_t','uint32_t panelstatus,int32_t panel',['panelstatus','panel']))
entry='#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/vehicles/DamageManager.cpp').read_text(),flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "DamageManager.h"\n#endif\n'
for name,result,args,values in ops:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    nativeValues=[('(tComponent)'+value if value=='component' else '(eLights)'+value if value=='light' else '(tComponentGroup*)group' if value=='group' else value) for value in values[1:]]
    native='new(self) CDamageManager();' if name=='Init' else 'CDamageManager::GetPanelStatus(panelstatus,panel);' if name=='GetPanelStatusFromWord' else '((CDamageManager*)self)->'+name+'('+','.join(nativeValues)+');'
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the entire original operation with its original native parameter types\n    '+('return ' if result!='void' else '')+native+'\n}\n'
    plain=[('(CDamageManager*)self' if value=='self' else '(tComponentGroup*)group' if value=='group' else value) for value in values]
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise the public C operation through a separate strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if result!='void' else '')+'Damage_'+name+'('+','.join(plain)+');\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
support='#include "Door.h"\n#include <string.h>\n'+method((saved/'src/vehicles/DoorSystem.c').read_text(),'Door_Init')+'\n'
fixture=(root/'utils/tests/damage.cpp.in').read_text().replace('@DAMAGE_INIT@','#ifndef ORIGINAL\n'+m.INIT+'\n#endif\n').replace('@DOOR_INIT@',CAR_INIT)
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('support.c',support),('fixture.cpp',fixture)]:(out/name).write_text(value,newline='\n')
print('Prepared all 20 original/C damage operations, packed-word helpers, mutable coefficients and constructor/save integration')
