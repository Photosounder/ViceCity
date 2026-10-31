//+ rouz edit (ChatGPT)
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
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#pragma warning( push )
#pragma warning( disable : 4005)
#pragma warning( pop )
#define FORCE_PC_SCALING
#ifndef LIBRW
#define WITHD3D
#endif
#ifdef ANISOTROPIC_FILTERING
#endif

#ifdef GTA_PC
#endif //GTA_PC

float texLoadTime;
int32_t texNumLoaded;


void*
TexRead_ReadTexture(void *stream)
{
    // Preserve the complete original ReadTexture operation and live mutation order
	uint32_t size, version;
	void *tex;

	if(!TexRead_HostFindChunk(stream,true,&size,&version))
		return NULL;

	float preloadTime = (float)Timer_GetCurrentTimeInCycles() / (float)Timer_GetCyclesPerMillisecond();

	if(!TexRead_HostReadNative(stream, &tex, size))
		return NULL;

	if (TexRead_HostPlayingGame()) {
		texLoadTime = (texNumLoaded * texLoadTime + (float)Timer_GetCurrentTimeInCycles() / (float)Timer_GetCyclesPerMillisecond() - preloadTime) / (float)(texNumLoaded+1);
		texNumLoaded++;
	}

#ifdef ANISOTROPIC_FILTERING
	if(tex && TexRead_HostMaxAnisotropy() > 1)	// BUG? this was RpAnisotTextureGetMaxAnisotropy, but that doesn't make much sense
		TexRead_HostSetAnisotropy(tex, TexRead_HostMaxAnisotropy());
#endif

	return tex;
}

void*
TexRead_DestroyTexture(void *texture, void *data)
{
    // Preserve the complete original DestroyTexture operation and live mutation order
	TexRead_HostDestroyTexture(texture);
	return texture;
}

void*
TexRead_ReadDictionary(void *stream)
{
    // Preserve the complete original ReadDictionary operation and live mutation order
	uint32_t size, version;
	int32_t numTextures;
	void *texDict;
	void *tex;

	if(!TexRead_HostFindChunk(stream,false,&size,&version))
		return NULL;
	if(TexRead_HostReadStream(stream, &numTextures, size) != size)
		return NULL;

	texDict = TexRead_HostCreateDictionary();
	if(texDict == NULL)
		return NULL;

	while(numTextures--){
		tex = TexRead_ReadTexture(stream);
		if(tex == NULL){
			TexRead_HostForAllTextures(texDict, TexRead_DestroyTexture, NULL);
			TexRead_HostDestroyDictionary(texDict);
			return NULL;
		}
		TexRead_HostAddTexture(texDict, tex);
	}

	return texDict;
}

static int32_t numberTextures = -1;
static int32_t streamPosition;

void*
TexRead_ReadDictionary1(void *stream)
{
    // Preserve the complete original ReadDictionary1 operation and live mutation order
	uint32_t size, version;
	int32_t numTextures;
	void *texDict;
	void *tex;

	numberTextures = 0;
	if(!TexRead_HostFindChunk(stream,false,&size,&version))
		return NULL;
	assert(size == 4);
	if(TexRead_HostReadStream(stream, &numTextures, size) != size)
		return NULL;

	texDict = TexRead_HostCreateDictionary();
	if(texDict == NULL)
		return NULL;

	numberTextures = numTextures/2;

	while(numTextures > numberTextures){
		numTextures--;

		tex = TexRead_ReadTexture(stream);
		if(tex == NULL){
			TexRead_HostForAllTextures(texDict, TexRead_DestroyTexture, NULL);
			TexRead_HostDestroyDictionary(texDict);
			return NULL;
		}
		TexRead_HostAddTexture(texDict, tex);
	}

	numberTextures = numTextures;
	streamPosition = TexRead_HostPosition(stream);

	return texDict;
}

void*
TexRead_ReadDictionary2(void *stream, void *texDict)
{
    // Preserve the complete original ReadDictionary2 operation and live mutation order
	void *tex;

	TexRead_HostSkipStream(stream, streamPosition - TexRead_HostPosition(stream));

	while(numberTextures--){
		tex = TexRead_ReadTexture(stream);
		if(tex == NULL){
			TexRead_HostForAllTextures(texDict, TexRead_DestroyTexture, NULL);
			TexRead_HostDestroyDictionary(texDict);
			return NULL;
		}
		TexRead_HostAddTexture(texDict, tex);
	}

	return texDict;
}

#ifdef GTA_PC

#ifdef LIBRW

#define CAPSVERSION 0


static void
TexRead_GetGPUcaps(GPUcaps *caps)
{
    // Preserve the complete original GetGPUcaps operation and live mutation order
	caps->version = CAPSVERSION;
	caps->platform = TexRead_HostPlatform();
	caps->subplatform = 0;
	caps->dxtSupport = 0;
	// TODO: more later
#ifdef RW_GL3
	caps->subplatform = TexRead_HostGLSubplatform();
	caps->dxtSupport = TexRead_HostGLDXT();
#endif
#ifdef RW_D3D9
	caps->dxtSupport = 1;	// TODO, probably
#endif
}

void
TexRead_ReadVideoCardCapsFile(GPUcaps *caps)
{
    // Preserve the complete original ReadVideoCardCapsFile operation and live mutation order
	memset(caps, 0, sizeof(GPUcaps));

	int32_t file = FileMgr_OpenFile("DATA\\CAPS.DAT", "rb");
	if (file != 0) {
		FileMgr_Read(file, (char*)&caps->version, 4);
		FileMgr_Read(file, (char*)&caps->platform, 4);
		FileMgr_Read(file, (char*)&caps->subplatform, 4);
		FileMgr_Read(file, (char*)&caps->dxtSupport, 4);
		FileMgr_CloseFile(file);
	}
}

bool
TexRead_CheckVideoCardCaps(void)
{
    // Preserve the complete original CheckVideoCardCaps operation and live mutation order
	GPUcaps caps, fcaps;
	TexRead_GetGPUcaps(&caps);
	TexRead_ReadVideoCardCapsFile(&fcaps);
	return caps.version != fcaps.version ||
		caps.platform != fcaps.platform ||
		caps.subplatform != fcaps.subplatform ||
		caps.dxtSupport != fcaps.dxtSupport;
}

void
TexRead_WriteVideoCardCapsFile(void)
{
    // Preserve the complete original WriteVideoCardCapsFile operation and live mutation order
	GPUcaps caps;
	TexRead_GetGPUcaps(&caps);
	int32_t file = FileMgr_OpenFile("DATA\\CAPS.DAT", "wb");
	if (file != 0) {
		FileMgr_Write(file, (char*)&caps.version, 4);
		FileMgr_Write(file, (char*)&caps.platform, 4);
		FileMgr_Write(file, (char*)&caps.subplatform, 4);
		FileMgr_Write(file, (char*)&caps.dxtSupport, 4);
		FileMgr_CloseFile(file);
	}
}


#else
void
TexRead_ReadVideoCardCapsFile(uint32_t *cap32, uint32_t *cap24, uint32_t *cap16, uint32_t *cap8)
{
    // Preserve the complete original ReadVideoCardCapsFile operation and live mutation order
	*cap32 = UINT32_MAX;
	*cap24 = UINT32_MAX;
	*cap16 = UINT32_MAX;
	*cap8 = UINT32_MAX;

	int32_t file = FileMgr_OpenFile("DATA\\CAPS.DAT", "rb");
	if (file != 0) {
		FileMgr_Read(file, (char*)cap32, 4);
		FileMgr_Read(file, (char*)cap24, 4);
		FileMgr_Read(file, (char*)cap16, 4);
		FileMgr_Read(file, (char*)cap8, 4);
		FileMgr_CloseFile(file);
	}
}

bool
TexRead_CheckVideoCardCaps(void)
{
    // Preserve the complete original CheckVideoCardCaps operation and live mutation order
	uint32_t cap32 = TexRead_HostRaster8888();
	uint32_t cap24 = TexRead_HostRaster888();
	uint32_t cap16 = TexRead_HostRaster1555();
	uint32_t cap8 = TexRead_HostRasterPAL8();
	uint32_t fcap32, fcap24, fcap16, fcap8;
	TexRead_ReadVideoCardCapsFile(&fcap32, &fcap24, &fcap16, &fcap8);
	return cap32 != fcap32 || cap24 != fcap24 || cap16 != fcap16 || cap8 != fcap8;
}

void
TexRead_WriteVideoCardCapsFile(void)
{
    // Preserve the complete original WriteVideoCardCapsFile operation and live mutation order
	uint32_t cap32 = TexRead_HostRaster8888();
	uint32_t cap24 = TexRead_HostRaster888();
	uint32_t cap16 = TexRead_HostRaster1555();
	uint32_t cap8 = TexRead_HostRasterPAL8();
	int32_t file = FileMgr_OpenFile("DATA\\CAPS.DAT", "wb");
	if (file != 0) {
		FileMgr_Write(file, (char*)&cap32, 4);
		FileMgr_Write(file, (char*)&cap24, 4);
		FileMgr_Write(file, (char*)&cap16, 4);
		FileMgr_Write(file, (char*)&cap8, 4);
		FileMgr_CloseFile(file);
	}
}
#endif

bool
TexRead_CanVideoCardDoDXT(void)
{
    // Preserve the complete original CanVideoCardDoDXT operation and live mutation order
#ifdef LIBRW
	// TODO
#ifdef RW_OPENGL
	return false;
#else
	return true;
#endif
#else
	return TexRead_HostDXT1() && TexRead_HostDXT3();
#endif
}

void
TexRead_ConvertingTexturesScreen(uint32_t num, uint32_t count, const char *text)
{
    // Preserve the complete original ConvertingTexturesScreen operation and live mutation order
	TexRead_HostHandleExit();

	void *splash = TexRead_HostLoadSplash();
	if (!TexRead_HostStartFrame())
		return;

	TexRead_HostRecipNearClip();
	TexRead_HostSpriteInit();
	TexRead_HostFontInit();
	TexRead_HostDefinedState();

	TexRead_HostAddressClamp();
	TexRead_HostSplashDraw(splash,Rect_Make(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT), 255, 255, 255, 255);

	TexRead_HostDrawRect(Rect_Make(SCREEN_SCALE_X(200.0f), SCREEN_SCALE_Y(240.0f), SCREEN_SCALE_FROM_RIGHT(200.0f), SCREEN_SCALE_Y(248.0f)), 64, 64, 64, 255);
#ifdef FIX_BUGS
	TexRead_HostDrawRect(Rect_Make(SCREEN_SCALE_X(200.0f), SCREEN_SCALE_Y(240.0f), (SCREEN_SCALE_FROM_RIGHT(200.0f) - SCREEN_SCALE_X(200.0f)) * ((float)num / (float)count) + SCREEN_SCALE_X(200.0f), SCREEN_SCALE_Y(248.0f)), 255, 150, 225, 255);
#else
	TexRead_HostDrawRect(Rect_Make(SCREEN_SCALE_X(200.0f), SCREEN_SCALE_Y(240.0f), (SCREEN_SCALE_FROM_RIGHT(200.0f) - SCREEN_SCALE_X(200.0f)) * ((float)num / (float)count) + SCREEN_SCALE_X(200.0f), SCREEN_SCALE_Y(248.0f)), 255, 217, 106, 255);
#endif
	TexRead_HostDrawRect(Rect_Make(SCREEN_SCALE_X(120.0f), SCREEN_SCALE_Y(150.0f), SCREEN_SCALE_FROM_RIGHT(120.0f), SCREEN_HEIGHT - SCREEN_SCALE_Y(220.0f)), 50, 50, 50, 210);

	TexRead_HostFontBackgroundOff();
	TexRead_HostFontPropOn();
	TexRead_HostFontScale(SCREEN_SCALE_X(0.45f), SCREEN_SCALE_Y(0.7f));
	TexRead_HostFontCentreOff();
	TexRead_HostFontWrap(SCREEN_SCALE_FROM_RIGHT(170.0f));
	TexRead_HostFontJustifyOff();
#ifdef FIX_BUGS
	TexRead_HostFontColor(255, 150, 225, 255);
#else
	TexRead_HostFontColor(255, 217, 106, 255);
#endif
	TexRead_HostFontBackgroundTextOff();
	TexRead_HostFontStyle(FONT_STANDARD);
	TexRead_HostFontPrint(SCREEN_SCALE_X(170.0f), SCREEN_SCALE_Y(160.0f), Text_Get(&TheText,text));
	TexRead_HostDrawFonts();
	TexRead_HostEndFrame();
}

void
TexRead_DealWithTxdWriteError(uint32_t num, uint32_t count, const char *text)
{
    // Preserve the complete original DealWithTxdWriteError operation and live mutation order
	while (!TexRead_HostQuit()) {
		TexRead_ConvertingTexturesScreen(num, count, text);
		TexRead_HostUpdatePads();
		if (TexRead_HostEscape())
			break;
	}
	TexRead_HostSetQuit(false);
	TexRead_HostLoadingScreen();
	TexRead_HostSetQuit(true);
}

#ifdef LIBRW
#define STREAMTELL(str) TexRead_HostTell(str)
#else
#define STREAMTELL(str) TexRead_HostLegacyTell(filesys,str)
#endif

bool
TexRead_CreateTxdImageForVideoCard(void)
{
    // Preserve the complete original CreateTxdImageForVideoCard operation and live mutation order
	uint8_t *buf = (uint8_t*)malloc(CDSTREAM_SECTOR_SIZE); // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
	// Initialize the directory record in its allocated storage
	CDirectory *pDir = (CDirectory*)malloc(sizeof(CDirectory));
	Directory_Initialize(pDir, TXDSTORESIZE);
//- rouz edit (ChatGPT)
	DirectoryInfo dirInfo;

	TexRead_HostFlushRequests();

#ifndef LIBRW
	void *filesys = TexRead_HostFileInterface();
#endif

	void *img = TexRead_HostOpenImage();
	if (img == NULL) {
		// original code does otherwise and it leaks
		free(buf); // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
		// Release the directory entries and then its owner storage
		Directory_Shutdown(pDir);
		free(pDir);
//- rouz edit (ChatGPT)

		if (TexRead_HostOSVersion() == OS_WINNT || TexRead_HostOSVersion() == OS_WIN2000 || TexRead_HostOSVersion() == OS_WINXP)
			TexRead_DealWithTxdWriteError(0, TXDSTORESIZE, "CVT_CRT");

		return false;
	}

#ifdef RW_GL3
	// so we can read back DXT with GLES
	// only works for textures that are not yet loaded
	// so let's hope that is the case for all
	TexRead_HostReadBack(true);
#endif
	
#ifdef DISABLE_VSYNC_ON_TEXTURE_CONVERSION
	// let's disable vsync and frame limiter to speed up texture conversion
	// (actually we probably don't need to disable frame limiter in here, but let's do it just in case =P)
	int8_t vsyncState = TexRead_HostVsync();
	int8_t frameLimiterState = TexRead_HostFrameLimiter();
	TexRead_HostSetVsync(0);
	TexRead_HostSetFrameLimiter(0);
#endif

	int32_t i;
	for (i = 0; i < TXDSTORESIZE; i++) {
		TexRead_ConvertingTexturesScreen(i, TXDSTORESIZE, "CVT_MSG");

		// Use the complete C texture dictionary owner with the original live arguments
		if (TxdStore_GetSlot(i) != NULL && TexRead_HostInImage(i + STREAM_OFFSET_TXD)) {
#ifdef FIX_BUGS
			// Use the complete C texture dictionary owner with the original live arguments
			if(strcmp(TxdStore_GetTxdName(i), "generic") == 0)
				continue;
#endif

			TexRead_HostRequestTxd(i, STREAMFLAGS_KEEP_IN_MEMORY);
			TexRead_HostRequestModelStream(0);
			TexRead_HostFlushChannels();

			char filename[64];
			// Use the complete C texture dictionary owner with the original live arguments
			sprintf(filename, "%s.txd", TxdStore_GetTxdName(i));

			// Use the complete C texture dictionary owner with the original live arguments
			if (TxdStore_GetSlot(i)->texDict) {

				int32_t pos = STREAMTELL(img);

				// Use the complete C texture dictionary owner with the original live arguments
				if (TexRead_HostWriteDictionary(TxdStore_GetSlot(i)->texDict, img) == NULL) {
					TexRead_DealWithTxdWriteError(i, TXDSTORESIZE, "CVT_ERR");
					TexRead_HostCloseStream(img);
					free(buf); // rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
					// Release the directory entries and then its owner storage
					Directory_Shutdown(pDir);
					free(pDir);
//- rouz edit (ChatGPT)
					TexRead_HostRemoveTxd(i);
#ifdef RW_GL3
					TexRead_HostReadBack(false);
#endif
					return false;
				}

				int32_t size = STREAMTELL(img) - pos;
				int32_t num = size % CDSTREAM_SECTOR_SIZE;

				size /= CDSTREAM_SECTOR_SIZE;
				if (num != 0) {
					size++;
					num = CDSTREAM_SECTOR_SIZE - num;
					TexRead_HostWriteStream(img, buf, num);
				}

				dirInfo.offset = pos / CDSTREAM_SECTOR_SIZE;
				dirInfo.size = size;
				strncpy(dirInfo.name, filename, sizeof(dirInfo.name));
				Directory_AddItem(pDir, &dirInfo);
				TexRead_HostRemoveTxd(i);
			}
			TexRead_HostFlushRequests();
		}
	}

#ifdef DISABLE_VSYNC_ON_TEXTURE_CONVERSION
	// restore vsync and frame limiter states
	TexRead_HostSetVsync(vsyncState);
	TexRead_HostSetFrameLimiter(frameLimiterState);
#endif

	TexRead_HostCloseStream(img);
	free(buf); // rouz edit (ChatGPT)

#ifdef RW_GL3
	TexRead_HostReadBack(false);
#endif

	if (!Directory_WriteDirFile(pDir, "models\\txd.dir")) {
		TexRead_DealWithTxdWriteError(i, TXDSTORESIZE, "CVT_ERR");
//+ rouz edit (ChatGPT)
		// Release the directory entries and then its owner storage
		Directory_Shutdown(pDir);
		free(pDir);
//- rouz edit (ChatGPT)
		return false;
	}

//+ rouz edit (ChatGPT)
	// Release the directory entries and then its owner storage
	Directory_Shutdown(pDir);
	free(pDir);
//- rouz edit (ChatGPT)

	TexRead_WriteVideoCardCapsFile();
	return true;
}
#endif // GTA_PC

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
