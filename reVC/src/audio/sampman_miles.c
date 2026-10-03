//+ rouz edit (ChatGPT)
#define _CRT_SECURE_NO_WARNINGS
#include "../core/config.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef AUDIO_MSS
#define COBJMACROS
#include <windows.h>
#include <direct.h>
#include <shlobj.h>
#include <shlguid.h>

#include <time.h>

#include "eax.h"
#include "eax-util.h"
#include "mss.h"

#include "sampman.h"
#include "AudioReflectionTypes.h"
#include "oal/AudioSampleHost.h"
#include "oal/AudioStreamHost.h"
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef int8_t int8;
typedef int32_t int32;
typedef uint8_t bool8;
typedef uintptr_t uintptr;
#define TRUE 1
#define FALSE 0
#define nil NULL
#define fcaseopen fopen
#define strcasecmp _stricmp
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#define _TODOCONST(value) (value)
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
#define Clamp(value,low,high) ((value)<(low)?(low):(value)>(high)?(high):(value))
#ifndef MASTER
#define ASSERT(value) ((void)((!!(value)) || (re3_assert(#value,__FILE__,__LINE__,__func__),0)))
#define TRACE(...) re3_trace(__FILE__,__LINE__,__func__,__VA_ARGS__)
#define USERERROR(...) re3_usererror(__VA_ARGS__)
#else
#define ASSERT(value) ((void)(value))
#define TRACE(...) ((void)0)
#define USERERROR(...) ((void)0)
#endif
#define DEV(...) ((void)0)
#define debug(...) re3_debug("[DBG]: " __VA_ARGS__)
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>

#pragma comment( lib, "mss32.lib" )

cSampleManager SampleManager = {0}; // rouz edit (ChatGPT)
uint32 BankStartOffset[MAX_SFX_BANKS];
///////////////////////////////////////////////////////////////

char SampleBankDescFilename[] = "AUDIO\\SFX.SDT";
char SampleBankDataFilename[] = "AUDIO\\SFX.RAW";

FILE *fpSampleDescHandle;
FILE *fpSampleDataHandle;
int8  gBankLoaded                  [MAX_SFX_BANKS];
int32 nSampleBankDiscStartOffset   [MAX_SFX_BANKS];
int32 nSampleBankSize              [MAX_SFX_BANKS];
int32 nSampleBankMemoryStartAddress[MAX_SFX_BANKS];
int32 _nSampleDataEndOffset;

int32 nPedSlotSfx    [MAX_PEDSFX];
int32 nPedSlotSfxAddr[MAX_PEDSFX];
uint8 nCurrentPedSlot;

#ifdef FIX_BUGS
uint32 gPlayerTalkSfx = UINT32_MAX;
void *gPlayerTalkData = 0;
#endif

uint8 nChannelVolume[MAXCHANNELS+MAX2DCHANNELS];

uint32 nStreamLength[TOTAL_STREAMED_SOUNDS];

///////////////////////////////////////////////////////////////
typedef struct tMP3Entry
{
	char aFilename[MAX_PATH];
	
	uint32 nTrackLength;
	uint32 nTrackStreamPos;
	
	struct tMP3Entry *pNext;
	char *pLinkPath;
} tMP3Entry;

uint32 nNumMP3s;
tMP3Entry *_pMP3List;
char _mp3DirectoryPath[MAX_PATH];
HSTREAM mp3Stream [MAX_STREAMS];
int8 nStreamPan   [MAX_STREAMS];
int8 nStreamVolume[MAX_STREAMS];
bool8 nStreamLoopedFlag[MAX_STREAMS];
uint32 _CurMP3Index;
int32 _CurMP3Pos;
bool8 _bIsMp3Active;
///////////////////////////////////////////////////////////////


bool8 _bSampmanInitialised = FALSE;
#ifdef EXTERNAL_3D_SOUND
//
// Miscellaneous globals / defines

//	Env		Size	Diffus	Room	RoomHF	RoomLF	DecTm	DcHF	DcLF	Refl	RefDel	Ref Pan				Revb	RevDel		Rev Pan				EchTm	EchDp	ModTm	ModDp	AirAbs	HFRef		LFRef	RRlOff	FLAGS

EAXLISTENERPROPERTIES StartEAX3 =
	{26,	1.7f,	0.8f,	-1000,	-1000,	-100,	4.42f,	0.14f,	1.00f,	429,	0.014f,	0.00f,0.00f,0.00f,	1023,	0.021f,		0.00f,0.00f,0.00f,	0.250f,	0.000f,	0.250f,	0.000f,	-5.0f,	2727.1f,	250.0f,	0.00f,	0x3f };

EAXLISTENERPROPERTIES FinishEAX3 =
	{26,	100.0f,	1.0f,	0,		-1000,	-2200,	20.0f,	1.39f,	1.00f,	1000,	0.069f,	0.00f,0.00f,0.00f,	400,	0.100f,		0.00f,0.00f,0.00f,	0.250f,	1.000f,	3.982f,	0.000f,	-18.0f,	3530.8f,	417.9f,	6.70f,	0x3f };

EAXLISTENERPROPERTIES EAX3Params;

S32         prevprovider=-1;
S32         curprovider=-1;
S32         usingEAX=0;
S32         usingEAX3=0;
HPROVIDER   opened_provider=0;
H3DSAMPLE   opened_samples[MAXCHANNELS] = {0};
#endif
HSAMPLE     opened_2dsamples[MAX2DCHANNELS] = {0};
HDIGDRIVER  DIG;
#ifdef EXTERNAL_3D_SOUND
S32         speaker_type=0;

U32 _maxSamples;
float _fPrevEaxRatioDestination;
bool8 _usingMilesFast2D;
float _fEffectsLevel;


struct
{
	HPROVIDER id;
	char name[80];
}providers[MAXPROVIDERS];

typedef struct provider_stuff
{
  char* name;
  HPROVIDER id;
} provider_stuff;


static int __cdecl comp(const provider_stuff*s1,const provider_stuff*s2)
{
  return( _stricmp(s1->name,s2->name) );
}

static void
add_providers()
{
	// Keep provider records separate from the math constant macro
	provider_stuff providerInfo[MAXPROVIDERS];
	U32   n,i,j;
	
	SampleManager_SetNum3DProvidersAvailable(&SampleManager, 0);
	
	HPROENUM next = HPROENUM_FIRST;
	
	n=0;
	while (AIL_enumerate_3D_providers(&next, &providerInfo[n].id, &providerInfo[n].name) && (n<MAXPROVIDERS))
	{
		++n;
	}
	
	qsort(providerInfo,n,sizeof(providerInfo[0]), (int (__cdecl *)(const void *, const void *))comp);
	
	for(i=0;i<n;i++)
	{
		providers[i].id=providerInfo[i].id;
		strcpy(providers[i].name, providerInfo[i].name);
		SampleManager_Set3DProviderName(&SampleManager, i, providers[i].name);
	}
	
	SampleManager_SetNum3DProvidersAvailable(&SampleManager, n);
	
	for(j=n;j<MAXPROVIDERS;j++)
		SampleManager_Set3DProviderName(&SampleManager, j, NULL);
}

static void
release_existing()
{
	for ( U32 i = 0; i < _maxSamples; i++ )
	{
		if ( opened_samples[i] )
		{
			AIL_release_3D_sample_handle(opened_samples[i]);
			opened_samples[i] = NULL;
		}
	}

	if ( opened_provider )
	{
		AIL_close_3D_provider(opened_provider);
		opened_provider = 0;
	}

	_fPrevEaxRatioDestination = 0.0f;
	_usingMilesFast2D = FALSE;
	_fEffectsLevel = 0.0f;
}

static bool8
set_new_provider(S32 index)
{
	DWORD result;
	
	if ( curprovider == index )
		return TRUE;

	//close the already opened provider
	curprovider = index;
	
	release_existing();
	
	if ( curprovider != -1 )
	{
		if ( !strcmp(providers[index].name, "Dolby Surround") )
			_maxSamples = MAXCHANNELS_SURROUND;
		else
			_maxSamples = MAXCHANNELS;
		
		AIL_set_3D_provider_preference(providers[index].id, "Maximum supported samples", &_maxSamples);
		
		//load the new provider
		result = AIL_open_3D_provider(providers[index].id);
		
		if (result != M3D_NOERR) 
		{
			curprovider=-1;
			
			OutputDebugStringA(AIL_last_error());
			
			release_existing();
			
			return FALSE;
		}
		else
		{
			opened_provider=providers[index].id;
			
			//see if we're running under an EAX compatible provider
			
			if ( !strcmp(providers[index].name, "Creative Labs EAX 3 (TM)") )
			{
				usingEAX = 1;
				usingEAX3 = 1;
			}
			else
			{
				usingEAX3 = 0;

				result=AIL_3D_room_type(opened_provider);
				usingEAX=(((S32)result)!=-1)?1:0; // will be something other than -1 on EAX				
			}
			
			if ( usingEAX3 )
			{
				OutputDebugString("DOING SPECIAL EAX 3 STUFF!");
				AIL_set_3D_provider_preference(opened_provider, "EAX all parameters", &FinishEAX3);
			}
			else if ( usingEAX )
			{
				AIL_set_3D_room_type(opened_provider, ENVIRONMENT_CAVE);
				
				if ( !strcmp(providers[index].name, "Miles Fast 2D Positional Audio") )
					_usingMilesFast2D = TRUE;
			}
			
			AIL_3D_provider_attribute(opened_provider, "Maximum supported samples", &_maxSamples);
			
			if ( _maxSamples > MAXCHANNELS )
				_maxSamples = MAXCHANNELS;
			
			SampleManager_SetSpeakerConfig(&SampleManager, speaker_type);
			
			//obtain a 3D sample handles
			for ( U32 i = 0; i < _maxSamples; ++i )
			{
				opened_samples[i] = AIL_allocate_3D_sample_handle(opened_provider);
				if ( opened_samples[i] != NULL )
					AIL_set_3D_sample_effects_level(opened_samples[i], 0.0f);
			}
			
			return TRUE;
		}	
	}
	
	return FALSE;
}
#endif

U32 RadioHandlers[9];

U32 WINAPI vfs_open_callback(char const* Filename, U32* FileHandle)
{
	// Retain the SDK's 32-bit file handle and radio-file tracking
	*FileHandle = (U32)fopen(Filename, "rb");

	// couldn't they just use stricmp once? and strlen? this is very inefficient
	if ((strcmp(Filename + strlen(Filename) - 4, ".adf") == 0) || (strcmp(Filename + strlen(Filename) - 4, ".ADF") == 0)) {
		for (int i = 0; i < ARRAY_SIZE(RadioHandlers); i++) {
			if (RadioHandlers[i] == 0) {
				RadioHandlers[i] = *FileHandle;
				break;
			}
		}
		strcpy((char*)Filename + strlen(Filename) - 4, ".mp3");
	}
	return *FileHandle;
}

void WINAPI vfs_close_callback(U32 FileHandle)
{
	// Release tracked radio handles before closing their files
	for (int i = 0; i < ARRAY_SIZE(RadioHandlers); i++) {
		if (RadioHandlers[i] == FileHandle) {
			RadioHandlers[i] = 0;
			break;
		}
	}
	fclose((FILE*)FileHandle);
}

S32 WINAPI vfs_seek_callback(U32 FileHandle, S32 Offset, U32 Type)
{
	fseek((FILE*)FileHandle, Offset, Type);
	return ftell((FILE*)FileHandle);
}

U32 WINAPI vfs_read_callback(U32 FileHandle, void* Buffer, U32 Bytes)
{
	fread(Buffer, Bytes, 1, (FILE*)FileHandle);
	uint8* _Buffer = (uint8*)Buffer;

	for (int i = 0; i < ARRAY_SIZE(RadioHandlers); i++) {
		if (FileHandle == RadioHandlers[i]) {
			for (U32 k = 0; k < Bytes; k++)
				_Buffer[k] ^= 0x22;
			break;
		}
	}
	return Bytes;
}





#ifdef EXTERNAL_3D_SOUND
void
SampleManager_SetSpeakerConfig(cSampleManager *manager, int32 which)
{
    // Operate on the explicitly supplied sample-manager state

	switch ( which )
	{
		case 1:
			speaker_type=AIL_3D_2_SPEAKER;
			break;
		
		case 2:
			speaker_type=AIL_3D_HEADPHONE;
			break;
		
		case 3:
			speaker_type=AIL_3D_4_SPEAKER;
			break;
			
		default:
			return;
			break;
	}
	
	if (opened_provider)
		AIL_set_3D_speaker_type(opened_provider, speaker_type);
}

uint32
SampleManager_GetMaximumSupportedChannels(cSampleManager *manager)
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

int8
SampleManager_GetCurrent3DProviderIndex(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	return curprovider;
}

int8
SampleManager_SetCurrent3DProvider(cSampleManager *manager, uint8 nProvider)
{
    // Operate on the explicitly supplied sample-manager state

	S32 savedprovider = curprovider;
	
	if ( nProvider < manager->m_nNumberOfProviders )
	{
		if ( set_new_provider(nProvider) )
			return curprovider;
		else if ( savedprovider != -1 && savedprovider < manager->m_nNumberOfProviders && set_new_provider(savedprovider) )
			return curprovider;
		else
			return -1;
	}
	else
		return curprovider;
}

int8
SampleManager_AutoDetect3DProviders(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if (!AudioSample_IsAudioInitialised())
		return -1;

	int eax = -1, eax2 = -1, eax3 = -1, ds3dh = -1, ds3ds = -1;

	for (uint32 i = 0; i < SampleManager_GetNum3DProvidersAvailable(manager); i++)
	{
		char* providername = SampleManager_Get3DProviderName(manager, i);

		if (!strcasecmp(providername, "CREATIVE LABS EAX (TM)")) {
			AudioSample_SetCurrent3DProvider(i);
			if (SampleManager_GetCurrent3DProviderIndex(manager) == i)
				eax = i;
		}

		if (!strcasecmp(providername, "CREATIVE LABS EAX 2 (TM)")) {
			AudioSample_SetCurrent3DProvider(i);
			if (SampleManager_GetCurrent3DProviderIndex(manager) == i)
				eax2 = i;
		}

		if (!strcasecmp(providername, "CREATIVE LABS EAX 3 (TM)")) {
			AudioSample_SetCurrent3DProvider(i);
			if (SampleManager_GetCurrent3DProviderIndex(manager) == i) {
				eax3 = i;
			}
		}

		if (!strcasecmp(providername, "DIRECTSOUND3D HARDWARE SUPPORT")) {
			AudioSample_SetCurrent3DProvider(i);
			if (SampleManager_GetCurrent3DProviderIndex(manager) == i)
				ds3dh = i;
		}

		if (!strcasecmp(providername, "DIRECTSOUND3D SOFTWARE EMULATION")) {
			AudioSample_SetCurrent3DProvider(i);
			if (SampleManager_GetCurrent3DProviderIndex(manager) == i)
				ds3ds = i;
		}
	}

	if (eax3 != -1)
		return eax3;
	if (eax2 != -1)
		return eax2;
	if (eax != -1)
		return eax;
	if (ds3dh != -1)
		return ds3dh;
	if (ds3ds != -1)
		return ds3ds;
	return -1;
}
#endif

static bool8
_ResolveLink(char const *path, char *out)
{
	// Resolve Windows shortcuts through the C COM interface
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
}

static void
_FindMP3s(void)
{
	tMP3Entry *pList;
	bool8 bShortcut;	
	bool8 bInitFirstEntry;	
	HANDLE hFind;
	char path[MAX_PATH];
	char filepath[MAX_PATH*2];
	S32 total_ms;
	WIN32_FIND_DATA fd;
	
	
	if ( GetCurrentDirectory(MAX_PATH, _mp3DirectoryPath) == 0 )
	{
		GetLastError();
		return;
	}
	
	OutputDebugString("Finding MP3s...");
	strcpy(path, _mp3DirectoryPath);
	strcat(path, "\\MP3\\");
	
	strcpy(_mp3DirectoryPath, path);
	OutputDebugString(_mp3DirectoryPath);
	
	strcat(path, "*");
	
	hFind = FindFirstFile(path, &fd);
	
	if ( hFind == INVALID_HANDLE_VALUE ) 
	{
		GetLastError();
		return;
	}
	
	strcpy(filepath, _mp3DirectoryPath);
	strcat(filepath, fd.cFileName);
	
	int32 filepathlen = strlen(filepath);
	
	if ( filepathlen <= 0)
	{
		FindClose(hFind);
		return;
	}

	if ( filepathlen > 4 )
	{
		if ( !strcmp(&filepath[filepathlen - 4], ".lnk") )
		{
			if ( _ResolveLink(filepath, filepath) )
			{
				OutputDebugString("Resolving Link");
				OutputDebugString(filepath);
			}
			
			bShortcut = TRUE;
		}
		else
			bShortcut = FALSE;
	}
	
	mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
	if ( mp3Stream[0] )
	{
		AIL_stream_ms_position(mp3Stream[0], &total_ms, NULL);
		
		AIL_close_stream(mp3Stream[0]);
		mp3Stream[0] = NULL;
		
		OutputDebugString(fd.cFileName);
		
		_pMP3List = (tMP3Entry*)malloc(sizeof(tMP3Entry)); // rouz edit (ChatGPT)
		
		if ( _pMP3List == NULL )
		{
			FindClose(hFind);
			return;
		}
		
		nNumMP3s = 1;
		
		strcpy(_pMP3List->aFilename, fd.cFileName);
		
		_pMP3List->nTrackLength = total_ms;
		
		_pMP3List->pNext = NULL;
		
		pList = _pMP3List;
		
		if ( bShortcut )
		{
			_pMP3List->pLinkPath = (char*)malloc(MAX_PATH*2); // rouz edit (ChatGPT)
			strcpy(_pMP3List->pLinkPath, filepath);
		}
		else
		{
			_pMP3List->pLinkPath = NULL;
		}
		bInitFirstEntry = FALSE;
	}
	else
	{
		strcat(filepath, " - NOT A VALID MP3");
		
		OutputDebugString(filepath);

		bInitFirstEntry = TRUE;
	}
	
	while ( TRUE )
	{
		if ( !FindNextFile(hFind, &fd) )
			break;
		
		if ( bInitFirstEntry )
		{
			strcpy(filepath, _mp3DirectoryPath);
			strcat(filepath, fd.cFileName);
			
			int32 filepathlen = strlen(filepath);
			
			if ( filepathlen > 0 )
			{
				if ( filepathlen > 4 )
				{
					if ( !strcmp(&filepath[filepathlen - 4], ".lnk") )
					{
						if ( _ResolveLink(filepath, filepath) )
						{
							OutputDebugString("Resolving Link");
							OutputDebugString(filepath);
						}
						
						bShortcut = TRUE;
					}
					else
					{
						bShortcut = FALSE;
						
						if ( filepathlen > MAX_PATH )
						{
							continue;
						}
					}
				}
				
				mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
				if ( mp3Stream[0] )
				{
					AIL_stream_ms_position(mp3Stream[0], &total_ms, NULL);
					
					AIL_close_stream(mp3Stream[0]);
					mp3Stream[0] = NULL;
					
					OutputDebugString(fd.cFileName);
					
					_pMP3List = (tMP3Entry*)malloc(sizeof(tMP3Entry)); // rouz edit (ChatGPT)
					
					if ( _pMP3List  == NULL)
						break;
					
					nNumMP3s = 1;
					
					strcpy(_pMP3List->aFilename, fd.cFileName);
					
					_pMP3List->nTrackLength = total_ms;
					_pMP3List->pNext = NULL;
					
					if ( bShortcut )
					{
						_pMP3List->pLinkPath = (char*)malloc(MAX_PATH*2); // rouz edit (ChatGPT)
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
		}
		else
		{
			strcpy(filepath, _mp3DirectoryPath);
			strcat(filepath, fd.cFileName);
			
			int32 filepathlen = strlen(filepath);
			
			if ( filepathlen > 0 )
			{
				if ( filepathlen > 4 )
				{
					if ( !strcmp(&filepath[filepathlen - 4], ".lnk") )
					{
						if ( _ResolveLink(filepath, filepath) )
						{
							OutputDebugString("Resolving Link");
							OutputDebugString(filepath);
						}
						
						bShortcut = TRUE;
					}
					else
					{
						bShortcut = FALSE;
					}
				}
				
				mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
				if ( mp3Stream[0] )
				{
					AIL_stream_ms_position(mp3Stream[0], &total_ms, NULL);
					
					AIL_close_stream(mp3Stream[0]);
					mp3Stream[0] = NULL;
					
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
						e->pLinkPath = (char*)malloc(MAX_PATH*2); // rouz edit (ChatGPT)
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
	}

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

void
SampleManager_ReleaseDigitalHandle(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if ( DIG )
	{
#ifdef EXTERNAL_3D_SOUND
		prevprovider = curprovider;
		release_existing();
		curprovider = -1;
#endif
		AIL_digital_handle_release(DIG);
	}
}

void
SampleManager_ReacquireDigitalHandle(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if ( DIG )
	{
		AIL_digital_handle_reacquire(DIG);
#ifdef EXTERNAL_3D_SOUND
		if ( prevprovider != -1 )
			set_new_provider(prevprovider);
#endif
	}
}

bool8
SampleManager_Initialise(cSampleManager *manager)
{
    // Replace Miles constructor registration with one explicit setup before SDK file use
    static bool callbacksRegistered = false;
    if(!callbacksRegistered) {
        // Retain the original callback registration order before Miles startup and file reads
        AIL_set_file_callbacks(vfs_open_callback, vfs_close_callback, vfs_seek_callback, vfs_read_callback);
        callbacksRegistered = true;
    }

    // Operate on the explicitly supplied sample-manager state

	TRACE("start");
	
	if ( _bSampmanInitialised )
		return TRUE;

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

#ifdef EXTERNAL_3D_SOUND
	// miles 
	TRACE("MILES");
	{
		curprovider = -1;
		prevprovider = -1;
		
		_usingMilesFast2D = FALSE;
		usingEAX=0;
		usingEAX3=0;
		
		_fEffectsLevel = 0.0f;
		
		_maxSamples = 0;
	
		opened_provider = 0;
		DIG = NULL;
		
		for ( int32 i = 0; i < MAXCHANNELS; i++ )
			opened_samples[i] = NULL;
	}
#endif

	// banks
	TRACE("banks");
	{
		fpSampleDescHandle = NULL;
		fpSampleDataHandle = NULL;
		
		_nSampleDataEndOffset = 0;
		
		for ( int32 i = 0; i < MAX_SFX_BANKS; i++ )
		{
			gBankLoaded[i]                   = LOADING_STATUS_NOT_LOADED;
			nSampleBankDiscStartOffset[i]    = 0;
			nSampleBankSize[i]               = 0;
			nSampleBankMemoryStartAddress[i] = 0;
		}
	}
	
	// pedsfx
	TRACE("pedsfx");
	{
		for ( int32 i = 0; i < MAX_PEDSFX; i++ )
		{
			nPedSlotSfx[i]     = NO_SAMPLE;
			nPedSlotSfxAddr[i] = 0;
		}
		
		nCurrentPedSlot = 0;
	}
	
	// channel volume
	TRACE("vol");
	{
		for ( int32 i = 0; i < MAXCHANNELS+MAX2DCHANNELS; i++ )
			nChannelVolume[i] = 0;
	}
	
	TRACE("mss");
	{
		AIL_set_redist_directory( "mss" );
		
		AIL_startup();
		
		AIL_set_preference(DIG_MIXER_CHANNELS, MAX_DIGITAL_MIXER_CHANNELS);
		
		DIG = AIL_open_digital_driver(DIGITALRATE, DIGITALBITS, DIGITALCHANNELS, 0);		
		
	}
	
#ifdef AUDIO_CACHE
	TRACE("cache");
	FILE *cacheFile = fcaseopen("audio\\sound.cache", "rb");
	bool8 CreateCache = FALSE;
	if (cacheFile) {
		fread(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
		fclose(cacheFile);
	}else
		CreateCache = TRUE;
#endif
	
	char filepath[MAX_PATH];
	bool8 bFileNotFound;
	S32 tatalms;

	TRACE("cdrom");
	{
		manager->m_bInitialised = FALSE;

		
		while (TRUE)
		{

			// Find path of WAVs (originally in HDD)
			int32 drive = 'C';
			
#ifndef NO_CDCHECK
			do
			{
				char latter[2];
				
				latter[0] = drive;
				latter[1] = '\0';
				
				strcpy(manager->m_szCDRomRootPath, latter);
				strcat(manager->m_szCDRomRootPath, ":\\");
				
				if ( GetDriveType(manager->m_szCDRomRootPath) == DRIVE_CDROM )
				{
					FILE *f;
#ifdef PS2_AUDIO_PATHS
					strcpy(filepath, manager->m_szCDRomRootPath);
					strcat(filepath, PS2StreamedNameTable[0]);
					f = fopen(filepath, "rb");

					if ( !f )
#endif
					{
						strcpy(filepath, manager->m_szCDRomRootPath);
						strcat(filepath, StreamedNameTable[0]);
					
						f = fopen(filepath, "rb");
					}
					if ( f )
					{
						fclose(f);
						strcpy(manager->m_MiscomPath, manager->m_szCDRomRootPath);
						break;
					}
				}

			} while ( ++drive <= 'Z' );
#else
			manager->m_MiscomPath[0] = '\0';
#endif

			if ( DIG == NULL )
			{
				OutputDebugString(AIL_last_error());
				SampleManager_Terminate(manager);
				return FALSE;
			}

#ifdef EXTERNAL_3D_SOUND
			add_providers();
#endif

			manager->m_szCDRomRootPath[0] = '\0';

			strcpy(manager->m_WavFilesPath, manager->m_szCDRomRootPath);

#ifdef AUDIO_CACHE
			if ( CreateCache )
#endif
			for ( int32 i = STREAMED_SOUND_MISSION_MOBR1; i < TOTAL_STREAMED_SOUNDS; i++ )
			{
#ifdef PS2_AUDIO_PATHS
				strcpy(filepath, manager->m_szCDRomRootPath);
				strcat(filepath, PS2StreamedNameTable[i]);

				mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);

				if ( !mp3Stream[0] )
#endif
				{
					strcpy(filepath, manager->m_szCDRomRootPath);
					strcat(filepath, StreamedNameTable[i]);

					mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
				}
				
				if ( mp3Stream[0] )
				{
					AIL_stream_ms_position(mp3Stream[0], &tatalms, NULL);
					
					AIL_close_stream(mp3Stream[0]);
					mp3Stream[0] = NULL;
					
					nStreamLength[i] = tatalms;
				}
				else
				{
					manager->m_bInitialised = FALSE;
					SampleManager_Terminate(manager);
					return FALSE;
				}
			}

			// Find path of MP3s (originally in CD-Rom)
			// if NO_CDCHECK is NOT defined but AUDIO_CACHE is defined, we still need to find MP3s' path, but will exit after the first file
#ifndef NO_CDCHECK
			// Restart the CD scan without redeclaring the drive variable
			drive = 'C';
			do
			{
				// Keep this drive prefix within the MP3 scan scope
				char latter[2];
				latter[0] = drive;
				latter[1] = '\0';
				
				strcpy(manager->m_szCDRomRootPath, latter);
				strcat(manager->m_szCDRomRootPath, ":");
				strcat(manager->m_MP3FilesPath, manager->m_szCDRomRootPath);
#else
			manager->m_MP3FilesPath[0] = '\0';
			{
#endif

				for (int32 i = 0; i < STREAMED_SOUND_MISSION_MOBR1; i++)
				{
#ifdef PS2_AUDIO_PATHS
					strcpy(filepath, manager->m_MP3FilesPath);
					strcat(filepath, PS2StreamedNameTable[i]);

					mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);

					if ( !mp3Stream[0] )
#endif
					{
						strcpy(filepath, manager->m_MP3FilesPath);
						strcat(filepath, StreamedNameTable[i]);

						mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
					}

					if (mp3Stream[0])
					{
						AIL_stream_ms_position(mp3Stream[0], &tatalms, NULL);

						AIL_close_stream(mp3Stream[0]);
						mp3Stream[0] = NULL;

						bFileNotFound = FALSE;
#ifdef AUDIO_CACHE
						if (!CreateCache)
							break;
						else
#endif
							nStreamLength[i] = tatalms;

					}
					else
					{
						bFileNotFound = TRUE;
						break;
					}
				}

#ifndef NO_CDCHECK
				if (!bFileNotFound) // otherwise try next drive
					break;

			}
			while (++drive <= 'Z');
#else
			}
#endif

			if ( !bFileNotFound ) {

#ifdef AUDIO_CACHE
			if ( CreateCache )
#endif
				for ( int32 i = STREAMED_SOUND_MISSION_COMPLETED4; i < STREAMED_SOUND_MISSION_PAGER; i++ )
				{
#ifdef PS2_AUDIO_PATHS
					strcpy(filepath, manager->m_MiscomPath);
					strcat(filepath, PS2StreamedNameTable[i]);

					mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);

					if ( !mp3Stream[0] )
#endif
					{
						strcpy(filepath, manager->m_MiscomPath);
						strcat(filepath, StreamedNameTable[i]);
					
						mp3Stream[0] = AIL_open_stream(DIG, filepath, 0);
					}
					
					if ( mp3Stream[0] )
					{
						AIL_stream_ms_position(mp3Stream[0], &tatalms, NULL);
						
						AIL_close_stream(mp3Stream[0]);
						mp3Stream[0] = NULL;
						
						nStreamLength[i] = tatalms;
						bFileNotFound = FALSE;
					}
					else
					{
						bFileNotFound = TRUE;
						break;
					}
				}
			}
			
			manager->m_bInitialised = !bFileNotFound;

			if ( !manager->m_bInitialised )
			{
#if !defined(GTA3_STEAM_PATCH) && !defined(NO_CDCHECK)
				// Preserve the existing CD prompt and quit decision through the game callback
				if ( AudioSample_WaitForUserCD() )
				{
					SampleManager_Terminate(manager);
					return FALSE;
				}
				continue;
#else
				manager->m_bInitialised = TRUE;
#endif
			}
			
			break;
		}
	}

#ifdef AUDIO_CACHE
	if (CreateCache) {
		cacheFile = fcaseopen("audio\\sound.cache", "wb");
		fwrite(nStreamLength, sizeof(uint32), TOTAL_STREAMED_SOUNDS, cacheFile);
		fclose(cacheFile);
	}
#endif

	if ( !SampleManager_InitialiseSampleBanks(manager) )
	{
		SampleManager_Terminate(manager);
		return FALSE;
	}
	
	nSampleBankMemoryStartAddress[SFX_BANK_0] = (int32)AIL_mem_alloc_lock(nSampleBankSize[SFX_BANK_0]);
	if ( !nSampleBankMemoryStartAddress[SFX_BANK_0] )
	{
		SampleManager_Terminate(manager);
		return FALSE;
	}

	nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = (int32)AIL_mem_alloc_lock(PED_BLOCKSIZE*MAX_PEDSFX);

#ifdef FIX_BUGS
	// Find biggest player comment
	uint32 nMaxPedSize = 0;
	for (uint32 i = PLAYER_COMMENTS_START; i <= PLAYER_COMMENTS_END; i++)
		nMaxPedSize = Max(nMaxPedSize, manager->m_aSamples[i].nSize);

	gPlayerTalkData = AIL_mem_alloc_lock(nMaxPedSize);
	if ( !gPlayerTalkData )
	{
		SampleManager_Terminate(manager);
		return FALSE;
	}
#endif

	SampleManager_LoadSampleBank(manager, SFX_BANK_0);

	TRACE("stream");
	{
		for ( int32 i = 0; i < MAX_STREAMS; i++ )
		{
			mp3Stream    [i] = NULL;
			nStreamPan   [i] = 63;
			nStreamVolume[i] = 100;
		}
	}
	
	for ( int32 i = 0; i < MAX2DCHANNELS; i++ )
	{
		opened_2dsamples[i] = AIL_allocate_sample_handle(DIG);
		if ( opened_2dsamples[i] )
		{
			AIL_init_sample(opened_2dsamples[i]);
			AIL_set_sample_type(opened_2dsamples[i], DIG_F_MONO_16, DIG_PCM_SIGN);
		}
	}
	
	TRACE("providerset");
	{
		_bSampmanInitialised = TRUE;

#ifdef EXTERNAL_3D_SOUND
		U32 n = 0;
		
		while ( n < manager->m_nNumberOfProviders )
		{
			if ( !strcmp(_strupr(providers[n].name), "DIRECTSOUND3D SOFTWARE EMULATION") )
			{
				set_new_provider(n);
				break;
			}
			n++;
		}
		
		if ( n == manager->m_nNumberOfProviders )
		{
			SampleManager_Terminate(manager);
			return FALSE;
		}
#endif
	}
	
	// mp3
	TRACE("mp3");
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
	
	TRACE("end");
	
	return TRUE;
}

void
SampleManager_Terminate(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	for ( int32 i = 0; i < MAX_STREAMS; i++ )
	{
		if ( mp3Stream[i] )
		{
			AIL_pause_stream(mp3Stream[i], 1);
			AIL_close_stream(mp3Stream[i]);
			mp3Stream[i] = NULL;
		}
	}
	
	for ( int32 i = 0; i < MAX2DCHANNELS; i++ )
	{
		if ( opened_2dsamples[i] )
		{
			AIL_release_sample_handle(opened_2dsamples[i]);
			opened_2dsamples[i] = NULL;
		}
	}

#ifdef EXTERNAL_3D_SOUND
	release_existing();
#endif
	
	_DeleteMP3Entries();
	
	if ( nSampleBankMemoryStartAddress[SFX_BANK_0] != 0 )
	{
		AIL_mem_free_lock((void *)nSampleBankMemoryStartAddress[SFX_BANK_0]);
		nSampleBankMemoryStartAddress[SFX_BANK_0] = 0;
	}

	if ( nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] != 0 )
	{
		AIL_mem_free_lock((void *)nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS]);
		nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] = 0;
	}

#ifdef FIX_BUGS
	if ( gPlayerTalkData != 0)
	{
		AIL_mem_free_lock(gPlayerTalkData);
		gPlayerTalkData = 0;
	}
#endif
	
	if ( DIG )
	{
		AIL_close_digital_driver(DIG);
		DIG = NULL;
	}
	
	AIL_shutdown();
	
	_bSampmanInitialised = FALSE;
}

bool8
SampleManager_CheckForAnAudioFileOnCD(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

#if !defined(NO_CDCHECK) // TODO: check steam, probably GTAVC_STEAM_PATCH needs to be added
	char filepath[MAX_PATH];
	FILE *f;
	
	strcpy(filepath, manager->m_MiscomPath);
	strcat(filepath, StreamedNameTable[STREAMED_SOUND_MISSION_COMPLETED4]);

	f = fopen(filepath, "rb");

	if ( f )
	{
		fclose(f);
		// Restore the frontend audio preferences when the CD file is available
		AudioSample_ApplyCDVolume(true);

		return TRUE;
	}

	// Silence audio when the CD file is unavailable
	AudioSample_ApplyCDVolume(false);

	return FALSE;
	
#else
	return TRUE;
#endif // #if !defined(NO_CDCHECK)
}

char
SampleManager_GetCDAudioDriveLetter(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	if ( strlen(manager->m_MiscomPath) != 0 )
		return manager->m_MiscomPath[0];
	else
		return '\0';
}

void
SampleManager_UpdateEffectsVolume(cSampleManager *manager) //[Y], cSampleManager::UpdateSoundBuffers ?
{
    // Operate on the explicitly supplied sample-manager state

	if ( _bSampmanInitialised )
	{
		for ( int32 i = 0; i < MAXCHANNELS+MAX2DCHANNELS; i++ )
		{
#ifdef EXTERNAL_3D_SOUND
			if ( i < MAXCHANNELS )
			{
				if ( opened_samples[i] && SampleManager_GetChannelUsedFlag(manager, i) )
				{
					if ( nChannelVolume[i] )
					{
						AIL_set_3D_sample_volume(opened_samples[i],
								manager->m_nEffectsFadeVolume * nChannelVolume[i] * manager->m_nEffectsVolume >> 14);
					}
				}
			}
			else
#endif
			{
				if ( opened_2dsamples[i - MAXCHANNELS] )
				{
					if ( SampleManager_GetChannelUsedFlag(manager, i - MAXCHANNELS) )
					{
						if ( nChannelVolume[i - MAXCHANNELS] )
						{
							AIL_set_sample_volume(opened_2dsamples[i - MAXCHANNELS],
									manager->m_nEffectsFadeVolume * nChannelVolume[i - MAXCHANNELS] * manager->m_nEffectsVolume >> 14);
						}
					}
				}
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

	if ( AudioSample_IsCodePaused() )
		return FALSE;
	
	if ( AudioSample_IsMusicInitialised()
		&& AudioSample_GetMusicMode() == MUSICMODE_CUTSCENE
		&& nBank != SFX_BANK_0 )
	{
		return FALSE;
	}
	
	if ( fseek(fpSampleDataHandle, nSampleBankDiscStartOffset[nBank], SEEK_SET) != 0 )
		return FALSE;
	
	if ( fread((void *)nSampleBankMemoryStartAddress[nBank], 1, nSampleBankSize[nBank],fpSampleDataHandle) != nSampleBankSize[nBank] )
		return FALSE;
	
	gBankLoaded[nBank] = LOADING_STATUS_LOADED;
	
	return TRUE;
}

void
SampleManager_UnloadSampleBank(cSampleManager *manager, uint8 nBank)
{
    // Operate on the explicitly supplied sample-manager state

	gBankLoaded[nBank] = LOADING_STATUS_NOT_LOADED;
}

int8
SampleManager_IsSampleBankLoaded(cSampleManager *manager, uint8 nBank)
{
    // Operate on the explicitly supplied sample-manager state

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
	
	if (fseek(fpSampleDataHandle, manager->m_aSamples[nSample].nOffset, SEEK_SET) != 0)
		return FALSE;

	if (fread(gPlayerTalkData, 1, manager->m_aSamples[nSample].nSize, fpSampleDataHandle) != manager->m_aSamples[nSample].nSize)
		return FALSE;

	gPlayerTalkSfx = nSample;

	return TRUE;
}
#endif

uint8
SampleManager_IsPedCommentLoaded(cSampleManager *manager, uint32 nComment)
{
    // Operate on the explicitly supplied sample-manager state

	int8 slot;

	for ( int32 i = 0; i < _TODOCONST(3); i++ )
	{
		slot = nCurrentPedSlot - i - 1;
#ifdef FIX_BUGS
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
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

	int8 slot;

	for ( int32 i = 0; i < _TODOCONST(3); i++ )
	{
		slot = nCurrentPedSlot - i - 1;
#ifdef FIX_BUGS
		if (slot < 0)
			slot += ARRAY_SIZE(nPedSlotSfx);
#endif
		if ( nComment == nPedSlotSfx[slot] )
			return slot;
	}
	
	return -1;
}

bool8
SampleManager_LoadPedComment(cSampleManager *manager, uint32 nComment)
{
    // Operate on the explicitly supplied sample-manager state

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
	
	if ( fseek(fpSampleDataHandle, manager->m_aSamples[nComment].nOffset, SEEK_SET) != 0 )
		return FALSE;
	
	if ( fread((void *)(nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE*nCurrentPedSlot), 1, manager->m_aSamples[nComment].nSize, fpSampleDataHandle) != manager->m_aSamples[nComment].nSize )
		return FALSE;
	
	nPedSlotSfxAddr[nCurrentPedSlot] = nSampleBankMemoryStartAddress[SFX_BANK_PED_COMMENTS] + PED_BLOCKSIZE*nCurrentPedSlot;
	nPedSlotSfx    [nCurrentPedSlot] = nComment;
	
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

	return manager->m_aSamples[nSample].nFrequency;
}

uint32
SampleManager_GetSampleLoopStartOffset(cSampleManager *manager, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	return manager->m_aSamples[nSample].nLoopStart;
}

int32
SampleManager_GetSampleLoopEndOffset(cSampleManager *manager, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	return manager->m_aSamples[nSample].nLoopEnd;
}

uint32
SampleManager_GetSampleLength(cSampleManager *manager, uint32 nSample)
{
    // Operate on the explicitly supplied sample-manager state

	return manager->m_aSamples[nSample].nSize >> 1;
}

bool8
SampleManager_UpdateReverb(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	if ( !usingEAX )
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
	
	if ( usingEAX3 )
	{
		fRatio = Min(fRatio * 1.67f, 1.0f);
		if ( EAX3ListenerInterpolate(&StartEAX3, &FinishEAX3, fRatio, &EAX3Params, false) )
		{
			AIL_set_3D_provider_preference(opened_provider, "EAX all parameters", &EAX3Params);
			_fEffectsLevel = fRatio * 0.75f;
		}
	}
	else
	{
		if ( _usingMilesFast2D )
			_fEffectsLevel = fRatio * 0.8f;
		else
			_fEffectsLevel = fRatio * 0.22f;
	}
	_fEffectsLevel = Min(_fEffectsLevel, 1.0f);

	_fPrevEaxRatioDestination = fRatio;
	
	return TRUE;
#endif
	return FALSE;
}

void
SampleManager_SetChannelReverbFlag(cSampleManager *manager, uint32 nChannel, bool8 nReverbFlag)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;
	
	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( usingEAX )
	{
		if ( nReverbFlag != FALSE )
		{
			if ( !b2d )
				AIL_set_3D_sample_effects_level(opened_samples[nChannel], _fEffectsLevel);
		}
		else
		{
			if ( !b2d )
				AIL_set_3D_sample_effects_level(opened_samples[nChannel], 0.0f);
		}
	}
#endif
}

bool8
SampleManager_InitialiseChannel(cSampleManager *manager, uint32 nChannel, uint32 nSfx, uint8 nBank)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
#endif
	
	int32 addr;
	
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
				addr = nPedSlotSfxAddr[slot];
				break;
			}
		}

		if (i == _TODOCONST(3))
			return FALSE;
	}

#ifdef EXTERNAL_3D_SOUND
	if ( b2d )
	{
#endif
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
		{
			AIL_set_sample_address(opened_2dsamples[nChannel - MAXCHANNELS], (void *)addr, manager->m_aSamples[nSfx].nSize);
			return TRUE;
		}
		else
			return FALSE;
#ifdef EXTERNAL_3D_SOUND
	}
	else
	{
		AILSOUNDINFO info;
		
		info.format   = WAVE_FORMAT_PCM;
		info.data_ptr = (void *)addr;
		info.channels = 1;
		info.data_len = manager->m_aSamples[nSfx].nSize;
		info.rate     = manager->m_aSamples[nSfx].nFrequency;
		info.bits     = 16;
	
		if ( AIL_set_3D_sample_info(opened_samples[nChannel], &info) == 0 )
		{
			OutputDebugString(AIL_last_error());
			return FALSE;
		}
		
		return TRUE;
	}
#endif
}

#ifdef EXTERNAL_3D_SOUND
void
SampleManager_SetChannelEmittingVolume(cSampleManager *manager, uint32 nChannel, uint32 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

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

	if ( opened_samples[nChannel] )
		AIL_set_3D_sample_volume(opened_samples[nChannel], manager->m_nEffectsFadeVolume*nChannelVolume[nChannel]*manager->m_nEffectsVolume >> 14);

}

void
SampleManager_SetChannel3DPosition(cSampleManager *manager, uint32 nChannel, float fX, float fY, float fZ)
{
    // Operate on the explicitly supplied sample-manager state

	if ( opened_samples[nChannel] )
		AIL_set_3D_position(opened_samples[nChannel], -fX, fY, fZ);
}

void
SampleManager_SetChannel3DDistances(cSampleManager *manager, uint32 nChannel, float fMax, float fMin)
{
    // Operate on the explicitly supplied sample-manager state

	if ( opened_samples[nChannel] )
		AIL_set_3D_sample_distances(opened_samples[nChannel], fMax, fMin);
}
#endif

void
SampleManager_SetChannelVolume(cSampleManager *manager, uint32 nChannel, uint32 nVolume)
{
    // Operate on the explicitly supplied sample-manager state

	uint32 vol = nVolume;
	if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;

#ifdef EXTERNAL_3D_SOUND
	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
#endif
			nChannelVolume[nChannel] = vol;
			
			// increase the volume for JB.MP3 and S4_BDBD.MP3
			if (   AudioSample_GetMusicMode()    == MUSICMODE_CUTSCENE
				&& AudioSample_GetCurrentTrack() != STREAMED_SOUND_CUTSCENE_FINALE )
			{
				nChannelVolume[nChannel] >>= 2;
			}

			if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			{
				AIL_set_sample_volume(opened_2dsamples[nChannel - MAXCHANNELS],
						manager->m_nEffectsFadeVolume*vol*manager->m_nEffectsVolume >> 14);
			}

#ifdef EXTERNAL_3D_SOUND
			break;
		}
	}
#endif
}

void
SampleManager_SetChannelPan(cSampleManager *manager, uint32 nChannel, uint32 nPan)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
#endif
#if !defined(FIX_BUGS) && defined(EXTERNAL_3D_SOUND)
			if ( opened_samples[nChannel - MAXCHANNELS] ) // BUG
#else
			if ( opened_2dsamples[nChannel - MAXCHANNELS] )
#endif
				AIL_set_sample_pan(opened_2dsamples[nChannel - MAXCHANNELS], nPan);

#ifdef EXTERNAL_3D_SOUND
			break;
		}
	}
#endif
}

void
SampleManager_SetChannelFrequency(cSampleManager *manager, uint32 nChannel, uint32 nFreq)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}

	if ( b2d )
	{
#endif
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_set_sample_playback_rate(opened_2dsamples[nChannel - MAXCHANNELS], nFreq);
#ifdef EXTERNAL_3D_SOUND
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_set_3D_sample_playback_rate(opened_samples[nChannel], nFreq);
	}
#endif
}

void
SampleManager_SetChannelLoopPoints(cSampleManager *manager, uint32 nChannel, uint32 nLoopStart, int32 nLoopEnd)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
#endif
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_set_sample_loop_block(opened_2dsamples[nChannel - MAXCHANNELS], nLoopStart, nLoopEnd);
#ifdef EXTERNAL_3D_SOUND
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_set_3D_sample_loop_block(opened_samples[nChannel], nLoopStart, nLoopEnd);
	}
#endif
}

void
SampleManager_SetChannelLoopCount(cSampleManager *manager, uint32 nChannel, uint32 nLoopCount)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
#endif
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_set_sample_loop_count(opened_2dsamples[nChannel - MAXCHANNELS], nLoopCount);
#ifdef EXTERNAL_3D_SOUND
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_set_3D_sample_loop_count(opened_samples[nChannel], nLoopCount);
	}
#endif
}

bool8
SampleManager_GetChannelUsedFlag(cSampleManager *manager, uint32 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
#endif
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			return AIL_sample_status(opened_2dsamples[nChannel - MAXCHANNELS]) == SMP_PLAYING;
		else
			return FALSE;
#ifdef EXTERNAL_3D_SOUND
	}
	else
	{
		if ( opened_samples[nChannel] )
			return AIL_3D_sample_status(opened_samples[nChannel]) == SMP_PLAYING;
		else
			return FALSE;
	}
#endif
	
}

void
SampleManager_StartChannel(cSampleManager *manager, uint32 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}

	if ( b2d )
	{
#endif
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_start_sample(opened_2dsamples[nChannel - MAXCHANNELS]);
#ifdef EXTERNAL_3D_SOUND
	}
	else
	{
		if ( opened_samples[nChannel] )
			AIL_start_3D_sample(opened_samples[nChannel]);
	}
#endif
}

void
SampleManager_StopChannel(cSampleManager *manager, uint32 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

#ifdef EXTERNAL_3D_SOUND
	bool8 b2d = FALSE;

	switch ( nChannel )
	{
		case CHANNEL_POLICE_RADIO:
		{
			b2d = TRUE;
			break;
		}
	}
	
	if ( b2d )
	{
#endif
		if ( opened_2dsamples[nChannel - MAXCHANNELS] )
			AIL_end_sample(opened_2dsamples[nChannel - MAXCHANNELS]);
#ifdef EXTERNAL_3D_SOUND
	}
	else
	{
		if ( opened_samples[nChannel] )
		{
			if ( AIL_3D_sample_status(opened_samples[nChannel]) == SMP_PLAYING )
				AIL_end_3D_sample(opened_samples[nChannel]);
		}
	}
#endif
}

void
SampleManager_PreloadStreamedFile(cSampleManager *manager, uint32 nFile, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	if ( manager->m_bInitialised  )
	{
		if ( nFile < TOTAL_STREAMED_SOUNDS )
		{
			if ( mp3Stream[nStream] )
			{
				AIL_pause_stream(mp3Stream[nStream], 1);
				AIL_close_stream(mp3Stream[nStream]);
			}
			
			char filepath[MAX_PATH];
#ifdef PS2_AUDIO_PATHS
			strcpy(filepath, nFile < STREAMED_SOUND_MISSION_COMPLETED4 ? manager->m_MP3FilesPath : (nFile < STREAMED_SOUND_MISSION_MOBR1 ? manager->m_MiscomPath : manager->m_WavFilesPath));
			strcat(filepath, PS2StreamedNameTable[nFile]);

			mp3Stream[nStream] = AIL_open_stream(DIG, filepath, 0);

			if ( !mp3Stream[nStream] )
#endif
			{
				strcpy(filepath, nFile < STREAMED_SOUND_MISSION_COMPLETED4 ? manager->m_MP3FilesPath : (nFile < STREAMED_SOUND_MISSION_MOBR1 ? manager->m_MiscomPath : manager->m_WavFilesPath));
				strcat(filepath, StreamedNameTable[nFile]);
			
				mp3Stream[nStream] = AIL_open_stream(DIG, filepath, 0);
			}
	
			if ( mp3Stream[nStream] )
			{
				AIL_set_stream_loop_count(mp3Stream[nStream], 1);
				AIL_service_stream(mp3Stream[nStream], 1);
			}
			else
				OutputDebugString(AIL_last_error());
		}
	}
}

void
SampleManager_PauseStream(cSampleManager *manager, bool8 nPauseFlag, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	if ( manager->m_bInitialised )
	{
		if ( mp3Stream[nStream] )
			AIL_pause_stream(mp3Stream[nStream], nPauseFlag != FALSE);
	}
}

void
SampleManager_StartPreloadedStreamedFile(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	if ( manager->m_bInitialised )
	{
		if ( mp3Stream[nStream] )
			AIL_start_stream(mp3Stream[nStream]);
	}
}

bool8
SampleManager_StartStreamedFile(cSampleManager *manager, uint32 nFile, uint32 nPos, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	uint32 i = 0;
	uint32 position = nPos;
	char filename[MAX_PATH];
	
	if ( !manager->m_bInitialised || nFile >= TOTAL_STREAMED_SOUNDS )
		return FALSE;

	if ( mp3Stream[nStream] )
	{
		AIL_pause_stream(mp3Stream[nStream], 1);
		AIL_close_stream(mp3Stream[nStream]);
	}
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
#ifdef PS2_AUDIO_PATHS
					strcpy(filename, manager->m_MiscomPath);
					strcat(filename, PS2StreamedNameTable[nFile]);

					mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);

					if ( !mp3Stream[nStream] )
#endif
					{
						strcpy(filename, manager->m_MiscomPath);
						strcat(filename, StreamedNameTable[nFile]);
						mp3Stream[nStream] =
						    AIL_open_stream(DIG, filename, 0);
					}
					if(mp3Stream[nStream]) {
						AIL_set_stream_loop_count(mp3Stream[nStream], nStreamLoopedFlag[nStream] ? 0 : 1);
						nStreamLoopedFlag[nStream] = TRUE;
						AIL_set_stream_ms_position(mp3Stream[nStream], position);
						AIL_pause_stream(mp3Stream[nStream], 0);
						return TRUE;
					}
					return FALSE;

				} else {
					if ( e->pLinkPath != NULL )
						mp3Stream[nStream] = AIL_open_stream(DIG, e->pLinkPath, 0);
					else {
						strcpy(filename, _mp3DirectoryPath);
						strcat(filename, e->aFilename);
					
						mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);
					}
										
					if ( mp3Stream[nStream] ) {
						AIL_set_stream_loop_count(mp3Stream[nStream], 1);
						AIL_set_stream_ms_position(mp3Stream[nStream], position);
						AIL_pause_stream(mp3Stream[nStream], 0);
						
						_bIsMp3Active = TRUE;
				
						return TRUE;
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
#ifdef PS2_AUDIO_PATHS
						strcpy(filename, manager->m_MiscomPath);
						strcat(filename, PS2StreamedNameTable[nFile]);

						mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);

						if ( !mp3Stream[nStream] )
#endif
						{
							strcpy(filename, manager->m_MiscomPath);
							strcat(filename, StreamedNameTable[nFile]);
							mp3Stream[nStream] =
							    AIL_open_stream(DIG, filename, 0);
						}
						if(mp3Stream[nStream]) {
							AIL_set_stream_loop_count(
							    mp3Stream[nStream], nStreamLoopedFlag[nStream] ? 0 : 1);
							nStreamLoopedFlag[nStream] = TRUE;
							AIL_set_stream_ms_position(
							    mp3Stream[nStream], position);
							AIL_pause_stream(mp3Stream[nStream],
							                 0);
							return TRUE;
						}
						return FALSE;
					}
				}
				if(mp3->pLinkPath != NULL)
					mp3Stream[nStream] = AIL_open_stream(DIG, mp3->pLinkPath, 0);
				else {
					strcpy(filename, _mp3DirectoryPath);
					strcat(filename, mp3->aFilename);

					mp3Stream[nStream] =
					    AIL_open_stream(DIG, filename, 0);
				}

				if(mp3Stream[nStream]) {
					AIL_set_stream_loop_count(mp3Stream[nStream], 1);
					AIL_set_stream_ms_position(mp3Stream[nStream], 0);
					AIL_pause_stream(mp3Stream[nStream], 0);
#ifdef FIX_BUGS
					_bIsMp3Active = TRUE;
#endif
					return TRUE;
				}

			}
			_bIsMp3Active = FALSE;
		}
		while ( ++i < nNumMP3s );
		position = 0;
		nFile = 0;
	}
#ifdef PS2_AUDIO_PATHS
	strcpy(filename, manager->m_MiscomPath);
	strcat(filename, PS2StreamedNameTable[nFile]);

	mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);

	if ( !mp3Stream[nStream] )
#endif
	{
		strcpy(filename, manager->m_MiscomPath);
		strcat(filename, StreamedNameTable[nFile]);
		mp3Stream[nStream] = AIL_open_stream(DIG, filename, 0);
	}

	if ( mp3Stream[nStream] )
	{
		AIL_set_stream_loop_count(mp3Stream[nStream], nStreamLoopedFlag[nStream] ? 0 : 1);
		nStreamLoopedFlag[nStream] = TRUE;
		AIL_set_stream_ms_position(mp3Stream[nStream], position);
		AIL_pause_stream(mp3Stream[nStream], 0);
		return TRUE;
	}
	return FALSE;
}

void
SampleManager_StopStreamedFile(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	if ( manager->m_bInitialised )
	{
		if ( mp3Stream[nStream] )
		{
			AIL_pause_stream(mp3Stream[nStream], 1);
			
			AIL_close_stream(mp3Stream[nStream]);
			mp3Stream[nStream] = NULL;
			
			if ( nStream == 0 )
				_bIsMp3Active = FALSE;
		}
	}
}

int32
SampleManager_GetStreamedFilePosition(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	S32 currentms;
	
	if ( manager->m_bInitialised )
	{
		if ( mp3Stream[nStream] )
		{
			if ( _bIsMp3Active )
			{
				tMP3Entry *mp3 = _GetMP3EntryByIndex(_CurMP3Index);
				
				if ( mp3 != NULL )
				{
					AIL_stream_ms_position(mp3Stream[nStream], NULL, &currentms);
					return currentms + mp3->nTrackStreamPos;
				}
				else
					return 0;
			}
			else
			{
				AIL_stream_ms_position(mp3Stream[nStream], NULL, &currentms);
				return currentms;
			}
		}
	}
	
	return 0;
}

void
SampleManager_SetStreamedVolumeAndPan(cSampleManager *manager, uint8 nVolume, uint8 nPan, bool8 nEffectFlag, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	uint8 vol = nVolume;
	float boostMult = 0.0f;
	
	if ( manager->m_bInitialised )
	{
		if ( vol > MAX_VOLUME ) vol = MAX_VOLUME;
		
		if ( AudioSample_GetRadioInCar() == USERTRACK && !AudioSample_CheckForMusicInterruptions() )
			boostMult = manager->m_nMP3BoostVolume / 64.f;
		
		nStreamVolume[nStream] = vol;
		nStreamPan[nStream]    = nPan;
		
		if ( mp3Stream[nStream] )
		{
			if ( nEffectFlag )
			{
				if ( nStream == 1 || nStream == 2 )
					AIL_set_stream_volume(mp3Stream[nStream], 128*vol*manager->m_nEffectsVolume >> 14);
				else
					AIL_set_stream_volume(mp3Stream[nStream], manager->m_nEffectsFadeVolume*vol*manager->m_nEffectsVolume >> 14);
			}
			else
				AIL_set_stream_volume(mp3Stream[nStream], (manager->m_nMusicFadeVolume*vol*(uint32)(manager->m_nMusicVolume * boostMult + manager->m_nMusicVolume)) >> 14);
			
			AIL_set_stream_pan(mp3Stream[nStream], nPan);
		}
	}
}

int32
SampleManager_GetStreamedFileLength(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	if ( manager->m_bInitialised )
		return nStreamLength[nStream];
	
	return 0;
}

bool8
SampleManager_IsStreamPlaying(cSampleManager *manager, uint8 nStream)
{
    // Operate on the explicitly supplied sample-manager state

	if ( manager->m_bInitialised )
	{
		if ( mp3Stream[nStream] )
		{
			if ( AIL_stream_status(mp3Stream[nStream]) == SMP_PLAYING )
				return TRUE;
			else
				return FALSE;
		}
	}
	
	return FALSE;
}

bool8
SampleManager_InitialiseSampleBanks(cSampleManager *manager)
{
    // Operate on the explicitly supplied sample-manager state

	int32 nBank = SFX_BANK_0;
	
	fpSampleDescHandle = fopen(SampleBankDescFilename, "rb");
	if ( fpSampleDescHandle == NULL )
		return FALSE;
	
	fpSampleDataHandle = fopen(SampleBankDataFilename, "rb");
	if ( fpSampleDataHandle == NULL )
	{
		fclose(fpSampleDescHandle);
		fpSampleDescHandle = NULL;
		
		return FALSE;
	}
	
	fseek(fpSampleDataHandle, 0, SEEK_END);
	_nSampleDataEndOffset = ftell(fpSampleDataHandle);
	rewind(fpSampleDataHandle);
	
	fread(manager->m_aSamples, sizeof(tSample), TOTAL_AUDIO_SAMPLES, fpSampleDescHandle);
	
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
	nSampleBankSize[SFX_BANK_PED_COMMENTS] = _nSampleDataEndOffset                       - nSampleBankDiscStartOffset[SFX_BANK_PED_COMMENTS];
	
	return TRUE;
}


void
SampleManager_SetStreamedFileLoopFlag(cSampleManager *manager, bool8 nLoopFlag, uint8 nChannel)
{
    // Operate on the explicitly supplied sample-manager state

	if (manager->m_bInitialised)
		nStreamLoopedFlag[nChannel] = nLoopFlag;
}

#endif
//- rouz edit (ChatGPT)
