"""Remove both legacy texture-pool classes using the actual Direct3D C interface."""
import re
from rect_migration import mask
from memory_heap_migration import body

RENAMES={'src/rw/TexturePools.cpp':'src/rw/TexturePools.c'}
POOL_FIELDS=['Format','size','levels','texturesMax','texturesUsed','texturesNum','pTextures']
PALETTE_FIELDS=['Max','Num','Data']
POOL=[('void','Create','D3DFORMAT _Format,int _size,uint32_t mipmapLevels,int32_t numTextures','CTexturePool::Create('),('void','Release','void','CTexturePool::Release('),('IDirect3DTexture8 *','FindTexture','void','CTexturePool::FindTexture('),('bool','AddTexture','IDirect3DTexture8 *texture','CTexturePool::AddTexture('),('void','Resize','int numTextures','CTexturePool::Resize(')]
PALETTE=[('void','Alloc','int max','CPaletteList::Alloc('),('void','Free','void','CPaletteList::Free('),('int','Find','void','CPaletteList::Find('),('void','Add','int item','CPaletteList::Add('),('void','Resize','int max','CPaletteList::Resize(')]
GLOBALS={'CreateTexture':'TexturePools_CreateTexture','ReleaseTexture':'TexturePools_ReleaseTexture','FindAvailablePaletteIndex':'TexturePools_FindAvailablePaletteIndex','AddAvailablePaletteIndex':'TexturePools_AddAvailablePaletteIndex','_TexturePoolsInitialise':'TexturePools_Initialise','_TexturePoolsShutdown':'TexturePools_Shutdown'}

def header():
    # Preserve all original records and use SDK C-compatible COM declarations directly
    value='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifndef LIBRW
#include <d3d8.h>
typedef struct CTexturePool {
    D3DFORMAT Format;
    int size;
    uint32_t levels;
    int32_t texturesMax,texturesUsed,texturesNum;
    IDirect3DTexture8 **pTextures;
} CTexturePool;
typedef struct CPaletteList {int Max,Num;int *Data;} CPaletteList;
#ifdef __cplusplus
extern "C" {
#endif
extern CTexturePool aTexturePools[12];
extern CPaletteList PaletteList;
extern int numTexturePools,MaxPaletteIndex;
extern bool bUsePaletteIndex;
'''
    for owner,operations in [('CTexturePool',POOL),('CPaletteList',PALETTE)]:
        # Replace each implicit class receiver with one explicit C record pointer
        value+=''.join(result+' '+owner+'_'+name+'('+owner+' *self'+('' if args=='void' else ','+args)+');\n' for result,name,args,_ in operations)
    value+='''#ifdef FIX_BUGS
int CTexturePool_GetSize(const CTexturePool *self);
#else
float CTexturePool_GetSize(const CTexturePool *self);
#endif
HRESULT TexturePools_CreateTexture(int width,int height,int levels,D3DFORMAT Format,IDirect3DTexture8 **texture);
void TexturePools_ReleaseTexture(IDirect3DTexture8 *texture);
int TexturePools_FindAvailablePaletteIndex(void);
void TexturePools_AddAvailablePaletteIndex(int index);
void TexturePools_Initialise(void);
void TexturePools_Shutdown(void);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
'''
    return value

def com_calls(source):
    # Use the SDK's C COM macros with exactly the original receiver and arguments
    source=source.replace('pTextures[i]->Release()','IDirect3DTexture8_Release(self->pTextures[i])')
    source=source.replace('texture->Release()','IDirect3DTexture8_Release(texture)').replace('texture->GetLevelCount()','IDirect3DTexture8_GetLevelCount(texture)').replace('texture->GetLevelDesc(0, &SURFACE_DESC)','IDirect3DTexture8_GetLevelDesc(texture,0,&SURFACE_DESC)')
    source=source.replace('_RwD3DDevice->CreateTexture(width, height, levels, 0, Format, D3DPOOL_MANAGED, texture)','IDirect3DDevice8_CreateTexture(_RwD3DDevice,width,height,levels,0,Format,D3DPOOL_MANAGED,texture)')
    return source

def method_body(original,owner,name,signature,fields):
    # Retain complete original algorithms and qualify only implicit receiver fields and recursive calls
    value=body(original,signature)
    for field in fields:value=re.sub(r'\b'+field+r'\b','self->'+field,value)
    operations=POOL if owner=='CTexturePool' else PALETTE
    for _,operation,_,_ in operations:
        # Keep recursive palette insertion and resize calls in C
        value=re.sub(r'(?<![\w>.])'+operation+r'\(([^\n]*?)\)',lambda match:owner+'_'+operation+'(self'+(','+match[1] if match[1].strip() else '')+')',value)
    for old,new in [('uint32','uint32_t'),('int32','int32_t'),('nil','NULL')]:value=re.sub(r'\b'+old+r'\b',new,value)
    value=value.replace('self->pTextures[i]->Release()','IDirect3DTexture8_Release(self->pTextures[i])')
    return value

def kernel(original):
    # Preserve every whole original operation while removing class syntax and C++ COM calls
    value='''//+ rouz edit (ChatGPT)
#define COBJMACROS
#include "TexturePools.h"
#ifndef LIBRW
#include <stdlib.h>
#include <stddef.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
extern IDirect3DDevice8 *_RwD3DDevice;
CTexturePool aTexturePools[12];
CPaletteList PaletteList;
int numTexturePools;
int MaxPaletteIndex;
bool bUsePaletteIndex=true;
'''
    for owner,operations,fields in [('CTexturePool',POOL,POOL_FIELDS),('CPaletteList',PALETTE,PALETTE_FIELDS)]:
        # Retain all original allocation, free-list, resizing and integer-counter behavior
        for result,name,args,signature in operations:
            value+=result+'\n'+owner+'_'+name+'('+owner+' *self'+('' if args=='void' else ','+args)+')\n{\n    // Preserve the complete original operation with an explicit C receiver\n'+method_body(original,owner,name,signature,fields)+'\n}\n'
    value+='''#ifdef FIX_BUGS
int
#else
float
#endif
CTexturePool_GetSize(const CTexturePool *self)
{
    // Preserve the original configuration-dependent getter return type
    return self->size;
}
'''
    start=original.index('HRESULT\nCreateTexture(');end=original.rindex('#endif // !LIBRW');tail=original[start:end]
    for name in ['Create','Release','FindTexture','AddTexture','Resize','GetSize']:
        # Convert whole global pool expressions without changing their evaluation or control flow
        tail=re.sub(r'\b(aTexturePools\[[^\]]+\])\.'+name+r'\(([^\n]*?)\)',lambda match:'CTexturePool_'+name+'(&'+match[1]+(','+match[2] if match[2].strip() else '')+')',tail)
    for _,name,_,_ in PALETTE:tail=re.sub(r'\bPaletteList\.'+name+r'\(([^\n]*?)\)',lambda match:'CPaletteList_'+name+'(&PaletteList'+(','+match[1] if match[1].strip() else '')+')',tail)
    for old,new in GLOBALS.items():tail=re.sub(r'(?<![\w>.])'+old+r'\b',new,tail)
    tail=com_calls(tail)
    for name in GLOBALS.values():
        # Describe each complete global operation before its original body
        tail=re.sub(r'(?m)^('+name+r'\([^\n]*\)\n\{\n)',lambda match:match[1]+'    // Preserve the complete original pool policy and native COM call order\n',tail)
    tail=re.sub(r'(?m)^(TexturePools_(?:Initialise|Shutdown|FindAvailablePaletteIndex))\(\)',r'\1(void)',tail)
    return value+tail+'\n#endif\n//- rouz edit (ChatGPT)\n'

def texture_pools_calls(source):
    # Replace both complete classes and every existing global client
    if 'CTexturePool::Create(' in source:return kernel(source)
    if 'class CTexturePool\n' in source and 'class CPaletteList\n' in source:return header()
    original=source;code=mask(source);changes=[]
    for match in re.finditer(r'\b(?:'+'|'.join(GLOBALS)+r')\b',code):changes.append((match.start(),match.end(),GLOBALS[match[0]]))
    for start,end,value in reversed(changes):source=source[:start]+value+source[end:]
    if source!=original:
        # Describe each client transition inside its original function body
        lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True)
        for index in reversed(range(len(lines))):
            if re.search(r'\bTexturePools_\w+',code[index]):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Use the complete C texture and palette pool owner\n')
        return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'
    return source

def texture_pools_project(source):
    # Compile the whole pool owner as C11 without adding a C++ SDK bridge
    import object_names_migration as objects
    source=source.replace('src\\rw\\TexturePools.cpp','src\\rw\\TexturePools.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\TexturePools\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain preceding per-file metadata and require the C compiler for all pool algorithms
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            return '<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>'
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Retain complete C texture/palette ownership before applying the complete motion-blur successor
    import mblur_migration as blur
    return blur.calls(texture_pools_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting C blur policy and its typed SDK primitive unit
    import mblur_migration as blur
    return blur.project(texture_pools_project(source))
