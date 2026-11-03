//+ rouz edit (ChatGPT)
#include "config.h"
#if !defined(_WIN32) && (defined(AUDIO_OAL) || defined(AUDIO_MSS))
#include "AudioSampleHost.h"
#include "PlatformFind.h"
#include <string.h>

static void CopyFindData(AudioSampleFindData *out, const WIN32_FIND_DATA *data)
{
    // Copy the existing directory search state without adding allocations
    memcpy(out->extension, data->extension, sizeof(out->extension));
    memcpy(out->folder, data->folder, sizeof(out->folder));
    memcpy(out->cFileName, data->cFileName, sizeof(out->cFileName));
    out->ftLastWriteTime = data->ftLastWriteTime;
}
void *AudioSample_FindFirstFile(const char *path, AudioSampleFindData *out)
{
    // Use the platform's original wildcard enumeration and directory handle
    WIN32_FIND_DATA data = {0};
    HANDLE handle = FindFirstFile(path, &data);
    CopyFindData(out, &data);
    return handle;
}
bool AudioSample_FindNextFile(void *handle, AudioSampleFindData *data)
{
    // Restore search state and return the platform's next directory entry
    WIN32_FIND_DATA native = {0};
    memcpy(native.extension, data->extension, sizeof(native.extension));
    memcpy(native.folder, data->folder, sizeof(native.folder));
    memcpy(native.cFileName, data->cFileName, sizeof(native.cFileName));
    native.ftLastWriteTime = data->ftLastWriteTime;
    bool found = FindNextFile(handle, &native);
    CopyFindData(data, &native);
    return found;
}
void AudioSample_FindClose(void *handle)
{
    // Close the original platform directory handle
    FindClose(handle);
}
#endif
//- rouz edit (ChatGPT)
