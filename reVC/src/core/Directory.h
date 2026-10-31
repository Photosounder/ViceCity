//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct DirectoryInfo {
	uint32_t offset;
	uint32_t size;
	char name[24];
} DirectoryInfo;
typedef struct CDirectory {
	DirectoryInfo *entries;
	int32_t maxEntries;
	int32_t numEntries;
} CDirectory;
#ifdef __cplusplus
extern "C" {
#endif
void Directory_Initialize(CDirectory *directory, int32_t capacity);
void Directory_Shutdown(CDirectory *directory);
void Directory_ReadDirFile(CDirectory *directory, const char *filename);
bool Directory_WriteDirFile(CDirectory *directory, const char *filename);
void Directory_AddItem(CDirectory *directory, const DirectoryInfo *dirinfo);
void Directory_AddItemWithImage(CDirectory *directory, const DirectoryInfo *dirinfo, int32_t imgId);
bool Directory_FindItem(CDirectory *directory, const char *name, uint32_t *offset, uint32_t *size);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
