"""Move complete bone mappings into C and separate the ped-node constants from native model classes."""
import re
import object_names_migration as objects
RENAMES={'src/animation/Bones.cpp':'src/animation/Bones.c'}

def ped_enum(original):
    # Preserve every original node enumerator, implicit value and comment without depending on a native class header
    start=original.index('enum PedNode {');end=original.index('\n};',start)+3
    return original[start:end]

def ped_header(original):
    # Expose the exact original node numbering to both C11 and remaining native consumers
    return '//+ rouz edit (ChatGPT)\n#pragma once\n\n'+ped_enum(original)+'\n//- rouz edit (ChatGPT)\n'

def header(original):
    # Retain all bone tag constants and give the existing free functions C language linkage
    split=original.index('int ConvertPedNode2BoneTag(')
    return '//+ rouz edit (ChatGPT)\n'+original[:split]+'''#ifdef __cplusplus
extern "C" {
#endif
'''+original[split:].rstrip()+'''
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def kernel(original):
    # Preserve both complete switch algorithms and the project's continuing assertion policy in C
    source=original[original.index('int\nConvertPedNode2BoneTag('):]
    source=source.replace('assert(','Bones_Assert(').replace('return nil;','return NULL;')
    source=source.replace('{\n\tswitch(node)', '{\n\t// Preserve every original ped node mapping and unsupported-node behavior\n\tswitch(node)')
    source=source.replace('{\n\tswitch(tag)', '{\n\t// Preserve every original bone tag name and the null result for unknown tags\n\tswitch(tag)')
    return '''//+ rouz edit (ChatGPT)
#include "Bones.h"
#include "PedNode.h"
#include "config.h"
#include <stddef.h>
void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
#ifndef MASTER
#define Bones_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define Bones_Assert(expression) (expression)
#endif
'''+source+'//- rouz edit (ChatGPT)\n'

def bones_calls(source):
    # Convert only the complete helper producer, its public API and the original node enum owner
    if '#include "common.h"' in source and '\nConvertPedNode2BoneTag(int node)\n{' in source:return kernel(source)
    if 'enum BoneTag\n{' in source and 'extern "C"' not in source:return header(source)
    if 'enum PedNode {' in source and 'class CPedModelInfo' in source:
        # Keep the native class and all its fields unchanged while sharing original constants through C
        return '//+ rouz edit (ChatGPT)\n'+source.replace(ped_enum(source),'#include "PedNode.h"')+'//- rouz edit (ChatGPT)\n'
    return source

def bones_project(source):
    # Select the complete helper as C11 without changing callers or adding native adapters
    source=source.replace('src\\animation\\Bones.cpp','src\\animation\\Bones.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\animation\\Bones\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain other project metadata and explicitly compile the helper producer as C11
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\modelinfo\\PedModelInfo\.h)"\s*/>',source)
    if anchor:
        # Keep the shared enum discoverable beside its original model header in generated projects
        path=anchor[1][:-len('PedModelInfo.h')]+'PedNode.h';entry='<ClInclude Include="'+path+'" />'
        if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)



def calls(source):
    # Preserve complete C bone mappings before the full frame accumulation owner and dispatch data conversion
    import frame_update_migration as frames
    return frames.calls(bones_calls(source))


def project(source):
    # Retain bone compilation metadata before selecting the C callbacks and native SDK primitive producer
    import frame_update_migration as frames
    return frames.project(bones_project(source))
