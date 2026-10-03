/*
 * Copyright (c) 2006, Creative Labs Inc.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided
 * that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, this list of conditions and
 * 	     the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 * 	     and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *     * Neither the name of Creative Labs Inc. nor the names of its contributors may be used to endorse or
 * 	     promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

//+ rouz edit (ChatGPT)
#include "aldlist.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif
#include <cita_windows.h>

#ifdef AUDIO_OAL
/* 
 * Init call
 */
static unsigned int ALDeviceList_ProbeMaxSources(void);

void ALDeviceList_Init(ALDeviceList *list)
{
    // Initialize all embedded records before caching device-name ownership
    memset(list, 0, sizeof(*list));

	char *devices;
	int index;
	const char *defaultDeviceName;
	const char *actualDeviceName;

	// DeviceInfo vector stores, for each enumerated device, it's device name, selection status, spec version #, and extension support
	list->nNumOfDevices = 0;

	list->defaultDeviceIndex = 0;

	if (alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT")) {
		devices = (char *)alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
		defaultDeviceName = (char *)alcGetString(NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
		
		index = 0;
		// go through device list (each device terminated with a single NULL, list terminated with double NULL)
		while (*devices != '\0') {
			if (strcmp(defaultDeviceName, devices) == 0) {
				list->defaultDeviceIndex = index;
			}
			ALCdevice *device = alcOpenDevice(devices);
			if (device) {
				ALCcontext *context = alcCreateContext(device, NULL);
				if (context) {
					alcMakeContextCurrent(context);
					// if new actual device name isn't already in the list, then add it...
					actualDeviceName = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
					if ((actualDeviceName != NULL) && (strlen(actualDeviceName) > 0) && (list->nNumOfDevices < AL_DEVICE_CAPACITY)) {
						ALDEVICEINFO *ALDeviceInfo = &list->aDeviceInfo[list->nNumOfDevices++];
						ALDeviceInfo->bSelected = true;
						// Allocate the owned device name at the enumeration call site for CITA attribution
                        ALDeviceInfo->strDeviceName = (char*)malloc(strlen(actualDeviceName) + 1);
                        if(!ALDeviceInfo->strDeviceName) abort();
                        strcpy(ALDeviceInfo->strDeviceName, actualDeviceName);
						alcGetIntegerv(device, ALC_MAJOR_VERSION, sizeof(int), &ALDeviceInfo->iMajorVersion);
						alcGetIntegerv(device, ALC_MINOR_VERSION, sizeof(int), &ALDeviceInfo->iMinorVersion);

						// Check for ALC Extensions
						if (alcIsExtensionPresent(device, "ALC_EXT_CAPTURE") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EXT_CAPTURE;
						if (alcIsExtensionPresent(device, "ALC_EXT_EFX") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EXT_EFX;

						// Check for AL Extensions
						if (alIsExtensionPresent("AL_EXT_OFFSET") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EXT_OFFSET;

						if (alIsExtensionPresent("AL_EXT_LINEAR_DISTANCE") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EXT_LINEAR_DISTANCE;
						if (alIsExtensionPresent("AL_EXT_EXPONENT_DISTANCE") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EXT_EXPONENT_DISTANCE;
						
						if (alIsExtensionPresent("EAX2.0") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EAX2;
						if (alIsExtensionPresent("EAX3.0") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EAX3;
						if (alIsExtensionPresent("EAX4.0") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EAX4;
						if (alIsExtensionPresent("EAX5.0") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EAX5;

						if (alIsExtensionPresent("EAX-RAM") == AL_TRUE)
							ALDeviceInfo->Extensions |= ADEXT_EAX_RAM;

						// Get Source Count
						ALDeviceInfo->uiSourceCount = ALDeviceList_ProbeMaxSources();
					}
					alcMakeContextCurrent(NULL);
					alcDestroyContext(context);
				}
				alcCloseDevice(device);
			}
			devices += strlen(devices) + 1;
			index += 1;
		}
	}

	ALDeviceList_ResetFilters(list);
}

/* 
 * Exit call
 */
void ALDeviceList_Destroy(ALDeviceList *list)
{
    // Release cached names after their final use without freeing caller-owned list storage
    for(unsigned int i = AL_DEVICE_CAPACITY; i-- > 0;) {
        // Release each initialized record and make repeated cleanup safe
        free(list->aDeviceInfo[i].strDeviceName);
        list->aDeviceInfo[i].strDeviceName = NULL;
    }
    list->nNumOfDevices = 0;
    list->defaultDeviceIndex = 0;
    list->filterIndex = 0;
}

/*
 * Returns the number of devices in the complete device list
 */
unsigned int ALDeviceList_GetNumDevices(ALDeviceList *list)
{
    // Query or filter explicitly owned C device-list state

	return list->nNumOfDevices;
}

/* 
 * Returns the device name at an index in the complete device list
 */
const char * ALDeviceList_GetDeviceName(ALDeviceList *list, unsigned int index)
{
    // Query or filter explicitly owned C device-list state

	if (index < ALDeviceList_GetNumDevices(list))
		return list->aDeviceInfo[index].strDeviceName;
	else
		return NULL;
}

/*
 * Returns the major and minor version numbers for a device at a specified index in the complete list
 */
void ALDeviceList_GetDeviceVersion(ALDeviceList *list, unsigned int index, int *major, int *minor)
{
    // Query or filter explicitly owned C device-list state

	if (index < ALDeviceList_GetNumDevices(list)) {
		if (major)
			*major = list->aDeviceInfo[index].iMajorVersion;
		if (minor)
			*minor = list->aDeviceInfo[index].iMinorVersion;
	}
	return;
}

/*
 * Returns the maximum number of Sources that can be generate on the given device
 */
unsigned int ALDeviceList_GetMaxNumSources(ALDeviceList *list, unsigned int index)
{
    // Query or filter explicitly owned C device-list state

	if (index < ALDeviceList_GetNumDevices(list))
		return list->aDeviceInfo[index].uiSourceCount;
	else
		return 0;
}

/*
 * Checks if the extension is supported on the given device
 */
bool ALDeviceList_IsExtensionSupported(ALDeviceList *list, int index, unsigned short ext)
{
    // Query or filter explicitly owned C device-list state

	// Reject invalid indexes before inspecting the fixed device array
    if(index < 0 || (unsigned int)index >= AL_DEVICE_CAPACITY || (unsigned int)index >= list->nNumOfDevices) return false;
    return !!(list->aDeviceInfo[index].Extensions & ext);
}

/*
 * returns the index of the default device in the complete device list
 */
int ALDeviceList_GetDefaultDevice(ALDeviceList *list)
{
    // Query or filter explicitly owned C device-list state

	return list->defaultDeviceIndex;
}

/* 
 * Deselects devices which don't have the specified minimum version
 */
void ALDeviceList_FilterDevicesMinVer(ALDeviceList *list, int major, int minor)
{
    // Query or filter explicitly owned C device-list state

	int dMajor, dMinor;
	for (unsigned int i = 0; i < list->nNumOfDevices; i++) {
		ALDeviceList_GetDeviceVersion(list, i, &dMajor, &dMinor);
		if ((dMajor < major) || ((dMajor == major) && (dMinor < minor))) {
			list->aDeviceInfo[i].bSelected = false;
		}
	}
}

/* 
 * Deselects devices which don't have the specified maximum version
 */
void ALDeviceList_FilterDevicesMaxVer(ALDeviceList *list, int major, int minor)
{
    // Query or filter explicitly owned C device-list state

	int dMajor, dMinor;
	for (unsigned int i = 0; i < list->nNumOfDevices; i++) {
		ALDeviceList_GetDeviceVersion(list, i, &dMajor, &dMinor);
		if ((dMajor > major) || ((dMajor == major) && (dMinor > minor))) {
			list->aDeviceInfo[i].bSelected = false;
		}
	}
}

/*
 * Deselects device which don't support the given extension name
 */
void
ALDeviceList_FilterDevicesExtension(ALDeviceList *list, unsigned short ext)
{
    // Query or filter explicitly owned C device-list state

	for (unsigned int i = 0; i < list->nNumOfDevices; i++) {
		if (!ALDeviceList_IsExtensionSupported(list, i, ext))
			list->aDeviceInfo[i].bSelected = false;
	}
}

/*
 * Resets all filtering, such that all devices are in the list
 */
void ALDeviceList_ResetFilters(ALDeviceList *list)
{
    // Query or filter explicitly owned C device-list state

	for (unsigned int i = 0; i < ALDeviceList_GetNumDevices(list); i++) {
		list->aDeviceInfo[i].bSelected = true;
	}
	list->filterIndex = 0;
}

/*
 * Gets index of first filtered device
 */
int ALDeviceList_GetFirstFilteredDevice(ALDeviceList *list)
{
    // Query or filter explicitly owned C device-list state

	unsigned int i;

	for (i = 0; i < ALDeviceList_GetNumDevices(list); i++) {
		if (list->aDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	list->filterIndex = i + 1;
	return i;
}

/*
 * Gets index of next filtered device
 */
int ALDeviceList_GetNextFilteredDevice(ALDeviceList *list)
{
    // Query or filter explicitly owned C device-list state

	unsigned int i;

	for (i = list->filterIndex; i < ALDeviceList_GetNumDevices(list); i++) {
		if (list->aDeviceInfo[i].bSelected == true) {
			break;
		}
	}
	list->filterIndex = i + 1;
	return i;
}

/*
 * Internal function to detemine max number of Sources that can be generated
 */
static unsigned int ALDeviceList_ProbeMaxSources(void)
{
    // Probe source capacity while retaining only initialized handles for cleanup
	ALuint uiSources[256];
	unsigned int iSourceCount = 0;

	// Clear AL Error Code
	alGetError();

	// Generate up to 256 Sources, checking for any errors
	for (iSourceCount = 0; iSourceCount < 256; iSourceCount++)
	{
		alGenSources(1, &uiSources[iSourceCount]);
		if (alGetError() != AL_NO_ERROR)
			break;
	}

	// Release the Sources
	alDeleteSources(iSourceCount, uiSources);
	if (alGetError() != AL_NO_ERROR)
	{
		// Retry cleanup only for handles that were successfully generated
        for (unsigned int i = 0; i < iSourceCount; i++)
		{
			alDeleteSources(1, &uiSources[i]);
		}
	}

	return iSourceCount;
}
#endif

//- rouz edit (ChatGPT)
