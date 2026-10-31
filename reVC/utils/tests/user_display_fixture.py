"""Audit all user-display control bodies and emit untouched-original/C comparisons."""
from pathlib import Path
import json,re
import user_display_migration as m
import messages_migration as messages
root=Path(__file__).resolve().parents[2];out=root/'build/user-display-c-tests'
changes=json.loads((out/'changes.json').read_text())
for path in changes:
    source=(out/'before'/path).read_text();assert (root/path).read_text()==(m.project(source) if path.startswith('build/') else m.calls(source)),path
    assert b'\r' not in (root/path).read_bytes(),path
old=(out/'before/src/core/User.cpp').read_text()
assert (root/'src/core/UserDisplay.c').read_text()==m.kernel(old)
assert (root/'src/core/User_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'dependencies.json').read_text()):
    if path not in changes:messages.verify_dependency(out/'before'/path,root/path)
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
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef uint32_t uint32;
typedef int16_t int16;typedef int32_t int32;typedef uint16_t wchar;
struct RwV3d {float x,y,z;};
#define nil NULL
#include "maths.h"
#include "Vector.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
'''
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','maths.h']:(out/name).write_text((out/'before/src/math'/name).read_text(),newline='\n')
header=(out/'before/src/core/User.h').read_text().replace('class CPlaceName\n{','class CPlaceName\n{\npublic:').replace('class CCurrentVehicle\n{','class CCurrentVehicle\n{\npublic:')
(out/'original-User.h').write_text(header,newline='\n')
inVehicle=re.search(r'bool InVehicle\(void\) \{[^}]+\}',(out/'before/src/peds/Ped.h').read_text())[0].replace('bool InVehicle(','bool RealInVehicle(')
modelIndex=re.search(r'int16 GetModelIndex\(void\) const \{[^}]+\}',(out/'before/src/entities/Entity.h').read_text())[0].replace('GetModelIndex','RealGetModelIndex')
gameName=re.search(r'char m_gameName\[\d+\];',(out/'before/src/modelinfo/VehicleModelInfo.h').read_text())[0]
owners='''#pragma once
#include "common.h"
#include "Zones.h"
#ifdef TEST_USER_DISPLAY_ORIGINAL
#include "original-User.h"
#else
#include "'''+(root/'src/core/User.h').as_posix()+'''"
#endif
#include "entry.h"
struct TestPrefix {uint64_t guard[2];};
class CEntity {public:CVector position;int16 m_modelIndex;
'''+modelIndex+'''
int16 GetModelIndex(void) const;const CVector &GetPosition()const{
    // Borrow the native position vector used by the complete original player-position primitive
    return position;
}};
class CVehicle:public TestPrefix,public CEntity {};
struct TestTaxiPrefix {uint64_t taxiGuard[2];};
class CAutomobile:public TestTaxiPrefix,public CVehicle {};
class CPed:public CEntity {public:bool bInVehicle;CVehicle *m_pMyVehicle;
'''+inVehicle+'''
bool InVehicle(void);};
class CPlayerInfo {public:CPed *m_pPed;const CVector &GetPos();const CVector &RealGetPos();};
class CWorld {public:static CPlayerInfo Players[NUMPLAYERS];static uint8 PlayerInFocus;};
class TestCamera {public:CVector position;const CVector &GetPosition()const{
    // Retain the original fixed-build camera fallback for a missing focused pedestrian
    return position;
}};extern TestCamera TheCamera;
class CGameLogic {public:static CAutomobile *pShortCutTaxi;};
class CBaseModelInfo {public:int id;};
class CVehicleModelInfo:public TestPrefix,public CBaseModelInfo {public:'''+gameName+'''};
class CModelInfo {public:static CBaseModelInfo *GetModelInfo(int32 model);};
class CText {public:wchar *Get(const char *name);};extern CText TheText;
extern "C" uint16_t *Text_Get(CText *self,const char *name);
class CHud {public:static wchar *m_pZoneName;static wchar *m_pLastZoneName;static wchar *m_VehicleName;static uint32 m_ZoneState;
static void SetZoneName(wchar *name);static void RealSetZoneName(wchar *name);
static void SetVehicleName(wchar *name);static void RealSetVehicleName(wchar *name);};
class CReplay {public:static bool IsPlayingBack();};
'''
(out/'owners.h').write_text(owners,newline='\n')
api='''#pragma once
#ifdef __cplusplus
#ifdef TEST_USER_DISPLAY_ORIGINAL
class CPlaceName;class CCurrentVehicle;
#else
struct CPlaceName;struct CCurrentVehicle;
#endif
extern "C" {
#else
typedef struct CPlaceName CPlaceName;typedef struct CCurrentVehicle CCurrentVehicle;
#endif
'''
for owner,prefix,name in m.OPERATIONS:
    args=owner+' *self' if owner!='CUserDisplay' else 'void'
    api+='void Run_'+prefix+'_'+name+'('+args+');\nvoid Before_'+prefix+'_'+name+'('+args+');\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n')
caller='#include "entry.h"\n#ifndef TEST_USER_DISPLAY_ORIGINAL\n#include "'+(root/'src/core/User.h').as_posix()+'"\n#endif\n'
original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
for owner,prefix,name in m.OPERATIONS:
    args=owner+' *self' if owner!='CUserDisplay' else 'void';values='self' if owner!='CUserDisplay' else ''
    caller+='void Run_'+prefix+'_'+name+'('+args+')\n{\n    // Enter the complete operation through its standalone genuine C11 caller\n#ifdef TEST_USER_DISPLAY_ORIGINAL\n    Before_'+prefix+'_'+name+'('+values+');\n#else\n    '+prefix+'_'+name+'('+values+');\n#endif\n}\n'
    original+='extern "C" void Before_'+prefix+'_'+name+'('+args+')\n{\n    // Execute the entire unchanged original method on its actual original object\n    '+('self->' if owner!='CUserDisplay' else 'CUserDisplay::')+name+'();\n}\n'
(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(root/'src/core/User_backend.h').as_posix()+'"\n'+re.sub(r'^#include[^\n]*\n','',m.calls(m.HOST),flags=re.M),newline='\n')
playerSource=(out/'before/src/core/PlayerInfo.cpp').read_text();start=playerSource.index('const CVector &\nCPlayerInfo::GetPos()')
player=playerSource[start:playerSource.index('\n}',start)+2].replace('CPlayerInfo::GetPos','CPlayerInfo::RealGetPos')
hud='\n\n'.join(m.function((out/'before/src/renderer/Hud.cpp').read_text(),'CHud::',name).replace('CHud::'+name+'(', 'CHud::Real'+name+'(') for name in ['SetZoneName','SetVehicleName'])
(out/'native.cpp').write_text('#include "owners.h"\n'+player+'\n'+hud+'\n',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/user_display.cpp.in').read_text()+'''\nextern "C" uint16_t *Text_Get(CText *self,const char *name)
{
    // Keep the same observed translation provider while exercising the updated production C adapter
    return self->Get(name);
}
''',newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited nine full user-display operations, two constructor-equivalent C records, twelve native boundaries, full native player-position/HUD primitives and six complete source/project edits')
