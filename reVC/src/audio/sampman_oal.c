//+ rouz edit (ChatGPT)
//#define JUICY_OAL

#define _CRT_SECURE_NO_WARNINGS
#include "../core/config.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#ifdef AUDIO_OAL
#define COBJMACROS
#ifdef _WIN32
#include <windows.h>
#define strcasecmp _stricmp
#define fcaseopen fopen
#else
#include <strings.h>
#include <unistd.h>
#include <sys/stat.h>
#include <alloca.h>
#endif

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int32_t int32;
typedef uint8_t bool8;
typedef uintptr_t uintptr;
#define FALSE 0
#define TRUE 1
#define nil NULL
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define _TODOCONST(value) (value)
#define DEV(...) ((void)0)
#define Min(a, b) ((a) < (b) ? (a) : (b))
#define Max(a, b) ((a) > (b) ? (a) : (b))
#define Clamp(value, low, high) ((value) < (low) ? (low) : (value) > (high) ? (high) : (value))
#ifndef MASTER
#define ASSERT(value) ((void)((!!(value)) || (re3_assert(#value, __FILE__, __LINE__, __func__), 0)))
#define TRACE(...) re3_trace(__FILE__, __LINE__, __func__, __VA_ARGS__)
#define USERERROR(...) re3_usererror(__VA_ARGS__)
#else
#define ASSERT(value) ((void)(value))
#define TRACE(...) ((void)0)
#define USERERROR(...) ((void)0)
#endif
#define debug(...) re3_debug("[DBG]: " __VA_ARGS__)


#include "eax.h"
#include "eax-util.h"

#ifdef _WIN32
#include <io.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#include <AL/efx-presets.h>

// for user MP3s
#include <direct.h>
#include <shlobj.h>
#include <shlguid.h>
#else
#define _getcwd getcwd
#endif

#if defined _MSC_VER && !defined CMAKE_NO_AUTOLINK
#pragma comment( lib, "OpenAL32.lib" )
#endif

#include "sampman.h"
#include "oal/oal_utils.h"
#include "oal/aldlist.h"
#include "oal/channel.h"
#include "oal/stream.h"
#include "oal/AudioStreamHost.h"
#include "oal/AudioSampleHost.h"
#ifdef AUDIO_OAL_USE_OPUS
#include <opusfile.h>
#endif

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>
#ifndef _WIN32
#define HANDLE void *
#define WIN32_FIND_DATA AudioSampleFindData
#define INVALID_HANDLE_VALUE NULL
#define FindFirstFile AudioSample_FindFirstFile
#define FindNextFile AudioSample_FindNextFile
#define FindClose AudioSample_FindClose
#define fcaseopen AudioSample_CaseOpen
#define casepath AudioStream_CasePath
#define MAX_PATH PATH_MAX
#define OutputDebugString(s) re3_debug("[DBG-2]: %s\n", s)
#endif

//TODO: fix eax3 reverb

cSampleManager SampleManager = {0}; // rouz edit (ChatGPT)
bool8 _bSampmanInitialised = FALSE;

uint32 BankStartOffset[MAX_SFX_BANKS];

int           prevprovider=-1;
int           curprovider=-1;
int           usingEAX=0;
int           usingEAX3=0;
//int         speaker_type=0;
ALCdevice    *ALDevice = NULL;
ALCcontext   *ALContext = NULL;
unsigned int _maxSamples;
float        _fPrevEaxRatioDestination; 
bool         _effectsSupported = false;
bool         _usingEFX;
float        _fEffectsLevel;
ALuint       ALEffect = AL_EFFECT_NULL;
ALuint       ALEffectSlot = AL_EFFECTSLOT_NULL;
struct
{ 
	const char *id;
	char name[256];
	int sources;
	bool bSupportsFx;
}providers[MAXPROVIDERS];

int defaultProvider;


char SampleBankDescFilename[] = "audio/sfx.SDT";
char SampleBankDataFilename[] = "audio/sfx.RAW";

FILE *fpSampleDescHandle;
#ifdef OPUS_SFX
OggOpusFile *fpSampleDataHandle;
#else
FILE *fpSampleDataHandle;
#endif
int8  gBankLoaded                  [MAX_SFX_BANKS];
int32 nSampleBankDiscStartOffset   [MAX_SFX_BANKS];
int32 nSampleBankSize              [MAX_SFX_BANKS];
uintptr nSampleBankMemoryStartAddress[MAX_SFX_BANKS];
int32 _nSampleDataEndOffset;

int32 nPedSlotSfx    [MAX_PEDSFX];
int32 nPedSlotSfxAddr[MAX_PEDSFX];
uint8 nCurrentPedSlot;

#ifdef FIX_BUGS
uint32 gPlayerTalkSfx = UINT32_MAX;
void *gPlayerTalkData = 0;
#endif

CChannel aChannel[NUM_CHANNELS];
uint8 nChannelVolume[NUM_CHANNELS];

uint32 nStreamLength[TOTAL_STREAMED_SOUNDS];
ALuint ALStreamSources[MAX_STREAMS][2];
ALuint ALStreamBuffers[MAX_STREAMS][NUM_STREAMBUFFERS];

typedef struct tMP3Entry
{
	char aFilename[MAX_PATH];

	uint32 nTrackLength;
	uint32 nTrackStreamPos;

	struct tMP3Entry* pNext;
	char* pLinkPath;
} tMP3Entry;

uint32 nNumMP3s;
tMP3Entry* _pMP3List;
char _mp3DirectoryPath[MAX_PATH]; 
CStream    *aStream[MAX_STREAMS];
uint8      nStreamPan   [MAX_STREAMS];
uint8      nStreamVolume[MAX_STREAMS];
bool8      nStreamLoopedFlag[MAX_STREAMS];
uint32 _CurMP3Index;
int32 _CurMP3Pos;
bool8 _bIsMp3Active;
///////////////////////////////////////////////////////////////
//	Env		Size	Diffus	Room	RoomHF	RoomLF	DecTm	DcHF	DcLF	Refl	RefDel	Ref Pan				Revb	RevDel		Rev Pan				EchTm	EchDp	ModTm	ModDp	AirAbs	HFRef		LFRef	RRlOff	FLAGS
EAXLISTENERPROPERTIES StartEAX3 =
	{26,	1.7f,	0.8f,	-1000,	-1000,	-100,	4.42f,	0.14f,	1.00f,	429,	0.014f,	0.00f,0.00f,0.00f,	1023,	0.021f,		0.00f,0.00f,0.00f,	0.250f,	0.000f,	0.250f,	0.000f,	-5.0f,	2727.1f,	250.0f,	0.00f,	0x3f };

EAXLISTENERPROPERTIES FinishEAX3 =
	{26,	100.0f,	1.0f,	0,		-1000,	-2200,	20.0f,	1.39f,	1.00f,	1000,	0.069f,	0.00f,0.00f,0.00f,	400,	0.100f,		0.00f,0.00f,0.00f,	0.250f,	1.000f,	3.982f,	0.000f,	-18.0f,	3530.8f,	417.9f,	6.70f,	0x3f };

EAXLISTENERPROPERTIES EAX3Params;


bool IsFXSupported(void)
{
	return _effectsSupported; // usingEAX || usingEAX3 || _usingEFX;
}

void EAX_SetAll(const EAXLISTENERPROPERTIES *allparameters)
{
	if ( usingEAX || usingEAX3 )
		EAX3_Set(ALEffect, allparameters);
	else
		EFX_Set(ALEffect, allparameters);
}

static ALDeviceList cachedDeviceList;
static bool cachedDeviceListInitialized = false;
static void ReleaseCachedDeviceList(void)
{
    // Retain provider name pointers until the original process-exit cleanup point
    if(cachedDeviceListInitialized) {
        // Release cached enumeration once after all provider use ends
        ALDeviceList_Destroy(&cachedDeviceList);
        cachedDeviceListInitialized = false;
    }
}
static void
add_providers()
{
	SampleManager_SetNum3DProvidersAvailable(&SampleManager, 0);

    // Replace the function-static C++ constructor and destructor with explicit C lifetime
    if(!cachedDeviceListInitialized) {
        // Enumerate once and register cleanup while provider IDs keep borrowed name pointers
        ALDeviceList_Init(&cachedDeviceList);
        if(atexit(ReleaseCachedDeviceList) != 0) abort();
        cachedDeviceListInitialized = true;
    }
    ALDeviceList *pDeviceList = &cachedDeviceList;

	if ((pDeviceList) && (ALDeviceList_GetNumDevices(pDeviceList)))
	{
		const int devNumber = Min(ALDeviceList_GetNumDevices(pDeviceList), MAXPROVIDERS);
		int n = 0;
		
		//for (int i = 0; i < devNumber; i++) 
		int i = ALDeviceList_GetDefaultDevice(pDeviceList);
		{
			if ( n < MAXPROVIDERS )
			{ 
				providers[n].id = ALDeviceList_GetDeviceName(pDeviceList, i);
				strcpy(providers[n].name, "OPENAL SOFT");
				providers[n].sources = ALDeviceList_GetMaxNumSources(pDeviceList, i);
				SampleManager_Set3DProviderName(&SampleManager, n, providers[n].name);
				n++;
			}

			if ( alGetEnumValue("AL_EFFECT_EAXREVERB") != 0
				|| ALDeviceList_IsExtensionSupported(pDeviceList, i, ADEXT_EAX2)
				|| ALDeviceList_IsExtensionSupported(pDeviceList, i, ADEXT_EAX3) 
				|| ALDeviceList_IsExtensionSupported(pDeviceList, i, ADEXT_EAX4)
				|| ALDeviceList_IsExtensionSupported(pDeviceList, i, ADEXT_EAX5) )
			{ 
				providers[n - 1].bSupportsFx = true;
				if ( n < MAXPROVIDERS )
				{ 
					providers[n].id = ALDeviceList_GetDeviceName(pDeviceList, i);
					strcpy(providers[n].name, "OPENAL SOFT EAX");
					providers[n].sources = ALDeviceList_GetMaxNumSources(pDeviceList, i);
					providers[n].bSupportsFx = true;
					SampleManager_Set3DProviderName(&SampleManager, n, providers[n].name);
					n++;
				}
				
				if ( n < MAXPROVIDERS )
				{ 
					providers[n].id = ALDeviceList_GetDeviceName(pDeviceList, i);
					strcpy(providers[n].name, "OPENAL SOFT EAX3");
					providers[n].sources = ALDeviceList_GetMaxNumSources(pDeviceList, i);
					providers[n].bSupportsFx = true;
					SampleManager_Set3DProviderName(&SampleManager, n, providers[n].name);
					n++;
				}
			}
		}
		SampleManager_SetNum3DProvidersAvailable(&SampleManager, n);
	
		for(int j=n;j<MAXPROVIDERS;j++)
			SampleManager_Set3DProviderName(&SampleManager, j, NULL);

		// devices are gone now
		//defaultProvider = ALDeviceList_GetDefaultDevice(pDeviceList);
		//if ( defaultProvider > MAXPROVIDERS )
		defaultProvider = 0;
	}
}

static void
release_existing()
{
	if ( IsFXSupported() )
	{
		if ( AudioEFX_alIsEffect(ALEffect) )
		{
			AudioEFX_alEffecti(ALEffect, AL_EFFECT_TYPE, AL_EFFECT_NULL);
		}
		
		if (AudioEFX_alIsAuxiliaryEffectSlot(ALEffectSlot))
		{
			AudioEFX_alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
		}
	}

	DEV("release_existing()\n");
}

static bool8
set_new_provider(int index)
{
	if ( curprovider == index )
		return TRUE;
	
	curprovider = index;
	
	release_existing();
	
	if ( curprovider != -1 )
	{
		DEV("set_new_provider()\n");
		
		usingEAX = 0;
		usingEAX3 = 0;
		_usingEFX = false;
		
		if ( !strcmp(&providers[index].name[strlen(providers[index].name) - strlen(" EAX3")], " EAX3") 
				&& alcIsExtensionPresent(ALDevice, (ALCchar*)ALC_EXT_EFX_NAME) )
		{
			
			usingEAX = 1;
			usingEAX3 = 1;
			AudioEFX_alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			EAX_SetAll(&FinishEAX3);

			DEV("EAX3\n");
		}
		else if ( alcIsExtensionPresent(ALDevice, (ALCchar*)ALC_EXT_EFX_NAME) )
		{
			
			if ( !strcmp(&providers[index].name[strlen(providers[index].name) - strlen(" EAX")], " EAX"))
			{
				usingEAX = 1;
				DEV("EAX1\n");
			}
			else
			{
				_usingEFX = true;
				DEV("EFX\n");
			}
			AudioEFX_alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			EAX_SetAll(&EAX30_ORIGINAL_PRESETS[EAX_ENVIRONMENT_CAVE]);
		}
		
		//SampleManager_SetSpeakerConfig(&SampleManager, speaker_type);
		
		if ( IsFXSupported() )
		{
			for ( int32 i = 0; i < MAXCHANNELS; i++ )
				CChannel_SetReverbMix(&aChannel[i], ALEffectSlot, 0.0f);
		}
		
		return TRUE;
	}
	
	return FALSE;
}

static bool8
IsThisTrackAt16KHz(uint32 track)
{
	return track == STREAMED_SOUND_RADIO_KCHAT || track == STREAMED_SOUND_RADIO_VCPR || track == STREAMED_SOUND_RADIO_POLICE;
}





void SampleManager_SetSpeakerConfig(cSampleManager *manager, int32 nConfig)
{
    // Operate on the explicitly supplied sample-manager state


}

uint32 SampleManager_GetMaximumSupportedChannels(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if ( _maxSamples > MAXCHANNELS )
		return MAXCHANNELS;
	
	return _maxSamples;
}

uint32 SampleManager_GetNum3DProvidersAvailable(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return manager->m_nNumberOfProviders;
}

void SampleManager_SetNum3DProvidersAvailable(cSampleManager *manager, uint32 num)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_nNumberOfProviders = num;
}

char *SampleManager_Get3DProviderName(cSampleManager *manager, uint8 id)
{
    // Operate on the explicitly supplied sample-manager state

	return manager->m_aAudioProviders[id];
}

void SampleManager_Set3DProviderName(cSampleManager *manager, uint8 id, char *name)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_aAudioProviders[id] = name;
}

int8 SampleManager_GetCurrent3DProviderIndex(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return curprovider;
}

int8 SampleManager_SetCurrent3DProvider(cSampleManager *manager, uint8 nProvider)
{
    // Operate on the explicitly supplied sample-manager state

	int savedprovider = curprovider;

	nProvider = Clamp(nProvider, 0, manager->m_nNumberOfProviders - 1);

	if ( set_new_provider(nProvider) )
		return curprovider;
	else if ( savedprovider != -1 && savedprovider < manager->m_nNumberOfProviders && set_new_provider(savedprovider) )
		return curprovider;
	else
		return curprovider;
}

int8
SampleManager_AutoDetect3DProviders(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if (!AudioSample_IsAudioInitialised())
		return -1;

	if (defaultProvider >= 0 && defaultProvider < manager->m_nNumberOfProviders) {
		if (set_new_provider(defaultProvider))
			return defaultProvider;
	}

	for (uint32 i = 0; i < SampleManager_GetNum3DProvidersAvailable(manager); i++)
	{
		char* providername = SampleManager_Get3DProviderName(manager, i);

		if (!strcasecmp(providername, "OPENAL SOFT")) {
			SampleManager_SetCurrent3DProvider(manager, i);
			if (SampleManager_GetCurrent3DProviderIndex(manager) == i)
				return i;
		}
	}

	return -1;
}

static bool8
_ResolveLink(char const *path, char *out)
{
#ifdef _WIN32
	// Resolve Windows shortcuts through the C COM interface
	size_t len = strlen(path);
	if (len < 4 || strcmp(&path[len - 4], ".lnk") != 0)
		return FALSE;
		
	IShellLinkA* psl;
	WIN32_FIND_DATA fd;
	char filepath[MAX_PATH];
	
	CoInitialize(NULL);
									   
	if (SUCCEEDED( CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkA, (LPVOID*)&psl ) ))
	{
		IPersistFile *ppf;

		if (SUCCEEDED(IShellLinkA_QueryInterface(psl, &IID_IPersistFile, (LPVOID*)&ppf)))
		{
			WCHAR wpath[MAX_PATH];
			
			MultiByteToWideChar(CP_ACP, 0, path, -1, wpath, MAX_PATH);
			
			if (SUCCEEDED(IPersistFile_Load(ppf, wpath, STGM_READ)))
			{
				/* Resolve the link */
				if (SUCCEEDED(IShellLinkA_Resolve(psl, NULL, SLR_ANY_MATCH|SLR_NO_UI|SLR_NOSEARCH)))
				{
					strcpy(filepath, path);
					
					if (SUCCEEDED(IShellLinkA_GetPath(psl, filepath, MAX_PATH, &fd, SLGP_UNCPRIORITY)))
					{
						OutputDebugString(fd.cFileName);
						
						strcpy(out, filepath);
						// FIX: Release the objects. Taken from SA.
#ifdef FIX_BUGS
						IPersistFile_Release(ppf);
						IShellLinkA_Release(psl);
#endif
						return TRUE;
					}
				}
			}
			
			IPersistFile_Release(ppf);
		}
		IShellLinkA_Release(psl);
	}
	
	return FALSE;
#else
	struct stat sb;

	if (lstat(path, &sb) == -1) {
		perror("lstat: ");
		return FALSE;
	}

	if (S_ISLNK(sb.st_mode)) {
		char* linkname = (char*)alloca(sb.st_size + 1);
		if (linkname == NULL) {
			fprintf(stderr, "insufficient memory\n");
			return FALSE;
		}

		if (readlink(path, linkname, sb.st_size + 1) < 0) {
			perror("readlink: ");
			return FALSE;
		}
		linkname[sb.st_size] = '\0';
		strcpy(out, linkname);
		return TRUE;
	} else {
		return FALSE;
	}
#endif
}

static void
_FindMP3s(void)
{
	tMP3Entry *pList;
	bool8 bShortcut;	
	bool8 bInitFirstEntry;	
	HANDLE hFind;
	char path[MAX_PATH];
	int total_ms;
	WIN32_FIND_DATA fd;
	char filepath[MAX_PATH + sizeof(fd.cFileName)];
	
	// Retain the platform working-directory query through its explicit CRT or POSIX name
	if (_getcwd(_mp3DirectoryPath, MAX_PATH) == NULL) {
		perror("getcwd: ");
		return;
	}

	if (strlen(_mp3DirectoryPath) + 1 > MAX_PATH - 10) {
		// This is not gonna end well
		printf("MP3 folder path is too long, no place left for file names. MP3 finding aborted.\n");
		return;
	}
	
	OutputDebugString("Finding MP3s...");
	strcpy(path, _mp3DirectoryPath);
	strcat(path, "\\MP3\\");

#if !defined(_WIN32)
	char *actualPath = casepath(path);
	if (actualPath) {
		strcpy(path, actualPath);
		free(actualPath);
	}
#endif
	
	strcpy(_mp3DirectoryPath, path);
	OutputDebugString(_mp3DirectoryPath);
	
	strcat(path, "*");
	
	hFind = FindFirstFile(path, &fd);
	
	if ( hFind == INVALID_HANDLE_VALUE ) 
	{
		return;
	}

	bShortcut = FALSE;
	bInitFirstEntry = TRUE;

	do
	{	
		strcpy(filepath, _mp3DirectoryPath);
		strcat(filepath, fd.cFileName);
			
		if (!strcmp(fd.cFileName, ".") || !strcmp(fd.cFileName, ".."))
			continue;

		size_t filepathlen = strlen(filepath);

		if ( bInitFirstEntry )
		{
			if (filepathlen > 0)
			{
				if (_ResolveLink(filepath, filepath))
				{
					OutputDebugString("Resolving Link");
					OutputDebugString(filepath);
					bShortcut = TRUE;
				}
				else
				{
					bShortcut = FALSE;
					if (filepathlen > MAX_PATH) {
						continue;
					}
				}
				if (aStream[0] && CStream_Open(aStream[0], filepath, 32000))
				{
					total_ms = CStream_GetLengthMS(aStream[0]);
					CStream_Close(aStream[0]);
					
					OutputDebugString(fd.cFileName);

					_pMP3List = (tMP3Entry*)malloc(sizeof(tMP3Entry)); // rouz edit (ChatGPT)

					if (_pMP3List == NULL)
						break;

					nNumMP3s = 1;

					strcpy(_pMP3List->aFilename, fd.cFileName);

					_pMP3List->nTrackLength = total_ms;
					_pMP3List->pNext = NULL;

					if (bShortcut)
					{
						_pMP3List->pLinkPath = (char*)malloc(MAX_PATH + sizeof(fd.cFileName)); // rouz edit (ChatGPT)
						strcpy(_pMP3List->pLinkPath, filepath);
					}
					else
					{
						_pMP3List->pLinkPath = NULL;
					}

					pList = _pMP3List;

					bInitFirstEntry = FALSE;
				}
				else
				{
					strcat(filepath, " - NOT A VALID MP3");
					OutputDebugString(filepath);
				}
			}
			else
				break;
		}
		else
		{
			if ( filepathlen > 0 )
			{
				if ( _ResolveLink(filepath, filepath) )
				{
					OutputDebugString("Resolving Link");
					OutputDebugString(filepath);
					bShortcut = TRUE;
				}
				else
					bShortcut = FALSE;
				
				if (aStream[0] && CStream_Open(aStream[0], filepath, 32000))
				{
					total_ms = CStream_GetLengthMS(aStream[0]);
					CStream_Close(aStream[0]);

					OutputDebugString(fd.cFileName);
					
					pList->pNext = (tMP3Entry*)malloc(sizeof(tMP3Entry)); // rouz edit (ChatGPT)
					
					tMP3Entry *e = pList->pNext;
					
					if ( e == NULL )
						break;
					
					pList = pList->pNext;
					
					strcpy(e->aFilename, fd.cFileName);
					e->nTrackLength = total_ms;
					e->pNext = NULL;
					
					if ( bShortcut )
					{
						e->pLinkPath = (char*)malloc(MAX_PATH + sizeof(fd.cFileName)); // rouz edit (ChatGPT)
						strcpy(e->pLinkPath, filepath);
					}
					else
					{
						e->pLinkPath = NULL;
					}
					
					nNumMP3s++;
					
					OutputDebugString(fd.cFileName);
				}
				else
				{
					strcat(filepath, " - NOT A VALID MP3");
					OutputDebugString(filepath);
				}
			}
		}
	} while (FindNextFile(hFind, &fd));

	FindClose(hFind);
}

static void
_DeleteMP3Entries(void)
{
	tMP3Entry *e = _pMP3List;

	while ( e != NULL )
	{
		tMP3Entry *next = e->pNext;
		
		if ( next == NULL )
			next = NULL;
		
		if ( e->pLinkPath != NULL )
		{
#ifndef FIX_BUGS
			free(e->pLinkPath); // rouz edit (ChatGPT)
#else
			free(e->pLinkPath); // rouz edit (ChatGPT)
#endif
			e->pLinkPath = NULL;
		}
		
		free(e); // rouz edit (ChatGPT)
		
		if ( next )
			e = next;
		else
			e = NULL;
		
		nNumMP3s--;
	}
	
	
	if ( nNumMP3s != 0 )
	{
		OutputDebugString("Not all MP3 entries were deleted");
		nNumMP3s = 0;
	}
	
	_pMP3List = NULL;
}

static tMP3Entry *
_GetMP3EntryByIndex(uint32 idx)
{
	uint32 n = ( idx < nNumMP3s ) ? idx : 0;
	
	if ( _pMP3List != NULL )
	{
		tMP3Entry *e = _pMP3List;
		
		for ( uint32 i = 0; i < n; i++ )
			e = e->pNext;
		
		return e;
			
	}
	
	return NULL;
}

static inline bool8
_GetMP3PosFromStreamPos(uint32 *pPosition, tMP3Entry **pEntry)
{
	_CurMP3Index = 0;
	
	for ( *pEntry = _pMP3List; *pEntry != NULL; *pEntry = (*pEntry)->pNext )
	{
		if (   *pPosition >= (*pEntry)->nTrackStreamPos
			&& *pPosition <  (*pEntry)->nTrackLength + (*pEntry)->nTrackStreamPos )
		{
			*pPosition -= (*pEntry)->nTrackStreamPos;
			_CurMP3Pos = *pPosition;
			
			return TRUE;
		}
		
		_CurMP3Index++;
	}
				
	*pPosition = 0;
	*pEntry = _pMP3List;
	_CurMP3Pos = 0;
	_CurMP3Index = 0;
	
	return FALSE;
}

bool8
SampleManager_IsMP3RadioChannelAvailable(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return nNumMP3s != 0;
}


void SampleManager_ReleaseDigitalHandle(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	// TODO? alcSuspendContext
}

void SampleManager_ReacquireDigitalHandle(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	// TODO? alcProcessContext
}

bool8
SampleManager_Initialise(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if ( _bSampmanInitialised )
		return TRUE;

    // Replace static C++ channel constructors with one initialization at the owning call site
    static bool channelStatesInitialized = false;
    if(!channelStatesInitialized) {
        // Initialize every embedded channel before the audio provider can use it
        for(int i = 0; i < NUM_CHANNELS; i++) CChannel_InitializeState(&aChannel[i]);
        channelStatesInitialized = true;
    }
	EFXInit();

	for(int i = 0; i < MAX_STREAMS; i++) {
		// Construct the stream without invoking C++ new.
		aStream[i] = (CStream*)malloc(sizeof(CStream));
		// Initialize the explicitly allocated stream without C++ construction
		if(!aStream[i]) abort();
		CStream_Init(aStream[i], ALStreamSources[i], ALStreamBuffers[i]);
	}
	CStream_Initialise();

	{
		for ( int32 i = 0; i < TOTAL_AUDIO_SAMPLES; i++ )
		{
			manager->m_aSamples[i].nOffset    = 0;
			manager->m_aSamples[i].nSize      = 0;
			manager->m_aSamples[i].nFrequency = 22050;
			manager->m_aSamples[i].nLoopStart = 0;
			manager->m_aSamples[i].nLoopEnd   = -1;
		}
		
		manager->m_nEffectsVolume     = MAX_VOLUME;
		manager->m_nMusicVolume       = MAX_VOLUME;
		manager->m_nEffectsFadeVolume = MAX_VOLUME;
		manager->m_nMusicFadeVolume   = MAX_VOLUME;
	
		manager->m_nMonoMode = 0;
	}
	
	{
		curprovider = -1;
		prevprovider = -1;
			
		_usingEFX = false;
		usingEAX =0;
		usingEAX3=0;
			
		_fEffectsLevel = 0.0f;
			
		_maxSamples = 0;
		
		ALDevice = NULL;
		ALContext = NULL;
	}
	
	{
		fpSampleDescHandle = NULL;
		fpSampleDataHandle = NULL;
		
		for ( int32 i = 0; i < MAX_SFX_BANKS; i++ )
		{
			gBankLoaded[i]                   = LOADING_STATUS_NOT_LOADED;
			nSampleBankDiscStartOffset[i]    = 0;
			nSampleBankSize[i]               = 0;
			nSampleBankMemoryStartAddress[i] = 0;
		}
	}
	
	{
		for ( int32 i = 0; i < MAX_PEDSFX; i++ )
		{
			nPedSlotSfx[i]     = NO_SAMPLE;
			nPedSlotSfxAddr[i] = 0;
		}
		
		nCurrentPedSlot = 0;
	}
	
	{
		for ( int32 i = 0; i < NUM_CHANNELS; i++ )
			nChannelVolume[i] = 0;
	}

	add_providers();

	{
		int index = 0;
		_maxSamples = Min(MAXCHANNELS, providers[index].sources);
		
		ALCint attr[] = {ALC_FREQUENCY,MAX_FREQ,
						ALC_MONO_SOURCES, MAX_DIGITAL_MIXER_CHANNELS - MAX2DCHANNELS,
						ALC_STEREO_SOURCES, MAX2DCHANNELS,
						0,
						};
		
		ALDevice  = alcOpenDevice(providers[index].id);
		ASSERT(ALDevice != NULL);
		
		ALContext = alcCreateContext(ALDevice, attr);
		ASSERT(ALContext != NULL);
		
		alcMakeContextCurrent(ALContext);
	
		const char* ext=(const char*)alGetString(AL_EXTENSIONS);
		ASSERT(strstr(ext,"AL_SOFT_loop_points")!=NULL);
		if ( strstr(ext,"AL_SOFT_loop_points")==NULL )
		{
			SampleManager_Terminate(manager);
			return FALSE;
		}
		
		alListenerf (AL_GAIN,     1.0f);
		alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
		alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		ALfloat orientation[6] = { 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
		alListenerfv(AL_ORIENTATION, orientation);
		
		alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
		
		if ( alcIsExtensionPresent(ALDevice, (ALCchar*)ALC_EXT_EFX_NAME) )
		{ 
			_effectsSupported = providers[index].bSupportsFx;
			AudioEFX_alGenAuxiliaryEffectSlots(1, &ALEffectSlot);
			AudioEFX_alGenEffects(1, &ALEffect);
		}

		alGenSources(MAX_STREAMS*2, ALStreamSources[0]);
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			alGenBuffers(NUM_STREAMBUFFERS, ALStreamBuffers[i]);
			alSourcei(ALStreamSources[i][0], AL_SOURCE_RELATIVE, AL_TRUE);
			alSource3f(ALStreamSources[i][0], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef(ALStreamSources[i][0], AL_GAIN, 1.0f);
			alSourcei(ALStreamSources[i][1], AL_SOURCE_RELATIVE, AL_TRUE);
			alSource3f(ALStreamSources[i][1], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSourcef(ALStreamSources[i][1], AL_GAIN, 1.0f);
		} 
		
		CChannel_InitChannels();

		for ( int32 i = 0; i < MAXCHANNELS; i++ )
			CChannel_Init(&aChannel[i], i, false);
		for ( int32 i = 0; i < MAX2DCHANNELS; i++ )
			CChannel_Init(&aChannel[MAXCHANNELS+i], MAXCHANNELS+i, true);
		
		if ( IsFXSupported() )
		{
			/**/
			AudioEFX_alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			/**/
			
			for ( int32 i = 0; i < MAXCHANNELS; i++ )
				CChannel_SetReverbMix(&aChannel[i], ALEffectSlot, 0.0f);
		}
	}

	{	
		for ( int32 i = 0; i < TOTAL_STREAMED_SOUNDS; i++ )
			nStreamLength[i] = 0;
	}

#ifdef AUDIO_CACHE
	FILE *cacheFile = fcaseopen("audio\\sound.cache", "rb");
	if (cacheFile) {
		debug("Loadind audio cache (If game crashes around here, then your cache is corrupted, remove audio/sound.cache)\n");
		fread(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
		fclose(cacheFile);
	} else
	{
		debug("Cannot load audio cache\n");
#endif

		for ( int32 i = 0; i < TOTAL_STREAMED_SOUNDS; i++ )
		{	
			if ( aStream[0] && (
#ifdef PS2_AUDIO_PATHS
				CStream_Open(aStream[0], PS2StreamedNameTable[i], IsThisTrackAt16KHz(i) ? 16000 : 32000) || 
#endif
				CStream_Open(aStream[0], StreamedNameTable[i], IsThisTrackAt16KHz(i) ? 16000 : 32000)) )
			{
				uint32 tatalms = CStream_GetLengthMS(aStream[0]);
				CStream_Close(aStream[0]);
				
				nStreamLength[i] = tatalms;
			} else
				USERERROR("Can't open '%s'\n", StreamedNameTable[i]);
		}
#ifdef AUDIO_CACHE
		cacheFile = fcaseopen("audio\\sound.cache", "wb");
		if(cacheFile) {
			debug("Saving audio cache\n");
			fwrite(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
			fclose(cacheFile);
		} else {
			debug("Cannot save audio cache\n");
		}
	}
#endif

	{
		if ( !SampleManager_InitialiseSampleBanks(manager) )
		{
			SampleManager_Terminate(manager);
			return FALSE;
		}
		
		nSampleBankMemoryStartAddress[SFX_BANK_0] = (uintptr)malloc(nSampleBankSize[SFX_BANK_0]);
		ASSERT(nSampleBankMemoryStartAddress[SFX_BANK_0] != 0);
		
		if ( nSampleBankMemoryStartAddress[SFX_BANK_0] == 0 )
		{
			SampleManager_Terminate(manager);
			return FALSE;
		}
		
		nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = (uintptr)malloc(PED_BLOCKSIZE*MAX_PEDSFX);
		ASSERT(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] != 0);

#ifdef FIX_BUGS
		// Find biggest player comment
		uint32 nMaxPedSize = 0;
		for (uint32 i = PLAYER_COMMENTS_START; i <= PLAYER_COMMENTS_END; i++)
			nMaxPedSize = Max(nMaxPedSize, manager->m_aSamples[i].nSize);

		gPlayerTalkData = malloc(nMaxPedSize);
		ASSERT(gPlayerTalkData != 0);
#endif

		SampleManager_LoadSampleBank(manager, SFX_BANK_0);
	}
	
	{
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			CStream_Close(aStream[i]);

			nStreamVolume[i] = 100;
			nStreamPan[i]    = 63;
		}
	}

	{
		_bSampmanInitialised = TRUE;
		
		if ( defaultProvider >= 0 && defaultProvider < manager->m_nNumberOfProviders )
		{
			set_new_provider(defaultProvider);
		}
		else
		{
			SampleManager_Terminate(manager);
			return FALSE;
		}
	}

	{
		nNumMP3s = 0;
		
		_pMP3List = NULL;
		
		_FindMP3s();
		
		if ( nNumMP3s != 0 )
		{
			nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER] = 0;
			
			for ( tMP3Entry *e = _pMP3List; e != NULL; e = e->pNext )
			{
				e->nTrackStreamPos = nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER];
				nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER] += e->nTrackLength;
			}
			
			time_t t = time(NULL);
			struct tm *localtm;
			bool8 bUseRandomTable;
			
			if ( t == -1 )
				bUseRandomTable = TRUE;
			else
			{
				bUseRandomTable = FALSE;
				localtm = localtime(&t);
			}
			
			int32 randval;
			if ( bUseRandomTable )
				randval = AudioSample_GetRandomValue(1);
			else
				randval = localtm->tm_sec * localtm->tm_min;
			
			_CurMP3Index = randval % nNumMP3s;
			
			tMP3Entry *randmp3 = _pMP3List;
			for ( int32 i = randval % nNumMP3s; i > 0; --i)
				randmp3 = randmp3->pNext;
			
			if ( bUseRandomTable )
				_CurMP3Pos = AudioSample_GetRandomValue(0)     % randmp3->nTrackLength;
			else
			{
				if ( localtm->tm_sec > 0 )
				{
					int32 s = localtm->tm_sec;
					_CurMP3Pos = s*s*s*s*s*s*s*s                 % randmp3->nTrackLength;
				}
				else
					_CurMP3Pos = AudioSample_GetRandomValue(0) % randmp3->nTrackLength;
			}
		}
		else
			_CurMP3Pos = 0;
		
		_bIsMp3Active = FALSE;
	}
	
	return TRUE;
}

void
SampleManager_Terminate(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	for (int32 i = 0; i < MAX_STREAMS; i++)
		CStream_Close(aStream[i]);

	// Finish decoding and pending stream cleanup before destroying OpenAL resources
	CStream_Terminate();
	for ( int32 i = 0; i < NUM_CHANNELS; i++ )
		CChannel_Term(&aChannel[i]);

	if ( IsFXSupported() )
	{
		if ( AudioEFX_alIsEffect(ALEffect) )
		{
			AudioEFX_alEffecti(ALEffect, AL_EFFECT_TYPE, AL_EFFECT_NULL);
			AudioEFX_alDeleteEffects(1, &ALEffect);
			ALEffect = AL_EFFECT_NULL;
		}
		
		if (AudioEFX_alIsAuxiliaryEffectSlot(ALEffectSlot))
		{
			AudioEFX_alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, AL_EFFECT_NULL);
			
			AudioEFX_alDeleteAuxiliaryEffectSlots(1, &ALEffectSlot);
			ALEffectSlot = AL_EFFECTSLOT_NULL;
		}
	}

	for ( int32 i = 0; i < MAX_STREAMS; i++ )
	{
		alDeleteBuffers(NUM_STREAMBUFFERS, ALStreamBuffers[i]);
	}
	alDeleteSources(MAX_STREAMS*2, ALStreamSources[0]);
	
	CChannel_DestroyChannels();
	
	if ( ALContext )
	{
		alcMakeContextCurrent(NULL);
		alcSuspendContext(ALContext);
		alcDestroyContext(ALContext);
	}
	if ( ALDevice )
		alcCloseDevice(ALDevice);
	
	ALDevice = NULL;
	ALContext = NULL;
	
	_fPrevEaxRatioDestination = 0.0f;
	_usingEFX                 = false;
	_fEffectsLevel            = 0.0f;
	
	_DeleteMP3Entries();

	// Stream shutdown completed before OpenAL teardown // rouz edit (ChatGPT)

	for(int32 i = 0; i < MAX_STREAMS; i++) {
		// Destroy and release the stream without invoking C++ delete.
		if(aStream[i]){
			CStream_Destroy(aStream[i]);
			free(aStream[i]);
			aStream[i] = nil;
		}
	}
	if ( nSampleBankMemoryStartAddress[SFX_BANK_0] != 0 )
	{
		free((void *)nSampleBankMemoryStartAddress[SFX_BANK_0]);
		nSampleBankMemoryStartAddress[SFX_BANK_0] = 0;
	}

	if ( nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] != 0 )
	{
		free((void *)nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS]);
		nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = 0;
	}

#ifdef FIX_BUGS
	if ( gPlayerTalkData != 0 )
	{
		free(gPlayerTalkData);
		gPlayerTalkData = 0;
	}
#endif
	
	_bSampmanInitialised = FALSE;
}

bool8 SampleManager_CheckForAnAudioFileOnCD(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return TRUE;
}

char SampleManager_GetCDAudioDriveLetter(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return '\0';
}

void
SampleManager_UpdateEffectsVolume(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if ( _bSampmanInitialised )
	{
		for ( int32 i = 0; i < NUM_CHANNELS; i++ )
		{
			if ( SampleManager_GetChannelUsedFlag(manager, i) )
			{
				if ( nChannelVolume[i] != 0 )
					CChannel_SetVolume(&aChannel[i], manager->m_nEffectsFadeVolume*nChannelVolume[i]*manager->m_nEffectsVolume >> 14);
			}
		}
	}
}

void
SampleManager_SetEffectsMasterVolume(cSampleManager *manager, uint8 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_nEffectsVolume = nVolume;
	SampleManager_UpdateEffectsVolume(manager);
}

void
SampleManager_SetMusicMasterVolume(cSampleManager *manager, uint8 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_nMusicVolume = nVolume;
}

void
SampleManager_SetMP3BoostVolume(cSampleManager *manager, uint8 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_nMP3BoostVolume = nVolume;
}

void
SampleManager_SetEffectsFadeVolume(cSampleManager *manager, uint8 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_nEffectsFadeVolume = nVolume;
	SampleManager_UpdateEffectsVolume(manager);
}

void
SampleManager_SetMusicFadeVolume(cSampleManager *manager, uint8 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_nMusicFadeVolume = nVolume;
}

void
SampleManager_SetMonoMode(cSampleManager *manager, bool8 nMode)
{
    // Operate on the explicitly supplied sample-manager state

	manager->m_nMonoMode = nMode;
}

bool8
SampleManager_LoadSampleBank(cSampleManager *manager, uint8 nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nBank < MAX_SFX_BANKS);
	
	if ( AudioSample_IsCodePaused() )
		return FALSE;
	
	if ( AudioSample_IsMusicInitialised()
		&& AudioSample_GetMusicMode() == MUSICMODE_CUTSCENE
		&& nBank != SFX_BANK_0 )
	{
		return FALSE;
	}
	
#ifdef OPUS_SFX
	int samplesRead = 0;
	int samplesSize = nSampleBankSize[nBank] / 2;
	op_pcm_seek(fpSampleDataHandle, 0);
	while (samplesSize > 0) {
		int size = op_read(fpSampleDataHandle, (opus_int16 *)(nSampleBankMemoryStartAddress[nBank] + samplesRead), samplesSize, NULL);
		if (size <= 0) {
			// huh?
			//assert(0);
			break;
		}
		samplesRead += size*2;
		samplesSize -= size;
	}
#else
	if ( fseek(fpSampleDataHandle, nSampleBankDiscStartOffset[nBank], SEEK_SET) != 0 )
		return FALSE;
	
	if ( fread((void *)nSampleBankMemoryStartAddress[nBank], 1, nSampleBankSize[nBank], fpSampleDataHandle) != nSampleBankSize[nBank] )
		return FALSE;
#endif
	gBankLoaded[nBank] = LOADING_STATUS_LOADED;
	
	return TRUE;
}

void
SampleManager_UnloadSampleBank(cSampleManager *manager, uint8 nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nBank < MAX_SFX_BANKS);
	
	gBankLoaded[nBank] = LOADING_STATUS_NOT_LOADED;
}

int8
SampleManager_IsSampleBankLoaded(cSampleManager *manager, uint8 nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nBank < MAX_SFX_BANKS);
	
	return gBankLoaded[nBank];
}

#ifdef FIX_BUGS
uint8
SampleManager_IsMissionAudioLoaded(cSampleManager *manager, uint8 nSlot, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT(nSlot == MISSION_AUDIO_PLAYER_COMMENT); // only MISSION_AUDIO_PLAYER_COMMENT is supported on PC
	
	return nSample == gPlayerTalkSfx ? LOADING_STATUS_LOADED : LOADING_STATUS_NOT_LOADED;
}

bool8
SampleManager_LoadMissionAudio(cSampleManager *manager, uint8 nSlot, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT(nSlot == MISSION_AUDIO_PLAYER_COMMENT); // only MISSION_AUDIO_PLAYER_COMMENT is supported on PC
	ASSERT(nSample < TOTAL_AUDIO_SAMPLES);
	
#ifdef OPUS_SFX
	// Read mission speech as PCM samples when the sample-data handle is an Opus decoder
	const tSample *sample = &manager->m_aSamples[nSample];
	if ((sample->nOffset % sizeof(opus_int16)) || (sample->nSize % sizeof(opus_int16)))
		return FALSE;
	if (op_pcm_seek(fpSampleDataHandle, sample->nOffset / sizeof(opus_int16)) != 0)
		return FALSE;
	int remaining = sample->nSize / sizeof(opus_int16);
	opus_int16 *output = (opus_int16 *)gPlayerTalkData;
	while (remaining > 0) {
		// Advance only after a successful decode and reject truncated mission speech
		int count = op_read(fpSampleDataHandle, output, remaining, NULL);
		if (count <= 0)
			return FALSE;
		output += count;
		remaining -= count;
	}
#else
	if (fseek(fpSampleDataHandle, manager->m_aSamples[nSample].nOffset, SEEK_SET) != 0)
		return FALSE;

	if (fread(gPlayerTalkData, 1, manager->m_aSamples[nSample].nSize, fpSampleDataHandle) != manager->m_aSamples[nSample].nSize)
		return FALSE;
#endif

	gPlayerTalkSfx = nSample;

	return TRUE;
}
#endif

uint8
SampleManager_IsPedCommentLoaded(cSampleManager *manager, uint32 nComment)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );
	
	for ( int32 i = 0; i < _TODOCONST(3); i++ )
	{
#ifdef FIX_BUGS
		int8 slot = (int8)nCurrentPedSlot - i - 1;
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
#else
		uint8 slot = nCurrentPedSlot - i - 1;
#endif
		if ( nComment == nPedSlotSfx[slot] )
			return LOADING_STATUS_LOADED;
	}
	
	return LOADING_STATUS_NOT_LOADED;
}


int32
SampleManager__GetPedCommentSlot(cSampleManager *manager, uint32 nComment)
{
    // Operate on the explicitly supplied sample-manager state

	for (int32 i = 0; i < _TODOCONST(3); i++)
	{
#ifdef FIX_BUGS
		int8 slot = (int8)nCurrentPedSlot - i - 1;
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
#else
		uint8 slot = nCurrentPedSlot - i - 1;
#endif
		if (nComment == nPedSlotSfx[slot])
			return slot;
	}

	return -1;
}

bool8
SampleManager_LoadPedComment(cSampleManager *manager, uint32 nComment)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nComment < TOTAL_AUDIO_SAMPLES );
	
	if ( AudioSample_IsCodePaused() )
		return FALSE;
	
	// no talking peds during cutsenes or the game end
	if ( AudioSample_IsMusicInitialised() )
	{
		switch ( AudioSample_GetMusicMode() )
		{
			case MUSICMODE_CUTSCENE:
			{
				return FALSE;

				break;
			}
		}
	}

#ifdef OPUS_SFX
	int samplesRead = 0;
	int samplesSize = manager->m_aSamples[nComment].nSize / 2;
	op_pcm_seek(fpSampleDataHandle, manager->m_aSamples[nComment].nOffset / 2);
	while (samplesSize > 0) {
		// Decode into the same ped-comment bank used by the raw sample path
		int size = op_read(fpSampleDataHandle, (opus_int16 *)(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE * nCurrentPedSlot + samplesRead),
		                   samplesSize, NULL);
		if (size <= 0) {
			return FALSE;
		}
		samplesRead += size * 2;
		samplesSize -= size;
	}
#else
	if ( fseek(fpSampleDataHandle, manager->m_aSamples[nComment].nOffset, SEEK_SET) != 0 )
		return FALSE;
	
	if ( fread((void *)(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE*nCurrentPedSlot), 1, manager->m_aSamples[nComment].nSize, fpSampleDataHandle) != manager->m_aSamples[nComment].nSize )
		return FALSE;

#endif
	nPedSlotSfx[nCurrentPedSlot] = nComment;
		
	if ( ++nCurrentPedSlot >= MAX_PEDSFX )
		nCurrentPedSlot = 0;
	
	return TRUE;
}

int32
SampleManager_GetBankContainingSound(cSampleManager *manager, uint32 offset)
{
    // Operate on the explicitly supplied sample-manager state

	if ( offset >= BankStartOffset[SFX_BANK_PED_COMMENTS] )
		return SFX_BANK_PED_COMMENTS;
	
	if ( offset >= BankStartOffset[SFX_BANK_0] )
		return SFX_BANK_0;
	
	return INVALID_SFX_BANK;
}

uint32
SampleManager_GetSampleBaseFrequency(cSampleManager *manager, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return manager->m_aSamples[nSample].nFrequency;
}

uint32
SampleManager_GetSampleLoopStartOffset(cSampleManager *manager, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return manager->m_aSamples[nSample].nLoopStart;
}

int32
SampleManager_GetSampleLoopEndOffset(cSampleManager *manager, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return manager->m_aSamples[nSample].nLoopEnd;
}

uint32
SampleManager_GetSampleLength(cSampleManager *manager, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nSample < TOTAL_AUDIO_SAMPLES );
	return manager->m_aSamples[nSample].nSize / sizeof(uint16);
}

bool8 SampleManager_UpdateReverb(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if ( !usingEAX && !_usingEFX )
		return FALSE;

	if ( AudioSample_GetFrameCounter() & 15 )
		return FALSE;

	float fRatio = 0.0f;

#ifdef AUDIO_REFLECTIONS
#define MIN_DIST 0.5f
#define CALCULATE_RATIO(value, maxDist, maxRatio) (value > MIN_DIST && value < maxDist ? value / maxDist * maxRatio : 0)

	fRatio += CALCULATE_RATIO(AudioSample_GetReflectionDistances()[REFLECTION_CEIL_NORTH], 10.0f, 1/2.f);
	fRatio += CALCULATE_RATIO(AudioSample_GetReflectionDistances()[REFLECTION_CEIL_SOUTH], 10.0f, 1/2.f);
	fRatio += CALCULATE_RATIO(AudioSample_GetReflectionDistances()[REFLECTION_CEIL_WEST], 10.0f, 1/2.f);
	fRatio += CALCULATE_RATIO(AudioSample_GetReflectionDistances()[REFLECTION_CEIL_EAST], 10.0f, 1/2.f);

	fRatio += CALCULATE_RATIO((AudioSample_GetReflectionDistances()[REFLECTION_NORTH] + AudioSample_GetReflectionDistances()[REFLECTION_SOUTH]) / 2.f, 4.0f, 1/3.f);
	fRatio += CALCULATE_RATIO((AudioSample_GetReflectionDistances()[REFLECTION_WEST] + AudioSample_GetReflectionDistances()[REFLECTION_EAST]) / 2.f, 4.0f, 1/3.f);

#undef CALCULATE_RATIO
#undef MIN_DIST
#endif
	
	fRatio = Clamp(fRatio, 0.0f, 0.6f);
	
	if ( fRatio == _fPrevEaxRatioDestination )
		return FALSE;
	
#ifdef JUICY_OAL
	if ( usingEAX3 || _usingEFX )
#else
	if ( usingEAX3 )
#endif
	{
		fRatio = Min(fRatio * 1.67f, 1.0f);
		if ( EAX3ListenerInterpolate(&StartEAX3, &FinishEAX3, fRatio, &EAX3Params, false) )
		{
			EAX_SetAll(&EAX3Params);
			
			/*
			if ( IsFXSupported() )
			{
				AudioEFX_alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			
				for ( int32 i = 0; i < MAXCHANNELS; i++ )
					CChannel_UpdateReverb(&aChannel[i], ALEffectSlot);
			}
			*/
			
			_fEffectsLevel = fRatio * 0.75f;
		}
	}
	else
	{
		if ( _usingEFX )
			_fEffectsLevel = fRatio * 0.8f;
		else
			_fEffectsLevel = fRatio * 0.22f;
	}
	_fEffectsLevel = Min(_fEffectsLevel, 1.0f);

	_fPrevEaxRatioDestination = fRatio;
	
	return TRUE;
}

void
SampleManager_SetChannelReverbFlag(cSampleManager *manager, uint32 nChannel, bool8 nReverbFlag)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	if ( usingEAX || _usingEFX )
	{
		if ( IsFXSupported() )
		{
			AudioEFX_alAuxiliaryEffectSloti(ALEffectSlot, AL_EFFECTSLOT_EFFECT, ALEffect);
			
			if ( nReverbFlag != FALSE )
				CChannel_SetReverbMix(&aChannel[nChannel], ALEffectSlot, _fEffectsLevel);
			else
				CChannel_SetReverbMix(&aChannel[nChannel], ALEffectSlot, 0.0f);
		}
	}
}

bool8
SampleManager_InitialiseChannel(cSampleManager *manager, uint32 nChannel, uint32 nSfx, uint8 nBank)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	uintptr addr;
	
	if ( nSfx < SAMPLEBANK_MAX )
	{
		if ( !SampleManager_IsSampleBankLoaded(manager, nBank) )
			return FALSE;
		
		addr = nSampleBankMemoryStartAddress[nBank] + manager->m_aSamples[nSfx].nOffset - manager->m_aSamples[BankStartOffset[nBank]].nOffset;
	}
#ifdef FIX_BUGS
	else if ( nSfx >= PLAYER_COMMENTS_START && nSfx <= PLAYER_COMMENTS_END )
	{
		if ( !SampleManager_IsMissionAudioLoaded(manager, MISSION_AUDIO_PLAYER_COMMENT, nSfx) )
			return FALSE;

		addr = (uintptr)gPlayerTalkData;
	}
#endif
	else
	{
		int32 i;
		for ( i = 0; i < _TODOCONST(3); i++ )
		{
			int32 slot = nCurrentPedSlot - i - 1;
#ifdef FIX_BUGS
			if (slot < 0)
				slot += ARRAY_SIZE(nPedSlotSfx);
#endif
			if ( nSfx == nPedSlotSfx[slot] )
			{
				addr = (nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE * slot);
				break;
			}
		}

		if (i == _TODOCONST(3))
			return FALSE;
	}
	
	if ( SampleManager_GetChannelUsedFlag(manager, nChannel) )
	{
		TRACE("Stopping channel %d - really!!!", nChannel);
		SampleManager_StopChannel(manager, nChannel);
	}
	
	CChannel_Reset(&aChannel[nChannel]);
	if ( CChannel_HasSource(&aChannel[nChannel]) )
	{	
		CChannel_SetSampleData(&aChannel[nChannel], (void*)addr, manager->m_aSamples[nSfx].nSize, manager->m_aSamples[nSfx].nFrequency);
		CChannel_SetLoopPoints(&aChannel[nChannel], 0, -1);
		CChannel_SetPitch(&aChannel[nChannel], 1.0f);
		return TRUE;
	}
	
	return FALSE;
}

void
SampleManager_SetChannelEmittingVolume(cSampleManager *manager, uint32 nChannel, uint32 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS );
	
	uint32 vol = nVolume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
	
	nChannelVolume[nChannel] = vol;
	
	if (AudioSample_GetMusicMode() == MUSICMODE_CUTSCENE ) {
		if (AudioSample_GetCurrentTrack() == STREAMED_SOUND_CUTSCENE_FINALE)
			nChannelVolume[nChannel] = 0;
		else
			nChannelVolume[nChannel] >>= 2;
	}

	// no idea, does this one looks like a bug or it's SetChannelVolume ?
	CChannel_SetVolume(&aChannel[nChannel], manager->m_nEffectsFadeVolume*nChannelVolume[nChannel]*manager->m_nEffectsVolume >> 14);
}

void
SampleManager_SetChannel3DPosition(cSampleManager *manager, uint32 nChannel, float fX, float fY, float fZ)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS );
	
	CChannel_SetPosition(&aChannel[nChannel], -fX, fY, fZ);
}

void
SampleManager_SetChannel3DDistances(cSampleManager *manager, uint32 nChannel, float fMax, float fMin)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < MAXCHANNELS );
	CChannel_SetDistances(&aChannel[nChannel], fMax, fMin);
}

void
SampleManager_SetChannelVolume(cSampleManager *manager, uint32 nChannel, uint32 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel >= MAXCHANNELS );
	ASSERT( nChannel < NUM_CHANNELS );
	
	if ( nChannel == CHANNEL_POLICE_RADIO )
	{
		uint32 vol = nVolume;
		if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
		
		nChannelVolume[nChannel] = vol;
		
		// increase the volume for JB.MP3 and S4_BDBD.MP3
		if (AudioSample_GetMusicMode() == MUSICMODE_CUTSCENE ) {
			if (AudioSample_GetCurrentTrack() == STREAMED_SOUND_CUTSCENE_FINALE)
				nChannelVolume[nChannel] = 0;
			else
				nChannelVolume[nChannel] >>= 2;
		}

		CChannel_SetVolume(&aChannel[nChannel], manager->m_nEffectsFadeVolume*vol*manager->m_nEffectsVolume >> 14);
	}
}

void
SampleManager_SetChannelPan(cSampleManager *manager, uint32 nChannel, uint32 nPan)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel >= MAXCHANNELS );
	ASSERT( nChannel < NUM_CHANNELS );
	
	if ( nChannel == CHANNEL_POLICE_RADIO )
	{
		CChannel_SetPan(&aChannel[nChannel], nPan);
	}
}

void
SampleManager_SetChannelFrequency(cSampleManager *manager, uint32 nChannel, uint32 nFreq)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	CChannel_SetCurrentFreq(&aChannel[nChannel], nFreq);
}

void
SampleManager_SetChannelLoopPoints(cSampleManager *manager, uint32 nChannel, uint32 nLoopStart, int32 nLoopEnd)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	CChannel_SetLoopPoints(&aChannel[nChannel], nLoopStart / (DIGITALBITS / 8), nLoopEnd / (DIGITALBITS / 8));
}

void
SampleManager_SetChannelLoopCount(cSampleManager *manager, uint32 nChannel, uint32 nLoopCount)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	CChannel_SetLoopCount(&aChannel[nChannel], nLoopCount);
}

bool8
SampleManager_GetChannelUsedFlag(cSampleManager *manager, uint32 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	return CChannel_IsUsed(&aChannel[nChannel]);
}

void
SampleManager_StartChannel(cSampleManager *manager, uint32 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	CChannel_Start(&aChannel[nChannel]);
}

void
SampleManager_StopChannel(cSampleManager *manager, uint32 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nChannel < NUM_CHANNELS );
	
	CChannel_Stop(&aChannel[nChannel]);
}

void
SampleManager_PreloadStreamedFile(cSampleManager *manager, uint32 nFile, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state
	
	ASSERT( nStream < MAX_STREAMS );

	if ( nFile < TOTAL_STREAMED_SOUNDS )
	{
		CStream *stream = aStream[nStream];

		CStream_Close(stream);
#ifdef PS2_AUDIO_PATHS
		if(!CStream_Open(stream, PS2StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000))
#endif
			CStream_Open(stream, StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
		if ( !CStream_Setup(stream, false, true) )
		{
			CStream_Close(stream);
		}
	}
}

void
SampleManager_PauseStream(cSampleManager *manager, bool8 nPauseFlag, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( CStream_IsOpened(stream) )
	{
		CStream_SetPause(stream, nPauseFlag != FALSE);
	}
}

void
SampleManager_StartPreloadedStreamedFile(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( CStream_IsOpened(stream) )
	{
		CStream_Start(stream);
	}
}

bool8
SampleManager_StartStreamedFile(cSampleManager *manager, uint32 nFile, uint32 nPos, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	uint32 i = 0;
	uint32 position = nPos;
	char filename[MAX_PATH];
	
	if ( nFile >= TOTAL_STREAMED_SOUNDS )
		return FALSE;

	CStream_Close(aStream[nStream]);

	if ( nFile == STREAMED_SOUND_RADIO_MP3_PLAYER )
	{
		do
		{
			// Just switched to MP3 player
			if ( !_bIsMp3Active && i == 0 )
			{
				if ( nPos > nStreamLength[STREAMED_SOUND_RADIO_MP3_PLAYER] )
					position = 0;
				tMP3Entry *e = _pMP3List;

				// Try to continue from previous song, if already started
				if(!_GetMP3PosFromStreamPos(&position, &e) && !e) {
					nFile = 0;

					CStream *stream = aStream[nStream];
#ifdef PS2_AUDIO_PATHS
					if(!CStream_Open(stream, PS2StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000))
#endif
						CStream_Open(stream, StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
					if ( CStream_Setup(stream, false, true) ) {
						CStream_SetLoopCount(stream, nStreamLoopedFlag[nStream] ? 0 : 1);
						nStreamLoopedFlag[nStream] = TRUE;
						if (position != 0)
							CStream_SetPosMS(stream, position);

						CStream_Start(stream);

						return TRUE;
					} else {
						CStream_Close(stream);
					}
					return FALSE;

				} else {

					if (e->pLinkPath != NULL)
						CStream_Open(aStream[nStream], e->pLinkPath, IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
					else {
						strcpy(filename, _mp3DirectoryPath);
						strcat(filename, e->aFilename);

						CStream_Open(aStream[nStream], filename, 32000);
					}

					if (CStream_Setup(aStream[nStream], false, true)) {
						if (position != 0)
							CStream_SetPosMS(aStream[nStream], position);

						CStream_Start(aStream[nStream]);

						_bIsMp3Active = TRUE;
						return TRUE;
					} else {
						CStream_Close(aStream[nStream]);
					}
					// fall through, start playing from another song
				}
			} else {
				if(++_CurMP3Index >= nNumMP3s) _CurMP3Index = 0;

				_CurMP3Pos = 0;

				tMP3Entry *mp3 = _GetMP3EntryByIndex(_CurMP3Index);
				if ( !mp3 )
				{
					mp3 = _pMP3List;
					if ( !_pMP3List )
					{
						nFile = 0;
						_bIsMp3Active = FALSE;

						CStream *stream = aStream[nStream];
#ifdef PS2_AUDIO_PATHS
						if(!CStream_Open(stream, PS2StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000))
#endif
							CStream_Open(stream, StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);

						if (CStream_Setup(stream, false, true)) {
							CStream_SetLoopCount(stream, nStreamLoopedFlag[nStream] ? 0 : 1);
							nStreamLoopedFlag[nStream] = TRUE;
							if (position != 0)
								CStream_SetPosMS(stream, position);

							CStream_Start(stream);

							return TRUE;
						} else {
							CStream_Close(stream);
						}
						return FALSE;
					}
				}
				if (mp3->pLinkPath != NULL)
					CStream_Open(aStream[nStream], mp3->pLinkPath, IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
				else {
					strcpy(filename, _mp3DirectoryPath);
					strcat(filename, mp3->aFilename);

					CStream_Open(aStream[nStream], filename, IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
				}

				if (CStream_Setup(aStream[nStream], false, true)) {
					CStream_Start(aStream[nStream]);
#ifdef FIX_BUGS
					_bIsMp3Active = TRUE;
#endif
					return TRUE;
				} else {
					CStream_Close(aStream[nStream]);
				}

			}
			_bIsMp3Active = FALSE;
		}
		while ( ++i < nNumMP3s );
		position = 0;
		nFile = 0;
	}
	strcpy(filename, StreamedNameTable[nFile]);
	
	CStream *stream = aStream[nStream];

#ifdef PS2_AUDIO_PATHS
	if(!CStream_Open(stream, PS2StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000))
#endif
		CStream_Open(stream, StreamedNameTable[nFile], IsThisTrackAt16KHz(nFile) ? 16000 : 32000);
	
	if ( CStream_Setup(stream, false, true) ) {
		CStream_SetLoopCount(stream, nStreamLoopedFlag[nStream] ? 0 : 1);
		nStreamLoopedFlag[nStream] = TRUE;
		if (position != 0)
			CStream_SetPosMS(stream, position);	

		CStream_Start(stream);
		
		return TRUE;
	} else {
		CStream_Close(stream);
	}
	return FALSE;
}

void
SampleManager_StopStreamedFile(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );

	CStream *stream = aStream[nStream];
	
	CStream_Close(stream);

	if ( nStream == 0 )
		_bIsMp3Active = FALSE;
}

int32
SampleManager_GetStreamedFilePosition(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( CStream_IsOpened(stream) )
	{
		if ( _bIsMp3Active )
		{
			tMP3Entry *mp3 = _GetMP3EntryByIndex(_CurMP3Index);
			
			if ( mp3 != NULL )
			{
				return CStream_GetPosMS(stream) + mp3->nTrackStreamPos;
			}
			else
				return 0;
		}
		else
		{
			return CStream_GetPosMS(stream);
		}
	}
	
	return 0;
}

void
SampleManager_SetStreamedVolumeAndPan(cSampleManager *manager, uint8 nVolume, uint8 nPan, bool8 nEffectFlag, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
	
	float boostMult = 0.0f;

	if ( nVolume > MAX_VOLUME )
		nVolume = MAX_VOLUME;
	
	if ( nPan > MAX_VOLUME )
		nPan = MAX_VOLUME;

	if ( AudioSample_GetRadioInCar() == USERTRACK && !AudioSample_CheckForMusicInterruptions() )
			boostMult = manager->m_nMP3BoostVolume / 64.f;
		
	nStreamVolume[nStream] = nVolume;
	nStreamPan   [nStream] = nPan;
	
	CStream *stream = aStream[nStream];
	
	if ( CStream_IsOpened(stream) )
	{
		if ( nEffectFlag ) {
			if ( nStream == 1 || nStream == 2 )
				CStream_SetVolume(stream, 128*nVolume*manager->m_nEffectsVolume >> 14);
			else
				CStream_SetVolume(stream, manager->m_nEffectsFadeVolume*nVolume*manager->m_nEffectsVolume >> 14);
		}
		else
			CStream_SetVolume(stream, (manager->m_nMusicFadeVolume*nVolume*(uint32)(manager->m_nMusicVolume * boostMult + manager->m_nMusicVolume)) >> 14);
		
		CStream_SetPan(stream, nPan);
	}
}

int32
SampleManager_GetStreamedFileLength(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < TOTAL_STREAMED_SOUNDS );

	return nStreamLength[nStream];
}

bool8
SampleManager_IsStreamPlaying(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	ASSERT( nStream < MAX_STREAMS );
	
	CStream *stream = aStream[nStream];
	
	if ( CStream_IsOpened(stream) )
	{
		if ( CStream_IsPlaying(stream) )
			return TRUE;
	}
	
	return FALSE;
}

void
SampleManager_Service(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	for ( int32 i = 0; i < MAX_STREAMS; i++ )
	{
		CStream *stream = aStream[i];
		
		if ( CStream_IsOpened(stream) )
			CStream_Update(stream);
	}
	int refCount = CChannel_channelsThatNeedService;
	for ( int32 i = 0; refCount && i < NUM_CHANNELS; i++ )
	{
		if ( CChannel_Update(&aChannel[i]) )
			refCount--;
	}
}

bool8
SampleManager_InitialiseSampleBanks(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	int32 nBank = SFX_BANK_0;
	
	fpSampleDescHandle = fcaseopen(SampleBankDescFilename, "rb");
	if ( fpSampleDescHandle == NULL )
		return FALSE;
#ifndef OPUS_SFX
	fpSampleDataHandle = fcaseopen(SampleBankDataFilename, "rb");
	if ( fpSampleDataHandle == NULL )
	{
		fclose(fpSampleDescHandle);
		fpSampleDescHandle = NULL;
		
		return FALSE;
	}
	
	fseek(fpSampleDataHandle, 0, SEEK_END);
	int32 _nSampleDataEndOffset = ftell(fpSampleDataHandle);
	rewind(fpSampleDataHandle);
#else
	int e;
	fpSampleDataHandle = op_open_file(SampleBankDataFilename, &e);
#endif
	fread(manager->m_aSamples, sizeof(tSample), TOTAL_AUDIO_SAMPLES, fpSampleDescHandle);
#ifdef OPUS_SFX
	int32 _nSampleDataEndOffset = manager->m_aSamples[TOTAL_AUDIO_SAMPLES - 1].nOffset + manager->m_aSamples[TOTAL_AUDIO_SAMPLES - 1].nSize;
#endif
	fclose(fpSampleDescHandle);
	fpSampleDescHandle = NULL;
	
	for ( uint32 i = 0; i < TOTAL_AUDIO_SAMPLES; i++ )
	{
#ifdef FIX_BUGS
		if (nBank >= MAX_SFX_BANKS) break;
#endif
		if ( BankStartOffset[nBank] == BankStartOffset[SFX_BANK_0] + i )
		{
			nSampleBankDiscStartOffset[nBank] = manager->m_aSamples[i].nOffset;
			nBank++;
		}
	}

	nSampleBankSize[SFX_BANK_0] = nSampleBankDiscStartOffset[SFX_BANK_PED_COMMENTS] - nSampleBankDiscStartOffset[SFX_BANK_0];
	nSampleBankSize[SFX_BANK_PED_COMMENTS]  = _nSampleDataEndOffset                      - nSampleBankDiscStartOffset[SFX_BANK_PED_COMMENTS];

	return TRUE;
}

void
SampleManager_SetStreamedFileLoopFlag(cSampleManager *manager, bool8 nLoopFlag, uint8 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	nStreamLoopedFlag[nChannel] = nLoopFlag;
}

#endif
//- rouz edit (ChatGPT)
