"""Move the complete player-skin controller and both loader helpers into C11."""
import re
from audio_controls_migration import function
import var_console_migration as variables
METHODS=['Initialise','Shutdown','GetSkinTexture','BeginFrontendSkinEdit','EndFrontendSkinEdit','RenderFrontendSkinEdit']
OBSERVER=None
HEADER=r'''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#define DEFAULT_SKIN_NAME "$$\"\""
#ifdef __cplusplus
#ifdef LIBRW
namespace rw { struct Texture;struct Clump; }
typedef rw::Texture PlayerSkinTexture;
typedef rw::Clump PlayerSkinClump;
#else
typedef struct RwTexture PlayerSkinTexture;
typedef struct RpClump PlayerSkinClump;
#endif
extern "C" {
#else
typedef struct PlayerSkinTexture PlayerSkinTexture;
typedef struct PlayerSkinClump PlayerSkinClump;
#endif
extern PlayerSkinClump *PlayerSkin_Clump;
extern float PlayerSkin_OldFov;
extern int32_t PlayerSkin_txdSlot;
void PlayerSkin_FindPlayerDff(uint32_t *offset,uint32_t *size);
void PlayerSkin_LoadPlayerDff(void);
void PlayerSkin_Initialise(void);
void PlayerSkin_Shutdown(void);
PlayerSkinTexture *PlayerSkin_GetSkinTexture(const char *name);
void PlayerSkin_BeginFrontendSkinEdit(void);
void PlayerSkin_EndFrontendSkinEdit(void);
void PlayerSkin_RenderFrontendSkinEdit(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
READERS=[
('int','Compare','const char *a,const char *b','return strcasecmp(a,b);'),
('int32_t','ImageCount','void','return CdStreamGetNumImages();'),
('void','AddImage','const char *path','CdStreamAddImage(path);'),
('void','ReadSectors','uint8_t *buffer,uint32_t offset,uint32_t size','CdStreamRead(0,buffer,offset,size);'),
('void','Sync','void','CdStreamSync(0);'),
('void','RemoveImages','void','CdStreamRemoveImages();'),
('uint8_t *','Allocate','uint32_t size,uint32_t alignment','return (uint8_t*)RwMallocAlign(size,alignment);'),
('void','Free','uint8_t *buffer','RwFreeAlign(buffer);'),
('void *','OpenStream','void *memory','return RwStreamOpen(rwSTREAMMEMORY,rwSTREAMREAD,memory);'),
('bool','FindChunk','void *stream','return RwStreamFindChunk((RwStream*)stream,rwID_CLUMP,NULL,NULL)!=0;'),
('PlayerSkinClump *','ReadClump','void *stream','return RpClumpStreamRead((RwStream*)stream);'),
('void','CloseStream','void *stream,void *memory','RwStreamClose((RwStream*)stream,memory);'),
('int32_t','AddSlot','const char *name','return CTxdStore::AddTxdSlot(name);'),
('void','CreateDictionary','int32_t slot','CTxdStore::Create(slot);'),
('void','AddRef','int32_t slot','CTxdStore::AddRef(slot);'),
('void','RemoveSlot','int32_t slot','CTxdStore::RemoveTxdSlot(slot);'),
('void','PushDictionary','void','CTxdStore::PushCurrentTxd();'),
('void','SetDictionary','int32_t slot','CTxdStore::SetCurrentTxd(slot);'),
('void','PopDictionary','void','CTxdStore::PopCurrentTxd();'),
('PlayerSkinTexture *','ReadTexture','const char *name','return RwTextureRead(name,NULL);'),
('char *','String','void','return gString;'),
('void *','ReadImage','const char *path','return RtBMPImageRead(path);'),
('void','RasterFormat','void *image,int32_t *width,int32_t *height,int32_t *depth,int32_t *format','RwImageFindRasterFormat((RwImage*)image,rwRASTERTYPETEXTURE,width,height,depth,format);'),
('void *','CreateRaster','int32_t width,int32_t height,int32_t depth,int32_t format','return RwRasterCreate(width,height,depth,format);'),
('void','RasterImage','void *raster,void *image','RwRasterSetFromImage((RwRaster*)raster,(RwImage*)image);'),
('PlayerSkinTexture *','CreateTexture','void *raster','return RwTextureCreate((RwRaster*)raster);'),
('void','TextureName','PlayerSkinTexture *texture,const char *name','RwTextureSetName(texture,name);'),
('void','TextureFilter','PlayerSkinTexture *texture','RwTextureSetFilterMode(texture,rwFILTERLINEAR);'),
('void','AddTexture','int32_t slot,PlayerSkinTexture *texture','RwTexDictionaryAddTexture(CTxdStore::GetSlot(slot)->texDict,texture);'),
('void','DestroyImage','void *image','RwImageDestroy((RwImage*)image);'),
('void','SetRenderers','PlayerSkinClump *clump','RpClumpForAllAtomics(clump,CClumpModelInfo::SetAtomicRendererCB,(void*)CVisibilityPlugins::RenderPlayerCB);'),
('void','LoadPlayer','void','CWorld::Players[0].LoadPlayerSkin();'),
('void','DestroyClump','PlayerSkinClump *clump','RpClumpDestroy(clump);'),
('void *','Frame','PlayerSkinClump *clump','return RpClumpGetFrame(clump);'),
('void','Transform','void *frame','RwFrameTransform((RwFrame*)frame,RwFrameGetMatrix(RwCameraGetFrame(Scene.camera)),rwCOMBINEREPLACE);'),
('void','Translate','void *frame,PlayerSkinVector value','const RwV3d position={value.x,value.y,value.z};RwFrameTranslate((RwFrame*)frame,&position,rwCOMBINEPRECONCAT);'),
('void','Rotate','void *frame,PlayerSkinVector value,float angle','const RwV3d axis={value.x,value.y,value.z};RwFrameRotate((RwFrame*)frame,&axis,angle,rwCOMBINEPRECONCAT);'),
('void','UpdateObjects','void *frame','RwFrameUpdateObjects((RwFrame*)frame);'),
('void','Ambient','PlayerSkinColor value','RwRGBAReal color={value.red,value.green,value.blue,value.alpha};SetAmbientColours(&color);'),
('void','RenderClump','PlayerSkinClump *clump','RpClumpRender(clump);')]
BACKEND='''//+ rouz edit (ChatGPT)
#pragma once
#include "PlayerSkin.h"
#include <stdbool.h>
typedef struct PlayerSkinVector {float x,y,z;} PlayerSkinVector;
typedef struct PlayerSkinColor {float red,green,blue,alpha;} PlayerSkinColor;
#ifdef __cplusplus
extern "C" {
#endif
typedef void (*PlayerSkinMemoryCallback)(void *memory);
'''+''.join(result+' PlayerSkin_Backend'+name+'('+args+');\n' for result,name,args,body in READERS)+'''void PlayerSkin_BackendWithMemory(uint8_t *buffer,uint32_t length,PlayerSkinMemoryCallback callback);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
HOST='''//+ rouz edit (ChatGPT)
#include "common.h"
#include "PlayerSkin_backend.h"
#include "main.h"
#include "TxdStore.h"
#include "rtbmp.h"
#include "ClumpModelInfo.h"
#include "VisibilityPlugins.h"
#include "World.h"
#include "PlayerInfo.h"
#include "CdStream.h"
#include "RwHelper.h"
#include "Lights.h"
#include "MemoryMgr.h"
'''+''.join(result+' PlayerSkin_Backend'+name+'('+args+')\n{\n    // Retain the original typed native operation and its borrowed owner arguments\n    '+body+'\n}\n' for result,name,args,body in READERS)+'''void PlayerSkin_BackendWithMemory(uint8_t *buffer,uint32_t length,PlayerSkinMemoryCallback callback)
{
    // Keep the same real native memory object alive through stream open, chunk loading and stream close
    RwMemory memory;memory.start=buffer;memory.length=length;callback(&memory);
}
//- rouz edit (ChatGPT)
'''

def kernel(source):
    # Relocate every complete original helper and controller operation into the C owner
    chunks=[function(source,'','FindPlayerDff'),function(source,'','LoadPlayerDff')]+[function(source,'CPlayerSkin::',name) for name in METHODS]
    load=chunks[1];a=load.index('\tmem.start = buffer;');b=load.index('\n\tRwFreeAlign(buffer);',a)
    scope=load[a:b];assert scope.startswith('\tmem.start = buffer;\n\tmem.length = size << 11;\n')
    scope=scope.split('\n',2)[2].replace('\tstream =','\tvoid *stream =',1).replace('&mem','memory')
    chunks[1]=load[:a]+'\tPlayerSkin_BackendWithMemory(buffer,size << 11,PlayerSkin_ReadMemory);\n'+load[b:]
    chunks[1]=chunks[1].replace('\tRwStream *stream;\n\tRwMemory mem;\n','')
    chunks.insert(1,'static void\nPlayerSkin_ReadMemory(void *memory)\n{\n    // Run the complete original stream sequence while the native memory owner remains alive\n'+scope+'\n}')
    body='\n\n'.join(chunks).replace('CPlayerSkin::','PlayerSkin_')
    body=body.replace('FindPlayerDff(uint32 &offset, uint32 &size)','PlayerSkin_FindPlayerDff(uint32_t *offset, uint32_t *size)')
    body=body.replace('offset = info.offset;','*offset = info.offset;').replace('size = info.size;','*size = info.size;')
    body=body.replace('FindPlayerDff(offset, size);','PlayerSkin_FindPlayerDff(&offset, &size);').replace('LoadPlayerDff(', 'PlayerSkin_LoadPlayerDff(')
    body=body.replace('gpPlayerClump','PlayerSkin_Clump').replace('gOldFov','PlayerSkin_OldFov').replace('m_txdSlot','PlayerSkin_txdSlot')
    body=body.replace('strcasecmp(', 'PlayerSkin_BackendCompare(')
    replacements={
    'CdStreamGetNumImages()':'PlayerSkin_BackendImageCount()',
    'CdStreamAddImage(':'PlayerSkin_BackendAddImage(',
    'CdStreamRead(0, buffer,':'PlayerSkin_BackendReadSectors(buffer,',
    'CdStreamSync(0)':'PlayerSkin_BackendSync()',
    'CdStreamRemoveImages()':'PlayerSkin_BackendRemoveImages()',
    '(uint8*)RwMallocAlign(':'PlayerSkin_BackendAllocate(',
    'RwFreeAlign(':'PlayerSkin_BackendFree(',
    'RwStreamOpen(rwSTREAMMEMORY, rwSTREAMREAD, memory)':'PlayerSkin_BackendOpenStream(memory)',
    'RwStreamFindChunk(stream, rwID_CLUMP, nil, nil)':'PlayerSkin_BackendFindChunk(stream)',
    'RpClumpStreamRead(stream)':'PlayerSkin_BackendReadClump(stream)',
    'RwStreamClose(stream, memory)':'PlayerSkin_BackendCloseStream(stream,memory)',
    'CTxdStore::AddTxdSlot(':'PlayerSkin_BackendAddSlot(',
    'CTxdStore::Create(':'PlayerSkin_BackendCreateDictionary(',
    'CTxdStore::AddRef(':'PlayerSkin_BackendAddRef(',
    'CTxdStore::RemoveTxdSlot(':'PlayerSkin_BackendRemoveSlot(',
    'CTxdStore::PushCurrentTxd()':'PlayerSkin_BackendPushDictionary()',
    'CTxdStore::SetCurrentTxd(':'PlayerSkin_BackendSetDictionary(',
    'CTxdStore::PopCurrentTxd()':'PlayerSkin_BackendPopDictionary()',
    'RwTextureRead(texName, NULL)':'PlayerSkin_BackendReadTexture(texName)',
    'gString':'PlayerSkin_BackendString()',
    'if (RwImage *image = RtBMPImageRead(PlayerSkin_BackendString())) {':'void *image = PlayerSkin_BackendReadImage(PlayerSkin_BackendString());\n\tif (image) {',
    'RwImageFindRasterFormat(image, rwRASTERTYPETEXTURE,':'PlayerSkin_BackendRasterFormat(image,',
    'RwRasterCreate(':'PlayerSkin_BackendCreateRaster(',
    'RwRasterSetFromImage(':'PlayerSkin_BackendRasterImage(',
    'RwTextureCreate(':'PlayerSkin_BackendCreateTexture(',
    'RwTextureSetName(':'PlayerSkin_BackendTextureName(',
    'RwTextureSetFilterMode(tex, rwFILTERLINEAR)':'PlayerSkin_BackendTextureFilter(tex)',
    'RwTexDictionaryAddTexture(CTxdStore::GetSlot(PlayerSkin_txdSlot)->texDict, tex)':'PlayerSkin_BackendAddTexture(PlayerSkin_txdSlot,tex)',
    'RwImageDestroy(':'PlayerSkin_BackendDestroyImage(',
    'RpClumpForAllAtomics(PlayerSkin_Clump, CClumpModelInfo::SetAtomicRendererCB, (void*)CVisibilityPlugins::RenderPlayerCB)':'PlayerSkin_BackendSetRenderers(PlayerSkin_Clump)',
    'CWorld::Players[0].LoadPlayerSkin()':'PlayerSkin_BackendLoadPlayer()',
    'RpClumpDestroy(':'PlayerSkin_BackendDestroyClump(',
    'RpClumpGetFrame(':'PlayerSkin_BackendFrame(',
    'RwFrameTransform(frame, RwFrameGetMatrix(RwCameraGetFrame(Scene.camera)), rwCOMBINEREPLACE)':'PlayerSkin_BackendTransform(frame)',
    'RwFrameTranslate(frame, &pos, rwCOMBINEPRECONCAT)':'PlayerSkin_BackendTranslate(frame,pos)',
    'RwFrameRotate(frame, &axis, rotation, rwCOMBINEPRECONCAT)':'PlayerSkin_BackendRotate(frame,axis,rotation)',
    'RwFrameUpdateObjects(':'PlayerSkin_BackendUpdateObjects(',
    'SetAmbientColours(&AmbientColor)':'PlayerSkin_BackendAmbient(AmbientColor)',
    'RpClumpRender(':'PlayerSkin_BackendRenderClump('}
    for old,new in replacements.items():body=body.replace(old,new)
    for old,new in [('RwTexture','PlayerSkinTexture'),('RwRaster','void'),('RwFrame','void'),('RwRGBAReal','PlayerSkinColor'),('RwV3d','PlayerSkinVector'),('uint32','uint32_t'),('uint8','uint8_t'),('int32','int32_t'),('nil','NULL')]:body=re.sub(r'\b'+old+r'\b',new,body)
    body=re.sub(r'(?m)^((?:void|PlayerSkinTexture\s*\*)\nPlayerSkin_\w+\([^\n]*\)\n\{\n)',r'\1    // Preserve the complete original state writes, native call order and branch decisions\n',body)
    body=re.sub(r'(?m)^(\t+)([^\n]*\{\n)',r'\1\2\1\t// Preserve the original conditional operation and traversal order\n',body)
    code=re.sub(r'//[^\n]*|"(?:\\.|[^"\\])*"','',body)
    assert not re.search(r'\b(?:CPlayerSkin|CTxdStore|CWorld|CClumpModelInfo|CVisibilityPlugins|RwMemory|RwStream|RwImage|std)\b|\bRw\w+\(',code),code
    return '''//+ rouz edit (ChatGPT)
#include "PlayerSkin_backend.h"
#include "FileMgr.h"
#include "Directory.h"
#include "Timer.h"
#include "Draw.h"
#include <stdio.h>
#include <string.h>
PlayerSkinClump *PlayerSkin_Clump;
float PlayerSkin_OldFov;
int32_t PlayerSkin_txdSlot;

'''+body+'\n//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace the whole controller boundary and every real caller without changing its arguments
    if 'CPlayerSkin' not in source:return variables.calls(source)
    if 'int CPlayerSkin::m_txdSlot;' in source:
        if OBSERVER:OBSERVER(source)
        return variables.calls(HOST)
    if 'class CPlayerSkin\n' in source:return variables.calls(HEADER)
    lines=[]
    for line in source.splitlines(keepends=True):
        changed=line.replace('CPlayerSkin::','PlayerSkin_')
        if changed!=line:changed=changed.rstrip('\n')+' // rouz edit (ChatGPT)\n'
        lines.append(changed)
    return variables.calls(''.join(lines))

def project(source):
    # Register the unique C11 kernel and native header while preserving all paired-source object paths
    entry='<ClCompile Include="..\\src\\renderer\\PlayerSkin.cpp" />'
    if '..\\src\\renderer\\PlayerSkinData.c"' not in source:source=source.replace(entry,entry+'\n    <ClCompile Include="..\\src\\renderer\\PlayerSkinData.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>')
    entry='<ClInclude Include="..\\src\\renderer\\PlayerSkin.h" />'
    if '..\\src\\renderer\\PlayerSkin_backend.h"' not in source:source=source.replace(entry,entry+'\n    <ClInclude Include="..\\src\\renderer\\PlayerSkin_backend.h" />')
    return variables.project(source)

def verify_legacy(source,transform,root):
    # Keep complete historical caller evidence for both the native boundary and every relocated C algorithm
    global OBSERVER
    captured=[];saved=OBSERVER;OBSERVER=captured.append
    try:expected=transform(source)
    finally:OBSERVER=saved
    assert len(captured)==1
    normalize=lambda value:re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*[\s\S]*?\*/','',value))
    assert normalize(expected)==normalize((root/'src/renderer/PlayerSkin.cpp').read_text())
    assert normalize(kernel(captured[0]))==normalize((root/'src/renderer/PlayerSkinData.c').read_text())
