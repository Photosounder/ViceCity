"""Prepare entire original/C owners and independent callers against actual COM SDK declarations."""
from pathlib import Path
import re

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/texture-pools-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <d3d8.h>
typedef struct PoolState {D3DFORMAT Format;int size;uint32_t levels;int32_t texturesMax,texturesUsed,texturesNum;IDirect3DTexture8 **pTextures;} PoolState;
typedef struct PaletteState {int Max,Num;int *Data;} PaletteState;
typedef struct PoolResult {void *pointer;int64_t value;} PoolResult;
enum {OP_Create,OP_Release,OP_Find,OP_Add,OP_Resize,OP_GetSize,OP_PalAlloc,OP_PalFree,OP_PalFind,OP_PalAdd,OP_PalResize,OP_CreateTexture,OP_ReleaseTexture,OP_FindIndex,OP_AddIndex,OP_Init,OP_Shutdown};
#ifdef __cplusplus
extern "C" {
#endif
void TestPools(unsigned operation,void *record,IDirect3DTexture8 *texture,int a,int b,int c,int d,PoolResult *result);
void *TestPoolRecord(bool palette);
void TestPoolDelete(void *record,bool palette);
void TestPoolState(void *record,PoolState *state);
void TestPaletteState(void *record,PaletteState *state);
void *TestGlobalPool(int index);
void *TestGlobalPalette(void);
void TestPoolTruncate(void *record,int count);
extern int numTexturePools,MaxPaletteIndex;
extern bool bUsePaletteIndex;
void *TestAlloc(size_t size);
void TestFree(void *memory);
#ifdef __cplusplus
}
#endif
'''
old='''case OP_Create:((CTexturePool*)record)->Create((D3DFORMAT)a,b,c,d);break;
case OP_Release:((CTexturePool*)record)->Release();break;
case OP_Find:result->pointer=((CTexturePool*)record)->FindTexture();break;
case OP_Add:result->value=((CTexturePool*)record)->AddTexture(texture);break;
case OP_Resize:((CTexturePool*)record)->Resize(a);break;
case OP_GetSize:result->value=(int64_t)((CTexturePool*)record)->GetSize();break;
case OP_PalAlloc:((CPaletteList*)record)->Alloc(a);break;
case OP_PalFree:((CPaletteList*)record)->Free();break;
case OP_PalFind:result->value=((CPaletteList*)record)->Find();break;
case OP_PalAdd:((CPaletteList*)record)->Add(a);break;
case OP_PalResize:((CPaletteList*)record)->Resize(a);break;
case OP_CreateTexture:result->value=CreateTexture(a,b,c,(D3DFORMAT)d,(IDirect3DTexture8**)record);break;
case OP_ReleaseTexture:ReleaseTexture(texture);break;
case OP_FindIndex:result->value=FindAvailablePaletteIndex();break;
case OP_AddIndex:AddAvailablePaletteIndex(a);break;
case OP_Init:_TexturePoolsInitialise();break;
case OP_Shutdown:_TexturePoolsShutdown();break;
'''
new='''case OP_Create:CTexturePool_Create((CTexturePool*)record,(D3DFORMAT)a,b,c,d);break;
case OP_Release:CTexturePool_Release((CTexturePool*)record);break;
case OP_Find:result->pointer=CTexturePool_FindTexture((CTexturePool*)record);break;
case OP_Add:result->value=CTexturePool_AddTexture((CTexturePool*)record,texture);break;
case OP_Resize:CTexturePool_Resize((CTexturePool*)record,a);break;
case OP_GetSize:result->value=(int64_t)CTexturePool_GetSize((CTexturePool*)record);break;
case OP_PalAlloc:CPaletteList_Alloc((CPaletteList*)record,a);break;
case OP_PalFree:CPaletteList_Free((CPaletteList*)record);break;
case OP_PalFind:result->value=CPaletteList_Find((CPaletteList*)record);break;
case OP_PalAdd:CPaletteList_Add((CPaletteList*)record,a);break;
case OP_PalResize:CPaletteList_Resize((CPaletteList*)record,a);break;
case OP_CreateTexture:result->value=TexturePools_CreateTexture(a,b,c,(D3DFORMAT)d,(IDirect3DTexture8**)record);break;
case OP_ReleaseTexture:TexturePools_ReleaseTexture(texture);break;
case OP_FindIndex:result->value=TexturePools_FindAvailablePaletteIndex();break;
case OP_AddIndex:TexturePools_AddAvailablePaletteIndex(a);break;
case OP_Init:TexturePools_Initialise();break;
case OP_Shutdown:TexturePools_Shutdown();break;
'''
original='#line 1 "TexturePools.cpp"\n'+(out/'before/src/rw/TexturePools.cpp').read_text().replace('#include "common.h"','#include <stdint.h>\n#include <stdlib.h>\ntypedef uint32_t uint32;typedef int32_t int32;\n#define nil NULL\n#define CITA_TLS thread_local\nextern "C" {\n#include <cita_windows.h>\n}')
beforeHeader='''#include <stdint.h>
typedef uint32_t uint32;typedef int32_t int32;
#include "original-api.h"
extern CTexturePool aTexturePools[12];
extern CPaletteList PaletteList;
HRESULT CreateTexture(int,int,int,D3DFORMAT,IDirect3DTexture8**);
void ReleaseTexture(IDirect3DTexture8*);
int FindAvailablePaletteIndex(void);
void AddAvailablePaletteIndex(int);
'''
for phase,cases in [('before',old),('after',new)]:
    # Enter every complete old C++ or new C operation through an independently compiled caller
    value='#include "entry.h"\n#include <stdlib.h>\n#include <string.h>\n'+(beforeHeader if phase=='before' else '#include "TexturePools.h"\n')
    value+='void TestPools(unsigned operation,void *record,IDirect3DTexture8 *texture,int a,int b,int c,int d,PoolResult *result)\n{\n    // Dispatch whole operations without implementing allocation, resize or pooling policy\n    result->pointer=NULL;result->value=0;switch(operation){\n'+cases+'    }\n}\n'
    value+='void *TestPoolRecord(bool palette)\n{\n    // Create independent caller-owned records and initialize every test field explicitly\n    void *record=' + ('(palette?(void*)new CPaletteList:(void*)new CTexturePool)' if phase=='before' else 'malloc(palette?sizeof(CPaletteList):sizeof(CTexturePool))')+';memset(record,0,palette?sizeof(CPaletteList):sizeof(CTexturePool));return record;\n}\n'
    value+='void TestPoolDelete(void *record,bool palette)\n{\n    // Release caller-owned record storage after each owner operation has released its buffers\n    '+('if(palette)delete (CPaletteList*)record;else delete (CTexturePool*)record;' if phase=='before' else '(void)palette;free(record);')+'\n}\n'
    value+='void TestPoolState(void *record,PoolState *state)\n{\n    // Observe initialized field bytes without aliasing an old class or new C record\n    memcpy(state,record,sizeof(*state));\n}\n'
    value+='void TestPaletteState(void *record,PaletteState *state)\n{\n    // Observe private original palette storage through its independently verified byte layout\n    memcpy(state,record,sizeof(*state));\n}\n'
    value+='void *TestGlobalPool(int index)\n{\n    // Borrow the original global pool record without changing its implicit zero-initialization boundary\n    return &aTexturePools[index];\n}\n'
    value+='void *TestGlobalPalette(void)\n{\n    // Borrow the original global palette record without copying it into shadow storage\n    return &PaletteList;\n}\n'
    value+='void TestPoolTruncate(void *record,int count)\n{\n    // Repair only caller-owned test metadata after observing the original unsafe shrink count\n    ((CTexturePool*)record)->texturesNum=count;\n}\n'
    (probe/(phase+'-caller.'+('cpp' if phase=='before' else 'c'))).write_text(value,newline='\n')
for name,value in [('entry.h',entry),('original-api.h',(out/'before/src/rw/TexturePools.h').read_text()),('original.cpp',original),('fixture.cpp',(root/'utils/tests/texture_pools_fixture.cpp.in').read_text())]:
    # Keep all original/C producer bodies complete and retain actual SDK C/C++ virtual-call declarations
    (probe/name).write_text(value,newline='\n')
print('Prepared all 17 complete pool operations, independent callers and real COM interface declarations')
