"""Project complete weapon effect policy and all native vector consumers into C ownership."""
import re
from rect_migration import mask
RENAMES={'src/weapons/WeaponEffects.cpp':'src/weapons/WeaponEffects.c'}


def header():
    # Expose only C scalar records and explicit original operations
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct WeaponEffectsVector {float x,y,z;} WeaponEffectsVector;
typedef struct WeaponEffects {
    bool m_bActive;
    WeaponEffectsVector m_vecPos;
    uint8_t m_nRed,m_nGreen,m_nBlue,m_nAlpha;
    float m_fSize,m_fRotation;
} WeaponEffects;
#ifdef __cplusplus
extern "C" {
#endif
extern WeaponEffects gCrossHair;
void WeaponEffects_Construct(WeaponEffects *self);
void WeaponEffects_Deinit(WeaponEffects *self);
void WeaponEffects_Init(void);
void WeaponEffects_Shutdown(void);
void WeaponEffects_MarkTarget(WeaponEffectsVector pos,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha,float size);
void WeaponEffects_ClearCrossHair(void);
void WeaponEffects_Render(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''


def boundary(original):
    # Translate SDK constants and query the current native player weapon without moving owner policy
    constants=sorted(set(re.findall(r'\brw(?:RENDERSTATE\w+|BLEND\w+)\b',original)))
    h='//+ rouz edit (ChatGPT)\n#pragma once\n#include "WeaponEffects.h"\n#include "Sprite2d_backend.h"\n#include "RwHelper_backend.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\nint32_t WeaponEffects_HostWeaponType(void);\nint32_t WeaponEffects_HostConstant(int32_t identity);\nenum {'+','.join('WEAPON_EFFECTS_CONST_'+v for v in constants)+'};\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
    native='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "WeaponEffects_backend.h"
#include "PlayerPed.h"
#include "World.h"
int32_t WeaponEffects_HostWeaponType(void)
{
    // Query the actual player weapon at the original rendering boundary
    return FindPlayerPed()->GetWeapon()->m_eWeaponType;
}
int32_t WeaponEffects_HostConstant(int32_t identity)
{
    // Translate SDK tokens while keeping every render-state decision in C
    switch(identity){'''+''.join('case WEAPON_EFFECTS_CONST_'+v+':return '+v+';' for v in constants)+'''}return 0;
}
//- rouz edit (ChatGPT)
'''
    return h,native


def producer(original):
    # Preserve every complete original method body while replacing only C++ representations and SDK leaves
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=re.sub(r'CWeaponEffects::CWeaponEffects\(\)\s*\{\s*\}', 'void WeaponEffects_Construct(WeaponEffects *self)\n{\n    // Preserve the empty original constructor and empty vector construction\n    (void)self;\n}',source)
    source=re.sub(r'CWeaponEffects::~CWeaponEffects\(\)\s*\{\s*\}', 'void WeaponEffects_Deinit(WeaponEffects *self)\n{\n    // Preserve the empty original destructor without changing texture ownership\n    (void)self;\n}',source)
    source=source.replace('RwTexture *gpCrossHairTex;','void *gpCrossHairTex;').replace('CWeaponEffects gCrossHair;','WeaponEffects gCrossHair;')
    source=source.replace('CWeaponEffects::','WeaponEffects_').replace('CVector pos','WeaponEffectsVector pos').replace('CVector(0.0f, 0.0f, 0.0f)','(WeaponEffectsVector){0.0f, 0.0f, 0.0f}').replace('RwV3d pos;','WeaponEffectsVector pos;')
    source=re.sub(r'\buint8\b','uint8_t',source);source=re.sub(r'\bint32\b','int32_t',source)
    projection=r'\(\[\]\(const RwV3d &spriteInput,.*?\}\)\(gCrossHair\.m_vecPos, &pos, &w, &h, true\)'
    source,count=re.subn(projection,'Sprite_CalcScreenCoors(Sprite_VectorRef(&gCrossHair.m_vecPos.x,&gCrossHair.m_vecPos.y,&gCrossHair.m_vecPos.z),Sprite_VectorOut(&pos.x,&pos.y,&pos.z),&w,&h,true)',source,flags=re.S);assert count==1
    source=source.replace('FindPlayerPed()->GetWeapon()->m_eWeaponType','WeaponEffects_HostWeaponType()').replace('RwTextureRead(','Sprite2d_HostTextureRead(').replace('RwTextureDestroy(','Shadow_HostTextureDestroy(').replace('RwTextureGetRaster(','Sprite2d_HostTextureRaster(').replace('RwRenderStateSet(','RwHelper_HostRenderState(')
    source=source.replace('PUSH_RENDERGROUP("WeaponEffects_Render");','#ifndef FINAL\n            PushRendergroup("CWeaponEffects::Render");\n#endif').replace('POP_RENDERGROUP();','#ifndef FINAL\n            PopRendergroup();\n#endif')
    source=re.sub(r'\brw(?:RENDERSTATE\w+|BLEND\w+)\b',lambda m:'WeaponEffects_HostConstant(WEAPON_EFFECTS_CONST_'+m[0]+')',source)
    source=source.replace('(void *)WeaponEffects_HostConstant','(void *)(uintptr_t)WeaponEffects_HostConstant').replace('nil','NULL').replace('FALSE','false').replace('TRUE','true')
    source=re.sub(r'(WeaponEffects_(?:Init|Shutdown|MarkTarget|ClearCrossHair|Render)\([^{}]*\)\s*\{)',r'\1\n    // Execute the complete original weapon effect operation with C-owned state',source)
    return '//+ rouz edit (ChatGPT)\n#include "WeaponEffects.h"\n#include "WeaponEffects_backend.h"\n#include "Sprite.h"\n#include "TxdStore.h"\n#include "RwHelper.h"\n#include "WeaponType.h"\n#define TWOPI (2.0f * 3.14159265358979323846f)\n'+source+'\n//- rouz edit (ChatGPT)\n'


def weapon_effects_calls(source):
    # Retain unrelated sources while converting every actual class reference and value argument
    if not re.search(r'\bCWeaponEffects\b',mask(source)):return source
    if re.search(r'class\s+CWeaponEffects\s*\{',mask(source)):return header()
    if 'CWeaponEffects::CWeaponEffects()' in source:return producer(source)
    adapter='''([](CVector weaponPosition,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha,float size){
    // Copy the original by-value vector before entering the complete C operation
    WeaponEffectsVector position={weaponPosition.x,weaponPosition.y,weaponPosition.z};
    WeaponEffects_MarkTarget(position,red,green,blue,alpha,size);
})'''
    source=source.replace('CWeaponEffects::MarkTarget',adapter)
    source=re.sub(r'\bCWeaponEffects::(\w+)',r'WeaponEffects_\1',source);source=re.sub(r'\bCWeaponEffects\b','WeaponEffects',source)
    lines=source.splitlines(keepends=True)
    for i in reversed(range(len(lines))):
        # Document each converted executable call at its original consumer site
        if re.search(r'WeaponEffects_(?:Init|Shutdown|ClearCrossHair|Render)\(',lines[i]):lines.insert(i,re.match(r'[ \t]*',lines[i])[0]+'// Enter the complete C weapon effect operation\n')
    return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'


def weapon_effects_project(source):
    # Select the complete C owner and only native primitive operations in generated projects
    import object_names_migration as objects
    source=source.replace('src\\weapons\\WeaponEffects.cpp','src\\weapons\\WeaponEffects.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"\n]*src\\weapons\\WeaponEffects\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing options while explicitly selecting genuine C11 compilation
            value=match[2] or ''
            for tag,text in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in value:value=re.sub('<'+tag+'>[^<]*</'+tag+'>','<'+tag+'>'+text+'</'+tag+'>',value)
                else:value+='\n      <'+tag+'>'+text+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+value.rstrip()+'\n    </ClCompile>';native=match[1][:-len('WeaponEffects.c')]+'WeaponEffects_native.cpp'
            if '<ClCompile Include="'+native+'"' not in source:result+='\n    <ClCompile Include="'+native+'" />'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)



def calls(source):
    # Preserve all preceding C weapon effects before applying complete C shot policy
    import shot_info_migration as shots
    return shots.calls(weapon_effects_calls(source))


def project(source):
    # Select the complete C shot producer after existing native project metadata
    import shot_info_migration as shots
    return shots.project(weapon_effects_project(source))
