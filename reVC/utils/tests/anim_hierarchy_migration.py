"""Move complete hierarchy lifetime and animation collection policy into C11."""
import re
from rect_migration import mask,closing
from doors_migration import method
import object_names_migration as objects
RENAMES={'src/animation/AnimBlendHierarchy.cpp':'src/animation/AnimBlendHierarchy.c'}
DECLARATIONS=[('void','Init','CAnimBlendHierarchy *self'),('void','Shutdown','CAnimBlendHierarchy *self'),('void','SetName','CAnimBlendHierarchy *self,char *name'),('void','CalcTotalTime','CAnimBlendHierarchy *self'),('void','CalcTotalTimeCompressed','CAnimBlendHierarchy *self'),('void','RemoveQuaternionFlips','CAnimBlendHierarchy *self'),('void','RemoveAnimSequences','CAnimBlendHierarchy *self'),('void','Uncompress','CAnimBlendHierarchy *self'),('void','RemoveUncompressedData','CAnimBlendHierarchy *self'),('void','MoveMemory','CAnimBlendHierarchy *self,bool onlyone'),('bool','IsCompressed','CAnimBlendHierarchy *self')]
OPERATIONS=[name for _,name,_ in DECLARATIONS]

def header():
    # Retain the complete native hierarchy layout with C-owned sequence and link types
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "AnimBlendSequence.h"
#include "LinkList.h"
typedef struct CAnimBlendHierarchy {
    char name[24];
    CAnimBlendSequence *sequences;
    int16_t numSequences;
    bool compressed,keepCompressed;
    float totalLength;
    CLink *linkPtr;
} CAnimBlendHierarchy;
#ifdef __cplusplus
extern "C" {
#endif
'''+''.join(result+' AnimHierarchy_'+name+'('+args+');\n' for result,name,args in DECLARATIONS)+'''#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Retain only the existing external animation-manager cache operation
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include "AnimBlendHierarchy.h"
#ifdef __cplusplus
extern "C" {
#endif
void AnimHierarchy_HostRemoveCache(CAnimBlendHierarchy *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend():
    # Leave the separate animation manager's cache ownership at its existing boundary
    return '''
void AnimHierarchy_HostRemoveCache(CAnimBlendHierarchy *self)
{
    // Remove the actual hierarchy from the manager cache before freeing any sequences
    CAnimManager::RemoveFromUncompressedCache(self);
}
'''

def kernel(original):
    # Preserve each complete hierarchy method including its conditional compression and allocator paths
    source='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "AnimBlendHierarchy.h"
#include "AnimBlendHierarchy_backend.h"
#include <stdlib.h>
#include <string.h>
void re3_assert(const char *expression,const char *filename,unsigned int line,const char *function);
#ifndef MASTER
#define Hierarchy_Assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define Hierarchy_Assert(expression) (expression)
#endif
#define Hierarchy_Max(a,b) ((a)>(b)?(a):(b))
'''
    for result,name,args in DECLARATIONS:
        if name=='IsCompressed':body='return !!self->compressed;'
        else:
            old='CAnimBlendHierarchy::'+('CAnimBlendHierarchy' if name=='Init' else name);text=method(original,old);body=text[text.index('{')+1:text.rfind('}')]
            body=body.replace('this->name','self->name').replace('CAnimManager::RemoveFromUncompressedCache(this);','AnimHierarchy_HostRemoveCache(self);')
            for operation in ['RemoveAnimSequences','RemoveQuaternionFlips','CalcTotalTime']:body=re.sub(r'(?<![\w])'+operation+r'\(\)','AnimHierarchy_'+operation+'(self)',body)
            for field in ['sequences','numSequences','compressed','totalLength','linkPtr']:body=re.sub(r'(?<![\w.>])\b'+field+r'\b','self->'+field,body)
            body=body.replace('nil','NULL').replace('Max(','Hierarchy_Max(').replace('assert(','Hierarchy_Assert(')
        if name=='MoveMemory':source+='#ifdef USE_CUSTOM_ALLOCATOR\n'
        source+=result+' AnimHierarchy_'+name+'('+args+')\n{\n    // Preserve the complete original hierarchy policy and its live field reads\n'+body.strip('\n')+'\n}\n'
        if name=='MoveMemory':source+='#endif\n'
    return source+'//- rouz edit (ChatGPT)\n'

def anim_hierarchy_calls(source):
    # Remove the complete hierarchy class and translate every diagnosed native receiver
    original=source
    if 'class CAnimBlendHierarchy\n' in source:return header()
    if 'CAnimBlendHierarchy::CAnimBlendHierarchy(void)\n{' in source:return kernel(source)
    source=source.replace('class CAnimBlendHierarchy;','struct CAnimBlendHierarchy;')
    code=mask(source);pattern=r'((?:pAnimBlendAssoc|pNewAnim)->hierarchy|\b(?:lastHier|hier|anim)|ms_aAnimations\[(?:[^\[\]\n]|\[[^\]\n]*\])+\])(->|\.)([A-Za-z0-9_]+)\('
    for match in reversed(list(re.finditer(pattern,code))):
        if match[3] not in OPERATIONS:continue
        end=closing(code,match.end()-1);receiver=source[match.start(1):match.end(1)];args=source[match.end():end-1];receiver='&'+receiver if match[2]=='.' else receiver
        if match[3]=='MoveMemory' and not args.strip():args='false'
        source=source[:match.start()]+'AnimHierarchy_'+match[3]+'('+receiver+(',' if args.strip() else '')+args+')'+source[end:]
    if re.search(r'(?m)^CAnimManager::RemoveFromUncompressedCache\(',source) and 'AnimHierarchy_HostRemoveCache(' not in source:
        source=source.replace('#include "common.h"','#include "common.h"\n#include "AnimBlendHierarchy_backend.h"',1)+backend()
    if source==original:return source
    oldLines=set(original.splitlines());lines=[]
    for line in source.splitlines():
        if line not in oldLines and not line.lstrip().startswith('#') and 'AnimHierarchy_' in line and 'HostRemoveCache(' not in line:lines.append(re.match(r'\s*',line)[0]+'// Use the complete C hierarchy at the original native operation boundary')
        lines.append(line)
    source='\n'.join(lines)+'\n'
    return source if source.lstrip().startswith('//+ rouz edit (ChatGPT)') and source.rstrip().endswith('//- rouz edit (ChatGPT)') else '//+ rouz edit (ChatGPT)\n'+source.rstrip('\n')+'\n//- rouz edit (ChatGPT)\n'

def anim_hierarchy_project(source):
    # Compile the complete renamed hierarchy producer as C11 and register its cache boundary
    source=source.replace('..\\src\\animation\\AnimBlendHierarchy.cpp','..\\src\\animation\\AnimBlendHierarchy.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="\.\.\\src\\animation\\AnimBlendHierarchy\.c"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve unrelated source metadata while selecting C11 explicitly
            body=match[1] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="..\\src\\animation\\AnimBlendHierarchy.c">'+body+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    anchor='<ClInclude Include="..\\src\\animation\\AnimBlendHierarchy.h" />';entry='<ClInclude Include="..\\src\\animation\\AnimBlendHierarchy_backend.h" />'
    if entry not in source:source=source.replace(anchor,anchor+'\n    '+entry)
    return objects.project(source)



def calls(source):
    # Preserve complete hierarchy ownership before complete association ownership
    import anim_association_migration as clumps
    return clumps.calls(anim_hierarchy_calls(source))


def project(source):
    # Retain hierarchy integration before the renamed C11 association producer
    import anim_association_migration as clumps
    return clumps.project(anim_hierarchy_project(source))
