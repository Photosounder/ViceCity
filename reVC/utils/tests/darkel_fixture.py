"""Prepare entire original/C rampage owners with observed native weapon and font primitives."""
from pathlib import Path
import re,json,sys
import darkel_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/darkel-c-tests';saved=out/'before';preview=out/'preview'
old=(saved/'src/control/Darkel.cpp').read_text();originalHeader=(saved/'src/control/Darkel.h').read_text()
if '--production' in sys.argv:
    for path in json.loads((out/'changes.json').read_text()):assert (root/path).read_text()==m.calls((saved/path).read_text()),path
    for name in ['RampageSystem.c','Darkel_backend.h']:assert (root/'src/control'/name).read_text()==(preview/name).read_text(),name
for name,path in [('WeaponType.h','src/weapons/WeaponType.h'),('ModelIds.h','src/modelinfo/ModelIds.h'),('PedType.h','src/peds/PedType.h'),('VehicleAppearance.h','src/modelinfo/VehicleAppearance.h'),('FontStyles.h','src/renderer/FontStyles.h'),('Draw_constants.h','src/renderer/Draw_constants.h'),('soundlist.h','src/audio/soundlist.h'),('AnimGroups.h','src/animation/AnimGroups.h')]:
    (out/name).write_text((saved/path).read_text() if (saved/path).exists() else (root/path).read_text(),newline='\n')
(out/'ModelIndices.h').write_text('#pragma once\n#include "ModelIds.h"\n',newline='\n')
(out/'config.h').write_text('#pragma once\n',newline='\n')
weaponHeader=(saved/'src/weapons/WeaponInfo.h').read_text();record=weaponHeader[weaponHeader.index('typedef struct WeaponInfoVector'):weaponHeader.index('#ifdef __cplusplus',weaponHeader.index('typedef struct CWeaponInfo'))]
weaponC='#pragma once\n#include <stdint.h>\n#include "WeaponType.h"\n#include "AnimGroups.h"\n'+record+'\n#ifdef __cplusplus\nextern "C" {\n#endif\nCWeaponInfo *WeaponInfo_GetWeaponInfo(enum eWeaponType type);\n#ifdef __cplusplus\n}\n#endif\n';(out/'WeaponInfo.h').write_text(weaponC,newline='\n')
capi='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "WeaponType.h"
#include "soundlist.h"
#include "FontStyles.h"
#include "VehicleAppearance.h"
#ifdef __cplusplus
class CPed;class CVehicle;class CPlayerPed;class CWeapon;class CBaseModelInfo;
extern "C" {
#else
typedef struct CPed CPed;typedef struct CVehicle CVehicle;typedef struct CPlayerPed CPlayerPed;typedef struct CWeapon CWeapon;typedef struct CBaseModelInfo CBaseModelInfo;
#endif
typedef struct CWanted {int32_t level,identity;} CWanted;
typedef struct CText {int unused;} CText;
extern CText TheText;
uint16_t *Text_Get(CText *text,const char *key);
void Wanted_SetWantedLevel(CWanted *wanted,int32_t level);
void DMAudio_PlayFrontEndSound(uint16_t sound,uint32_t value);
const uint32_t *Timer_GetTimeInMilliseconds(void);const uint32_t *Timer_GetFrameCounter(void);
float Draw_BackendScreenWidth(void);float Draw_BackendScreenHeight(void);float Draw_GetAspectRatio(void);float Draw_ScaleY(float y);
#ifdef __cplusplus
}
#endif
'''
(out/'capi.h').write_text(capi,newline='\n')
for name in ['Timer.h','Text.h','DMAudio.h','Wanted.h','Draw.h','Draw_backend.h']:(out/name).write_text('#pragma once\n#include "capi.h"\n',newline='\n')
common='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "config.h"
#include "capi.h"
#include "Draw_constants.h"
typedef int32_t int32;typedef uint32_t uint32;typedef uint16_t uint16;typedef uint8_t uint8;typedef int8_t int8;typedef uint16_t wchar;
#define nil NULL
#define Min(a,b) ((a)<(b)?(a):(b))
struct TestScreen {int width,height;};extern TestScreen RsGlobal;
'''
nativeCommon=(saved/'src/core/common.h').read_text();start=nativeCommon.index('#ifdef GTA_PS2\n\t#ifdef GTA_PAL');end=nativeCommon.index('#include "maths.h"',start)
(out/'common.h').write_text(common+nativeCommon[start:end],newline='\n')
(out/'original-Darkel.h').write_text(originalHeader.replace('private:','public:'),newline='\n')
owners='''#pragma once
#include "common.h"
#include "WeaponInfo.h"
#include "PedType.h"
#ifdef TEST_DARKEL_ORIGINAL
#include "original-Darkel.h"
#define GLOBAL(name) CDarkel::name
#else
#include "@PREVIEW@/Darkel.h"
#define GLOBAL(name) Darkel_##name
#endif
void Observe(unsigned point,int identity);
template<class T>struct Observed {T value;int identity;unsigned read,write;operator T()const{Observe(read,identity);return value;}Observed &operator=(T v){Observe(write,identity);value=v;return *this;}};
struct NativePrefix {uintptr_t padding[3];};
class CEntity {public:int identity;Observed<int32_t> model;int GetModelIndex();};
class CPed:public NativePrefix,public CEntity {public:Observed<bool> bChrisCriminal;uint32_t m_nPedType;};
class CVehicle:public NativePrefix,public CEntity {public:Observed<int> appearance;eVehicleAppearance GetVehicleAppearance();};
class CWeapon {public:int identity;eWeaponType m_eWeaponType;eWeaponState m_eWeaponState;int32_t m_nAmmoTotal,m_nAmmoInClip;CWeaponInfo *GetInfo();};
struct PlayerPrefix {uintptr_t padding[5];};
class CPlayerPed:public PlayerPrefix,public CPed {public:int8_t m_nSelectedWepSlot;uint8_t m_currentWeapon;CWanted *m_pWanted;CWeapon weapons[TOTAL_WEAPON_SLOTS];CWeapon *GetWeapon();CWeapon &GetWeapon(uint8_t slot);int GetWeaponSlot(eWeaponType);void RemoveWeaponAnims(int,float);int32 GiveWeapon(eWeaponType,uint32,bool unused=true);void SetCurrentWeapon(eWeaponType);void SetCurrentWeapon(int);void MakeChangesForNewWeapon(int32);void MakeChangesForNewWeapon(eWeaponType);void SetAmmo(eWeaponType,uint32);void ClearWeaponTarget();void RemoveWeaponModel(int);void RemoveDrivebyAnims();};
CPlayerPed *FindPlayerPed();CVehicle *FindPlayerVehicle();
class CBaseModelInfo {public:int identity;uint16_t references;void AddRef();void RemoveRef();};
class CModelInfo {public:static CBaseModelInfo *GetModelInfo(int);};
class CPopulation {public:static int32 m_AllRandomPedsThisType;};
class CStats {public:static int32 CarsExploded,HelisDestroyed,BoatsExploded,PeopleKilledByPlayer,PedsKilledOfThisType[NUM_PEDTYPES],HeadsPopped,KillsSinceLastCheckpoint,PeopleKilledByOthers;static void AnotherKillFrenzyPassed();};
class CGameLogic {public:static void ClearShortCut();static void RemoveShortCutDropOffPointForMission();};
class CReplay {public:static bool IsPlayingBack();};
struct CRGBA {uint8_t r,g,b,a;CRGBA(uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha):r(red),g(green),b(blue),a(alpha){}};
class CFont {public:static void SetJustifyOff();static void SetBackgroundOff();static void SetCentreSize(float);static void SetCentreOn();static void SetPropOn();static void SetScale(float,float);static void SetColor(CRGBA);static void SetFontStyle(int16_t);static void PrintString(float,float,uint16_t*);static void SetCentreOff();static void SetRightJustifyOn();};
extern char gString[256];extern uint16_t gUString[256];void AsciiToUnicode(const char*,uint16_t*);
#include "entry.h"
'''.replace('@PREVIEW@',preview.as_posix())
(out/'owners.h').write_text(owners,newline='\n')
api='#pragma once\n#include "capi.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n';caller='#include "'+(preview/'Darkel.h').as_posix()+'"\n#include "entry.h"\n';original='#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n'
for ret,name,args in m.SIGNATURES:
    values=[] if args=='void' else [re.search(r'(\w+)\s*$',arg)[1] for arg in args.split(',')]
    api+=ret+' Run_Darkel_'+name+'('+args+');\n'+ret+' Before_Darkel_'+name+'('+args+');\n'
    original+='extern "C" '+ret+' Before_Darkel_'+name+'('+args+')\n{\n    // Invoke the complete original operation with the exact original parameter conversions\n    '+('return ' if ret!='void' else '')+'CDarkel::'+name+'('+','.join(values)+');\n}\n'
    caller+=ret+' Run_Darkel_'+name+'('+args+')\n{\n    // Exercise every public operation through a standalone strict C11 caller\n#ifdef TEST_DARKEL_ORIGINAL\n    '+('return ' if ret!='void' else '')+'Before_Darkel_'+name+'('+','.join(values)+');\n#else\n    '+('return ' if ret!='void' else '')+'Darkel_'+name+'('+','.join(values)+');\n#endif\n}\n'
api+='#ifdef __cplusplus\n}\n#endif\n';(out/'entry.h').write_text(api,newline='\n');(out/'caller.c').write_text(caller,newline='\n');(out/'original.cpp').write_text(original,newline='\n')
from stats_test_dependencies import aliases
current=m.calls((saved/'src/control/Darkel.cpp').read_text())
(out/'host.cpp').write_text('#include "owners.h"\n#include "'+(preview/'Darkel_backend.h').as_posix()+'"\n'+aliases(current)+re.sub(r'^#include[^\n]*\n','',current,flags=re.M),newline='\n');(out/'kernel.c').write_text((preview/'RampageSystem.c').read_text(),newline='\n')
(out/'fixture.cpp').write_text((root/'utils/tests/darkel.cpp.in').read_text(),newline='\n')
print('Prepared 14 complete original/C operations, 18 full canonical state observations and native weapon/font models')
