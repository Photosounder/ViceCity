"""Prepare complete original/C producers, exact native primitives, and independent runtime callers."""
from pathlib import Path
import re
from memory_heap_migration import body
from rect_migration import mask
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/weapon-effects-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True);saved=out/'before';tree=out/'overlay'
original=(saved/'src/weapons/WeaponEffects.cpp').read_text()
sdk='''#pragma once
#include "config.h"
#include "Sprite.h"
#include "TxdStore.h"
#include "WeaponType.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef int32_t int32;
typedef uint8_t uint8;
typedef int32_t RwRenderState;
struct RwV3d {float x,y,z;};
struct CVector: RwV3d {
    CVector(void) {}
    CVector(float a,float b,float c) {x=a;y=b;z=c;}
};
struct RwRaster {uint32_t id;};
struct RwTexture {uint32_t id;bool alive;RwRaster *raster;};
struct TestWeapon {int m_eWeaponType;};
struct TestPlayer {TestWeapon weapon;TestWeapon *GetWeapon(void){return &weapon;}};
TestPlayer *FindPlayerPed(void);
RwTexture *RwTextureRead(const char *,const char *);
void RwTextureDestroy(RwTexture *);
RwRaster *RwTextureGetRaster(RwTexture *);
void RwRenderStateSet(RwRenderState,void *);
extern "C" void PushRendergroup(const char *);
extern "C" void PopRendergroup(void);
#ifndef FINAL
#define PUSH_RENDERGROUP(s) PushRendergroup(s)
#define POP_RENDERGROUP() PopRendergroup()
#else
#define PUSH_RENDERGROUP(s)
#define POP_RENDERGROUP()
#endif
#define VALIDATE_SIZE(t,n) static_assert(sizeof(t)==n,"Record size")
#define nil NULL
#define TRUE 1
#define FALSE 0
#define TWOPI (((float)3.14159265358979323846)*2)
'''
for i,name in enumerate(sorted(set(re.findall(r'\brw(?:RENDERSTATE\w+|BLEND\w+)\b',original)))):sdk+='enum {'+name+'='+str(i+200)+'};\n'
(probe/'owners.h').write_text(sdk,newline='\n')
(probe/'original-WeaponEffects.h').write_text((saved/'src/weapons/WeaponEffects.h').read_text(),newline='\n')
(probe/'original.cpp').write_text('#include "owners.h"\n#include "original-WeaponEffects.h"\n'+re.sub(r'^#include[^\n]*\n','',original,flags=re.M),newline='\n')
(probe/'after-owner.c').write_text((tree/'src/weapons/WeaponEffects.c').read_text(),newline='\n')
native=(tree/'src/weapons/WeaponEffects_native.cpp').read_text()
(probe/'native.cpp').write_text('#include "owners.h"\n#include "WeaponEffects_backend.h"\n'+re.sub(r'^#include[^\n]*\n','',native,flags=re.M),newline='\n')
peers={'rw/RwHelper_native.cpp':['RwHelper_HostRenderState'],'renderer/Shadow_native.cpp':['Shadow_HostTextureDestroy'],'renderer/Sprite2d_native.cpp':['Sprite2d_HostTextureRead','Sprite2d_HostTextureRaster']}
code='#include "owners.h"\n#include "WeaponEffects_backend.h"\n'
for path,names in peers.items():
    # Retain complete actual peer primitive definitions without replacing owner policy
    source=(tree/'src'/path).read_text()
    for name in names:
        # Extract each typed native definition together with its original SDK conversions
        match=re.search(r'(?m)^[\w *]+\b'+name+r'\([^{};]*\)\s*\{',mask(source));assert match,(path,name);declaration=source[match.start():match.end()-1].strip();code+=declaration+'\n{'+body(source,declaration)+'}\n'
(probe/'peers.cpp').write_text(code,newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct TestState {float x,y,z,size,rotation;uint8_t r,g,b,a;bool active;} TestState;
#ifdef __cplusplus
extern "C" {
#endif
void TestAction(unsigned operation);
void TestRead(TestState *state);
void TestRotation(float value);
#ifdef __cplusplus
}
#endif
'''
(probe/'entry.h').write_text(entry,newline='\n')
for before in [True,False]:
    # Enter the original C++ value API or the independently compiled complete C API
    prefix='#include "owners.h"\n#include "original-WeaponEffects.h"\nextern CWeaponEffects gCrossHair;\n' if before else '#include "WeaponEffects.h"\n'
    owner='CWeaponEffects::' if before else 'WeaponEffects_';vector='CVector(2,3,4)' if before else '(WeaponEffectsVector){2,3,4}'
    code=prefix+'#include "entry.h"\nvoid TestAction(unsigned operation)\n{\n    // Dispatch complete application operations with the original argument values\n    switch(operation){case 0:'+owner+'Init();break;case 1:'+owner+'Shutdown();break;case 2:'+owner+'MarkTarget('+vector+',7,17,27,37,0.375f);break;case 3:'+owner+'ClearCrossHair();break;case 4:'+owner+'Render();break;}\n}\nvoid TestRead(TestState *state)\n{\n    // Observe each defined live field independently of padding and SDK representation\n    state->x=gCrossHair.m_vecPos.x;state->y=gCrossHair.m_vecPos.y;state->z=gCrossHair.m_vecPos.z;state->size=gCrossHair.m_fSize;state->rotation=gCrossHair.m_fRotation;state->r=gCrossHair.m_nRed;state->g=gCrossHair.m_nGreen;state->b=gCrossHair.m_nBlue;state->a=gCrossHair.m_nAlpha;state->active=gCrossHair.m_bActive;\n}\nvoid TestRotation(float value)\n{\n    // Allow controlled external draw callbacks to mutate the actual owner field\n    gCrossHair.m_fRotation=value;\n}\n'
    (probe/('before-caller.cpp' if before else 'after-caller.c')).write_text(code,newline='\n')
(probe/'fixture.cpp').write_text((root/'utils/tests/weapon_effects_cases.cpp.in').read_text(),newline='\n')
print('Prepared both complete owners, independent callers and four exact native peer primitives')
