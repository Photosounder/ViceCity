#ifndef ALDEVICELIST_H
#define ALDEVICELIST_H

//+ rouz edit (ChatGPT)
#ifdef AUDIO_OAL
#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#define AL_DEVICE_CAPACITY 64
enum
{
	ADEXT_EXT_CAPTURE = (1 << 0),
	ADEXT_EXT_EFX = (1 << 1),
	ADEXT_EXT_OFFSET = (1 << 2),
	ADEXT_EXT_LINEAR_DISTANCE = (1 << 3),
	ADEXT_EXT_EXPONENT_DISTANCE = (1 << 4),
	ADEXT_EAX2 = (1 << 5),
	ADEXT_EAX3 = (1 << 6),
	ADEXT_EAX4 = (1 << 7),
	ADEXT_EAX5 = (1 << 8),
	ADEXT_EAX_RAM = (1 << 9),
};

typedef struct ALDEVICEINFO {
	char		   *strDeviceName;
	int				iMajorVersion;
	int				iMinorVersion;
	unsigned int	uiSourceCount;
	unsigned short  Extensions;
	bool			bSelected;
} ALDEVICEINFO;
typedef ALDEVICEINFO *LPALDEVICEINFO;
typedef struct ALDeviceList {
    ALDEVICEINFO aDeviceInfo[AL_DEVICE_CAPACITY];
    unsigned int nNumOfDevices;
    int defaultDeviceIndex;
    int filterIndex;
} ALDeviceList;
#ifdef __cplusplus
extern "C" {
#endif
void ALDeviceList_Init(ALDeviceList *list);
void ALDeviceList_Destroy(ALDeviceList *list);
unsigned int ALDeviceList_GetNumDevices(ALDeviceList *list);
const char * ALDeviceList_GetDeviceName(ALDeviceList *list, unsigned int index);
void ALDeviceList_GetDeviceVersion(ALDeviceList *list, unsigned int index, int *major, int *minor);
unsigned int ALDeviceList_GetMaxNumSources(ALDeviceList *list, unsigned int index);
bool ALDeviceList_IsExtensionSupported(ALDeviceList *list, int index, unsigned short ext);
int ALDeviceList_GetDefaultDevice(ALDeviceList *list);
void ALDeviceList_FilterDevicesMinVer(ALDeviceList *list, int major, int minor);
void ALDeviceList_FilterDevicesMaxVer(ALDeviceList *list, int major, int minor);
void ALDeviceList_FilterDevicesExtension(ALDeviceList *list, unsigned short ext);
void ALDeviceList_ResetFilters(ALDeviceList *list);
int ALDeviceList_GetFirstFilteredDevice(ALDeviceList *list);
int ALDeviceList_GetNextFilteredDevice(ALDeviceList *list);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
#endif
