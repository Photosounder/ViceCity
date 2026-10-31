"""Convert complete sprite policy, consumers, and obsolete native forwarding into C."""
from pathlib import Path
import re
from rect_migration import mask,closing,arguments
from sprite_schema import RENAMES,signatures
import sprite_schema as schema,sprite_kernel as kernel,sprite_peers as peers


def original_owner():
    # Obtain the immutable owner only for its unchanged overload declarations and separately versioned public header
    return (Path(__file__).resolve().parents[2]/'.diagnostics/sprite-c-tests/before/src/renderer/Sprite.cpp').read_text()


def sprite_calls(source):
    # Keep unrelated whole sources byte-identical and recognize complete original definitions before consumer syntax
    if 'CSprite' not in source and 'ScrollBar_BackendProject(void *input' not in source and 'ScrollBar_BackendProject(points.input.owner' not in source:return source
    if re.search(r'class\s+CSprite\s*\{',source):
        # Require both original inline getter bodies rather than silently substituting another historical implementation
        for depth in ['Near','Far']:assert re.search(r'Get'+depth+r'ScreenZ\(void\)\s*\{\s*return m_f2D'+depth+r'ScreenZ;\s*\}',source)
        return schema.header(original_owner())
    if 'CSprite::CalcHorizonCoors(void)' in source:return kernel.producer(source)
    original=source;source=peers.remove(source)
    source=source.replace('ScrollBar_BackendProject(void *input,void *output','ScrollBar_BackendProject(ScrollBarPoint input,ScrollBarPoint output').replace('ScrollBar_BackendProject(points.input.owner,points.output.owner','ScrollBar_BackendProject(points.input,points.output')
    changes=[];code=mask(source)
    for match in re.finditer(r'\bCSprite::(\w+)\s*\(',code):
        # Keep each original complete argument list and its temporary lifetimes at the native consumer boundary
        name=match[1];opening=code.index('(',match.start());end=closing(code,opening);raw=source[opening+1:end-1];args=arguments(raw)
        if name=='CalcScreenCoors':
            # Bind the original SDK references once before borrowing their actual scalar subobjects for C
            assert len(args)==5
            replacement='([](const RwV3d &spriteInput,RwV3d *spriteOutput,float *spriteWidth,float *spriteHeight,bool spriteFarClip){\n    // Keep actual native input and output subobjects alive through the complete synchronous C projection\n    return Sprite_CalcScreenCoors(Sprite_VectorRef(&spriteInput.x,&spriteInput.y,&spriteInput.z),Sprite_VectorOut(&spriteOutput->x,&spriteOutput->y,&spriteOutput->z),spriteWidth,spriteHeight,spriteFarClip);\n})('+raw+')'
        elif name.startswith('RenderBufferedOneXLUSprite2D'):
            # Preserve the original SDK colour conversion and full-expression lifetime before passing live fields to C
            row=next(r for r in signatures(original_owner()) if r[1]==name);params=row[2].replace('SpriteColourRef colour','const RwRGBA &colour');values=[]
            for param in arguments(row[2]):
                # Forward each scalar once and replace only the typed SDK colour receiver with its actual byte fields
                arg=re.search(r'(\w+)\s*$',param)[1];values.append('Sprite_ColourRef(&colour.red,&colour.green,&colour.blue,&colour.alpha)' if arg=='colour' else arg)
            replacement='([]('+params+'){\n    // Retain the original typed colour reference and argument evaluation at the C operation boundary\n    Sprite_'+name+'('+','.join(values)+');\n})('+raw+')'
        else:
            # Preserve every ordinary scalar call without introducing an unnecessary compatibility wrapper
            assert name!='Set6Vertices2D',name;replacement='Sprite_'+name+'('+raw+')'
        changes.append((match.start(),end,replacement))
    for first,last,replacement in reversed(changes):source=source[:first]+replacement+source[last:]
    if source==original:return source
    lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True);depth=0;continuation=False;indices=[]
    for index,line in enumerate(lines):
        # Keep inserted comments outside preprocessing continuations while documenting each changed executable call block
        directive=continuation or line.lstrip().startswith('#');continuation=line.rstrip().endswith('\\') if directive else False
        if not directive and depth>0 and 'Sprite_' in line and not line.lstrip().startswith('//'):indices.append(index)
        depth+=code[index].count('{')-code[index].count('}')
    for index in reversed(indices):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Use the complete C sprite operation with the original argument values\n')
    return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'


def sprite_project(source):
    # Select complete C policy and peer forwarding while retaining only typed SDK storage and primitives as native
    import object_names_migration as objects
    for old,new in RENAMES.items():source=source.replace(old.replace('/','\\'),new.replace('/','\\'))
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"\n]*src\\renderer\\(?:Sprite|Sprite_bindings)\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve existing file options and make the intended C11 language explicit
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                # Apply per-file language metadata without changing any unrelated configuration option
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';base=match[1][:match[1].rfind('\\')+1]
            if match[1].endswith('Sprite.c'):
                # Add only the actual native primitives and the genuine C peer forwarders
                if '<ClCompile Include="'+base+'Sprite_native.cpp"' not in source:result+='\n    <ClCompile Include="'+base+'Sprite_native.cpp" />'
                if '<ClCompile Include="'+base+'Sprite_bindings.c"' not in source:result+='\n    <ClCompile Include="'+base+'Sprite_bindings.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)



def calls(source):
    # Apply the complete C two dimensional sprite owner after preceding sprite policy
    import sprite2d_migration as sprite2d
    return sprite2d.calls(sprite_calls(source))


def project(source):
    # Select the complete C two dimensional sprite producer after existing project metadata
    import sprite2d_migration as sprite2d
    return sprite2d.project(sprite_project(source))
