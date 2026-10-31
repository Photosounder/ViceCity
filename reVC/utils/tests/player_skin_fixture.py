"""Audit all player-skin operations and emit actual-owner original/C comparison programs."""
from pathlib import Path
import json,re
import player_skin_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/player-skin-c-tests'
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.project(old) if path.startswith('build/') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/renderer/PlayerSkinData.c').read_text()==m.kernel((out/'before/src/renderer/PlayerSkin.cpp').read_text())
assert (root/'src/renderer/PlayerSkin_backend.h').read_text()==m.BACKEND
for path in json.loads((out/'untouched.json').read_text()):assert (out/'before'/path).read_bytes()==(root/path).read_bytes(),path
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "config.h"
typedef uint8_t uint8;typedef uint32_t uint32;typedef int32_t int32;
typedef uint8_t RwUInt8;typedef uint32_t RwUInt32;typedef int32_t RwInt32;typedef int32_t RwBool;typedef char RwChar;typedef float RwReal;
#ifdef LIBRW
namespace rw {struct Texture{};struct Clump{};}
typedef rw::Texture RwTexture;typedef rw::Clump RpClump;
#else
struct RwTexture{};struct RpClump{};
#endif
struct RwRaster{};struct RwImage{};struct RwFrame{};struct RwCamera{};struct RwTexDictionary{};struct RpAtomic{};
struct RwV3d {float x,y,z;};struct RwRGBAReal {float red,green,blue,alpha;};
struct RwMatrix {RwV3d right;uint32_t flags;RwV3d up;uint32_t pad1;RwV3d at;uint32_t pad2;RwV3d pos;uint32_t pad3;};
class RwStream {public:virtual ~RwStream(){};virtual uint32 write8(const void *,uint32)=0;virtual uint32 read8(void *,uint32)=0;virtual void seek(int32,int32=1)=0;virtual uint32 tell(void)=0;virtual bool eof(void)=0;};
struct RwMemory {RwUInt8 *start;RwUInt32 length;};
enum RwStreamType {rwSTREAMMEMORY=3};enum RwStreamAccessType {rwSTREAMREAD=1};enum {rwID_CLUMP=16,rwRASTERTYPETEXTURE=4};
enum RwTextureFilterMode {rwFILTERLINEAR=2};enum RwOpCombineType {rwCOMBINEREPLACE=0,rwCOMBINEPRECONCAT=1};
typedef RpAtomic *(*RpAtomicCallBack)(RpAtomic *,void *);
RwStream *RwStreamOpen(RwStreamType,RwStreamAccessType,const void *);RwBool RwStreamClose(RwStream *,void *);RwBool RwStreamFindChunk(RwStream *,RwUInt32,RwUInt32 *,RwUInt32 *);
RpClump *RpClumpStreamRead(RwStream *);RpClump *RpClumpForAllAtomics(RpClump *,RpAtomicCallBack,void *);RwBool RpClumpDestroy(RpClump *);RpClump *RpClumpRender(RpClump *);RwFrame *RpClumpGetFrame(const RpClump *);
RwTexture *RwTextureRead(const char *,const char *);RwImage *RtBMPImageRead(const char *);RwImage *RwImageFindRasterFormat(RwImage *,RwInt32,RwInt32 *,RwInt32 *,RwInt32 *,RwInt32 *);
RwRaster *RwRasterCreate(RwInt32,RwInt32,RwInt32,RwInt32);RwRaster *RwRasterSetFromImage(RwRaster *,RwImage *);RwTexture *RwTextureCreate(RwRaster *);RwTexture *RwTextureSetName(RwTexture *,const char *);RwTexture *RwTextureSetFilterMode(RwTexture *,RwTextureFilterMode);RwTexture *RwTexDictionaryAddTexture(RwTexDictionary *,RwTexture *);RwBool RwImageDestroy(RwImage *);
RwFrame *RwCameraGetFrame(const RwCamera *);RwMatrix *RwFrameGetMatrix(RwFrame *);RwFrame *RwFrameTransform(RwFrame *,const RwMatrix *,RwOpCombineType);RwFrame *RwFrameTranslate(RwFrame *,const RwV3d *,RwOpCombineType);RwFrame *RwFrameRotate(RwFrame *,const RwV3d *,RwReal,RwOpCombineType);RwFrame *RwFrameUpdateObjects(RwFrame *);
#define nil NULL
#endif
'''
(out/'common.h').write_text(common,newline='\n')
owners='''#pragma once
#include "common.h"
#include "PlayerSkin_backend.h"
#include "Directory.h"
#include "FileMgr.h"
#include "Timer.h"
#include "Draw.h"
#include "Draw_backend.h"
#ifdef NATIVE_PROJECT_HEADERS
#include "rtbmp.h"
#endif
extern char gString[256];
struct TxdDef {RwTexDictionary *texDict;int refs;};
class CTxdStore {public:static int AddTxdSlot(const char *);static void Create(int);static void AddRef(int);static void RemoveTxdSlot(int);static void PushCurrentTxd(void);static void SetCurrentTxd(int);static void PopCurrentTxd(void);static TxdDef *GetSlot(int);};
class CClumpModelInfo {public:static RpAtomic *SetAtomicRendererCB(RpAtomic *,void *);};
class CVisibilityPlugins {public:static RpAtomic *RenderPlayerCB(RpAtomic *);};
class CPlayerInfo {public:uint64_t prefix[4];void LoadPlayerSkin(void);};
class CWorld {public:static CPlayerInfo Players[2];};
struct GlobalScene {void *world;RwCamera *camera;};extern GlobalScene Scene;
int32 CdStreamGetNumImages(void);bool CdStreamAddImage(const char *);int32 CdStreamRead(int32,void *,uint32,uint32);int32 CdStreamSync(int32);void CdStreamRemoveImages(void);
void *RwMallocAlign(RwUInt32,RwUInt32);void RwFreeAlign(void *);void SetAmbientColours(RwRGBAReal *);
'''
(out/'owners.h').write_text(owners,newline='\n')
for name in ['PlayerSkin.h','PlayerSkin_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
operations=[('Find','FindPlayerDff','void','uint32_t *offset,uint32_t *size','offset,size'),('Load','LoadPlayerDff','void','void','')]+[(name,name,'void *' if name=='GetSkinTexture' else 'void','const char *name' if name=='GetSkinTexture' else 'void','name' if name=='GetSkinTexture' else '') for name in m.METHODS]
prototypes='''int32_t TestSkin_Slot(void);void TestSkin_SetSlot(int32_t value);void *TestSkin_Clump(void);void TestSkin_SetClump(void *value);float *TestSkin_OldFov(void);
'''
for alias,method,result,args,invoke in operations:prototypes+=result+' TestSkin_'+alias+'('+args+');'+result+' TestSkin_Run'+alias+'('+args+');\n'
(out/'caller.h').write_text('#pragma once\n#include <stdint.h>\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
caller='#include "caller.h"\n'
for alias,method,result,args,invoke in operations:caller+=result+' TestSkin_Run'+alias+'('+args+')\n{\n    // Enter the complete actual operation through a separately compiled C11 caller\n    '+('return ' if result!='void' else '')+'TestSkin_'+alias+'('+invoke+');\n}\n'
(out/'caller.c').write_text(caller,newline='\n')
old=(out/'before/src/renderer/PlayerSkin.cpp').read_text();header=(out/'before/src/renderer/PlayerSkin.h').read_text()
for before in [True,False]:
    text='#include "owners.h"\n#include "caller.h"\n'
    if before:text+='namespace Original {\n'+header.replace('#pragma once','').replace('class CPlayerSkin\n{','class CPlayerSkin\n{\npublic:')+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    slot='Original::CPlayerSkin::m_txdSlot' if before else 'PlayerSkin_txdSlot';clump='Original::gpPlayerClump' if before else 'PlayerSkin_Clump';fov='Original::gOldFov' if before else 'PlayerSkin_OldFov'
    text+='extern "C" {\nint32_t TestSkin_Slot(void)\n{\n    // Observe the actual canonical signed dictionary slot\n    return '+slot+';\n}\nvoid TestSkin_SetSlot(int32_t value)\n{\n    // Mutate the actual dictionary slot during allowed native owner callbacks\n    '+slot+'=value;\n}\nvoid *TestSkin_Clump(void)\n{\n    // Observe the actual canonical native clump identity\n    return '+clump+';\n}\nvoid TestSkin_SetClump(void *value)\n{\n    // Mutate the actual canonical clump during allowed native owner callbacks\n    '+clump+'=(RpClump*)value;\n}\nfloat *TestSkin_OldFov(void)\n{\n    // Retain a pointer to the actual live saved FOV field\n    return &'+fov+';\n}\n'
    for alias,method,result,args,invoke in operations:
        prefix='Original::'+('CPlayerSkin::' if method in m.METHODS else '') if before else 'PlayerSkin_'
        if before and method=='FindPlayerDff':invoke='*offset,*size'
        text+=result+' TestSkin_'+alias+'('+args+')\n{\n    // Execute the complete actual original or C helper and controller operation\n    '+('return ' if result!='void' else '')+prefix+method+'('+invoke+');\n}\n'
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text+'}\n',newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',m.HOST,flags=re.M),newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/player_skin.cpp.in').read_text(),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all eight player-skin operations, canonical C state, 41 typed native boundaries and seven complete source/project migrations')
