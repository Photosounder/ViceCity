"""Replace native shadow consumers with explicit C records and lifecycle calls."""
import re
from rect_migration import mask
from shadow_cameras_schema import RENAMES,camera_header,cutscene_header

CONSUMERS=['src/peds/Ped.h','src/peds/Ped.cpp','src/objects/CutsceneObject.h','src/objects/CutsceneObject.cpp','src/renderer/Shadows.h','src/renderer/Shadows.cpp']

def shadow_cameras_calls(source):
    # Replace only the diagnosed shadow record owners and their existing consumer operations
    original=source
    if re.search(r'class\s+CShadowCamera\s*\{',source):return camera_header()
    if re.search(r'class\s+CCutsceneShadow\s*\{',source):return cutscene_header()
    if 'CShadowCamera::CShadowCamera()' in source:return camera_producer(source)
    if 'CCutsceneShadow::CCutsceneShadow()' in source:return cutscene_producer(source)
    if 'RwHelper_RpClumpGetBoundingSphereScope' in source:
        # Extend the complete preceding C peer while retaining its existing SDK receiver entry
        import shadow_cameras_peer as peer
        return peer.producer(source)
    if re.search(r'(?m)^void\s*\*\s*RpClumpGetBoundingSphere\([^;]+;',source) and 'RwHelperSphereRef' not in source:
        # Expose canonical C sphere receiver fields from the existing C public helper header
        import shadow_cameras_peer as peer
        return peer.header(source)
    source=source.replace('class CCutsceneShadow','struct CutsceneShadow').replace('CCutsceneShadow','CutsceneShadow').replace('CShadowCamera','ShadowCamera')
    source=re.sub(r'(\b\w+)->~CutsceneShadow\(\)',r'CutsceneShadow_Deinit(\1)',source)
    source=re.sub(r'std::allocator<CutsceneShadow>\(\)\.construct\((\w+)\)',r'CutsceneShadow_Init(\1)',source)
    for method in ['IsInitialized','UpdateForCutscene','GetShadowRwTexture']:
        # Preserve the original receiver evaluation once for each former member call
        cast='(RwTexture*)' if method=='GetShadowRwTexture' else ''
        source=re.sub(r'(m_pRTShadow|m_pShadow|shadow|pShadow)->'+method+r'\(\)',lambda m:cast+'CutsceneShadow_'+method+'('+m[1]+')',source)
    source=re.sub(r'(\b\w+)->GetShadowCamera\(\)',r'CutsceneShadow_GetShadowCamera(\1,CUTSCENE_SHADOW_RASTER)',source)
    source=re.sub(r'(shadow)->GetRwCamera\(\)',r'(RwCamera*)ShadowCamera_GetRwCamera(\1)',source)
    source=re.sub(r'(m_pRTShadow|m_pShadow|shadow|pShadow)->Create\(([^;\n]*)\)',r'CutsceneShadow_Create(\1,\2)',source)
    source=re.sub(r'(m_pRTShadow|m_pShadow|shadow|pShadow)->SetLightProperties\(([^;\n]*)\)',r'(RwFrame*)CutsceneShadow_SetLightProperties(\1,\2)',source)
    source=re.sub(r'(\b\w+)->DrawBorderAroundTexture\((\w+)\)',r'CutsceneShadow_DrawBorderAroundTexture(\1,&\2)',source)
    source=source.replace('RwRGBA bordercolor =','ShadowColour bordercolor =')
    source=source.replace('CVector center(pShadow->GetBaseSphere().center);','const ShadowSphere baseSphere=CutsceneShadow_GetBaseSphere(pShadow);\n\t\tCVector center(baseSphere.center.x,baseSphere.center.y,baseSphere.center.z);')
    if source==original:return source
    # Annotate changed executable blocks without altering unbraced branch ownership
    lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True);depth=0;indices=[]
    for index,line in enumerate(lines):
        # Keep declarations and preprocessor directives free of inserted executable-block comments
        if depth>0 and not line.lstrip().startswith(('#','//')) and any(token in line for token in ['CutsceneShadow_','ShadowCamera_','ShadowColour bordercolor','const ShadowSphere baseSphere','CVector center(baseSphere']):indices.append(index)
        depth+=code[index].count('{')-code[index].count('}')
    for index in reversed(indices):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Use the C shadow operation with its canonical record and explicit lifecycle\n')
    return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'


def camera_producer(source):
    # Preserve the complete original camera bodies while selecting actual C records and typed SDK primitives
    from pathlib import Path
    import shadow_cameras_schema as schema,shadow_cameras_resources as resources,shadow_cameras_effects as effects
    other=(Path(__file__).resolve().parents[2]/'.diagnostics/shadow-cameras-c-tests/before/src/renderer/CutsceneShadow.cpp').read_text()
    camera,_=resources.resource_methods(source,other)
    return schema.camera_lifecycle().replace('#include "RwHelper_backend.h"',resources.prelude())+'\n//+ rouz edit (ChatGPT)\n'+camera+'//- rouz edit (ChatGPT)\n'+'\n//+ rouz edit (ChatGPT)\n'+effects.methods(source)+'//- rouz edit (ChatGPT)\n'


def cutscene_producer(source):
    # Retain the complete parent resource and update algorithms with explicit embedded camera lifecycle
    from pathlib import Path
    import shadow_cameras_schema as schema,shadow_cameras_resources as resources,shadow_cameras_cutscene as cutscene
    other=(Path(__file__).resolve().parents[2]/'.diagnostics/shadow-cameras-c-tests/before/src/renderer/ShadowCamera.cpp').read_text()
    _,parent=resources.resource_methods(other,source)
    return schema.cutscene_lifecycle().replace('#include "CutsceneShadow.h"','#include "CutsceneShadow.h"\n'+resources.prelude())+'\n//+ rouz edit (ChatGPT)\n'+parent+'//- rouz edit (ChatGPT)\n'+'\n//+ rouz edit (ChatGPT)\n'+cutscene.methods(source)+'//- rouz edit (ChatGPT)\n'


def boundary():
    # Describe the complete typed SDK primitive boundary independently of project source generation
    import shadow_cameras_resources as resources,shadow_cameras_effects as effects,shadow_cameras_cutscene as cutscene
    return cutscene.extend(*effects.extend(*resources.boundary()))


def shadow_cameras_project(source):
    # Select strict C11 for both complete owners and retain only typed SDK primitives as native code
    import object_names_migration as objects
    for old,new in RENAMES.items():source=source.replace(old.replace('/','\\'),new.replace('/','\\'))
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"\n]*src\\renderer\\(?:ShadowCamera|CutsceneShadow)\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve all existing per-file options while replacing the source language explicitly
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                # Override inherited language metadata only for the two converted C producers
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';base=match[1][:match[1].rfind('\\')+1]
            if match[1].endswith('ShadowCamera.c') and '<ClCompile Include="'+base+'Shadow_native.cpp"' not in source:result+='\n    <ClCompile Include="'+base+'Shadow_native.cpp" />'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)



def calls(source):
    # Preserve every preceding complete C owner before applying the entire sprite successor
    import sprite_migration as sprite
    return sprite.calls(shadow_cameras_calls(source))


def project(source):
    # Select complete C sprite policy and peer bindings after the existing shadow metadata
    import sprite_migration as sprite
    return sprite.project(shadow_cameras_project(source))
