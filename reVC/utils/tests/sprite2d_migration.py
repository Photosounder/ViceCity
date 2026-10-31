"""Project complete 2D sprite policy and explicit consumer lifetimes without a C++ owner wrapper."""
from pathlib import Path
import re
from functools import lru_cache
from rect_migration import mask
from sprite2d_schema import RENAMES
import sprite2d_schema as schema,sprite2d_kernel as kernel,sprite2d_calls as client,sprite2d_lifetimes as lifetime


@lru_cache(maxsize=1)
def owner():
    # Use the immutable original overload definitions for independent whole-source consumer projection
    return (Path(__file__).resolve().parents[2]/'.diagnostics/sprite2d-c-tests/before/src/renderer/Sprite2d.cpp').read_text()


@lru_cache(maxsize=1)
def record_names():
    # Diagnose record receiver names from the complete immutable declaration set rather than rewriting arbitrary Draw methods
    saved=Path(__file__).resolve().parents[2]/'.diagnostics/sprite2d-c-tests/before/src';names={'fontButtonSprites'}
    for path in saved.rglob('*'):
        if path.suffix not in {'.cpp','.h'}:continue
        for match in re.finditer(r'\bCSprite2d\s*\*?\s*((?:\w+::)*\w+)',mask(path.read_text())):names.add(match[1].split('::')[-1])
    return names


def sprite2d_calls(source):
    # Retain unrelated sources byte-identical while converting each complete existing owner declaration or consumer
    if not re.search(r'\bCSprite2d\b',mask(source)) and not re.search(r'(?:\.|->)(?:Draw|Delete|SetTexture|SetAddressing|SetRenderState)\s*\(',mask(source)):return source
    if re.search(r'class\s+CSprite2d\s*\{',mask(source)):return schema.header(owner())
    if 'CSprite2d::SetRecipNearClip(void)' in source:return kernel.producer(source)
    original=source
    if re.search(r'\bCSprite2d\b',mask(source)):source=lifetime.calls(source)
    source=client.member_calls(source,owner(),record_names());source=client.static_calls(source,owner())
    source=source.replace('RwTextureSetFilterMode(WaypointSprite.m_pTexture,','RwTextureSetFilterMode((RwTexture*)WaypointSprite.m_pTexture,')
    code=mask(source);changes=[]
    for match in re.finditer(r'\bCSprite2d\b',code):changes.append((match.start(),match.end(),'Sprite2d'))
    for first,last,value in reversed(changes):source=source[:first]+value+source[last:]
    source=re.sub(r'\bclass Sprite2d\s*;', 'struct Sprite2d;',source)
    if source==original:return source
    lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True);depth=0;continuation=False;indices=[]
    for index,line in enumerate(lines):
        # Document changed executable blocks without breaking preprocessor continuations or existing C reference adapters
        directive=continuation or line.lstrip().startswith('#');continuation=line.rstrip().endswith('\\') if directive else False
        if not directive and depth>0 and not line.lstrip().startswith('//') and not line.lstrip().startswith(('static ','void ','int ','float ','bool ')) and ('Sprite2d_' in line or 'RwTextureSetFilterMode((RwTexture*)WaypointSprite.m_pTexture' in line):
            # Keep a single descriptive comment when the generated block already has one
            if index==0 or not lines[index-1].lstrip().startswith('//'):indices.append(index)
        depth+=code[index].count('{')-code[index].count('}')
    for index in reversed(indices):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Preserve the original operation through the C sprite interface\n')
    return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n' 


def sprite2d_project(source):
    # Select the complete C owner and retain only actual SDK vertex storage and primitives as native
    import object_names_migration as objects
    for old,new in RENAMES.items():source=source.replace(old.replace('/','\\'),new.replace('/','\\'))
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"\n]*src\\renderer\\Sprite2d\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing per-file options while making C11 compilation explicit
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                # Apply language metadata only to the converted producer
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';base=match[1][:-len('Sprite2d.c')]
            if '<ClCompile Include="'+base+'Sprite2d_native.cpp"' not in source:result+='\n    <ClCompile Include="'+base+'Sprite2d_native.cpp" />'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)



def calls(source):
    # Preserve preceding C sprite ownership before converting the entire weapon effect owner
    import weapon_effects_migration as effects
    return effects.calls(sprite2d_calls(source))


def project(source):
    # Add the complete C weapon effect producer after the preceding C project metadata
    import weapon_effects_migration as effects
    return effects.project(sprite2d_project(source))
