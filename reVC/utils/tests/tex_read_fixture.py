"""Prepare whole original/C texture readers with actual C directory, rectangle and string code."""
from pathlib import Path
import re
import tex_read_migration as m

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/tex-read-c-tests';probe=out/'runtime';probe.mkdir(exist_ok=True)
owners='''#pragma once
#define FORCE_PC_SCALING
#include "config.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include "Rect.h"
#include "Directory.h"
#include "TxdStore.h"
#include "FileMgr.h"
#include "Timer.h"
#include "Text.h"
#include "Draw.h"
#include "Draw_constants.h"
#include "StreamingInfo.h"
#include "Streaming_constants.h"
#include "CdStream_constants.h"
#include "crossplatform_constants.h"
#include "FontStyles.h"
typedef uint32_t uint32,RwUInt32;
typedef int32_t int32,RwInt32;
typedef int8_t int8;
typedef uint8_t uint8;
typedef int RwBool,RwRasterType;
#define nil NULL
#ifdef _MSC_VER
#define CITA_TLS __declspec(thread)
#else
#define CITA_TLS __thread
#endif
extern "C" {
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
void re3_assert(const char*,const char*,unsigned int,const char*);
}
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
struct RwTexDictionary;
struct RwTexture {unsigned id;bool alive;int anisotropy;RwTexDictionary *dictionary;};
struct RwTexDictionary {unsigned id;bool alive;unsigned count;RwTexture *textures[32];int imageSlot;};
struct RwStream {
    unsigned id;int32_t position,total,nextTexture,failNative,failChunk;
    bool structChunk,shortRead,nullTexture;
    struct {struct {uint32_t position;} memory;struct {void *fpFile;} file;} Type;
    int32_t tell(void);
};
typedef RwTexture *(*RwTextureCallBack)(RwTexture*,void*);
enum {rwID_TEXTURENATIVE=33,rwID_STRUCT=44,rwSTREAMFILENAME=1,rwSTREAMWRITE=2,rwRENDERSTATETEXTUREADDRESS=3,rwTEXTUREADDRESSCLAMP=4,GS_INIT_PLAYING_GAME=9};
enum {rwRASTERTYPETEXTURE=1,rwRASTERFORMAT8888=16,rwRASTERFORMAT888=32,rwRASTERFORMAT1555=64,rwRASTERFORMATPAL8=128,D3DFMT_DXT1=1,D3DFMT_DXT3=3,rwSTANDARDNATIVETEXTUREREAD=0};
struct FakeRwGlobal {RwBool (*stdFunc[1])(RwStream*,RwTexture**,uint32_t);};
extern FakeRwGlobal FakeGlobal;
#define RWSRCGLOBAL(value) (FakeGlobal.value)
struct RwFileFunctions {long (*rwftell)(void*);};
RwFileFunctions *RwOsGetFileInterface(void);
struct CRGBA {uint8_t r,g,b,a;CRGBA(uint8_t r,uint8_t g,uint8_t b,uint8_t a):r(r),g(g),b(b),a(a){}};
class CSprite2d {public:static void SetRecipNearClip(void);static void InitPerFrame(void);static void DrawRect(const CRect&,const CRGBA&);void Draw(const CRect&,const CRGBA&);};
class CFont {public:static void InitPerFrame(void);static void SetBackgroundOff(void);static void SetPropOn(void);static void SetScale(float,float);static void SetCentreOff(void);static void SetWrapx(float);static void SetJustifyOff(void);static void SetColor(const CRGBA&);static void SetBackGroundOnlyTextOff(void);static void SetFontStyle(int16_t);static void PrintString(float,float,uint16_t*);static void DrawFonts(void);};
class CPad {public:static void UpdatePads(void);static CPad *GetPad(int);bool GetEscapeJustDown(void);};
class CStreaming {public:static void FlushRequestList(void);static bool IsObjectInCdImage(int32_t);static void RequestTxd(int32_t,int32_t);static void RequestModelStream(int32_t);static void FlushChannels(void);static void RemoveTxd(int32_t);};
struct FakeFrontend {int8_t m_PrefsVsync,m_PrefsFrameLimiter;};
struct FakeRsGlobal {bool quit;int width,height;};
extern FakeFrontend FrontEndMenuManager;
extern FakeRsGlobal RsGlobal;
extern int gGameState;
extern int64_t _dwOperatingSystemVersion;
namespace rw {extern uint32_t platform;namespace gl3 {struct Caps {uint32_t gles,dxtSupported;};extern Caps gl3Caps;extern bool needToReadBackTextures;}}
#define SCREEN_WIDTH ((float)RsGlobal.width)
#define SCREEN_HEIGHT ((float)RsGlobal.height)
#define SCREEN_STRETCH_X(a) ((a) * (float) SCREEN_WIDTH / DEFAULT_SCREEN_WIDTH)
#define SCREEN_STRETCH_Y(a) ((a) * (float) SCREEN_HEIGHT / DEFAULT_SCREEN_HEIGHT)
#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a) * DEFAULT_ASPECT_RATIO / Draw_GetAspectRatio())
#else
#define SCREEN_SCALE_AR(a) (a)
#endif
#define SCREEN_SCALE_X(a) SCREEN_SCALE_AR(SCREEN_STRETCH_X(a))
#define SCREEN_SCALE_Y(a) SCREEN_STRETCH_Y(a)
#define SCREEN_SCALE_FROM_RIGHT(a) (SCREEN_WIDTH - SCREEN_SCALE_X(a))
#ifdef LIBRW
#define STREAMPOS(stream) ((stream)->tell())
#else
#define STREAMPOS(stream) ((stream)->Type.memory.position)
#endif
RwBool RwStreamFindChunk(RwStream*,uint32_t,uint32_t*,uint32_t*);
uint32_t RwStreamRead(RwStream*,void*,uint32_t);
RwBool rwNativeTextureHackRead(RwStream*,RwTexture**,int32_t);
RwTexDictionary *RwTexDictionaryCreate(void);
RwBool RwTexDictionaryDestroy(RwTexDictionary*);
RwTexture *RwTexDictionaryAddTexture(RwTexDictionary*,RwTexture*);
RwTexDictionary *RwTexDictionaryForAllTextures(RwTexDictionary*,RwTextureCallBack,void*);
RwBool RwTextureDestroy(RwTexture*);
RwStream *RwStreamSkip(RwStream*,uint32_t);
RwStream *RwStreamOpen(int,int,const char*);
RwBool RwStreamClose(RwStream*,void*);
RwStream *RwStreamWrite(RwStream*,const void*,uint32_t);
const RwTexDictionary *RwTexDictionaryStreamWrite(const RwTexDictionary*,RwStream*);
int32_t RpAnisotGetMaxSupportedMaxAnisotropy(void);
void RpAnisotTextureSetMaxAnisotropy(RwTexture*,int32_t);
extern "C" RwInt32 _rwD3D8FindCorrectRasterFormat(RwRasterType,RwInt32);
extern "C" RwBool _rwD3D8CheckValidTextureFormat(RwInt32);
void HandleExit(void);
CSprite2d *LoadSplash(const char*);
bool DoRWStuffStartOfFrame(int,int,int,int,int,int,int);
void DefinedState(void);
RwBool RwRenderStateSet(int,void*);
void DoRWStuffEndOfFrame(void);
void LoadingScreen(const char*,const char*,const char*);
#ifdef ORIGINAL
extern float texLoadTime;
extern int32 texNumLoaded;
RwTexture *RwTextureGtaStreamRead(RwStream*);
RwTexture *destroyTexture(RwTexture*,void*);
RwTexDictionary *RwTexDictionaryGtaStreamRead(RwStream*);
RwTexDictionary *RwTexDictionaryGtaStreamRead1(RwStream*);
RwTexDictionary *RwTexDictionaryGtaStreamRead2(RwStream*,RwTexDictionary*);
#ifdef LIBRW
#include "original-caps.h"
void ReadVideoCardCapsFile(GPUcaps*);
#else
void ReadVideoCardCapsFile(uint32&,uint32&,uint32&,uint32&);
#endif
bool CheckVideoCardCaps(void);
void WriteVideoCardCapsFile(void);
bool CanVideoCardDoDXT(void);
void ConvertingTexturesScreen(uint32,uint32,const char*);
void DealWithTxdWriteError(uint32,uint32,const char*);
bool CreateTxdImageForVideoCard(void);
#else
#include "TexRead.h"
#endif
'''
entry='''#pragma once
#include <stdint.h>
typedef struct TexResult {void *pointer;int number;} TexResult;
enum {OP_Texture,OP_Destroy,OP_Dictionary,OP_Dictionary1,OP_Dictionary2,OP_ReadCaps,OP_CheckCaps,OP_WriteCaps,OP_DXT,OP_Screen,OP_Error,OP_Image,OP_Binding,OP_Binding1,OP_Binding2};
#ifdef __cplusplus
extern "C" {
#endif
TexResult TestTex(unsigned operation,void *a,void *b,uint32_t num,uint32_t count,const char *text);
#ifdef __cplusplus
}
#endif
'''
before='''    case OP_Texture:result.pointer=RwTextureGtaStreamRead((RwStream*)a);break;
    case OP_Destroy:result.pointer=destroyTexture((RwTexture*)a,b);break;
    case OP_Dictionary:result.pointer=RwTexDictionaryGtaStreamRead((RwStream*)a);break;
    case OP_Dictionary1:result.pointer=RwTexDictionaryGtaStreamRead1((RwStream*)a);break;
    case OP_Dictionary2:result.pointer=RwTexDictionaryGtaStreamRead2((RwStream*)a,(RwTexDictionary*)b);break;
    case OP_ReadCaps:{
#ifdef LIBRW
        GPUcaps caps;ReadVideoCardCapsFile(&caps);memcpy(a,&caps,sizeof(caps));
#else
        uint32_t *caps=(uint32_t*)a;ReadVideoCardCapsFile(caps[0],caps[1],caps[2],caps[3]);
#endif
        break;}
    case OP_CheckCaps:result.number=CheckVideoCardCaps();break;
    case OP_WriteCaps:WriteVideoCardCapsFile();break;
    case OP_DXT:result.number=CanVideoCardDoDXT();break;
    case OP_Screen:ConvertingTexturesScreen(num,count,text);break;
    case OP_Error:DealWithTxdWriteError(num,count,text);break;
    case OP_Image:result.number=CreateTxdImageForVideoCard();break;
    case OP_Binding:result.pointer=TxdStore_HostReadDictionary(a);break;
    case OP_Binding1:result.pointer=TxdStore_HostReadDictionary1(a);break;
    case OP_Binding2:result.pointer=TxdStore_HostReadDictionary2(a,b);break;
'''
after='''    case OP_Texture:result.pointer=TexRead_ReadTexture(a);break;
    case OP_Destroy:result.pointer=TexRead_DestroyTexture(a,b);break;
    case OP_Dictionary:result.pointer=TexRead_ReadDictionary(a);break;
    case OP_Dictionary1:result.pointer=TexRead_ReadDictionary1(a);break;
    case OP_Dictionary2:result.pointer=TexRead_ReadDictionary2(a,b);break;
    case OP_ReadCaps:{
#ifdef LIBRW
        GPUcaps caps;TexRead_ReadVideoCardCapsFile(&caps);memcpy(a,&caps,sizeof(caps));
#else
        uint32_t *caps=(uint32_t*)a;TexRead_ReadVideoCardCapsFile(&caps[0],&caps[1],&caps[2],&caps[3]);
#endif
        break;}
    case OP_CheckCaps:result.number=TexRead_CheckVideoCardCaps();break;
    case OP_WriteCaps:TexRead_WriteVideoCardCapsFile();break;
    case OP_DXT:result.number=TexRead_CanVideoCardDoDXT();break;
    case OP_Screen:TexRead_ConvertingTexturesScreen(num,count,text);break;
    case OP_Error:TexRead_DealWithTxdWriteError(num,count,text);break;
    case OP_Image:result.number=TexRead_CreateTxdImageForVideoCard();break;
    case OP_Binding:result.pointer=TxdStore_HostReadDictionary(a);break;
    case OP_Binding1:result.pointer=TxdStore_HostReadDictionary1(a);break;
    case OP_Binding2:result.pointer=TxdStore_HostReadDictionary2(a,b);break;
'''
for name,cases,prefix in [('before-caller.cpp',before,'#include "owners.h"\n#include "TxdStore_backend.h"\n'),('after-caller.c',after,'#include "TexRead.h"\n#include "TxdStore_backend.h"\n#include <string.h>\n')]:
    # Keep independent original C++ and candidate C callers on their actual public declarations
    value=prefix+'#include "entry.h"\nTexResult TestTex(unsigned operation,void *a,void *b,uint32_t num,uint32_t count,const char *text)\n{\n    // Invoke every public owner operation without reconstructing its control flow\n    TexResult result={0};switch(operation){\n'+cases+'    }return result;\n}\n'
    (probe/name).write_text(value,newline='\n')
old_bindings=m.forwarder_definitions((out/'before/src/rw/TxdStore_native.cpp').read_text())
assert len(old_bindings)==3
(probe/'before-bindings.cpp').write_text('#include "owners.h"\n#include "TxdStore_backend.h"\n'+'\n'.join(definition for _,_,definition,_ in old_bindings)+'\n',newline='\n')
original=(out/'before/src/rw/TexRead.cpp').read_text()
caps=re.search(r'struct GPUcaps\n\{[\s\S]*?\n\};',original)[0]
original=original.replace(caps,'')
original=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
native=re.sub(r'^#include[^\n]*\n','',m.native(),flags=re.M)
for name,value in [('owners.h',owners),('entry.h',entry),('original-caps.h',caps+'\n'),('original.cpp','#include "owners.h"\n'+original),('native.cpp','#include "owners.h"\n#include "TexRead_backend.h"\n'+native),('fixture.cpp',(root/'utils/tests/tex_read_fixture.cpp.in').read_text())]:
    # Share the exact original capability record declaration while retaining all complete original function bodies
    (probe/name).write_text(value,newline='\n')
for name,source,kind in [('Streaming_constants.h','src/core/Streaming.h','stream'),('CdStream_constants.h','src/core/CdStream.h','cd'),('crossplatform_constants.h','src/skel/crossplatform.h','os')]:
    # Use the exact captured original constants on both sides of the fixture boundary
    (probe/name).write_text(m.constants((out/'before'/source).read_text(),kind),newline='\n')
print('Prepared complete original/C texture policy and typed native SDK callbacks')
