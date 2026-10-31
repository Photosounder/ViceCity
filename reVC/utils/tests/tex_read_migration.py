"""Project complete texture loading, capability persistence and archive conversion into C."""
import re
from rect_migration import mask,closing
from tex_read_schema import HOST,NAMES

RENAMES={'src/rw/TexRead.cpp':'src/rw/TexRead.c'}

def constant_block(source,marker):
    # Capture existing shared constant declarations without changing values or conditional layout
    index=source.index(marker)
    start=source.rfind('enum',0,index)
    return source[start:source.index('};',index)+2]

def constants(source,kind):
    # Expose existing C-compatible constants once for both C and native consumers
    if kind=='cd':value=re.search(r'(?m)^#define CDSTREAM_SECTOR_SIZE[^\n]*',source)[0]
    elif kind=='os':value=constant_block(source,'OS_WIN95 = 0')
    else:value=constant_block(source,'STREAM_OFFSET_TXD = MODELINFOSIZE')
    return '//+ rouz edit (ChatGPT)\n#pragma once\n'+('#include "config.h"\n' if kind=='stream' else '')+value+'\n//- rouz edit (ChatGPT)\n'

def header():
    # Declare opaque SDK handles, complete C operations and the original capability record
    return '''//+ rouz edit (ChatGPT)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
typedef struct GPUcaps {uint32_t version,platform,subplatform,dxtSupport;} GPUcaps;
#ifdef __cplusplus
extern "C" {
#endif
extern float texLoadTime;
extern int32_t texNumLoaded;
void *TexRead_ReadTexture(void *stream);
void *TexRead_DestroyTexture(void *texture,void *data);
void *TexRead_ReadDictionary(void *stream);
void *TexRead_ReadDictionary1(void *stream);
void *TexRead_ReadDictionary2(void *stream,void *dictionary);
#ifdef GTA_PC
#ifdef LIBRW
void TexRead_ReadVideoCardCapsFile(GPUcaps *caps);
#else
void TexRead_ReadVideoCardCapsFile(uint32_t *cap32,uint32_t *cap24,uint32_t *cap16,uint32_t *cap8);
#endif
bool TexRead_CheckVideoCardCaps(void);
void TexRead_WriteVideoCardCapsFile(void);
bool TexRead_CanVideoCardDoDXT(void);
void TexRead_ConvertingTexturesScreen(uint32_t num,uint32_t count,const char *text);
void TexRead_DealWithTxdWriteError(uint32_t num,uint32_t count,const char *text);
bool TexRead_CreateTxdImageForVideoCard(void);
#endif
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''

def backend_header():
    # Keep metadata reads and SDK calls outside the complete C owner
    value='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "Rect.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    value+='typedef void *(*TexReadTextureVisitor)(void *texture,void *data);\n#ifdef LIBRW\ntypedef int32_t TexReadStreamPosition;\n#else\ntypedef uint32_t TexReadStreamPosition;\n#endif\n'
    value+='bool TexRead_HostReadNative(void *stream,void **texture,uint32_t size);\nvoid TexRead_HostForAllTextures(void *dictionary,TexReadTextureVisitor visitor,void *data);\n'
    value+=''.join(result+' TexRead_Host'+name+'('+args+');\n' for result,name,args,_,_ in HOST)
    return value+'void re3_assert(const char *expr,const char *filename,unsigned int lineno,const char *func);\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'

def kernel(original):
    # Preserve all complete original bodies including both SDK capability implementations
    source=re.sub(r'^#include[^\n]*\n','',original,flags=re.M)
    source=re.sub(r'^extern "C"[^\n]*\n','',source,flags=re.M)
    source=re.sub(r'#ifdef LIBRW\n#define READNATIVE[^\n]*\n#else\n#define READNATIVE[^\n]*\n#endif\n','',source)
    source=re.sub(r'struct GPUcaps\n\{[\s\S]*?\n\};\n','',source)
    signature='ReadVideoCardCapsFile(uint32 &cap32, uint32 &cap24, uint32 &cap16, uint32 &cap8)'
    start=source.index(signature);opening=source.index('{',start);end=source.index('\n}',opening)+2
    value=source[start:end].replace('uint32 &','uint32 *')
    for name in ['cap32','cap24','cap16','cap8']:
        value=value.replace('(char*)&'+name,'(char*)'+name)
        value=re.sub(r'(?m)^(\s*)'+name+r' =',r'\1*'+name+' =',value)
    source=source[:start]+value+source[end:]
    source=source.replace('ReadVideoCardCapsFile(fcap32, fcap24, fcap16, fcap8)','ReadVideoCardCapsFile(&fcap32, &fcap24, &fcap16, &fcap8)')
    for match in reversed(list(re.finditer(r'\bCRGBA\(',mask(source)))):
        # Flatten the original temporary color arguments at the SDK value boundary
        end=closing(mask(source),match.end()-1)
        source=source[:match.start()]+source[match.end():end-1]+source[end:]
    mapping={
        'RwStreamFindChunk(stream, rwID_TEXTURENATIVE, &size, &version)':'TexRead_HostFindChunk(stream,true,&size,&version)',
        'RwStreamFindChunk(stream, rwID_STRUCT, &size, &version)':'TexRead_HostFindChunk(stream,false,&size,&version)',
        'gGameState == GS_INIT_PLAYING_GAME':'TexRead_HostPlayingGame()',
        'READNATIVE(':'TexRead_HostReadNative(',
        'RpAnisotGetMaxSupportedMaxAnisotropy()':'TexRead_HostMaxAnisotropy()',
        'RpAnisotTextureSetMaxAnisotropy':'TexRead_HostSetAnisotropy',
        'RwTextureDestroy':'TexRead_HostDestroyTexture','RwTexDictionaryCreate':'TexRead_HostCreateDictionary',
        'RwTexDictionaryForAllTextures':'TexRead_HostForAllTextures','RwTexDictionaryDestroy':'TexRead_HostDestroyDictionary',
        'RwTexDictionaryAddTexture':'TexRead_HostAddTexture','RwStreamRead':'TexRead_HostReadStream',
        'RwStreamSkip':'TexRead_HostSkipStream','STREAMPOS(':'TexRead_HostPosition(',
        'rw::platform':'TexRead_HostPlatform()','rw::gl3::gl3Caps.gles':'TexRead_HostGLSubplatform()',
        'rw::gl3::gl3Caps.dxtSupported':'TexRead_HostGLDXT()',
        '_rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE, rwRASTERFORMAT8888)':'TexRead_HostRaster8888()',
        '_rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE, rwRASTERFORMAT888)':'TexRead_HostRaster888()',
        '_rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE, rwRASTERFORMAT1555)':'TexRead_HostRaster1555()',
        '_rwD3D8FindCorrectRasterFormat(rwRASTERTYPETEXTURE, rwRASTERFORMATPAL8 | rwRASTERFORMAT8888)':'TexRead_HostRasterPAL8()',
        '_rwD3D8CheckValidTextureFormat(D3DFMT_DXT1)':'TexRead_HostDXT1()',
        '_rwD3D8CheckValidTextureFormat(D3DFMT_DXT3)':'TexRead_HostDXT3()',
        'HandleExit()':'TexRead_HostHandleExit()','LoadSplash(nil)':'TexRead_HostLoadSplash()',
        'DoRWStuffStartOfFrame(0, 0, 0, 0, 0, 0, 255)':'TexRead_HostStartFrame()',
        'CSprite2d::SetRecipNearClip':'TexRead_HostRecipNearClip','CSprite2d::InitPerFrame':'TexRead_HostSpriteInit',
        'CFont::InitPerFrame':'TexRead_HostFontInit','DefinedState()':'TexRead_HostDefinedState()',
        'RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS, (void*)rwTEXTUREADDRESSCLAMP)':'TexRead_HostAddressClamp()',
        'splash->Draw(':'TexRead_HostSplashDraw(splash,','CSprite2d::DrawRect':'TexRead_HostDrawRect',
        'CFont::SetBackgroundOff':'TexRead_HostFontBackgroundOff','CFont::SetPropOn':'TexRead_HostFontPropOn',
        'CFont::SetScale':'TexRead_HostFontScale','CFont::SetCentreOff':'TexRead_HostFontCentreOff',
        'CFont::SetWrapx':'TexRead_HostFontWrap','CFont::SetJustifyOff':'TexRead_HostFontJustifyOff',
        'CFont::SetColor':'TexRead_HostFontColor','CFont::SetBackGroundOnlyTextOff':'TexRead_HostFontBackgroundTextOff',
        'CFont::SetFontStyle':'TexRead_HostFontStyle','CFont::PrintString':'TexRead_HostFontPrint',
        'CFont::DrawFonts':'TexRead_HostDrawFonts','DoRWStuffEndOfFrame()':'TexRead_HostEndFrame()',
        'CPad::UpdatePads()':'TexRead_HostUpdatePads()','CPad::GetPad(0)->GetEscapeJustDown()':'TexRead_HostEscape()',
        'RsGlobal.quit = false':'TexRead_HostSetQuit(false)','RsGlobal.quit = true':'TexRead_HostSetQuit(true)',
        'RsGlobal.quit':'TexRead_HostQuit()','LoadingScreen(nil, nil, nil)':'TexRead_HostLoadingScreen()',
        'CStreaming::FlushRequestList()':'TexRead_HostFlushRequests()',
        'RwOsGetFileInterface()':'TexRead_HostFileInterface()',
        'RwStreamOpen(rwSTREAMFILENAME, rwSTREAMWRITE, "models\\\\txd.img")':'TexRead_HostOpenImage()',
        '_dwOperatingSystemVersion':'TexRead_HostOSVersion()',
        'rw::gl3::needToReadBackTextures = true':'TexRead_HostReadBack(true)',
        'rw::gl3::needToReadBackTextures = false':'TexRead_HostReadBack(false)',
        'FrontEndMenuManager.m_PrefsVsync = 0':'TexRead_HostSetVsync(0)',
        'FrontEndMenuManager.m_PrefsFrameLimiter = 0':'TexRead_HostSetFrameLimiter(0)',
        'FrontEndMenuManager.m_PrefsVsync = vsyncState':'TexRead_HostSetVsync(vsyncState)',
        'FrontEndMenuManager.m_PrefsFrameLimiter = frameLimiterState':'TexRead_HostSetFrameLimiter(frameLimiterState)',
        'FrontEndMenuManager.m_PrefsVsync':'TexRead_HostVsync()',
        'FrontEndMenuManager.m_PrefsFrameLimiter':'TexRead_HostFrameLimiter()',
        'CStreaming::IsObjectInCdImage':'TexRead_HostInImage','CStreaming::RequestTxd':'TexRead_HostRequestTxd',
        'CStreaming::RequestModelStream':'TexRead_HostRequestModelStream','CStreaming::FlushChannels':'TexRead_HostFlushChannels',
        'RwTexDictionaryStreamWrite':'TexRead_HostWriteDictionary','RwStreamWrite':'TexRead_HostWriteStream',
        'CStreaming::RemoveTxd':'TexRead_HostRemoveTxd',
        'RwStreamClose(img, nil)':'TexRead_HostCloseStream(img)',
    }
    for old,new in mapping.items():source=source.replace(old,new)
    source=source.replace('((RwTexDictionary*)TxdStore_GetSlot(i)->texDict)','TxdStore_GetSlot(i)->texDict')
    source=source.replace('#define STREAMTELL(str)\tstr->tell()','#define STREAMTELL(str) TexRead_HostTell(str)')
    source=source.replace('#define STREAMTELL(str) filesys->rwftell((str)->Type.file.fpFile)','#define STREAMTELL(str) TexRead_HostLegacyTell(filesys,str)')
    for old,new in [('RwTexture','void'),('RwTexDictionary','void'),('RwStream','void'),('CSprite2d','void'),('RwFileFunctions','void'),('RwUInt32','uint32_t'),('RwInt32','int32_t'),('uint32','uint32_t'),('int32','int32_t'),('uint8','uint8_t'),('int8','int8_t'),('nil','NULL')]:
        # Keep primitive widths and SDK pointer representations unchanged
        source=re.sub(r'\b'+old+r'\b',new,source)
    for old,new in NAMES.items():source=re.sub(r'\b'+old+r'\b','TexRead_'+new,source)
    source=source.replace('TexRead_CreateTxdImageForVideoCard()','TexRead_CreateTxdImageForVideoCard(void)')
    for name in NAMES.values():
        # Describe each whole converted function without altering its original control flow
        source=re.sub(r'(?m)^(TexRead_'+name+r'\([^\n]*\)\n\{\n)',lambda match:match[1]+'    // Preserve the complete original '+name+' operation and live mutation order\n',source)
    prefix='''//+ rouz edit (ChatGPT)
#define FORCE_PC_SCALING
#include "config.h"
#include "TexRead.h"
#include "TexRead_backend.h"
#include "Timer.h"
#include "FileMgr.h"
#include "Directory.h"
#include "Text.h"
#include "TxdStore.h"
#include "Draw.h"
#include "Draw_constants.h"
#include "FontStyles.h"
#include "Streaming_constants.h"
#include "StreamingInfo.h"
#include "CdStream_constants.h"
#include "crossplatform_constants.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#define CITA_REPORT_TO_STDERR
#include <cita_windows.h>
#undef assert
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
#ifdef GTA_PS2
#define SCREEN_WIDTH ((float)640)
#ifdef GTA_PAL
#define SCREEN_HEIGHT ((float)512)
#else
#define SCREEN_HEIGHT ((float)448)
#endif
#else
#define SCREEN_WIDTH TexRead_HostScreenWidth()
#define SCREEN_HEIGHT TexRead_HostScreenHeight()
#endif
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
'''
    return prefix+source+'\n//- rouz edit (ChatGPT)\n'

FORWARDERS=['TxdStore_HostReadDictionary','TxdStore_HostReadDictionary1','TxdStore_HostReadDictionary2']

def forwarder_definitions(source):
    # Select whole existing dictionary-read forwarding functions at the old C++ owner boundary
    code=mask(source);rows=[]
    for name in FORWARDERS:
        match=re.search(r'(?m)^[\w *]+\b'+name+r'\([^\n]*\)\n\{',code)
        if match:
            end=match.end();depth=1
            while depth:
                if code[end]=='{':depth+=1
                elif code[end]=='}':depth-=1
                end+=1
            if 'RwTexDictionaryGtaStreamRead' in code[match.end():end-1]:rows.append((match.start(),end,source[match.start():end],name))
    return sorted(rows)

def consumer_calls(source,opaque=False):
    # Move each native client to the explicit C API while preserving its original SDK pointer type
    code=mask(source)
    for old,name in NAMES.items():
        if old in ['GetGPUcaps','destroyTexture','ReadVideoCardCapsFile']:continue
        for match in reversed(list(re.finditer(r'\b'+old+r'\(',mask(source)))):
            end=closing(mask(source),match.end()-1);value='TexRead_'+name+source[match.end()-1:end]
            if old.startswith('RwTexDictionaryGta') and not opaque:value='((RwTexDictionary*)'+value+')'
            if old=='RwTextureGtaStreamRead' and not opaque:value='((RwTexture*)'+value+')'
            source=source[:match.start()]+value+source[end:]
    if opaque:source=source.replace('(RwStream*)','').replace('(RwTexDictionary*)','')
    return source

def bindings(original):
    # Retain the three existing C forwarding ABIs directly in C after their target algorithms become C
    rows=forwarder_definitions(original);assert len(rows)==3
    return '//+ rouz edit (ChatGPT)\n#include "TexRead.h"\n#include "TxdStore_backend.h"\n'+'\n'.join(consumer_calls(definition,True) for _,_,definition,_ in rows)+'\n//- rouz edit (ChatGPT)\n'

def tex_read_calls(source):
    # Stage complete texture policy, all consumers and shared original constant declarations
    if 'RwTextureGtaStreamRead(RwStream *stream)\n{' in source:return kernel(source)
    if '#define CDSTREAM_SECTOR_SIZE 2048' in source and 'void CdStreamInitThread(void);' in source:
        return '//+ rouz edit (ChatGPT)\n'+source.replace('#define CDSTREAM_SECTOR_SIZE 2048','#include "CdStream_constants.h"')+'//- rouz edit (ChatGPT)\n'
    if 'enum eWinVersion\n' in source and 'OS_WIN95 = 0' in source and 'char *_strdate(char *buf);' in source:
        return '//+ rouz edit (ChatGPT)\n'+source.replace(constant_block(source,'OS_WIN95 = 0'),'#include "crossplatform_constants.h"')+'//- rouz edit (ChatGPT)\n'
    if 'STREAM_OFFSET_TXD = MODELINFOSIZE' in source and 'class CStreaming' in source:
        return '//+ rouz edit (ChatGPT)\n'+source.replace(constant_block(source,'STREAM_OFFSET_TXD = MODELINFOSIZE'),'#include "Streaming_constants.h"')+'//- rouz edit (ChatGPT)\n'
    if 'RwTexDictionary *RwTexDictionaryGtaStreamRead(RwStream *stream);' in source:
        start=source.index('RwTexDictionary *RwTexDictionaryGtaStreamRead(RwStream *stream);')
        end=source.index('bool CreateTxdImageForVideoCard();',start)+len('bool CreateTxdImageForVideoCard();')
        return '//+ rouz edit (ChatGPT)\n'+source[:start]+'#include "TexRead.h"'+source[end:]+'//- rouz edit (ChatGPT)\n'
    original=source
    for start,end,_,_ in reversed(forwarder_definitions(source)):
        source=source[:start]+source[end:]
    source=consumer_calls(source)
    if source!=original:
        lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True)
        for index in reversed(range(len(lines))):
            if re.search(r'\bTexRead_\w+\(',code[index]):
                lines.insert(index,re.match(r'\s*',lines[index])[0]+'// Use the complete C texture reader with the original live SDK arguments\n')
        return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'
    return source

def tex_read_project(source):
    # Select both complete texture C units and keep external SDK primitives in a distinct native unit
    import object_names_migration as objects
    source=source.replace('src\\rw\\TexRead.cpp','src\\rw\\TexRead.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\TexRead\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Retain existing metadata and explicitly compile both texture units as C11
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';prefix=match[1][:-len('TexRead.c')]
            for name in ['TexRead_native.cpp','TexRead_bindings.c']:
                if '<ClCompile Include="'+prefix+name+'"' not in source:
                    result+='\n    <ClCompile Include="'+prefix+name+'"'
                    result+=('>\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>' if name.endswith('.c') else ' />')
            return result
        source=re.sub(pattern,metadata,source)
    anchor=re.search(r'<ClInclude Include="([^"]*src\\rw\\RwHelper\.h)"\s*/>',source)
    if anchor:
        for name in ['TexRead.h','TexRead_backend.h']:
            entry='<ClInclude Include="'+anchor[1][:-len('RwHelper.h')]+name+'" />'
            if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    for old,new in [('CdStream.h','CdStream_constants.h'),('Streaming.h','Streaming_constants.h'),('crossplatform.h','crossplatform_constants.h')]:
        # Keep shared C constant declarations visible beside their original native headers
        anchor=re.search(r'<ClInclude Include="([^"]*\\'+re.escape(old)+r')"\s*/>',source)
        if anchor:
            entry='<ClInclude Include="'+anchor[1][:-len(old)]+new+'" />'
            if entry not in source:source=source[:anchor.end()]+'\n    '+entry+source[anchor.end():]
    return objects.project(source)

def native():
    # Keep only actual external operations and the typed SDK callback trampoline in C++
    value='''//+ rouz edit (ChatGPT)
#define FORCE_PC_SCALING
#ifndef LIBRW
#define WITHD3D
#endif
#include "common.h"
#ifdef ANISOTROPIC_FILTERING
#include "rpanisot.h"
#endif
#include "crossplatform.h"
#include "platform.h"
#include "TexRead_backend.h"
#ifdef GTA_PC
#include "main.h"
#include "Pad.h"
#include "Streaming.h"
#include "Font.h"
#include "Sprite2d.h"
#include "RwHelper.h"
#include "Frontend.h"
#ifndef LIBRW
extern "C" RwInt32 _rwD3D8FindCorrectRasterFormat(RwRasterType type,RwInt32 flags);
extern "C" RwBool _rwD3D8CheckValidTextureFormat(RwInt32 format);
#endif
#endif
bool TexRead_HostReadNative(void *stream,void **texture,uint32_t size)
{
    // Keep the SDK output slot correctly typed and copy it only after a successful native read
    RwTexture *value;
#ifdef LIBRW
    bool result=rwNativeTextureHackRead((RwStream*)stream,&value,size)!=0;
#else
    bool result=RWSRCGLOBAL(stdFunc[rwSTANDARDNATIVETEXTUREREAD]((RwStream*)stream,&value,size))!=0;
#endif
    if(result)*texture=value;
    return result;
}
struct TexReadVisitorContext {TexReadTextureVisitor visitor;void *data;};
static RwTexture *TexRead_VisitTexture(RwTexture *texture,void *opaque)
{
    // Enter the C callback through the current synchronous enumeration context
    TexReadVisitorContext *context=(TexReadVisitorContext*)opaque;
    return (RwTexture*)context->visitor(texture,context->data);
}
void TexRead_HostForAllTextures(void *dictionary,TexReadTextureVisitor visitor,void *data)
{
    // Keep each callback context local so nested SDK enumerations retain their own visitor and data
    TexReadVisitorContext context={visitor,data};
    RwTexDictionaryForAllTextures((RwTexDictionary*)dictionary,TexRead_VisitTexture,&context);
}
'''
    for result,name,args,expression,guard in HOST:
        # Bind one external SDK operation or live metadata access while all texture policy remains in C
        if guard:value+='#if '+guard+'\n'
        value+=result+' TexRead_Host'+name+'('+args+')\n{\n    // Preserve the original external primitive and its live borrowed arguments\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
        if guard:value+='#endif\n'
    return value+'//- rouz edit (ChatGPT)\n'


def calls(source):
    # Retain complete texture ownership before applying the complete C clump successor
    import clump_read_migration as clump
    return clump.calls(tex_read_calls(source))

def project(source):
    # Preserve preceding C metadata before selecting the complete C clump owner and native SDK unit
    import clump_read_migration as clump
    return clump.project(tex_read_project(source))
